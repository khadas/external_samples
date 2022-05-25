//ai->aenc
#include <stdio.h>
#include <sys/poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "sample_comm.h"


static bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	quit = true;
}

static RK_U32 code_type = RK_AUDIO_ID_ADPCM_G726;

static FILE *save_file;

//声音质量增强
RK_S32 test_init_ai_vqe(RK_S32 s32SampleRate) {
	AI_VQE_CONFIG_S stAiVqeConfig, stAiVqeConfig2;
	RK_S32 result;
	RK_S32 s32VqeGapMs = 16;
	int s32DevId = 0;
	int s32ChnIndex = 0;
	const char *pVqeCfgPath = "/oem/usr/share/vqefiles/config_aivqe.json";

	// Need to config enCfgMode to VQE attr even the VQE is not enabled
	memset(&stAiVqeConfig, 0, sizeof(AI_VQE_CONFIG_S));
	if (pVqeCfgPath != RK_NULL) {
		stAiVqeConfig.enCfgMode = AIO_VQE_CONFIG_LOAD_FILE;
		memcpy(stAiVqeConfig.aCfgFile, pVqeCfgPath, strlen(pVqeCfgPath));
	}

	if (s32VqeGapMs != 16 && s32VqeGapMs != 10) {
		RK_LOGE("Invalid gap: %d, just supports 16ms or 10ms for AI VQE", s32VqeGapMs);
		return RK_FAILURE;
	}

	stAiVqeConfig.s32WorkSampleRate = s32SampleRate;
	stAiVqeConfig.s32FrameSample = s32SampleRate * s32VqeGapMs / 1000;
	result = RK_MPI_AI_SetVqeAttr(s32DevId, s32ChnIndex, 0, 0, &stAiVqeConfig);
	if (result != RK_SUCCESS) {
		RK_LOGE("%s: SetVqeAttr(%d,%d) failed with %#x",
				__FUNCTION__, s32DevId, s32ChnIndex, result);
		return result;
	}

	result = RK_MPI_AI_GetVqeAttr(s32DevId, s32ChnIndex, &stAiVqeConfig2);
	if (result != RK_SUCCESS) {
		RK_LOGE("%s: SetVqeAttr(%d,%d) failed with %#x",
				__FUNCTION__, s32DevId, s32ChnIndex, result);
		return result;
	}

	result = memcmp(&stAiVqeConfig, &stAiVqeConfig2, sizeof(AI_VQE_CONFIG_S));
	if (result != RK_SUCCESS) {
		RK_LOGE("%s: set/get vqe config is different: %d", __FUNCTION__, result);
		return result;
	}

	result = RK_MPI_AI_EnableVqe(s32DevId, s32ChnIndex);
	if (result != RK_SUCCESS) {
		RK_LOGE("%s: EnableVqe(%d,%d) failed with %#x",
				__FUNCTION__, s32DevId, s32ChnIndex, result);
		return result;
	}

	return RK_SUCCESS;
}

RK_S32 ai_set_other(RK_S32 s32SetTrackMode, RK_S32 s32SetVolume)
{
	printf("\n=======%s=======\n",__func__);
	int s32DevId = 0;

	RK_MPI_AI_SetVolume(s32DevId, s32SetVolume);

	if (s32SetTrackMode) {
		RK_LOGI("test info : set track mode = %d",s32SetTrackMode);
		RK_MPI_AI_SetTrackMode(s32DevId, (AUDIO_TRACK_MODE_E)s32SetTrackMode);
		s32SetTrackMode = 0;
	}

	AUDIO_TRACK_MODE_E trackMode;
	RK_MPI_AI_GetTrackMode(s32DevId, &trackMode);
	RK_LOGI("test info : get track mode = %d", trackMode);

	return 0;
}

RK_S32 test_open_device_ai(int s32Channel, RK_S32 s32DeviceSampleRate, RK_S32 s32SampleRate ,RK_S32 u32FrameCnt, RK_S32 track_mode)
{
	printf("\n=======%s=======\n",__func__);
	AIO_ATTR_S aiAttr;
	AI_CHN_PARAM_S pstParams;
	RK_S32 result;
	int aiDevId = 0;
	int aiChn = 0;
	memset(&aiAttr, 0, sizeof(AIO_ATTR_S));

	RK_BOOL needResample = (s32DeviceSampleRate != s32SampleRate) ? RK_TRUE : RK_FALSE;

	sprintf((char *)aiAttr.u8CardName, "%s","hw:0,0");

	//s32DeviceSampleRate和s32SampleRate,s32SampleRate可以使用其他采样率，需要调用重采样函数。默认一样采样率。
	aiAttr.soundCard.channels = 2;
	aiAttr.soundCard.sampleRate = s32DeviceSampleRate;
	aiAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;
	aiAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	aiAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)s32SampleRate;

	aiAttr.enSoundmode = ((s32Channel == 2) ? AUDIO_SOUND_MODE_STEREO : AUDIO_SOUND_MODE_MONO);//AUDIO_SOUND_MODE_STEREO;  //AUDIO_SOUND_MODE_MONO
	aiAttr.u32FrmNum = 4;
	aiAttr.u32PtNumPerFrm = u32FrameCnt;

	aiAttr.u32EXFlag = 0;
	aiAttr.u32ChnCnt = 2;

	result = RK_MPI_AI_SetPubAttr(aiDevId, &aiAttr);
	if (result != 0) {
			RK_LOGE("ai set attr fail, reason = %d", result);
			goto __FAILED;
	}

	result = RK_MPI_AI_Enable(aiDevId);
	if (result != 0) {
			RK_LOGE("ai enable fail, reason = %d", result);
			goto __FAILED;
	}

	memset(&pstParams, 0, sizeof(AI_CHN_PARAM_S));
	pstParams.enLoopbackMode = AUDIO_LOOPBACK_NONE;
	pstParams.s32UsrFrmDepth = 1;
	result = RK_MPI_AI_SetChnParam(aiDevId, aiChn, &pstParams);
	if (result != RK_SUCCESS) {
			RK_LOGE("ai set channel params, aiChn = %d", aiChn);
			return RK_FAILURE;
	}

	//test_init_ai_vqe(s32SampleRate);

	result =  RK_MPI_AI_EnableChn(aiDevId, aiChn);
	if (result != 0) {
			RK_LOGE("ai enable channel fail, aiChn = %d, reason = %x", aiChn, result);
			return RK_FAILURE;
	}


	if (needResample == RK_TRUE) {
		RK_LOGI("need to resample %d -> %d", s32DeviceSampleRate, s32SampleRate);
		result = RK_MPI_AI_EnableReSmp(aiDevId, aiChn, (AUDIO_SAMPLE_RATE_E)s32SampleRate);
		if (result != 0) {
			RK_LOGE("ai enable channel fail, reason = %x, aiChn = %d", result,aiChn);
			return RK_FAILURE;
		}
	}

	ai_set_other(track_mode, 100);

	return RK_SUCCESS;
__FAILED:
	return RK_FAILURE;

}


RK_S32 test_init_mpi_aenc(int s32Channel, RK_S32 s32SampleRate)
{
	printf("\n=======%s=======\n",__func__);
	RK_S32 s32ret = 0;
	AENC_CHN_ATTR_S pstChnAttr;
	RK_CODEC_ID_E enCodecId = (RK_CODEC_ID_E)code_type;//RK_AUDIO_ID_ADPCM_G726 RK_AUDIO_ID_PCM_ALAW
	AUDIO_BIT_WIDTH_E enBitwidth = AUDIO_BIT_WIDTH_16;

	memset(&pstChnAttr, 0, sizeof(AENC_CHN_ATTR_S));

	printf("codecId=%d\n",(int)enCodecId);

	pstChnAttr.stCodecAttr.enType		= enCodecId;
	pstChnAttr.stCodecAttr.u32Channels	= s32Channel;
	pstChnAttr.stCodecAttr.u32SampleRate	= s32SampleRate;
	pstChnAttr.stCodecAttr.enBitwidth	= enBitwidth;
	pstChnAttr.stCodecAttr.pstResv		= RK_NULL;

	pstChnAttr.enType		= enCodecId;
	pstChnAttr.u32BufCount	= 4;

	s32ret = RK_MPI_AENC_CreateChn(0, &pstChnAttr);
	if (s32ret) {
			RK_LOGE("create aenc chn %d err:0x%x\n", enCodecId, s32ret);
	}
	return s32ret;
}

static RK_CHAR optstr[] = "?::d:c:r:o:t:m:";
static void print_usage(const RK_CHAR *name) {
	printf("usage example:\n");
	printf("\t%s [-r 8000] [-c 1] [-m 8] -o /tmp/aenc.g726\n", name);
	printf("\t-r: sample rate, Default:16000\n");
	printf("\t-c: channel count, Default:2\n");
	printf("\t-m: track_mode, Default:0\n");
	printf("\t-t: --encode: encode type, Default:g726, Value:g711a, g711u, g726\n");
	printf("\t-o: output path, Default:\"/tmp/aenc.g726\"\n");
}


// 编码g726的时候，只支持8000 单声道，所以需要设置所以需要设置track_mode为8
int main(int argc, char *argv[]) {
	RK_S32 u32SampleRate = 16000;
	RK_U32 u32ChnCnt = 2;
	RK_U32 u32FrameCnt = 1024;
	RK_CHAR *pOutPath = "/tmp/aenc.g726";
	RK_CHAR *pCodecName = "g726";
	int track_mode=0;
	int ret = 0;
	int c;

	while ((c = getopt(argc, argv, optstr)) != -1) {
		switch (c) {
		case 'r':
			u32SampleRate = atoi(optarg);
			break;
		case 'c':
			u32ChnCnt = atoi(optarg);
			break;
		case 'o':
			pOutPath = optarg;
			break;
			case 'm':
			track_mode = atoi(optarg);
			break;
		case 't':
			if (!strcmp(optarg, "g711a")) {
				code_type = RK_AUDIO_ID_PCM_ALAW;
				pCodecName = "g711a";
			} else if (!strcmp(optarg, "g711u")) {
				code_type = RK_AUDIO_ID_PCM_MULAW;
				pCodecName = "g711u";
			} else if (!strcmp(optarg, "g726")) {
				code_type = RK_AUDIO_ID_ADPCM_G726;
				pCodecName = "g726";
			} else {
				printf("ERROR: Invalid encoder type.\n");
				return 0;
			}
			break;
		case '?':
		default:
			print_usage(argv[0]);
			return 0;
		}
	}

	printf("#SampleRate: %d\n", u32SampleRate);
	printf("#Channel Count: %d\n", u32ChnCnt);
	printf("#Frame Count: %d\n", u32FrameCnt);
	printf("#track_mode: %d\n", track_mode);
	printf("#CodecName:%s\n", pCodecName);
	printf("#Output Path: %s\n", pOutPath);

	if (pOutPath) {
		save_file = fopen(pOutPath, "w");
		if (!save_file) {
			printf("ERROR: open file: %s fail, exit\n", pOutPath);
			return 0;
		}
	}

	signal(SIGINT, sigterm_handler);

	RK_MPI_SYS_Init();

	test_open_device_ai(u32ChnCnt, u32SampleRate, u32SampleRate, u32FrameCnt, track_mode);

	test_init_mpi_aenc(u32ChnCnt, u32SampleRate);


	//ai bind aenc
	MPP_CHN_S stSrcChn, stDestChn;
	stSrcChn.enModId    = RK_ID_AI;
	stSrcChn.s32DevId   = 0;
	stSrcChn.s32ChnId   = 0;

	stDestChn.enModId   = RK_ID_AENC;
	stDestChn.s32DevId  = 0;
	stDestChn.s32ChnId  = 0;

	// 3. bind AI-AENC
	ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (ret) {
	printf("Bind AI[0] to AENC[0] failed! ret=%d\n", ret);
	return -1;
	}

	printf("%s initial finish\n", __func__);


	AUDIO_STREAM_S pstStream;
	RK_S32 eos = 0;
	while (!quit) {
		ret = RK_MPI_AENC_GetStream(0, &pstStream, -1);
		if (ret == RK_SUCCESS) {
			MB_BLK bBlk = pstStream.pMbBlk;
			RK_VOID *pstFrame = RK_MPI_MB_Handle2VirAddr(bBlk);
			RK_S32 frameSize = pstStream.u32Len;
			RK_S32 timeStamp = pstStream.u64TimeStamp;
			eos = (frameSize <= 0) ? 1 : 0;
			if (pstFrame) {
				RK_LOGI("get frame data = %p, size = %d, timeStamp = %lld",
																					pstFrame, frameSize, timeStamp);
				if (save_file) {
					fwrite(pstFrame, frameSize, 1, save_file);
					fflush(save_file);
				}
				RK_MPI_AENC_ReleaseStream(0, &pstStream);
			}
		} else {
			RK_LOGE("fail to get aenc frame.");
		}

		if (eos) {
			RK_LOGI("get eos stream.");
			break;
		}
	}

	RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	RK_MPI_AI_DisableVqe(0, 0);
	RK_MPI_AI_DisableChn(0, 0);
	RK_MPI_AI_Disable(0);
	RK_MPI_AENC_DestroyChn(0);
	RK_MPI_SYS_Exit();
	return 0;
}
