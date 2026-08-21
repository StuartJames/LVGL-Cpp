CSRCS += EG_DeviceContext.cpp
CSRCS += EL_DrawArc.cpp
CSRCS += EL_DrawBase.cpp
CSRCS += EL_DrawImage.cpp
CSRCS += EL_DrawLabel.cpp
CSRCS += EL_DrawLine.cpp
CSRCS += EL_DrawMask.cpp
CSRCS += EL_DrawPolygon.cpp
CSRCS += EL_DrawRect.cpp
CSRCS += EL_DrawTransform.cpp
CSRCS += EL_EventDC.cpp
CSRCS += EL_ImageBuffer.cpp
CSRCS += EL_ImageCache.cpp
CSRCS += EL_ImageDecoder.cpp
CSRCS += EL_LayerContext.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw"

include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/arm2d/EG_GPU_ARM2D.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/EL_NXP.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/renesas/EG_Renesas.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/sdl/EL_SDL.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/stm32_dma2d/EG_GPU_STM32_DMA2D.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/sw/EL_SoftDraw.mk
include $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/swm341_dma2d/EG_GPU_SWM341_DMA2D.mk
