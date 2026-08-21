CSRCS += EG_PXP_Context.cpp
CSRCS += EG_PXP_Blend.cpp
CSRCS += EG_PXP_GPU_OSA.cpp
CSRCS += EG_PXP_GPU.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/pxp
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/pxp

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/pxp"
