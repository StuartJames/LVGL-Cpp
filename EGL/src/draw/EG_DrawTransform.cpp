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
#include "draw/EG_DrawContext.h"
#include "draw/EG_DrawTransform.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Rect.h"

void Transform(const EGDrawContext *pDrawContext, const EGRect *pRect, const void *pSourceBuffer, EG_Coord_t SourceWidth, EG_Coord_t SourceHeight, EG_Coord_t SourceStep,
               const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat, EG_Color_t *pColorBuffer, EG_OPA_t *pBufferOPA)
{
  if(pDrawContext->TransformProc == nullptr) {
    EG_LOG_WARN("No Transform function installed");
    return;
  }
  pDrawContext->TransformProc(pRect, pSourceBuffer, SourceWidth, SourceHeight, SourceStep, pImage, ColorFormat, pColorBuffer, pBufferOPA);
}

