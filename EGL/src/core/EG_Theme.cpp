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

#include "EGL.h"
#include "core/EG_Object.h"
#include "core/EG_Theme.h"

EGTheme::EGTheme(void) :
  m_pDisplay(nullptr),
  m_PrimaryColor(EG_ColorWhite()),
  m_SecondaryColor(EG_ColorBlack()),
  m_pFontSmall(nullptr),
  m_pFontNormal(nullptr),
  m_pFontLarge(nullptr),
  m_Flags(0),
  m_pUserData(nullptr),
  m_Initialised(false) 
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGTheme* EGTheme::GetFromObj(EGObject *pObj)
{
	EGDisplay *pDisplay = pObj ? pObj->GetDisplay() : EGDisplay::GetDefault();
	return EGDisplay::GetTheme(pDisplay);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Apply the active theme on an object
void EGTheme::ThemeApply(EGObject *pObj)
{
	EGTheme *pTheme = GetFromObj(pObj);
	if(pTheme == nullptr) return;
	pObj->RemoveAllStyles();
	pTheme->ApplyTheme(pObj); // Apply the theme including the base theme(s)
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const EG_Font_t *EGTheme::GetFontSmall(EGObject *pObj)
{
	EGTheme *pTheme = GetFromObj(pObj);
	return pTheme ? pTheme->m_pFontSmall : EG_FONT_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const EG_Font_t *EGTheme::GetFontNormal(EGObject *pObj)
{
	EGTheme *pTheme = GetFromObj(pObj);
	return pTheme ? pTheme->m_pFontNormal : EG_FONT_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const EG_Font_t *EGTheme::GetFontLarge(EGObject *pObj)
{
	EGTheme *pTheme = GetFromObj(pObj);
	return pTheme ? pTheme->m_pFontLarge : EG_FONT_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGTheme::GetColorPrimary(EGObject *pObj)
{
	EGTheme *pTheme = GetFromObj(pObj);
	return pTheme ? pTheme->m_PrimaryColor : EG_MainPalette(EG_PALETTE_BLUE_GREY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGTheme::GetColorSecondary(EGObject *pObj)
{
	EGTheme *pTheme = GetFromObj(pObj);
	return pTheme ? pTheme->m_SecondaryColor : EG_MainPalette(EG_PALETTE_BLUE);
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGTheme::IsInitialised(void)
{
	return m_Initialised;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGTheme::StyleInitialiseReset(EGStyle *pStyle)
{
	if(m_Initialised) pStyle->Reset();
	else pStyle->Initialise();
}

