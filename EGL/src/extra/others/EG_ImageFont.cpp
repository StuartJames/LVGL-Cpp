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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "extra/others/EG_ImageFont.h"

/////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_IMGFONT

/////////////////////////////////////////////////////////////////////////////////////

EGImageFont::EGImageFont(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////

EGImageFont::~EGImageFont(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////

EG_Font_t* EGImageFont::Create(uint16_t Height)
{
	m_pFont->pProperties = (void*)this;
	m_pFont->GetGlyphPropsCB = GetGlyphDiscriptor;
	m_pFont->GetGlyphBitmapCB = GetGlyphBitmap;
	m_pFont->SubPixel = EG_FONT_SUBPX_NONE;
	m_pFont->LineHeight = Height;
	m_pFont->BaseLine = 0;
	m_pFont->UnderlinePosition = 0;
	m_pFont->UnderlineThickness = 0;
	return m_pFont;
}

/////////////////////////////////////////////////////////////////////////////////////

bool EGImageFont::GetGlyphDiscriptor(const EG_Font_t *pFont, EG_FontGlyphProps_t *pDiscriptor,
																	uint32_t Unicode, uint32_t UnicodeNext)
{
EG_ImageHeader_t Header;

	EG_ASSERT_NULL(pFont);
  EGImageFont *pImageFont = (EGImageFont*)pFont->pProperties;
	if(!pImageFont->GetPath(pImageFont->m_pFont, pImageFont->m_Path, EG_IMAGEFONT_PATH_MAX_LEN, Unicode, UnicodeNext)) return false;
	if(EGImageDecoder::GetInfo(pImageFont->m_Path, &Header) != EG_RES_OK) return false;
	pDiscriptor->IsPlaceholder = 0;
	pDiscriptor->AdvWidth = Header.Width;
	pDiscriptor->BoxWidth = Header.Width;
	pDiscriptor->BoxHeight = Header.Height;
	pDiscriptor->BitsPerPixel = EG_IMGFONT_BPP; // Is image identifier 
	pDiscriptor->OffsetX = 0;
	pDiscriptor->OffsetY = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

const uint8_t* EGImageFont::GetGlyphBitmap(const EG_Font_t *pFont, uint32_t Unicode)
{
	EG_UNUSED(Unicode);
	EG_ASSERT_NULL(pFont);
  EGImageFont *pImageFont = (EGImageFont*)pFont->pProperties;
	return (uint8_t *)pImageFont->m_Path;
}


#endif 
