# Rockchip sample Makefile

SRCS := $(wildcard *.c)
OBJS  := $(SRCS:%.c=%.o)

.PHONY : clean

clean:
	@rm -f $(OBJS)
	@make -C isp2.x clean
	@make -C isp3.x clean

