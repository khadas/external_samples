# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)

.PHONY : clean all

TARGET1 = sample_camera_stresstest

all: $(TARGET1)

$(TARGET1):$(COMM_OBJ) $(TARGET1).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

install:
	@mv -f $(TARGET1) $(SAMPLE_OUT_DIR)/bin

clean:
	@rm -f $(TARGET1)
	@rm -f $(OBJS)

