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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "extra/widgets/EG_Spinner.h"

#if EG_USE_SPINNER

const EG_ClassType_t c_SpinnerClass = {
  .pBaseClassType = &c_ArcClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGSpinner::EGSpinner(void) :
  EGArc(),
  m_TimeParam(500),
  m_ArcLengthParam(90)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGSpinner::EGSpinner(EGObject *pParent, uint32_t Time, uint32_t ArcLength) : EGArc()
{
// 	ESP_LOGI("[Spiner]", "New: %p", (void*)pParent);
  Attach(this, pParent, &c_SpinnerClass);
  m_TimeParam = Time;
	m_ArcLengthParam = ArcLength;
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinner::Configure(void)
{
  EGArc::Configure();
	ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	EGAnimate Animate;
	Animate.SetItem(this);
	Animate.SetExcCB(EGSpinner::EndAngle);
	Animate.SetRepeatCount(EG_ANIM_REPEAT_INFINITE);
	Animate.SetTime(m_TimeParam);
	Animate.SetValues(m_ArcLengthParam, 360 + m_ArcLengthParam);
	EGAnimate::Create(&Animate);

  Animate.SetPathCB(EGAnimate::PathEaseInOut);
	Animate.SetValues(0, 360);
	Animate.SetExcCB(EGSpinner::StartAngle);
	EGAnimate::Create(&Animate);

	SetBackgroundAngles(0, 360);
	SetRotation(270);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinner::StartAngle(EGAnimate *pAnimation, int32_t Angle)
{
	((EGArc*)pAnimation->m_pItem)->SetStartAngle((uint16_t)Angle);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinner::EndAngle(EGAnimate *pAnimation, int32_t Angle)
{
	((EGArc*)pAnimation->m_pItem)->SetEndAngle((uint16_t)Angle);
}

#endif 
