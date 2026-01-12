/*
 *                LEGL 2025-2026 HydraSystems.
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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "EG_Font.h"


// This describes a glyph.
typedef struct {
#if EG_FONT_FMT_TXT_LARGE == 0
    uint32_t BitmapIndex : 20;     //  Start index of the bitmap. A font can be max 1 MB.
    uint32_t AdvWidth : 12;            //  Draw the next glyph after this width. 8.4 format (real_value * 16 is stored).
    uint8_t BoxWidth;                  //  Width of the glyph's bounding box
    uint8_t BoxHeight;                  //  Height of the glyph's bounding box
    int8_t OffsetX;                   //  x offset of the bounding box
    int8_t OffsetY;                   //  y offset of the bounding box. Measured from the top of the line
#else
    uint32_t BitmapIndex;          //  Start index of the bitmap. A font can be max 4 GB.
    uint32_t AdvWidth;                 //  Draw the next glyph after this width. 28.4 format (real_value * 16 is stored).
    uint16_t BoxWidth;                 //  Width of the glyph's bounding box
    uint16_t BoxHeight;                 //  Height of the glyph's bounding box
    int16_t OffsetX;                  //  x offset of the bounding box
    int16_t OffsetY;                  //  y offset of the bounding box. Measured from the top of the line
#endif
} EG_FontFmtTextGlyphProps_t;

// Format of font character map.
typedef enum : uint8_t {
    EG_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    EG_FONT_FMT_TXT_CMAP_SPARSE_FULL,
    EG_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    EG_FONT_FMT_TXT_CMAP_SPARSE_TINY,
} EG_FontFmtTextCmapType_t;


/**
 * Map codepoints to a `pGlyphProps`s
 * Several formats are supported to optimize memory usage
 * See https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md
 */
typedef struct {
    uint32_t RangeStart;    // First Unicode character for this range
    uint16_t RangeLength;   // Number of Unicode characters related to this range.
    uint16_t GlyphIDStart;  // First glyph ID (array index of `pGlyphProps`) for this range
    /* According the specification there are 4 formats:
        https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md

    For simplicity introduce "relative code point":
        rcp = codepoint - RangeStart

    and a search function:
        search a "value" in an "array" and returns the index of "value".

    Format 0 tiny
        UnicodeList == NULL && pGlyphIDOffsetList == NULL
        glyph_id = GlyphIDStart + rcp

    Format 0 full
        UnicodeList == NULL && pGlyphIDOffsetList != NULL
        glyph_id = GlyphIDStart + pGlyphIDOffsetList[rcp]

    Sparse tiny
        UnicodeList != NULL && pGlyphIDOffsetList == NULL
        glyph_id = GlyphIDStart + search(UnicodeList, rcp)

    Sparse full
        UnicodeList != NULL && pGlyphIDOffsetList != NULL
        glyph_id = GlyphIDStart + pGlyphIDOffsetList[search(UnicodeList, rcp)]  */
    const uint16_t * UnicodeList;
    const void *pGlyphIDOffsetList;
    uint16_t    ListLength;   // Length of `UnicodeList` and/or `pGlyphIDOffsetList`
    uint8_t     Type;         // Type of this character map
} EG_FontFmtTextCmap_t;

// A simple mapping of kern values from pairs
typedef struct {
    /*To get a kern value of two code points:
       1. Get the `glyph_id_left` and `glyph_id_right` from `EG_FontFmtTextCmap_t
       2. for(i = 0; i < pair_cnt * 2; i += 2)
             if(gylph_ids[i] == glyph_id_left &&
                gylph_ids[i+1] == glyph_id_right)
                 return values[i / 2];
     */
    const void * glyph_ids;
    const int8_t * values;
    uint32_t pair_cnt   : 30;
    uint32_t glyph_ids_size : 2;    // 0: `glyph_ids` is stored as `uint8_t`; 1: as `uint16_t`
} EG_FontFmtKernPair_t;

// More complex but more optimal class based kern value storage
typedef struct {
    /*To get a kern value of two code points:
          1. Get the `glyph_id_left` and `glyph_id_right` from `EG_FontFmtTextCmap_t
          2. Get the class of the left and right glyphs as `left_class` and `right_class`
              left_class = left_class_mapping[glyph_id_left];
              right_class = right_class_mapping[glyph_id_right];
          3. value = class_pair_values[(left_class-1)*right_class_cnt + (right_class-1)]
        */

    const int8_t * class_pair_values;    // left_class_cnt * right_class_cnt value
    const uint8_t * left_class_mapping;   // Map the glyph_ids to classes: index -> glyph_id -> class_id
    const uint8_t * right_class_mapping;  // Map the glyph_ids to classes: index -> glyph_id -> class_id
    uint8_t left_class_cnt;
    uint8_t right_class_cnt;
} EG_FontFmtKernClasses_t;

// Bitmap formats
typedef enum : uint8_t {
    EG_FONT_FMT_TXT_PLAIN      = 0,
    EG_FONT_FMT_TXT_COMPRESSED = 1,
    EG_FONT_FMT_TXT_COMPRESSED_NO_PREFILTER = 1,
} EG_FontBitmapFormat_e;

typedef struct {
    uint32_t last_letter;
    uint32_t last_glyph_id;
} EG_FontFmtGlyphCache_t;

// Describe store additional data for fonts
typedef struct {
    const uint8_t * GlyphBitmap;                      // The bitmaps of all glyphs
    const EG_FontFmtTextGlyphProps_t *pGlyphProps;    // Describe the glyphs
    const EG_FontFmtTextCmap_t *pCmaps;               // Map the glyphs to Unicode characters.
    const void * pKernProps;                          // Store kerning values. 
    uint16_t KernScale;                               // Scale kern values in 12.4 format
    uint16_t CmapNumber     : 9;                      // Number of cmap tables
    uint16_t BitsPerPixel   : 4;                      // Bit per pixel: 1, 2, 3, 4, 8
    uint16_t KernClasses    : 1;                      // Type of `pKernProps`
    uint16_t BitmapFormat   : 2;                      // storage format of the bitmap 
    EG_FontFmtGlyphCache_t *pCache;            // Cache the last letter and is glyph id
} EG_FontFmtTextProps_t;


/**
 * Used as `GetGlyphBitmapCB` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font pointer to font
 * @param unicode_letter a unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t * EG_FontGetBitmapFmtText(const EG_Font_t *pFont, uint32_t letter);

/**
 * Used as `GetGlyphPropsCB` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter a UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
bool EG_FontGetGlyphPropsFmtText(const EG_Font_t *pFont, EG_FontGlyphProps_t * dsc_out, uint32_t unicode_letter,
                                   uint32_t unicode_letter_next);

// Free the allocated memories.
void EG_FontCleanUpFmtText(void);

