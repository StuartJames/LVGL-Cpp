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

#include <stddef.h>
#include "misc/EG_Text.h"
#include "draw/EG_DeviceContext.h"

#if EG_USE_ARABIC_PERSIAN_CHARS == 1

#define EG_UNDEF_ARABIC_PERSIAN_CHARS     (UINT32_MAX)
#define EG_AP_ALPHABET_BASE_CODE          0x0622
#define EG_AP_END_CHARS_LIST              {0,0,0,0,0,{0,0}}

uint32_t _lv_txt_ap_calc_bytes_cnt(const char * txt);
void _lv_txt_ap_proc(const char * txt, char * txt_out);

#endif
