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

#include "font/EG_Font.h"
#include "misc/EG_Utilities.h"
#include "misc/EG_Log.h"
#include "misc/EG_Assert.h"

/////////////////////////////////////////////////////////////////////////////////////////

const uint8_t *EG_FontGetGlyphBitmap(const EG_Font_t *pFont, uint32_t Character)
{
	EG_ASSERT_NULL(pFont);
	return pFont->GetGlyphBitmapCB(pFont, Character);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EG_FontGetGlyphProps(const EG_Font_t *pFont, EG_FontGlyphProps_t *pProps, uint32_t Character, uint32_t NextCharacter)
{
	EG_ASSERT_NULL(pFont);
	EG_ASSERT_NULL(pProps);
#if EG_USE_FONT_PLACEHOLDER
	const EG_Font_t *pPlaceholderFont = nullptr;
#endif
	const EG_Font_t *pIterFont = pFont;
	pProps->ResolvedFont = nullptr;
	while(pIterFont) {
		bool found = pIterFont->GetGlyphPropsCB(pIterFont, pProps, Character, NextCharacter);
		if(found) {
			if(!pProps->IsPlaceholder) {
				pProps->ResolvedFont = pIterFont;
				return true;
			}
#if EG_USE_FONT_PLACEHOLDER
			else if(pPlaceholderFont == nullptr) {
				pPlaceholderFont = pIterFont;
			}
#endif
		}
		pIterFont = pIterFont->pFallback;
	}
#if EG_USE_FONT_PLACEHOLDER
	if(pPlaceholderFont != nullptr) {
		pPlaceholderFont->GetGlyphPropsCB(pPlaceholderFont, pProps, Character, NextCharacter);
		pProps->ResolvedFont = pPlaceholderFont;
		return true;
	}
#endif
	if(Character < 0x20 ||
		 Character == 0xf8ff || // EG_SYMBOL_DUMMY
		 Character == 0x200c) { // ZERO WIDTH NON-JOINER
		pProps->BoxWidth = 0;
		pProps->AdvWidth = 0;
	}
	else {
#if EG_USE_FONT_PLACEHOLDER
		pProps->BoxWidth = pFont->LineHeight / 2;
		pProps->AdvWidth = pProps->BoxWidth + 2;
#else
		pProps->BoxWidth = 0;
		pProps->AdvWidth = 0;
#endif
	}
	pProps->ResolvedFont = nullptr;
	pProps->BoxHeight = pFont->LineHeight;
	pProps->OffsetX = 0;
	pProps->OffsetY = 0;
	pProps->BitsPerPixel = 1;
	pProps->IsPlaceholder = true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

uint16_t EG_FontGetGlyphWidth(const EG_Font_t *pFont, uint32_t Character, uint32_t NextCharacter)
{
	EG_ASSERT_NULL(pFont);
	EG_FontGlyphProps_t GlyphProps;
	EG_FontGetGlyphProps(pFont, &GlyphProps, Character, NextCharacter);
	return GlyphProps.AdvWidth;
}
