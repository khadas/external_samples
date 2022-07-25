#ifndef __APP_PARAM_INFO_H__
#define __APP_PARAM_INFO_H__

typedef enum {
	RK_VENC_TYPE_H264 = 1,
	RK_VENC_TYPE_H265,
	RK_VENC_TYPE_INVALID
} eVENC_TYPE;

struct app_param_info
{
	uint32_t head;
	int32_t len;
	int32_t cam_mirror_flip;
	int32_t cam_fps_denominator;
	int32_t cam_fps_numerator;

	int32_t night_mode;
	int32_t led_value;
	int32_t venc_w;
	int32_t venc_h;
	int32_t venc_type;
	int32_t venc_bitrate;

	uint32_t crc32;
};
#define RK_APP_PARAM_OFFSET   "rk_app_param_offset"
#define RK_CAM_W              "rk_cam_w"
#define RK_CAM_H              "rk_cam_h"
#define RK_VENC_W             "rk_venc_w"
#define RK_VENC_H             "rk_venc_h"
#define RK_VENC_TYPE          "rk_venc_type"
#define RK_VENC_BITRATE       "rk_venc_bitrate"
#define RK_CAM_MIRROR_FLIP    "rk_cam_mirror_flip"
#define RK_CAM_FPS_NUME       "rk_cam_fps_numerator"
#define RK_CAM_FPS_DENOM      "rk_cam_fps_denominator"
#define RK_LED_VALUE          "rk_led_value"
#define RK_NIGHT_MODE         "rk_night_mode"
#define RK_IQ_BIN_ADDR_STRINGS "rk_iqbin_addr"
#define RK_IQ_BIN_SIZE_STRINGS "rk_iqbin_size"

#endif//  __APP_PARAM_INFO_H__
