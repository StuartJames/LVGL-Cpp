CSRCS += EL_Font.cpp
CSRCS += EL_FontFmtText.cpp
CSRCS += EL_FontLoader.cpp

CSRCS += EG_FontDejavu16PersianHebrew.cpp
CSRCS += EL_FontMontserrat8.cpp
CSRCS += EL_FontMontserrat10.cpp
CSRCS += EL_FontMontserrat12.cpp
CSRCS += EL_FontMontserrat12_subpx.cpp
CSRCS += EL_FontMontserrat14.cpp
CSRCS += EL_FontMontserrat16.cpp
CSRCS += EL_FontMontserrat18.cpp
CSRCS += EL_FontMontserrat20.cpp
CSRCS += EL_FontMontserrat22.cpp
CSRCS += EL_FontMontserrat24.cpp
CSRCS += EL_FontMontserrat26.cpp
CSRCS += EL_FontMontserrat28.cpp
CSRCS += EL_FontMontserrat28Compressed.cpp
CSRCS += EL_FontMontserrat30.cpp
CSRCS += EL_FontMontserrat32.cpp
CSRCS += EL_FontMontserrat34.cpp
CSRCS += EL_FontMontserrat36.cpp
CSRCS += EL_FontMontserrat38.cpp
CSRCS += EL_FontMontserrat40.cpp
CSRCS += EL_FontMontserrat42.cpp
CSRCS += EL_FontMontserrat44.cpp
CSRCS += EL_FontMontserrat46.cpp
CSRCS += EL_FontMontserrat48.cpp
CSRCS += EG_FontSimsun16_cjk.cpp
CSRCS += EG_FontUNSCII8.cpp
CSRCS += EG_FontUNSCII16.cpp

DEPPATH += --dep-path $(EGL_DIR)/$(EGL_DIR_NAME)/src/font
VPATH += :$(EGL_DIR)/$(EGL_DIR_NAME)/src/font

CFLAGS += "-I$(EGL_DIR)/$(EGL_DIR_NAME)/src/font"
