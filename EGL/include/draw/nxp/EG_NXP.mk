DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp"

include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/pxp/EG_PXP.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/vglite/EG_VGLite.mk
