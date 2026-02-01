/*
 *                EGL 2025-2026 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or   
 *  modify it under the terms of the GNU General Public License as  
 *  published by the Free Software Foundation; either version 2 of  
 *  the License, or (at your option) any later version.             
 *                                                                  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of  
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   
 *  GNU General Public License for more details.                    
 * 
 *  Based on a design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include "EG_Assert.h"
#include "EG_Math.h"
#include "EG_Types.h"

/*Error checking*/
#if EG_COLOR_DEPTH == 24
#error "EG_COLOR_DEPTH 24 is deprecated. Use EG_COLOR_DEPTH 32 instead (EG_Config.h)"
#endif

#if EG_COLOR_DEPTH != 16 && EG_COLOR_16_SWAP != 0
#error "EG_COLOR_16_SWAP requires EG_COLOR_DEPTH == 16. Set it in EG_Config.h"
#endif

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////

EG_EXPORT_CONST_INT(EG_COLOR_DEPTH);
EG_EXPORT_CONST_INT(EG_COLOR_16_SWAP);

#if EG_COLOR_DEPTH == 1
#define EG_COLOR_SIZE 8
#elif EG_COLOR_DEPTH == 8
#define EG_COLOR_SIZE 8
#elif EG_COLOR_DEPTH == 16
#define EG_COLOR_SIZE 16
#elif EG_COLOR_DEPTH == 32
#define EG_COLOR_SIZE 32
#else
#error "Invalid EG_COLOR_DEPTH in EG_Config.h! Set it to 1, 8, 16 or 32!"
#endif

#if defined(__cplusplus) && !defined(_EG_COLOR_HAS_MODERN_CPP)
/**
* MSVC compiler's definition of the __cplusplus indicating 199711L regardless to C++ standard version
* see https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-cplusplus
* so we use _MSC_VER macro instead of __cplusplus
*/
#ifdef _MSC_VER
#if _MSC_VER >= 1900 /*Visual Studio 2015*/
#define _EG_COLOR_HAS_MODERN_CPP 1
#endif
#else
#if __cplusplus >= 201103L
#define _EG_COLOR_HAS_MODERN_CPP 1
#endif
#endif
#endif /*__cplusplus*/

#ifndef _EG_COLOR_HAS_MODERN_CPP
#define _EG_COLOR_HAS_MODERN_CPP 0
#endif

#if _EG_COLOR_HAS_MODERN_CPP == 1
//Fix msvc compiler error C4576 inside C++ code
#define _EG_COLOR_MAKE_TYPE_HELPER EG_Color_t
#else
#define _EG_COLOR_MAKE_TYPE_HELPER (EG_Color_t)
#endif

/*---------------------------------------
 * Macros for all existing color depths
 * to set/get values of the color channels
 *------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////

#define EG_COLOR_SET_R1(c, v) (c).ch.red = (uint8_t)((v)&0x1)
#define EG_COLOR_SET_G1(c, v) (c).ch.green = (uint8_t)((v)&0x1)
#define EG_COLOR_SET_B1(c, v) (c).ch.blue = (uint8_t)((v)&0x1)
#define EG_COLOR_SET_A1(c, v)	do {} while(0)

#define EG_COLOR_GET_R1(c) (c).ch.red
#define EG_COLOR_GET_G1(c) (c).ch.green
#define EG_COLOR_GET_B1(c) (c).ch.blue
#define EG_COLOR_GET_A1(c) 0xFF

#define _EG_COLOR_ZERO_INITIALIZER1 {0x00}
#define EG_COLOR_MAKE1(r8, g8, b8){(uint8_t)((b8 >> 7) | (g8 >> 7) | (r8 >> 7))}

#define EG_COLOR_SET_R8(c, v) (c).ch.red = (uint8_t)((v)&0x7U)
#define EG_COLOR_SET_G8(c, v) (c).ch.green = (uint8_t)((v)&0x7U)
#define EG_COLOR_SET_B8(c, v) (c).ch.blue = (uint8_t)((v)&0x3U)
#define EG_COLOR_SET_A8(c, v) do {} while(0)

#define EG_COLOR_GET_R8(c) (c).ch.red
#define EG_COLOR_GET_G8(c) (c).ch.green
#define EG_COLOR_GET_B8(c) (c).ch.blue
#define EG_COLOR_GET_A8(c) 0xFF

#define _EG_COLOR_ZERO_INITIALIZER8 {{0x00, 0x00, 0x00}}
#define EG_COLOR_MAKE8(r8, g8, b8){{(uint8_t)((b8 >> 6) & 0x3U), (uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 5) & 0x7U)}}

/////////////////////////////////////////////////////////////////////////////

#define EG_COLOR_SET_R16(c, v) (c).ch.red = (uint8_t)((v)&0x1FU)
#if EG_COLOR_16_SWAP == 0
#define EG_COLOR_SET_G16(c, v) (c).ch.green = (uint8_t)((v)&0x3FU)
#else
#define EG_COLOR_SET_G16(c, v){(c).ch.green_h = (uint8_t)(((v) >> 3) & 0x7); (c).ch.green_l = (uint8_t)((v)&0x7); }
#endif
#define EG_COLOR_SET_B16(c, v) (c).ch.blue = (uint8_t)((v)&0x1FU)
#define EG_COLOR_SET_A16(c, v) do {} while(0)

#define EG_COLOR_GET_R16(c) (c).ch.red
#if EG_COLOR_16_SWAP == 0
#define EG_COLOR_GET_G16(c) (c).ch.green
#else
#define EG_COLOR_GET_G16(c) (((c).ch.green_h << 3) + (c).ch.green_l)
#endif
#define EG_COLOR_GET_B16(c) (c).ch.blue
#define EG_COLOR_GET_A16(c) 0xFF

#if EG_COLOR_16_SWAP == 0
#define _EG_COLOR_ZERO_INITIALIZER16 {{0x00, 0x00, 0x00 }}
#define EG_COLOR_MAKE16(r8, g8, b8){{(uint8_t)((b8 >> 3) & 0x1FU), (uint8_t)((g8 >> 2) & 0x3FU), (uint8_t)((r8 >> 3) & 0x1FU)}}
#else
#define _EG_COLOR_ZERO_INITIALIZER16 {{0x00, 0x00, 0x00, 0x00}}
#define EG_COLOR_MAKE16(r8, g8, b8){{(uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 3) & 0x1FU), (uint8_t)((b8 >> 3) & 0x1FU), (uint8_t)((g8 >> 2) & 0x7U)}}
#endif

/////////////////////////////////////////////////////////////////////////////

#define EG_COLOR_SET_R32(c, v) (c).ch.red = (uint8_t)((v)&0xFF)
#define EG_COLOR_SET_G32(c, v) (c).ch.green = (uint8_t)((v)&0xFF)
#define EG_COLOR_SET_B32(c, v) (c).ch.blue = (uint8_t)((v)&0xFF)
#define EG_COLOR_SET_A32(c, v) (c).ch.alpha = (uint8_t)((v)&0xFF)

#define EG_COLOR_GET_R32(c) (c).ch.red
#define EG_COLOR_GET_G32(c) (c).ch.green
#define EG_COLOR_GET_B32(c) (c).ch.blue
#define EG_COLOR_GET_A32(c) (c).ch.alpha

#define _EG_COLOR_ZERO_INITIALIZER32 {{0x00, 0x00, 0x00, 0x00}}
#define EG_COLOR_MAKE32(r8, g8, b8) {{b8, g8, r8, 0xff}}  // Fixed 0xff alpha

/////////////////////////////////////////////////////////////////////////////

/*---------------------------------------
 * Macros for the current color depth
 * to set/get values of the color channels
 *------------------------------------------*/
#define EG_COLOR_SET_R(c, v) EG_CONCAT(EG_COLOR_SET_R, EG_COLOR_DEPTH)(c, v)
#define EG_COLOR_SET_G(c, v) EG_CONCAT(EG_COLOR_SET_G, EG_COLOR_DEPTH)(c, v)
#define EG_COLOR_SET_B(c, v) EG_CONCAT(EG_COLOR_SET_B, EG_COLOR_DEPTH)(c, v)
#define EG_COLOR_SET_A(c, v) EG_CONCAT(EG_COLOR_SET_A, EG_COLOR_DEPTH)(c, v)

#define EG_COLOR_GET_R(c) EG_CONCAT(EG_COLOR_GET_R, EG_COLOR_DEPTH)(c)
#define EG_COLOR_GET_G(c) EG_CONCAT(EG_COLOR_GET_G, EG_COLOR_DEPTH)(c)
#define EG_COLOR_GET_B(c) EG_CONCAT(EG_COLOR_GET_B, EG_COLOR_DEPTH)(c)
#define EG_COLOR_GET_A(c) EG_CONCAT(EG_COLOR_GET_A, EG_COLOR_DEPTH)(c)

#define _EG_COLOR_ZERO_INITIALIZER EG_CONCAT(_EG_COLOR_ZERO_INITIALIZER, EG_COLOR_DEPTH)
#define EG_COLOR_MAKE(r8, g8, b8) EG_CONCAT(EG_COLOR_MAKE, EG_COLOR_DEPTH)(r8, g8, b8)

/////////////////////////////////////////////////////////////////////////////

typedef union {
	uint8_t full; /*must be declared first to set all bits of byte via initializer list*/
	union {
		uint8_t blue : 1;
		uint8_t green : 1;
		uint8_t red : 1;
	} ch;
} EG_Color1_t;

typedef union {
	struct {
		uint8_t blue : 2;
		uint8_t green : 3;
		uint8_t red : 3;
	} ch;
	uint8_t full;
} EG_Color8_t;

typedef union {
	struct {
#if EG_COLOR_16_SWAP == 0
		uint16_t blue : 5;
		uint16_t green : 6;
		uint16_t red : 5;
#else
		uint16_t green_h : 3;
		uint16_t red : 5;
		uint16_t blue : 5;
		uint16_t green_l : 3;
#endif
	} ch;
	uint16_t full;
} EG_Color16_t;

typedef union {
	struct {
		uint8_t blue;
		uint8_t green;
		uint8_t red;
		uint8_t alpha;
	} ch;
	uint32_t full;
} EG_Color32_t;

typedef EG_CONCAT3(uint, EG_COLOR_SIZE, _t) EG_ColorInt_t;
typedef EG_CONCAT3(EG_Color, EG_COLOR_DEPTH, _t) EG_Color_t;

typedef struct {
	uint16_t Hue;
	uint8_t Sat;
	uint8_t Val;
} EG_ColorHSV_t;

typedef uint8_t EG_OPA_t;

struct EG_ColorFilterProps_t;

typedef EG_Color_t (*EG_ColorFilterCB_t)(const struct EG_ColorFilterProps_t *, EG_Color_t, EG_OPA_t);

typedef struct EG_ColorFilterProps_t {
	EG_ColorFilterProps_t(void) : FilterCB(nullptr), pParam(nullptr){};
	EG_ColorFilterCB_t FilterCB;
	void *pParam;
} EG_ColorFilterProps_t;

// Opacity percentages.
enum {
	EG_OPA_TRANSP = 0,
	EG_OPA_0 = 0,
	EG_OPA_MIN = 2,      // Opacities below this will be transparent
	EG_OPA_10 = 25,
	EG_OPA_20 = 51,
	EG_OPA_30 = 76,
	EG_OPA_40 = 102,
	EG_OPA_50 = 127,
	EG_OPA_60 = 153,
	EG_OPA_70 = 178,
	EG_OPA_80 = 204,
	EG_OPA_90 = 229,
	EG_OPA_MAX = 253,       // Opacities above this will fully cover
	EG_OPA_100 = 255,
	EG_OPA_COVER = 255,
};


typedef enum : uint8_t {
	EG_PALETTE_RED,          // 00
	EG_PALETTE_PINK,         // 01
	EG_PALETTE_PURPLE,       // 02
	EG_PALETTE_DEEP_PURPLE,  // 03
	EG_PALETTE_INDIGO,       // 04
	EG_PALETTE_BLUE,         // 05
	EG_PALETTE_LIGHT_BLUE,   // 06
	EG_PALETTE_CYAN,         // 07
	EG_PALETTE_TEAL,         // 08
	EG_PALETTE_GREEN,        // 09
	EG_PALETTE_LIGHT_GREEN,  // 10
	EG_PALETTE_LIME,         // 11
	EG_PALETTE_YELLOW,       // 12
	EG_PALETTE_AMBER,        // 13
	EG_PALETTE_ORANGE,       // 14
	EG_PALETTE_DEEP_ORANGE,  // 15
	EG_PALETTE_BROWN,        // 16
	EG_PALETTE_BLUE_GREY,    // 17
	EG_PALETTE_GREY,         // 18
	EG_PALETTE_LAST,         // 19
	EG_PALETTE_NONE = 0xff,
} EG_Palette_e;

// Image color format
typedef enum EG_ImageColorFormat_t : uint8_t{
    EG_COLOR_FORMAT_UNKNOWN = 0,

    EG_COLOR_FORMAT_RAW,              // Contains the file as it is. Needs custom decoder function
    EG_COLOR_FORMAT_RAW_ALPHA,        // Contains the file as it is. The image has alpha. Needs custom decoder function
    EG_COLOR_FORMAT_RAW_CHROMA_KEYED, // Contains the file as it is. The image is chroma keyed. Needs custom decoder function

    EG_COLOR_FORMAT_NATIVE,               // Color format and depth should match with EG_COLOR settings
    EG_COLOR_FORMAT_NATIVE_ALPHA,         // Same as `EG_COLOR_FORMAT_NATIVE` but every pixel has an alpha byte
    EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED,  // Same as `EG_COLOR_FORMAT_NATIVE` but EG_COLOR_TRANSP pixels will be transparent

    EG_COLOR_FORMAT_INDEXED_1BIT,             // Can have 2 different colors in a palette (can't be chroma keyed)
    EG_COLOR_FORMAT_INDEXED_2BIT,             // Can have 4 different colors in a palette (can't be chroma keyed)
    EG_COLOR_FORMAT_INDEXED_4BIT,             // Can have 16 different colors in a palette (can't be chroma keyed)
    EG_COLOR_FORMAT_INDEXED_8BIT,             // Can have 256 different colors in a palette (can't be chroma keyed)

    EG_COLOR_FORMAT_ALPHA_1BIT,               // Can have one color and it can be drawn or not
    EG_COLOR_FORMAT_ALPHA_2BIT,               // Can have one color but 4 different alpha value
    EG_COLOR_FORMAT_ALPHA_4BIT,               // Can have one color but 16 different alpha value
    EG_COLOR_FORMAT_ALPHA_8BIT,               // Can have one color but 256 different alpha value

    EG_COLOR_FORMAT_RGB888,
    EG_COLOR_FORMAT_RGBA8888,
    EG_COLOR_FORMAT_RGBX8888,
    EG_COLOR_FORMAT_RGB565,
    EG_COLOR_FORMAT_RGBA5658,
    EG_COLOR_FORMAT_RGB565A8,

    EG_COLOR_FORMAT_RESERVED_15,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_16,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_17,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_18,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_19,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_20,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_21,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_22,              // Reserved for further use.
    EG_COLOR_FORMAT_RESERVED_23,              // Reserved for further use.

    EG_COLOR_FORMAT_USER_ENCODED_0,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_1,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_2,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_3,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_4,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_5,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_6,           // User holder encoding format.
    EG_COLOR_FORMAT_USER_ENCODED_7,           // User holder encoding format.
} EG_ImageColorFormat_t;

/////////////////////////////////////////////////////////////////////////////

/*In color conversations:
 * - When converting to bigger color type the LSB weight of 1 LSB is calculated
 *   E.g. 16 bit Red has 5 bits
 *         8 bit Red has 3 bits
 *        ----------------------
 *        8 bit red LSB = (2^5 - 1) / (2^3 - 1) = 31 / 7 = 4
 *
 * - When calculating to smaller color type simply shift out the LSBs
 *   E.g.  8 bit Red has 3 bits
 *        16 bit Red has 5 bits
 *        ----------------------
 *         Shift right with 5 - 3 = 2
 */
static inline uint8_t EG_ColorTo1(EG_Color_t Color)
{
#if EG_COLOR_DEPTH == 1
	return Color.full;
#elif EG_COLOR_DEPTH == 8
	if((EG_COLOR_GET_R(Color) & 0x4) || (EG_COLOR_GET_G(Color) & 0x4) || (EG_COLOR_GET_B(Color) & 0x2)) {
		return 1;
	}
	else {
		return 0;
	}
#elif EG_COLOR_DEPTH == 16
	if((EG_COLOR_GET_R(Color) & 0x10) || (EG_COLOR_GET_G(Color) & 0x20) || (EG_COLOR_GET_B(Color) & 0x10)) {
		return 1;
	}
	else {
		return 0;
	}
#elif EG_COLOR_DEPTH == 32
	if((EG_COLOR_GET_R(Color) & 0x80) || (EG_COLOR_GET_G(Color) & 0x80) || (EG_COLOR_GET_B(Color) & 0x80)) {
		return 1;
	}
	else {
		return 0;
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////

static inline uint8_t EG_ColorTo8(EG_Color_t Color)
{
#if EG_COLOR_DEPTH == 1
	if(Color.full == 0)
		return 0;
	else
		return 0xFF;
#elif EG_COLOR_DEPTH == 8
	return Color.full;
#elif EG_COLOR_DEPTH == 16
	EG_Color8_t ColorOut;
	EG_COLOR_SET_R8(ColorOut, EG_COLOR_GET_R(Color) >> 2); /*5 - 3  = 2*/
	EG_COLOR_SET_G8(ColorOut, EG_COLOR_GET_G(Color) >> 3); /*6 - 3  = 3*/
	EG_COLOR_SET_B8(ColorOut, EG_COLOR_GET_B(Color) >> 3); /*5 - 2  = 3*/
	return ColorOut.full;
#elif EG_COLOR_DEPTH == 32
	EG_Color8_t ColorOut;
	EG_COLOR_SET_R8(ColorOut, EG_COLOR_GET_R(Color) >> 5); /*8 - 3  = 5*/
	EG_COLOR_SET_G8(ColorOut, EG_COLOR_GET_G(Color) >> 5); /*8 - 3  = 5*/
	EG_COLOR_SET_B8(ColorOut, EG_COLOR_GET_B(Color) >> 6); /*8 - 2  = 6*/
	return ColorOut.full;
#endif
}

/////////////////////////////////////////////////////////////////////////////

static inline uint16_t EG_ColorTo16(EG_Color_t Color)
{
#if EG_COLOR_DEPTH == 1
	if(Color.full == 0)
		return 0;
	else
		return 0xFFFF;
#elif EG_COLOR_DEPTH == 8
	EG_Color16_t ColorOut;
	EG_COLOR_SET_R16(ColorOut, EG_COLOR_GET_R(Color) * 4);  /*(2^5 - 1)/(2^3 - 1) = 31/7 = 4*/
	EG_COLOR_SET_G16(ColorOut, EG_COLOR_GET_G(Color) * 9);  /*(2^6 - 1)/(2^3 - 1) = 63/7 = 9*/
	EG_COLOR_SET_B16(ColorOut, EG_COLOR_GET_B(Color) * 10); /*(2^5 - 1)/(2^2 - 1) = 31/3 = 10*/
	return ColorOut.full;
#elif EG_COLOR_DEPTH == 16
	return Color.full;
#elif EG_COLOR_DEPTH == 32
	EG_Color16_t ColorOut;
	EG_COLOR_SET_R16(ColorOut, EG_COLOR_GET_R(Color) >> 3); /*8 - 5  = 3*/
	EG_COLOR_SET_G16(ColorOut, EG_COLOR_GET_G(Color) >> 2); /*8 - 6  = 2*/
	EG_COLOR_SET_B16(ColorOut, EG_COLOR_GET_B(Color) >> 3); /*8 - 5  = 3*/
	return ColorOut.full;
#endif
}

/////////////////////////////////////////////////////////////////////////////

static inline uint32_t EG_ColorTo32(EG_Color_t Color)
{
#if EG_COLOR_DEPTH == 1
	if(Color.full == 0)
		return 0xFF000000;
	else
		return 0xFFFFFFFF;
#elif EG_COLOR_DEPTH == 8
	EG_Color32_t ColorOut;
	EG_COLOR_SET_R32(ColorOut, EG_COLOR_GET_R(Color) * 36); /*(2^8 - 1)/(2^3 - 1) = 255/7 = 36*/
	EG_COLOR_SET_G32(ColorOut, EG_COLOR_GET_G(Color) * 36); /*(2^8 - 1)/(2^3 - 1) = 255/7 = 36*/
	EG_COLOR_SET_B32(ColorOut, EG_COLOR_GET_B(Color) * 85); /*(2^8 - 1)/(2^2 - 1) = 255/3 = 85*/
	EG_COLOR_SET_A32(ColorOut, 0xFF);
	return ColorOut.full;
#elif EG_COLOR_DEPTH == 16
	/**
     * The floating point math for conversion is:
     *  valueto = valuefrom * ( (2^bitsto - 1) / (float)(2^bitsfrom - 1) )
     * The faster integer math for conversion is:
     *  valueto = ( valuefrom * multiplier + adder ) >> divisor
     *   multiplier = FLOOR( ( (2^bitsto - 1) << divisor ) / (float)(2^bitsfrom - 1) )
     *
     * Find the first divisor where ( adder >> divisor ) <= 0
     *
     * 5-bit to 8-bit: ( 31 * multiplier + adder ) >> divisor = 255
     * divisor  multiplier  adder  min (0)  max (31)
     *       0           8      7        7       255
     *       1          16     14        7       255
     *       2          32     28        7       255
     *       3          65     25        3       255
     *       4         131     19        1       255
     *       5         263      7        0       255
     *
     * 6-bit to 8-bit: 255 = ( 63 * multiplier + adder ) >> divisor
     * divisor  multiplier  adder  min (0)  max (63)
     *       0           4      3        3       255
     *       1           8      6        3       255
     *       2          16     12        3       255
     *       3          32     24        3       255
     *       4          64     48        3       255
     *       5         129     33        1       255
     *       6         259      3        0       255
     */

	EG_Color32_t ColorOut;
	EG_COLOR_SET_R32(ColorOut, (EG_COLOR_GET_R(Color) * 263 + 7) >> 5);
	EG_COLOR_SET_G32(ColorOut, (EG_COLOR_GET_G(Color) * 259 + 3) >> 6);
	EG_COLOR_SET_B32(ColorOut, (EG_COLOR_GET_B(Color) * 263 + 7) >> 5);
	EG_COLOR_SET_A32(ColorOut, 0xFF);
	return ColorOut.full;
#elif EG_COLOR_DEPTH == 32
	return Color.full;
#endif
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Mix two colors with a given ratio.
 * @param Color1 the first color to Mix (usually the foreground)
 * @param Color2 the second color to Mix (usually the background)
 * @param Mix The ratio of the colors. 0: full `Color2`, 255: full `Color1`, 127: half `Color1` and half`Color2`
 * @return the mixed color
 */
static inline EG_Color_t EG_ATTRIBUTE_FAST_MEM EG_ColorMix(EG_Color_t Color1, EG_Color_t Color2, uint8_t Mix)
{
EG_Color_t ColorOut;

#if EG_COLOR_DEPTH == 16 && EG_COLOR_MIX_ROUND_OFS == 0
#if EG_COLOR_16_SWAP == 1
	Color1.full = Color1.full << 8 | Color1.full >> 8;
	Color2.full = Color2.full << 8 | Color2.full >> 8;
#endif
	// Source: https://stackoverflow.com/a/50012418/1999969
	Mix = (uint32_t)((uint32_t)Mix + 4) >> 3;
	uint32_t bg = (uint32_t)((uint32_t)Color2.full | ((uint32_t)Color2.full << 16)) &
		0x7E0F81F;      // 0b00000111111000001111100000011111
	uint32_t fg = (uint32_t)((uint32_t)Color1.full | ((uint32_t)Color1.full << 16)) & 0x7E0F81F;
	uint32_t result = ((((fg - bg) * Mix) >> 5) + bg) & 0x7E0F81F;
	ColorOut.full = (uint16_t)((result >> 16) | result);
#if EG_COLOR_16_SWAP == 1
	ColorOut.full = ColorOut.full << 8 | ColorOut.full >> 8;
#endif
#elif EG_COLOR_DEPTH != 1	// EG_COLOR_DEPTH == 8, 16 or 32
	EG_COLOR_SET_R(ColorOut, EG_UDIV255((uint16_t)EG_COLOR_GET_R(Color1) * Mix + EG_COLOR_GET_R(Color2) * (255 - Mix) + EG_COLOR_MIX_ROUND_OFS));
	EG_COLOR_SET_G(ColorOut, EG_UDIV255((uint16_t)EG_COLOR_GET_G(Color1) * Mix + EG_COLOR_GET_G(Color2) * (255 - Mix) + EG_COLOR_MIX_ROUND_OFS));
	EG_COLOR_SET_B(ColorOut, EG_UDIV255((uint16_t)EG_COLOR_GET_B(Color1) * Mix + EG_COLOR_GET_B(Color2) * (255 - Mix) + EG_COLOR_MIX_ROUND_OFS));
	EG_COLOR_SET_A(ColorOut, 0xFF);
#else                   	// EG_COLOR_DEPTH == 1
	ColorOut.full = (Mix > EG_OPA_50) ? Color1.full : Color2.full;
#endif
	return ColorOut;
}

/////////////////////////////////////////////////////////////////////////////

static inline void EG_ATTRIBUTE_FAST_MEM EG_ColorPreMultiply(EG_Color_t Color, uint8_t Mix, uint16_t *pOut)
{
#if EG_COLOR_DEPTH != 1
	pOut[0] = (uint16_t)EG_COLOR_GET_R(Color) * Mix;
	pOut[1] = (uint16_t)EG_COLOR_GET_G(Color) * Mix;
	pOut[2] = (uint16_t)EG_COLOR_GET_B(Color) * Mix;
#else
	(void)Mix;
	// Pre-multiplication can't be used with 1 BitsPerPixel
	pOut[0] = EG_COLOR_GET_R(Color);
	pOut[1] = EG_COLOR_GET_G(Color);
	pOut[2] = EG_COLOR_GET_B(Color);
#endif
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Mix two colors with a given ratio. It runs faster then `EG_ColorMix` but requires some pre computation.
 * @param premult_c1 The first color. Should be preprocessed with `EG_ColorPreMultiply(Color1)`
 * @param Color2 The second color. As it is no pre computation required on it
 * @param Mix The ratio of the colors. 0: full `Color1`, 255: full `Color2`, 127: half `Color1` and half `Color2`.
 *            Should be modified like Mix = `255 - Mix`
 * @return the mixed color
 * @note 255 won't give clearly `Color1`.
 */
static inline EG_Color_t EG_ATTRIBUTE_FAST_MEM EG_ColorMixPreMultiply(uint16_t *premult_c1, EG_Color_t Color2, uint8_t Mix)
{
	EG_Color_t ColorOut;
#if EG_COLOR_DEPTH != 1
	// EG_COLOR_DEPTH == 8 or 32
	EG_COLOR_SET_R(ColorOut, EG_UDIV255(premult_c1[0] + EG_COLOR_GET_R(Color2) * Mix + EG_COLOR_MIX_ROUND_OFS));
	EG_COLOR_SET_G(ColorOut, EG_UDIV255(premult_c1[1] + EG_COLOR_GET_G(Color2) * Mix + EG_COLOR_MIX_ROUND_OFS));
	EG_COLOR_SET_B(ColorOut, EG_UDIV255(premult_c1[2] + EG_COLOR_GET_B(Color2) * Mix + EG_COLOR_MIX_ROUND_OFS));
	EG_COLOR_SET_A(ColorOut, 0xFF);
#else
	// EG_COLOR_DEPTH == 1
	/*Restore color1*/
	EG_Color_t Color1;
	EG_COLOR_SET_R(Color1, premult_c1[0]);
	EG_COLOR_SET_G(Color1, premult_c1[1]);
	EG_COLOR_SET_B(Color1, premult_c1[2]);
	ColorOut.full = Mix > EG_OPA_50 ? Color2.full : Color1.full;
#endif

	return ColorOut;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Mix two colors. Both color can have alpha value.
 * @param bg_color background color
 * @param bg_opa alpha of the background color
 * @param fg_color foreground color
 * @param fg_opa alpha of the foreground color
 * @param res_color the result color
 * @param res_opa the result opacity
 */
static inline void EG_ATTRIBUTE_FAST_MEM EG_ColorMixWithAlpha(EG_Color_t bg_color, EG_OPA_t bg_opa,
																															EG_Color_t fg_color, EG_OPA_t fg_opa,
																															EG_Color_t *res_color, EG_OPA_t *res_opa)
{
	/*Pick the foreground if it's fully opaque or the Background is fully transparent*/
	if(fg_opa >= EG_OPA_MAX || bg_opa <= EG_OPA_MIN) {
		res_color->full = fg_color.full;
		*res_opa = fg_opa;
	}
	/*Transparent foreground: use the Background*/
	else if(fg_opa <= EG_OPA_MIN) {
		res_color->full = bg_color.full;
		*res_opa = bg_opa;
	}
	/*Opaque background: use simple Mix*/
	else if(bg_opa >= EG_OPA_MAX) {
		*res_color = EG_ColorMix(fg_color, bg_color, fg_opa);
		*res_opa = EG_OPA_COVER;
	}
	/*Both colors have alpha. Expensive calculation need to be applied*/
	else {
		/*Save the parameters and the result. If they will be asked again don't compute again*/
		static EG_OPA_t fg_opa_save = 0;
		static EG_OPA_t bg_opa_save = 0;
		static EG_Color_t fg_color_save = _EG_COLOR_ZERO_INITIALIZER;
		static EG_Color_t bg_color_save = _EG_COLOR_ZERO_INITIALIZER;
		static EG_Color_t res_color_saved = _EG_COLOR_ZERO_INITIALIZER;
		static EG_OPA_t res_opa_saved = 0;

		if(fg_opa != fg_opa_save || bg_opa != bg_opa_save || fg_color.full != fg_color_save.full ||
			 bg_color.full != bg_color_save.full) {
			fg_opa_save = fg_opa;
			bg_opa_save = bg_opa;
			fg_color_save.full = fg_color.full;
			bg_color_save.full = bg_color.full;
			/*Info:
             * https://en.wikipedia.org/wiki/Alpha_compositing#Analytical_derivation_of_the_over_operator*/
			res_opa_saved = 255 - ((uint16_t)((uint16_t)(255 - fg_opa) * (255 - bg_opa)) >> 8);
			EG_ASSERT(res_opa_saved != 0);
			EG_OPA_t ratio = (uint16_t)((uint16_t)fg_opa * 255) / res_opa_saved;
			res_color_saved = EG_ColorMix(fg_color, bg_color, ratio);
		}

		res_color->full = res_color_saved.full;
		*res_opa = res_opa_saved;
	}
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Get the brightness of a color
 * @param Color to get the brightness of
 * @return the brightness [0..255]
 */
static inline uint8_t EG_ColorGetBrightness(EG_Color_t Color)
{
	EG_Color32_t Color32;
	Color32.full = EG_ColorTo32(Color);
	uint16_t Bright = (uint16_t)(3u * EG_COLOR_GET_R32(Color32) + EG_COLOR_GET_B32(Color32) + 4u * EG_COLOR_GET_G32(Color32));
	return (uint8_t)(Bright >> 3);
}

/////////////////////////////////////////////////////////////////////////////

static inline EG_Color_t EG_MixColor(uint8_t r, uint8_t g, uint8_t b)
{
	return _EG_COLOR_MAKE_TYPE_HELPER EG_COLOR_MAKE(r, g, b);
}

static inline EG_Color_t EG_ColorHex(uint32_t Color)
{
#if EG_COLOR_DEPTH == 16
	EG_Color_t ColorOut;
#if EG_COLOR_16_SWAP == 0
	/* Convert 4 bytes per pixel in format ARGB32 to R5G6B5 format
        naive way (by calling EG_MixColor with components):
                    r = ((Color & 0xFF0000) >> 19)
                    g = ((Color & 0xFF00) >> 10)
                    b = ((Color & 0xFF) >> 3)
                    rgb565 = (r << 11) | (g << 5) | b
        That's 3 mask, 5 bitshift and 2 or operations

        A better solution:
                    r = ((Color & 0xF80000) >> 8)
                    g = ((Color & 0xFC00) >> 5)
                    b = ((Color & 0xFF) >> 3)
                    rgb565 = r | g | b
        That's 3 mask, 3 bitshifts and 2 or operations */
	ColorOut.full = (uint16_t)(((Color & 0xF80000) >> 8) | ((Color & 0xFC00) >> 5) | ((Color & 0xFF) >> 3));
#else
	/* We want: rrrr rrrr GGGg gggg bbbb bbbb => gggb bbbb rrrr rGGG */
	ColorOut.full = (uint16_t)(((Color & 0xF80000) >> 16) | ((Color & 0xFC00) >> 13) | ((Color & 0x1C00) << 3) | ((Color & 0xF8) << 5));
#endif
	return ColorOut;
#elif EG_COLOR_DEPTH == 32
	EG_Color_t ColorOut;
	ColorOut.full = Color | 0xFF000000;
	return ColorOut;
#else
	return EG_MixColor((uint8_t)((Color >> 16) & 0xFF), (uint8_t)((Color >> 8) & 0xFF), (uint8_t)(Color & 0xFF));
#endif
}

/////////////////////////////////////////////////////////////////////////////

static inline EG_Color_t EG_ColorHex3(uint32_t Color)
{
	return EG_MixColor((uint8_t)(((Color >> 4) & 0xF0) | ((Color >> 8) & 0xF)),
                     (uint8_t)((Color & 0xF0) | ((Color & 0xF0) >> 4)),
										 (uint8_t)((Color & 0xF) | ((Color & 0xF) << 4)));
}

/////////////////////////////////////////////////////////////////////////////

static inline void EG_ColorFilterInitialise(EG_ColorFilterProps_t *pFilter, EG_ColorFilterCB_t FilterCB)
{
	pFilter->FilterCB = FilterCB;
}

/////////////////////////////////////////////////////////////////////////////

void /*EG_ATTRIBUTE_FAST_MEM*/ EG_ColorFill(EG_Color_t *buf, EG_Color_t Color, uint32_t px_num);

EG_Color_t EG_LightenColor(EG_Color_t Color, EG_OPA_t Level);

EG_Color_t EG_DarkenColor(EG_Color_t Color, EG_OPA_t Level);

EG_Color_t EG_ChangeColorBrightness(EG_Color_t Color, EG_OPA_t Level);

/**
 * Convert a HSV color to RGB
 * @param h hue [0..359]
 * @param s saturation [0..100]
 * @param v value [0..100]
 * @return the given RGB color in RGB (with EG_COLOR_DEPTH depth)
 */
EG_Color_t EG_HSVToRGB(uint16_t h, uint8_t s, uint8_t v);

/**
 * Convert a 32-bit RGB color to HSV
 * @param r8 8-bit red
 * @param g8 8-bit green
 * @param b8 8-bit blue
 * @return the given RGB color in HSV
 */
EG_ColorHSV_t EG_RGBToHSV(uint8_t r8, uint8_t g8, uint8_t b8);

/**
 * Convert a color to HSV
 * @param color color
 * @return the given color in HSV
 */
EG_ColorHSV_t EG_ColorToHSV(EG_Color_t Color);

uint32_t EG_ColorToInt(EG_Color_t Color);

bool EG_ColorEqual(EG_Color_t Color1, EG_Color_t Color2);

bool EG_ColorEqual32(EG_Color_t Color1, EG_Color_t Color2);

/////////////////////////////////////////////////////////////////////////////

/**
 * Just a wrapper around EG_COLOR_CHROMA_KEY because it might be more convenient to use a function in some cases
 * @return EG_COLOR_CHROMA_KEY
 */
static inline EG_Color_t EG_ColorChromaKey(void)
{
	return EG_COLOR_CHROMA_KEY;
}

/////////////////////////////////////////////////////////////////////////////

/*Source: https://vuetifyjs.com/en/styles/colors/#material-colors*/

EG_Color_t EG_MainPalette(EG_Palette_e p);

EG_Color_t EG_LightPalette(EG_Palette_e p, uint8_t Level);

EG_Color_t EG_DarkPalette(EG_Palette_e p, uint8_t Level);

static inline EG_Color_t EG_ColorWhite(void)
{
	return EG_MixColor(0xff, 0xff, 0xff);
}

static inline EG_Color_t EG_ColorBlack(void)
{
	return EG_MixColor(0x00, 0x0, 0x00);
}
