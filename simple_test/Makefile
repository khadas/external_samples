# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)
#TARGETALL = $(patsubst %.c, %, ${SRCS})
.PHONY : clean all

TARGET1=$(subst simple_vi_rockiva, ,$(TARGET))
all: $(TARGET1)

# ${TARGETALL}:${COMM_OBJ} ${TARGETALL}.o
	# @$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

CONFIG_ROCKIVA=n
#需要链接rockiva的库和h文件
ifeq ($(CONFIG_ROCKIVA), y)
TARGET1 = $(TARGET)
RK_ROCKIVA_DIR := ${RK_MEDIA_OUTPUT}/../rockiva/librockiva/rockiva-rv1106-Linux
IVE_INC_FLAGS := ${RK_ROCKIVE_DIR}/include
IVE_LD_FLAGS := -L$(RK_ROCKIVE_DIR)/lib -lrockiva -lrknnmrt
endif

T_AO = simple_ao
T_AI_AENC = simple_ai_aenc
T_ADEC_AO = simple_adec_ao
T_VI_GET_FRAME = simple_vi_get_frame
T_VI_VENC = simple_vi_venc
T_VENC_OSD = simple_venc_osd
T_VI_IVS = simple_vi_ivs
T_AI = simple_ai
T_VI_VPSS_VENC = simple_vi_vpss_venc
T_VENC_JPEG = simple_venc_jpeg
T_VI_VENC_RTSP = simple_vi_venc_rtsp
T_VI_VENC_CHANGE_RESOLUTION = simple_vi_venc_change_resolution

ifeq ($(CONFIG_ROCKIVA), y)
T_VI_ROCKIVA = simple_vi_rockiva
endif

$(T_AO):$(COMM_OBJ) $(T_AO).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_AI_AENC):$(COMM_OBJ) $(T_AI_AENC).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_ADEC_AO):$(COMM_OBJ) $(T_ADEC_AO).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_VI_GET_FRAME):$(COMM_OBJ) $(T_VI_GET_FRAME).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_VI_VENC):$(COMM_OBJ) $(T_VI_VENC).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_VENC_OSD):$(COMM_OBJ) $(T_VENC_OSD).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_VI_IVS):$(COMM_OBJ) $(T_VI_IVS).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_AI):$(COMM_OBJ) $(T_AI).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_VI_VPSS_VENC):$(COMM_OBJ) $(T_VI_VPSS_VENC).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(T_VENC_JPEG):$(COMM_OBJ) $(T_VENC_JPEG).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)
	
$(T_VI_VENC_RTSP):$(COMM_OBJ) $(T_VI_VENC_RTSP).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)
	
$(T_VI_VENC_CHANGE_RESOLUTION):$(COMM_OBJ) $(T_VI_VENC_CHANGE_RESOLUTION).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

ifeq ($(CONFIG_ROCKIVA), y)
CFLAGS += -I$(IVE_INC_FLAGS)
$(T_VI_ROCKIVA):$(COMM_OBJ) $(T_VI_ROCKIVA).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS) $(IVE_LD_FLAGS)
endif


install:
	@mv -f $(TARGET1) $(SAMPLE_OUT_DIR)/bin
	@rm -f $(OBJS)
clean:
	@rm -f $(OBJS)
	rm -f $(TARGET1)



