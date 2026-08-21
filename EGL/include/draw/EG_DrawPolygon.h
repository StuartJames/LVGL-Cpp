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

#include "EG_DrawBase.h"
#include "EG_DrawRect.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGDeviceContext;

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawPolygon : public EGDrawBase
{
public:
                    EGDrawPolygon();
  void              Draw(EGDeviceContext *pDC, const EGPoint *pVertices, uint16_t VerticesCount);

  EG_BlendMode_e        m_BlendMode;
	EG_OPA_t              m_FillOPA;	      // Background
	EG_Color_t            m_FillColor;      // First element of a gradient is a color, so it maps well here
	EG_Color_t            m_Color;	        // Border
	int32_t               m_Width;
	EG_OPA_t              m_OPA;
};
