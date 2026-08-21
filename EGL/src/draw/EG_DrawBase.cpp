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
 *
 */

#include "draw/EG_DrawBase.h"
#include "draw/EG_EventDC.h"

//////////////////////////////////////////////////////////////////////////////////////

EGDrawBase::EGDrawBase(void) :
  m_pContext(nullptr),
  m_pLayer(nullptr),
  m_pObj(nullptr),
  m_ID1(0),
  m_ID2(0),
  m_ShadowOffsetX(0),
  m_ShadowOffsetY(0),
  m_ShadowColor(EG_ColorBlack()),
  m_ShadowOPA(EG_OPA_COVER),
  m_ShadowBlurRadius(0),
  m_ShadowQuality(EG_BLUR_QUALITY_AUTO),
  m_ShadowWidth(0),
  m_ShadowSpread(0),
  m_pExtData(nullptr)
{
}
