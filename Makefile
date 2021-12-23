
ifeq ($(MEDIA_PARAM), )
    MEDIA_PARAM:=../Makefile.param
    include $(MEDIA_PARAM)
endif

export CONFIG_RK_SAMPLE=y
export LC_ALL=C
SHELL:=/bin/bash

CURRENT_DIR := $(shell pwd)

CC := $(RK_MEDIA_CROSS)-gcc

PKG_NAME := sample
PKG_BIN ?= out
PKG_BUILD ?= build

RK_MEDIA_OPTS += -Wl,-rpath-link,${RK_MEDIA_OUTPUT}/lib:$(RK_MEDIA_OUTPUT)/root/usr/lib
PKG_CONF_OPTS += -DCMAKE_C_FLAGS="$(RK_MEDIA_OPTS)" -DCMAKE_CXX_FLAGS="$(RK_MEDIA_OPTS)"
PKG_CONF_OPTS += -DRKPLATFORM=ON

# debug: build cmake with more message
# PKG_CONF_OPTS += -DCMAKE_VERBOSE_MAKEFILE=ON
#
ifeq ($(RK_MEDIA_CHIP), rv1126)
PKG_CONF_OPTS += -DCMAKE_SYSTEM_PROCESSOR=armv7l
PKG_CONF_OPTS += -DARCH64=OFF
endif

ifeq ($(RK_MEDIA_CHIP), rk3588)
PKG_CONF_OPTS += -DARCH64=ON
endif

ifeq ($(CONFIG_RK_SAMPLE),y)
PKG_TARGET := sample-build
else
PKG_TARGET :=
$(warning Not config source RK_SAMPLE, Skip...)
endif

ifeq ($(PKG_BIN),)
$(error ### $(CURRENT_DIR): PKG_BIN is NULL, Please Check !!!)
endif

COMM_DIR := $(CURRENT_DIR)/common
COMM_SRC := $(wildcard $(COMM_DIR)/*.c)
ifeq ($(RK_MEDIA_CHIP), rv1126)
COMM_SRC += $(wildcard $(COMM_DIR)/isp2.x/*.c)
endif
ifeq ($(RK_MEDIA_CHIP), rk3588)
COMM_SRC += $(wildcard $(COMM_DIR)/isp3.x/*.c)
endif
COMM_OBJ := $(COMM_SRC:%.c=%.o)

INC_FLAGS := -I$(COMM_DIR)
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/uAPI
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/uAPI2
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/common
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/xcore
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/algos
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/iq_parser
INC_FLAGS += -I$(RK_MEDIA_OUTPUT)/include/rkaiq/iq_parser_v2
CFLAGS += -g -Wall $(INC_FLAGS) $(PKG_CONF_OPTS) -lpthread -lm -ldl
LD_FLAGS += -L$(RK_MEDIA_OUTPUT)/lib  -lrockit -lrockchip_mpp -lasound -ldrm -lrkaiq
LD_FLAGS += -L$(RK_MEDIA_OUTPUT)/root/usr/lib -lasound

ifeq ($(RK_MEDIA_CHIP), rv1126)
INC_FLAGS += -I$(COMM_DIR)/isp2.x
CFLAGS += -DISP_HW_V20
LD_FLAGS += -L$(CURRENT_DIR)/lib  -lrtsp_32bit
endif

ifeq ($(RK_MEDIA_CHIP), rk3588)
INC_FLAGS += -I$(COMM_DIR)/isp3.x
CFLAGS += -DISP_HW_V30
LD_FLAGS += -L$(CURRENT_DIR)/lib  -lrtsp_64bit
endif

export SAMPLE_OUT_DIR=$(CURRENT_DIR)/out
export PKG_CONF_OPTS
export COMM_OBJ
export CC
export CFLAGS
export LD_FLAGS

all: $(PKG_TARGET)
	@echo "build $(PKG_NAME) done";

sample-build: libasound librkaiq librockit
	@mkdir -p $(SAMPLE_OUT_DIR)/bin
	@make -C $(CURRENT_DIR)/vi;
	@make -C $(CURRENT_DIR)/vi install;
	@make -C $(CURRENT_DIR)/vo;
	@make -C $(CURRENT_DIR)/vo install;
	@make -C $(CURRENT_DIR)/venc;
	@make -C $(CURRENT_DIR)/venc install;
	@make -C $(CURRENT_DIR)/avs;
	@make -C $(CURRENT_DIR)/avs install;
	@make -C $(CURRENT_DIR)/test;
	@make -C $(CURRENT_DIR)/test install;
	@cp -rfa $(SAMPLE_OUT_DIR)/* $(RK_MEDIA_OUTPUT)

libasound:
	@test ! -d $(RK_MEDIA_TOP_DIR)/alsa-lib || make -C $(RK_MEDIA_TOP_DIR)/alsa-lib

librkaiq:
	@test ! -d $(RK_MEDIA_TOP_DIR)/isp || make -C $(RK_MEDIA_TOP_DIR)/isp

librockit:
	@test ! -d $(RK_MEDIA_TOP_DIR)/rockit || make -C $(RK_MEDIA_TOP_DIR)/rockit

clean:
	@make -C $(CURRENT_DIR)/common clean
	@make -C $(CURRENT_DIR)/vi clean
	@make -C $(CURRENT_DIR)/vo clean
	@make -C $(CURRENT_DIR)/venc clean
	@make -C $(CURRENT_DIR)/avs clean
	@make -C $(CURRENT_DIR)/test clean
	@rm -rf $(SAMPLE_OUT_DIR)

distclean: clean
