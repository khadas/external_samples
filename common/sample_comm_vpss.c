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

RK_S32 SAMPLE_COMM_VPSS_CreateChn(SAMPLE_VPSS_CTX_S *ctx) {
	RK_S32 chnIndex;
	RK_S32 s32Ret = RK_SUCCESS;
	ROTATION_E rotation = ROTATION_0;

	if (ctx->s32GrpId >= VPSS_MAX_GRP_NUM) {
		RK_LOGE("s32GrpId is less than the maximum channel: %d", VPSS_MAX_GRP_NUM);
		return RK_FAILURE;
	}

	ctx->stGrpVpssAttr.u32MaxW = 4096;
	ctx->stGrpVpssAttr.u32MaxH = 4096;
	ctx->stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;
	ctx->stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;
	s32Ret = RK_MPI_VPSS_CreateGrp(ctx->s32GrpId, &ctx->stGrpVpssAttr);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = RK_MPI_VPSS_ResetGrp(ctx->s32GrpId);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_ResetGrp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = RK_MPI_VPSS_SetGrpCrop(ctx->s32GrpId, &ctx->stCropInfo);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	s32Ret = RK_MPI_VPSS_GetGrpCrop(ctx->s32GrpId, &ctx->stCropInfo);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_GetGrpCrop failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	for (chnIndex = 0; chnIndex < VPSS_MAX_CHN_NUM; chnIndex++) {
		if (ctx->stVpssChnAttr[chnIndex].u32Width &&
		    ctx->stVpssChnAttr[chnIndex].u32Height) {

			s32Ret = RK_MPI_VPSS_SetChnCrop(ctx->s32GrpId, chnIndex,
			                                &ctx->stChnCropInfo[chnIndex]);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_SetChnCrop failed with %#x!\n", s32Ret);
				return s32Ret;
			}
			s32Ret = RK_MPI_VPSS_GetChnCrop(ctx->s32GrpId, chnIndex,
			                                &ctx->stChnCropInfo[chnIndex]);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_GetChnCrop failed with %#x!\n", s32Ret);
				return s32Ret;
			}

			s32Ret = RK_MPI_VPSS_SetChnRotation(ctx->s32GrpId, chnIndex,
			                                    (ROTATION_E)ctx->s32ChnRotation);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_SetChnRotation failed with %#x!\n", s32Ret);
				return s32Ret;
			}
			s32Ret = RK_MPI_VPSS_GetChnRotation(ctx->s32GrpId, chnIndex, &rotation);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_GetChnRotation failed with %#x!\n", s32Ret);
				return s32Ret;
			}
			if (rotation != ctx->s32ChnRotation) {
				s32Ret = RK_FAILURE;
				return s32Ret;
			}
#if 0   
            VPSS_ROTATION_EX_ATTR_S stRotationEx;
            stRotationEx.stRotationEx.u32Angle = ctx->stRotationEx[chnIndex].stRotationEx.u32Angle;
            s32Ret = RK_MPI_VPSS_SetChnRotationEx(ctx->s32GrpId, chnIndex, &stRotationEx);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("RK_MPI_VPSS_SetChnRotationEx failed with %#x!\n", s32Ret);
                return s32Ret;
            }
            s32Ret = RK_MPI_VPSS_GetChnRotationEx(ctx->s32GrpId, chnIndex, &stRotationEx);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("RK_MPI_VPSS_GetChnRotationEx failed with %#x!\n", s32Ret);
                return s32Ret;
            }
            if (ctx->stRotationEx[chnIndex].stRotationEx.u32Angle != 
                    stRotationEx.stRotationEx.u32Angle) {
                s32Ret = RK_FAILURE;
                RK_LOGE("Set Angle failed with %#x!\n", s32Ret);
                return s32Ret;
            }
#endif
			s32Ret = RK_MPI_VPSS_SetChnAttr(ctx->s32GrpId, chnIndex,
			                                &ctx->stVpssChnAttr[chnIndex]);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_SetChnAttr failed with %#x!\n", s32Ret);
				return s32Ret;
			}
			s32Ret = RK_MPI_VPSS_GetChnAttr(ctx->s32GrpId, chnIndex,
			                                &ctx->stVpssChnAttr[chnIndex]);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_GetChnAttr failed with %#x!\n", s32Ret);
				return s32Ret;
			}
			s32Ret = RK_MPI_VPSS_EnableChn(ctx->s32GrpId, chnIndex);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_EnableChn failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	s32Ret = RK_MPI_VPSS_StartGrp(ctx->s32GrpId);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_StartGrp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return RK_SUCCESS;
}

RK_S32 SAMPLE_COMM_VPSS_DestroyChn(SAMPLE_VPSS_CTX_S *ctx) {
	RK_S32 chnIndex;
	RK_S32 s32Ret = RK_FAILURE;

	for (chnIndex = 0; chnIndex < VPSS_MAX_CHN_NUM; chnIndex++) {
		if (ctx->stVpssChnAttr[chnIndex].u32Width &&
		    ctx->stVpssChnAttr[chnIndex].u32Height) {

			s32Ret = RK_MPI_VPSS_DisableChn(ctx->s32GrpId, chnIndex);
			if (s32Ret != RK_SUCCESS) {
				RK_LOGE("RK_MPI_VPSS_DisableChn failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	s32Ret = RK_MPI_VPSS_StopGrp(ctx->s32GrpId);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_StopGrp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = RK_MPI_VPSS_DestroyGrp(ctx->s32GrpId);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("RK_MPI_VPSS_DestroyGrp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return RK_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
