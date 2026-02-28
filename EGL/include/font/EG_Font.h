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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "EG_SymbolDef.h"
#include "../misc/EG_Rect.h"

/////////////////////////////////////////////////////////////////////////////////////////

//  imgfont identifier 
#define EG_IMGFONT_BPP 9

struct EG_Font_t;
// Describes the properties of a glyph.
typedef struct {
    const struct EG_Font_t *ResolvedFont; //  Pointer to a font where the glyph was actually found after handling callbacks
    uint16_t AdvWidth; //  The glyph needs this space. Draw the next glyph after this width.
    uint16_t BoxWidth; //  Width of the glyph's bounding box
    uint16_t BoxHeight; //  Height of the glyph's bounding box
    int16_t OffsetX;  //  x offset of the bounding box
    int16_t OffsetY;  //  y offset of the bounding box
    uint8_t BitsPerPixel: 4;  //  Bit-per-pixel: 1, 2, 4, 8
    uint8_t IsPlaceholder: 1; // * Glyph is missing. But placeholder will still be displayed 
} EG_FontGlyphProps_t;

// The bitmaps might be upscaled by 3 to achieve subpixel rendering.
typedef enum : uint8_t {
    EG_FONT_SUBPX_NONE,
    EG_FONT_SUBPX_HOR,
    EG_FONT_SUBPX_VER,
    EG_FONT_SUBPX_BOTH,
} EG_SubPixelMode_e;

typedef uint8_t EG_FontSubPixel_t;

// Describe the properties of a font
typedef struct EG_Font_t {
    bool (*GetGlyphPropsCB)(const struct EG_Font_t *, EG_FontGlyphProps_t *, uint32_t , uint32_t);  // Get a glyph's properties from a font
    const uint8_t * (*GetGlyphBitmapCB)(const struct EG_Font_t *, uint32_t);   // Get a glyph's bitmap from a font
    // Pointer to the font in a font pack (must have the same line height)
    EG_Coord_t  LineHeight;             //  The real line height where any text fits
    EG_Coord_t  BaseLine;               //  Base line measured from the top of the LineHeight
    uint8_t     SubPixel  : 2;          //  An element of `EG_FontSubPixel_t`

    int8_t UnderlinePosition;           //  Distance between the top of the underline and base line (< 0 means below the base line)
    int8_t UnderlineThickness;          //  Thickness of the underline

    const void  *pProperties;           //  Store implementation specific or run_time data or caching here
    const struct EG_Font_t *pFallback;  //  Fallback font for missing glyph. Resolved recursively 
#if EG_USE_EXT_DATA
    void        *pExtData;              //  Custom user data for font.
#endif
} EG_Font_t;

/////////////////////////////////////////////////////////////////////////////////////////

// Return with the bitmap of a font.
const uint8_t * EG_FontGetGlyphBitmap(const EG_Font_t *pFont, uint32_t Character);

bool EG_FontGetGlyphProps(const EG_Font_t *pFont, EG_FontGlyphProps_t *pProps, uint32_t Character, uint32_t NextCharactert);

// Get the width of a glyph with kerning
uint16_t EG_FontGetGlyphWidth(const EG_Font_t *pFont, uint32_t Character, uint32_t NextCharacter);

// Get the line height of a font. All characters fit into this height
 static inline EG_Coord_t EG_FontGetLineHeight(const EG_Font_t *pFont)
{
    return pFont->LineHeight;
}

#define EG_FONT_DECLARE(font_name) extern const EG_Font_t font_name;

#if EG_FONT_MONTSERRAT_8
EG_FONT_DECLARE(EG_FontMontserrat8)
#endif

#if EG_FONT_MONTSERRAT_10
EG_FONT_DECLARE(EG_FontMontserrat10)
#endif

#if EG_FONT_MONTSERRAT_12
EG_FONT_DECLARE(EG_FontMontserrat12)
#endif

#if EG_FONT_MONTSERRAT_14
EG_FONT_DECLARE(EG_FontMontserrat14)
#endif

#if EG_FONT_MONTSERRAT_16
EG_FONT_DECLARE(EG_FontMontserrat16)
#endif

#if EG_FONT_MONTSERRAT_18
EG_FONT_DECLARE(EG_FontMontserrat18)
#endif

#if EG_FONT_MONTSERRAT_20
EG_FONT_DECLARE(EG_FontMontserrat20)
#endif

#if EG_FONT_MONTSERRAT_22
EG_FONT_DECLARE(EG_FontMontserrat22)
#endif

#if EG_FONT_MONTSERRAT_24
EG_FONT_DECLARE(EG_FontMontserrat24)
#endif

#if EG_FONT_MONTSERRAT_26
EG_FONT_DECLARE(EG_FontMontserrat26)
#endif

#if EG_FONT_MONTSERRAT_28
EG_FONT_DECLARE(EG_FontMontserrat28)
#endif

#if EG_FONT_MONTSERRAT_30
EG_FONT_DECLARE(EG_FontMontserrat30)
#endif

#if EG_FONT_MONTSERRAT_32
EG_FONT_DECLARE(EG_FontMontserrat32)
#endif

#if EG_FONT_MONTSERRAT_34
EG_FONT_DECLARE(EG_FontMontserrat34)
#endif

#if EG_FONT_MONTSERRAT_36
EG_FONT_DECLARE(EG_FontMontserrat36)
#endif

#if EG_FONT_MONTSERRAT_38
EG_FONT_DECLARE(EG_FontMontserrat38)
#endif

#if EG_FONT_MONTSERRAT_40
EG_FONT_DECLARE(EG_FontMontserrat40)
#endif

#if EG_FONT_MONTSERRAT_42
EG_FONT_DECLARE(EG_FontMontserrat42)
#endif

#if EG_FONT_MONTSERRAT_44
EG_FONT_DECLARE(EG_FontMontserrat44)
#endif

#if EG_FONT_MONTSERRAT_46
EG_FONT_DECLARE(EG_FontMontserrat46)
#endif

#if EG_FONT_MONTSERRAT_48
EG_FONT_DECLARE(EG_FontMontserrat48)
#endif

#if EG_FONT_MONTSERRAT_12_SUBPX
EG_FONT_DECLARE(EG_FontMontserrat12_SubPx)
#endif

#if EG_FONT_MONTSERRAT_28_COMPRESSED
EG_FONT_DECLARE(EG_FontMontserrat12_Compressed)
#endif

#if EG_FONT_DEJAVU_16_PERSIAN_HEBREW
EG_FONT_DECLARE(EG_FontDejavu16PersianHebrew)
#endif

#if EG_FONT_SIMSUN_16_CJK
EG_FONT_DECLARE(EG_FontSimSun16_cjk)
#endif

#if EG_FONT_UNSCII_8
EG_FONT_DECLARE(EG_FontUnscii8)
#endif

#if EG_FONT_UNSCII_16
EG_FONT_DECLARE(EG_FontUnscii16)
#endif

// Declare the custom (user defined) fonts
#ifdef EG_FONT_CUSTOM_DECLARE
EG_FONT_CUSTOM_DECLARE
#endif

/**
 * Just a wrapper around EG_FONT_DEFAULT because it might be more convenient to use a function in some cases
 * @return  pointer to EG_FONT_DEFAULT
 */
static inline const EG_Font_t* EG_DefaultFont(void)
{
    return EG_FONT_DEFAULT;
}

