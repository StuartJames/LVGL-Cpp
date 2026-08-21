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

#include "../EG_IntrnlConfig.h"
#include "EG_DrawBase.h"
#include "../misc/EG_Color.h"
#include "../misc/EG_Point.h"
#include "../misc/EG_Rect.h"
#include "../misc/EG_Style.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGDeviceContext;

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawLine : public EGDrawBase
{
public:
                    EGDrawLine(void);
  void              Draw(EGDeviceContext  *pDC, const EGPoint *pPoint1, const EGPoint *pPoint2);

  EG_Color_t        m_Color;
  int32_t           m_Width;
  int32_t           m_DashWidth;
  int32_t           m_DashGap;
  EG_OPA_t          m_OPA;
  EG_BlendMode_e    m_BlendMode : 3;
  uint8_t           m_RoundStart : 1;
  uint8_t           m_RoundEnd   : 1;
  uint8_t           m_RawEnd     : 1; // Do not bother with perpendicular line ending if it's not visible
};
