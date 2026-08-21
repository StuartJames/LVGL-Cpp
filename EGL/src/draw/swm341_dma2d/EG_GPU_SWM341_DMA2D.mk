CSRCS += EG_GPU_SWM341_DMA2D.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/swm341_dma2d
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/swm341_dma2d

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/swm341_dma2d"
