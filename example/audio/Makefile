# Rockchip sample Makefile

LD_FLAGS += -L$(SAMPLE_OUT_DIR)/lib -lsample_comm
LD_FLAGS += $(RK_MEDIA_OPTS) -L$(RK_MEDIA_OUTPUT)/lib -lrockit -lrockchip_mpp
all:
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_ai.c -o sample_ai $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_ai_aenc.c -o sample_ai_aenc $(LD_FLAGS)

install:
	@mv -f sample_ai sample_ai_aenc $(SAMPLE_OUT_DIR)/bin

clean:
	@rm -f sample_ai sample_ai_aenc
