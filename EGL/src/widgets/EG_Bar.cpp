/* 
 *        Copyright (Center) 2025-2026 HydraSystems..
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
 *  Based on Animate design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.Animate.1    Original by LVGL Kft
 *
 */


#include "widgets/EG_Bar.h"

#if EG_USE_BAR != 0

#include "misc/EG_Assert.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Animate.h"
#include "misc/EG_Math.h"

///////////////////////////////////////////////////////////////////////////////////////

#define BAR_CLASS &c_BarClass

#define EG_BAR_SIZE_MIN 4
#define EG_BAR_IS_ANIMATING(anim_struct) (((anim_struct).AnimationState) != EG_BAR_ANIM_STATE_INV)
#define EG_BAR_GET_ANIM_VALUE(orig_value, anim_struct) (EG_BAR_IS_ANIMATING(anim_struct) ? ((anim_struct).AnimationEnd) : (orig_value))
#define EG_BAR_ANIM_STATE_START 0
#define EG_BAR_ANIM_STATE_END 256
#define EG_BAR_ANIM_STATE_INV -1
#define EG_BAR_ANIM_STATE_NORM 8

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_BarClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGBar::Event,
	.WidthDef = EG_DPI_DEF * 2,
	.HeightDef = EG_DPI_DEF / 10,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGBar::EGBar(void) : EGObject()
{
	m_MinimumValue = 0;
	m_MaximumValue = 100;
	m_StartValue = 0;
	m_CurrentValue = 0;
	m_IndicatorRect.Set(0, 0, 0, 0);
	m_Mode = EG_BAR_MODE_NORMAL;
}

///////////////////////////////////////////////////////////////////////////////////////

EGBar::EGBar(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_BarClass*/) : EGObject()
{
	m_MinimumValue = 0;
	m_MaximumValue = 100;
	m_StartValue = 0;
	m_CurrentValue = 0;
	m_IndicatorRect.Set(0, 0, 0, 0);
	m_Mode = EG_BAR_MODE_NORMAL;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGBar::~EGBar(void)
{
	EGAnimate::Delete(&m_CurrentValueAnimation, nullptr);
	EGAnimate::Delete(&m_StartValueAnimation, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::Configure(void)
{
  EGObject::Configure();
	m_MinimumValue = 0;
	m_MaximumValue = 100;
	m_StartValue = 0;
	m_CurrentValue = 0;
	m_IndicatorRect.Set(0, 0, 0, 0);
	m_Mode = EG_BAR_MODE_NORMAL;
	InitialiseAnimmation(&m_CurrentValueAnimation);
	InitialiseAnimmation(&m_StartValueAnimation);
	ClearFlag(EG_OBJ_FLAG_CHECKABLE);
	ClearFlag(EG_OBJ_FLAG_SCROLLABLE);
	SetValue(0, EG_ANIM_OFF);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::SetValue(int32_t Value, EG_AnimateEnable_e Animate)
{
	if(m_CurrentValue == Value) return;
	Value = EG_CLAMP(m_MinimumValue, Value, m_MaximumValue);
	Value = Value < m_StartValue ? m_StartValue : Value; // Can't be smaller than the left Value
	if(m_CurrentValue == Value) return;
	SetValueWithAnimation(Value, &m_CurrentValue, &m_CurrentValueAnimation, Animate);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::SetStartValue(int32_t Value, EG_AnimateEnable_e Animate)
{
	if(m_Mode != EG_BAR_MODE_RANGE) return;
	Value = EG_CLAMP(m_MinimumValue, Value, m_MaximumValue);
	Value = Value > m_CurrentValue ? m_CurrentValue : Value; // Can't be greater than the right Value
	if(m_StartValue == Value) return;
	SetValueWithAnimation(Value, &m_StartValue, &m_StartValueAnimation, Animate);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::SetRange(int32_t Minimum, int32_t Maximum)
{
	if(m_MinimumValue == Minimum && m_MaximumValue == Maximum) return;
	m_MaximumValue = Maximum;
	m_MinimumValue = Minimum;
	if(m_Mode != EG_BAR_MODE_RANGE)	m_StartValue = Minimum;
	if(m_CurrentValue > Maximum) {
		m_CurrentValue = Maximum;
		SetValue(m_CurrentValue, EG_ANIM_OFF);
	}
	if(m_CurrentValue < Minimum) {
		m_CurrentValue = Minimum;
		SetValue(m_CurrentValue, EG_ANIM_OFF);
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::SetMode(EG_BarMode_e mode)
{
	m_Mode = mode;
	if(m_Mode != EG_BAR_MODE_RANGE) {
		m_StartValue = m_MinimumValue;
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

int32_t EGBar::GetValue(void) const
{
	return EG_BAR_GET_ANIM_VALUE(m_CurrentValue, m_CurrentValueAnimation);
}

///////////////////////////////////////////////////////////////////////////////////////

int32_t EGBar::GetStartValue(void) const
{
	if(m_Mode != EG_BAR_MODE_RANGE) return m_MinimumValue;

	return EG_BAR_GET_ANIM_VALUE(m_StartValue, m_StartValueAnimation);
}

///////////////////////////////////////////////////////////////////////////////////////

int32_t EGBar::GetMinimumValue(void) const
{
	return m_MinimumValue;
}

///////////////////////////////////////////////////////////////////////////////////////

int32_t EGBar::GetMaximumValue(void) const
{
	return m_MaximumValue;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_BarMode_e EGBar::GetMode(void) const
{
	return m_Mode;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::Event(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(BAR_CLASS) != EG_RES_OK) return;// Call the ancestor's event handler
	EG_EventCode_e Code = pEvent->GetCode();
	EGObject *pObj = pEvent->GetTarget();
	EGBar *pBar = (EGBar*)pEvent->GetTarget();
	if(Code == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		EG_Coord_t IndicatorSize;
		IndicatorSize = pObj->CalculateExtDrawSize(EG_PART_INDICATOR);
		EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();	// Bg size is handled by lv_obj
		*pSize = EG_MAX(*pSize, IndicatorSize);
		EG_Coord_t PadLeft = pObj->GetStylePadLeft(EG_PART_MAIN);		// Calculate the indicator area
		EG_Coord_t PadRight = pObj->GetStylePadRight(EG_PART_MAIN);
		EG_Coord_t PadTop = pObj->GetStylePadTop(EG_PART_MAIN);
		EG_Coord_t PadBottom = pObj->GetStylePadBottom(EG_PART_MAIN);
		EG_Coord_t pad = EG_MIN4(PadLeft, PadRight, PadTop, PadBottom);
		if(pad < 0) {
			*pSize = EG_MAX(*pSize, -pad);
		}
	}
	else if(Code == EG_EVENT_PRESSED || Code == EG_EVENT_RELEASED) {
		pObj->InvalidateArea(&pBar->m_IndicatorRect);
	}
	else if(Code == EG_EVENT_DRAW_MAIN) {
		pBar->DrawIndicator(pEvent);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::DrawIndicator(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	EGRect Rect(m_Rect);
	EG_Coord_t TransferWidth = GetStyleTransformWidth(EG_PART_MAIN);
	EG_Coord_t TransferHeight = GetStyleTransformHeight(EG_PART_MAIN);
	Rect.Inflate(TransferWidth, TransferHeight);
	EG_Coord_t BarWidth = Rect.GetWidth();
	EG_Coord_t BarHeight = Rect.GetHeight();
	int32_t range = m_MaximumValue - m_MinimumValue;
	bool Horizontal = BarWidth >= BarHeight ? true : false;
	bool sym = false;
	if(m_Mode == EG_BAR_MODE_SYMMETRICAL && m_MinimumValue < 0 && m_MaximumValue > 0 &&
		 m_StartValue == m_MinimumValue) sym = true;

	// Calculate the indicator area
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_MAIN);
	// Respect padding and minimum width/height too
	Rect.Copy(&m_IndicatorRect);
	m_IndicatorRect.Deflate(PadLeft, PadRight, PadTop, PadBottom);
	if(Horizontal && m_IndicatorRect.GetHeight() < EG_BAR_SIZE_MIN) {
		m_IndicatorRect.SetY1(m_Rect.GetY1() + (BarHeight / 2) - (EG_BAR_SIZE_MIN / 2));
		m_IndicatorRect.SetY2(m_IndicatorRect.GetY1() + EG_BAR_SIZE_MIN);
	}
	else if(!Horizontal && m_IndicatorRect.GetWidth() < EG_BAR_SIZE_MIN) {
		m_IndicatorRect.SetX1(m_Rect.GetX1() + (BarWidth / 2) - (EG_BAR_SIZE_MIN / 2));
		m_IndicatorRect.SetX2(m_IndicatorRect.GetX1() + EG_BAR_SIZE_MIN);
	}
	EG_Coord_t indicw = m_IndicatorRect.GetWidth();
	EG_Coord_t indich = m_IndicatorRect.GetHeight();
	EG_Coord_t anim_length = Horizontal ? indicw : indich;	// Calculate the indicator length
	EG_Coord_t anim_cur_value_x, anim_start_value_x;
	EG_Coord_t Axis1, Axis2;
  EG_Coord_t CalculatedLength;
	if(Horizontal) {
		Axis1 = m_IndicatorRect.GetX1();
	  Axis2 = m_IndicatorRect.GetX2();
	}
	else {
		Axis1 = m_IndicatorRect.GetY1();
		Axis2 = m_IndicatorRect.GetY2();
	}
	if(EG_BAR_IS_ANIMATING(m_StartValueAnimation)) {
		EG_Coord_t anim_start_value_start_x =	(int32_t)((int32_t)anim_length * (m_StartValueAnimation.AnimstionStart - m_MinimumValue)) / range;
		EG_Coord_t anim_start_value_end_x =	(int32_t)((int32_t)anim_length * (m_StartValueAnimation.AnimationEnd - m_MinimumValue)) / range;
		anim_start_value_x = (((anim_start_value_end_x - anim_start_value_start_x) * m_StartValueAnimation.AnimationState) / EG_BAR_ANIM_STATE_END);
		anim_start_value_x += anim_start_value_start_x;
	}
	else {
		anim_start_value_x = (int32_t)((int32_t)anim_length * (m_StartValue - m_MinimumValue)) / range;
	}
	if(EG_BAR_IS_ANIMATING(m_CurrentValueAnimation)) {
		EG_Coord_t anim_cur_value_start_x = (int32_t)((int32_t)anim_length * (m_CurrentValueAnimation.AnimstionStart - m_MinimumValue)) / range;
		EG_Coord_t anim_cur_value_end_x = (int32_t)((int32_t)anim_length * (m_CurrentValueAnimation.AnimationEnd - m_MinimumValue)) / range;
		anim_cur_value_x = anim_cur_value_start_x + (((anim_cur_value_end_x - anim_cur_value_start_x) *	m_CurrentValueAnimation.AnimationState) / EG_BAR_ANIM_STATE_END);
	}
	else {
		anim_cur_value_x = (int32_t)((int32_t)anim_length * (m_CurrentValue - m_MinimumValue)) / range;
	}
	EG_BaseDirection_e base_dir = GetStyleBaseDirection(EG_PART_MAIN);
	if(Horizontal && base_dir == EG_BASE_DIR_RTL) {
		EG_Coord_t tmp;		// Swap axes
		tmp = Axis1;
		Axis1 = Axis2;
		Axis2 = tmp;
		anim_cur_value_x = -anim_cur_value_x;
		anim_start_value_x = -anim_start_value_x;
	}
	if(Horizontal) {	// Set the indicator length
		Axis2 = Axis1 + anim_cur_value_x;
		Axis1 += anim_start_value_x;
	}
	else {
		Axis1 = Axis2 - anim_cur_value_x + 1;
		Axis2 -= anim_start_value_x;
	}
	if(sym) {
		EG_Coord_t zero, shift;
		shift = (-m_MinimumValue * anim_length) / range;
		if(Horizontal) {
			zero = Axis1 + shift;
			if(Axis2 > zero) Axis1 = zero;
			else {
				Axis1 = Axis2;
				Axis2 = zero;
			}
		}
		else {
			zero = Axis2 - shift + 1;
			if(Axis1 > zero) Axis2 = zero;
			else {
				Axis2 = Axis1;
				Axis1 = zero;
			}
			if(Axis2 < Axis1) {
				zero = Axis1;				// swap
				Axis1 = Axis2;
				Axis2 = zero;
			}
		}
	}
  if(Horizontal){
		m_IndicatorRect.SetX1(Axis1);
	  m_IndicatorRect.SetX2(Axis2);
		CalculatedLength = m_IndicatorRect.GetWidth();
  }
  else{
		m_IndicatorRect.SetY1(Axis1);
		m_IndicatorRect.SetY2(Axis2);
		CalculatedLength = m_IndicatorRect.GetHeight();
  } 
  // Do not draw Animate zero length indicator but at least call the draw part events
	if(!sym && CalculatedLength <= 1) {
		EGDrawDiscriptor DrawDiscriptor;
		InitDrawDescriptor(&DrawDiscriptor, pContext);
		DrawDiscriptor.m_Part = EG_PART_INDICATOR;
		DrawDiscriptor.m_pClass = m_pClass;
		DrawDiscriptor.m_Type = EG_BAR_DRAW_PART_INDICATOR;
		DrawDiscriptor.m_pRect = &m_IndicatorRect;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		return;
	}
//	EGRect indic_area(m_IndicatorRect);
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_INDICATOR, &DrawRect);
  EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_Part = EG_PART_INDICATOR;
	DrawDiscriptor.m_pClass = m_pClass;
	DrawDiscriptor.m_Type = EG_BAR_DRAW_PART_INDICATOR;
	DrawDiscriptor.m_pDrawRect = &DrawRect;
	DrawDiscriptor.m_pRect = &m_IndicatorRect;
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
	EG_Coord_t bg_radius = GetStyleRadius(EG_PART_MAIN);
	EG_Coord_t short_side = EG_MIN(BarWidth, BarHeight);
	if(bg_radius > short_side >> 1) bg_radius = short_side >> 1;
	EG_Coord_t indic_radius = DrawRect.m_Radius;
	short_side = EG_MIN(indicw, indich);
	if(indic_radius > short_side >> 1) indic_radius = short_side >> 1;
	/* Draw only the shadow and outline only if the indicator is long enough.
     *The radius of the bg and the indicator can make Animate strange shape where
     *it'd be very difficult to draw shadow.*/
	if((Horizontal && m_IndicatorRect.GetWidth() > indic_radius * 2) ||
		 (!Horizontal && m_IndicatorRect.GetHeight() > indic_radius * 2)) {
		EG_OPA_t bg_opa = DrawRect.m_BackgroundOPA;
		EG_OPA_t bg_img_opa = DrawRect.m_BackImageOPA;
		EG_OPA_t border_opa = DrawRect.m_BorderOPA;
		DrawRect.m_BackgroundOPA = EG_OPA_TRANSP;
		DrawRect.m_BackImageOPA = EG_OPA_TRANSP;
		DrawRect.m_BorderOPA = EG_OPA_TRANSP;
		DrawRect.Draw(pContext, &m_IndicatorRect);
		DrawRect.m_BackgroundOPA = bg_opa;
		DrawRect.m_BackImageOPA = bg_img_opa;
		DrawRect.m_BorderOPA = border_opa;
	}
#if EG_DRAW_COMPLEX
	MaskRadiusParam_t BackMaskParam;
	EGRect BackMaskRect;
	BackMaskRect.SetX1(m_Rect.GetX1() + PadLeft);
	BackMaskRect.SetX2(m_Rect.GetX2() - PadRight);
	BackMaskRect.SetY1(m_Rect.GetY1() + PadTop);
	BackMaskRect.SetY2(m_Rect.GetY2() - PadBottom);
	DrawMaskSetRadius(&BackMaskParam, &BackMaskRect, bg_radius, false);
	EG_Coord_t mask_bg_id = DrawMaskAdd(&BackMaskParam, NULL);
#endif
	// Draw_only the background and background image
	EG_OPA_t shadow_opa = DrawRect.m_ShadowOPA;
	EG_OPA_t border_opa = DrawRect.m_BorderOPA;
	DrawRect.m_BorderOPA = EG_OPA_TRANSP;
	DrawRect.m_ShadowOPA = EG_OPA_TRANSP;
	// Get the Maximum possible indicator area. The gradient should be applied on this
	EGRect MaskMaxRect(Rect);
	MaskMaxRect.Deflate(PadLeft, PadRight, PadTop, PadBottom);
	if(Horizontal && MaskMaxRect.GetHeight() < EG_BAR_SIZE_MIN) {
		MaskMaxRect.SetY1(m_Rect.GetY1() + (BarHeight / 2) - (EG_BAR_SIZE_MIN / 2));
		MaskMaxRect.SetY2(MaskMaxRect.GetY1() + EG_BAR_SIZE_MIN);
	}
	else if(!Horizontal && MaskMaxRect.GetWidth() < EG_BAR_SIZE_MIN) {
		MaskMaxRect.SetX1(m_Rect.GetX1() + (BarWidth / 2) - (EG_BAR_SIZE_MIN / 2));
		MaskMaxRect.SetX2(MaskMaxRect.GetX1() + EG_BAR_SIZE_MIN);
	}
#if EG_DRAW_COMPLEX
	// Create Animate mask to the current indicator area to see only this part from the whole gradient.
	MaskRadiusParam_t mask_indic_param;
	DrawMaskSetRadius(&mask_indic_param, &m_IndicatorRect, DrawRect.m_Radius, false);
	int16_t mask_indic_id = DrawMaskAdd(&mask_indic_param, NULL);
#endif
	DrawRect.Draw(pContext, &MaskMaxRect);
	DrawRect.m_BorderOPA = border_opa;
	DrawRect.m_ShadowOPA = shadow_opa;
	// Draw the border
	DrawRect.m_BackgroundOPA = EG_OPA_TRANSP;
	DrawRect.m_BackImageOPA = EG_OPA_TRANSP;
	DrawRect.m_ShadowOPA = EG_OPA_TRANSP;
	DrawRect.Draw(pContext, &m_IndicatorRect);
#if EG_DRAW_COMPLEX
	DrawMaskFreeParam(&mask_indic_param);
	DrawMaskFreeParam(&BackMaskParam);
	DrawMaskRemove(mask_indic_id);
	DrawMaskRemove(mask_bg_id);
#endif
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::Animate(void *pValue, int32_t Value)
{
	EG_BarAnimation_t *pAnimate = (EG_BarAnimation_t*)pValue;
	pAnimate->AnimationState = Value;
	pAnimate->pBar->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::AnimateEnd(EGAnimate *pAnimation)
{
	EG_BarAnimation_t *pAnimate = (EG_BarAnimation_t*)pAnimation->m_pItem;
	EGBar *pBar = (EGBar*)pAnimate->pBar;
	pAnimate->AnimationState = EG_BAR_ANIM_STATE_INV;
	if(pAnimate == &pBar->m_CurrentValueAnimation)	pBar->m_CurrentValue = pAnimate->AnimationEnd;
	else if(pAnimate == &pBar->m_StartValueAnimation) pBar->m_StartValue = pAnimate->AnimationEnd;
	pBar->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::SetValueWithAnimation(int32_t NewValue, int32_t *pValue, EG_BarAnimation_t *pInfo, EG_AnimateEnable_e Enable)
{
	if(Enable == EG_ANIM_OFF) {
		EGAnimate::Delete(pInfo, nullptr);
		pInfo->AnimationState = EG_BAR_ANIM_STATE_INV;
		*pValue = NewValue;
		Invalidate();
	}
	else {
		if(pInfo->AnimationState == EG_BAR_ANIM_STATE_INV) {		// No animation in progress -> simply set the values
			pInfo->AnimstionStart = *pValue;
			pInfo->AnimationEnd = NewValue;
		}
		else {		// Animation in progress. Start from the animation end Value
			pInfo->AnimstionStart = pInfo->AnimationEnd;
			pInfo->AnimationEnd = NewValue;
		}
		*pValue = NewValue;
		EGAnimate::Delete(pInfo, nullptr);		// Stop the previous animation if it exists
		EGAnimate Animate;
		Animate.SetItem(pInfo);
		Animate.SetExcCB(EGBar::Animate);
		Animate.SetValues(EG_BAR_ANIM_STATE_START, EG_BAR_ANIM_STATE_END);
		Animate.SetEndCB(EGBar::AnimateEnd);
		Animate.SetTime(GetStyleAnimationTime(EG_PART_MAIN));
		EGAnimate::Create(&Animate);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGBar::InitialiseAnimmation(EG_BarAnimation_t *pBarAnimate)
{
	pBarAnimate->pBar = this;
	pBarAnimate->AnimstionStart = 0;
	pBarAnimate->AnimationEnd = 0;
	pBarAnimate->AnimationState = EG_BAR_ANIM_STATE_INV;
}

#endif
