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

#include "widgets/EG_Checkbox.h"
#if EG_USE_CHECKBOX != 0

#include "misc/EG_Assert.h"
#include "misc/EG_ArabicPersianText.h"
#include "core/EG_Group.h"
#include "draw/EG_DeviceContext.h"

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_CheckboxClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGCheckbox::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGCheckbox::EGCheckbox(void) : EGObject(),
	m_pText(nullptr),
	m_StaticText(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGCheckbox::EGCheckbox(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_CheckboxClass*/) : EGObject(),
	m_pText(nullptr),
	m_StaticText(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGCheckbox::~EGCheckbox(void)
{
	if(!m_StaticText) {
		EG_FreeMem((void *)m_pText);
		m_pText = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCheckbox::Configure(void)
{
  EGObject::Configure();
	m_pText = "Check box";
	m_StaticText = 1;
	AddFlag(EG_OBJ_FLAG_CLICKABLE);
	AddFlag(EG_OBJ_FLAG_CHECKABLE);
	AddFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCheckbox::SetText(const char *pText)
{
#if EG_USE_ARABIC_PERSIAN_CHARS
	size_t Length = _lv_txt_ap_calc_bytes_cnt(pText);
#else
	size_t Length = strlen(pText);
#endif
	char *pTextMem = (char*)m_pText;
	if(!m_StaticText) pTextMem = (char*)EG_ReallocMem(pTextMem, Length + 1);
	else pTextMem = (char*)EG_AllocMem(Length + 1);
#if EG_USE_ARABIC_PERSIAN_CHARS
	_lv_txt_ap_proc(pText, pTextMem);
#else
	strcpy(pTextMem, pText);
#endif
	m_pText = pTextMem;
	m_StaticText = 0;
	RefreshSelfSize();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCheckbox::SetStaticText(const char *pText)
{
	if(!m_StaticText) EG_FreeMem((void *)m_pText);
	m_pText = (char *)pText;
	m_StaticText = 1;
	RefreshSelfSize();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGCheckbox::GetText(void)
{
	return m_pText;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCheckbox::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(&c_CheckboxClass) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGCheckbox *pCheckbox = (EGCheckbox*)pEvent->GetTarget();
  pCheckbox->Event(pEvent); // dereference once
}

void EGCheckbox::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  if(Code == EG_EVENT_GET_SELF_SIZE) {
		EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
		const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
		int32_t FontHeight = EG_FontGetLineHeight(pFont);
		int32_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
		int32_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
		EGSize TextSize;
		EG_GetTextSize(&TextSize, m_pText, pFont, Kerning, LineSpace, EG_COORD_MAX, EG_TEXT_FLAG_NONE);
		int32_t bg_colp = GetStylePadColumn(EG_PART_MAIN);
		int32_t marker_leftp = GetStylePadLeft(EG_PART_INDICATOR);
		int32_t marker_rightp = GetStylePadRight(EG_PART_INDICATOR);
		int32_t marker_topp = GetStylePadTop(EG_PART_INDICATOR);
		int32_t marker_bottomp = GetStylePadBottom(EG_PART_INDICATOR);
		EGPoint marker_size;
		marker_size.m_X = FontHeight + marker_leftp + marker_rightp;
		marker_size.m_Y = FontHeight + marker_topp + marker_bottomp;
		pPoint->m_X = marker_size.m_X + TextSize.m_X + bg_colp;
		pPoint->m_Y = EG_MAX(marker_size.m_Y, TextSize.m_Y);
	}
	else if(Code == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		int32_t *pSize = (int32_t*)pEvent->GetParam();
		int32_t m = CalculateExtDrawSize(EG_PART_INDICATOR);
		*pSize = EG_MAX(*pSize, m);
	}
	else if(Code == EG_EVENT_DRAW_MAIN)	Draw(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCheckbox::Draw(EGEvent *pEvent)
{
	EGDeviceContext *pDC = pEvent->GetDeviceContext();
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	int32_t FontHeight = EG_FontGetLineHeight(pFont);
	int32_t bg_border = GetStyleBorderWidth(EG_PART_MAIN);
	int32_t bg_topp = GetStylePadTop(EG_PART_MAIN) + bg_border;
	int32_t bg_leftp = GetStylePadLeft(EG_PART_MAIN) + bg_border;
	int32_t bg_colp = GetStylePadColumn(EG_PART_MAIN);
	int32_t marker_leftp = GetStylePadLeft(EG_PART_INDICATOR);
	int32_t marker_rightp = GetStylePadRight(EG_PART_INDICATOR);
	int32_t marker_topp = GetStylePadTop(EG_PART_INDICATOR);
	int32_t marker_bottomp = GetStylePadBottom(EG_PART_INDICATOR);
	int32_t TransformWidth = GetStyleTransformWidth(EG_PART_INDICATOR);
	int32_t TransformHeight = GetStyleTransformHeight(EG_PART_INDICATOR);
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_INDICATOR, &DrawRect);
	EGRect MarkerRect;
	MarkerRect.SetX1(m_Rect.GetX1() + bg_leftp);
	MarkerRect.SetX2(MarkerRect.GetX1() + FontHeight + marker_leftp + marker_rightp - 1);
	MarkerRect.SetY1(m_Rect.GetY1() + bg_topp);
	MarkerRect.SetY2(MarkerRect.GetY1() + FontHeight + marker_topp + marker_bottomp - 1);
	EGRect MarkerTransform(&MarkerRect);
	MarkerTransform.Inflate(TransformWidth, TransformHeight);
	EGEventDC EventDC(pDC); 
	EventDC.m_pDrawRect = &DrawRect;
	EventDC.m_pClass = m_pClass;
	EventDC.m_Type = EG_CHECKBOX_DRAW_PART_BOX;
	EventDC.m_pRect = &MarkerTransform;
	EventDC.m_Part = EG_PART_INDICATOR;
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &EventDC); 
	DrawRect.Draw(pDC, &MarkerTransform);
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &EventDC);
	int32_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	int32_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
	EGSize TextSize;
	EG_GetTextSize(&TextSize, m_pText, pFont, Kerning, LineSpace, EG_COORD_MAX, EG_TEXT_FLAG_NONE);
	EGDrawLabel DrawLabel;
	InititialseDrawLabel(EG_PART_MAIN, &DrawLabel);
	int32_t y_ofs = (MarkerRect.GetHeight() - FontHeight) / 2;
	EGRect TextRect;
	TextRect.SetX1(MarkerRect.GetX2() + bg_colp);
	TextRect.SetX2(TextRect.GetX1() + TextSize.m_X);
	TextRect.SetY1(m_Rect.GetY1() + bg_topp + y_ofs);
	TextRect.SetY2(TextRect.GetY1() + TextSize.m_Y);
	DrawLabel.Draw(pDC, &TextRect, m_pText, nullptr);
}


#endif
