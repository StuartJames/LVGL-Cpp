
/*
 *                LEGL 2025-2026 HydraSystems.
 *
 *  This m_Pressedogram is free software; you can redistribute it and/or   
 *  modify it under the terms of the GNU General Public License as  
 *  published by the Free Software Foundation; either version 2 of  
 *  the License, or (at your option) any later version.             
 *                                                                  
 *  This m_Pressedogram is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of  
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   
 *  GNU General Public License for more details.                    
 * 
 *  Based on a design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Dem_Screeniption
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "EGL.h"

//////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_THEME_MONO

#include "extra/themes/EG_MonoTheme.h"
#include "misc/lv_gc.h"

//////////////////////////////////////////////////////////////////////////////////////

#define COLOR_FG Dark ? EG_ColorWhite() : EG_ColorBlack()
#define COLOR_BG Dark ? EG_ColorBlack() : EG_ColorWhite()

#define BORDER_W_NORMAL 1
#define BORDER_W_PR 3
#define BORDER_W_DIS 0
#define BORDER_W_FOCUS 1
#define BORDER_W_EDIT 2
#define PAD_DEF 4

//////////////////////////////////////////////////////////////////////////////////////

EGMonoTheme *EGMonoTheme::m_pTheme = nullptr;

//////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGMonoTheme::SetTheme(bool Dark, const EG_Font_t *pFont, EGDisplay *pDisplay /*= nullptr*/)
{
  if(pDisplay == nullptr) pDisplay = EGDisplay::GetDefault();
  if(pDisplay == nullptr) return nullptr;
  if(m_pTheme == nullptr) m_pTheme = new EGMonoTheme;
  ((EGMonoTheme*)m_pTheme)->Initialise(pDisplay, Dark, pFont);
  pDisplay->m_pTheme = m_pTheme;
  return m_pTheme;
}

//////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGMonoTheme::GetTheme(void)
{
	if(m_pTheme == nullptr) return nullptr;
  return (EGTheme*)m_pTheme;
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGMonoTheme::IsCreated(void)
{
	return (m_pTheme == nullptr) ? false : true;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGMonoTheme::Initialise(EGDisplay *pDisplay, bool Dark, const EG_Font_t *pFont)
{
	m_pDisplay = pDisplay;
	m_pFontSmall = EG_FONT_DEFAULT;
	m_pFontNormal = EG_FONT_DEFAULT;
	m_pFontLarge = EG_FONT_DEFAULT;
	InitialiseStyles(Dark, pFont);
	if(pDisplay == nullptr || EGDisplay::GetTheme(pDisplay) == this) EGObject::ReportStyleChange(nullptr);
	m_Initialised = true;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGMonoTheme::InitialiseStyles(bool Dark, const EG_Font_t *pFont)
{
	StyleInitialiseReset(&m_Scrollbar);
	m_Scrollbar.SetBackOPA(EG_OPA_COVER);
	m_Scrollbar.SetBackColor(COLOR_FG);
	m_Scrollbar.SetWidth(PAD_DEF);

	StyleInitialiseReset(&m_Screen);
	m_Screen.SetBackOPA(EG_OPA_COVER);
	m_Screen.SetBackColor(COLOR_BG);
	m_Screen.SetTextColor(COLOR_FG);
	m_Screen.SetPaddingRow(PAD_DEF);
	m_Screen.SetPaddingColumn(PAD_DEF);
	m_Screen.SetTextFont(pFont);

	StyleInitialiseReset(&m_Card);
	m_Card.SetBackOPA(EG_OPA_COVER);
	m_Card.SetBackColor(COLOR_BG);
	m_Card.SetBorderColor(COLOR_FG);
	m_Card.SetRadius(2);
	m_Card.SetBorderWidth(BORDER_W_NORMAL);
	m_Card.SetPaddingAll(PAD_DEF);
	m_Card.SetPaddingGap(PAD_DEF);
	m_Card.SetTextColor(COLOR_FG);
	m_Card.SetLineWidth(2);
	m_Card.SetLineColor(COLOR_FG);
	m_Card.SetArcWidth(2);
	m_Card.SetArcColor(COLOR_FG);
	m_Card.SetOutlineColor(COLOR_FG);
	m_Card.SetAnimateTime(300);

	StyleInitialiseReset(&m_Pressed);
	m_Pressed.SetBorderWidth(BORDER_W_PR);

	StyleInitialiseReset(&m_Inverted);
	m_Inverted.SetBackOPA(EG_OPA_COVER);
	m_Inverted.SetBackColor(COLOR_FG);
	m_Inverted.SetBorderColor(COLOR_BG);
	m_Inverted.SetLineColor(COLOR_BG);
	m_Inverted.SetArcColor(COLOR_BG);
	m_Inverted.SetTextColor(COLOR_BG);
	m_Inverted.SetOutlineColor(COLOR_BG);

	StyleInitialiseReset(&m_Disabled);
	m_Disabled.SetBorderWidth(BORDER_W_DIS);

	StyleInitialiseReset(&m_Focus);
	m_Focus.SetOutlineWidth(1);
	m_Focus.SetOutlinePad(BORDER_W_FOCUS);

	StyleInitialiseReset(&m_Edit);
	m_Edit.SetOutlineWidth(BORDER_W_EDIT);

	StyleInitialiseReset(&m_LargeBorder);
	m_LargeBorder.SetBorderWidth(BORDER_W_EDIT);

	StyleInitialiseReset(&m_PadGap);
	m_PadGap.SetPaddingGap(PAD_DEF);

	StyleInitialiseReset(&m_PadZero);
	m_PadZero.SetPaddingAll(0);
	m_PadZero.SetPaddingGap(0);

	StyleInitialiseReset(&m_NoRadius);
	m_NoRadius.SetRadius(0);

	StyleInitialiseReset(&m_RadiusCircle);
	m_RadiusCircle.SetRadius(EG_RADIUS_CIRCLE);

	StyleInitialiseReset(&m_LargeLineSpace);
	m_LargeLineSpace.SetTextLineSpace(6);

	StyleInitialiseReset(&m_Underline);
	m_Underline.SetTextDecoration(EG_TEXT_DECOR_UNDERLINE);

#if EG_USE_TEXTAREA
	StyleInitialiseReset(&m_TextCursor);
	m_TextCursor.SetBorderSide(EG_BORDER_SIDE_LEFT);
	m_TextCursor.SetBorderColor(COLOR_FG);
	m_TextCursor.SetBorderWidth(2);
	m_TextCursor.SetBackOPA(EG_OPA_TRANSP);
	m_TextCursor.SetAnimateTime(500);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

void EGMonoTheme::ApplyTheme(EGObject *pObj)
{
	EGObject *pParent = pObj->GetParent();
	if(pParent == NULL) {
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		return;
	}

	if(EGObject::IsKindOf(pObj, &c_ObjectClass)) {
#if EG_USE_TABVIEW
		// Tabview content area*/
		if(EGObject::IsKindOf(pParent, &c_TabViewClass))	return;
		else if(EGObject::IsKindOf(pParent->GetParent(), &c_TabViewClass)) {		// Tabview pages
			pObj->AddStyle(&m_Card, 0);
			pObj->AddStyle(&m_NoRadius, 0);
			pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
			return;
		}
#endif

#if EG_USE_WIN
		// Header*/
		if(pObj->GetIndex() == 0 && EGObject::IsKindOf(pParent , &c_WindowClass)) {
			pObj->AddStyle(&m_Card, 0);
			pObj->AddStyle(&m_NoRadius, 0);
			return;
		}
		// Content*/
		else if(pObj->GetIndex() == 1 && EGObject::IsKindOf(pParent , &c_WindowClass)) {
			pObj->AddStyle(&m_Card, 0);
			pObj->AddStyle(&m_NoRadius, 0);
			pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
			return;
		}
#endif
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
	}
#if EG_USE_BTN
	else if(EGObject::IsKindOf(pObj, &c_ButtonClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Pressed, EG_STATE_PRESSED);
		pObj->AddStyle(&m_Inverted, EG_STATE_CHECKED);
		pObj->AddStyle(&m_Disabled, EG_STATE_DISABLED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_BTNMATRIX
	else if(EGObject::IsKindOf(pObj, &c_ButtonMatrixClass)) {
#if EG_USE_MSGBOX
		if(pParent , &c_MsgBoxClass)){
			pObj->AddStyle(&m_PadGap, 0);
			pObj->AddStyle(&m_Card, EG_PART_ITEMS);
			pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
			pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
			pObj->AddStyle(&m_Underline, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_LargeBorder, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			return;
		}
#endif
#if EG_USE_TABVIEW
		if(EGObject::IsKindOf(pParent , &c_TabViewClass)){
			pObj->AddStyle(&m_PadGap, 0);
			pObj->AddStyle(&m_Card, EG_PART_ITEMS);
			pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
			pObj->AddStyle(&m_Inverted, EG_PART_ITEMS | EG_STATE_CHECKED);
			pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
			pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_Underline, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_LargeBorder, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			return;
		}
#endif
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Card, EG_PART_ITEMS);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_Inverted, EG_PART_ITEMS | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
		pObj->AddStyle(&m_Underline, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_LargeBorder, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
	}
#endif

#if EG_USE_BAR
	else if(EGObject::IsKindOf(pObj, &c_BarClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadZero, 0);
		pObj->AddStyle(&m_Inverted, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
	}
#endif

#if EG_USE_SLIDER
	else if(EGObject::IsKindOf(pObj, &c_SliderClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadZero, 0);
		pObj->AddStyle(&m_Inverted, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Card, EG_PART_KNOB);
		pObj->AddStyle(&m_RadiusCircle, EG_PART_KNOB);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_TABLE
	else if(EGObject::IsKindOf(pObj, &c_TableClass)) {
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_Card, EG_PART_ITEMS);
		pObj->AddStyle(&m_NoRadius, EG_PART_ITEMS);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Inverted, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_CHECKBOX
	else if(EGObject::IsKindOf(pObj, &c_CheckboxClass)) {
		pObj->AddStyle(&m_PadGap, EG_PART_MAIN);
		pObj->AddStyle(&m_Card, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Disabled, EG_PART_INDICATOR | EG_STATE_DISABLED);
		pObj->AddStyle(&m_Inverted, EG_PART_INDICATOR | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Pressed, EG_PART_INDICATOR | EG_STATE_PRESSED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_SWITCH
	else if(EGObject::IsKindOf(pObj, &c_SwitchClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_RadiusCircle, 0);
		pObj->AddStyle(&m_PadZero, 0);
		pObj->AddStyle(&m_Inverted, EG_PART_INDICATOR);
		pObj->AddStyle(&m_RadiusCircle, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Card, EG_PART_KNOB);
		pObj->AddStyle(&m_RadiusCircle, EG_PART_KNOB);
		pObj->AddStyle(&m_PadZero, EG_PART_KNOB);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_CHART
	else if(EGObject::IsKindOf(pObj, &c_ChartClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_Card, EG_PART_ITEMS);
		pObj->AddStyle(&m_Card, EG_PART_TICKS);
		pObj->AddStyle(&m_Card, EG_PART_CURSOR);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
	}
#endif

#if EG_USE_ROLLER
	else if(EGObject::IsKindOf(pObj, &c_RollerClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_LargeLineSpace, 0);
		pObj->AddStyle(&m_Inverted, EG_PART_SELECTED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_DROPDOWN
	else if(EGObject::IsKindOf(pObj, &c_DropDownClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Pressed, EG_STATE_PRESSED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
	else if(EGObject::IsKindOf(pObj, &c_DropDownListClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_LargeLineSpace, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_Inverted, EG_PART_SELECTED | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Pressed, EG_PART_SELECTED | EG_STATE_PRESSED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_ARC
	else if(EGObject::IsKindOf(pObj, &c_ArcClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Inverted, EG_PART_INDICATOR);
		pObj->AddStyle(&m_PadZero, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Card, EG_PART_KNOB);
		pObj->AddStyle(&m_RadiusCircle, EG_PART_KNOB);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_METER
	else if(EGObject::IsKindOf(pObj, &c_MeterClass)) {
		pObj->AddStyle(&m_Card, 0);
	}
#endif

#if EG_USE_TEXTAREA
	else if(EGObject::IsKindOf(pObj, &c_TextBoxClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_TextCursor, EG_PART_CURSOR | EG_STATE_FOCUSED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUSED);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif

#if EG_USE_CALENDAR
	else if(EGObject::IsKindOf(pObj, &c_CalendarClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_NoRadius, 0);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
		pObj->AddStyle(&m_LargeBorder, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
	}
#endif

#if EG_USE_KEYBOARD
	else if(EGObject::IsKindOf(pObj, &c_KeyboardClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Card, EG_PART_ITEMS);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_Inverted, EG_PART_ITEMS | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
		pObj->AddStyle(&m_LargeBorder, EG_PART_ITEMS | EG_STATE_EDITED);
	}
#endif
#if EG_USE_LIST
	else if(EGObject::IsKindOf(pObj, &c_ListClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		return;
	}
	else if(EGObject::IsKindOf(pObj, &c_ListTextClass)) {
	}
	else if(EGObject::IsKindOf(pObj, &c_ListButtonClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Pressed, EG_STATE_PRESSED);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_LargeBorder, EG_STATE_EDITED);
	}
#endif
#if EG_USE_MSGBOX
	else if(EGObject::IsKindOf(pObj, &c_MsgBoxClass)) {
		pObj->AddStyle(&m_Card, 0);
		return;
	}
#endif
#if EG_USE_SPINBOX
	else if(EGObject::IsKindOf(pObj, &c_SpinboxClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Inverted, EG_PART_CURSOR);
		pObj->AddStyle(&m_Focus, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Edit, EG_STATE_EDITED);
	}
#endif
#if EG_USE_TILEVIEW
	else if(EGObject::IsKindOf(pObj, &c_TabViewClass)) {
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
	}
	else if(EGObject::IsKindOf(pObj, &c_TileViewTitleClass)) {
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
	}
#endif

#if EG_USE_LED
	else if(EGObject::IsKindOf(pObj, &c_LedClass)) {
		pObj->AddStyle(&m_Card, 0);
	}
#endif
}

#endif
