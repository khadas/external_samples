/* thunder boot service
/1.aiq init & run
/2.rockit vi & venc init, bind
*/
#include <stdio.h>
#include <sys/poll.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include "rk_type.h"
#include "rk_debug.h"
#include "rk_defines.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_sys.h"
#include <rk_aiq_user_api2_camgroup.h>
#include <rk_aiq_user_api2_imgproc.h>
#include <rk_aiq_user_api2_sysctl.h>

#include "rk_meta_app_param.h"

#define RKAIQ
//#define ENABLE_GET_STREAM

#define MAP_SIZE (4096UL * 50) //MAP_SIZE = 4 * 50 K
#define MAP_MASK (MAP_SIZE - 1) //MAP_MASK = 0XFFF

#define MAP_SIZE_NIGHT (4096UL) //MAP_SIZE = 4K
#define MAP_MASK_NIGHT (MAP_SIZE_NIGHT - 1) //MAP_MASK = 0XFFF

static FILE *venc0_file = NULL;
static RK_S32 g_s32FrameCnt = -1;
static VI_CHN_BUF_WRAP_S g_stViWrap;
static bool g_bWrap = false;
static RK_U32 g_u32WrapLine = 0;
static char *g_sEntityName = NULL;
static bool quit = false;
static struct app_param_info *g_pAppParam = NULL;

static void sigterm_handler(int sig) {
    quit = true;
}

long get_cmd_val(const char *string, int len) {
    char *addr;
    long value;

    addr = getenv(string);
    value = strtol(addr, NULL, len);
    printf("get %s value: 0x%0lx\n", string, value);
    return value;
}

RK_U64 TEST_COMM_GetNowUs() {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (RK_U64)time.tv_sec * 1000000 + (RK_U64)time.tv_nsec / 1000; /* microseconds */
}

#ifdef ENABLE_GET_STREAM

// fast client to get media buffer.
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
    VENC_CHN_REF_BUF_SHARE_S stVencChnRefBufShare;
    VENC_CHN_ATTR_S stAttr;
    RK_U32 fps = 0;
    RK_U32 gop = 0;

    fps = (RK_U32)get_cmd_val("rk_cam_fps_denominator", 10) / ((RK_U32)get_cmd_val("rk_cam_fps_numerator", 10));
    RK_ASSERT(fps > 0);
    gop = fps * 2;

    memset(&stAttr,0,sizeof(VENC_CHN_ATTR_S));
    stVencChnBufWrap.bEnable = g_bWrap;
    stVencChnBufWrap.u32BufLine = g_u32WrapLine;

    memset(&stVencChnRefBufShare, 0, sizeof(VENC_CHN_REF_BUF_SHARE_S));
    stVencChnRefBufShare.bEnable = true;

    memset(&stVencChnRefBufShare, 0, sizeof(VENC_CHN_REF_BUF_SHARE_S));
    stVencChnRefBufShare.bEnable = true;

    if (enType == RK_VIDEO_ID_AVC) {
        stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        stAttr.stRcAttr.stH264Cbr.u32BitRate = g_pAppParam->venc_bitrate;
        stAttr.stRcAttr.stH264Cbr.u32Gop = gop;
    } else if (enType == RK_VIDEO_ID_HEVC) {
        stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
        stAttr.stRcAttr.stH265Cbr.u32BitRate = g_pAppParam->venc_bitrate;
        stAttr.stRcAttr.stH265Cbr.u32Gop = gop;
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
    stAttr.stVencAttr.u32StreamBufCnt = 4;
    stAttr.stVencAttr.u32BufSize = width * height / 2;
    stAttr.stVencAttr.enMirror = MIRROR_NONE;

    RK_MPI_VENC_CreateChn(chnId, &stAttr);
    RK_MPI_VENC_SetChnBufWrapAttr(chnId, &stVencChnBufWrap);
    RK_MPI_VENC_SetChnRefBufShareAttr(chnId, &stVencChnRefBufShare);

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
    vi_chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
    vi_chn_attr.stSize.u32Width = width;
    vi_chn_attr.stSize.u32Height = height;
    vi_chn_attr.enPixelFormat = RK_FMT_YUV420SP;
    vi_chn_attr.enCompressMode = COMPRESS_MODE_NONE;
    vi_chn_attr.u32Depth = 2;

    if (g_sEntityName != NULL)
        memcpy(vi_chn_attr.stIspOpt.aEntityName, g_sEntityName, strlen(g_sEntityName));
    ret = RK_MPI_VI_SetChnAttr(0, channelId, &vi_chn_attr);

    g_stViWrap.bEnable           = RK_TRUE;
    g_stViWrap.u32BufLine        = g_u32WrapLine;
    g_stViWrap.u32WrapBufferSize = g_stViWrap.u32BufLine * width * 3 / 2;
    RK_MPI_VI_SetChnWrapBufAttr(0, channelId, &g_stViWrap);

    ret |= RK_MPI_VI_EnableChn(0, channelId);
    if (ret) {
        printf("ERROR: create VI error! ret=%d\n", ret);
        return ret;
    }

    return ret;
}

static int g_err_cnt = 0;
static bool g_should_quit = false;

static XCamReturn SAMPLE_COMM_ISP_ErrCb(rk_aiq_err_msg_t* msg) {
	g_err_cnt++;
    if (g_err_cnt <= 2)
        printf("=== %u ===\n", msg->err_code);
    if (msg->err_code == XCAM_RETURN_BYPASS)
        g_should_quit = true;
}

#if 1
void klog(const char *log) {
    FILE *fp = fopen("/dev/kmsg", "w");
    if (NULL != fp) {
        fprintf(fp, "[app]: %s\n", log);
        fclose(fp);
    }
}
#else
void klog(const char *log) {
	return;
}
#endif


int main(int argc, char *argv[])
{
    klog("main");

    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 u32Width = 0;
    RK_U32 u32Height = 0;
    RK_CHAR *pOutPath = NULL;
    RK_CHAR *pCodecName = "H264";

    RK_CODEC_ID_E enCodecType = RK_VIDEO_ID_Max;//RK_VIDEO_ID_HEVC;
    RK_S32 s32chnlId = 0;

    int mem_fd = -1;
    off_t appParamOffs = 0, metaAddr = 0;
    void *metaVirmem = NULL, *appVirAddr = NULL;
    RK_U32 metaSize = (RK_U32)get_cmd_val("meta_part_size", 16);
    metaAddr = (off_t)get_cmd_val("meta_load_addr", 16);
    if((mem_fd = open("/dev/mem", O_RDONLY)) < 0)
    {
        printf("cannot open /dev/mem.\n");
        return -1;
    }

    metaVirmem = mmap(NULL , metaSize, PROT_READ, MAP_SHARED, mem_fd, metaAddr);
    if (metaVirmem != MAP_FAILED) {
        RK_U32 app_param_offset = (RK_U32)get_cmd_val(RK_APP_PARAM_OFFSET, 16);
        appVirAddr = metaVirmem + app_param_offset;
        g_pAppParam = (struct app_param_info *)(appVirAddr);
        u32Width = g_pAppParam->venc_w;
        u32Height = g_pAppParam->venc_h;
        switch (g_pAppParam->venc_type)
        {
        case 1:
            enCodecType = RK_VIDEO_ID_AVC; // h.264
            break;
        case 2:
        default:
            enCodecType = RK_VIDEO_ID_HEVC; // h.265
            break;
        }
        printf("\n read from meta: w:%d, h:%d, bitrate:%d, venc type:%d\n",
                g_pAppParam->venc_w, g_pAppParam->venc_h,
                g_pAppParam->venc_bitrate, g_pAppParam->venc_type);
    } else {
        printf("mmap fail.\n");
        return -1;
    }

    g_bWrap = true;
    g_u32WrapLine = u32Height / 8; // 360   // 1080
    g_s32FrameCnt = 20;
    g_sEntityName = "/dev/video11";

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
    klog("sys_init");
    test_venc_init(0, u32Width, u32Height, enCodecType);//RK_VIDEO_ID_AVC RK_VIDEO_ID_HEVC
    klog("venc_init");

    vi_dev_init();
    klog("vi_dev");
    vi_chn_init(s32chnlId, u32Width, u32Height);
    klog("vi_chn");

    test_venc_init(s32chnlId, u32Width, u32Height, enCodecType);//RK_VIDEO_ID_AVC RK_VIDEO_ID_HEVC
    // venc init, if is fast boot, must first init venc.
    test_venc_init(0, u32Width, u32Height, enCodecType);//RK_VIDEO_ID_AVC RK_VIDEO_ID_HEVC

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
        printf("111 RK_MPI_SYS_Bind fail\n");
        goto __FAILED;
    }
    klog("bind");

#if 1
    // venc init, if is fast boot, must first init venc.
    if (fork() > 0) {

    }
    else {
        while(1)
            usleep(1000 * 1000); //when client online
        printf("sub service exit main\n");
        return 0;
    }

    RK_MPI_VI_ResumeChn(0, s32chnlId);
    klog("vi resume");
#endif

#ifdef RKAIQ
    int is_bw_night, file_size, fd, ret = 0;
    void *mem, *vir_addr, *iq_mem, *vir_iqaddr;
    off_t bw_night_addr, addr_iq;
    RK_S64 s64AiqInitStart = TEST_COMM_GetNowUs();
    bw_night_addr = (off_t)get_cmd_val("bw_night_addr", 16);
    if((fd = open ("/dev/mem", O_RDWR | O_SYNC)) < 0)
    {
        perror ("open error");
        return -1;
    }
    mem = mmap (0 , MAP_SIZE_NIGHT, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bw_night_addr & ~MAP_MASK_NIGHT);
    vir_addr = mem + (bw_night_addr & MAP_MASK_NIGHT);
    is_bw_night = *((unsigned long *) vir_addr);

    if (pOutPath) {
        venc0_file = fopen(pOutPath, "w");
        if (!venc0_file) {
            return 0;
        }
    }

    addr_iq = (off_t)get_cmd_val("rk_iqbin_addr", 16);
    file_size = (int)get_cmd_val("rk_iqbin_size", 16);
    iq_mem = mmap (0 , file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_iq & ~MAP_MASK);
    vir_iqaddr = iq_mem + (addr_iq & MAP_MASK);
    klog("mmap iq mem");

    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    rk_aiq_sys_ctx_t *aiq_ctx;
    rk_aiq_static_info_t aiq_static_info;
    rk_aiq_uapi2_sysctl_enumStaticMetas(s32chnlId, &aiq_static_info);
    klog("aiq enum");

    if (is_bw_night) {
        printf("=====night mode=====\n");
        ret = rk_aiq_uapi2_sysctl_preInit_scene(aiq_static_info.sensor_info.sensor_name, "normal", "night");
        if (ret < 0) {
            printf("%s: failed to set night scene\n", aiq_static_info.sensor_info.sensor_name);
            return -1;
        }
    } else {
        printf("=====day mode=======\n");
        ret = rk_aiq_uapi2_sysctl_preInit_scene(aiq_static_info.sensor_info.sensor_name, "normal", "day");
        if (ret < 0) {
            printf("%s: failed to set day scene\n", aiq_static_info.sensor_info.sensor_name);
            return -1;
        }
    }
    klog("preint scene");

    ret = rk_aiq_uapi2_sysctl_preInit_iq_addr(aiq_static_info.sensor_info.sensor_name, vir_iqaddr, (size_t *)file_size);
    if (ret < 0) {
        printf("%s: failed to load binary iqfiles\n", aiq_static_info.sensor_info.sensor_name);
    }
    klog("preinit iq addr");

    rk_aiq_tb_info_t tb_info;
	tb_info.magic = sizeof(rk_aiq_tb_info_t) - 2;
	tb_info.is_pre_aiq = true;
	rk_aiq_uapi2_sysctl_preInit_tb_info(aiq_static_info.sensor_info.sensor_name, &tb_info);
    klog("preinit tb info");

    ret = aiq_ctx = rk_aiq_uapi2_sysctl_init(aiq_static_info.sensor_info.sensor_name,
                                       "/etc/iqfiles/", SAMPLE_COMM_ISP_ErrCb, NULL);
    klog("sysctl init");
    if (ret < 0) {
        printf("%s: failed to init aiq\n", aiq_static_info.sensor_info.sensor_name);
    }

    if (rk_aiq_uapi2_sysctl_prepare(aiq_ctx, 0, 0, hdr_mode)) {
        printf("rkaiq engine prepare failed !\n");
        return -1;
    }
    klog("sysctl prepare");

    if (rk_aiq_uapi2_sysctl_start(aiq_ctx)) {
        printf("rk_aiq_uapi2_sysctl_start  failed\n");
        return -1;
    }
    klog("sysctl start");
    RK_S64 s64AiqInitEnd = TEST_COMM_GetNowUs();
    printf("Aiq:%lld us\n", s64AiqInitEnd - s64AiqInitStart);
#endif
    klog("aiq start");
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

#ifdef RKAIQ
    while (1) {
        if (g_should_quit) {
            klog("should quit");
            rk_aiq_uapi2_sysctl_stop(aiq_ctx, false);
            klog("aiq stop");
            rk_aiq_uapi2_sysctl_deinit(aiq_ctx);
            klog("aiq deinit");
            break;
        }
        usleep(1 * 1000); //when client online
    }
#endif

    munmap(metaVirmem, metaSize);
    if (mem_fd >= 0) close(mem_fd);
    if (fd >= 0) close(fd);
__FAILED:
    //fclose(fd);
    //munmap(mem, MAP_SIZE_NIGHT);
    //munmap(iq_mem, file_size);
    //RK_MPI_SYS_Exit();
    printf("main service exit main\n");
    return 0;
}
