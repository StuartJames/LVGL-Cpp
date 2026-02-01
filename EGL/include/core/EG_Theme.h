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

#include "EG_Object.h"

///////////////////////////////////////////////////////////////////////////////

class EGDisplay;

///////////////////////////////////////////////////////////////////////////////

class EGTheme
{
public:
                            EGTheme();
  virtual void              ApplyTheme(EGObject *pObj) = 0;
  virtual bool              IsInitialised(void);

  static void               ThemeApply(EGObject *pObj); // Apply the active theme on an object
  static EGTheme*           GetFromObj(EGObject *pObj); // Get the theme assigned to the display of the object
  static const EG_Font_t*   GetFontSmall(EGObject *pObj); // Get the small font of the theme
  static const EG_Font_t*   GetFontNormal(EGObject *pObj); // Get the normal font of the theme
  static const EG_Font_t*   GetFontLarge(EGObject *pObj); // Get the subtitle font of the theme
  static EG_Color_t         GetColorPrimary(EGObject *pObj); // Get the primary color of the theme
  static EG_Color_t         GetColorSecondary(EGObject *pObj); // Get the secondary color of the theme


  EGDisplay                 *m_pDisplay;
  EG_Color_t                 m_PrimaryColor;
  EG_Color_t                 m_SecondaryColor;
  const EG_Font_t           *m_pFontSmall;
  const EG_Font_t           *m_pFontNormal;
  const EG_Font_t           *m_pFontLarge;
  uint32_t                   m_Flags;                 // Any custom flag used by the theme

protected:
  void                      StyleInitialiseReset(EGStyle *pStyle);

  void                      *m_pUserData;
  bool                       m_Initialised;

};