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

#include "draw/EG_DeviceContext.h"

/////////////////////////////////////////////////////////////////////////////////

EGBlendBase::EGBlendBase(EGDeviceContext *pDC) :
  m_pContext(pDC),
  m_pRect(nullptr),
  m_pSourceBuffer(nullptr),
  m_Color(EG_ColorBlack()),
  m_pMaskBuffer(nullptr),
  m_MaskResult(EG_DRAW_MASK_RESULT_UNKNOWN),
  m_pMaskRect(nullptr),
  m_OPA(EG_OPA_COVER),
  m_BlendMode(EG_BLEND_MODE_NORMAL)
{
}

/////////////////////////////////////////////////////////////////////////////////
// expose statics
void             (*EGDeviceContext::BlendProc)(EGBlendBase *pBlend);
void             (*EGDeviceContext::InitBufferProc)(EGDeviceContext *pDC);
void             (*EGDeviceContext::DrawLineProc)(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
void             (*EGDeviceContext::DrawArcProc)(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
void             (*EGDeviceContext::DrawRectProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
void             (*EGDeviceContext::DrawBackgroundProc)(const EGDrawRect *pDrawRect, const EGRect *pRect);
void             (*EGDeviceContext::DrawImageDecodedProc)(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
EG_Result_t      (*EGDeviceContext::DrawImageProc)(EGDrawImage *pDrawImage, const EGRect *pRect, const void * src);
void             (*EGDeviceContext::DrawCharacterProc)(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, uint32_t Char);
void             (*EGDeviceContext::DrawPolygonProc)(const EGDrawPolygon *pDrawPolygon, const EGPoint *m_pVertices, uint16_t m_VerticesCount);
void             (*EGDeviceContext::TransformProc)(const EGRect *pRect, const void *pSrceBuffer, int32_t SourceWidth,
                   int32_t SourceHeight, int32_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t cf, EG_Color_t *pColorBoffer, EG_OPA_t *pOpaBuf);
void             (*EGDeviceContext::WaitForFinishProc)(void);
void             (*EGDeviceContext::CopyBufferProc)(void *pDestBuffer, int32_t DestStride, EGRect *pDestArea,
                   void *pSourceBuffer, int32_t SourceStride, EGRect *pSourceArea);
bool             (*EGDeviceContext::LayerIntialiseProc)(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags);
void             (*EGDeviceContext::LayerAdjustProc)(EGLayerContext *pDrawLayer,	 EGDrawLayerFlags_e Flags);
void             (*EGDeviceContext::LayerBlendProc)(EGLayerContext *pDrawLayer, EGDrawImage *pDrawImage);
void             (*EGDeviceContext::LayerDestroyProc)(EGLayerContext *pDrawLayer);

//const EGRect    *EGDeviceContext::m_pClipRect = nullptr;      // The current clip area with absolute coordinates, always the same or smaller than `buf_area`

/////////////////////////////////////////////////////////////////////////////////

EGDeviceContext::EGDeviceContext(void) :
  m_pDrawBuffer(nullptr),
  m_pDrawRect(nullptr),
  m_pClipRect(nullptr),
  m_pExtData(nullptr)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDeviceContext::WaitForFinish(void) const
{
  if(WaitForFinishProc) WaitForFinishProc();
}

