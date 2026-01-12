
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

#include "EG_ImageDecoder.h"

//namespace EG_ImageCache
//{

/////////////////////////////////////////////////////////////////////////////////

typedef struct {
    ImageDecoderDescriptor_t DecoderDSC; /**< Image information*/

    /** Count the cache entries's life. Add `time_to_open` to `life` when the entry is used.
     * Decrement all lifes by one every in every ::lv_img_cache_open.
     * If life == 0 the entry can be reused*/
    int32_t Life;
} ImageCacheEntry_t;

/////////////////////////////////////////////////////////////////////////////////

ImageCacheEntry_t* ImageCacheOpen(const void *pSource, EG_Color_t Color, int32_t FrameID);

void SetImageCacheSize(uint16_t NewSlotNumber);

void InvalidateImageCacheSource(const void *pSource);

//}