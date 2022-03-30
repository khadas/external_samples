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

typedef struct _rkMpiCtx { SAMPLE_VI_CTX_S vi; } SAMPLE_MPI_CTX_S;

static bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	quit = true;
}

static RK_CHAR optstr[] = "?::a::d:f:w:h:c:o:e:n:I:i:l:M:";
static const struct option long_options[] = {
    {"aiq", optional_argument, NULL, 'a'},
    {"pixel_format", optional_argument, NULL, 'f'},
    {"device_name", required_argument, NULL, 'd'},
    {"camera_num", required_argument, NULL, 'n'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"loop_count", required_argument, NULL, 'l'},
    {"output_path", required_argument, NULL, 'o'},
    {"camid", required_argument, NULL, 'I'},
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
	printf("\t%s -w 2560 -h 1520 -a /etc/iqfiles/ -l 10 -o /data/\n", name);
#ifdef RKAIQ
	printf("\t-a | --aiq: enable aiq with dirpath provided, eg:-a /etc/iqfiles/, "
	       "set dirpath empty to using path by default, without this option aiq "
	       "should run in other application\n");
	printf("\t-M | --multictx: switch of multictx in isp, set 0 to disable, set "
	       "1 to enable. Default: 0\n");
#endif
	printf("\t-d | --device_name: set pcDeviceName, eg: /dev/video0 Default "
	       "NULL\n");
	printf("\t-I | --camid: camera ctx id, Default 0\n");
	printf("\t-n | --camera_num: camera number, Default 6\n");
	printf("\t-w | --width: camera with, Default 1920\n");
	printf("\t-h | --height: camera height, Default 1080\n");
	printf("\t-f | --pixel_format: camera Format, Default nv12, "
	       "Value:nv12,nv16,yuyv,uyvy,afbc\n");
	printf("\t-l | --loop_count: loop count, Default -1\n");
	printf("\t-o | --output_path: vi output file path, Default NULL\n");
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
			if (ctx->stViFrame.stVFrame.u64PrivateData <= 0) {
			    //SAMPLE_COMM_VI_ReleaseChnFrame(ctx);
				//continue;
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
				fwrite(pData, 1, ctx->stViFrame.stVFrame.u64PrivateData, fp);
				fflush(fp);
			}

			printf(
			    "SAMPLE_COMM_VI_GetChnFrame DevId %d ok:data %p size:%d loop:%d seq:%d "
			    "pts:%lld ms\n",
			    ctx->s32DevId, pData, ctx->stViFrame.stVFrame.u64PrivateData, loopCount,
			    ctx->stViFrame.stVFrame.u32TimeRef,
			    ctx->stViFrame.stVFrame.u64PTS / 1000);

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
* function    : main()
* Description : main
******************************************************************************/
int main(int argc, char *argv[]) {
	RK_S32 s32Ret = RK_FAILURE;
	SAMPLE_MPI_CTX_S *ctx;
	int video_width = 1920;
	int video_height = 1080;
	RK_CHAR *pOutPath = NULL;
	RK_CHAR *pDeviceName = NULL;
	RK_S32 i;
	RK_S32 s32CamId = 0;
	RK_S32 s32ChnId = 0;
	RK_S32 s32loopCnt = -1;
	PIXEL_FORMAT_E PixelFormat = RK_FMT_YUV420SP;
	COMPRESS_MODE_E CompressMode = COMPRESS_MODE_NONE;
	MPP_CHN_S stSrcChn, stDestChn;
	pthread_t vi_thread_id;

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
		case 'd':
			pDeviceName = optarg;
			break;
		case 'f':
			if (!strcmp(optarg, "nv12")) {
				s32ChnId = 0;
				PixelFormat = RK_FMT_YUV420SP;
				CompressMode = COMPRESS_MODE_NONE;
			} else if (!strcmp(optarg, "nv16")) {
				s32ChnId = 0;
				PixelFormat = RK_FMT_YUV422SP;
				CompressMode = COMPRESS_MODE_NONE;
			} else if (!strcmp(optarg, "yuyv")) {
				s32ChnId = 0;
				PixelFormat = RK_FMT_YUV422_YUYV;
				CompressMode = COMPRESS_MODE_NONE;
			} else if (!strcmp(optarg, "uyvy")) {
				s32ChnId = 0;
				PixelFormat = RK_FMT_YUV422_UYVY;
				CompressMode = COMPRESS_MODE_NONE;
			} else if (!strcmp(optarg, "afbc")) {
				s32ChnId = 2;
				PixelFormat = RK_FMT_YUV420SP;
				CompressMode = COMPRESS_AFBC_16x16;
			}
			break;
		case 'w':
			video_width = atoi(optarg);
			break;
		case 'h':
			video_height = atoi(optarg);
			break;
		case 'I':
			s32CamId = atoi(optarg);
			break;
		case 'l':
			s32loopCnt = atoi(optarg);
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

	printf("#CameraIdx: %d\n", s32CamId);
	printf("#pDeviceName: %s\n", pDeviceName);
	printf("#Output Path: %s\n", pOutPath);
	printf("#IQ Path: %s\n", iq_file_dir);
	if (iq_file_dir) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
		printf("#bMultictx: %d\n\n", bMultictx);
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;

		SAMPLE_COMM_ISP_Init(s32CamId, hdr_mode, bMultictx, iq_file_dir);
		SAMPLE_COMM_ISP_Run(s32CamId);
#endif
	}

	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		goto __FAILED;
	}

	// Init VI
	ctx->vi.u32Width = video_width;
	ctx->vi.u32Height = video_height;
	ctx->vi.s32DevId = s32CamId;
	ctx->vi.u32PipeId = ctx->vi.s32DevId;
	ctx->vi.s32ChnId = s32ChnId;
	ctx->vi.stChnAttr.stIspOpt.u32BufCount = 3;
	ctx->vi.stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
	ctx->vi.stChnAttr.u32Depth = 2;
	ctx->vi.stChnAttr.enPixelFormat = PixelFormat;
	ctx->vi.stChnAttr.enCompressMode = CompressMode;
	ctx->vi.stChnAttr.stFrameRate.s32SrcFrameRate = -1;
	ctx->vi.stChnAttr.stFrameRate.s32DstFrameRate = -1;
	ctx->vi.dstFilePath = pOutPath;
	ctx->vi.s32loopCount = s32loopCnt;
	if (pDeviceName) {
		strcpy(ctx->vi.stChnAttr.stIspOpt.aEntityName, pDeviceName);
	}
	SAMPLE_COMM_VI_CreateChn(&ctx->vi);

	pthread_create(&vi_thread_id, 0, vi_get_stream, (void *)(&ctx->vi));

	printf("%s initial finish\n", __func__);

	while (!quit) {
		sleep(1);
	}

	printf("%s exit!\n", __func__);

	pthread_join(vi_thread_id, NULL);

	// Destroy VI
	SAMPLE_COMM_VI_DestroyChn(&ctx->vi);

__FAILED:
	RK_MPI_SYS_Exit();
	if (iq_file_dir) {
#ifdef RKAIQ
		SAMPLE_COMM_ISP_Stop(s32CamId);
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
