/**
 * @file lv_tiny_ttf.h
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
EG_Font_t * lv_tiny_ttf_create_file(const char * path, EG_Coord_t font_size);

/* create a font from the specified file or path with the specified line height with the specified cache size.*/
EG_Font_t * lv_tiny_ttf_create_file_ex(const char * path, EG_Coord_t font_size, size_t cache_size);
#endif /*EG_TINY_TTF_FILE_SUPPORT*/

/* create a font from the specified data pointer with the specified line height.*/
EG_Font_t * lv_tiny_ttf_create_data(const void * data, size_t data_size, EG_Coord_t font_size);

/* create a font from the specified data pointer with the specified line height and the specified cache size.*/
EG_Font_t * lv_tiny_ttf_create_data_ex(const void * data, size_t data_size, EG_Coord_t font_size, size_t cache_size);

/* set the size of the font to a new font_size*/
void lv_tiny_ttf_set_size(EG_Font_t * font, EG_Coord_t font_size);

/* destroy a font previously created with lv_tiny_ttf_create_xxxx()*/
void lv_tiny_ttf_destroy(EG_Font_t * font);

/**********************
 *      MACROS
 **********************/

#endif 