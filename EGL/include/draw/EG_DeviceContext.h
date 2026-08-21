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

//#pragma once

#ifndef __EGDRAWCONTEXT__
#define __EGDRAWCONTEXT__

#include "../EG_IntrnlConfig.h"

#include "misc/EG_Style.h"
#include "misc/EG_Text.h"
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
  void  *pExtData;
} EG_DrawMask_t;

/////////////////////////////////////////////////////////////////////////////////

class EGBlendBase
{
public:
                          EGBlendBase(EGDeviceContext *pDC);
                          ~EGBlendBase(void){};
  EGDeviceContext         *m_pContext;
  const EGRect            *m_pRect;          // The area with absolute coordinates to draw on 
  const EG_Color_t        *m_pSourceBuffer;  // Pointer to an image to blend. If set `fill_color` is ignored 
  EG_Color_t               m_Color;          // Fill color
  EG_OPA_t                *m_pMaskBuffer;    // NULL if ignored, or an alpha mask to apply on `blend_area`
  DrawMaskRes_t            m_MaskResult;     // The result of the previous mask operation 
  const EGRect            *m_pMaskRect;      // The area of `mask_buf` with absolute coordinates
  EG_OPA_t                 m_OPA;            // The overall opacity
  EG_BlendMode_e           m_BlendMode;      // E.g. EG_BLEND_MODE_ADDITIVE
};

/////////////////////////////////////////////////////////////////////////////////

class EGDeviceContext
{
public:
                          EGDeviceContext(void);
  void                    WaitForFinish(void) const;

  void                    *m_pDrawBuffer;         // Pointer to a buffer to draw into
  EGRect                  *m_pDrawRect;           // The position and size of `buf` (absolute coordinates)
  const EGRect            *m_pClipRect;           // The current clip area with absolute coordinates, always the same or smaller than `buf_area`
  void                    *m_pExtData;

  static void             (*BlendProc)(EGBlendBase *pBlend);
  static void             (*InitBufferProc)(EGDeviceContext *pDC);
  static void             (*DrawLineProc)(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
  static void             (*DrawArcProc)(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
  static void             (*DrawRectProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void             (*DrawBackgroundProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void             (*DrawImageDecodedProc)(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
  static EG_Result_t      (*DrawImageProc)(EGDrawImage *pDrawImage, const EGRect *pRect, const void *Srce);
  static void             (*DrawCharacterProc)(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, uint32_t Alpha);
  static void             (*DrawPolygonProc)(const EGDrawPolygon *pDrawPolygon, const EGPoint *m_pVertices, uint16_t m_VerticesCount);
  static void             (*TransformProc)(const EGRect *pRect, const void *pSrceBuffer, int32_t SourceWidth,
                            int32_t SourceHeight, int32_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t cf, EG_Color_t *pColorBoffer, EG_OPA_t *pOpaBuf);
  static void             (*WaitForFinishProc)(void);
  static void             (*CopyBufferProc)(void *pDestBuffer, int32_t DestStride, EGRect *pDestArea,
                            void *pSourceBuffer, int32_t SourceStride, EGRect *pSourceArea);
  static bool             (*LayerIntialiseProc)(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags);
  static void             (*LayerAdjustProc)(EGLayerContext *pDrawLayer,	 EGDrawLayerFlags_e Flags);
  static void             (*LayerBlendProc)(EGLayerContext *pDrawLayer, EGDrawImage *pImage);
  static void             (*LayerDestroyProc)(EGLayerContext *pDrawLayer);

};

#endif