CSRCS += EG_Dave2Context.cpp
CSRCS += EG_Dave2DrawLabel.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/renesas
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/renesas

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/draw/renesas"
