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

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDeviceContext;
class EGDrawArc;
class EGDrawRect;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDrawArc : public EGDrawBase
{
public:
                  EGDrawArc(void);
  void            Draw(EGDeviceContext  *m_pContext, const EGPoint *center, uint16_t radius,  uint16_t start_angle, uint16_t end_angle);

  static void     GetArcRect(int32_t X, int32_t Y, uint16_t Radius, uint16_t StartAngle, uint16_t EndAngle, int32_t Width, bool Rounded, EGRect *pRect);

  EG_Color_t          m_Color;
  int32_t             m_Width;
  uint16_t            m_StartAngle;
  uint16_t            m_EndAngle;
  const void         *m_pImageSource;
  EG_OPA_t            m_OPA;
  EG_BlendMode_e      m_BlendMode : 3;
  uint8_t             m_Rounded : 1;
};



