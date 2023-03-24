# Rockchip sample Makefile

LD_FLAGS += -L$(COMM_DIR)/lib -lsample_comm
LD_FLAGS += $(RK_MEDIA_OPTS) -L$(RK_MEDIA_OUTPUT)/lib -lrkaiq

ifeq ($(RK_MEDIA_CHIP), rv1106)
LD_FLAGS += -lrockit_full
else
LD_FLAGS += -lrockit
endif

all:
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_vi.c         -o sample_vi          $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_multi_vi.c    -o sample_multi_vi    $(LD_FLAGS)

install:
	@mv -f sample_vi sample_multi_vi $(SAMPLE_OUT_DIR)/bin

clean:
	@rm -f sample_vi
	@rm -f sample_multi_vi
