CSRCS += EG_SdlContext.cpp
CSRCS += EG_SdlArc.cpp
CSRCS += EG_SdlBackgroundg.cpp
CSRCS += EG_SdlComposite.cpp
CSRCS += EG_SdlImage.cpp
CSRCS += EG_SdlLabel.cpp
CSRCS += EG_SdlLine.cpp
CSRCS += EG_SdlMask.cpp
CSRCS += EG_SdlPolygon.cpp
CSRCS += EG_SdlRect.cpp
CSRCS += EG_SdlStackBlur.cpp
CSRCS += EG_SdlTextureCache.cpp
CSRCS += EG_SdlUtils.cpp
CSRCS += EG_SdlLayer.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/sdl
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/sdl

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/sdl"
