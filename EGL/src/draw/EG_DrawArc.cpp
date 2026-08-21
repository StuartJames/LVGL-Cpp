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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#include "draw/EG_DrawArc.h"
//#include "draw/EG_DeviceContext.h"
#include "hal/EG_HALDisplay.h"
//#include "draw/sw/EG_SoftContext.h"

/////////////////////////////////////////////////////////////////////////////

EGDrawArc::EGDrawArc(void) : EGDrawBase(),
  m_Color(EG_ColorBlack()),
  m_Width(1),
  m_StartAngle(0),
  m_EndAngle(0),
  m_pImageSource(nullptr),
	m_OPA(EG_OPA_COVER),
  m_BlendMode(EG_BLEND_MODE_NORMAL),
  m_Rounded(0)
{
}

/////////////////////////////////////////////////////////////////////////////

void EGDrawArc::Draw(EGDeviceContext *pDC, const EGPoint *pCenter, uint16_t Radius, uint16_t StartAngle, uint16_t EndAngle)
{
	if(m_OPA <= EG_OPA_MIN) return;
	if(m_Width == 0) return;
	if(StartAngle == EndAngle) return;
  m_pContext = pDC;
	pDC->DrawArcProc(this, pCenter, Radius, StartAngle, EndAngle);
	//  const lv_draw_backend_t * backend = lv_draw_backend_get();
	//  backend->draw_arc(center_x, center_y, Radius, StartAngle, EndAngle, clip_area, dsc);
}

/////////////////////////////////////////////////////////////////////////////

void EGDrawArc::GetArcRect(int32_t x, int32_t y, uint16_t Radius, uint16_t StartAngle, uint16_t EndAngle, int32_t Width, bool Rounded, EGRect *pRect)
{
	int32_t OutsideRad = Radius;
	if(EndAngle == StartAngle + 360) {	// Special case: full arc invalidation
		pRect->SetX1(x - OutsideRad);
		pRect->SetY1(y - OutsideRad);
		pRect->SetX2(x + OutsideRad);
		pRect->SetY2(y + OutsideRad);
		return;
	}
	if(StartAngle > 360) StartAngle -= 360;
	if(EndAngle > 360) EndAngle -= 360;
	int32_t InsideRad = Radius - Width;
	int32_t Inflate = Rounded ? Width / 2 + 1 : 0;
	uint8_t StartQuarter = StartAngle / 90;
	uint8_t EndQuarter = EndAngle / 90;
	// 360 deg still counts as Quadrant 3 (360 / 90 would be 4)
	if(StartQuarter == 4) StartQuarter = 3;
	if(EndQuarter == 4) EndQuarter = 3;
	if(StartQuarter == EndQuarter && StartAngle <= EndAngle) {
		if(StartQuarter == 0) {
			pRect->SetY1(y + ((EG_TrigoSin(StartAngle) * InsideRad) >> EG_TRIGO_SHIFT) - Inflate);
			pRect->SetX2(x + ((EG_TrigoSin(StartAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
			pRect->SetY2(y + ((EG_TrigoSin(EndAngle) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
			pRect->SetX1(x + ((EG_TrigoSin(EndAngle + 90) * InsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		}
		else if(StartQuarter == 1) {
			pRect->SetY2(y + ((EG_TrigoSin(StartAngle) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
			pRect->SetX2(x + ((EG_TrigoSin(StartAngle + 90) * InsideRad) >> EG_TRIGO_SHIFT) + Inflate);
			pRect->SetY1(y + ((EG_TrigoSin(EndAngle) * InsideRad) >> EG_TRIGO_SHIFT) - Inflate);
			pRect->SetX1(x + ((EG_TrigoSin(EndAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		}
		else if(StartQuarter == 2) {
			pRect->SetX1(x + ((EG_TrigoSin(StartAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
			pRect->SetY2(y + ((EG_TrigoSin(StartAngle) * InsideRad) >> EG_TRIGO_SHIFT) + Inflate);
			pRect->SetY1(y + ((EG_TrigoSin(EndAngle) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
			pRect->SetX2(x + ((EG_TrigoSin(EndAngle + 90) * InsideRad) >> EG_TRIGO_SHIFT) + Inflate);
		}
		else if(StartQuarter == 3) {
			pRect->SetX1(x + ((EG_TrigoSin(StartAngle + 90) * InsideRad) >> EG_TRIGO_SHIFT) - Inflate);
			pRect->SetY1(y + ((EG_TrigoSin(StartAngle) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
			pRect->SetX2(x + ((EG_TrigoSin(EndAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
			pRect->SetY2(y + ((EG_TrigoSin(EndAngle) * InsideRad) >> EG_TRIGO_SHIFT) + Inflate);
		}
	}
	else if(StartQuarter == 0 && EndQuarter == 1) {
		pRect->SetX1(x + ((EG_TrigoSin(EndAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		pRect->SetY1(y + ((EG_MIN(EG_TrigoSin(EndAngle), EG_TrigoSin(StartAngle)) * InsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		pRect->SetX2(x + ((EG_TrigoSin(StartAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
		pRect->SetY2(y + OutsideRad + Inflate);
	}
	else if(StartQuarter == 1 && EndQuarter == 2) {
		pRect->SetX1(x - OutsideRad - Inflate);
		pRect->SetY1(y + ((EG_TrigoSin(EndAngle) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		pRect->SetX2(x + ((EG_MAX(EG_TrigoSin(StartAngle + 90), EG_TrigoSin(EndAngle + 90)) * InsideRad) >> EG_TRIGO_SHIFT) + Inflate);
		pRect->SetY2(y + ((EG_TrigoSin(StartAngle) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
	}
	else if(StartQuarter == 2 && EndQuarter == 3) {
		pRect->SetX1(x + ((EG_TrigoSin(StartAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		pRect->SetY1(y - OutsideRad - Inflate);
		pRect->SetX2(x + ((EG_TrigoSin(EndAngle + 90) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
		pRect->SetY2(y + (EG_MAX(EG_TrigoSin(EndAngle) * InsideRad, EG_TrigoSin(StartAngle) * InsideRad) >> EG_TRIGO_SHIFT) + Inflate);
	}
	else if(StartQuarter == 3 && EndQuarter == 0) {
		pRect->SetX1(x + ((EG_MIN(EG_TrigoSin(EndAngle + 90), EG_TrigoSin(StartAngle + 90)) * InsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		pRect->SetY1(y + ((EG_TrigoSin(StartAngle) * OutsideRad) >> EG_TRIGO_SHIFT) - Inflate);
		pRect->SetX2(x + OutsideRad + Inflate);
		pRect->SetY2(y + ((EG_TrigoSin(EndAngle) * OutsideRad) >> EG_TRIGO_SHIFT) + Inflate);
	}
	else {
		pRect->SetX1(x - OutsideRad);
		pRect->SetY1(y - OutsideRad);
		pRect->SetX2(x + OutsideRad);
		pRect->SetY2(y + OutsideRad);
	}
}

