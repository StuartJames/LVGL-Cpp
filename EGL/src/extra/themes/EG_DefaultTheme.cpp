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

#include "EGL.h" // To see all the widgets

#if EG_USE_THEME_DEFAULT

#include "extra/themes/EG_DefaultTheme.h"
#include "misc/lv_gc.h"

//////////////////////////////////////////////////////////////////////////////////////

// SCREEN
const EG_Color_t LightColorScreen = EG_LightPalette(EG_PALETTE_GREY, 4);
const EG_Color_t LightColorCard   = EG_ColorWhite();
const EG_Color_t LightColorText   = EG_DarkPalette(EG_PALETTE_GREY, 4);
const EG_Color_t LightColorGrey   = EG_LightPalette(EG_PALETTE_GREY, 2);
const EG_Color_t DarkColorScreen  = EG_ColorHex(0x15171A);
const EG_Color_t DarkColorCard    = EG_ColorHex(0x282b30);
const EG_Color_t DarkColorText    = EG_LightPalette(EG_PALETTE_GREY, 5);
const EG_Color_t DarkColorGrey    = EG_ColorHex(0x2f3237);

#define MODE_DARK 0x01
#define RADIUS_DEFAULT (m_DisplaySize == DISP_LARGE ? EG_DisplayDPX(m_pDisplay, 12) : EG_DisplayDPX(m_pDisplay, 8))


#define TRANSITION_TIME EG_THEME_DEFAULT_TRANSITION_TIME
#define BORDER_WIDTH EG_DisplayDPX(m_pDisplay, 2)
#define OUTLINE_WIDTH EG_DisplayDPX(m_pDisplay, 3)

#define PAD_DEF (m_DisplaySize == DISP_LARGE ? EG_DisplayDPX(m_pDisplay, 24) : m_DisplaySize == DISP_MEDIUM ? EG_DisplayDPX(m_pDisplay, 20) : EG_DisplayDPX(m_pDisplay, 16))
#define PAD_SMALL (m_DisplaySize == DISP_LARGE ? EG_DisplayDPX(m_pDisplay, 14) : m_DisplaySize == DISP_MEDIUM ? EG_DisplayDPX(m_pDisplay, 12) : EG_DisplayDPX(m_pDisplay, 10))
#define PAD_TINY (m_DisplaySize == DISP_LARGE ? EG_DisplayDPX(m_pDisplay, 8) : m_DisplaySize == DISP_MEDIUM ? EG_DisplayDPX(m_pDisplay, 6) : EG_DisplayDPX(m_pDisplay, 2))

//////////////////////////////////////////////////////////////////////////////////////

EGDefTheme *EGDefTheme::m_pTheme = nullptr;
EG_StyleTransitionDiscriptor_t EGDefTheme::m_TransDelayed;
EG_StyleTransitionDiscriptor_t EGDefTheme::m_TransNormal;
EG_ColorFilterProps_t     EGDefTheme::m_DarkFilter;
EG_ColorFilterProps_t     EGDefTheme::m_GreyFilter;

//////////////////////////////////////////////////////////////////////////////////////

EGDefTheme::EGDefTheme(void) : EGTheme(),
  m_DisplaySize(DISP_MEDIUM)
{
}

//////////////////////////////////////////////////////////////////////////////////////

EGDefTheme::~EGDefTheme(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGDefTheme::DarkColorFilterCB(const EG_ColorFilterProps_t *pFilter, EG_Color_t Color, EG_OPA_t OPA)
{
EG_UNUSED(pFilter);

	return EG_DarkenColor(Color, OPA);
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGDefTheme::GreyFilterCB(const EG_ColorFilterProps_t *pFilter, EG_Color_t Color, EG_OPA_t OPA)
{
EG_UNUSED(pFilter);

	if(m_pTheme->m_Flags & MODE_DARK)	return EG_ColorMix(EG_DarkPalette(EG_PALETTE_GREY, 2), Color, OPA);
	else return EG_ColorMix(EG_LightPalette(EG_PALETTE_GREY, 2), Color, OPA);
}

//////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGDefTheme::SetTheme(EG_Color_t PrimaryColor, EG_Color_t SecondaryColor, bool Dark, const EG_Font_t *pFont, EGDisplay *pDisplay /*= nullptr*/)
{
  if(pDisplay == nullptr) pDisplay = EGDisplay::GetDefault();
  if(pDisplay == nullptr) return nullptr;
  if(m_pTheme == nullptr) m_pTheme = new EGDefTheme;
  ((EGDefTheme*)m_pTheme)->Initialise(pDisplay, PrimaryColor, SecondaryColor, Dark, pFont);
  pDisplay->m_pTheme = m_pTheme;
  return m_pTheme;
}

//////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGDefTheme::GetTheme(void)
{
	if(m_pTheme == nullptr) return nullptr;
  return (EGTheme*)m_pTheme;
}

////////////////////////////////////////////////////////////////////////////////////

bool EGDefTheme::IsCreated(void)
{
	return (m_pTheme == nullptr) ? false : true;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDefTheme::Initialise(EGDisplay *pDisplay, EG_Color_t PrimaryColor, EG_Color_t SecondaryColor, bool Dark, const EG_Font_t *pFont)
{
	if(GetHorizontalResolution() <= 320)	m_DisplaySize = DISP_SMALL;
	else if(GetHorizontalResolution() < 720)	m_DisplaySize = DISP_MEDIUM;
	else m_DisplaySize = DISP_LARGE;
  m_pDisplay = pDisplay;
	m_PrimaryColor = PrimaryColor;
	m_SecondaryColor = SecondaryColor;
	m_pFontSmall = pFont;
	m_pFontNormal = pFont;
	m_pFontLarge = pFont;
	m_Flags = Dark ? MODE_DARK : 0;
	InitialiseStyles(pDisplay);
	if(pDisplay == nullptr || EGDisplay::GetTheme(pDisplay) == this){
    EGObject::ReportStyleChange(nullptr);
  }
	m_Initialised = true;
//  ESP_LOGI("[Theme ]", "Primary:%X", m_PrimaryColor);
//  ESP_LOGI("[Theme ]", "Secondary:%X", m_SecondaryColor);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDefTheme::InitialiseStyles(EGDisplay *pDisplay)
{
static const EGStyleProperty_e TransitionProperties[] = {
  EG_STYLE_BG_OPA,
  EG_STYLE_BG_COLOR,
  EG_STYLE_TRANSFORM_WIDTH,
  EG_STYLE_TRANSFORM_HEIGHT,
  EG_STYLE_TRANSLATE_Y,
  EG_STYLE_TRANSLATE_X,
  EG_STYLE_TRANSFORM_ZOOM,
  EG_STYLE_TRANSFORM_ANGLE,
  EG_STYLE_COLOR_FILTER_OPA,
  EG_STYLE_COLOR_FILTER_DSC,
  EG_STYLE_PROP_INV
};

  if(m_Flags & MODE_DARK){
    m_ScreenColor = DarkColorScreen;
    m_TextColor   = DarkColorText;
    m_CardColor   = DarkColorCard;
    m_GreyColor   = DarkColorGrey;
  }
  else{
    m_ScreenColor = LightColorScreen;
    m_TextColor   = LightColorText;
    m_CardColor   = LightColorCard;
    m_GreyColor   = LightColorGrey;
  }

	StyleInitialiseReset(&m_TransitionDelayed);
	StyleInitialiseReset(&m_TransitionNormal);
#if TRANSITION_TIME
	InitialiseTransitionDiscriptor(&m_TransDelayed, TransitionProperties, EGAnimate::PathLinear, TRANSITION_TIME, 70, NULL);
	InitialiseTransitionDiscriptor(&m_TransNormal, TransitionProperties, EGAnimate::PathLinear, TRANSITION_TIME, 0, NULL);
	m_TransitionDelayed.SetTransition(&m_TransDelayed); // Go back to default state with delay
	m_TransitionNormal.SetTransition(&m_TransNormal); // Go back to default state with delay
#endif
	StyleInitialiseReset(&m_Scrollbar);
	EG_Color_t ScrollBarColor = (m_Flags & MODE_DARK) ? EG_DarkPalette(EG_PALETTE_GREY, 2) :	EG_MainPalette(EG_PALETTE_GREY);
	m_Scrollbar.SetBackColor(ScrollBarColor);
	m_Scrollbar.SetRadius(EG_RADIUS_CIRCLE);
	m_Scrollbar.SetPaddingAll(EG_DisplayDPX(pDisplay, 7));
	m_Scrollbar.SetWidth(EG_DisplayDPX(pDisplay, 5));
	m_Scrollbar.SetBackOPA(EG_OPA_40);
#if TRANSITION_TIME
	m_Scrollbar.SetTransition(&m_TransNormal);
#endif
	StyleInitialiseReset(&m_ScrollbarScrolled);
	m_ScrollbarScrolled.SetBackOPA(EG_OPA_COVER);

	StyleInitialiseReset(&m_Screen);
	m_Screen.SetBackOPA(EG_OPA_COVER);
	m_Screen.SetBackColor(m_ScreenColor);
	m_Screen.SetTextColor(m_TextColor);
	m_Screen.SetPaddingRow(PAD_SMALL);
	m_Screen.SetPaddingColumn(PAD_SMALL);

	StyleInitialiseReset(&m_Card);
	m_Card.SetRadius(RADIUS_DEFAULT);
	m_Card.SetBackOPA(EG_OPA_COVER);
	m_Card.SetBackColor(m_CardColor);
	m_Card.SetBorderColor(m_GreyColor);
	m_Card.SetBorderWidth(BORDER_WIDTH);
	m_Card.SetBorderPost(true);
	m_Card.SetTextColor(m_TextColor);
	m_Card.SetPaddingAll(PAD_DEF);
	m_Card.SetPaddingRow(PAD_SMALL);
	m_Card.SetPaddingColumn(PAD_SMALL);
	m_Card.SetLineColor(EG_MainPalette(EG_PALETTE_GREY));
	m_Card.SetLineWidth(EG_DisplayDPX(pDisplay, 1));

	StyleInitialiseReset(&m_OutlinePrimary);
	m_OutlinePrimary.SetOutlineColor(m_PrimaryColor);
	m_OutlinePrimary.SetOutlineWidth(OUTLINE_WIDTH);
	m_OutlinePrimary.SetOutlinePad(OUTLINE_WIDTH);
	m_OutlinePrimary.SetOutlineOPA(EG_OPA_50);

	StyleInitialiseReset(&m_OutlineSecondary);
	m_OutlineSecondary.SetOutlineColor(m_SecondaryColor);
	m_OutlineSecondary.SetOutlineWidth(OUTLINE_WIDTH);
	m_OutlineSecondary.SetOutlineOPA(EG_OPA_50);

	StyleInitialiseReset(&m_Button);
	m_Button.SetRadius(((m_DisplaySize == DISP_LARGE) ? EG_DisplayDPX(pDisplay, 16) :	(m_DisplaySize == DISP_MEDIUM) ?
                                                        EG_DisplayDPX(pDisplay, 12) : EG_DisplayDPX(pDisplay, 8)));
	m_Button.SetBackOPA(EG_OPA_COVER);
	m_Button.SetBackColor(m_GreyColor);
	if(!(m_Flags & MODE_DARK)) {
		m_Button.SetShadowColor(EG_MainPalette(EG_PALETTE_GREY));
		m_Button.SetShadowWidth(EG_DPX(3));
		m_Button.SetShadowOPA(EG_OPA_50);
		m_Button.SetShadowOffsetY(EG_DisplayDPX(pDisplay, EG_DPX(4)));
	}
	m_Button.SetTextColor(m_TextColor);
	m_Button.SetHorizontalPadding(PAD_DEF);
	m_Button.SetVerticalPadding(PAD_SMALL);
	m_Button.SetPaddingColumn(EG_DisplayDPX(pDisplay, 5));
	m_Button.SetPaddingRow(EG_DisplayDPX(pDisplay, 5));

	EG_ColorFilterInitialise(&m_DarkFilter, DarkColorFilterCB);

	EG_ColorFilterInitialise(&m_GreyFilter, GreyFilterCB);

	StyleInitialiseReset(&m_Pressed);
	m_Pressed.SetColorFilterDiscriptor(&m_DarkFilter);
	m_Pressed.SetColorFilterOPA(35);

	StyleInitialiseReset(&m_Disabled);
	m_Disabled.SetColorFilterDiscriptor(&m_GreyFilter);
	m_Disabled.SetColorFilterOPA(EG_OPA_50);

	StyleInitialiseReset(&m_ClipCorner);
	m_ClipCorner.SetClipCorner(true);
	m_ClipCorner.SetBorderPost(true);

	StyleInitialiseReset(&m_PadNormal);
	m_PadNormal.SetPaddingAll(PAD_DEF);
	m_PadNormal.SetPaddingRow(PAD_DEF);
	m_PadNormal.SetPaddingColumn(PAD_DEF);

	StyleInitialiseReset(&m_PadSmall);
	m_PadSmall.SetPaddingAll(PAD_SMALL);
	m_PadSmall.SetPaddingGap(PAD_SMALL);

	StyleInitialiseReset(&m_PadGap);
	m_PadGap.SetPaddingRow(EG_DisplayDPX(pDisplay, 10));
	m_PadGap.SetPaddingColumn(EG_DisplayDPX(pDisplay, 10));

	StyleInitialiseReset(&m_LineSpaceLarge);
	m_LineSpaceLarge.SetTextLineSpace(EG_DisplayDPX(pDisplay, 20));

	StyleInitialiseReset(&m_TextAlignCenter);
	m_TextAlignCenter.SetTextAlign(EG_TEXT_ALIGN_CENTER);

	StyleInitialiseReset(&m_PadZero);
	m_PadZero.SetPaddingAll(0);
	m_PadZero.SetPaddingRow(0);
	m_PadZero.SetPaddingColumn(0);

	StyleInitialiseReset(&m_PadTiny);
	m_PadTiny.SetPaddingAll(PAD_TINY);
	m_PadTiny.SetPaddingRow(PAD_TINY);
	m_PadTiny.SetPaddingColumn( PAD_TINY);

	StyleInitialiseReset(&m_BackgroundColorPrimary);
	m_BackgroundColorPrimary.SetBackColor(m_PrimaryColor);
	m_BackgroundColorPrimary.SetTextColor(EG_ColorWhite());
	m_BackgroundColorPrimary.SetBackOPA(EG_OPA_COVER);

	StyleInitialiseReset(&m_BackgroundColorPrimaryMuted);
	m_BackgroundColorPrimaryMuted.SetBackColor(m_PrimaryColor);
	m_BackgroundColorPrimaryMuted.SetTextColor(m_PrimaryColor);
	m_BackgroundColorPrimaryMuted.SetBackOPA(EG_OPA_20);

	StyleInitialiseReset(&m_BackgroundColorSecondary);
	m_BackgroundColorSecondary.SetBackColor(m_SecondaryColor);
	m_BackgroundColorSecondary.SetTextColor(EG_ColorWhite());
	m_BackgroundColorSecondary.SetBackOPA(EG_OPA_COVER);

	StyleInitialiseReset(&m_BackgroundColorSecondaryMuted);
	m_BackgroundColorSecondaryMuted.SetBackColor(m_SecondaryColor);
	m_BackgroundColorSecondaryMuted.SetTextColor(m_SecondaryColor);
	m_BackgroundColorSecondaryMuted.SetBackOPA(EG_OPA_20);

	StyleInitialiseReset(&m_BackgroundColorGrey);
	m_BackgroundColorGrey.SetBackColor(m_GreyColor);
	m_BackgroundColorGrey.SetBackOPA(EG_OPA_COVER);
	m_BackgroundColorGrey.SetTextColor(m_TextColor);

	StyleInitialiseReset(&m_BackgroundColorWhite);
	m_BackgroundColorWhite.SetBackColor(m_CardColor);
	m_BackgroundColorWhite.SetBackOPA(EG_OPA_COVER);
	m_BackgroundColorWhite.SetTextColor(m_TextColor);

	StyleInitialiseReset(&m_Circle);
	m_Circle.SetRadius(EG_RADIUS_CIRCLE);

	StyleInitialiseReset(&m_NoRadius);
	m_NoRadius.SetRadius(0);

#if EG_THEME_DEFAULT_GROW
	StyleInitialiseReset(&m_Grow);
	m_Grow.SetTransformWidth(EG_DisplayDPX(pDisplay, 3));
	m_Grow.SetTransformHeight(EG_DisplayDPX(pDisplay, 3));
#endif

	StyleInitialiseReset(&m_Knob);
	m_Knob.SetBackColor(m_PrimaryColor);
	m_Knob.SetBackOPA(EG_OPA_COVER);
	m_Knob.SetPaddingAll(EG_DisplayDPX(pDisplay, 6));
	m_Knob.SetRadius(EG_RADIUS_CIRCLE);

	StyleInitialiseReset(&m_Animate);
	m_Animate.SetAnimateTime(200);

	StyleInitialiseReset(&m_AnimateFast);
	m_AnimateFast.SetAnimateTime(120);

#if EG_USE_ARC
	StyleInitialiseReset(&m_ArcIndicator);
	m_ArcIndicator.SetArcColor(m_GreyColor);
	m_ArcIndicator.SetArcWidth(EG_DisplayDPX(pDisplay, 15));
	m_ArcIndicator.SetArcRounded(true);

	StyleInitialiseReset(&m_ArcIndicatorPrimary);
	m_ArcIndicatorPrimary.SetArcColor(m_PrimaryColor);
#endif

#if EG_USE_DROPDOWN
	StyleInitialiseReset(&m_DropDownList);
	m_DropDownList.SetMaxHeight(EG_DPI_DEF * 2);
#endif
#if EG_USE_CHECKBOX
	StyleInitialiseReset(&m_CheckboxMarker);
	m_CheckboxMarker.SetPaddingAll(EG_DisplayDPX(pDisplay, 3));
	m_CheckboxMarker.SetBorderWidth(BORDER_WIDTH);
	m_CheckboxMarker.SetBorderColor(m_PrimaryColor);
	m_CheckboxMarker.SetBackColor(m_CardColor);
	m_CheckboxMarker.SetBackOPA(EG_OPA_COVER);
	m_CheckboxMarker.SetRadius(RADIUS_DEFAULT / 2);

	StyleInitialiseReset(&m_CheckboxMarkerChecked);
	m_CheckboxMarkerChecked.SetBackImageSource(EG_SYMBOL_OK);
	m_CheckboxMarkerChecked.SetTextColor(EG_ColorWhite());
	m_CheckboxMarkerChecked.SetTextFont(m_pFontSmall);
#endif

#if EG_USE_SWITCH
	StyleInitialiseReset(&m_SwitchKnob);
	m_SwitchKnob.SetPaddingAll(-EG_DisplayDPX(pDisplay, 4));
	m_SwitchKnob.SetBackColor(EG_ColorWhite());
#endif

#if EG_USE_LINE
	StyleInitialiseReset(&m_Line);
	m_Line.SetLineWidth(1);
	m_Line.SetLineColor(m_TextColor);
#endif

#if EG_USE_CHART
	StyleInitialiseReset(&m_ChartBackground);
	m_ChartBackground.SetBorderPost(false);
	m_ChartBackground.SetPaddingColumn(EG_DisplayDPX(pDisplay, 10));
	m_ChartBackground.SetLineColor(m_GreyColor);

	StyleInitialiseReset(&m_ChartSeries);
	m_ChartSeries.SetLineWidth(EG_DisplayDPX(pDisplay, 3));
	m_ChartSeries.SetRadius(EG_DisplayDPX(pDisplay, 3));
	m_ChartSeries.SetSize(EG_DisplayDPX(pDisplay, 8));
	m_ChartSeries.SetPaddingColumn(EG_DisplayDPX(pDisplay, 2));

	StyleInitialiseReset(&m_ChartIndicator);
	m_ChartIndicator.SetRadius(EG_RADIUS_CIRCLE);
	m_ChartIndicator.SetSize(EG_DisplayDPX(pDisplay, 8));
	m_ChartIndicator.SetBackColor(m_PrimaryColor);
	m_ChartIndicator.SetBackOPA(EG_OPA_COVER);

	StyleInitialiseReset(&m_ChartTicks);
	m_ChartTicks.SetLineWidth(EG_DisplayDPX(pDisplay, 1));
	m_ChartTicks.SetLineColor(m_TextColor);
	m_ChartTicks.SetPaddingAll(EG_DisplayDPX(pDisplay, 2));
	m_ChartTicks.SetTextColor(EG_MainPalette(EG_PALETTE_GREY));
#endif

#if EG_USE_MENU
	StyleInitialiseReset(&m_MenuBackground);
	m_MenuBackground.SetPaddingAll(0);
	m_MenuBackground.SetPaddingGap(0);
	m_MenuBackground.SetRadius(0);
	m_MenuBackground.SetClipCorner(true);
	m_MenuBackground.SetBorderSide(EG_BORDER_SIDE_NONE);

	StyleInitialiseReset(&m_MenuSection);
	m_MenuSection.SetRadius(RADIUS_DEFAULT);
	m_MenuSection.SetClipCorner(true);
	m_MenuSection.SetBackOPA(EG_OPA_COVER);
	m_MenuSection.SetBackColor(m_CardColor);
	m_MenuSection.SetTextColor(m_TextColor);

	StyleInitialiseReset(&m_MenuContainer);
	m_MenuContainer.SetHorizontalPadding(PAD_SMALL);
	m_MenuContainer.SetVerticalPadding(PAD_SMALL);
	m_MenuContainer.SetPaddingGap(PAD_SMALL);
	m_MenuContainer.SetBorderWidth(EG_DisplayDPX(pDisplay, 1));
	m_MenuContainer.SetBorderOPA(EG_OPA_10);
	m_MenuContainer.SetBorderColor(m_TextColor);
	m_MenuContainer.SetBorderSide(EG_BORDER_SIDE_NONE);

	StyleInitialiseReset(&m_MenuSidebarContainer);
	m_MenuSidebarContainer.SetPaddingAll(0);
	m_MenuSidebarContainer.SetPaddingGap(0);
	m_MenuSidebarContainer.SetBorderWidth(EG_DisplayDPX(pDisplay, 1));
	m_MenuSidebarContainer.SetBorderOPA(EG_OPA_10);
	m_MenuSidebarContainer.SetBorderColor(m_TextColor);
	m_MenuSidebarContainer.SetBorderSide(EG_BORDER_SIDE_RIGHT);

	StyleInitialiseReset(&m_MenuMainContainer);
	m_MenuMainContainer.SetPaddingAll(0);
	m_MenuMainContainer.SetPaddingGap(0);

	StyleInitialiseReset(&m_MenuHeaderContainer);
	m_MenuHeaderContainer.SetHorizontalPadding(PAD_SMALL);
	m_MenuHeaderContainer.SetVerticalPadding(PAD_TINY);
	m_MenuHeaderContainer.SetPaddingGap(PAD_SMALL);

	StyleInitialiseReset(&m_MenuHeaderButton);
	m_MenuHeaderButton.SetHorizontalPadding(PAD_TINY);
	m_MenuHeaderButton.SetVerticalPadding(PAD_TINY);
	m_MenuHeaderButton.SetShadowOPA(EG_OPA_TRANSP);
	m_MenuHeaderButton.SetBackOPA(EG_OPA_TRANSP);
	m_MenuHeaderButton.SetTextColor(m_TextColor);

	StyleInitialiseReset(&m_MenuPage);
	m_MenuPage.SetHorizontalPadding(0);
	m_MenuPage.SetPaddingGap(0);

	StyleInitialiseReset(&m_MenuPressed);
	m_MenuPressed.SetBackOPA(EG_OPA_20);
	m_MenuPressed.SetBackColor(EG_MainPalette(EG_PALETTE_GREY));

	StyleInitialiseReset(&m_MenuSeparator);
	m_MenuSeparator.SetBackOPA(EG_OPA_TRANSP);
	m_MenuSeparator.SetVerticalPadding(PAD_TINY);
#endif

#if EG_USE_METER
	StyleInitialiseReset(&m_MeterMarker);
	m_MeterMarker.SetLineWidth(EG_DisplayDPX(pDisplay, 5));
	m_MeterMarker.SetLineColor(m_TextColor);
	m_MeterMarker.SetSize(EG_DisplayDPX(pDisplay, 20));
	m_MeterMarker.SetPaddingLeft(EG_DisplayDPX(pDisplay, 15));

	StyleInitialiseReset(&m_MeterIndicator);
	m_MeterIndicator.SetRadius(EG_RADIUS_CIRCLE);
	m_MeterIndicator.SetBackColor(m_TextColor);
	m_MeterIndicator.SetBackOPA(EG_OPA_COVER);
	m_MeterIndicator.SetSize(EG_DisplayDPX(pDisplay, 15));
#endif

#if EG_USE_TABLE
	StyleInitialiseReset(&m_TableCell);
	m_TableCell.SetBorderWidth(EG_DisplayDPX(pDisplay, 1));
	m_TableCell.SetBorderColor(m_GreyColor);
	m_TableCell.SetBorderSide((EG_BorderSide_t)(EG_BORDER_SIDE_TOP | EG_BORDER_SIDE_BOTTOM));
#endif

#if EG_USE_TEXTAREA
	StyleInitialiseReset(&m_TextAreaCursor);
	m_TextAreaCursor.SetBorderColor(m_TextColor);
	m_TextAreaCursor.SetBorderWidth(EG_DisplayDPX(pDisplay, 2));
	m_TextAreaCursor.SetPaddingLeft(-EG_DisplayDPX(pDisplay, 1));
	m_TextAreaCursor.SetBorderSide(EG_BORDER_SIDE_LEFT);
	m_TextAreaCursor.SetAnimateTime(400);

	StyleInitialiseReset(&m_TextAreaPlaceholder);
	m_TextAreaPlaceholder.SetTextColor((m_Flags & MODE_DARK) ? EG_DarkPalette(EG_PALETTE_GREY, 2) : EG_LightPalette(EG_PALETTE_GREY, 1));
#endif

#if EG_USE_CALENDAR
	StyleInitialiseReset(&m_CalendarButtonBackground);
	m_CalendarButtonBackground.SetPaddingAll(PAD_SMALL);
	m_CalendarButtonBackground.SetPaddingGap(PAD_SMALL / 2);

	StyleInitialiseReset(&m_CalendarButtonDay);
	m_CalendarButtonDay.SetBorderWidth(EG_DisplayDPX(pDisplay, 1));
	m_CalendarButtonDay.SetBorderColor(m_GreyColor);
	m_CalendarButtonDay.SetBackColor(m_CardColor);
	m_CalendarButtonDay.SetBackOPA(EG_OPA_20);

	StyleInitialiseReset(&m_CalendarHeader);
	m_CalendarHeader.SetHorizontalPadding(PAD_SMALL);
	m_CalendarHeader.SetPaddingTop(PAD_SMALL);
	m_CalendarHeader.SetPaddingBottom(PAD_TINY);
	m_CalendarHeader.SetPaddingGap( PAD_SMALL);
#endif

#if EG_USE_COLORWHEEL
	StyleInitialiseReset(&m_ColorwheelMain);
	m_ColorwheelMain.SetArcWidth(EG_DisplayDPX(pDisplay, 10));
#endif

#if EG_USE_MSGBOX
	// To add space for for the button shadow
	StyleInitialiseReset(&m_MessageBoxButtonBackground);
	m_MessageBoxButtonBackground.SetPaddingAll(EG_DisplayDPX(pDisplay, 4));

	StyleInitialiseReset(&m_MessageBoxBackground);
	m_MessageBoxBackground.SetMaxWidth(EG_PCT(100));

	StyleInitialiseReset(&m_MessageBoxBackdropBackground);
	m_MessageBoxBackdropBackground.SetBackColor(EG_MainPalette(EG_PALETTE_GREY));
	m_MessageBoxBackdropBackground.SetBackOPA(EG_OPA_50);
#endif
#if EG_USE_KEYBOARD
	StyleInitialiseReset(&m_KeyboardButtonBackground);
	m_KeyboardButtonBackground.SetShadowWidth(0);
	m_KeyboardButtonBackground.SetRadius(m_DisplaySize == DISP_SMALL ? RADIUS_DEFAULT / 2 : RADIUS_DEFAULT);
#endif

#if EG_USE_TABVIEW
	StyleInitialiseReset(&m_TabButton);
	m_TabButton.SetBorderColor(m_PrimaryColor);
	m_TabButton.SetBorderWidth(BORDER_WIDTH * 2);
	m_TabButton.SetBorderSide(EG_BORDER_SIDE_BOTTOM);

	StyleInitialiseReset(&m_TabBackgroundFocus);
	m_TabBackgroundFocus.SetOutlinePad(-BORDER_WIDTH);
#endif

#if EG_USE_LIST
	StyleInitialiseReset(&m_ListBackground);
	m_ListBackground.SetHorizontalPadding(PAD_DEF);
	m_ListBackground.SetVerticalPadding(0);
	m_ListBackground.SetPaddingGap(0);
	m_ListBackground.SetClipCorner(true);

	StyleInitialiseReset(&m_ListButton);
	m_ListButton.SetBorderWidth(EG_DisplayDPX(pDisplay, 1));
	m_ListButton.SetBorderColor(m_GreyColor);
	m_ListButton.SetBorderSide(EG_BORDER_SIDE_BOTTOM);
	m_ListButton.SetPaddingAll(PAD_SMALL);
	m_ListButton.SetPaddingColumn(PAD_SMALL);

	StyleInitialiseReset(&m_ListItemGrow);
	m_ListItemGrow.SetTransformWidth(PAD_DEF);
#endif

#if EG_USE_LED
	StyleInitialiseReset(&m_Led);
	m_Led.SetBackOPA(EG_OPA_COVER);
	m_Led.SetBackColor(EG_ColorWhite());
	m_Led.SetBackGradientColor(EG_MainPalette(EG_PALETTE_GREY));
	m_Led.SetRadius(EG_RADIUS_CIRCLE);
	m_Led.SetShadowWidth(EG_DisplayDPX(pDisplay, 15));
	m_Led.SetShadowColor(EG_ColorWhite());
	m_Led.SetShadowSpread(EG_DisplayDPX(pDisplay, 5));
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDefTheme::ApplyTheme(EGObject *pObj)
{
	EGObject *pParent = pObj->GetParent();
	if(pParent == nullptr) {    // for screens only
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
		return;
	}
	if(EGObject::IsKindOf(pObj, &c_ObjectClass)) { 

#if EG_USE_TABVIEW
		// Tabview content area
		if(EGObject::IsKindOf(pParent, &c_TabViewClass)) return;
		else if(EGObject::IsKindOf(pParent->GetParent(), &c_TabViewClass)) {		// Tabview pages
			pObj->AddStyle(&m_PadNormal, 0);
			pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
			pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
			return;
    }
#endif

#if EG_USE_WIN
		// Header
		if(pObj->GetIndex() == 0 && EGObject::IsKindOf(pParent, &c_WindowClass)) {
			pObj->AddStyle(&m_BackgroundColorGrey, 0);
			pObj->AddStyle(&m_PadTiny, 0);
			return;
		}
		// Content
		else if(pObj->GetIndex() == 1 && EGObject::IsKindOf(pParent, &c_WindowClass)) {
			pObj->AddStyle(&m_Screen, 0);
			pObj->AddStyle(&m_PadNormal, 0);
			pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
			pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
			return;
		}
#endif

#if EG_USE_CALENDAR
		if(EGObject::IsKindOf(pParent, &c_CalendarClass)) {
			// No style
			return;
		}
#endif

		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
	}
#if EG_USE_BTN
	else if(EGObject::IsKindOf(pObj, &c_ButtonClass)) {
		pObj->AddStyle(&m_Button, 0);
    pObj->AddStyle(&m_BackgroundColorPrimary, 0);
		pObj->AddStyle(&m_TransitionDelayed, 0);
		pObj->AddStyle(&m_Pressed, EG_STATE_PRESSED);
		pObj->AddStyle(&m_TransitionNormal, EG_STATE_PRESSED);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
#if EG_THEME_DEFAULT_GROW
		pObj->AddStyle(&m_Grow, EG_STATE_PRESSED);
#endif
		pObj->AddStyle(&m_BackgroundColorSecondary, EG_STATE_CHECKED);
		pObj->AddStyle(&m_Disabled, EG_STATE_DISABLED);

#if EG_USE_MENU
		if(EGObject::IsKindOf(pParent, &c_MenuSidebarHeaderContainerClass) ||
			  EGObject::IsKindOf(pParent, &c_MenuMainHeaderContainerClass)) {
			pObj->AddStyle(&m_MenuHeaderButton, 0);
			pObj->AddStyle(&m_MenuPressed, EG_STATE_PRESSED);
		}
#endif
	}
#endif

#if EG_USE_LINE
	else if(EGObject::IsKindOf(pObj, &c_LineClass)) {
		pObj->AddStyle(&m_Line, 0);
	}
#endif

#if EG_USE_BTNMATRIX
	else if(EGObject::IsKindOf(pObj, &c_ButtonMatrixClass)) {
#if EG_USE_MSGBOX
		if(EGObject::IsKindOf(pParent, &c_MsgBoxClass)) {
			pObj->AddStyle(&m_MessageBoxButtonBackground, 0);
			pObj->AddStyle(&m_PadGap, 0);
			pObj->AddStyle(&m_Button, EG_PART_ITEMS);
			pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
			pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
			pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_ITEMS | EG_STATE_CHECKED);
			pObj->AddStyle(&m_BackgroundColorPrimaryMuted, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_BackgroundColorSecondaryMuted, EG_PART_ITEMS | EG_STATE_EDITED);
			return;
		}
#endif
#if EG_USE_TABVIEW
		if(EGObject::IsKindOf(pParent, &c_TabViewClass)) {
			pObj->AddStyle(&m_BackgroundColorWhite, 0);
			pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_TabBackgroundFocus, EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
			pObj->AddStyle(&m_BackgroundColorPrimaryMuted, EG_PART_ITEMS | EG_STATE_CHECKED);
			pObj->AddStyle(&m_TabButton, EG_PART_ITEMS | EG_STATE_CHECKED);
			pObj->AddStyle(&m_OutlinePrimary, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_OutlineSecondary, EG_PART_ITEMS | EG_STATE_EDITED);
			pObj->AddStyle(&m_TabBackgroundFocus, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			return;
		}
#endif

#if EG_USE_CALENDAR
		if(EGObject::IsKindOf(pParent, &c_CalendarClass)) {
			pObj->AddStyle(&m_CalendarButtonBackground, 0);
			pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
			pObj->AddStyle(&m_CalendarButtonDay, EG_PART_ITEMS);
			pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
			pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
			pObj->AddStyle(&m_OutlinePrimary, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
			pObj->AddStyle(&m_OutlineSecondary, EG_PART_ITEMS | EG_STATE_EDITED);
			return;
		}
#endif
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_Button, EG_PART_ITEMS);
		pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_ITEMS | EG_STATE_CHECKED);
		pObj->AddStyle(&m_OutlinePrimary, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_PART_ITEMS | EG_STATE_EDITED);
	}
#endif

#if EG_USE_BAR
	else if(EGObject::IsKindOf(pObj, &c_BarClass)) {
		pObj->AddStyle(&m_BackgroundColorPrimaryMuted, 0);
		pObj->AddStyle(&m_Circle, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Circle, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_SLIDER
	else if(EGObject::IsKindOf(pObj, &c_SliderClass)) {
		pObj->AddStyle(&m_BackgroundColorPrimaryMuted, 0);
		pObj->AddStyle(&m_Circle, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Circle, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Knob, EG_PART_KNOB);
#if EG_THEME_DEFAULT_GROW
		pObj->AddStyle(&m_Grow, EG_PART_KNOB | EG_STATE_PRESSED);
#endif
		pObj->AddStyle(&m_TransitionDelayed, EG_PART_KNOB);
		pObj->AddStyle(&m_TransitionNormal, EG_PART_KNOB | EG_STATE_PRESSED);
	}
#endif

#if EG_USE_TABLE
	else if(EGObject::IsKindOf(pObj, &c_TableClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadZero, 0);
		pObj->AddStyle(&m_NoRadius, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
		pObj->AddStyle(&m_BackgroundColorWhite, EG_PART_ITEMS);
		pObj->AddStyle(&m_TableCell, EG_PART_ITEMS);
		pObj->AddStyle(&m_PadNormal, EG_PART_ITEMS);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_BackgroundColorSecondary, EG_PART_ITEMS | EG_STATE_EDITED);
	}
#endif

#if EG_USE_CHECKBOX
	else if(EGObject::IsKindOf(pObj, &c_CheckboxClass)) {
		pObj->AddStyle(&m_PadGap, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_Disabled, EG_PART_INDICATOR | EG_STATE_DISABLED);
		pObj->AddStyle(&m_CheckboxMarker, EG_PART_INDICATOR);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_INDICATOR | EG_STATE_CHECKED);
		pObj->AddStyle(&m_CheckboxMarkerChecked, EG_PART_INDICATOR | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Pressed, EG_PART_INDICATOR | EG_STATE_PRESSED);
#if EG_THEME_DEFAULT_GROW
		pObj->AddStyle(&m_Grow, EG_PART_INDICATOR | EG_STATE_PRESSED);
#endif
		pObj->AddStyle(&m_TransitionNormal, EG_PART_INDICATOR | EG_STATE_PRESSED);
		pObj->AddStyle(&m_TransitionDelayed, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_SWITCH
	else if(EGObject::IsKindOf(pObj, &c_SwitchClass)) {
		pObj->AddStyle(&m_BackgroundColorGrey, 0);
		pObj->AddStyle(&m_Circle, 0);
		pObj->AddStyle(&m_AnimateFast, 0);
		pObj->AddStyle(&m_Disabled, EG_STATE_DISABLED);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_INDICATOR | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Circle, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Disabled, EG_PART_INDICATOR | EG_STATE_DISABLED);
		pObj->AddStyle(&m_Knob, EG_PART_KNOB);
		pObj->AddStyle(&m_BackgroundColorWhite, EG_PART_KNOB);
		pObj->AddStyle(&m_SwitchKnob, EG_PART_KNOB);
		pObj->AddStyle(&m_Disabled, EG_PART_KNOB | EG_STATE_DISABLED);

		pObj->AddStyle(&m_TransitionNormal, EG_PART_INDICATOR | EG_STATE_CHECKED);
		pObj->AddStyle(&m_TransitionNormal, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_CHART
	else if(EGObject::IsKindOf(pObj, &c_ChartClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadSmall, 0);
		pObj->AddStyle(&m_ChartBackground, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
		pObj->AddStyle(&m_ChartSeries, EG_PART_ITEMS);
		pObj->AddStyle(&m_ChartIndicator, EG_PART_INDICATOR);
		pObj->AddStyle(&m_ChartTicks, EG_PART_TICKS);
		pObj->AddStyle(&m_ChartSeries, EG_PART_CURSOR);
	}
#endif

#if EG_USE_ROLLER
	else if(EGObject::IsKindOf(pObj, &c_RollerClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Animate, 0);
		pObj->AddStyle(&m_LineSpaceLarge, 0);
		pObj->AddStyle(&m_TextAlignCenter, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_SELECTED);
	}
#endif

#if EG_USE_DROPDOWN
	else if(EGObject::IsKindOf(pObj, &c_DropDownClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadSmall, 0);
		pObj->AddStyle(&m_TransitionDelayed, 0);
		pObj->AddStyle(&m_TransitionNormal, EG_STATE_PRESSED);
		pObj->AddStyle(&m_Pressed, EG_STATE_PRESSED);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_TransitionNormal, EG_PART_INDICATOR);
	}
	else if(EGObject::IsKindOf(pObj, &c_DropDownListClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_ClipCorner, 0);
		pObj->AddStyle(&m_LineSpaceLarge, 0);
		pObj->AddStyle(&m_DropDownList, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
		pObj->AddStyle(&m_BackgroundColorWhite, EG_PART_SELECTED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_SELECTED | EG_STATE_CHECKED);
		pObj->AddStyle(&m_Pressed, EG_PART_SELECTED | EG_STATE_PRESSED);
	}
#endif

#if EG_USE_ARC
	else if(EGObject::IsKindOf(pObj, &c_ArcClass)) {
		pObj->AddStyle(&m_ArcIndicator, 0);
		pObj->AddStyle(&m_ArcIndicator, EG_PART_INDICATOR);
		pObj->AddStyle(&m_ArcIndicatorPrimary, EG_PART_INDICATOR);
		pObj->AddStyle(&m_Knob, EG_PART_KNOB);
	}
#endif

#if EG_USE_SPINNER
	else if(EGObject::IsKindOf(pObj, &c_SpinnerClass)) {
		pObj->AddStyle(&m_ArcIndicator, 0);
		pObj->AddStyle(&m_ArcIndicator, EG_PART_INDICATOR);
		pObj->AddStyle(&m_ArcIndicatorPrimary, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_METER
	else if(EGObject::IsKindOf(pObj, &c_MeterClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_Circle, 0);
		pObj->AddStyle(&m_MeterIndicator, EG_PART_INDICATOR);
	}
#endif

#if EG_USE_TEXTAREA
	else if(EGObject::IsKindOf(pObj, &c_EditClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadSmall, 0);
		pObj->AddStyle(&m_Disabled, EG_STATE_DISABLED);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
		pObj->AddStyle(&m_TextAreaCursor, EG_PART_CURSOR | EG_STATE_FOCUSED);
		pObj->AddStyle(&m_TextAreaPlaceholder, EG_PART_TEXTAREA_PLACEHOLDER);
	}
#endif

#if EG_USE_CALENDAR
	else if(EGObject::IsKindOf(pObj, &c_CalendarClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadZero, 0);
	}
#endif

#if EG_USE_CALENDAR_MONTH_HEADER
	else if(EGObject::IsKindOf(pObj, &c_CalendarMonthHeaderClass)) {
		pObj->AddStyle(&m_CalendarHeader, 0);
	}
#endif

#if EG_USE_CALENDAR_DROPDOWN_HEADER
	else if(EGObject::IsKindOf(pObj, &c_CalendarDropDownHeaderClass)) {
		pObj->AddStyle(&m_CalendarHeader, 0);
	}
#endif

#if EG_USE_KEYBOARD
	else if(EGObject::IsKindOf(pObj, &c_KeyboardClass)) {
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(m_DisplaySize == DISP_LARGE ? &m_PadSmall : &m_PadTiny, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_Button, EG_PART_ITEMS);
		pObj->AddStyle(&m_Disabled, EG_PART_ITEMS | EG_STATE_DISABLED);
		pObj->AddStyle(&m_BackgroundColorWhite, EG_PART_ITEMS);
		pObj->AddStyle(&m_KeyboardButtonBackground, EG_PART_ITEMS);
		pObj->AddStyle(&m_Pressed, EG_PART_ITEMS | EG_STATE_PRESSED);
		pObj->AddStyle(&m_BackgroundColorGrey, EG_PART_ITEMS | EG_STATE_CHECKED);
		pObj->AddStyle(&m_BackgroundColorPrimaryMuted, EG_PART_ITEMS | EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_BackgroundColorSecondaryMuted, EG_PART_ITEMS | EG_STATE_EDITED);
	}
#endif
#if EG_USE_LIST
	else if(EGObject::IsKindOf(pObj, &c_ListClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_ListBackground, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
		return;
	}
	else if(EGObject::IsKindOf(pObj, &c_ListTextClass)) {
		pObj->AddStyle(&m_BackgroundColorGrey, 0);
		pObj->AddStyle(&m_ListItemGrow, 0);
	}
	else if(EGObject::IsKindOf(pObj, &c_ListButtonClass)) {
		pObj->AddStyle(&m_BackgroundColorWhite, 0);
		pObj->AddStyle(&m_ListButton, 0);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_ListItemGrow, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_ListItemGrow, EG_STATE_PRESSED);
		pObj->AddStyle(&m_Pressed, EG_STATE_PRESSED);
	}
#endif
#if EG_USE_MENU
	else if(EGObject::IsKindOf(pObj, &c_MenuClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_MenuBackground, 0);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuSidebarContainerClass)) {
		pObj->AddStyle(&m_MenuSidebarContainer, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuMainContainerClass)) {
		pObj->AddStyle(&m_MenuMainContainer, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuContainerClass)) {
		pObj->AddStyle(&m_MenuContainer, 0);
		pObj->AddStyle(&m_MenuPressed, EG_STATE_PRESSED);
		pObj->AddStyle(&m_BackgroundColorPrimaryMuted, EG_STATE_PRESSED | EG_STATE_CHECKED);
		pObj->AddStyle(&m_BackgroundColorPrimaryMuted, EG_STATE_CHECKED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_STATE_FOCUS_KEY);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuSidebarHeaderContainerClass) ||
					EGObject::IsKindOf(pObj, &c_MenuMainHeaderContainerClass)) {
		pObj->AddStyle(&m_MenuHeaderContainer, 0);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuPageClass)) {
		pObj->AddStyle(&m_MenuPage, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuSectionClass)) {
		pObj->AddStyle(&m_MenuSection, 0);
	}
	else if(EGObject::IsKindOf(pObj, &c_MenuSeparatorClass)) {
		pObj->AddStyle(&m_MenuSeparator, 0);
	}
#endif
#if EG_USE_MSGBOX
	else if(EGObject::IsKindOf(pObj, &c_MsgBoxClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_MessageBoxBackground, 0);
		return;
	}
	else if(EGObject::IsKindOf(pObj, &c_MsgBoxBackdropClass)) {
		pObj->AddStyle(&m_MessageBoxBackdropBackground, 0);
	}
#endif
#if EG_USE_SPINBOX
	else if(EGObject::IsKindOf(pObj, &c_SpinboxClass)) {
		pObj->AddStyle(&m_Card, 0);
		pObj->AddStyle(&m_PadSmall, 0);
		pObj->AddStyle(&m_OutlinePrimary, EG_STATE_FOCUS_KEY);
		pObj->AddStyle(&m_OutlineSecondary, EG_STATE_EDITED);
		pObj->AddStyle(&m_BackgroundColorPrimary, EG_PART_CURSOR);
	}
#endif
#if EG_USE_TILEVIEW
	else if(EGObject::IsKindOf(pObj, &c_TabViewClass)) {
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
	}
	else if(EGObject::IsKindOf(pObj, &c_TileViewTitleClass)) {
		pObj->AddStyle(&m_Scrollbar, EG_PART_SCROLLBAR);
		pObj->AddStyle(&m_ScrollbarScrolled, EG_PART_SCROLLBAR | EG_STATE_SCROLLED);
	}
#endif

#if EG_USE_TABVIEW
	else if(EGObject::IsKindOf(pObj, &c_TabViewClass)) {
		pObj->AddStyle(&m_Screen, 0);
		pObj->AddStyle(&m_PadZero, 0);
	}
#endif

#if EG_USE_WIN
	else if(EGObject::IsKindOf(pObj, &c_WindowClass)) {
		pObj->AddStyle(&m_ClipCorner, 0);
	}
#endif

#if EG_USE_COLORWHEEL
	else if(EGObject::IsKindOf(pObj, &c_ColorWheelClass)) {
		pObj->AddStyle(&m_ColorwheelMain, 0);
		pObj->AddStyle(&m_PadNormal, 0);
		pObj->AddStyle(&m_BackgroundColorWhite, EG_PART_KNOB);
		pObj->AddStyle(&m_PadNormal, EG_PART_KNOB);
	}
#endif

#if EG_USE_LED
	else if(EGObject::IsKindOf(pObj, &c_LedClass)) {
		pObj->AddStyle(&m_Led, 0);
	}
#endif
}

#endif
