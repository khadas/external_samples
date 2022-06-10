# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)

.PHONY : clean all

TARGET1 = sample_vi
TARGET2 = sample_multi_vi
TARGET3 = sample_dual_camera

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1):$(COMM_OBJ) $(TARGET1).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(TARGET2):$(COMM_OBJ) $(TARGET2).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

$(TARGET3):$(COMM_OBJ) $(TARGET3).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

install:
	@mv -f $(TARGET1) $(SAMPLE_OUT_DIR)/bin
	@mv -f $(TARGET2) $(SAMPLE_OUT_DIR)/bin
	@mv -f $(TARGET3) $(SAMPLE_OUT_DIR)/bin

clean:
	@rm -f $(TARGET1)
	@rm -f $(TARGET2)
	@rm -f $(TARGET3)
	@rm -f $(OBJS)

