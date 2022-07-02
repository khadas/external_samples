
#include <stdio.h>
#include <sys/poll.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

#include "rk_type.h"
#include "rk_debug.h"
#include "rk_defines.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_sys.h"
#include "rk_aiq_user_api2_camgroup.h"
#include "rk_aiq_user_api2_imgproc.h"
#include "rk_aiq_user_api2_sysctl.h"

//#define RKAIQ
//#define ENABLE_GET_STREAM

static FILE *venc0_file = NULL;
static RK_S32 g_s32FrameCnt = -1;
static RK_U32 g_u32WrapLine = 0;
static char *g_sEntityName = NULL;
static bool quit = false;

static void sigterm_handler(int sig) {
    quit = true;
}

#ifdef ENABLE_GET_STREAM
RK_U64 TEST_COMM_GetNowUs() {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (RK_U64)time.tv_sec * 1000000 + (RK_U64)time.tv_nsec / 1000; /* microseconds */
}

static void  *GetMediaBuffer0(void *arg) {
    (void)arg;
    void *pData = RK_NULL;
    int loopCount = 0;
    int s32Ret;
    VENC_STREAM_S stFrame;

    FILE *fp = fopen("/tmp/pts.txt", "wb");

    stFrame.pstPack = malloc(sizeof(VENC_PACK_S));

    while (!quit) {
        s32Ret = RK_MPI_VENC_GetStream(0, &stFrame, -1);
        if (s32Ret == RK_SUCCESS) {
            if (venc0_file) {
                pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
                fwrite(pData, 1, stFrame.pstPack->u32Len, venc0_file);
                fflush(venc0_file);
            }
            RK_U64 nowUs = TEST_COMM_GetNowUs();

            printf("chn:0, loopCount:%d enc->seq:%d wd:%d pts=%lld delay=%lldus\n", loopCount,
                    stFrame.u32Seq, stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS, nowUs - stFrame.pstPack->u64PTS);
            if (fp) {
                char str[128];
                snprintf(str, sizeof(str), "seq:%u, pts:%llums\n", stFrame.u32Seq, stFrame.pstPack->u64PTS / 1000);
                fputs(str, fp);
                fsync(fileno(fp));
            }

            s32Ret = RK_MPI_VENC_ReleaseStream(0, &stFrame);
            loopCount++;
        } else {
            printf("RK_MPI_VENC_GetChnFrame fail %x\n", s32Ret);
        }

        if ((g_s32FrameCnt >= 0) && (loopCount > g_s32FrameCnt)) {
            quit = true;
            break;
        }

        usleep(10*1000);
    }

    if (venc0_file)
        fclose(venc0_file);

    if (fp)
        fclose(fp);

    free(stFrame.pstPack);
    return NULL;
}
#endif

static RK_S32 test_venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType)
{
    VENC_RECV_PIC_PARAM_S stRecvParam;
    VENC_CHN_BUF_WRAP_S stVencChnBufWrap;
    VENC_CHN_ATTR_S stAttr;

    memset(&stAttr,0,sizeof(VENC_CHN_ATTR_S));
    stVencChnBufWrap.bEnable = false;
    stVencChnBufWrap.u32BufLine = 1080;

    if (enType == RK_VIDEO_ID_AVC) {
        stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        stAttr.stRcAttr.stH264Cbr.u32BitRate = 2 * 1024;
        stAttr.stRcAttr.stH264Cbr.u32Gop = 60;
    } else if (enType == RK_VIDEO_ID_HEVC) {
        stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
        stAttr.stRcAttr.stH265Cbr.u32BitRate = 2 * 1024;
        stAttr.stRcAttr.stH265Cbr.u32Gop = 60;
    }

    stAttr.stVencAttr.enType = enType;
    stAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
    if (enType == RK_VIDEO_ID_AVC)
        stAttr.stVencAttr.u32Profile = H264E_PROFILE_HIGH;
    else if (enType == RK_VIDEO_ID_HEVC)
        stAttr.stVencAttr.u32Profile = H265E_PROFILE_MAIN;
    stAttr.stVencAttr.u32PicWidth = width;
    stAttr.stVencAttr.u32PicHeight = height;
    stAttr.stVencAttr.u32VirWidth = width;
    stAttr.stVencAttr.u32VirHeight = height;
    stAttr.stVencAttr.u32StreamBufCnt = 5;
    stAttr.stVencAttr.u32BufSize = width * height * 3 / 2;
    stAttr.stVencAttr.enMirror = MIRROR_NONE;

    RK_MPI_VENC_CreateChn(chnId, &stAttr);
    RK_MPI_VENC_SetChnBufWrapAttr(chnId, &stVencChnBufWrap);

    memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));
    stRecvParam.s32RecvPicNum = -1;
    RK_MPI_VENC_StartRecvFrame(chnId, &stRecvParam);

    return 0;
}

//demo板dev默认都是0，根据不同的channel 来选择不同的vi节点
int vi_dev_init() {
    //printf("%s\n", __func__);
    int ret = 0;
    int devId=0;
    int pipeId = devId;

    VI_DEV_ATTR_S stDevAttr;
    VI_DEV_BIND_PIPE_S stBindPipe;
    memset(&stDevAttr, 0, sizeof(stDevAttr));
    memset(&stBindPipe, 0, sizeof(stBindPipe));
    // 0. get dev config status
    ret = RK_MPI_VI_GetDevAttr(devId, &stDevAttr);
    if (ret == RK_ERR_VI_NOT_CONFIG) {
        // 0-1.config dev
        ret = RK_MPI_VI_SetDevAttr(devId, &stDevAttr);
        if (ret != RK_SUCCESS) {
            return -1;
        }
    }

    // 1.get dev enable status
    ret = RK_MPI_VI_GetDevIsEnable(devId);
    if (ret != RK_SUCCESS) {
        // 1-2.enable dev
        ret = RK_MPI_VI_EnableDev(devId);
        if (ret != RK_SUCCESS) {
            return -1;
        }
        // 1-3.bind dev/pipe
        stBindPipe.u32Num = pipeId;
        stBindPipe.PipeId[0] = pipeId;
        ret = RK_MPI_VI_SetDevBindPipe(devId, &stBindPipe);
        if (ret != RK_SUCCESS) {
            return -1;
        }
    }

    return 0;
}

int vi_chn_init(int channelId, int width, int height) {
    int ret;
    int buf_cnt = 1;

    // VI init
    VI_CHN_ATTR_S vi_chn_attr;
    memset(&vi_chn_attr, 0, sizeof(vi_chn_attr));
    vi_chn_attr.stIspOpt.u32BufCount = buf_cnt;
    vi_chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;//VI_V4L2_MEMORY_TYPE_MMAP;
    vi_chn_attr.stSize.u32Width = width;
    vi_chn_attr.stSize.u32Height = height;
    vi_chn_attr.enPixelFormat = RK_FMT_YUV420SP;

    if (channelId == 2)                                // FBCPATH
        vi_chn_attr.enCompressMode = COMPRESS_MODE_NONE; // COMPRESS_AFBC_16x16;
    vi_chn_attr.u32Depth = 2;

    if (g_sEntityName != NULL)
        memcpy(vi_chn_attr.stIspOpt.aEntityName, g_sEntityName, strlen(g_sEntityName));
    ret = RK_MPI_VI_SetChnAttr(0, channelId, &vi_chn_attr);
    ret |= RK_MPI_VI_EnableChn(0, channelId);
    if (ret) {
        printf("ERROR: create VI error! ret=%d\n", ret);
        return ret;
    }

    return ret;
}

int main(int argc, char *argv[])
{
    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 u32Width = 1920;
    RK_U32 u32Height = 1080;
    RK_CHAR *pOutPath = NULL;
    RK_CODEC_ID_E enCodecType = RK_VIDEO_ID_HEVC;
    RK_CHAR *pCodecName = "H264";
    RK_S32 s32chnlId = 0;

    g_s32FrameCnt = 20;
    g_sEntityName = "/dev/video11";

#ifdef RKAIQ
        rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
        SAMPLE_COMM_ISP_Init(s32chnlId, hdr_mode, false, "/etc/iqfiles");
        SAMPLE_COMM_ISP_Run(s32chnlId);
#endif

#ifdef ENABLE_GET_STREAM
    pOutPath = "/tmp/venc-test.bin";
    int c;

    if (pOutPath) {
        venc0_file = fopen(pOutPath, "w");
        if (!venc0_file) {
            return 0;
        }
    }
#endif

    signal(SIGINT, sigterm_handler);

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        goto __FAILED;
    }

    vi_dev_init();
    vi_chn_init(s32chnlId, u32Width, u32Height);

    test_venc_init(0, u32Width, u32Height, enCodecType);//RK_VIDEO_ID_AVC RK_VIDEO_ID_HEVC

    // venc init, if is fast boot, must first init venc.


    MPP_CHN_S stSrcChn, stDestChn;
    // bind vi to venc
    stSrcChn.enModId    = RK_ID_VI;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;

    stDestChn.enModId   = RK_ID_VENC;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = 0;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

#ifdef ENABLE_GET_STREAM
    GetMediaBuffer0(NULL);

    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    s32Ret = RK_MPI_VI_DisableChn(0, 0);
    s32Ret = RK_MPI_VENC_StopRecvFrame(0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VENC_DestroyChn(0);
    s32Ret = RK_MPI_VI_DisableDev(0);
#endif
    if (fork() > 0) {
        goto __FAILED;
    }
    else {
        while (1) {
            usleep(1000 * 1000); //when client online
        }
    }
__FAILED:
    //RK_MPI_SYS_Exit();
    return 0;
}
