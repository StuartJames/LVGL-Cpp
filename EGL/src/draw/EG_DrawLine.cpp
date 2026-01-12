/*
 *                LEGL 2025-2026 HydraSystems.
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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include <stdbool.h>
#include "core/EG_Refresh.h"
#include "misc/EG_Math.h"
#include "draw/EG_DrawLine.h"
#include "draw/sw/EG_SoftContext.h"

//////////////////////////////////////////////////////////////////////////////////////

EGDrawLine::EGDrawLine(void) :
  m_pContext(nullptr),
  m_Color(EG_ColorBlack()),
  m_Width(1),
  m_DashWidth(1),
  m_DashGap(1),
  m_OPA(EG_OPA_COVER),
  m_BlendMode(EG_BLEND_MODE_NORMAL),
  m_RoundStart(0),
  m_RoundEnd(0),
  m_RawEnd(0)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGDrawLine::Draw(const EGDrawContext  *pDrawContext, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
  if(m_Width == 0) return;
  if(m_OPA <= EG_OPA_MIN) return;
  m_pContext = pDrawContext;
  pDrawContext->DrawLineProc(this, pPoint1, pPoint2);
}

