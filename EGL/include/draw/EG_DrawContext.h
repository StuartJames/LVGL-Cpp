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

//#pragma once

#ifndef __EGDRAWCONTEXT__
#define __EGDRAWCONTEXT__

#include "../EG_IntrnlConfig.h"

#include "../misc/EG_Style.h"
#include "../misc/EG_Text.h"
#include "EG_ImageDecoder.h"
#include "EG_ImageCache.h"

#include "EG_DrawRect.h"
#include "EG_DrawLabel.h"
#include "EG_DrawImage.h"
#include "EG_DrawLine.h"
#include "EG_DrawPolygon.h"
#include "EG_DrawArc.h"
#include "EG_DrawMask.h"
#include "EG_DrawTransform.h"
#include "EG_LayerContext.h"

/////////////////////////////////////////////////////////////////////////////////

typedef struct {
    void  *pUserData;
} EG_DrawMask_t;

/////////////////////////////////////////////////////////////////////////////////

class EGDrawContext
{
public:
                          EGDrawContext(void);
  void                    WaitForFinish(void) const;

  static void             *m_pDrawBuffer;         // Pointer to a buffer to draw into
  static EGRect           *m_pDrawRect;           // The position and size of `buf` (absolute coordinates)
  static const EGRect     *m_pClipRect;       // The current clip area with absolute coordinates, always the same or smaller than `buf_area`
  static size_t            m_LayerInstanceSize;

#if EG_USE_USER_DATA
  void                    *m_pUserData;
#endif

  static void             (*InitBuffer)(EGDrawContext *pContext);
  static void             (*DrawLineProc)(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
  static void             (*DrawArcProc)(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
  static void             (*DrawRectProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void             (*DrawBackgroundProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void             (*DrawImageDecodedProc)(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
  static EG_Result_t      (*DrawImageProc)(EGDrawImage *pDrawImage, const EGRect *pRect, const void * src);
  static void             (*DrawCharacterProc)(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, uint32_t Alpha);
  static void             (*DrawPolygonProc)(const EGDrawPolygon *pDrawPolygon, const EGDrawRect *pRect, const EGPoint *pPoints, uint16_t PointCount);
  static void             (*TransformProc)(const EGRect *pRect, const void *pSrceBuffer, EG_Coord_t SourceWidth,
                            EG_Coord_t SourceHeight, EG_Coord_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t cf, EG_Color_t *pColorBoffer, EG_OPA_t *pOpaBuf);
  static void             (*WaitForFinishProc)(void);
  static void             (*CopyBufferProc)(void *pDestBuffer, EG_Coord_t DestStride, EGRect *pDestArea,
                            void *pSourceBuffer, EG_Coord_t SourceStride, EGRect *pSourceArea);
  static EGLayerContext*  (*IntialiseLayerProc)(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags);
  static void             (*LayerAdjustProc)(EGLayerContext *pDrawLayer,	 EGDrawLayerFlags_e Flags);
  static void             (*LayerBlendProc)(EGLayerContext *pDrawLayer, EGDrawImage *pImage);
  static void             (*LayerDestroyProc)(EGLayerContext *pDrawLayer);
};

#endif