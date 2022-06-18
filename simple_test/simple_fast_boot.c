
#include <stdio.h>
#include <sys/poll.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "sample_comm.h"

static FILE *venc0_file;
static RK_S32 g_s32FrameCnt = -1;
static VI_CHN_BUF_WRAP_S g_stViWrap;
static bool g_bWrap = false;
static RK_U32 g_u32WrapLine = 0;
static char *g_sEntityName = NULL;
static bool quit = false;

static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    quit = true;
}

RK_U64 TEST_COMM_GetNowUs() {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (RK_U64)time.tv_sec * 1000000 + (RK_U64)time.tv_nsec / 1000; /* microseconds */
}

static void *GetMediaBuffer0(void *arg) {
    (void)arg;
    void *pData = RK_NULL;
    int loopCount = 0;
    int s32Ret;
    VENC_STREAM_S stFrame;
    FILE *fp = fopen("/tmp/pts.txt", "wb");

    printf("========%s========\n", __func__);
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
                snprintf(str, sizeof(str), "pts:%llums\n", stFrame.pstPack->u64PTS / 1000);
                fputs(str, fp);
                fsync(fileno(fp));
            }

            s32Ret = RK_MPI_VENC_ReleaseStream(0, &stFrame);
            if (s32Ret != RK_SUCCESS) {
                printf("RK_MPI_VENC_ReleaseStream fail %x\n", s32Ret);
            }
            loopCount++;
        } else {
            printf("RK_MPI_VI_GetChnFrame fail %x\n", s32Ret);
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

static RK_S32 test_venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType)
{
    //printf("========%s========\n", __func__);
    VENC_RECV_PIC_PARAM_S stRecvParam;
    VENC_CHN_BUF_WRAP_S stVencChnBufWrap;
    VENC_CHN_ATTR_S stAttr;

    memset(&stAttr,0,sizeof(VENC_CHN_ATTR_S));
    stVencChnBufWrap.bEnable = g_bWrap;
    stVencChnBufWrap.u32BufLine = g_u32WrapLine;

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
    // stRecvParam.s32RecvPicNum = 100;         //recv 100 slice
    // RK_MPI_VENC_StartRecvFrame(chnId, &stRecvParam);

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
            printf("RK_MPI_VI_SetDevAttr %x\n", ret);
            return -1;
        }
    } else {
        printf("RK_MPI_VI_SetDevAttr already\n");
    }
    // 1.get dev enable status
    ret = RK_MPI_VI_GetDevIsEnable(devId);
    if (ret != RK_SUCCESS) {
        // 1-2.enable dev
        ret = RK_MPI_VI_EnableDev(devId);
        if (ret != RK_SUCCESS) {
            printf("RK_MPI_VI_EnableDev %x\n", ret);
            return -1;
        }
        // 1-3.bind dev/pipe
        stBindPipe.u32Num = pipeId;
        stBindPipe.PipeId[0] = pipeId;
        ret = RK_MPI_VI_SetDevBindPipe(devId, &stBindPipe);
        if (ret != RK_SUCCESS) {
            printf("RK_MPI_VI_SetDevBindPipe %x\n", ret);
            return -1;
        }
    } else {
        printf("RK_MPI_VI_EnableDev already\n");
    }

    return 0;
}

int vi_chn_init(int channelId, int width, int height) {
    int ret;
    int buf_cnt = 2;
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

    // set wrap mode attr
    memset(&g_stViWrap, 0, sizeof(VI_CHN_BUF_WRAP_S));
    if (g_bWrap) {
        if (g_u32WrapLine < 128 || g_u32WrapLine > height) {
            RK_LOGE("wrap mode buffer line must between [128, H]");
            return -1;
        }
        g_stViWrap.bEnable           = RK_TRUE;
        g_stViWrap.u32BufLine        = g_u32WrapLine;
        g_stViWrap.u32WrapBufferSize = g_stViWrap.u32BufLine * width * 3 / 2;  // nv12 (w * wrapLine *3 / 2)
        //printf("set channel wrap line: %d, wrapBuffSize = %d\n", g_u32WrapLine, g_stViWrap.u32WrapBufferSize);
        RK_MPI_VI_SetChnWrapBufAttr(0, channelId, &g_stViWrap);
    }

    ret |= RK_MPI_VI_EnableChn(0, channelId);
    if (ret) {
        printf("ERROR: create VI error! ret=%d\n", ret);
        return ret;
    }

    return ret;
}

static RK_CHAR optstr[] = "?::w:h:c:I:e:o:n:r:l:a:M:";
static void print_usage(const RK_CHAR *name) {
    printf("usage example:\n");
    printf("\t%s -I 0 -w 1920 -h 1080 -o /tmp/venc.h264\n", name);
#ifdef RKAIQ
    printf("\t-a | --aiq: enable aiq with dirpath provided, eg:-a "
                   "/etc/iqfiles/, "
                   "set dirpath empty to using path by default, without this option aiq "
                   "should run in other application\n");
    printf("\t-M | --multictx: switch of multictx in isp, set 0 to disable, set "
                   "1 to enable. Default: 0\n");
#endif
    printf("\t-w | --width: VI width, Default:1920\n");
    printf("\t-h | --height: VI height, Default:1080\n");
    printf("\t-c | --frame_cnt: frame number of output, Default:150\n");
    printf("\t-n | --name:  set the entityName (required, default null\n");
    printf("\t-r | --wrap: enable wrap mode\n");
    printf("\t-l | --wrap_line: config wrap line\n");
    printf("\t-I | --camid: camera ctx id, Default 0. 0:rkisp_mainpath,1:rkisp_selfpath,2:rkisp_bypasspath\n");
    printf("\t-e | --encode: encode type, Default:h264, Value:h264, h265, mjpeg\n");
    printf("\t-o | --output_path: encode save file path, Default:NULL\n");
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
    int c;

    int ssFd = -1, viDev = -1;
    const char *ssName = "/dev/video8";
    ssFd = open(ssName, O_RDWR | O_CLOEXEC);
    if (ssFd < 0) {
        printf("failed to open camera:%s\n", ssName);
        return -1;
    }
    //printf("simple_fast_boot %s start\n", __func__);

    viDev = open("/dev/mpi/vvi", O_RDONLY);
    if (viDev < 0) {
        printf("failed to open /dev/mpi/vvi\n");
        return -1;
    }
#ifdef RKAIQ
    RK_BOOL bMultictx = RK_FALSE;
#endif
    char *iq_file_dir = NULL;

    while ((c = getopt(argc, argv, optstr)) != -1) {
        const char *tmp_optarg = optarg;
        switch (c) {
            case 'a':
                if (!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
                    tmp_optarg = argv[optind++];
                }
                if (tmp_optarg) {
                    iq_file_dir = (char *)tmp_optarg;
                } else {
                    iq_file_dir = NULL;
                }
            break;
            case 'w':
                u32Width = atoi(optarg);
            break;
            case 'h':
                u32Height = atoi(optarg);
            break;
            case 'I':
                s32chnlId = atoi(optarg);
            break;
            case 'c':
                g_s32FrameCnt = atoi(optarg);
                break;
            case 'e':
                if (!strcmp(optarg, "h264")) {
                    enCodecType = RK_VIDEO_ID_AVC;
                    pCodecName = "H264";
                } else if (!strcmp(optarg, "h265")) {
                    enCodecType = RK_VIDEO_ID_HEVC;
                    pCodecName = "H265";
                } else if (!strcmp(optarg, "mjpeg")) {
                    enCodecType = RK_VIDEO_ID_MJPEG;
                    pCodecName = "MJPEG";
                } else {
                    printf("ERROR: Invalid encoder type.\n");
                    return 0;
                }
            break;
            case 'n':
                g_sEntityName = optarg;
                break;
            case 'r':
                g_bWrap = atoi(optarg);
            break;
            case 'l':
                g_u32WrapLine = atoi(optarg);
            break;
            case 'o':
                pOutPath = optarg;
            break;
#ifdef RKAIQ
            case 'M':
                if (atoi(optarg)) {
                    bMultictx = RK_TRUE;
                }
            break;
#endif
            case '?':
                default:
                print_usage(argv[0]);
            return 0;
        }
    }

    printf("#CodecName:%s\n", pCodecName);
    printf("#Resolution: %dx%d\n", u32Width, u32Height);
    printf("#Output Path: %s\n", pOutPath);
    printf("#CameraIdx: %d\n", s32chnlId);
    printf("#Frame Count to save: %d\n", g_s32FrameCnt);
    printf("#Wrap mode: %d\n", g_bWrap);
    printf("#Wrap line: %d\n", g_u32WrapLine);
    printf("#IQ Path: %s\n", iq_file_dir);
    printf("\n");

    if (pOutPath) {
        venc0_file = fopen(pOutPath, "w");
        if (!venc0_file) {
            printf("ERROR: open file: %s fail, exit\n", pOutPath);
            return 0;
        }
    }
    signal(SIGINT, sigterm_handler);

    RK_S64 s64VencInitStart = TEST_COMM_GetNowUs();
    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        printf("rk mpi sys init fail!\n");
        goto __FAILED;
    }
    // venc init, if is fast boot, must first init venc.
    test_venc_init(0, u32Width, u32Height, enCodecType);//RK_VIDEO_ID_AVC RK_VIDEO_ID_HEVC
    RK_S64 s64VencInitEnd = TEST_COMM_GetNowUs();
    printf("sys/venc:%lld us\n", s64VencInitEnd - s64VencInitStart);

    if (iq_file_dir) {
#ifdef RKAIQ
        //printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
        RK_S64 s64AiqInitStart = TEST_COMM_GetNowUs();
        rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;

        SAMPLE_COMM_ISP_Init(s32chnlId, hdr_mode, bMultictx, iq_file_dir);
        SAMPLE_COMM_ISP_Run(s32chnlId);
        RK_S64 s64AiqInitEnd = TEST_COMM_GetNowUs();
        printf("Aiq:%lld us\n", s64AiqInitEnd - s64AiqInitStart);
#endif
    }

    RK_S64 s64ViInitStart = TEST_COMM_GetNowUs();
    vi_dev_init();
    vi_chn_init(s32chnlId, u32Width, u32Height);

    RK_S64 s64ViInitEnd = TEST_COMM_GetNowUs();
    printf("vi:%lld us\n", s64ViInitEnd - s64ViInitStart);

    MPP_CHN_S stSrcChn, stDestChn;
    // bind vi to venc
    stSrcChn.enModId    = RK_ID_VI;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;

    stDestChn.enModId   = RK_ID_VENC;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = 0;
    //printf("====RK_MPI_SYS_Bind vi0 to venc0====\n");
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        printf("bind 0 ch venc failed\n");
        goto __FAILED;
    }

    pthread_t main_thread;
    pthread_create(&main_thread, NULL, GetMediaBuffer0, NULL);

    while (!quit) {
        usleep(50000);
    }

    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        printf("RK_MPI_SYS_UnBind fail %x\n", s32Ret);
    }

    s32Ret = RK_MPI_VI_DisableChn(0, 0);
    printf("RK_MPI_VI_DisableChn %x\n", s32Ret);

    s32Ret = RK_MPI_VENC_StopRecvFrame(0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VENC_DestroyChn(0);
    if (s32Ret != RK_SUCCESS) {
        printf("RK_MPI_VDEC_DestroyChn fail %x\n", s32Ret);
    }

    s32Ret = RK_MPI_VI_DisableDev(0);
    printf("RK_MPI_VI_DisableDev %x\n", s32Ret);

__FAILED:
    printf("test running exit:%d\n", s32Ret);
    RK_MPI_SYS_Exit();

    if (iq_file_dir) {
#ifdef RKAIQ
        SAMPLE_COMM_ISP_Stop(s32chnlId);
#endif
    }
    return 0;
}
