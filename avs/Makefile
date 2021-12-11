# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)
TARGET := $(SRCS:%.c=%)

.PHONY : clean all

TARGET1 = sample_multi_vi_avs

all: $(TARGET1)

$(TARGET1):$(COMM_OBJ) $(TARGET1).o
	@$(CC) $(CFLAGS) -o $@ $^ $(LD_FLAGS)

install:
	@cp -f $(TARGET1) $(RK_MEDIA_OUTPUT)/bin

clean:
	@rm -f $(TARGET1)
	@rm -f $(OBJS)

