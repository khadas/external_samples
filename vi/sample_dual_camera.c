
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <time.h>
#include <unistd.h>

#include "rtsp_demo.h"
#include "sample_comm.h"

rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session_0, g_rtsp_session_1;

typedef struct _rkMpiCtx {
	SAMPLE_VI_CTX_S vi[2];
	SAMPLE_VENC_CTX_S venc[2];
} SAMPLE_MPI_CTX_S;

static bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	quit = true;
}

static RK_CHAR optstr[] = "?::w:h:s:";
static const struct option long_options[] = {
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"sensorid", required_argument, NULL, 's'},
    {"help", optional_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

/******************************************************************************
 * function : venc thread
 ******************************************************************************/
static void *venc0_get_stream(void *pArgs) {
	printf("#Start %s , arg:%p\n", __func__, pArgs);
	SAMPLE_VENC_CTX_S *ctx = (SAMPLE_VENC_CTX_S *)(pArgs);
	RK_S32 s32Ret = RK_FAILURE;
	char name[256] = {0};
	FILE *fp = RK_NULL;
	void *pData = RK_NULL;
	RK_S32 loopCount = 0;

	while (!quit) {
		s32Ret = SAMPLE_COMM_VENC_GetStream(ctx, &pData);
		if (s32Ret == RK_SUCCESS) {
			// exit when complete
			if (ctx->s32loopCount > 0) {
				if (loopCount >= ctx->s32loopCount) {
					SAMPLE_COMM_VENC_ReleaseStream(ctx);
					quit = true;
					break;
				}
			}

			PrintStreamDetails(ctx->s32ChnId, ctx->stFrame.pstPack->u32Len);
			rtsp_tx_video(g_rtsp_session_0, pData, ctx->stFrame.pstPack->u32Len,
			              ctx->stFrame.pstPack->u64PTS);
			rtsp_do_event(g_rtsplive);

			RK_LOGD("chn:%d, loopCount:%d wd:%d\n", ctx->s32ChnId, loopCount,
			        ctx->stFrame.pstPack->u32Len);

			SAMPLE_COMM_VENC_ReleaseStream(ctx);
			loopCount++;
		}
		usleep(1000);
	}

	return RK_NULL;
}

static void *venc1_get_stream(void *pArgs) {
	printf("#Start %s , arg:%p\n", __func__, pArgs);
	SAMPLE_VENC_CTX_S *ctx = (SAMPLE_VENC_CTX_S *)(pArgs);
	RK_S32 s32Ret = RK_FAILURE;
	char name[256] = {0};
	FILE *fp = RK_NULL;
	void *pData = RK_NULL;
	RK_S32 loopCount = 0;

	while (!quit) {
		s32Ret = SAMPLE_COMM_VENC_GetStream(ctx, &pData);
		if (s32Ret == RK_SUCCESS) {
			// exit when complete
			if (ctx->s32loopCount > 0) {
				if (loopCount >= ctx->s32loopCount) {
					SAMPLE_COMM_VENC_ReleaseStream(ctx);
					quit = true;
					break;
				}
			}

			PrintStreamDetails(ctx->s32ChnId, ctx->stFrame.pstPack->u32Len);
			rtsp_tx_video(g_rtsp_session_1, pData, ctx->stFrame.pstPack->u32Len,
			              ctx->stFrame.pstPack->u64PTS);
			rtsp_do_event(g_rtsplive);

			RK_LOGD("chn:%d, loopCount:%d wd:%d\n", ctx->s32ChnId, loopCount,
			        ctx->stFrame.pstPack->u32Len);

			SAMPLE_COMM_VENC_ReleaseStream(ctx);
			loopCount++;
		}
		usleep(1000);
	}

	return RK_NULL;
}

static void print_usage(const RK_CHAR *name) {
	printf("usage example:\n");
	printf("\t%s -s 0 -w 1920 -h 1080 -s 1 -w 1920 -h 1080 \n", name);
	printf("\trtsp://xx.xx.xx.xx/live/0, sensor 0 mainpath,Default OPEN\n");
	printf("\trtsp://xx.xx.xx.xx/live/1, sensor 1 mainpath,Default OPEN\n");
	printf("\t-s | --sensor id\n");
	printf("\t-w | --width: camera with, Default 1920,max 1920\n");
	printf("\t-h | --height: camera height, Default 1080,max 1080\n");
}
/******************************************************************************
 * function    : main()
 * Description : main
 ******************************************************************************/
int main(int argc, char *argv[]) {
	RK_S32 s32Ret = RK_FAILURE;
	SAMPLE_MPI_CTX_S *ctx;
	int video_width_0 = 1920;
	int video_height_0 = 1080;
	int video_width_1 = 1920;
	int video_height_1 = 1080;
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H265;
	VENC_RC_MODE_E enRcMode = VENC_RC_MODE_H265CBR;
	RK_CHAR *pCodecName = "H265";
	RK_S32 s32CamId = -1;
	MPP_CHN_S stSrcChn[2], stDestChn[2];
	RK_CHAR *pOutPathVenc = "/userdata";
	RK_S32 s32CamNum = 2;
	RK_S32 s32loopCnt = -1;
	RK_S32 s32BitRate = 4 * 1024;
	RK_S32 i;
	char *iq_file_dir = "/oem/usr/share/iqfiles";

	if (argc < 2) {
		print_usage(argv[0]);
		return 0;
	}

	ctx = (SAMPLE_MPI_CTX_S *)(malloc(sizeof(SAMPLE_MPI_CTX_S)));
	memset(ctx, 0, sizeof(SAMPLE_MPI_CTX_S));

	signal(SIGINT, sigterm_handler);

#ifdef RKAIQ
	RK_BOOL bMultictx = RK_TRUE;
#endif
	int c;
	while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
		const char *tmp_optarg = optarg;
		switch (c) {
		case 's':
			s32CamId = atoi(optarg);
			break;
		case 'w':
			if (s32CamId == 0)
				video_width_0 = atoi(optarg);
			if (s32CamId == 1)
				video_width_1 = atoi(optarg);
			break;
		case 'h':
			if (s32CamId == 0)
				video_height_0 = atoi(optarg);
			if (s32CamId == 1)
				video_height_1 = atoi(optarg);
			break;
		case '?':
		default:
			print_usage(argv[0]);
			return 0;
		}
	}
	printf("sensor 0:%d*%d\n", video_width_0, video_height_0);
	printf("sensor 1:%d*%d\n", video_width_1, video_height_1);
	if (video_width_0 <= 0 || video_width_1 <= 0 || video_height_0 <= 0 ||
	    video_height_1 <= 0) {
		printf("invalid width/height,please check!\n");
		return -1;
	}
	printf("#CodecName:%s\n", pCodecName);
	printf("#IQ Path: %s\n", iq_file_dir);
	if (iq_file_dir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
		printf("#bMultictx: %d\n\n", bMultictx);
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
		for (int i = 0; i < s32CamNum; i++) {
			SAMPLE_COMM_ISP_Init(i, hdr_mode, bMultictx, iq_file_dir);
			SAMPLE_COMM_ISP_Run(i);
		}
#endif
	}

	// init rtsp
	g_rtsplive = create_rtsp_demo(554);
	g_rtsp_session_0 = rtsp_new_session(g_rtsplive, "/live/0");
	g_rtsp_session_1 = rtsp_new_session(g_rtsplive, "/live/1");
	rtsp_set_video(g_rtsp_session_0, RTSP_CODEC_ID_VIDEO_H265, NULL, 0);
	rtsp_set_video(g_rtsp_session_1, RTSP_CODEC_ID_VIDEO_H265, NULL, 0);
	rtsp_sync_video_ts(g_rtsp_session_0, rtsp_get_reltime(), rtsp_get_ntptime());
	rtsp_sync_video_ts(g_rtsp_session_1, rtsp_get_reltime(), rtsp_get_ntptime());

	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		goto __FAILED;
	}

	// Init VI[0] ~ VI[1]
	for (i = 0; i < s32CamNum; i++) {
		if (i == 0) {
			ctx->vi[i].u32Width = video_width_0;
			ctx->vi[i].u32Height = video_height_0;
		}
		if (i == 1) {
			ctx->vi[i].u32Width = video_width_1;
			ctx->vi[i].u32Height = video_height_1;
		}
		ctx->vi[i].s32DevId = i;
		ctx->vi[i].u32PipeId = i;
		ctx->vi[i].s32ChnId = 0;
		ctx->vi[i].stChnAttr.stIspOpt.u32BufCount = 2;
		ctx->vi[i].stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
		ctx->vi[i].stChnAttr.u32Depth = 2;
		ctx->vi[i].stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
		ctx->vi[i].stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
		ctx->vi[i].stChnAttr.stFrameRate.s32SrcFrameRate = -1;
		ctx->vi[i].stChnAttr.stFrameRate.s32DstFrameRate = -1;
		SAMPLE_COMM_VI_CreateChn(&ctx->vi[i]);

		ctx->venc[i].s32ChnId = i;
		if (i == 0) {
			ctx->venc[i].u32Width = video_width_0;
			ctx->venc[i].u32Height = video_height_0;
		}
		if (i == 1) {
			ctx->venc[i].u32Width = video_width_1;
			ctx->venc[i].u32Height = video_height_1;
		}
		ctx->venc[i].u32Fps = 25;
		ctx->venc[i].u32Gop = 50;
		ctx->venc[i].u32BitRate = s32BitRate;
		ctx->venc[i].enCodecType = enCodecType;
		ctx->venc[i].enRcMode = enRcMode;
		if (i == 0)
			ctx->venc[i].getStreamCbFunc = venc0_get_stream;
		if (i == 1)
			ctx->venc[i].getStreamCbFunc = venc1_get_stream;
		ctx->venc[i].s32loopCount = s32loopCnt;
		ctx->venc[i].dstFilePath = pOutPathVenc;
		// H264  66：Baseline  77：Main Profile 100：High Profile
		// H265  0：Main Profile  1：Main 10 Profile
		// MJPEG 0：Baseline
		ctx->venc[i].stChnAttr.stVencAttr.u32Profile = 0;
		ctx->venc[i].stChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
		SAMPLE_COMM_VENC_CreateChn(&ctx->venc[i]);

		stSrcChn[i].enModId = RK_ID_VI;
		stSrcChn[i].s32DevId = i;
		stSrcChn[i].s32ChnId = ctx->vi[i].s32ChnId;
		stDestChn[i].enModId = RK_ID_VENC;
		stDestChn[i].s32DevId = 0;
		stDestChn[i].s32ChnId = ctx->venc[i].s32ChnId;
		SAMPLE_COMM_Bind(&stSrcChn[i], &stDestChn[i]);
	}

	printf("%s initial finish\n", __func__);

	while (!quit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);
	for (i = 0; i < s32CamNum; i++) {
		if (ctx->venc[i].getStreamCbFunc) {
			pthread_join(ctx->venc[i].getStreamThread, NULL);
		}
	}

	if (g_rtsplive)
		rtsp_del_demo(g_rtsplive);

	for (i = 0; i < s32CamNum; i++) {
		stSrcChn[i].enModId = RK_ID_VI;
		stSrcChn[i].s32DevId = ctx->vi[i].s32DevId;
		stSrcChn[i].s32ChnId = ctx->vi[i].s32ChnId;
		stDestChn[i].enModId = RK_ID_VENC;
		stDestChn[i].s32DevId = ctx->venc[i].s32ChnId;
		stDestChn[i].s32ChnId = i;
		SAMPLE_COMM_UnBind(&stSrcChn[i], &stDestChn[i]);
	}

	// Destroy VI[0]
	for (i = 0; i < s32CamNum; i++) {
		SAMPLE_COMM_VENC_DestroyChn(&ctx->venc[i]);
		SAMPLE_COMM_VI_DestroyChn(&ctx->vi[i]);
	}

__FAILED:
	RK_MPI_SYS_Exit();
	if (iq_file_dir) {
#ifdef RKAIQ
		for (int i = 0; i < s32CamNum; i++) {
			SAMPLE_COMM_ISP_Stop(i);
		}
#endif
	}
__FAILED2:
	if (ctx) {
		free(ctx);
		ctx = RK_NULL;
	}

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */