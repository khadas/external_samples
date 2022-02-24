# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)

.PHONY : clean all

TARGET1 = sample_vi_vpss_osd_venc

ifeq ($(AVS_ENABLE),y)
TARGET2 = sample_multi_vi_avs_osd_venc
all: $(TARGET1) $(TARGET2)
else
all: $(TARGET1)
endif

$(TARGET1):$(COMM_OBJ) $(TARGET1).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

ifeq ($(AVS_ENABLE),y)
$(TARGET2):$(COMM_OBJ) $(TARGET2).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)
endif

install:
	@cp -f $(TARGET1) $(SAMPLE_OUT_DIR)/bin
ifeq ($(AVS_ENABLE),y)
	@cp -f $(TARGET2) $(SAMPLE_OUT_DIR)/bin
endif

clean:
	@rm -f $(TARGET1)
ifeq ($(AVS_ENABLE),y)
	@rm -f $(TARGET2)
endif
	@rm -f $(OBJS)

