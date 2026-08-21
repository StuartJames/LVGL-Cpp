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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "EGL.h"

#if EG_USE_FREETYPE

enum EG_FreeTypeFontStyle_e{
    FT_FONT_STYLE_NORMAL = 0,
    FT_FONT_STYLE_ITALIC = 1 << 0,
    FT_FONT_STYLE_BOLD   = 1 << 1
} ;

typedef struct {
    const char * name;  /* The name of the font file */
    const void * mem;   /* The pointer of the font file */
    size_t mem_size;    /* The size of the memory */
    EG_Font_t * font;   /* point to lvgl font */
    uint16_t weight;    /* font size */
    uint16_t style;     /* font style */
} lv_ft_info_t;

/**
 * init freetype library
 * @param max_faces Maximum number of opened FT_Face objects managed by this cache instance. Use 0 for defaults.
 * @param max_sizes Maximum number of opened FT_Size objects managed by this cache instance. Use 0 for defaults.
 * @param max_bytes Maximum number of bytes to use for cached data nodes. Use 0 for defaults.
 *                  Note that this value does not account for managed FT_Face and FT_Size objects.
 * @return true on success, otherwise false.
 */
bool lv_freetype_init(uint16_t max_faces, uint16_t max_sizes, uint32_t max_bytes);

/**
 * Destroy freetype library
 */
void lv_freetype_destroy(void);

/**
 * Creates a font with info parameter specified.
 * @param info See lv_ft_info_t for details.
 *             when success, lv_ft_info_t->font point to the font you created.
 * @return true on success, otherwise false.
 */
bool lv_ft_font_init(lv_ft_info_t * info);

/**
 * Destroy a font that has been created.
 * @param font pointer to font.
 */
void lv_ft_font_destroy(EG_Font_t * font);



#endif 
