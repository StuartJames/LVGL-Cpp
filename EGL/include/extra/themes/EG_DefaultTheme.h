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

#pragma once

#include "core/EG_Object.h"
#include "../../core/EG_Theme.h"

#if EG_USE_THEME_DEFAULT

//////////////////////////////////////////////////////////////////////////////////////

class EGDefTheme : public EGTheme
{
public:
                      EGDefTheme(void);
                      ~EGDefTheme(void);
  void                Initialise(EGDisplay *pDisplay, EG_Color_t color_primary, EG_Color_t color_secondary, bool dark, const EG_Font_t * font);
  virtual void        ApplyTheme(EGObject *pObj);

  static EGTheme*     SetTheme(EG_Color_t PrimaryColor, EG_Color_t SecondaryColor, bool Dark, const EG_Font_t *pFont, EGDisplay *pDisplay = nullptr);
  static bool         IsCreated(void);
  static EGTheme*     GetTheme(void);

private:
  void                InitialiseStyles(EGDisplay *pDisplay);
  static EG_Color_t   DarkColorFilterCB(const EG_ColorFilterProps_t *pFilter, EG_Color_t Color, EG_OPA_t OPA);
  static EG_Color_t   GreyFilterCB(const EG_ColorFilterProps_t *pFilter, EG_Color_t Color, EG_OPA_t OPA);

  static EGDefTheme   *m_pTheme;

  typedef enum {
    DISP_LARGE = 1,
    DISP_MEDIUM,
    DISP_SMALL
  } DisplaySize_t;

  DisplaySize_t       m_DisplaySize;
  EG_Color_t          m_ScreenColor;
  EG_Color_t          m_TextColor;
  EG_Color_t          m_CardColor;
  EG_Color_t          m_GreyColor;

  EGStyle             m_Screen;
	EGStyle             m_Scrollbar;
	EGStyle             m_ScrollbarScrolled;
	EGStyle             m_Card;
	EGStyle             m_Button;

	// Utility
	EGStyle             m_BackgroundColorPrimary;
	EGStyle             m_BackgroundColorPrimaryMuted;
	EGStyle             m_BackgroundColorSecondary;
	EGStyle             m_BackgroundColorSecondaryMuted;
	EGStyle             m_BackgroundColorGrey;
	EGStyle             m_BackgroundColorWhite;
	EGStyle             m_Pressed;
	EGStyle             m_Disabled;
	EGStyle             m_PadZero;
	EGStyle             m_PadTiny;
	EGStyle             m_PadSmall;
	EGStyle             m_PadNormal;
	EGStyle             m_PadGap;
	EGStyle             m_LineSpaceLarge;
	EGStyle             m_TextAlignCenter;
	EGStyle             m_OutlinePrimary;
	EGStyle             m_OutlineSecondary;
	EGStyle             m_Circle;
	EGStyle             m_NoRadius;
	EGStyle             m_ClipCorner;
#if EG_THEME_DEFAULT_GROW
	EGStyle             m_Grow;
#endif
	EGStyle             m_TransitionDelayed;
	EGStyle             m_TransitionNormal;
	EGStyle             m_Animate;
	EGStyle             m_AnimateFast;
	// Parts
	EGStyle             m_Knob;
	EGStyle             m_Indicator;
#if EG_USE_ARC
	EGStyle             m_ArcIndicator;
	EGStyle             m_ArcIndicatorPrimary;
#endif
#if EG_USE_CHART
	EGStyle             m_ChartSeries;
  EGStyle             m_ChartIndicator;
  EGStyle             m_ChartTicks;
  EGStyle             m_ChartBackground;
#endif
#if EG_USE_DROPDOWN
	EGStyle             m_DropDownList;
#endif
#if EG_USE_CHECKBOX
	EGStyle             m_CheckboxMarker;
  EGStyle             m_CheckboxMarkerChecked;
#endif
#if EG_USE_SWITCH
	EGStyle             m_SwitchKnob;
#endif
#if EG_USE_LINE
	EGStyle             m_Line;
#endif
#if EG_USE_TABLE
	EGStyle             m_TableCell;
#endif
#if EG_USE_METER
	EGStyle             m_MeterMarker;
  EGStyle             m_MeterIndicator;
#endif
#if EG_USE_TEXTAREA
	EGStyle             m_TextAreaCursor;
  EGStyle             m_TextAreaPlaceholder;
#endif
#if EG_USE_CALENDAR
	EGStyle             m_CalendarButtonBackground;
  EGStyle             m_CalendarButtonDay;
  EGStyle             m_CalendarHeader;
#endif
#if EG_USE_COLORWHEEL
	EGStyle             m_ColorwheelMain;
#endif
#if EG_USE_MENU
	EGStyle             m_MenuBackground;
  EGStyle             m_MenuContainer;
  EGStyle             m_MenuSidebarContainer;
  EGStyle             m_MenuMainContainer;
  EGStyle             m_MenuPage;
  EGStyle             m_MenuHeaderContainer;
  EGStyle             m_MenuHeaderButton;
	EGStyle	            m_MenuSection;
  EGStyle             m_MenuPressed;
  EGStyle             m_MenuSeparator;
#endif
#if EG_USE_MSGBOX
	EGStyle             m_MessageBoxBackground;
  EGStyle             m_MessageBoxButtonBackground;
  EGStyle             m_MessageBoxBackdropBackground;
#endif
#if EG_USE_KEYBOARD
	EGStyle             m_KeyboardButtonBackground;
#endif
#if EG_USE_LIST
	EGStyle             m_ListBackground;
  EGStyle             m_ListButton;
  EGStyle             m_ListItemGrow;
  EGStyle             m_ListLabel;
#endif
#if EG_USE_TABVIEW
	EGStyle             m_TabBackgroundFocus;
  EGStyle             m_TabButton;
#endif
#if EG_USE_LED
	EGStyle             m_Led;
#endif

  static EG_StyleTransitionDiscriptor_t m_TransDelayed;
  static EG_StyleTransitionDiscriptor_t m_TransNormal;
	static EG_ColorFilterProps_t     m_DarkFilter;
	static EG_ColorFilterProps_t     m_GreyFilter;
};

#endif

