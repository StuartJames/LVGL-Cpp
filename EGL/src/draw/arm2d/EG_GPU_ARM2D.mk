CSRCS += EG_GPU_ARM2D.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/arm2d
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/arm2d

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/arm2d"
