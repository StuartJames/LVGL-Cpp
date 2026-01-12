/**
 * @file lv_imgfont.h
 *
 */

#pragma once

#include "EGL.h"

#if EG_USE_IMGFONT

/* gets the image path name of this character */
typedef bool (*lv_get_imgfont_path_cb_t)(const EG_Font_t * font, void * img_src,
                                         uint16_t len, uint32_t unicode, uint32_t unicode_next);

/**
 * Creates a image font with info parameter specified.
 * @param height font size
 * @param path_cb a function to get the image path name of character.
 * @return pointer to the new imgfont or NULL if create error.
 */
EG_Font_t * lv_imgfont_create(uint16_t height, lv_get_imgfont_path_cb_t path_cb);

/**
 * Destroy a image font that has been created.
 * @param font pointer to image font handle.
 */
void lv_imgfont_destroy(EG_Font_t * font);

#endif 