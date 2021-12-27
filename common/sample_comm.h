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
#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "loadbmp.h"
#include "rk_debug.h"
#include "rk_defines.h"
#include "rk_mpi_avs.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_rgn.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_tde.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_vpss.h"
#include "sample_comm_isp.h"

/*******************************************************
    macro define
*******************************************************/
#define RKAIQ
typedef void *(*Thread_Func)(void *param);

/*******************************************************
    enum define
*******************************************************/
typedef enum rk_CODEC_TYPE_E {
	RK_CODEC_TYPE_NONE = -1,
	// Audio
	RK_CODEC_TYPE_MP3,
	RK_CODEC_TYPE_MP2,
	RK_CODEC_TYPE_G711A,
	RK_CODEC_TYPE_G711U,
	RK_CODEC_TYPE_G726,
	// Video
	RK_CODEC_TYPE_H264,
	RK_CODEC_TYPE_H265,
	RK_CODEC_TYPE_JPEG,
	RK_CODEC_TYPE_MJPEG,
	RK_CODEC_TYPE_NB
} CODEC_TYPE_E;

/*******************************************************
    structure define
*******************************************************/
typedef struct _rkMpiVICtx {
	RK_U32 u32Width;
	RK_U32 u32Height;
	RK_S32 s32loopCount;
	VI_DEV s32DevId;
	VI_PIPE u32PipeId;
	VI_CHN s32ChnId;
	VI_DEV_ATTR_S stDevAttr;
	VI_DEV_BIND_PIPE_S stBindPipe;
	VI_CHN_ATTR_S stChnAttr;
	VI_SAVE_FILE_INFO_S stDebugFile;
	VI_FRAME_S stViFrame;
	VI_CHN_STATUS_S stChnStatus;
	RK_CHAR *dstFilePath;
} SAMPLE_VI_CTX_S;

typedef struct _rkMpiVOCtx {
	RK_U32 u32DispBufLen;
	VO_CHN s32ChnId;
	VO_DEV s32DevId;
	VO_LAYER s32LayerId;
	VO_LAYER_MODE_E Volayer_mode;
	VO_PUB_ATTR_S stVoPubAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
} SAMPLE_VO_CTX_S;

typedef struct _rkMpiVENCCtx {
	RK_U32 u32Width;
	RK_U32 u32Height;
	RK_U32 u32Fps;
	RK_U32 u32Gop;
	RK_U32 u32BitRate;
	RK_S32 s32loopCount;
	CODEC_TYPE_E enCodecType;
	VENC_RC_MODE_E enRcMode;
	VENC_CHN s32ChnId;
	VENC_CHN_ATTR_S stChnAttr;
	VENC_STREAM_S stFrame;
	pthread_t getStreamThread;
	Thread_Func getStreamCbFunc;
	RK_CHAR *srcFilePath;
	RK_CHAR *dstFilePath;
} SAMPLE_VENC_CTX_S;

typedef struct _rkMpiRGNCtx {
	const char *srcFileBmpName;
	RK_U32 u32BmpFormat;
	RK_U32 u32BgAlpha;
	RK_U32 u32FgAlpha;
	BITMAP_S stBitmap;
	MPP_CHN_S stMppChn;
	RECT_S stRegion;
	RK_U32 u32Color;
	RK_U32 u32Layer;
	RGN_HANDLE rgnHandle;
	RGN_ATTR_S stRgnAttr;
	RGN_CHN_ATTR_S stRgnChnAttr;
} SAMPLE_RGN_CTX_S;

typedef struct _rkMpiVPSSCtx {
	VPSS_GRP s32GrpId;
	VPSS_CHN s32ChnId;
	RK_S32 s32RotationEx;
	RK_S32 s32GrpCropRatio;
	RK_S32 s32ChnCropRatio;
	VPSS_GRP_ATTR_S stGrpVpssAttr;
	VPSS_CROP_INFO_S stCropInfo;
	RK_S32 s32ChnRotation[VPSS_MAX_CHN_NUM];
	VPSS_CROP_INFO_S stChnCropInfo[VPSS_MAX_CHN_NUM];
	VPSS_ROTATION_EX_ATTR_S stRotationEx[VPSS_MAX_CHN_NUM];
	VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_CHN_NUM];
} SAMPLE_VPSS_CTX_S;

typedef struct _rkMpiAVSCtx {
	RK_S32 s32loopCount;
	AVS_GRP s32GrpId;
	AVS_CHN s32ChnId;
	AVS_MOD_PARAM_S stAvsModParam;
	AVS_GRP_ATTR_S stAvsGrpAttr;
	AVS_OUTPUT_ATTR_S stAvsOutAttr;
	AVS_CHN_ATTR_S stAvsChnAttr[AVS_MAX_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	RK_CHAR *dstFilePath;
} SAMPLE_AVS_CTX_S;

/*******************************************************
    function announce
*******************************************************/
void PrintStreamDetails(int chnId, int framesize);
void SIGSEGV_handler(int signum, siginfo_t *sig_info, void *context);

RK_S32 SAMPLE_COMM_VI_CreateChn(SAMPLE_VI_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VI_DestroyChn(SAMPLE_VI_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VI_GetChnFrame(SAMPLE_VI_CTX_S *ctx, void **pdata);
RK_S32 SAMPLE_COMM_VI_ReleaseChnFrame(SAMPLE_VI_CTX_S *ctx);

RK_S32 SAMPLE_COMM_VO_CreateChn(SAMPLE_VO_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VO_DestroyChn(SAMPLE_VO_CTX_S *ctx);

RK_S32 SAMPLE_COMM_VPSS_CreateChn(SAMPLE_VPSS_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VPSS_DestroyChn(SAMPLE_VPSS_CTX_S *ctx);

RK_S32 SAMPLE_COMM_AVS_CreateChn(SAMPLE_AVS_CTX_S *ctx);
RK_S32 SAMPLE_COMM_AVS_DestroyChn(SAMPLE_AVS_CTX_S *ctx);
RK_S32 SAMPLE_COMM_AVS_GetChnFrame(SAMPLE_AVS_CTX_S *ctx, void **pdata);
RK_S32 SAMPLE_COMM_AVS_ReleaseChnFrame(SAMPLE_AVS_CTX_S *ctx);

RK_S32 SAMPLE_COMM_VO_CreateChn(SAMPLE_VO_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VO_DestroyChn(SAMPLE_VO_CTX_S *ctx);

RK_S32 SAMPLE_COMM_VENC_CreateChn(SAMPLE_VENC_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VENC_GetStream(SAMPLE_VENC_CTX_S *ctx, void **pdata);
RK_S32 SAMPLE_COMM_VENC_ReleaseStream(SAMPLE_VENC_CTX_S *ctx);
RK_S32 SAMPLE_COMM_VENC_DestroyChn(SAMPLE_VENC_CTX_S *ctx);

RK_S32 SAMPLE_COMM_RGN_CreateChn(SAMPLE_RGN_CTX_S *ctx);
RK_S32 SAMPLE_COMM_RGN_DestroyChn(SAMPLE_RGN_CTX_S *ctx);

RK_S32 SAMPLE_COMM_Bind(const MPP_CHN_S *pstSrcChn, const MPP_CHN_S *pstDestChn);
RK_S32 SAMPLE_COMM_UnBind(const MPP_CHN_S *pstSrcChn, const MPP_CHN_S *pstDestChn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __SAMPLE_COMMON_H__ */
