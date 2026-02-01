/*******************************************************************************
 * Size: 8 px
 * Bpp: 1
 * Opts: --no-compress --no-prefilter --BitsPerPixel 1 --size 8 --font unscii-8.ttf -r 0x20-0x7F --format lvgl -o EG_FontUnscii8.c --force-fast-kern-format
 ******************************************************************************/

#ifdef EG_EGL_H_INCLUDE_SIMPLE
    #include "EGL.h"
#else
    #include "EGL.h"
#endif

#ifndef EG_FONT_UNSCII_8
    #define EG_FONT_UNSCII_8 1
#endif

#if EG_FONT_UNSCII_8

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static EG_ATTRIBUTE_LARGE_CONST const uint8_t GlyphBitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xcc,

    /* U+0022 "\"" */
    0xcf, 0x3c, 0xc0,

    /* U+0023 "#" */
    0x6c, 0xdb, 0xfb, 0x6f, 0xed, 0x9b, 0x0,

    /* U+0024 "$" */
    0x31, 0xfc, 0x1e, 0xf, 0xe3, 0x0,

    /* U+0025 "%" */
    0xc7, 0x98, 0x61, 0x86, 0x78, 0xc0,

    /* U+0026 "&" */
    0x38, 0xd8, 0xe3, 0xbd, 0xd9, 0x9d, 0x80,

    /* U+0027 "'" */
    0x6f, 0x0,

    /* U+0028 "(" */
    0x36, 0xcc, 0xc6, 0x30,

    /* U+0029 ")" */
    0xc6, 0x33, 0x36, 0xc0,

    /* U+002A "*" */
    0x66, 0x3c, 0xff, 0x3c, 0x66,

    /* U+002B "+" */
    0x30, 0xcf, 0xcc, 0x30,

    /* U+002C "," */
    0x6f, 0x0,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x3, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc0,

    /* U+0030 "0" */
    0x7b, 0x3d, 0xfb, 0xcf, 0x37, 0x80,

    /* U+0031 "1" */
    0x31, 0xc3, 0xc, 0x30, 0xcf, 0xc0,

    /* U+0032 "2" */
    0x7b, 0x31, 0x8c, 0x63, 0xf, 0xc0,

    /* U+0033 "3" */
    0x7b, 0x30, 0xce, 0xf, 0x37, 0x80,

    /* U+0034 "4" */
    0x1c, 0x79, 0xb6, 0x6f, 0xe1, 0x83, 0x0,

    /* U+0035 "5" */
    0xff, 0xf, 0x83, 0xf, 0x37, 0x80,

    /* U+0036 "6" */
    0x39, 0x8c, 0x3e, 0xcf, 0x37, 0x80,

    /* U+0037 "7" */
    0xfc, 0x30, 0xc6, 0x30, 0xc3, 0x0,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xde, 0xcf, 0x37, 0x80,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xdf, 0xc, 0x67, 0x0,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0x6c, 0x6, 0xf0,

    /* U+003C "<" */
    0x19, 0x99, 0x86, 0x18, 0x60,

    /* U+003D "=" */
    0xfc, 0xf, 0xc0,

    /* U+003E ">" */
    0xc3, 0xc, 0x33, 0x33, 0x0,

    /* U+003F "?" */
    0x7b, 0x30, 0xc6, 0x30, 0x3, 0x0,

    /* U+0040 "@" */
    0x7d, 0x8f, 0x7e, 0xfd, 0xf8, 0x1f, 0x0,

    /* U+0041 "A" */
    0x31, 0xec, 0xf3, 0xff, 0x3c, 0xc0,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3f, 0x80,

    /* U+0043 "C" */
    0x7b, 0x3c, 0x30, 0xc3, 0x37, 0x80,

    /* U+0044 "D" */
    0xf3, 0x6c, 0xf3, 0xcf, 0x6f, 0x0,

    /* U+0045 "E" */
    0xff, 0xc, 0x3e, 0xc3, 0xf, 0xc0,

    /* U+0046 "F" */
    0xff, 0xc, 0x3e, 0xc3, 0xc, 0x0,

    /* U+0047 "G" */
    0x7b, 0x3c, 0x37, 0xcf, 0x37, 0xc0,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xcf, 0x3c, 0xc0,

    /* U+0049 "I" */
    0xfc, 0xc3, 0xc, 0x30, 0xcf, 0xc0,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xf, 0x37, 0x80,

    /* U+004B "K" */
    0xc7, 0x9b, 0x67, 0x8d, 0x99, 0xb1, 0x80,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xf, 0xc0,

    /* U+004D "M" */
    0xc7, 0xdf, 0xfe, 0xbc, 0x78, 0xf1, 0x80,

    /* U+004E "N" */
    0xc7, 0xcf, 0xde, 0xfc, 0xf8, 0xf1, 0x80,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xfe, 0xc3, 0xc, 0x0,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x66, 0xc0,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xfe, 0xdb, 0x3c, 0xc0,

    /* U+0053 "S" */
    0x7b, 0x3c, 0x1e, 0xf, 0x37, 0x80,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0x0,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xe3, 0x0,

    /* U+0057 "W" */
    0xc7, 0x8f, 0x1e, 0xbf, 0xfd, 0xf1, 0x80,

    /* U+0058 "X" */
    0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3,

    /* U+0059 "Y" */
    0xc3, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x18,

    /* U+005A "Z" */
    0xfc, 0x31, 0x8c, 0x63, 0xf, 0xc0,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xf0,

    /* U+005C "\\" */
    0xc0, 0x60, 0x30, 0x18, 0xc, 0x6, 0x3,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0xf0,

    /* U+005E "^" */
    0x10, 0x71, 0xb6, 0x30,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xc6, 0x30,

    /* U+0061 "a" */
    0x78, 0x37, 0xf3, 0x7c,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3f, 0x80,

    /* U+0063 "c" */
    0x7e, 0x31, 0x87, 0x80,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+0065 "e" */
    0x7b, 0x3f, 0xf0, 0x78,

    /* U+0066 "f" */
    0x3b, 0x3e, 0xc6, 0x31, 0x80,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xdf, 0xf, 0xe0,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xc0,

    /* U+0069 "i" */
    0x60, 0x38, 0xc6, 0x31, 0xe0,

    /* U+006A "j" */
    0x18, 0x6, 0x31, 0x8c, 0x7e,

    /* U+006B "k" */
    0xc3, 0xc, 0xf6, 0xf3, 0x6c, 0xc0,

    /* U+006C "l" */
    0xe3, 0x18, 0xc6, 0x31, 0xe0,

    /* U+006D "m" */
    0xcd, 0xff, 0x5e, 0xbc, 0x60,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcc,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0x78,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xfe, 0xc3, 0x0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xdf, 0xc, 0x30,

    /* U+0072 "r" */
    0xfb, 0x3c, 0x30, 0xc0,

    /* U+0073 "s" */
    0x7f, 0x7, 0x83, 0xf8,

    /* U+0074 "t" */
    0x61, 0x8f, 0xd8, 0x61, 0x83, 0xc0,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0x7c,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xde, 0x30,

    /* U+0077 "w" */
    0xc7, 0x8f, 0x5b, 0xe6, 0xc0,

    /* U+0078 "x" */
    0xc6, 0xd8, 0xe3, 0x6c, 0x60,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xdf, 0xd, 0xe0,

    /* U+007A "z" */
    0xfc, 0x63, 0x18, 0xfc,

    /* U+007B "{" */
    0x1c, 0xc3, 0x38, 0x30, 0xc1, 0xc0,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xe0, 0xc3, 0x7, 0x30, 0xce, 0x0,

    /* U+007E "~" */
    0x77, 0xb8,

    /* U+007F "" */
    0xc1, 0x42, 0xbd, 0x2c, 0x40, 0x81, 0x0
};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const EG_FontFmtTextGlyphProps_t pGlyphProps[] = {
    {.BitmapIndex = 0, .AdvWidth = 0, .BoxWidth = 0, .BoxHeight = 0, .OffsetX = 0, .OffsetY = 0} /* id = 0 reserved */,
    {.BitmapIndex = 0, .AdvWidth = 128, .BoxWidth = 1, .BoxHeight = 1, .OffsetX = 0, .OffsetY = 8},
    {.BitmapIndex = 1, .AdvWidth = 128, .BoxWidth = 2, .BoxHeight = 7, .OffsetX = 3, .OffsetY = 1},
    {.BitmapIndex = 3, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 3, .OffsetX = 1, .OffsetY = 5},
    {.BitmapIndex = 6, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 13, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 19, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 6, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 25, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 32, .AdvWidth = 128, .BoxWidth = 3, .BoxHeight = 3, .OffsetX = 2, .OffsetY = 5},
    {.BitmapIndex = 34, .AdvWidth = 128, .BoxWidth = 4, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 1},
    {.BitmapIndex = 38, .AdvWidth = 128, .BoxWidth = 4, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 1},
    {.BitmapIndex = 42, .AdvWidth = 128, .BoxWidth = 8, .BoxHeight = 5, .OffsetX = 0, .OffsetY = 2},
    {.BitmapIndex = 47, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 2},
    {.BitmapIndex = 51, .AdvWidth = 128, .BoxWidth = 3, .BoxHeight = 3, .OffsetX = 2, .OffsetY = 0},
    {.BitmapIndex = 53, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 1, .OffsetX = 1, .OffsetY = 4},
    {.BitmapIndex = 54, .AdvWidth = 128, .BoxWidth = 2, .BoxHeight = 2, .OffsetX = 3, .OffsetY = 1},
    {.BitmapIndex = 55, .AdvWidth = 128, .BoxWidth = 8, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 62, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 68, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 74, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 80, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 86, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 93, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 99, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 105, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 111, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 117, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 123, .AdvWidth = 128, .BoxWidth = 2, .BoxHeight = 6, .OffsetX = 3, .OffsetY = 1},
    {.BitmapIndex = 125, .AdvWidth = 128, .BoxWidth = 3, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 0},
    {.BitmapIndex = 128, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 133, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 3, .OffsetX = 1, .OffsetY = 3},
    {.BitmapIndex = 136, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 141, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 147, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 154, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 160, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 166, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 172, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 178, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 184, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 190, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 196, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 202, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 208, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 214, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 221, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 227, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 234, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 241, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 247, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 253, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 259, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 265, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 271, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 277, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 283, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 289, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 296, .AdvWidth = 128, .BoxWidth = 8, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 303, .AdvWidth = 128, .BoxWidth = 8, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 310, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 316, .AdvWidth = 128, .BoxWidth = 4, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 1},
    {.BitmapIndex = 320, .AdvWidth = 128, .BoxWidth = 8, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 327, .AdvWidth = 128, .BoxWidth = 4, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 1},
    {.BitmapIndex = 331, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 4, .OffsetX = 0, .OffsetY = 4},
    {.BitmapIndex = 335, .AdvWidth = 128, .BoxWidth = 8, .BoxHeight = 1, .OffsetX = 0, .OffsetY = 0},
    {.BitmapIndex = 336, .AdvWidth = 128, .BoxWidth = 4, .BoxHeight = 3, .OffsetX = 3, .OffsetY = 5},
    {.BitmapIndex = 338, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 342, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 348, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 352, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 358, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 362, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 367, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 6, .OffsetX = 1, .OffsetY = 0},
    {.BitmapIndex = 372, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 378, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 1},
    {.BitmapIndex = 383, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 8, .OffsetX = 1, .OffsetY = 0},
    {.BitmapIndex = 388, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 394, .AdvWidth = 128, .BoxWidth = 5, .BoxHeight = 7, .OffsetX = 2, .OffsetY = 1},
    {.BitmapIndex = 399, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 5, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 404, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 408, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 412, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 6, .OffsetX = 1, .OffsetY = 0},
    {.BitmapIndex = 417, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 6, .OffsetX = 1, .OffsetY = 0},
    {.BitmapIndex = 422, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 426, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 430, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 436, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 440, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 444, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 5, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 449, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 5, .OffsetX = 0, .OffsetY = 1},
    {.BitmapIndex = 454, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 6, .OffsetX = 1, .OffsetY = 0},
    {.BitmapIndex = 459, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 5, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 463, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 469, .AdvWidth = 128, .BoxWidth = 2, .BoxHeight = 7, .OffsetX = 3, .OffsetY = 1},
    {.BitmapIndex = 471, .AdvWidth = 128, .BoxWidth = 6, .BoxHeight = 7, .OffsetX = 1, .OffsetY = 1},
    {.BitmapIndex = 477, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 2, .OffsetX = 0, .OffsetY = 6},
    {.BitmapIndex = 479, .AdvWidth = 128, .BoxWidth = 7, .BoxHeight = 7, .OffsetX = 0, .OffsetY = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

/*Collect the unicode lists and glyph_id offsets*/
static const EG_FontFmtTextCmap_t pCmaps[] = {
    {
        .RangeStart = 32, .RangeLength = 96, .GlyphIDStart = 1,
        .UnicodeList = NULL, .pGlyphIDOffsetList = NULL, .ListLength = 0, .Type = EG_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if EG_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  EG_FontFmtGlyphCache_t pCache;
static const EG_FontFmtTextProps_t Props = {
#else
static EG_FontFmtTextProps_t Props = {
#endif
    .GlyphBitmap = GlyphBitmap,
    .pGlyphProps = pGlyphProps,
    .pCmaps = pCmaps,
    .pKernProps = NULL,
    .KernScale = 0,
    .CmapNumber = 1,
    .BitsPerPixel = 1,
    .KernClasses = 0,
    .BitmapFormat = 0,
#if EG_VERSION_CHECK(8, 0, 0)
    .pCache = &pCache
#endif
};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if EG_VERSION_CHECK(8, 0, 0)
const EG_Font_t EG_FontUnscii8 = {
#else
EG_Font_t EG_FontUnscii8 = {
#endif
    .GetGlyphPropsCB = EG_FontGetGlyphPropsFmtText,    /*Function pointer to get glyph's data*/
    .GetGlyphBitmapCB = EG_FontGetBitmapFmtText,    /*Function pointer to get glyph's bitmap*/
    .LineHeight = 9,          /*The maximum line height required by the font*/
    .BaseLine = 0,             /*Baseline measured from the bottom of the line*/
#if !(EG_VERSION_MAJOR == 6 && EG_VERSION_MINOR == 0)
    .SubPixel = EG_FONT_SUBPX_NONE,
#endif
#if EG_VERSION_CHECK(7, 4, 0) || EG_VERSION_MAJOR >= 8
    .UnderlinePosition = 0,
    .UnderlineThickness = 0,
#endif
    .pProperties = &Props,           /*The custom font data. Will be accessed by `GetGlyphBitmapCB/dsc` */
    .pFallback = NULL,
#if EG_USE_USER_DATA
    .pExtData = NULL,
#endif
};

#endif 
