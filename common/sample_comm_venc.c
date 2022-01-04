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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>

#include "sample_comm.h"

RK_S32 SAMPLE_COMM_VENC_CreateChn(SAMPLE_VENC_CTX_S *ctx) {
	RK_S32 s32Ret = RK_FAILURE;
	VENC_RECV_PIC_PARAM_S stRecvParam;

	switch (ctx->enCodecType) {
	case RK_CODEC_TYPE_H265:
		ctx->stChnAttr.stVencAttr.enType = RK_VIDEO_ID_HEVC;
		if (ctx->enRcMode == VENC_RC_MODE_H265CBR) {
			ctx->stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
			ctx->stChnAttr.stRcAttr.stH265Cbr.u32Gop = ctx->u32Gop;
			ctx->stChnAttr.stRcAttr.stH265Cbr.u32BitRate = ctx->u32BitRate;
			// frame rate: in u32Fps/1, out u32Fps/1.
			ctx->stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = ctx->u32Fps;
		} else if (ctx->enRcMode == VENC_RC_MODE_H265VBR) {
			ctx->stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
			ctx->stChnAttr.stRcAttr.stH265Vbr.u32Gop = ctx->u32Gop;
			ctx->stChnAttr.stRcAttr.stH265Vbr.u32BitRate = ctx->u32BitRate;
			// frame rate: in u32Fps/1, out u32Fps/1.
			ctx->stChnAttr.stRcAttr.stH265Vbr.fr32DstFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH265Vbr.fr32DstFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stH265Vbr.u32SrcFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH265Vbr.u32SrcFrameRateNum = ctx->u32Fps;
		}
		break;
	case RK_CODEC_TYPE_MJPEG:
		ctx->stChnAttr.stVencAttr.enType = RK_VIDEO_ID_MPEG4;
		if (ctx->enRcMode == VENC_RC_MODE_MJPEGCBR) {
			ctx->stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
			// frame rate: in u32Fps/1, out u32Fps/1.
			ctx->stChnAttr.stRcAttr.stMjpegCbr.fr32DstFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stMjpegCbr.fr32DstFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stMjpegCbr.u32SrcFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stMjpegCbr.u32SrcFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stMjpegCbr.u32BitRate =
			    ctx->u32Width * ctx->u32Height * 8;
		} else if (ctx->enRcMode == VENC_RC_MODE_MJPEGVBR) {
			ctx->stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
			// frame rate: in u32Fps/1, out u32Fps/1.
			ctx->stChnAttr.stRcAttr.stMjpegVbr.fr32DstFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stMjpegVbr.fr32DstFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stMjpegVbr.u32SrcFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stMjpegVbr.u32SrcFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stMjpegVbr.u32BitRate =
			    ctx->u32Width * ctx->u32Height * 8;
		}
		break;
	case RK_CODEC_TYPE_H264:
	default:
		ctx->stChnAttr.stVencAttr.enType = RK_VIDEO_ID_AVC;
		if (ctx->enRcMode == VENC_RC_MODE_H264CBR) {
			ctx->stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
			ctx->stChnAttr.stRcAttr.stH264Cbr.u32Gop = ctx->u32Gop;
			ctx->stChnAttr.stRcAttr.stH264Cbr.u32BitRate = ctx->u32BitRate;
			// frame rate: in u32Fps/1, out u32Fps/1.
			ctx->stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = ctx->u32Fps;
		} else if (ctx->enRcMode == VENC_RC_MODE_H264VBR) {
			ctx->stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
			ctx->stChnAttr.stRcAttr.stH264Vbr.u32Gop = ctx->u32Gop;
			ctx->stChnAttr.stRcAttr.stH264Vbr.u32BitRate = ctx->u32BitRate;
			// frame rate: in u32Fps/1, out u32Fps/1.
			ctx->stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateNum = ctx->u32Fps;
			ctx->stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateDen = 1;
			ctx->stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateNum = ctx->u32Fps;
		}
		break;
	}

	ctx->stChnAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
	ctx->stChnAttr.stVencAttr.u32PicWidth = ctx->u32Width;
	ctx->stChnAttr.stVencAttr.u32PicHeight = ctx->u32Height;
	ctx->stChnAttr.stVencAttr.u32VirWidth = ctx->u32Width;
	ctx->stChnAttr.stVencAttr.u32VirHeight = ctx->u32Height;
	ctx->stChnAttr.stVencAttr.u32StreamBufCnt = 5;
	ctx->stChnAttr.stVencAttr.u32BufSize = ctx->u32Width * ctx->u32Height * 3 / 2;

	s32Ret = RK_MPI_VENC_CreateChn(ctx->s32ChnId, &ctx->stChnAttr);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VENC_CreateChn failed %x", s32Ret);
		return s32Ret;
	}

	stRecvParam.s32RecvPicNum = -1;
	s32Ret = RK_MPI_VENC_StartRecvFrame(ctx->s32ChnId, &stRecvParam);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("create %d ch venc failed", ctx->s32ChnId);
		return s32Ret;
	}

	if (ctx->getStreamCbFunc) {
		ctx->stFrame.pstPack = (VENC_PACK_S *)(malloc(sizeof(VENC_PACK_S)));
		pthread_create(&ctx->getStreamThread, 0, ctx->getStreamCbFunc, (void *)(ctx));
	}

	return RK_SUCCESS;
}

RK_S32 SAMPLE_COMM_VENC_GetStream(SAMPLE_VENC_CTX_S *ctx, void **pdata) {
	RK_S32 s32Ret = RK_FAILURE;

	s32Ret = RK_MPI_VENC_GetStream(ctx->s32ChnId, &ctx->stFrame, -1);
	if (s32Ret == RK_SUCCESS) {
		*pdata = RK_MPI_MB_Handle2VirAddr(ctx->stFrame.pstPack->pMbBlk);
	} else {
		RK_LOGE("RK_MPI_VENC_GetStream fail %x", s32Ret);
	}

	return s32Ret;
}

RK_S32 SAMPLE_COMM_VENC_ReleaseStream(SAMPLE_VENC_CTX_S *ctx) {
	RK_S32 s32Ret = RK_FAILURE;

	s32Ret = RK_MPI_VENC_ReleaseStream(ctx->s32ChnId, &ctx->stFrame);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
	}

	return s32Ret;
}

RK_S32 SAMPLE_COMM_VENC_DestroyChn(SAMPLE_VENC_CTX_S *ctx) {
	RK_S32 s32Ret = RK_FAILURE;

	s32Ret = RK_MPI_VENC_StopRecvFrame(ctx->s32ChnId);
	if (s32Ret != RK_SUCCESS) {
		return s32Ret;
	}
	RK_LOGE("destroy enc chn:%d", ctx->s32ChnId);
	s32Ret = RK_MPI_VENC_DestroyChn(ctx->s32ChnId);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VDEC_DestroyChn fail %x", s32Ret);
	}

	if (ctx->stFrame.pstPack) {
		free(ctx->stFrame.pstPack);
	}

	return RK_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
