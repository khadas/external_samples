/*
 * Copyright 2021 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <time.h>
#include <unistd.h>

#include "sample_comm.h"

typedef struct _rkMpiCtx {
	SAMPLE_VI_CTX_S vi[6];
	SAMPLE_VO_CTX_S vo;
	SAMPLE_AVS_CTX_S avs;
	SAMPLE_VENC_CTX_S venc;
	SAMPLE_RGN_CTX_S rgn[2];
} SAMPLE_MPI_CTX_S;
RK_S32 s32test = 1;

static int g_loopcount = 1;
static bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	g_loopcount = 0;
	quit = true;
}

static RK_CHAR optstr[] = "?::n:l:o:";
static const struct option long_options[] = {
    {"index", required_argument, NULL, 'n'},
    {"loop_count", required_argument, NULL, 'l'},
    {NULL, 0, NULL, 0},
};

/******************************************************************************
* function : show usage
******************************************************************************/
static void print_usage(const RK_CHAR *name) {
	printf("Usage : %s -n <index> -l <loop count>\n", name);
	printf("index:\n");
	printf("\t 0)isp stresstest...\n");
	printf("\t 1)venc stresstest...\n");
	printf("\t 2)vi[6]->avs->venc stresstest...\n");
}

/******************************************************************************
* function : vi thread
******************************************************************************/
static void *vi_get_stream(void *pArgs) {
	SAMPLE_VI_CTX_S *ctx = (SAMPLE_VI_CTX_S *)(pArgs);
	RK_S32 s32Ret = RK_FAILURE;
	char name[256] = {0};
	FILE *fp = RK_NULL;
	void *pData = RK_NULL;
	RK_S32 loopCount = 0;

	if (ctx->dstFilePath) {
		snprintf(name, sizeof(name), "/%s/vi_%d.bin", ctx->dstFilePath, ctx->s32DevId);
		fp = fopen(name, "wb");
		if (fp == RK_NULL) {
			printf("chn %d can't open %s file !\n", ctx->s32DevId, ctx->dstFilePath);
			quit = true;
			return RK_NULL;
		}
	}

	while (!quit) {
		s32Ret = SAMPLE_COMM_VI_GetChnFrame(ctx, &pData);
		if (s32Ret == RK_SUCCESS) {
			if (ctx->stViFrame.u32Len <= 0) {
				continue;
			}

			// exit when complete
			if (ctx->s32loopCount > 0) {
				if (loopCount >= ctx->s32loopCount) {
					SAMPLE_COMM_VI_ReleaseChnFrame(ctx);
					quit = true;
					break;
				}
			}

			if (fp) {
				fwrite(pData, 1, ctx->stViFrame.u32Len, fp);
				fflush(fp);
			}

			printf("SAMPLE_COMM_VI_GetChnFrame DevId %d ok:data %p loop:%d seq:%d "
			       "pts:%lld ms\n",
			       ctx->s32DevId, pData, loopCount, ctx->stViFrame.s32Seq,
			       ctx->stViFrame.s64PTS / 1000);
			SAMPLE_COMM_VI_ReleaseChnFrame(ctx);
			loopCount++;
		}
		usleep(1000);
	}

	if (fp)
		fclose(fp);

	return RK_NULL;
}

/******************************************************************************
* function : venc thread
******************************************************************************/
static void *venc_get_stream(void *pArgs) {
	SAMPLE_VENC_CTX_S *ctx = (SAMPLE_VENC_CTX_S *)(pArgs);
	RK_S32 s32Ret = RK_FAILURE;
	char name[256] = {0};
	FILE *fp = RK_NULL;
	void *pData = RK_NULL;
	RK_S32 loopCount = 0;

	if (ctx->dstFilePath) {
		snprintf(name, sizeof(name), "/%s/venc_%d.bin", ctx->dstFilePath, ctx->s32ChnId);
		fp = fopen(name, "wb");
		if (fp == RK_NULL) {
			printf("chn %d can't open %s file !\n", ctx->s32ChnId, ctx->dstFilePath);
			quit = true;
			return RK_NULL;
		}
	}

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

			if (fp) {
				fwrite(pData, 1, ctx->stFrame.pstPack->u32Len, fp);
				fflush(fp);
			}

			PrintStreamDetails(ctx->s32ChnId, ctx->stFrame.pstPack->u32Len);

			RK_LOGD("chn:%d, loopCount:%d wd:%d\n", ctx->s32ChnId, loopCount,
			        ctx->stFrame.pstPack->u32Len);

			SAMPLE_COMM_VENC_ReleaseStream(ctx);
			loopCount++;
		}
		usleep(1000);
	}

	if (fp)
		fclose(fp);

	return RK_NULL;
}

/******************************************************************************
* function    : SAMPLE_COMM_ISP_Stresstest
******************************************************************************/
int SAMPLE_CAMERA_ISP_Stresstest(SAMPLE_MPI_CTX_S *ctx, char *pIqFileDir) {
	RK_S32 s32Ret = RK_FAILURE;
	int video_width = 1920;
	int video_height = 1080;
	RK_CHAR *pDeviceName = NULL;
	RK_CHAR *pOutPath = "/data/";
	char *iq_file_dir = pIqFileDir;
	RK_S32 i, s32CamNum = 6;
	RK_S32 s32loopCnt = 60;
	MPP_CHN_S stSrcChn, stDestChn;
	FILE *fp = RK_NULL;
	void *pData = RK_NULL;
	RK_BOOL bMultictx = RK_FALSE;
	pthread_t vi_thread_id[6];
	quit = false;

	printf("#CameraNum: %d\n", s32CamNum);
	printf("#pDeviceName: %s\n", pDeviceName);
	printf("#Output Path: %s\n", pOutPath);
	printf("#IQ Path: %s\n", iq_file_dir);
	if (iq_file_dir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
		printf("#bMultictx: %d\n\n", bMultictx);
		int fps = 30;
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
		rk_aiq_camgroup_instance_cfg_t camgroup_cfg;
		camgroup_cfg.sns_num = s32CamNum;
		camgroup_cfg.config_file_dir = iq_file_dir;

		SAMPLE_COMM_ISP_CamGroup_Init(0, hdr_mode, bMultictx, &camgroup_cfg);
#endif
	}

	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		printf("RK_MPI_SYS_Init failed!\n");
		goto __FAILED;
	}

	// Init VI
	for (i = 0; i < s32CamNum; i++) {
		ctx->vi[i].u32Width = video_width;
		ctx->vi[i].u32Height = video_height;
		ctx->vi[i].s32DevId = i;
		ctx->vi[i].u32PipeId = ctx->vi[i].s32DevId;
		ctx->vi[i].s32ChnId = 0;
		ctx->vi[i].stChnAttr.stIspOpt.u32BufCount = 3;
		ctx->vi[i].stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
		ctx->vi[i].stChnAttr.u32Depth = 2;
		ctx->vi[i].stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
		ctx->vi[i].stChnAttr.stFrameRate.s32SrcFrameRate = -1;
		ctx->vi[i].stChnAttr.stFrameRate.s32DstFrameRate = -1;
		ctx->vi[i].dstFilePath = pOutPath;
		ctx->vi[i].s32loopCount = s32loopCnt;
		SAMPLE_COMM_VI_CreateChn(&ctx->vi[i]);

		pthread_create(&vi_thread_id[i], 0, vi_get_stream, (void *)(&ctx->vi[i]));
	}
	printf("%s initial finish\n", __func__);

	while (!quit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);

	for (i = 0; i < s32CamNum; i++) {
		pthread_join(vi_thread_id[i], NULL);
	}

	// Destroy VI[0]
	for (i = 0; i < s32CamNum; i++) {
		SAMPLE_COMM_VI_DestroyChn(&ctx->vi[i]);
	}
__FAILED:
	RK_MPI_SYS_Exit();
	if (iq_file_dir) {
#ifdef RKAIQ
		SAMPLE_COMM_ISP_CamGroup_Stop(0);
#endif
	}

	return 0;
}

/******************************************************************************
* function    : SAMPLE_COMM_VENC_Stresstest
******************************************************************************/
int SAMPLE_CAMERA_VENC_SetConfig(SAMPLE_MPI_CTX_S *ctx, MPP_CHN_S *pSrc,
                                 MPP_CHN_S *pDest) {
	RK_S32 s32Ret = RK_FAILURE;
	quit = false;

	printf("%s entering!\n", __func__);
	// UnBind avs[0] and VENC[0]
	SAMPLE_COMM_UnBind(pSrc, pDest);

	// Destroy VENC[0]
	SAMPLE_COMM_VENC_DestroyChn(&ctx->venc);

	// Init VENC[0]
	SAMPLE_COMM_VENC_CreateChn(&ctx->venc);

	// Bind AVS[0] and VENC[0]
	SAMPLE_COMM_Bind(pSrc, pDest);

	while (!quit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);

	if (ctx->venc.getStreamCbFunc) {
		pthread_join(ctx->venc.getStreamThread, NULL);
	}

	return 0;
}

int SAMPLE_CAMERA_VENC_Stresstest(SAMPLE_MPI_CTX_S *ctx, char *pCodecName) {
	RK_S32 s32Ret = RK_FAILURE;
	int video_width = 2560;
	int video_height = 1520;
	int venc_width = 8192;
	int venc_height = 2700;
	RK_CHAR *pAvsLutFilePath = "/usr/share/avs_mesh/";
	RK_CHAR *pInPathBmp = "/usr/share/image.bmp";
	RK_CHAR *pOutPathVenc = "/data/";
	RK_CHAR *iq_file_dir = "/etc/iqfiles";
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H265;
	VENC_RC_MODE_E enRcMode = VENC_RC_MODE_H265CBR;
	RK_S32 s32BitRate = 4 * 1024;
	RK_S32 s32CamId = 0;
	MPP_CHN_S stSrcChn, stDestChn;
	RK_S32 s32CamNum = 6;
	RK_S32 s32loopCnt = 200;
	RK_S32 i;
	RK_BOOL bMultictx = RK_FALSE;
	quit = false;

	if (pCodecName == "H265") {
		enCodecType = RK_CODEC_TYPE_H265;
		enRcMode = VENC_RC_MODE_H265CBR;
	} else if (pCodecName == "H264") {
		enCodecType = RK_CODEC_TYPE_H264;
		enRcMode = VENC_RC_MODE_H264CBR;
	}

	printf("#CameraIdx: %d\n", s32CamId);
	printf("#pAvsLutFilePath: %s\n", pAvsLutFilePath);
	printf("#CodecName:%s\n", pCodecName);
	printf("#Output Path: %s\n", pOutPathVenc);

	if (iq_file_dir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
		printf("#bMultictx: %d\n\n", bMultictx);
		int fps = 30;
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
		rk_aiq_camgroup_instance_cfg_t camgroup_cfg;
		camgroup_cfg.sns_num = s32CamNum;
		camgroup_cfg.config_file_dir = iq_file_dir;

		SAMPLE_COMM_ISP_CamGroup_Init(s32CamId, hdr_mode, bMultictx, &camgroup_cfg);
#endif
	}

	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		goto __FAILED;
	}

	// Init VI[0] ~ VI[5]
	for (i = 0; i < s32CamNum; i++) {
		ctx->vi[i].u32Width = video_width;
		ctx->vi[i].u32Height = video_height;
		ctx->vi[i].s32DevId = i;
		ctx->vi[i].u32PipeId = i;
		ctx->vi[i].s32ChnId = 2; // rk3588 mainpath:0 selfpath:1 fbcpath:2
		ctx->vi[i].stChnAttr.stIspOpt.u32BufCount = 6;
		ctx->vi[i].stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
		ctx->vi[i].stChnAttr.u32Depth = 2;
		ctx->vi[i].stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
		ctx->vi[i].stChnAttr.enCompressMode = COMPRESS_AFBC_16x16;
		ctx->vi[i].stChnAttr.stFrameRate.s32SrcFrameRate = -1;
		ctx->vi[i].stChnAttr.stFrameRate.s32DstFrameRate = -1;
		SAMPLE_COMM_VI_CreateChn(&ctx->vi[i]);
	}

	// Init avs[0]
	ctx->avs.s32GrpId = 0;
	ctx->avs.s32ChnId = 0;
	ctx->avs.stAvsModParam.u32WorkingSetSize = 67 * 1024;
	ctx->avs.stAvsGrpAttr.enMode = 0; // 0: blend 1: no blend
	ctx->avs.stAvsGrpAttr.u32PipeNum = s32CamNum;
	ctx->avs.stAvsGrpAttr.stGainAttr.enMode = AVS_GAIN_MODE_AUTO;
	ctx->avs.stAvsGrpAttr.stOutAttr.enPrjMode = AVS_PROJECTION_EQUIRECTANGULAR;
	ctx->avs.stAvsGrpAttr.stOutAttr.stCenter.s32X = 5088 / 2;
	ctx->avs.stAvsGrpAttr.stOutAttr.stCenter.s32Y = 1520 / 2;
	ctx->avs.stAvsGrpAttr.stOutAttr.stFOV.u32FOVX = 36000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stFOV.u32FOVY = 18000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Roll = 9000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Pitch = 9000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Yaw = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Roll = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Pitch = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Yaw = 0;
	ctx->avs.stAvsGrpAttr.stLUT.enAccuracy = AVS_LUT_ACCURACY_HIGH;
	ctx->avs.stAvsGrpAttr.bSyncPipe = RK_FALSE;
	ctx->avs.stAvsGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	ctx->avs.stAvsGrpAttr.stFrameRate.s32DstFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].enCompressMode = COMPRESS_AFBC_16x16;
	ctx->avs.stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].u32Depth = 1;
	ctx->avs.stAvsChnAttr[0].u32Width = 8192;
	ctx->avs.stAvsChnAttr[0].u32Height = 2700;
	ctx->avs.stAvsChnAttr[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
	strcpy(ctx->avs.stAvsGrpAttr.stLUT.aFilePath, pAvsLutFilePath);
	SAMPLE_COMM_AVS_CreateChn(&ctx->avs);

	// Init VENC[0]
	ctx->venc.s32ChnId = 0;
	ctx->venc.u32Width = venc_width;
	ctx->venc.u32Height = venc_height;
	ctx->venc.u32Fps = 30;
	ctx->venc.u32Gop = 50;
	ctx->venc.u32BitRate = s32BitRate;
	ctx->venc.enCodecType = enCodecType;
	ctx->venc.enRcMode = enRcMode;
	ctx->venc.getStreamCbFunc = venc_get_stream;
	ctx->venc.s32loopCount = s32loopCnt;
	ctx->venc.dstFilePath = pOutPathVenc;
	// H264  66：Baseline  77：Main Profile 100：High Profile
	// H265  0：Main Profile  1：Main 10 Profile
	// MJPEG 0：Baseline
	ctx->venc.stChnAttr.stVencAttr.u32Profile = 0;
	ctx->venc.stChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP; // VENC_GOPMODE_SMARTP
	SAMPLE_COMM_VENC_CreateChn(&ctx->venc);

	// Bind VI[0]~VI[5] and avs[0]
	for (i = 0; i < s32CamNum; i++) {
		stSrcChn.enModId = RK_ID_VI;
		stSrcChn.s32DevId = ctx->vi[i].s32DevId;
		stSrcChn.s32ChnId = ctx->vi[i].s32ChnId;
		stDestChn.enModId = RK_ID_AVS;
		stDestChn.s32DevId = ctx->avs.s32GrpId;
		stDestChn.s32ChnId = i;
		SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);
	}

	// Bind AVS[0] and VENC[0]
	stSrcChn.enModId = RK_ID_AVS;
	stSrcChn.s32DevId = ctx->avs.s32GrpId;
	stSrcChn.s32ChnId = ctx->avs.s32ChnId;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc.s32ChnId;
	SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

	printf("%s initial finish\n", __func__);

	while (!quit) {
		sleep(1);
	}

	if (ctx->venc.getStreamCbFunc) {
		pthread_join(ctx->venc.getStreamThread, NULL);
	}

	if (g_loopcount > 0) {
		g_loopcount--;
		printf("sample_camera_stresstest: g_loopcount(%d)\n", g_loopcount);
	}

	while (g_loopcount) {
		if (g_loopcount % 2 == 0) {
			printf("#CodecName:%s\n", "H265");
			ctx->venc.enCodecType = RK_CODEC_TYPE_H265;
			ctx->venc.enRcMode = VENC_RC_MODE_H265CBR;
			s32Ret = SAMPLE_CAMERA_VENC_SetConfig(ctx, &stSrcChn, &stDestChn);
		} else {
			printf("#CodecName:%s\n", "H264");
			ctx->venc.enCodecType = RK_CODEC_TYPE_H264;
			ctx->venc.enRcMode = VENC_RC_MODE_H264CBR;
			s32Ret = SAMPLE_CAMERA_VENC_SetConfig(ctx, &stSrcChn, &stDestChn);
		}

		if (g_loopcount > 0) {
			g_loopcount--;
		} else {
			break;
		}

		sleep(2);
		printf("sample_camera_stresstest: g_loopcount(%d)\n", g_loopcount);
	}

	printf("%s exit!\n", __func__);

	// UnBind avs[0] and VENC[0]
	stSrcChn.enModId = RK_ID_AVS;
	stSrcChn.s32DevId = ctx->avs.s32GrpId;
	stSrcChn.s32ChnId = ctx->avs.s32ChnId;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc.s32ChnId;
	SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);

	// UnBind Bind VI[0]~VI[5]
	for (i = 0; i < s32CamNum; i++) {
		stSrcChn.enModId = RK_ID_VI;
		stSrcChn.s32DevId = ctx->vi[i].s32DevId;
		stSrcChn.s32ChnId = ctx->vi[i].s32ChnId;
		stDestChn.enModId = RK_ID_AVS;
		stDestChn.s32DevId = ctx->avs.s32GrpId;
		stDestChn.s32ChnId = i;
		SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
	}

	// Destroy VENC[0]
	SAMPLE_COMM_VENC_DestroyChn(&ctx->venc);
	// Destroy AVS[0]
	SAMPLE_COMM_AVS_DestroyChn(&ctx->avs);
	// Destroy VI[0]
	for (i = 0; i < s32CamNum; i++) {
		SAMPLE_COMM_VI_DestroyChn(&ctx->vi[i]);
	}

__FAILED:
	RK_MPI_SYS_Exit();
	if (iq_file_dir) {
#ifdef RKAIQ
		SAMPLE_COMM_ISP_CamGroup_Stop(s32CamId);
#endif
	}

	return 0;
}

/******************************************************************************
* function    : SAMPLE_COMM_VI_AVS_VENC_Stresstest
******************************************************************************/
int SAMPLE_CAMERA_VI_AVS_VENC_Stresstest(SAMPLE_MPI_CTX_S *ctx) {
	RK_S32 s32Ret = RK_FAILURE;
	int video_width = 2560;
	int video_height = 1520;
	int venc_width = 8192;
	int venc_height = 2700;
	RK_CHAR *pAvsLutFilePath = "/usr/share/avs_mesh/";
	RK_CHAR *pInPathBmp = "/usr/share/image.bmp";
	RK_CHAR *pOutPathVenc = "/data/";
	RK_CHAR *iq_file_dir = "/etc/iqfiles";
	RK_CHAR *pCodecName = "H265";
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H265;
	VENC_RC_MODE_E enRcMode = VENC_RC_MODE_H265CBR;
	RK_S32 s32BitRate = 4 * 1024;
	RK_S32 s32CamId = 0;
	MPP_CHN_S stSrcChn, stDestChn;
	RK_S32 s32CamNum = 6;
	RK_S32 s32loopCnt = 200;
	RK_S32 i;
	RK_BOOL bMultictx = RK_FALSE;
	quit = false;

	if (pCodecName == "H265") {
		enCodecType = RK_CODEC_TYPE_H265;
		enRcMode = VENC_RC_MODE_H265CBR;
	} else if (pCodecName == "H264") {
		enCodecType = RK_CODEC_TYPE_H264;
		enRcMode = VENC_RC_MODE_H264CBR;
	}

	printf("#CameraIdx: %d\n", s32CamId);
	printf("#pAvsLutFilePath: %s\n", pAvsLutFilePath);
	printf("#CodecName:%s\n", pCodecName);
	printf("#Output Path: %s\n", pOutPathVenc);
	printf("#IQ Path: %s\n", iq_file_dir);
	if (iq_file_dir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
		printf("#bMultictx: %d\n\n", bMultictx);
		int fps = 30;
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
		rk_aiq_camgroup_instance_cfg_t camgroup_cfg;
		camgroup_cfg.sns_num = s32CamNum;
		camgroup_cfg.config_file_dir = iq_file_dir;

		SAMPLE_COMM_ISP_CamGroup_Init(s32CamId, hdr_mode, bMultictx, &camgroup_cfg);
#endif
	}

	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		goto __FAILED;
	}

	// Init VI[0] ~ VI[5]
	for (i = 0; i < s32CamNum; i++) {
		ctx->vi[i].u32Width = video_width;
		ctx->vi[i].u32Height = video_height;
		ctx->vi[i].s32DevId = i;
		ctx->vi[i].u32PipeId = i;
		ctx->vi[i].s32ChnId = 2; // rk3588 mainpath:0 selfpath:1 fbcpath:2
		ctx->vi[i].stChnAttr.stIspOpt.u32BufCount = 6;
		ctx->vi[i].stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
		ctx->vi[i].stChnAttr.u32Depth = 2;
		ctx->vi[i].stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
		ctx->vi[i].stChnAttr.enCompressMode = COMPRESS_AFBC_16x16;
		ctx->vi[i].stChnAttr.stFrameRate.s32SrcFrameRate = -1;
		ctx->vi[i].stChnAttr.stFrameRate.s32DstFrameRate = -1;
		SAMPLE_COMM_VI_CreateChn(&ctx->vi[i]);
	}

	// Init avs[0]
	ctx->avs.s32GrpId = 0;
	ctx->avs.s32ChnId = 0;
	ctx->avs.stAvsModParam.u32WorkingSetSize = 67 * 1024;
	ctx->avs.stAvsGrpAttr.enMode = 0; // 0: blend 1: no blend
	ctx->avs.stAvsGrpAttr.u32PipeNum = s32CamNum;
	ctx->avs.stAvsGrpAttr.stGainAttr.enMode = AVS_GAIN_MODE_AUTO;
	ctx->avs.stAvsGrpAttr.stOutAttr.enPrjMode = AVS_PROJECTION_EQUIRECTANGULAR;
	ctx->avs.stAvsGrpAttr.stOutAttr.stCenter.s32X = 5088 / 2;
	ctx->avs.stAvsGrpAttr.stOutAttr.stCenter.s32Y = 1520 / 2;
	ctx->avs.stAvsGrpAttr.stOutAttr.stFOV.u32FOVX = 36000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stFOV.u32FOVY = 18000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Roll = 9000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Pitch = 9000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Yaw = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Roll = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Pitch = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Yaw = 0;
	ctx->avs.stAvsGrpAttr.stLUT.enAccuracy = AVS_LUT_ACCURACY_HIGH;
	ctx->avs.stAvsGrpAttr.bSyncPipe = RK_FALSE;
	ctx->avs.stAvsGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	ctx->avs.stAvsGrpAttr.stFrameRate.s32DstFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].enCompressMode = COMPRESS_AFBC_16x16;
	ctx->avs.stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].u32Depth = 1;
	ctx->avs.stAvsChnAttr[0].u32Width = 8192;
	ctx->avs.stAvsChnAttr[0].u32Height = 2700;
	ctx->avs.stAvsChnAttr[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
	strcpy(ctx->avs.stAvsGrpAttr.stLUT.aFilePath, pAvsLutFilePath);
	SAMPLE_COMM_AVS_CreateChn(&ctx->avs);

	// Init VENC[0]
	ctx->venc.s32ChnId = 0;
	ctx->venc.u32Width = venc_width;
	ctx->venc.u32Height = venc_height;
	ctx->venc.u32Fps = 30;
	ctx->venc.u32Gop = 50;
	ctx->venc.u32BitRate = s32BitRate;
	ctx->venc.enCodecType = enCodecType;
	ctx->venc.enRcMode = enRcMode;
	ctx->venc.getStreamCbFunc = venc_get_stream;
	ctx->venc.s32loopCount = s32loopCnt;
	ctx->venc.dstFilePath = pOutPathVenc;
	// H264  66：Baseline  77：Main Profile 100：High Profile
	// H265  0：Main Profile  1：Main 10 Profile
	// MJPEG 0：Baseline
	ctx->venc.stChnAttr.stVencAttr.u32Profile = 0;
	ctx->venc.stChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP; // VENC_GOPMODE_SMARTP
	SAMPLE_COMM_VENC_CreateChn(&ctx->venc);

	// Init RGN[0]
	ctx->rgn[0].rgnHandle = 1;
	ctx->rgn[0].stRgnAttr.enType = OVERLAY_RGN;
	ctx->rgn[0].stMppChn.enModId = RK_ID_VENC;
	ctx->rgn[0].stMppChn.s32ChnId = 0;
	ctx->rgn[0].stMppChn.s32DevId = ctx->venc.s32ChnId;
	ctx->rgn[0].stRegion.s32X = 0;        // must be 16 aligned
	ctx->rgn[0].stRegion.s32Y = 0;        // must be 16 aligned
	ctx->rgn[0].stRegion.u32Width = 576;  // must be 16 aligned
	ctx->rgn[0].stRegion.u32Height = 288; // must be 16 aligned
	ctx->rgn[0].u32BmpFormat = RK_FMT_BGRA5551;
	ctx->rgn[0].u32BgAlpha = 128;
	ctx->rgn[0].u32FgAlpha = 128;
	ctx->rgn[0].u32Layer = 1;
	ctx->rgn[0].srcFileBmpName = pInPathBmp;
	// SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[0]);

	// Bind VI[0]~VI[5] and avs[0]
	for (i = 0; i < s32CamNum; i++) {
		stSrcChn.enModId = RK_ID_VI;
		stSrcChn.s32DevId = ctx->vi[i].s32DevId;
		stSrcChn.s32ChnId = ctx->vi[i].s32ChnId;
		stDestChn.enModId = RK_ID_AVS;
		stDestChn.s32DevId = ctx->avs.s32GrpId;
		stDestChn.s32ChnId = i;
		SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);
	}

	// Bind AVS[0] and VENC[0]
	stSrcChn.enModId = RK_ID_AVS;
	stSrcChn.s32DevId = ctx->avs.s32GrpId;
	stSrcChn.s32ChnId = ctx->avs.s32ChnId;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc.s32ChnId;
	SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

	printf("%s initial finish\n", __func__);

	while (!quit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);

	if (ctx->venc.getStreamCbFunc) {
		pthread_join(ctx->venc.getStreamThread, NULL);
	}

	// UnBind avs[0] and VENC[0]
	stSrcChn.enModId = RK_ID_AVS;
	stSrcChn.s32DevId = ctx->avs.s32GrpId;
	stSrcChn.s32ChnId = ctx->avs.s32ChnId;
	stDestChn.enModId = RK_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = ctx->venc.s32ChnId;
	SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);

	// UnBind Bind VI[0]~VI[5] and AVS[0]
	for (i = 0; i < s32CamNum; i++) {
		stSrcChn.enModId = RK_ID_VI;
		stSrcChn.s32DevId = ctx->vi[i].s32DevId;
		stSrcChn.s32ChnId = ctx->vi[i].s32ChnId;
		stDestChn.enModId = RK_ID_AVS;
		stDestChn.s32DevId = ctx->avs.s32GrpId;
		stDestChn.s32ChnId = i;
		SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
	}

	// Destroy RGN[0]
	// SAMPLE_COMM_RGN_DestroyChn(&ctx->rgn[0]);
	// Destroy VENC[0]
	SAMPLE_COMM_VENC_DestroyChn(&ctx->venc);
	// Destroy AVS[0]
	SAMPLE_COMM_AVS_DestroyChn(&ctx->avs);
	// Destroy VI[0]
	for (i = 0; i < s32CamNum; i++) {
		SAMPLE_COMM_VI_DestroyChn(&ctx->vi[i]);
	}

__FAILED:
	RK_MPI_SYS_Exit();
	if (iq_file_dir) {
#ifdef RKAIQ
		SAMPLE_COMM_ISP_CamGroup_Stop(s32CamId);
#endif
	}

	return 0;
}

/******************************************************************************
* function    : main()
* Description : main
******************************************************************************/
int main(int argc, char *argv[]) {
	RK_S32 s32Ret = RK_FAILURE;
	RK_S32 s32Index;
	struct sigaction sa;
	RK_CHAR *iq_file_dir = "/etc/iqfiles";
	RK_BOOL bMultictx = RK_FALSE;

	RK_S32 s32CamId = 0;
	SAMPLE_MPI_CTX_S ctx;

	if (argc < 2) {
		print_usage(argv[0]);
		return 0;
	}

	signal(SIGINT, sigterm_handler);

	int c;
	while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
		const char *tmp_optarg = optarg;
		switch (c) {
		case 'n':
			s32Index = atoi(optarg);
			break;
		case 'l':
			g_loopcount = atoi(optarg);
			break;
		case '?':
		default:
			print_usage(argv[0]);
			return 0;
		}
	}

	memset(&ctx, 0, sizeof(SAMPLE_MPI_CTX_S));

	while (g_loopcount) {
		switch (s32Index) {
		case 0:
			if (g_loopcount % 2 == 0) {
				s32Ret = SAMPLE_CAMERA_ISP_Stresstest(&ctx, "/etc/iqfiles");
			} else {
				s32Ret = SAMPLE_CAMERA_ISP_Stresstest(&ctx, "/usr/share/iqfiles");
			}
			break;
		case 1:
			s32Ret = SAMPLE_CAMERA_VENC_Stresstest(&ctx, "h265");
			g_loopcount = 0;
			break;
		case 2:
			s32Ret = SAMPLE_CAMERA_VI_AVS_VENC_Stresstest(&ctx);
			break;
		default:
			printf("the index %d is invaild!\n", s32Index);
			print_usage(argv[0]);
			return RK_FAILURE;
		}

		if (g_loopcount > 0) {
			g_loopcount--;
		} else {
			break;
		}
		printf("sample_camera_stresstest: g_loopcount(%d)\n", g_loopcount);

		sleep(2);
	}

	printf("%s finish!\n", __func__);

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
