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

#if EG_USE_TINY_TTF

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

#if EG_TINY_TTF_FILE_SUPPORT
/* create a font from the specified file or path with the specified line height.*/
EG_Font_t * lv_tiny_ttf_create_file(const char * path, int32_t font_size);

/* create a font from the specified file or path with the specified line height with the specified cache size.*/
EG_Font_t * lv_tiny_ttf_create_file_ex(const char * path, int32_t font_size, size_t cache_size);
#endif /*EG_TINY_TTF_FILE_SUPPORT*/

/* create a font from the specified data pointer with the specified line height.*/
EG_Font_t * lv_tiny_ttf_create_data(const void * data, size_t data_size, int32_t font_size);

/* create a font from the specified data pointer with the specified line height and the specified cache size.*/
EG_Font_t * lv_tiny_ttf_create_data_ex(const void * data, size_t data_size, int32_t font_size, size_t cache_size);

/* set the size of the font to a new font_size*/
void lv_tiny_ttf_set_size(EG_Font_t * font, int32_t font_size);

/* destroy a font previously created with lv_tiny_ttf_create_xxxx()*/
void lv_tiny_ttf_destroy(EG_Font_t * font);

/**********************
 *      MACROS
 **********************/

#endif 