# Rockchip sample Makefile

all:
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_vi.c         -o sample_vi          $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_multi_vi.c    -o sample_multi_vi    $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_dual_camera.c -o sample_dual_camera $(LD_FLAGS)

install:
	@mv -f sample_vi sample_multi_vi sample_dual_camera $(SAMPLE_OUT_DIR)/bin

clean:
	@rm -f sample_vi
	@rm -f sample_multi_vi
	@rm -f sample_dual_camera
