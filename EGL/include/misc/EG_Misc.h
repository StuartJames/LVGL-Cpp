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
#include <stdint.h>
#include "EG_Memory.h"
#include "EG_LinkList.h"
#include "EG_Timer.h"
#include "EG_Types.h"
#include "../draw/EG_ImageCache.h"
#include "../draw/EG_DrawMask.h"
#include "../core/EG_ObjPosition.h"

/////////////////////////////////////////////////////////////////////////////

#if EG_IMG_CACHE_DEF_SIZE
#define EG_IMG_CACHE_DEF            1
#else
#define EG_IMG_CACHE_DEF            0
#endif

#define EG_DISPATCH(f, t, n)            f(t, n)
#define EG_DISPATCH_COND(f, t, n, m, v) EG_CONCAT3(EG_DISPATCH, m, v)(f, t, n)

#define EG_DISPATCH00(f, t, n)          EG_DISPATCH(f, t, n)
#define EG_DISPATCH01(f, t, n)
#define EG_DISPATCH10(f, t, n)
#define EG_DISPATCH11(f, t, n)          EG_DISPATCH(f, t, n)

#define EG_ITERATE_ROOTS(f)                                                                            \
    EG_DISPATCH(f, lv_ll_t, _lv_anim_ll)                                                               \
    EG_DISPATCH_COND(f, ImageCacheEntry_t*, _lv_img_cache_array, EG_IMG_CACHE_DEF, 1)              \
    EG_DISPATCH_COND(f, ImageCacheEntry_t, _lv_img_cache_single, EG_IMG_CACHE_DEF, 0)              \
    EG_DISPATCH(f, EG_MemoryBufferArr_t , EG_MemBuffer)                                                      \
    EG_DISPATCH_COND(f, EG_DrawMaskCircleListArray_t , _lv_circle_cache, EG_DRAW_COMPLEX, 1)  \
    EG_DISPATCH_COND(f, EG_DrawMaskListArray_t , EG_DrawMaskArray, EG_DRAW_COMPLEX, 1)            \
    EG_DISPATCH_COND(f, uint8_t *, _lv_font_decompr_buf, EG_USE_FONT_COMPRESSED, 1)                    \
    EG_DISPATCH(f, uint8_t * , _lv_grad_cache_mem)                                                     \
    EG_DISPATCH(f, uint8_t * , StyleCustomPropertyFlagLookupTable)

#define EG_DEFINE_ROOT(root_type, root_name) root_type root_name;
#define EG_ROOTS EG_ITERATE_ROOTS(EG_DEFINE_ROOT)

#if EG_ENABLE_GC == 1
#if EG_MEM_CUSTOM != 1
#error "GC requires CUSTOM_MEM"
#endif 
#include EG_GC_INCLUDE
#else 
#define EG_GC_ROOT(x) x
#define EG_EXTERN_ROOT(root_type, root_name) extern root_type root_name;
EG_ITERATE_ROOTS(EG_EXTERN_ROOT)
#endif

void EG_GC_ClearRoots(void);
