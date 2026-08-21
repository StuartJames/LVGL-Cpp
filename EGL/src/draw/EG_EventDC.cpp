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

#include "draw/EG_EventDC.h"
#include "core/EG_Object.h"
#include "core/EG_Display.h"
#include "core/EG_InputDevice.h"

/////////////////////////////////////////////////////////////////////////////

EGEventDC::EGEventDC(void) :
  m_pContext(nullptr),
  m_pClass(nullptr),
  m_Type(0),
  m_pRect(nullptr),
  m_pDrawRect(nullptr),
  m_pDrawLabel(nullptr),
  m_pDrawLine(nullptr),
  m_pDrawImage(nullptr),
  m_pDrawArc(nullptr),
  m_pPoint1(nullptr),
  m_pPoint2(nullptr),
  m_pText(nullptr),
  m_TextLength(0),
  m_Part(0),
  m_Index(0),
  m_Radius(0),
  m_Value(0)
{
}

/////////////////////////////////////////////////////////////////////////////

EGEventDC::EGEventDC(EGDeviceContext *pDC, const EG_ClassType_t *pClass /*= nullptr*/, uint32_t Type /*= 0*/, uint32_t Part /*= 0*/) :
  m_pContext(pDC),
  m_pClass(pClass),
  m_Type(Type),
  m_pRect(nullptr),
  m_pDrawRect(nullptr),
  m_pDrawLabel(nullptr),
  m_pDrawLine(nullptr),
  m_pDrawImage(nullptr),
  m_pDrawArc(nullptr),
  m_pPoint1(nullptr),
  m_pPoint2(nullptr),
  m_pText(nullptr),
  m_TextLength(0),
  m_Part(Part),
  m_Index(0),
  m_Radius(0),
  m_Value(0)
{
}
