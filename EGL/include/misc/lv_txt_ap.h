/**
 * @file lv_txt_ap.h
 *
 */

#pragma once

#include <stddef.h>
#include "misc/EG_Text.h"
#include "draw/EG_DrawContext.h"

#if EG_USE_ARABIC_PERSIAN_CHARS == 1

#define LV_UNDEF_ARABIC_PERSIAN_CHARS     (UINT32_MAX)
#define LV_AP_ALPHABET_BASE_CODE          0x0622
#define LV_AP_END_CHARS_LIST              {0,0,0,0,0,{0,0}}

uint32_t _lv_txt_ap_calc_bytes_cnt(const char * txt);
void _lv_txt_ap_proc(const char * txt, char * txt_out);

#endif
