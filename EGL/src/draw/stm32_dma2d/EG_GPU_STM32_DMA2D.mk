CSRCS += EG_GPU_STM32_DMA2D.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/stm32_dma2d
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/stm32_dma2d

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/stm32_dma2d"
