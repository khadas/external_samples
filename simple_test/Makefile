# Rockchip sample Makefile

T_FAST_BOOT = simple_fast_boot
SIMPLE_FAST_LD_FLAGS += $(RK_MEDIA_OPTS) -L$(RK_MEDIA_OUTPUT)/lib -Wl,-Bstatic -lpthread -lrockit_tiny -lrockchip_mpp -lrkaiq \
		  -lm -lrga -lstdc++ -Wl,-Bdynamic 
SIMPLE_FAST_LD_FLAGS += -L$(SAMPLE_OUT_DIR) -lsample_comm

SOURCES_C     = $(wildcard *.c)
ifneq ($(CONFIG_ROCKIVA), y)
SOURCES_C     = $(filter-out simple_vi_rockiva.c, $(wildcard *.c))
endif
OBJECTS       = $(basename $(SOURCES_C))

all: $(OBJECTS)

simple_adec_ao: simple_adec_ao.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_ai_aenc: simple_ai_aenc.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_ai: simple_ai.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_ao: simple_ao.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_fast_boot: simple_fast_boot.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(SIMPLE_FAST_LD_FLAGS)

simple_venc_jpeg: simple_venc_jpeg.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_venc_osd: simple_venc_osd.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_get_frame: simple_vi_get_frame.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_ivs: simple_vi_ivs.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_rockiva: simple_vi_rockiva.c
	@echo "build iva simple"
ifeq ($(CONFIG_ROCKIVA), y)
	@make -C $(RK_MEDIA_TOP_DIR)/iva
	$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS) -DROCKIVA -lrockiva -lrknnmrt -I$(RK_MEDIA_OUTPUT)/include/rockiva
endif

simple_vi_venc: simple_vi_venc.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_venc_change_resolution: simple_vi_venc_change_resolution.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_venc_rtsp: simple_vi_venc_rtsp.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_venc_wrap: simple_vi_venc_wrap.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

simple_vi_vpss_venc: simple_vi_vpss_venc.c
	@$(SAMPLE_CC) $^ -o $@ $(SAMPLE_CFLAGS) $(LD_FLAGS)

install:
	@mv -f $(OBJECTS) $(SAMPLE_OUT_DIR)/bin

clean:
	@echo "clean simple"
