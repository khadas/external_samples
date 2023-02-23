# Rockchip sample Makefile

LD_FLAGS += -L$(SAMPLE_OUT_DIR)/lib -lsample_comm
LD_FLAGS += $(RK_MEDIA_OPTS) -L$(RK_MEDIA_OUTPUT)/lib -lrockit -lrockchip_mpp -lrkaiq -lrockit_full
all:
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_multi_vi_avs.c         -o sample_multi_vi_avs          $(LD_FLAGS)

install:
	@mv -f sample_multi_vi_avs $(SAMPLE_OUT_DIR)/bin

clean:
	@rm -f sample_multi_vi_avs
