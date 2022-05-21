# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)
#TARGETALL = $(patsubst %.c, %, ${SRCS})
.PHONY : clean all

all: $(TARGET)

# ${TARGETALL}:${COMM_OBJ} ${TARGETALL}.o
	# @$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)



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



install:
	@cp -f $(TARGET) $(SAMPLE_OUT_DIR)/bin
	@rm -f $(OBJS)
clean:
	@rm -f $(OBJS)
	rm -f $(TARGET)



