# Rockchip sample Makefile

# ifeq ($(CONFIG_RK_IVA),y)
# 	LD_FLAGS += -DROCKIVA
# 	LD_FLAGS += -DROCKIVA -lrockiva
# endif
LD_FLAGS += -L$(SAMPLE_OUT_DIR)/lib -lsample_comm
LD_FLAGS += $(RK_MEDIA_OPTS) -L$(RK_MEDIA_OUTPUT)/lib -lrockit -lrockchip_mpp -lpthread -lrkaiq -lrtsp
# LD_FLAGS += -ldrm
all:
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_demo_vi_venc.c -o sample_demo_vi_venc $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_demo_multi_camera_eptz.c -o sample_demo_multi_camera_eptz $(LD_FLAGS)
	@$(SAMPLE_CC) $(SAMPLE_CFLAGS) sample_demo_dual_camera.c -o sample_demo_dual_camera $(LD_FLAGS) -DROCKIVA -lrockiva

install:
	@mv -f sample_demo_vi_venc $(SAMPLE_OUT_DIR)/bin
	@mv -f sample_demo_multi_camera_eptz $(SAMPLE_OUT_DIR)/bin
	@mv -f sample_demo_dual_camera $(SAMPLE_OUT_DIR)/bin

clean:
	@echo "clean demo"
