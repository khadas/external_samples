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
#include <rk_mpi_mmz.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "rtsp_demo.h"
#include "sample_comm.h"

#ifdef ROCKIVA
#include "rockiva/rockiva_ba_api.h"
#endif

#define VENC_CHN_MAX 3
#define BUFFER_SIZE 255
#define RGN_NUM_MAX 4
#define GET_STREAM_TIMEOUT 2000
#define SEND_STREAM_TIMEOUT 2000
#define RGN_ATTACH_VPSS 0
#define RGN_ATTACH_VENC 1
typedef struct _rkThreadStatus {
	RK_BOOL bIfMainThreadQuit;
	RK_BOOL bIfVencThreadQuit[VENC_CHN_MAX];
	RK_BOOL bIfViThreadQuit;
	RK_BOOL bIvsDetectThreadQuit;
	RK_BOOL bIfVpssIvaTHreadQuit;
} ThreadStatus;

/* global param */

static ThreadStatus *gPThreadStatus = RK_NULL;
static RK_S32 g_exit_result = RK_SUCCESS;
static pthread_mutex_t g_rtsp_mutex = {0};
static RK_BOOL g_rtsp_ifenbale = RK_FALSE;
rtsp_demo_handle g_rtsplive = RK_NULL;
static rtsp_session_handle g_rtsp_session[VENC_CHN_MAX] = {RK_NULL};

static RK_S32 aiisp_callback(MB_BLK pAinrBlk, RK_VOID *pPrivateData) {
	if (pAinrBlk == RK_NULL) {
		return RK_FAILURE;
	}
	RK_VOID *pAinrParam = RK_MPI_MB_Handle2VirAddr(pAinrBlk);
	if (pAinrParam == RK_NULL) {
		return RK_FAILURE;
	}
	memset(pAinrParam, 0, sizeof(rk_ainr_param));
	SAMPLE_COMM_ISP_GetAINrParams(0, pAinrParam);
	return RK_SUCCESS;
}
typedef struct _rkMpiCtx {
	SAMPLE_VI_CTX_S vi;
	SAMPLE_VENC_CTX_S venc[VENC_CHN_MAX];
	SAMPLE_RGN_CTX_S rgn[RGN_NUM_MAX];
	SAMPLE_VPSS_CTX_S vpss;
#ifdef ROCKIT_IVS
	SAMPLE_IVS_CTX_S ivs;
#endif
#ifdef ROCKIVA
	SAMPLE_IVA_CTX_S iva;
#endif
} SAMPLE_MPI_CTX_S;

static void program_handle_error(const char *func, RK_U32 line) {
	RK_LOGE("func: <%s> line: <%d> error exit!", func, line);
	g_exit_result = RK_FAILURE;
	gPThreadStatus->bIfMainThreadQuit = RK_TRUE;
}

static void program_normal_exit(const char *func, RK_U32 line) {
	RK_LOGE("func: <%s> line: <%d> normal exit!", func, line);
	gPThreadStatus->bIfMainThreadQuit = RK_TRUE;
}

static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	program_normal_exit(__func__, __LINE__);
}

static void *venc_get_stream(void *pArgs) {
	SAMPLE_VENC_CTX_S *ctx = (SAMPLE_VENC_CTX_S *)pArgs;
	RK_S32 s32Ret = RK_FAILURE;
	FILE *fp = RK_NULL;
	RK_S32 s32fd = 0;
	RK_S32 loopCount = 0;
	RK_VOID *pData = RK_NULL;
	RK_CHAR name[BUFFER_SIZE] = {0};
	sprintf(name, "venc_%d_get_stream", ctx->s32ChnId);
	prctl(PR_SET_NAME, name);

	if (ctx->dstFilePath) {
		memset(name, 0, BUFFER_SIZE);
		snprintf(name, sizeof(name), "/%s/venc_%d.bin", ctx->dstFilePath, ctx->s32ChnId);
		fp = fopen(name, "wb");
		if (fp == RK_NULL) {
			RK_LOGE("chn %d can't open %s file !\n", ctx->s32ChnId, ctx->dstFilePath);
			program_handle_error(__func__, __LINE__);
			return RK_NULL;
		}
		s32fd = fileno(fp);
	}
	while (!gPThreadStatus->bIfVencThreadQuit[ctx->s32ChnId]) {
		s32Ret = SAMPLE_COMM_VENC_GetStream(ctx, &pData);
		if (s32Ret == RK_SUCCESS) {
			if (ctx->s32loopCount > 0) {
				if (loopCount >= ctx->s32loopCount) {
					SAMPLE_COMM_VENC_ReleaseStream(ctx);
					program_normal_exit(__func__, __LINE__);
					break;
				}
			}

			if (fp && !gPThreadStatus->bIfMainThreadQuit) {
				fwrite(pData, 1, ctx->stFrame.pstPack->u32Len, fp);
				fflush(fp);
			}

			// PrintStreamDetails(ctx->s32ChnId, ctx->stFrame.pstPack->u32Len);
			if (g_rtsp_ifenbale) {
				pthread_mutex_lock(&g_rtsp_mutex);
				rtsp_tx_video(g_rtsp_session[ctx->s32ChnId], pData,
				              ctx->stFrame.pstPack->u32Len, ctx->stFrame.pstPack->u64PTS);
				rtsp_do_event(g_rtsplive);
				pthread_mutex_unlock(&g_rtsp_mutex);
			} else {
				RK_LOGD("venc %d get_stream count: %d", ctx->s32ChnId, loopCount);
			}

			RK_LOGI("venc %d get_stream count: %d", ctx->s32ChnId, loopCount);

			SAMPLE_COMM_VENC_ReleaseStream(ctx);
			loopCount++;
		}
	}

	if (fp) {
		fsync(s32fd);
		fclose(fp);
		fp = RK_NULL;
	}

	RK_LOGE("venc_get_stream chnid:%d exit", ctx->s32ChnId);
	return RK_NULL;
}

static RK_S32 rtsp_init(CODEC_TYPE_E enCodecType) {
	RK_S32 i = 0;
	g_rtsplive = create_rtsp_demo(554);
	RK_CHAR rtspAddr[BUFFER_SIZE] = {0};

	for (i = 0; i < VENC_CHN_MAX; i++) {
		sprintf(rtspAddr, "/live/%d", i);
		g_rtsp_session[i] = rtsp_new_session(g_rtsplive, rtspAddr);
		if (enCodecType == RK_CODEC_TYPE_H264) {
			rtsp_set_video(g_rtsp_session[i], RTSP_CODEC_ID_VIDEO_H264, RK_NULL, 0);
		} else if (enCodecType == RK_CODEC_TYPE_H265) {
			rtsp_set_video(g_rtsp_session[i], RTSP_CODEC_ID_VIDEO_H265, RK_NULL, 0);
		} else {
			RK_LOGE("not support other type\n");
			g_rtsp_ifenbale = RK_FALSE;
			return RK_SUCCESS;
		}
		rtsp_sync_video_ts(g_rtsp_session[i], rtsp_get_reltime(), rtsp_get_ntptime());
		RK_LOGE("rtsp <%s> init success", rtspAddr);
	}
	g_rtsp_ifenbale = RK_TRUE;
	return RK_SUCCESS;
}

static RK_S32 rtsp_deinit(void) {
	if (g_rtsplive)
		rtsp_del_demo(g_rtsplive);
	return RK_SUCCESS;
}

static RK_S32 global_param_init(void) {

	gPThreadStatus = (ThreadStatus *)malloc(sizeof(ThreadStatus));
	if (!gPThreadStatus) {
		printf("malloc for gPThreadStatus failure\n");
		goto __global_init_fail;
	}
	memset(gPThreadStatus, 0, sizeof(ThreadStatus));

	if (RK_SUCCESS != pthread_mutex_init(&g_rtsp_mutex, RK_NULL)) {
		RK_LOGE("pthread_mutex_init failure");
		goto __global_init_fail;
	}

	return RK_SUCCESS;

__global_init_fail:
	if (gPThreadStatus) {
		free(gPThreadStatus);
		gPThreadStatus = RK_NULL;
	}
	return RK_FAILURE;
}

static RK_S32 global_param_deinit(void) {

	if (gPThreadStatus) {
		free(gPThreadStatus);
		gPThreadStatus = RK_NULL;
	}

	pthread_mutex_destroy(&g_rtsp_mutex);

	return RK_SUCCESS;
}

static RK_CHAR optstr[] = "?::a::w:h:o:l:b:f:r:v:e:i:s:I:";
static const struct option long_options[] = {
    {"aiq", optional_argument, RK_NULL, 'a'},
    {"width", required_argument, RK_NULL, 'w'},
    {"height", required_argument, RK_NULL, 'h'},
    {"encode", required_argument, RK_NULL, 'e'},
    {"output_path", required_argument, RK_NULL, 'o'},
    {"loop_count", required_argument, RK_NULL, 'l'},
    {"bitrate", required_argument, NULL, 'b'},
    {"fps", required_argument, RK_NULL, 'f'},
    {"vi_buff_cnt", required_argument, RK_NULL, 'v'},
    {"vi_chnid", required_argument, RK_NULL, 'v' + 'i'},
    {"rgn_attach_module", required_argument, RK_NULL, 'r'},
    {"iva_detect_speed", required_argument, RK_NULL, 'd'},
    {"iva_model_path", required_argument, RK_NULL, 'i' + 'm'},
    {"enable_aiisp", required_argument, RK_NULL, 'e' + 'a'},
    {"enable_iva", required_argument, RK_NULL, 'e' + 'i'},
    {"enable_ivs", required_argument, RK_NULL, 'e' + 's'},
    {"inputBmpPath1", required_argument, RK_NULL, 'i'},
    {"inputBmpPath2", required_argument, RK_NULL, 'I'},
    {"help", optional_argument, RK_NULL, '?'},
    {RK_NULL, 0, RK_NULL, 0},
};

/******************************************************************************
 * function : show usage
 ******************************************************************************/
static void print_usage(const RK_CHAR *name) {
	printf("usage example:\n");
	printf("\t%s -w 2688 -h 1520 -a /etc/iqfiles/ -e h264cbr -b 4096 --enable_aiisp 1\n",
	       name);
#ifdef RKAIQ
	printf("\t-a | --aiq : enable aiq with dirpath provided, eg:-a /etc/iqfiles/, \n"
	       "\t		set dirpath empty to using path by default, without "
	       "this option aiq \n"
	       "\t		should run in other application\n");
#endif
	printf("\t-w | --width : mainStream width, must is sensor width\n");
	printf("\t-h | --height : mainStream height, must is sensor height\n");
	printf("\t-e | --encode: encode type, Default:h264cbr, Value:h264cbr, "
	       "h264vbr, h264avbr "
	       "h265cbr, h265vbr, h265avbr, mjpegcbr, mjpegvbr\n");
	printf("\t-b | --bitrate: encode bitrate, Default 4096\n");
	printf("\t-o | --output_path : encode output file path, Default: RK_NULL\n");
	printf("\t-l | --loop_count : when encoder output frameCounts equal to "
	       "<loop_count>, "
	       "process will exit. Default: -1\n");
	printf("\t-v | --vi_buff_cnt : main stream vi buffer num, Default: 6\n");
	printf("\t--vi_chnid : vi channel id, default: 0\n");
	printf("\t-r | --rgn_attach_module : where to attach rgn, 0: vpss, 1: venc, 2: "
	       "close. default: 1\n");
	printf("\t-i | --inputpathbmp1 : input bmp file path. default: RK_NULL\n");
	printf("\t-I | --inputpathbmp2 : input bmp file path. default: RK_NULL\n");
	printf("\t-f | --fps : set fps, default: 25\n");
	printf("\t--enable_aiisp : enable ai isp, 0: close, 1: enable. default: 1\n");
	printf("\t--enable_iva : enable iva, 0: close, 1: enable. default: 1\n");
	printf("\t--enable_ivs : enable ivs, 0: close, 1: enable. default: 0\n");
	printf("\t--iva_detect_speed : iva detect framerate. default: 10\n");
	printf("\t--iva_model_path : iva model data path, default: /oem/usr/lib\n");
}

#ifdef ROCKIVA
static void rkIvaEvent_callback(const RockIvaBaResult *result,
                                const RockIvaExecuteStatus status, void *userData) {

	if (result->objNum == 0)
		return;
	for (int i = 0; i < result->objNum; i++) {
		RK_LOGD("topLeft:[%d,%d], bottomRight:[%d,%d],"
		        "objId is %d, frameId is %d, score is %d, type is %d\n",
		        result->triggerObjects[i].objInfo.rect.topLeft.x,
		        result->triggerObjects[i].objInfo.rect.topLeft.y,
		        result->triggerObjects[i].objInfo.rect.bottomRight.x,
		        result->triggerObjects[i].objInfo.rect.bottomRight.y,
		        result->triggerObjects[i].objInfo.objId,
		        result->triggerObjects[i].objInfo.frameId,
		        result->triggerObjects[i].objInfo.score,
		        result->triggerObjects[i].objInfo.type);
	}
}

static void rkIvaFrame_releaseCallBack(const RockIvaReleaseFrames *releaseFrames,
                                       void *userdata) {
	/* when iva handle out of the video frame，this func will be called*/
	RK_S32 s32Ret = RK_SUCCESS;
	for (RK_S32 i = 0; i < releaseFrames->count; i++) {
		if (!releaseFrames->frames[i].extData) {
			RK_LOGE("---------error release frame is null");
			program_handle_error(__func__, __LINE__);
			continue;
		}
		s32Ret = RK_MPI_VPSS_ReleaseChnFrame(0, 3, releaseFrames->frames[i].extData);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("RK_MPI_VI_ReleaseChnFrame failure:%#X", s32Ret);
			program_handle_error(__func__, __LINE__);
		}
		free(releaseFrames->frames[i].extData);
	}
}

static void *vpss_iva_thread(void *pArgs) {
	prctl(PR_SET_NAME, "vpss_iva_thread");
	SAMPLE_MPI_CTX_S *ctx = (SAMPLE_MPI_CTX_S *)pArgs;
	RK_S32 s32Ret = RK_FAILURE;
	RK_CHAR *pData = RK_NULL;
	RK_S32 s32Fd = 0;
	RockIvaImage ivaImage;
	RK_U32 u32Loopcount = 0;
	RK_U32 u32GetOneFrameTime = 1000 / ctx->iva.u32IvaDetectFrameRate;
	VIDEO_FRAME_INFO_S *stVpssFrame = NULL;

	while (!gPThreadStatus->bIfVpssIvaTHreadQuit) {
		s32Ret = RK_MPI_VPSS_GetChnFrame(0, 3, &ctx->vpss.stChnFrameInfos, -1);
		if (s32Ret == RK_SUCCESS) {
			stVpssFrame = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
			if (!stVpssFrame) {
				RK_LOGE("-----error malloc fail for stVpssFrame");
				RK_MPI_VPSS_ReleaseChnFrame(ctx->vpss.s32GrpId, 3,
				                            &ctx->vpss.stChnFrameInfos);
				continue;
			}
			memcpy(stVpssFrame, &ctx->vpss.stChnFrameInfos, sizeof(VIDEO_FRAME_INFO_S));
			s32Fd = RK_MPI_MB_Handle2Fd(stVpssFrame->stVFrame.pMbBlk);
			memset(&ivaImage, 0, sizeof(RockIvaImage));
			ivaImage.info.transformMode = ctx->iva.eImageTransform;
			ivaImage.info.width = stVpssFrame->stVFrame.u32Width;
			ivaImage.info.height = stVpssFrame->stVFrame.u32Height;
			ivaImage.info.format = ctx->iva.eImageFormat;
			ivaImage.frameId = u32Loopcount;
			ivaImage.dataAddr = NULL;
			ivaImage.dataPhyAddr = NULL;
			ivaImage.dataFd = s32Fd;
			ivaImage.extData = stVpssFrame;
			s32Ret = ROCKIVA_PushFrame(ctx->iva.ivahandle, &ivaImage, NULL);
			u32Loopcount++;
		}
		usleep(u32GetOneFrameTime * 1000);
	}

	RK_LOGE("vpss_iva_thread exit !!!");
	return RK_NULL;
}
#endif

#ifdef ROCKIT_IVS
static void *ivs_detect_thread(void *pArgs) {
	printf("enter ive detect thread------------>\n");
	prctl(PR_SET_NAME, "ivs_detect_thread");
	SAMPLE_IVS_CTX_S *ctx = (SAMPLE_IVS_CTX_S *)pArgs;
	RK_S32 s32Ret = RK_FAILURE;
	IVS_RESULT_INFO_S stResults;
	RK_U32 u32IvsDetectCount = 0;
	IVS_CHN_ATTR_S pstAttr;

	memset(&pstAttr, 0, sizeof(IVS_CHN_ATTR_S));

	RK_MPI_IVS_GetChnAttr(ctx->s32ChnId, &pstAttr);

	RK_LOGE("odIfEnable:%d ", pstAttr.bODEnable);

	while (!gPThreadStatus->bIvsDetectThreadQuit) {
		memset(&stResults, 0, sizeof(IVS_RESULT_INFO_S));
		s32Ret = RK_MPI_IVS_GetResults(ctx->s32ChnId, &stResults, GET_STREAM_TIMEOUT);
		if (s32Ret == RK_SUCCESS) {
			u32IvsDetectCount++;
			// RK_LOGD("s32ReNum: %d", stResults.s32ResultNum);
			if (stResults.s32ResultNum == 1) {
				for (int i = 0; i < stResults.pstResults->stMdInfo.u32RectNum; i++) {
					RK_LOGD("%d: [%d, %d, %d, %d]\n", i,
					        stResults.pstResults->stMdInfo.stRect[i].s32X,
					        stResults.pstResults->stMdInfo.stRect[i].s32Y,
					        stResults.pstResults->stMdInfo.stRect[i].u32Width,
					        stResults.pstResults->stMdInfo.stRect[i].u32Height);
				}
			}

			RK_MPI_IVS_ReleaseResults(ctx->s32ChnId, &stResults);

		} else {
			RK_LOGE("RK_MPI_IVS_GetResults failure:%X", s32Ret);
		}
	}

	RK_LOGE("ivs_detect_thread exit");
	return RK_NULL;
}
#endif

static RK_S32 rgn_init(RK_CHAR *bmp1_file_path, RK_CHAR *bmp2_file_path,
                       SAMPLE_MPI_CTX_S *ctx, RK_S32 rgn_attach_module) {

	RK_S32 s32Ret = RK_FAILURE;
	MPP_CHN_S RGN_CHN;
	RK_U32 u32Width = 0;
	RK_U32 u32Height = 0;
	if (rgn_attach_module == RGN_ATTACH_VENC) {
		RGN_CHN.enModId = RK_ID_VENC;
		RGN_CHN.s32ChnId = 0;
		RGN_CHN.s32DevId = 0;
	} else if (rgn_attach_module == RGN_ATTACH_VPSS) {
		RGN_CHN.enModId = RK_ID_VPSS;
		RGN_CHN.s32ChnId = VPSS_MAX_CHN_NUM;
		RGN_CHN.s32DevId = ctx->vpss.s32GrpId;
	} else {
		RK_LOGE("RGN closed");
		return 0;
	}

	/* Init RGN[0] */
	ctx->rgn[0].rgnHandle = 0;
	ctx->rgn[0].stRgnAttr.enType = COVER_RGN;
	ctx->rgn[0].stMppChn.enModId = RGN_CHN.enModId;
	ctx->rgn[0].stMppChn.s32ChnId = RGN_CHN.s32ChnId;
	ctx->rgn[0].stMppChn.s32DevId = RGN_CHN.s32DevId;
	ctx->rgn[0].stRegion.s32X = 0;        // must be 16 aligned
	ctx->rgn[0].stRegion.s32Y = 0;        // must be 16 aligned
	ctx->rgn[0].stRegion.u32Width = 256;  // must be 16 aligned
	ctx->rgn[0].stRegion.u32Height = 256; // must be 16 aligned
	ctx->rgn[0].u32Color = 0x00f800;      // green
	ctx->rgn[0].u32Layer = 1;
	s32Ret = SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[0]);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("SAMPLE_COMM_RGN_CreateChn Failure s32Ret:%#X rgn handle:%d", s32Ret,
		        ctx->rgn[0].rgnHandle);
		return s32Ret;
	}

	ctx->rgn[1].rgnHandle = 1;
	ctx->rgn[1].stRgnAttr.enType = COVER_RGN;
	ctx->rgn[1].stMppChn.enModId = RGN_CHN.enModId;
	ctx->rgn[1].stMppChn.s32ChnId = RGN_CHN.s32ChnId;
	ctx->rgn[1].stMppChn.s32DevId = RGN_CHN.s32DevId;
	ctx->rgn[1].stRegion.s32X = 0;
	;                                     // must be 16 aligned
	ctx->rgn[1].stRegion.s32Y = 0;        // must be 16 aligned
	ctx->rgn[1].stRegion.u32Width = 128;  // must be 16 aligned
	ctx->rgn[1].stRegion.u32Height = 128; // must be 16 aligned
	ctx->rgn[1].u32Color = 0x00ffff;      // blue
	ctx->rgn[1].u32Layer = 2;
	s32Ret = SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[1]);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("SAMPLE_COMM_RGN_CreateChn Failure s32Ret:%#X rgn handle:%d", s32Ret,
		        ctx->rgn[1].rgnHandle);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_GetBmpResolution(bmp1_file_path, &u32Width, &u32Height);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("SAMPLE_COMM_GetBmpResolution failure");
		u32Width = 256;
		u32Height = 256;
	}
	ctx->rgn[2].rgnHandle = 2;
	ctx->rgn[2].stRgnAttr.unAttr.stOverlay.u32CanvasNum = 2;
	ctx->rgn[2].stRgnAttr.enType = OVERLAY_RGN;
	ctx->rgn[2].stMppChn.enModId = RGN_CHN.enModId;
	ctx->rgn[2].stMppChn.s32ChnId = RGN_CHN.s32ChnId;
	ctx->rgn[2].stMppChn.s32DevId = RGN_CHN.s32DevId;
	ctx->rgn[2].stRegion.s32X = 256;            // must be 16 aligned
	ctx->rgn[2].stRegion.s32Y = 256;            // must be 16 aligned
	ctx->rgn[2].stRegion.u32Width = u32Width;   // must be 8 aligned
	ctx->rgn[2].stRegion.u32Height = u32Height; // must be 8 aligned
	ctx->rgn[2].u32BmpFormat = RK_FMT_BGRA5551;
	ctx->rgn[2].u32BgAlpha = 128;
	ctx->rgn[2].u32FgAlpha = 128;
	ctx->rgn[2].u32Layer = 3;
	ctx->rgn[2].srcFileBmpName = bmp1_file_path;
	if (bmp1_file_path) {
		s32Ret = SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[2]);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("SAMPLE_COMM_RGN_CreateChn Failure s32Ret:%#X rgn handle:%d", s32Ret,
			        ctx->rgn[2].rgnHandle);
			return s32Ret;
		}
	} else {
		RK_LOGE("input_bmp1 file is NULL, overlay rgn skips");
	}

	s32Ret = SAMPLE_COMM_GetBmpResolution(bmp2_file_path, &u32Width, &u32Height);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("SAMPLE_COMM_GetBmpResolution failure");
		u32Width = 128;
		u32Height = 128;
	}
	/* Init RGN[3] */
	ctx->rgn[3].rgnHandle = 3;
	ctx->rgn[3].stRgnAttr.unAttr.stOverlay.u32CanvasNum = 2;
	ctx->rgn[3].stRgnAttr.enType = OVERLAY_RGN;
	ctx->rgn[3].stMppChn.enModId = RGN_CHN.enModId;
	ctx->rgn[3].stMppChn.s32ChnId = RGN_CHN.s32ChnId;
	ctx->rgn[3].stMppChn.s32DevId = RGN_CHN.s32DevId;
	ctx->rgn[3].stRegion.s32X = 512;            // must be 16 aligned
	ctx->rgn[3].stRegion.s32Y = 512;            // must be 16 aligned
	ctx->rgn[3].stRegion.u32Width = u32Width;   // must be 8 aligned
	ctx->rgn[3].stRegion.u32Height = u32Height; // must be 8 aligned
	ctx->rgn[3].u32BmpFormat = RK_FMT_BGRA5551;
	ctx->rgn[3].u32BgAlpha = 128;
	ctx->rgn[3].u32FgAlpha = 128;
	ctx->rgn[3].u32Layer = 4;
	ctx->rgn[3].srcFileBmpName = bmp2_file_path;
	if (bmp2_file_path) {
		s32Ret = SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[3]);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("SAMPLE_COMM_RGN_CreateChn Failure s32Ret:%#X rgn handle:%d", s32Ret,
			        ctx->rgn[3].rgnHandle);
			return s32Ret;
		}
	} else {
		RK_LOGE("input_bmp2 file is NULL, overlay rgn skips");
	}

	return s32Ret;
}

static RK_S32 rgn_deinit(SAMPLE_MPI_CTX_S *ctx, RK_S32 rgn_attach_module) {
	RK_S32 s32Ret = RK_SUCCESS;
	if (rgn_attach_module = 2) {
		return 0;
	}
	for (RK_S32 i = 0; i < RGN_NUM_MAX; i++) {
		if (!ctx->rgn[i].srcFileBmpName && ctx->rgn[i].stRgnAttr.enType == OVERLAY_RGN) {
			continue;
		}
		s32Ret = SAMPLE_COMM_RGN_DestroyChn(&ctx->rgn[i]);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("SAMPLE_COMM_RGN_DestroyChn Failure s32Ret:%#X rgn handle:%d", s32Ret,
			        ctx->rgn[i].rgnHandle);
		}
	}
	return s32Ret;
}

/******************************************************************************
 * function    : main()
 * Description : main
 ******************************************************************************/
int main(int argc, char *argv[]) {
	SAMPLE_MPI_CTX_S *ctx;
	RK_U32 u32VideoWidth = 2688;
	RK_U32 u32VideoHeight = 1520;
	RK_U32 u32SubVideoWidth = 1280;
	RK_U32 u32SubVideoHeight = 720;
	RK_U32 u32ThirdVideoWidth = 704;
	RK_U32 u32ThirdVideoHeight = 576;
	RK_U32 u32ViBuffCnt = 5;
	RK_U32 u32IvsWidth = 896;
	RK_U32 u32IvsHeight = 512;
	RK_U32 u32IvaDetectFrameRate = 10;
	RK_S32 s32Ret = RK_FAILURE;
	RK_CHAR *pInPathBmp1 = NULL;
	RK_CHAR *pInPathBmp2 = NULL;
	RK_CHAR *pOutPathVenc = NULL;
	RK_CHAR *pIvaModelPath = "/oem/usr/lib/";
	RK_CHAR *pIqFileDir = RK_NULL;
	RK_BOOL bMultictx = RK_FALSE;
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
	VENC_RC_MODE_E enRcMode = VENC_RC_MODE_H264CBR;
	RK_CHAR *pCodecName = "H264";
	RK_S32 s32CamId = 0;
	RK_S32 s32loopCnt = -1;
	RK_BOOL enable_ai_isp = RK_TRUE;
	RK_BOOL enable_iva = RK_TRUE;
	RK_BOOL enable_ivs = RK_FALSE;
	RK_S32 s32BitRate = 4 * 1024;
	RK_U32 u32VencFps = 25;
	rk_aiq_working_mode_t eHdrMode = RK_AIQ_WORKING_MODE_NORMAL;
	RK_U32 rgn_attach_module = 1; // 0:vpss,1:venc
	MPP_CHN_S stSrcChn, stDestChn;

	pthread_t vpss_iva_thread_id;

	pthread_t ivs_detect_thread_id;

	if (argc < 2) {
		print_usage(argv[0]);
		g_exit_result = RK_FAILURE;
		goto __PARAM_INIT_FAILED;
	}

	ctx = (SAMPLE_MPI_CTX_S *)(malloc(sizeof(SAMPLE_MPI_CTX_S)));
	if (!ctx) {
		RK_LOGE("ctx is null, malloc failure");
		return RK_FAILURE;
	}
	memset(ctx, 0, sizeof(SAMPLE_MPI_CTX_S));

	s32Ret = global_param_init();
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("global_param_init failure");
		g_exit_result = RK_FAILURE;
		goto __PARAM_INIT_FAILED;
	}

	signal(SIGINT, sigterm_handler);
	signal(SIGTERM, sigterm_handler);

	RK_S32 c = 0;
	while ((c = getopt_long(argc, argv, optstr, long_options, RK_NULL)) != -1) {
		const char *tmp_optarg = optarg;
		switch (c) {
		case 'a':
			if (!optarg && RK_NULL != argv[optind] && '-' != argv[optind][0]) {
				tmp_optarg = argv[optind++];
			}
			if (tmp_optarg) {
				pIqFileDir = (char *)tmp_optarg;
			} else {
				pIqFileDir = RK_NULL;
			}
			break;
		case 'w':
			u32VideoWidth = atoi(optarg);
			break;
		case 'h':
			u32VideoHeight = atoi(optarg);
			break;
		case 'b':
			s32BitRate = atoi(optarg);
			break;
		case 'e':
			if (!strcmp(optarg, "h264cbr")) {
				enCodecType = RK_CODEC_TYPE_H264;
				enRcMode = VENC_RC_MODE_H264CBR;
			} else if (!strcmp(optarg, "h264vbr")) {
				enCodecType = RK_CODEC_TYPE_H264;
				enRcMode = VENC_RC_MODE_H264VBR;
			} else if (!strcmp(optarg, "h264avbr")) {
				enCodecType = RK_CODEC_TYPE_H264;
				enRcMode = VENC_RC_MODE_H264AVBR;
			} else if (!strcmp(optarg, "h265cbr")) {
				enCodecType = RK_CODEC_TYPE_H265;
				enRcMode = VENC_RC_MODE_H265CBR;
			} else if (!strcmp(optarg, "h265vbr")) {
				enCodecType = RK_CODEC_TYPE_H265;
				enRcMode = VENC_RC_MODE_H265VBR;
			} else if (!strcmp(optarg, "h265avbr")) {
				enCodecType = RK_CODEC_TYPE_H265;
				enRcMode = VENC_RC_MODE_H265AVBR;
			} else {
				printf("ERROR: Invalid encoder type.\n");
				print_usage(argv[0]);
				g_exit_result = RK_FAILURE;
				goto __PARAM_INIT_FAILED;
			}
			break;
		case 'i' + 'm':
			pIvaModelPath = optarg;
			break;
		case 'd':
			u32IvaDetectFrameRate = atoi(optarg);
			break;
		case 'o':
			pOutPathVenc = optarg;
			break;
		case 'l':
			s32loopCnt = atoi(optarg);
			break;
		case 'f':
			u32VencFps = atoi(optarg);
			break;
		case 'v':
			u32ViBuffCnt = atoi(optarg);
			break;
		case 'r':
			rgn_attach_module = atoi(optarg);
			break;
		case 'e' + 'a':
			enable_ai_isp = atoi(optarg);
			break;
		case 'e' + 's':
			enable_ivs = atoi(optarg);
			break;
		case 'e' + 'i':
			enable_iva = atoi(optarg);
			break;
		case 'i':
			pInPathBmp1 = optarg;
			break;
		case 'I':
			pInPathBmp2 = optarg;
			break;
		case '?':
		default:
			print_usage(argv[0]);
			return 0;
		}
	}

	printf("#CameraIdx: %d\n", s32CamId);
	printf("#CodecName:%s\n", pCodecName);
	printf("#Output Path: %s\n", pOutPathVenc);
	printf("#RGN_ATTACH: %d\n", rgn_attach_module);
	printf("#IQ Path: %s\n", pIqFileDir);
	if (pIqFileDir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", pIqFileDir);
		printf("#bMultictx: %d\n\n", bMultictx);

		s32Ret = SAMPLE_COMM_ISP_Init(s32CamId, eHdrMode, bMultictx, pIqFileDir);
		s32Ret |= SAMPLE_COMM_ISP_Run(s32CamId);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("ISP init failure");
			g_exit_result = RK_FAILURE;
			goto __FAILED2;
		}
#endif
	}

	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		RK_LOGE("RK_MPI_SYS_Init failure");
		g_exit_result = RK_FAILURE;
		goto __FAILED;
	}

	s32Ret = rtsp_init(enCodecType);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("rtsp_init failure");
		g_exit_result = RK_FAILURE;
		goto __FAILED;
	}

#ifdef ROCKIVA
	/* Init iva */
	ctx->iva.pModelDataPath = pIvaModelPath;
	ctx->iva.u32ImageHeight = u32IvsWidth;
	ctx->iva.u32ImageWidth = u32IvsHeight;
	ctx->iva.u32DetectStartX = 0;
	ctx->iva.u32DetectStartY = 0;
	ctx->iva.u32DetectWidth = u32IvsWidth;
	ctx->iva.u32DetectHight = u32IvsHeight;
	ctx->iva.eImageTransform = ROCKIVA_IMAGE_TRANSFORM_NONE;
	ctx->iva.eImageFormat = ROCKIVA_IMAGE_FORMAT_YUV420SP_NV12;
	ctx->iva.eModeType = ROCKIVA_DET_MODEL_PFP;
	ctx->iva.u32IvaDetectFrameRate = u32IvaDetectFrameRate;
	ctx->iva.resultCallback = rkIvaEvent_callback;
	ctx->iva.releaseCallback = rkIvaFrame_releaseCallBack;
	s32Ret = SAMPLE_COMM_IVA_Create(&ctx->iva);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("SAMPLE_COMM_IVA_Create failure:%#X", s32Ret);
		goto __FAILED;
	}
#endif

	/* Init VI[0] */
	ctx->vi.u32Width = u32VideoWidth;
	ctx->vi.u32Height = u32VideoHeight;
	ctx->vi.s32DevId = 0;
	ctx->vi.u32PipeId = ctx->vi.s32DevId;
	ctx->vi.s32ChnId = 0;
	ctx->vi.stChnAttr.stIspOpt.stMaxSize.u32Width = u32VideoWidth;
	ctx->vi.stChnAttr.stIspOpt.stMaxSize.u32Height = u32VideoHeight;
	ctx->vi.stChnAttr.stIspOpt.u32BufCount = u32ViBuffCnt;
	ctx->vi.stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
	// ctx->vi.stChnAttr.u32Depth = 4;
	ctx->vi.stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
	ctx->vi.stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
	ctx->vi.stChnAttr.stFrameRate.s32SrcFrameRate = -1;
	ctx->vi.stChnAttr.stFrameRate.s32DstFrameRate = -1;
	s32Ret = SAMPLE_COMM_VI_CreateChn(&ctx->vi);
	if (s32Ret != RK_SUCCESS) {
		g_exit_result = RK_FAILURE;
		RK_LOGE("SAMPLE_COMM_VI_CreateChn failure:%d", s32Ret);
		goto __FAILED;
	}

	// Init VPSS[0]
	ctx->vpss.s32GrpId = 0;
	ctx->vpss.s32ChnId = 0;
	ctx->vpss.enVProcDevType = VIDEO_PROC_DEV_RGA;
	ctx->vpss.stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;
	ctx->vpss.stGrpVpssAttr.enCompressMode = COMPRESS_MODE_NONE; // no compress
	ctx->vpss.s32ChnRotation[0] = ROTATION_0;

	// SET VPSS[0,0]
	ctx->vpss.stVpssChnAttr[0].enChnMode = VPSS_CHN_MODE_USER;
	ctx->vpss.stVpssChnAttr[0].enCompressMode = COMPRESS_MODE_NONE;
	ctx->vpss.stVpssChnAttr[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
	ctx->vpss.stVpssChnAttr[0].enPixelFormat = RK_FMT_YUV420SP;
	ctx->vpss.stVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
	ctx->vpss.stVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
	ctx->vpss.stVpssChnAttr[0].u32Width = u32VideoWidth;
	ctx->vpss.stVpssChnAttr[0].u32Height = u32VideoHeight;
	ctx->vpss.stVpssChnAttr[0].u32Depth = 0;

	// SET VPSS[0,1]
	ctx->vpss.stVpssChnAttr[1].enChnMode = VPSS_CHN_MODE_USER;
	ctx->vpss.stVpssChnAttr[1].enCompressMode = COMPRESS_MODE_NONE;
	ctx->vpss.stVpssChnAttr[1].enDynamicRange = DYNAMIC_RANGE_SDR8;
	ctx->vpss.stVpssChnAttr[1].enPixelFormat = RK_FMT_YUV420SP;
	ctx->vpss.stVpssChnAttr[1].stFrameRate.s32SrcFrameRate = -1;
	ctx->vpss.stVpssChnAttr[1].stFrameRate.s32DstFrameRate = -1;
	ctx->vpss.stVpssChnAttr[1].u32Width = u32SubVideoWidth;
	ctx->vpss.stVpssChnAttr[1].u32Height = u32SubVideoHeight;
	ctx->vpss.stVpssChnAttr[1].u32Depth = 0;

	// SET VPSS[0,2]
	ctx->vpss.stVpssChnAttr[2].enChnMode = VPSS_CHN_MODE_USER;
	ctx->vpss.stVpssChnAttr[2].enCompressMode = COMPRESS_MODE_NONE;
	ctx->vpss.stVpssChnAttr[2].enDynamicRange = DYNAMIC_RANGE_SDR8;
	ctx->vpss.stVpssChnAttr[2].enPixelFormat = RK_FMT_YUV420SP;
	ctx->vpss.stVpssChnAttr[2].stFrameRate.s32SrcFrameRate = -1;
	ctx->vpss.stVpssChnAttr[2].stFrameRate.s32DstFrameRate = -1;
	ctx->vpss.stVpssChnAttr[2].u32Width = u32ThirdVideoWidth;
	ctx->vpss.stVpssChnAttr[2].u32Height = u32ThirdVideoHeight;
	ctx->vpss.stVpssChnAttr[2].u32Depth = 0;

#if (defined ROCKIT_IVS) || (defined ROCKIVA)
	// SET VPSS[0,3]
	ctx->vpss.stVpssChnAttr[3].enChnMode = VPSS_CHN_MODE_USER;
	ctx->vpss.stVpssChnAttr[3].enCompressMode = COMPRESS_MODE_NONE;
	ctx->vpss.stVpssChnAttr[3].enDynamicRange = DYNAMIC_RANGE_SDR8;
	ctx->vpss.stVpssChnAttr[3].enPixelFormat = RK_FMT_YUV420SP;
	ctx->vpss.stVpssChnAttr[3].stFrameRate.s32SrcFrameRate = 25;
	ctx->vpss.stVpssChnAttr[3].stFrameRate.s32DstFrameRate = 5;
	ctx->vpss.stVpssChnAttr[3].u32Width = u32IvsWidth;
	ctx->vpss.stVpssChnAttr[3].u32Height = u32IvsHeight;
	ctx->vpss.stVpssChnAttr[3].u32Depth = 0;
	ctx->vpss.stVpssChnAttr[3].u32FrameBufCnt = 1;
	if (enable_iva) {
		ctx->vpss.stVpssChnAttr[3].u32Depth += 1;
	}

#endif
	SAMPLE_COMM_VPSS_CreateChn(&ctx->vpss);
	// set ai isp mode
	if (enable_ai_isp) {
		AIISP_ATTR_S stAIISPAttr;
		MB_BLK pAinrBlk = RK_NULL;
		s32Ret = RK_MPI_MMZ_Alloc(&pAinrBlk, sizeof(rk_ainr_param), 0);
		if (RK_SUCCESS != s32Ret) {
			return s32Ret;
		}
		memset(&stAIISPAttr, 0, sizeof(AIISP_ATTR_S));
		stAIISPAttr.bEnable = RK_TRUE;
		stAIISPAttr.stAiIspCallback.pfUpdateCallback = aiisp_callback;
		stAIISPAttr.stAiIspCallback.pAinrBlk = pAinrBlk;
		stAIISPAttr.stAiIspCallback.pPrivateData = RK_NULL;

		s32Ret = RK_MPI_VPSS_SetGrpAIISPAttr(0, &stAIISPAttr);
		if (RK_SUCCESS != s32Ret) {
			RK_LOGE("VPSS GRP 0 RK_MPI_VPSS_SetGrpAIISPAttr failed with %#x!", s32Ret);
			goto __FAILED;
		}
		RK_LOGD("VPSS GRP 0 RK_MPI_VPSS_SetGrpAIISPAttr success.");
	}

	// Init VENC[0]
	ctx->venc[0].s32ChnId = 0;
	ctx->venc[0].u32Width = u32VideoWidth;
	ctx->venc[0].u32Height = u32VideoHeight;
	ctx->venc[0].u32Fps = u32VencFps;
	ctx->venc[0].u32Gop = 50;
	ctx->venc[0].u32BitRate = s32BitRate;
	ctx->venc[0].enCodecType = enCodecType;
	ctx->venc[0].enRcMode = enRcMode;
	ctx->venc[0].getStreamCbFunc = venc_get_stream;
	ctx->venc[0].s32loopCount = s32loopCnt;
	ctx->venc[0].dstFilePath = pOutPathVenc;
	// H264  66：Baseline  77：Main Profile 100：High Profile
	// H265  0：Main Profile  1：Main 10 Profile
	// MJPEG 0：Baseline
	ctx->venc[0].stChnAttr.stGopAttr.enGopMode =
	    VENC_GOPMODE_NORMALP; // VENC_GOPMODE_SMARTP
	if (RK_CODEC_TYPE_H264 != enCodecType) {
		ctx->venc[0].stChnAttr.stVencAttr.u32Profile = 0;
	} else {
		ctx->venc[0].stChnAttr.stVencAttr.u32Profile = 100;
	}
	SAMPLE_COMM_VENC_CreateChn(&ctx->venc[0]);

	// Init VENC[1]
	ctx->venc[1].s32ChnId = 1;
	ctx->venc[1].u32Width = u32SubVideoWidth;
	ctx->venc[1].u32Height = u32SubVideoHeight;
	ctx->venc[1].u32Fps = u32VencFps;
	ctx->venc[1].u32Gop = 50;
	ctx->venc[1].u32BitRate = s32BitRate;
	ctx->venc[1].enCodecType = enCodecType;
	ctx->venc[1].enRcMode = enRcMode;
	ctx->venc[1].getStreamCbFunc = venc_get_stream;
	ctx->venc[1].s32loopCount = s32loopCnt;
	ctx->venc[1].dstFilePath = pOutPathVenc;
	// H264  66：Baseline  77：Main Profile 100：High Profile
	// H265  0：Main Profile  1：Main 10 Profile
	// MJPEG 0：Baseline
	ctx->venc[1].stChnAttr.stGopAttr.enGopMode =
	    VENC_GOPMODE_NORMALP; // VENC_GOPMODE_SMARTP
	if (RK_CODEC_TYPE_H264 != enCodecType) {
		ctx->venc[1].stChnAttr.stVencAttr.u32Profile = 0;
	} else {
		ctx->venc[1].stChnAttr.stVencAttr.u32Profile = 100;
	}
	SAMPLE_COMM_VENC_CreateChn(&ctx->venc[1]);

	// Init VENC[2]
	ctx->venc[2].s32ChnId = 2;
	ctx->venc[2].u32Width = u32ThirdVideoWidth;
	ctx->venc[2].u32Height = u32ThirdVideoHeight;
	ctx->venc[2].u32Fps = u32VencFps;
	ctx->venc[2].u32Gop = 50;
	ctx->venc[2].u32BitRate = s32BitRate;
	ctx->venc[2].enCodecType = enCodecType;
	ctx->venc[2].enRcMode = enRcMode;
	ctx->venc[2].getStreamCbFunc = venc_get_stream;
	ctx->venc[2].s32loopCount = s32loopCnt;
	ctx->venc[2].dstFilePath = pOutPathVenc;
	// H264  66：Baseline  77：Main Profile 100：High Profile
	// H265  0：Main Profile  1：Main 10 Profile
	// MJPEG 0：Baseline
	ctx->venc[2].stChnAttr.stGopAttr.enGopMode =
	    VENC_GOPMODE_NORMALP; // VENC_GOPMODE_SMARTP
	if (RK_CODEC_TYPE_H264 != enCodecType) {
		ctx->venc[2].stChnAttr.stVencAttr.u32Profile = 0;
	} else {
		ctx->venc[2].stChnAttr.stVencAttr.u32Profile = 100;
	}
	SAMPLE_COMM_VENC_CreateChn(&ctx->venc[2]);

#ifdef ROCKIT_IVS
	/* Init ivs */
	if (enable_ivs) {
		ctx->ivs.s32ChnId = 0;
		ctx->ivs.stIvsAttr.enMode = IVS_MODE_MD_OD;
		ctx->ivs.stIvsAttr.u32PicWidth = u32IvsWidth;
		ctx->ivs.stIvsAttr.u32PicHeight = u32IvsHeight;
		ctx->ivs.stIvsAttr.enPixelFormat = RK_FMT_YUV420SP;
		ctx->ivs.stIvsAttr.s32Gop = 30;
		ctx->ivs.stIvsAttr.bSmearEnable = RK_FALSE;
		ctx->ivs.stIvsAttr.bWeightpEnable = RK_FALSE;
		ctx->ivs.stIvsAttr.bMDEnable = RK_TRUE;
		ctx->ivs.stIvsAttr.s32MDInterval = 5;
		ctx->ivs.stIvsAttr.bMDNightMode = RK_TRUE;
		ctx->ivs.stIvsAttr.u32MDSensibility = 3;
		ctx->ivs.stIvsAttr.bODEnable = RK_TRUE;
		ctx->ivs.stIvsAttr.s32ODInterval = 1;
		ctx->ivs.stIvsAttr.s32ODPercent = 7;
		s32Ret = SAMPLE_COMM_IVS_Create(&ctx->ivs);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("SAMPLE_COMM_IVS_Create failure:%X", s32Ret);
			program_handle_error(__func__, __LINE__);
		}
	}
#endif

	/*rgn init*/
	rgn_init(pInPathBmp1, pInPathBmp2, ctx, rgn_attach_module);

	// Bind VI and VPSS[0]
	stSrcChn.enModId = RK_ID_VI;
	stSrcChn.s32DevId = ctx->vi.s32DevId;
	stSrcChn.s32ChnId = ctx->vi.s32ChnId;
	stDestChn.enModId = RK_ID_VPSS;
	stDestChn.s32DevId = ctx->vpss.s32GrpId;
	stDestChn.s32ChnId = ctx->vpss.s32ChnId;
	SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

	/* Bind VPSS and VENC[0] */
	stSrcChn.enModId = RK_ID_VPSS;
	stSrcChn.s32DevId = ctx->vpss.s32GrpId;
	stSrcChn.s32ChnId = ctx->vpss.s32ChnId;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc[0].s32ChnId;
	SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

	/* Bind VPSS and VENC[1] */
	stSrcChn.enModId = RK_ID_VPSS;
	stSrcChn.s32DevId = ctx->vpss.s32GrpId;
	stSrcChn.s32ChnId = 1;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc[1].s32ChnId;
	SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

	/* Bind VPSS and VENC[2] */
	stSrcChn.enModId = RK_ID_VPSS;
	stSrcChn.s32DevId = ctx->vpss.s32GrpId;
	stSrcChn.s32ChnId = 2;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc[2].s32ChnId;
	SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

#ifdef ROCKIT_IVS
	if (enable_ivs) {
		/* VPSS bind IVS[0]*/

		stSrcChn.enModId = RK_ID_VPSS;
		stSrcChn.s32DevId = ctx->vpss.s32GrpId;
		stSrcChn.s32ChnId = 3;
		stDestChn.enModId = RK_ID_IVS;
		stDestChn.s32DevId = 0;
		stDestChn.s32ChnId = ctx->ivs.s32ChnId;
		SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

		/* ivs detect thread launch */
		pthread_create(&ivs_detect_thread_id, 0, ivs_detect_thread, (void *)&ctx->ivs);
	}
#endif
#ifdef ROCKIVA
	// vpss iva thread launch
	if (enable_iva)
		pthread_create(&vpss_iva_thread_id, 0, vpss_iva_thread, (void *)ctx);
#endif

	printf("%s initial finish\n", __func__);

	while (!gPThreadStatus->bIfMainThreadQuit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);

	rgn_deinit(ctx, rgn_attach_module);
#ifdef ROCKIVA
	/* Destroy IVA */
	if (enable_iva) {
		gPThreadStatus->bIfVpssIvaTHreadQuit = RK_TRUE;
		pthread_join(vpss_iva_thread_id, RK_NULL);
		SAMPLE_COMM_IVA_Destroy(&ctx->iva);
	}

#endif
#ifdef ROCKIT_IVS
	if (enable_ivs) {
		/* ivs detect thread exit*/
		gPThreadStatus->bIvsDetectThreadQuit = RK_TRUE;
		pthread_join(ivs_detect_thread_id, RK_NULL);
		/* VPSS[3] unbind IVS[0]*/
		stSrcChn.enModId = RK_ID_VPSS;
		stSrcChn.s32DevId = ctx->vpss.s32GrpId;
		stSrcChn.s32ChnId = 0;
		stDestChn.enModId = RK_ID_IVS;
		stDestChn.s32DevId = 0;
		stDestChn.s32ChnId = ctx->ivs.s32ChnId;
		s32Ret = SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("VPSS and IVS bind failure:%X", s32Ret);
			g_exit_result = RK_FAILURE;
		}

		/* ivs chn destroy*/
		SAMPLE_COMM_IVS_Destroy(ctx->ivs.s32ChnId);
	}
#endif

	for (int i = 0; i < VENC_CHN_MAX; i++) {
		gPThreadStatus->bIfVencThreadQuit[i] = true;
		pthread_join(ctx->venc[i].getStreamThread, RK_NULL);
	}
	/* Venc[0] unbind VPSS[0,0]*/
	stSrcChn.enModId = RK_ID_VPSS;
	stSrcChn.s32DevId = ctx->vpss.s32GrpId;
	stSrcChn.s32ChnId = ctx->vpss.s32ChnId;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc[0].s32ChnId;
	s32Ret = SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("venc[0] and Vpss unbind failure:%X", s32Ret);
		g_exit_result = RK_FAILURE;
	}

	/* Venc[1] unbind VPSS[0,1]*/
	stSrcChn.enModId = RK_ID_VPSS;
	stSrcChn.s32DevId = ctx->vpss.s32GrpId;
	stSrcChn.s32ChnId = 1;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc[1].s32ChnId;
	s32Ret = SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("Venc[1] and Vpss unbind failure:%X", s32Ret);
		g_exit_result = RK_FAILURE;
	}

	/* Venc[2] unbind VPSS[0,2]*/
	stSrcChn.enModId = RK_ID_VPSS;
	stSrcChn.s32DevId = ctx->vpss.s32GrpId;
	stSrcChn.s32ChnId = 2;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc[2].s32ChnId;
	s32Ret = SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("Venc[2] and Vpss unbind failure:%X", s32Ret);
		g_exit_result = RK_FAILURE;
	}

	SAMPLE_COMM_VENC_DestroyChn(&ctx->venc[0]);
	SAMPLE_COMM_VENC_DestroyChn(&ctx->venc[1]);
	SAMPLE_COMM_VENC_DestroyChn(&ctx->venc[2]);

	// UnBind VI and VPSS
	stSrcChn.enModId = RK_ID_VI;
	stSrcChn.s32DevId = ctx->vi.s32DevId;
	stSrcChn.s32ChnId = ctx->vi.s32ChnId;
	stDestChn.enModId = RK_ID_VPSS;
	stDestChn.s32DevId = ctx->vpss.s32GrpId;
	stDestChn.s32ChnId = ctx->vpss.s32ChnId;
	s32Ret = SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("VI and Vpss unbind failure:%X", s32Ret);
		g_exit_result = RK_FAILURE;
	}
	// Destroy VPSS
	SAMPLE_COMM_VPSS_DestroyChn(&ctx->vpss);
	// Destroy VI
	SAMPLE_COMM_VI_DestroyChn(&ctx->vi);
	/* rtsp deinit */
	rtsp_deinit();
__FAILED:
	RK_MPI_SYS_Exit();
	if (pIqFileDir) {
#ifdef RKAIQ
		SAMPLE_COMM_ISP_Stop(s32CamId);
#endif
	}
__FAILED2:
	global_param_deinit();

	if (ctx) {
		free(ctx);
		ctx = RK_NULL;
	}
__PARAM_INIT_FAILED:
	return g_exit_result;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
