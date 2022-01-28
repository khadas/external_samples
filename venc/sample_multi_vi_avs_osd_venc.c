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

#include "rtsp_demo.h"
#include "sample_comm.h"

rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session;

typedef struct _rkMpiCtx {
	SAMPLE_VI_CTX_S vi[6];
	SAMPLE_AVS_CTX_S avs;
	SAMPLE_VENC_CTX_S venc;
	SAMPLE_RGN_CTX_S rgn[2];
	SAMPLE_VPSS_CTX_S vpss;
	SAMPLE_VO_CTX_S vo;
} SAMPLE_MPI_CTX_S;

static bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	quit = true;
}

static RK_CHAR optstr[] = "?::a::A:b:w:h:l:o:e:d:D:I:i:L:M:";
static const struct option long_options[] = {
    {"aiq", optional_argument, NULL, 'a'},
    {"avs_lut_name", required_argument, NULL, 'A'},
    {"bitrate", required_argument, NULL, 'b'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"input_bmp_name", required_argument, NULL, 'i'},
    {"loop_count", required_argument, NULL, 'l'},
    {"output_path", required_argument, NULL, 'o'},
    {"encode", required_argument, NULL, 'e'},
    {"multictx", required_argument, NULL, 'M'},
    {"fps", required_argument, NULL, 'f'},
    {"hdr_mode", required_argument, NULL, 'h' + 'm'},
    {"help", optional_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

/******************************************************************************
* function : show usage
******************************************************************************/
static void print_usage(const RK_CHAR *name) {
	printf("usage example:\n");
	printf("\t%s -w 2560 -h 1520 -a /etc/iqfiles/ -e h265cbr -b 4096 -i "
	       "/usr/share/image.bmp -o /data/\n",
	       name);
	printf("\trtsp://xx.xx.xx.xx/live/0, Default OPEN\n");
#ifdef RKAIQ
	printf("\t-a | --aiq: enable aiq with dirpath provided, eg:-a "
	       "/etc/iqfiles/, "
	       "set dirpath empty to using path by default, without this option aiq "
	       "should run in other application\n");
	printf("\t-M | --multictx: switch of multictx in isp, set 0 to disable, set "
	       "1 to enable. Default: 0\n");
#endif
	printf("\t-A | --calib_file_path: input file path of calib_file_xxx.pto "
	       "Default /usr/share/avs_calib/calib_file_pos.pto\n");
	printf("\t-w | --width: camera with, Default 1920\n");
	printf("\t-h | --height: camera height, Default 1080\n");
	printf("\t-e | --encode: encode type, Default:h264cbr, Value:h264cbr, "
	       "h264vbr, h264avbr "
	       "h265cbr, h265vbr, h265avbr, mjpegcbr, mjpegvbr\n");
	printf("\t-b | --bitrate: encode bitrate, Default 4096\n");
	printf("\t-i | --input_bmp_name: input file path of logo.bmp, Default NULL\n");
	printf("\t-l | --loop_count: loop count, Default -1\n");
	printf("\t-o | --output_path: encode output file path, Default NULL\n");
	printf("\t-D | --disp devid: display DevId, Default -1\n");
	printf("\t-L | --disp layerid: display LayerId, Default 0\n");
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
			rtsp_tx_video(g_rtsp_session, pData, ctx->stFrame.pstPack->u32Len,
			              ctx->stFrame.pstPack->u64PTS);
			rtsp_do_event(g_rtsplive);

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
* function    : main()
* Description : main
******************************************************************************/
int main(int argc, char *argv[]) {
	RK_S32 s32Ret = RK_FAILURE;
	SAMPLE_MPI_CTX_S *ctx;
	int video_width = 2560;
	int video_height = 1520;
	int avs_width = 8192;
	int avs_height = 2700;
	int venc_width = 8192;
	int venc_height = 2700;
	int disp_width = 0;
	int disp_height = 0;
	RK_CHAR *pAvsCalibFilePath = "/usr/share/avs_calib/calib_file_pos.pto";
	RK_CHAR *pAvsMeshAlphaPath = "/usr/share/test/";
	RK_CHAR *pAvsLutFilePath = NULL;
	RK_CHAR *pInPathBmp = NULL;
	RK_CHAR *pOutPathVenc = NULL;
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
	VENC_RC_MODE_E enRcMode = VENC_RC_MODE_H264CBR;
	RK_CHAR *pCodecName = "H264";
	RK_S32 s32CamId = 0;
	RK_S32 s32DisId = -1;
	RK_S32 s32DisLayerId = 0;
	MPP_CHN_S stSrcChn, stDestChn;
	RK_S32 s32CamNum = 6;
	RK_S32 s32loopCnt = -1;
	RK_S32 s32BitRate = 4 * 1024;
	RK_S32 i;

	if (argc < 2) {
		print_usage(argv[0]);
		return 0;
	}

	ctx = (SAMPLE_MPI_CTX_S *)(malloc(sizeof(SAMPLE_MPI_CTX_S)));
	memset(ctx, 0, sizeof(SAMPLE_MPI_CTX_S));

	signal(SIGINT, sigterm_handler);

#ifdef RKAIQ
	RK_BOOL bMultictx = RK_FALSE;
#endif
	int c;
	char *iq_file_dir = NULL;
	while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
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
		case 'A':
			// pAvsLutFilePath = optarg;
			pAvsCalibFilePath = optarg;
			break;
		case 'b':
			s32BitRate = atoi(optarg);
			break;
		case 'D':
			s32DisId = atoi(optarg);
			break;
		case 'e':
			if (!strcmp(optarg, "h264cbr")) {
				enCodecType = RK_CODEC_TYPE_H264;
				enRcMode = VENC_RC_MODE_H264CBR;
				pCodecName = "H264";
			} else if (!strcmp(optarg, "h264vbr")) {
				enCodecType = RK_CODEC_TYPE_H264;
				enRcMode = VENC_RC_MODE_H264VBR;
				pCodecName = "H264";
			} else if (!strcmp(optarg, "h264avbr")) {
				enCodecType = RK_CODEC_TYPE_H264;
				enRcMode = VENC_RC_MODE_H264AVBR;
				pCodecName = "H264";
			} else if (!strcmp(optarg, "h265cbr")) {
				enCodecType = RK_CODEC_TYPE_H265;
				enRcMode = VENC_RC_MODE_H265CBR;
				pCodecName = "H265";
			} else if (!strcmp(optarg, "h265vbr")) {
				enCodecType = RK_CODEC_TYPE_H265;
				enRcMode = VENC_RC_MODE_H265VBR;
				pCodecName = "H265";
			} else if (!strcmp(optarg, "h265avbr")) {
				enCodecType = RK_CODEC_TYPE_H265;
				enRcMode = VENC_RC_MODE_H265AVBR;
				pCodecName = "H265";
			} else if (!strcmp(optarg, "mjpegcbr")) {
				enCodecType = RK_CODEC_TYPE_MJPEG;
				enRcMode = VENC_RC_MODE_MJPEGCBR;
				pCodecName = "MJPEG";
			} else if (!strcmp(optarg, "mjpegvbr")) {
				enCodecType = RK_CODEC_TYPE_MJPEG;
				enRcMode = VENC_RC_MODE_MJPEGVBR;
				pCodecName = "MJPEG";
			} else {
				printf("ERROR: Invalid encoder type.\n");
				return 0;
			}
			break;
		case 'w':
			video_width = atoi(optarg);
			break;
		case 'h':
			video_height = atoi(optarg);
			break;
		case 'i':
			pInPathBmp = optarg;
			break;
		case 'l':
			s32loopCnt = atoi(optarg);
			break;
		case 'L':
			s32DisLayerId = atoi(optarg);
			break;
		case 'o':
			pOutPathVenc = optarg;
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

	printf("#CameraIdx: %d\n", s32CamId);
	printf("#pAvsLutFilePath: %s\n", pAvsLutFilePath);
	printf("#pAvsCalibFilePath: %s\n", pAvsCalibFilePath);
	printf("#pAvsMeshAlphaPath: %s\n", pAvsMeshAlphaPath);
	printf("#CodecName:%s\n", pCodecName);
	printf("#Output Path: %s\n", pOutPathVenc);
	printf("#IQ Path: %s\n", iq_file_dir);
	if (iq_file_dir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
		printf("#bMultictx: %d\n\n", bMultictx);
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
		rk_aiq_camgroup_instance_cfg_t camgroup_cfg;

		memset(&camgroup_cfg, 0, sizeof(camgroup_cfg));
		camgroup_cfg.sns_num = s32CamNum;
		camgroup_cfg.config_file_dir = iq_file_dir;

		SAMPLE_COMM_ISP_CamGroup_Init(s32CamId, hdr_mode, bMultictx, &camgroup_cfg);
#endif
	}

	// init rtsp
	g_rtsplive = create_rtsp_demo(554);
	g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/0");
	if (enCodecType == RK_CODEC_TYPE_H264) {
		rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
	} else if (enCodecType == RK_CODEC_TYPE_H265) {
		rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H265, NULL, 0);
	} else {
		printf("not support other type\n");
		return -1;
	}
	rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime());

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
	ctx->avs.stAvsGrpAttr.stOutAttr.stCenter.s32X = 4196;
	ctx->avs.stAvsGrpAttr.stOutAttr.stCenter.s32Y = 2080;
	ctx->avs.stAvsGrpAttr.stOutAttr.stFOV.u32FOVX = 28000;
	ctx->avs.stAvsGrpAttr.stOutAttr.stFOV.u32FOVY = 9500;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Roll = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Pitch = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stORIRotation.s32Yaw = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Roll = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Pitch = 0;
	ctx->avs.stAvsGrpAttr.stOutAttr.stRotation.s32Yaw = 0;
	ctx->avs.stAvsGrpAttr.stLUT.enAccuracy = AVS_LUT_ACCURACY_HIGH;
	ctx->avs.stAvsGrpAttr.bSyncPipe = RK_TRUE;
	ctx->avs.stAvsGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	ctx->avs.stAvsGrpAttr.stFrameRate.s32DstFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].enCompressMode = COMPRESS_AFBC_16x16;
	ctx->avs.stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
	ctx->avs.stAvsChnAttr[0].u32Depth = 1;
	ctx->avs.stAvsChnAttr[0].u32Width = avs_width;
	ctx->avs.stAvsChnAttr[0].u32Height = avs_height;
	ctx->avs.stAvsChnAttr[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
	if (pAvsCalibFilePath) {
		strcpy(ctx->avs.stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath, pAvsCalibFilePath);
	}
	if (pAvsMeshAlphaPath) {
		strcpy(ctx->avs.stAvsGrpAttr.stOutAttr.stCalib.aMeshAlphaPath, pAvsMeshAlphaPath);
	}
	if (pAvsLutFilePath) {
		strcpy(ctx->avs.stAvsGrpAttr.stLUT.aFilePath, pAvsLutFilePath);
	}
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
	ctx->rgn[0].rgnHandle = 0;
	ctx->rgn[0].stRgnAttr.enType = COVER_RGN;
	ctx->rgn[0].stMppChn.enModId = RK_ID_VENC;
	ctx->rgn[0].stMppChn.s32ChnId = 0;
	ctx->rgn[0].stMppChn.s32DevId = ctx->venc.s32ChnId;
	ctx->rgn[0].stRegion.s32X = 1328;    // must be 16 aligned
	ctx->rgn[0].stRegion.s32Y = 976;     // must be 16 aligned
	ctx->rgn[0].stRegion.u32Width = 576; // must be 16 aligned
	ctx->rgn[0].stRegion.u32Height = 96; // must be 16 aligned
	ctx->rgn[0].u32Color = 0x00f800;
	ctx->rgn[0].u32Layer = 1;
	// SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[0]);

	// Init RGN[1]
	ctx->rgn[1].rgnHandle = 1;
	ctx->rgn[1].stRgnAttr.enType = OVERLAY_RGN;
	ctx->rgn[1].stMppChn.enModId = RK_ID_VENC;
	ctx->rgn[1].stMppChn.s32ChnId = 0;
	ctx->rgn[1].stMppChn.s32DevId = ctx->venc.s32ChnId;
	ctx->rgn[1].stRegion.s32X = 0;        // must be 16 aligned
	ctx->rgn[1].stRegion.s32Y = 0;        // must be 16 aligned
	ctx->rgn[1].stRegion.u32Width = 576;  // must be 16 aligned
	ctx->rgn[1].stRegion.u32Height = 288; // must be 16 aligned
	ctx->rgn[1].u32BmpFormat = RK_FMT_BGRA5551;
	ctx->rgn[1].u32BgAlpha = 128;
	ctx->rgn[1].u32FgAlpha = 128;
	ctx->rgn[1].u32Layer = 1;
	ctx->rgn[1].srcFileBmpName = pInPathBmp;
	SAMPLE_COMM_RGN_CreateChn(&ctx->rgn[1]);

	if (s32DisId >= 0) {
		// Init VPSS[0]
		ctx->vpss.s32GrpId = 0; // 0~85:gpu, 86~169:rga, 170~255:isp
		ctx->vpss.s32ChnId = 0;
		// RGA_device: VIDEO_PROC_DEV_RGA GPU_device: VIDEO_PROC_DEV_GPU
		ctx->vpss.enVProcDevType = VIDEO_PROC_DEV_RGA;
		ctx->vpss.stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;
		ctx->vpss.stGrpVpssAttr.enCompressMode = COMPRESS_MODE_NONE; // no compress
		ctx->vpss.s32ChnRotation[0] = ROTATION_0;
		ctx->vpss.stVpssChnAttr[0].enChnMode = VPSS_CHN_MODE_USER;
		ctx->vpss.stVpssChnAttr[0].enCompressMode = COMPRESS_MODE_NONE;
		ctx->vpss.stVpssChnAttr[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
		ctx->vpss.stVpssChnAttr[0].enPixelFormat = RK_FMT_YUV420SP;
		ctx->vpss.stVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
		ctx->vpss.stVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
		ctx->vpss.stVpssChnAttr[0].u32Width = avs_width;
		ctx->vpss.stVpssChnAttr[0].u32Height = avs_height;
		if (s32DisId == 3) { // MIPI
			ctx->vpss.s32ChnRotation[0] = ROTATION_90;
			ctx->vpss.stVpssChnAttr[0].u32Width = avs_height;
			ctx->vpss.stVpssChnAttr[0].u32Height = avs_width;
		}
		SAMPLE_COMM_VPSS_CreateChn(&ctx->vpss);

		// Init VO[0]
		ctx->vo.s32DevId = s32DisId;
		ctx->vo.s32ChnId = 0;
		ctx->vo.s32LayerId = s32DisLayerId;
		ctx->vo.Volayer_mode = VO_LAYER_MODE_GRAPHIC;
		ctx->vo.u32DispBufLen = 3;
		ctx->vo.stVoPubAttr.enIntfType = VO_INTF_MIPI;
		ctx->vo.stVoPubAttr.enIntfSync = VO_OUTPUT_DEFAULT;
		ctx->vo.stLayerAttr.stDispRect.s32X = 0;
		ctx->vo.stLayerAttr.stDispRect.s32Y = 0;
		ctx->vo.stLayerAttr.stDispRect.u32Width = disp_width;
		ctx->vo.stLayerAttr.stDispRect.u32Height = disp_height;
		ctx->vo.stLayerAttr.stImageSize.u32Width = disp_width;
		ctx->vo.stLayerAttr.stImageSize.u32Height = disp_height;
		ctx->vo.stLayerAttr.u32DispFrmRt = 30;
		ctx->vo.stLayerAttr.enPixFormat = RK_FMT_RGB888;
		ctx->vo.stLayerAttr.bDoubleFrame = RK_FALSE;
		ctx->vo.stChnAttr.stRect.s32X = 0;
		ctx->vo.stChnAttr.stRect.s32Y = 0;
		ctx->vo.stChnAttr.stRect.u32Width = disp_width;
		ctx->vo.stChnAttr.stRect.u32Height = disp_height;
		ctx->vo.stChnAttr.u32Priority = 1;
		if (s32DisId == 3) { // MIPI
			ctx->vo.stVoPubAttr.enIntfType = VO_INTF_MIPI;
			ctx->vo.stVoPubAttr.enIntfSync = VO_OUTPUT_DEFAULT;
		} else {
			ctx->vo.stVoPubAttr.enIntfType = VO_INTF_HDMI;
			ctx->vo.stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
		}
		SAMPLE_COMM_VO_CreateChn(&ctx->vo);
	}

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

	if (s32DisId >= 0) {
		// Bind AVS[0] and VPSS[0]
		stSrcChn.enModId = RK_ID_AVS;
		stSrcChn.s32DevId = ctx->avs.s32GrpId;
		stSrcChn.s32ChnId = ctx->avs.s32ChnId;
		stDestChn.enModId = RK_ID_VPSS;
		stDestChn.s32DevId = ctx->vpss.s32GrpId;
		stDestChn.s32ChnId = ctx->vpss.s32ChnId;
		SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);

		// Bind VPSS[0] and VO[0]
		stSrcChn.enModId = RK_ID_VPSS;
		stSrcChn.s32DevId = ctx->vpss.s32GrpId;
		stSrcChn.s32ChnId = ctx->vpss.s32ChnId;
		stDestChn.enModId = RK_ID_VO;
		stDestChn.s32DevId = ctx->vo.s32LayerId;
		stDestChn.s32ChnId = ctx->vo.s32ChnId;
		SAMPLE_COMM_Bind(&stSrcChn, &stDestChn);
	}

	printf("%s initial finish\n", __func__);

	while (!quit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);

	if (ctx->venc.getStreamCbFunc) {
		pthread_join(ctx->venc.getStreamThread, NULL);
	}

	if (g_rtsplive)
		rtsp_del_demo(g_rtsplive);

	if (s32DisId >= 0) {
		// UnBind VPSS[0] and VO[0]
		stSrcChn.enModId = RK_ID_VPSS;
		stSrcChn.s32DevId = ctx->vpss.s32GrpId;
		stSrcChn.s32ChnId = ctx->vpss.s32ChnId;
		stDestChn.enModId = RK_ID_VO;
		stDestChn.s32DevId = ctx->vo.s32LayerId;
		stDestChn.s32ChnId = ctx->vo.s32ChnId;
		SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);

		// UnBind AVS[0] and VPSS[0]
		stSrcChn.enModId = RK_ID_AVS;
		stSrcChn.s32DevId = ctx->avs.s32GrpId;
		stSrcChn.s32ChnId = ctx->avs.s32ChnId;
		stDestChn.enModId = RK_ID_VPSS;
		stDestChn.s32DevId = ctx->vpss.s32GrpId;
		stDestChn.s32ChnId = ctx->vpss.s32ChnId;
		SAMPLE_COMM_UnBind(&stSrcChn, &stDestChn);
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

	if (s32DisId >= 0) {
		// Destroy VO[0]
		SAMPLE_COMM_VO_DestroyChn(&ctx->vo);
		// Destroy VPSS[0]
		SAMPLE_COMM_VPSS_DestroyChn(&ctx->vpss);
	}
	// Destroy RGN[1]
	SAMPLE_COMM_RGN_DestroyChn(&ctx->rgn[1]);
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
