
#include "EGL.h" /*To see all the widgets*/

#if EG_USE_THEME_BASIC

#include "extra/themes/EG_BasicTheme.h"
#include "misc/lv_gc.h"

////////////////////////////////////////////////////////////////////////////////////

#define COLOR_SCR EG_LightPalette(EG_PALETTE_GREY, 4)
#define COLOR_WHITE EG_ColorWhite()
#define COLOR_LIGHT EG_LightPalette(EG_PALETTE_GREY, 2)
#define COLOR_DARK EG_MainPalette(EG_PALETTE_GREY)
#define COLOR_DIM EG_DarkPalette(EG_PALETTE_GREY, 2)
#define SCROLLBAR_WIDTH 2

//////////////////////////////////////////////////////////////////////////////////////

EGBasicTheme *EGBasicTheme::m_pTheme = nullptr;

//////////////////////////////////////////////////////////////////////////////////////

EGBasicTheme::EGBasicTheme(void) : EGTheme()
{ 
}

//////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGBasicTheme::SetTheme(EGDisplay *pDisplay /*= nullptr*/)
{
  if(pDisplay == nullptr) pDisplay = EGDisplay::GetDefault();
  if(pDisplay == nullptr) return nullptr;
  if(m_pTheme == nullptr) m_pTheme = new EGBasicTheme;
  ((EGBasicTheme*)m_pTheme)->Initialise(pDisplay);
  pDisplay->m_pTheme = m_pTheme;
  return m_pTheme;
}

//////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGBasicTheme::GetTheme(void)
{
	if(m_pTheme == nullptr) return nullptr;
  return (EGTheme*)m_pTheme;
}

////////////////////////////////////////////////////////////////////////////////////

bool EGBasicTheme::IsCreated(void)
{
	return (m_pTheme == nullptr) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////////

void EGBasicTheme::Initialise(EGDisplay *pDisplay)
{
	m_pDisplay = pDisplay;
	m_pFontSmall = EG_FONT_DEFAULT;
	m_pFontNormal = EG_FONT_DEFAULT;
	m_pFontLarge = EG_FONT_DEFAULT;
	InitialiseStyles();
	if(pDisplay == NULL || EGDisplay::GetTheme(pDisplay) == this) EGObject::ReportStyleChange(NULL);
	m_Initialised = true;
}

////////////////////////////////////////////////////////////////////////////////////

void EGBasicTheme::InitialiseStyles(void)
{
	StyleInitialiseReset(&m_Scrollbar);
	m_Scrollbar.SetBackOPA(EG_OPA_COVER);
	m_Scrollbar.SetBackColor(COLOR_DARK);
	m_Scrollbar.SetWidth(SCROLLBAR_WIDTH);

	StyleInitialiseReset(&m_Screen);
	m_Screen.SetBackOPA(EG_OPA_COVER);
	m_Screen.SetBackColor(COLOR_SCR);
	m_Screen.SetTextColor(COLOR_DIM);

	StyleInitialiseReset(&m_Transparent);
	m_Transparent.SetBackOPA(EG_OPA_TRANSP);

	StyleInitialiseReset(&m_White);
	m_White.SetBackOPA(EG_OPA_COVER);
	m_White.SetBackColor(COLOR_WHITE);
	m_White.SetLineWidth(1);
	m_White.SetLineColor(COLOR_WHITE);
	m_White.SetArcWidth(2);
	m_White.SetArcColor(COLOR_WHITE);

	StyleInitialiseReset(&m_Light);
	m_Light.SetBackOPA(EG_OPA_COVER);
	m_Light.SetBackColor(COLOR_LIGHT);
	m_Light.SetLineWidth(1);
	m_Light.SetLineColor(COLOR_LIGHT);
	m_Light.SetArcWidth(2);
	m_Light.SetArcColor(COLOR_LIGHT);

	StyleInitialiseReset(&m_Dark);
	m_Dark.SetBackOPA(EG_OPA_COVER);
	m_Dark.SetBackColor(COLOR_DARK);
	m_Dark.SetLineWidth(1);
	m_Dark.SetLineColor( COLOR_DARK);
	m_Dark.SetArcWidth(2);
	m_Dark.SetArcColor(COLOR_DARK);

	StyleInitialiseReset(&m_Dim);
	m_Dim.SetBackOPA(EG_OPA_COVER);
	m_Dim.SetBackColor(COLOR_DIM);
	m_Dim.SetLineWidth(1);
	m_Dim.SetLineColor(COLOR_DIM);
	m_Dim.SetArcWidth(2);
	m_Dim.SetArcColor(COLOR_DIM);

#if EG_USE_ARC || EG_USE_COLORWHEEL
	StyleInitialiseReset(&m_ArcLine);
	m_ArcLine.SetArcWidth(6);
	StyleInitialiseReset(&m_ArcKnob);
	m_ArcLine.SetPaddingAll(5);
#endif

#if EG_USE_TEXTAREA
	StyleInitialiseReset(&m_TextCursor);
	m_TextCursor.SetBorderSide(EG_BORDER_SIDE_LEFT);
	m_TextCursor.SetBorderColor(COLOR_DIM);
	m_TextCursor.SetBorderWidth(2);
	m_TextCursor.SetBackOPA(EG_OPA_TRANSP);
	m_TextCursor.SetAnimateTime(500);
#endif
}

////////////////////////////////////////////////////////////////////////////////////

void EGBasicTheme::ApplyTheme(EGObject *pObj)
{
	EGObject *pParent = pObj->GetParent();
	if(pParent == NULL) {
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		return;
	}
	if(EGObject::IsKindOf(pObj, &c_ObjectClass)) {
#if EG_USE_TABVIEW
		// Tabview content area
		if(EGObject::IsKindOf(pParent, &c_TabViewClass)) {
  		pObj->AddStyle(&m_Screen, 0);
			return;
		}
		/*Tabview pages*/
		else if(EGObject::IsKindOf(pParent->GetParent(), &c_TabViewClass)) {
  		pObj->AddStyle(&m_Screen, 0);
		  pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
			return;
		}
#endif

#if EG_USE_WIN
		/*Header*/
		if(pObj->GetIndex() == 0 && EGObject::IsKindOf(pParent, &c_WindowClass)) {
			pObj->AddStyle(&m_Light, 0);
			return;
		}
		/*Content*/
		else if(pObj->GetIndex() == 1 && EGObject::IsKindOf(pParent, &c_WindowClass)) {
			pObj->AddStyle(&m_Light, 0);
			pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
			return;
		}
#endif
		pObj->AddStyle(&m_White, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
	}
#if EG_USE_BTN
	else if(EGObject::IsKindOf(pObj, &c_ButtonClass)) {
		pObj->AddStyle(&m_Dark, 0);
	}
#endif

#if EG_USE_BTNMATRIX
	else if(EGObject::IsKindOf(pObj, &c_ButtonMatrixClass)) {
#if EG_USE_MSGBOX
    if(EGObject::IsKindOf(pParent, &c_MsgBoxClass)){
			pObj->AddStyle(&m_Light, 0);
			return;
		}
#endif
#if EG_USE_TABVIEW
    if(EGObject::IsKindOf(pParent, &c_TabViewClass)){
			pObj->AddStyle(&m_Light, 0);
			return;
		}
#endif
		pObj->AddStyle(&m_White, 0);
		pObj->AddStyle(&m_Light, EG_PART_ITEMS);
	}
#endif

#if EG_USE_BAR
	else if(EGObject::IsKindOf(pObj, &c_BarClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Dark, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_SLIDER
	else if(EGObject::IsKindOf(pObj, &c_SliderClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Dark, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Dim, EG_PART_KNOB);
	}
#endif

#if EG_USE_TABLE
	else if(EGObject::IsKindOf(pObj, &c_TableClass)) {
		pObj->AddStyle(&m_Light, EG_PART_ITEMS);
		pObj->AddStyle(&m_Scrollbar, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_CHECKBOX
	else if(EGObject::IsKindOf(pObj, &c_CheckboxClass)) {
		pObj->AddStyle(&m_Light, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Dark, EG_PART_INDICATOR | EG_STATE_CHECKED);
	}
#endif

#if EG_USE_SWITCH
	else if(EGObject::IsKindOf(pObj, &c_SwitchClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Dim, EG_PART_KNOB);
	}
#endif

#if EG_USE_CHART
	else if(EGObject::IsKindOf(pObj, &c_ChartClass)) {
		pObj->AddStyle(&m_White, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_Light, EG_PART_ITEMS);
		pObj->AddStyle(&m_Dark, EG_PART_TICKS);
		pObj->AddStyle(&m_Dark, EG_PART_CURSOR);
	}
#endif

#if EG_USE_ROLLER
	else if(EGObject::IsKindOf(pObj, &c_RollerClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Dark, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_DROPDOWN
	else if(EGObject::IsKindOf(pObj, &c_DropDownClass)) {
		pObj->AddStyle(&m_White, 0);
	}
	else if(EGObject::IsKindOf(pObj, &c_DropDownListClass)) {
		pObj->AddStyle(&m_White, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_Light, EG_PART_SELECTED);
		pObj->AddStyle(&m_Dark, EG_PART_SELECTED | EG_STATE_CHECKED);
	}
#endif

#if EG_USE_ARC
	else if(EGObject::IsKindOf(pObj, &c_ArcClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Transparent, 0);
		pObj->AddStyle(&m_ArcLine, 0);
		pObj->AddStyle(&m_Dark, EG_PART_INDICATOR);
		pObj->AddStyle(&m_ArcLine, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Dim, EG_PART_KNOB);
		pObj->AddStyle(&m_ArcKnob, EG_PART_KNOB);
	}
#endif

#if EG_USE_SPINNER
	else if(EGObject::IsKindOf(pObj, &c_SpinnerClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Transparent, 0);
		pObj->AddStyle(&m_ArcLine, 0);
		pObj->AddStyle(&m_Dark, EG_PART_INDICATOR);
		pObj->AddStyle(&m_ArcLine, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_COLORWHEEL
	else if(EGObject::IsKindOf(pObj, &c_ColorWheelClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Transparent, 0);
		pObj->AddStyle(&m_ArcLine, 0);
		pObj->AddStyle(&m_Dim, EG_PART_KNOB);
		pObj->AddStyle(&m_ArcKnob, EG_PART_KNOB);
	}
#endif

#if EG_USE_METER
	else if(EGObject::IsKindOf(pObj, &c_MeterClass)) {
		pObj->AddStyle(&m_Light, 0);
	}
#endif

#if EG_USE_TEXTAREA
	else if(EGObject::IsKindOf(pObj, &c_EditClass)) {
		pObj->AddStyle(&m_White, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_TextCursor, EG_PART_CURSOR | EG_STATE_FOCUSED);
	}
#endif

#if EG_USE_CALENDAR
	else if(EGObject::IsKindOf(pObj, &c_CalendarClass)) {
		pObj->AddStyle(&m_Light, 0);
	}
#endif

#if EG_USE_KEYBOARD
	else if(EGObject::IsKindOf(pObj, &c_KeyboardClass)) {
		pObj->AddStyle(&m_Screen, 9);
		pObj->AddStyle(&m_White, EG_PART_ITEMS);
		pObj->AddStyle(&m_Light, EG_PART_ITEMS | EG_STATE_CHECKED);
	}
#endif
#if EG_USE_LIST
	else if(EGObject::IsKindOf(pObj, &c_ListClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		return;
	}
	else if(EGObject::IsKindOf(pObj, &c_ListTextClass)) {
	}
	else if(EGObject::IsKindOf(pObj, &c_ListButtonClass)) {
		pObj->AddStyle(&m_Dark, 0);
	}
#endif
#if EG_USE_MSGBOX
	else if(EGObject::IsKindOf(pObj, &c_MsgBoxClass)) {
		pObj->AddStyle(&m_Light, 0);
		return;
	}
#endif
#if EG_USE_SPINBOX
	else if(EGObject::IsKindOf(pObj, &c_SpinboxClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Dark, EG_PART_CURSOR);
	}
#endif
#if EG_USE_TILEVIEW
	else if(EGObject::IsKindOf(pObj, &c_TileViewClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
	}
	else if(EGObject::IsKindOf(pObj, &c_TileViewTitleClass)) {
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
	}
#endif

#if EG_USE_COLORWHEEL
	else if(EGObject::IsKindOf(pObj, &c_ColorWheelClass)) {
		pObj->AddStyle(&m_Light, 0);
		pObj->AddStyle(&m_Light, EG_PART_KNOB);
	}
#endif

#if EG_USE_LED
	else if(EGObject::IsKindOf(pObj, &c_LedClass)) {
		pObj->AddStyle(&m_Light, 0);
	}
#endif
}

#endif
