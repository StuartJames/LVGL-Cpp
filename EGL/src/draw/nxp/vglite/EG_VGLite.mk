CSRCS += EG_VGLite.cpp
CSRCS += EG_VGLite_Arc.cpp
CSRCS += EG_VGLite_Blend.cpp
CSRCS += EG_VGLite_Line.cpp
CSRCS += EG_VGLite_Rect.cpp
CSRCS += EG_VGLite_Bufffer.cpp
CSRCS += EG_VGLite_Utils.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/vglite
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/vglite

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/nxp/vglite"
