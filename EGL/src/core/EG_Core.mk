CSRCS += EG_DevixeScroll.cpp
CSRCS += EG_Display.cpp
CSRCS += EG_Event.cpp
CSRCS += EG_Group.cpp
CSRCS += EG_InputDevice.cpp
CSRCS += EG_ObjClass.cpp
CSRCS += EG_ObjDraw.cpp
CSRCS += EG_Object.cpp
CSRCS += EG_ObjLocalStyle.cpp
CSRCS += EG_ObjPosition.cpp
CSRCS += EG_ObjScroll.cpp
CSRCS += EG_ObjStyle.cpp
CSRCS += EG_ObjTree.cpp
CSRCS += EG_Refresh.cpp
CSRCS += EG_Root.cpp
CSRCS += EG_Theme.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/core
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/core

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/core"
