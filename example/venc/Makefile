# Rockchip sample Makefile
LD_FLAGS += -L$(SAMPLE_OUT_DIR)/lib -lsample_comm
LD_FLAGS += $(RK_MEDIA_OPTS) -L$(RK_MEDIA_OUTPUT)/lib -lrockit -lrockchip_mpp -lrkaiq -lrtsp
all:
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_vi_vpss_osd_venc.c -o sample_vi_vpss_osd_venc $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_multi_vi_avs_osd_venc.c -o sample_multi_vi_avs_osd_venc $(LD_FLAGS)

install:
	@mv -f sample_vi_vpss_osd_venc sample_multi_vi_avs_osd_venc $(SAMPLE_OUT_DIR)/bin

clean:
	@echo "clean venc"
