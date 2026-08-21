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
#include "draw/EG_DrawPolygon.h"
#include "misc/EG_Math.h"
#include "misc/EG_Memory.h"

//////////////////////////////////////////////////////////////////////////////////////

EGDrawPolygon::EGDrawPolygon() : EGDrawBase(),
  m_BlendMode(EG_BLEND_MODE_NORMAL),
  m_FillOPA(EG_OPA_COVER),
  m_FillColor(EG_ColorWhite()),
  m_Color(EG_ColorBlack()),
  m_Width(0),
  m_OPA(EG_OPA_COVER)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawPolygon::Draw(EGDeviceContext *pDC, const EGPoint *pVertices, uint16_t VerticesCount)
{
  if(VerticesCount < 3) return;
  if(pVertices == nullptr) return;
  m_pContext = pDC;
  pDC->DrawPolygonProc(this, pVertices, VerticesCount);
}


