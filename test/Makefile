# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)

.PHONY : clean all

TARGET1 = sample_camera_stresstest
TARGET2 = sample_multi_camera_stresstest

ifeq ($(RK_MEDIA_CHIP), rk3588)
MULLTI_CAMERA_ENABLE = y
endif


ifeq ($(MULLTI_CAMERA_ENABLE),y)
all: $(TARGET1) $(TARGET2)
else
all: $(TARGET1)
endif

$(TARGET1):$(COMM_OBJ) $(TARGET1).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(TARGET2):$(COMM_OBJ) $(TARGET2).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

install:
	@cp -f $(TARGET1) $(SAMPLE_OUT_DIR)/bin
ifeq ($(MULLTI_CAMERA_ENABLE),y)
	@cp -f $(TARGET2) $(SAMPLE_OUT_DIR)/bin
endif

clean:
	@rm -f $(TARGET1)
ifeq ($(MULLTI_CAMERA_ENABLE),y)
	@rm -f $(TARGET2)
endif
	@rm -f $(OBJS)

