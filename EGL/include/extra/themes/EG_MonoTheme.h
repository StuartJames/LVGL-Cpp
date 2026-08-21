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

#pragma once

#include "core/EG_Object.h"

//////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_THEME_MONO


class EGMonoTheme : public EGTheme
{
public:
                      EGMonoTheme(void) : EGTheme(){};
  void                Initialise(EGDisplay *pDisplay, bool Dark, const EG_Font_t *pFont);
  virtual void        ApplyTheme(EGObject *pObj);

  static EGTheme*     SetTheme(bool Dark, const EG_Font_t *pFont, EGDisplay *pDisplay = nullptr);
  static bool         IsCreated(void);
  static EGTheme*     GetTheme(void);

private:
  void                InitialiseStyles(bool Dark, const EG_Font_t *pFont);

  static EGMonoTheme  *m_pTheme;

  EGStyle             m_Screen;
	EGStyle             m_Card;
	EGStyle             m_Scrollbar;
	EGStyle             m_Button;
	EGStyle             m_Pressed;
	EGStyle             m_Inverted;
	EGStyle             m_Disabled;
	EGStyle             m_Focus;
	EGStyle             m_Edit;
	EGStyle             m_PadGap;
	EGStyle             m_PadZero;
	EGStyle             m_NoRadius;
	EGStyle             m_RadiusCircle;
	EGStyle             m_LargeBorder;
	EGStyle             m_LargeLineSpace;
	EGStyle             m_Underline;
#if EG_USE_TEXTAREA
	EGStyle             m_TextCursor;
#endif
};

#endif
