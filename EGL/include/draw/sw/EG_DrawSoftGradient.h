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


#include "../../misc/EG_Color.h"
#include "../../misc/EG_Style.h"
#include "EG_DrawSoftDither.h"
#if EG_GRADIENT_MAX_STOPS < 2
#error LVGL needs at least 2 stops for gradients. Please increase the EG_GRADIENT_MAX_STOPS
#endif

/////////////////////////////////////////////////////////////////////////////////

#if _DITHER_GRADIENT
typedef EG_Color32_t EG_GradientColor_t;
#else
typedef EG_Color_t EG_GradientColor_t;
#endif

/** To avoid recomputing gradient for each draw operation,
 *  it's possible to cache the computation in this structure instance.
 *  Whenever possible, this structure is reused instead of recomputing the gradient map*/
typedef struct EG_GradCacheItem_t {
	uint32_t Key;             //  A discriminating key that's built from the drawing operation.
                            //  If the key does not match, the cache item is not used 
	uint32_t Life : 30;       //  A life counter that's incremented on usage. Higher counter is
                            //  less likely to be evicted from the cache 
	uint32_t Filled : 1;      //  Used to skip dithering in it if already done 
	uint32_t UnCached : 1;    //  The cache was too small so this item is not managed by the cache
	EG_Color_t *pMap;         //  The computed gradient low bitdepth color map, points into the
                            //  cache's buffer, no free needed 
	EG_Coord_t AllocatedSize; //  The map allocated size in colors 
	EG_Coord_t Size;          //  The computed gradient color map size, in colors 
#if _DITHER_GRADIENT
	EG_Color32_t *pHighMap;   //  If dithering, we need to store the current, high bitdepth gradient
                            //  map too, points to the cache's buffer, no free needed 
#if EG_DITHER_ERROR_DIFFUSION == 1
	EG_SColor24_t *pError;    //  Error diffusion dithering algorithm requires storing the last error
                            //  drawn, points to the cache's buffer, no free needed  
	EG_Coord_t Width;         //  The error array width in pixels 
#endif
#endif
} EG_GradCacheItem_t;

/////////////////////////////////////////////////////////////////////////////////

EG_GradientColor_t /*EG_ATTRIBUTE_FAST_MEM*/ EG_GradientCalculate(const EG_GradDescriptor_t *dsc, EG_Coord_t range,
																																	EG_Coord_t frac);

void EG_GradientSetCacheSize(size_t max_bytes);

void EG_GradientFreeCache(void);

EG_GradCacheItem_t *EG_GetGradient(const EG_GradDescriptor_t *gradient, EG_Coord_t w, EG_Coord_t h);

void EG_GradientCleanup(EG_GradCacheItem_t *grad);
