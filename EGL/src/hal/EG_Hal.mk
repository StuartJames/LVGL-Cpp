CSRCS += EG_HALDisplay.cpp
CSRCS += EG_HALInputDevice.cpp
CSRCS += EG_HALTick.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/hal
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/hal

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/hal"
