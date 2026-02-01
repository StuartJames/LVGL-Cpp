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

/////////////////////////////////////////////////////////////////////////////////
// expose statics
void             (*EGDrawContext::InitBuffer)(EGDrawContext *pContext);
void             (*EGDrawContext::DrawLineProc)(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
void             (*EGDrawContext::DrawArcProc)(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
void             (*EGDrawContext::DrawRectProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
void             (*EGDrawContext::DrawBackgroundProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
void             (*EGDrawContext::DrawImageDecodedProc)(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
EG_Result_t      (*EGDrawContext::DrawImageProc)(EGDrawImage *pDrawImage, const EGRect *pRect, const void * src);
void             (*EGDrawContext::DrawCharacterProc)(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, uint32_t Char);
void             (*EGDrawContext::DrawPolygonProc)(const EGDrawPolygon *pDrawPolygon, const EGDrawRect *pRect, const EGPoint *pPoints, uint16_t PointCount);
void             (*EGDrawContext::TransformProc)(const EGRect *pRect, const void *pSrceBuffer, EG_Coord_t SourceWidth,
                   EG_Coord_t SourceHeight, EG_Coord_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t cf, EG_Color_t *pColorBoffer, EG_OPA_t *pOpaBuf);
void             (*EGDrawContext::WaitForFinishProc)(void);
void             (*EGDrawContext::CopyBufferProc)(void *pDestBuffer, EG_Coord_t DestStride, EGRect *pDestArea,
                   void *pSourceBuffer, EG_Coord_t SourceStride, EGRect *pSourceArea);
EGLayerContext*  (*EGDrawContext::IntialiseLayerProc)(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags);
void             (*EGDrawContext::LayerAdjustProc)(EGLayerContext *pDrawLayer,	 EGDrawLayerFlags_e Flags);
void             (*EGDrawContext::LayerBlendProc)(EGLayerContext *pDrawLayer, EGDrawImage *pImage);
void             (*EGDrawContext::LayerDestroyProc)(EGLayerContext *pDrawLayer);

void            *EGDrawContext::m_pDrawBuffer = nullptr; 
EGRect          *EGDrawContext::m_pDrawRect = nullptr;          // The position and size of `buf` (absolute coordinates)
const EGRect    *EGDrawContext::m_pClipRect = nullptr;      // The current clip area with absolute coordinates, always the same or smaller than `buf_area`
size_t           EGDrawContext::m_LayerInstanceSize = 0;

/////////////////////////////////////////////////////////////////////////////////

EGDrawContext::EGDrawContext(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawContext::WaitForFinish(void) const
{
  if(WaitForFinishProc) WaitForFinishProc();
}

