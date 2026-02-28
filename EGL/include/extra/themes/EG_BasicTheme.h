
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

////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_THEME_BASIC

////////////////////////////////////////////////////////////////////////////////////

class EGBasicTheme : public EGTheme
{
public:
                        EGBasicTheme(void);
  void                  Initialise(EGDisplay *pDisplay);
  virtual void          ApplyTheme(EGObject *pObj);
  
  static EGTheme*       SetTheme(EGDisplay *pDisplay = nullptr);
  static bool           IsCreated(void);
  static EGTheme*       GetTheme(void);
  
private:  
  void                  InitialiseStyles(void);
  
  static EGBasicTheme   *m_pTheme;
  
	EGStyle               m_Screen;
	EGStyle               m_Transparent;
	EGStyle               m_White;
	EGStyle               m_Light;
	EGStyle               m_Dark;
	EGStyle               m_Dim;
	EGStyle               m_Scrollbar;
#if EG_USE_ARC || EG_USE_COLORWHEEL
	EGStyle               m_ArcLine;
	EGStyle               m_ArcKnob;
#endif
#if EG_USE_TEXTAREA
	EGStyle               m_TextCursor;
#endif

	static EG_ColorFilterProps_t     m_GreyFilter;

};

#endif
