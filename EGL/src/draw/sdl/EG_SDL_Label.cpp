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

#include "EG_IntrnlConfig.h"

#if EG_USE_GPU_SDL

#include EG_GPU_SDL_INCLUDE_PATH

#include "draw/EG_DrawLabel.h"
#include "misc/EG_Utilities.h"
#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawCharacter(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, uint32_t Character)
{
  
  EGSDLContext *pDC = (EGSDLContext*)pDrawLabel->m_pContext;
	const EGRect *pClipRect = pDC->m_pClipRect;
	const EG_Font_t *pFont = pDrawLabel->m_pFont;
	EG_OPA_t OPA = pDrawLabel->m_OPA;
	EG_Color_t Color = pDrawLabel->m_Color;
	if(OPA < EG_OPA_MIN) return;
	if(OPA > EG_OPA_MAX) OPA = EG_OPA_COVER;
	if(pFont == nullptr) {
		EG_LOG_WARN("DrawCharacter: font is nullptr");
		return;
	}
	EG_FontGlyphProps_t Glyph;
	bool Result = EG_FontGetGlyphProps(pFont, &Glyph, Character, '\0');
	if(Result == false) {
		// Add warning if the pDrawLabel is not found but do not print warning for non printable ASCII chars (e.g. '\n')
		if(Character >= 0x20 &&
			 Character != 0xf8ff && // EG_SYMBOL_DUMMY
			 Character != 0x200c) { // ZERO WIDTH NON-JOINER
			EG_LOG_WARN("DrawCharacter: glyph pDrawLabel. not found for U+%X", Character);
			//  draw placeholder
			EGRect GlyphRect;
			int32_t BeginX = pPos->m_X + Glyph.OffsetX;
			int32_t BeginY = pPos->m_Y + Glyph.OffsetY;
			GlyphRect.Set(BeginX, BeginY, BeginX + Glyph.BoxWidth, BeginY + Glyph.BoxHeight);
			EGDrawRect DrawRect;
			DrawRect.m_BackgroundOPA = EG_OPA_MIN;
			DrawRect.m_OutlineOPA = EG_OPA_MIN;
			DrawRect.m_ShadowOPA = EG_OPA_MIN;
			DrawRect.m_BackImageOPA = EG_OPA_MIN;
			DrawRect.m_BorderColor = pDrawLabel->m_Color;
			DrawRect.m_BorderWidth = 1;
			DrawRect.Draw(pDrawLabel->m_pContext, &GlyphRect);
		}
		return;
	}
	// Don't draw anything if the character is empty. E.g. space
	if((Glyph.BoxHeight == 0) || (Glyph.BoxWidth == 0)) return;
	int32_t PosX = pPos->m_X + Glyph.OffsetX;
	int32_t PosY = pPos->m_Y + (pFont->LineHeight - pFont->BaseLine) - Glyph.BoxHeight - Glyph.OffsetY;
	const EGRect CharRect(PosX, PosY, PosX + Glyph.BoxWidth - 1, PosY + Glyph.BoxHeight - 1);
	EGRect DrawRect;
	// If the Character is completely out of pMask don't draw it
	if(!DrawRect.Intersect(&CharRect, pClipRect)) return;
	EG_FontGlyphKey_t GlyphKey = pDC->CreateFontGlyphKey(pFont, Character);
	bool GlyphFound = false;
	SDL_Texture *pTexture = TextureCacheGet(&GlyphKey, sizeof(GlyphKey), &GlyphFound);
	bool InCache = false;
	if(!GlyphFound) {
		if(Glyph.ResolvedFont) {
			pFont = Glyph.ResolvedFont;
		}
		const uint8_t *pBMP = EG_FontGetGlyphBitmap(pFont, Character);
		uint8_t *pBuffer = (uint8_t*)EG_AllocMem(Glyph.BoxWidth * Glyph.BoxHeight);
		SDLTo8BPP(pBuffer, pBMP, Glyph.BoxWidth, Glyph.BoxHeight, Glyph.BoxWidth, Glyph.BitsPerPixel);
		SDL_Surface *pMask = CreateOPASurface(pBuffer, Glyph.BoxWidth, Glyph.BoxHeight, Glyph.BoxWidth);
		pTexture = SDL_CreateTextureFromSurface(pDC->m_pRenderer, pMask);
		SDL_FreeSurface(pMask);
		EG_FreeMem(pBuffer);
		InCache = TextureCachePut(&GlyphKey, sizeof(GlyphKey), pTexture);
	}
	else InCache = true;
	if(!pTexture) return;
	EGRect TempRect(CharRect);
  EGRect TempClip(pClipRect);
  EGRect ApplyRect;
	bool HasComposite = pDC->CompositeBegin(&CharRect, pClipRect, nullptr, pDrawLabel->m_BlendMode, &TempRect, &TempClip, &ApplyRect);
	pDC->TransformAreasOffset(HasComposite, &ApplyRect, &TempRect, &TempClip);
	// If the Character is completely out of pMask don't draw it
	if(!DrawRect.Intersect(&TempRect, &TempClip)) {
		if(!InCache) {
			EG_LOG_WARN("Texture is not cached, this will impact performance.");
			SDL_DestroyTexture(pTexture);
		}
		return;
	}
	SDL_Rect SrceRect, DestRect;
	RectToSDLRect(&DrawRect, &DestRect);
	SrceRect.x = DrawRect.GetX1() - TempRect.GetX1();
	SrceRect.y = DrawRect.GetY1() - TempRect.GetY1();
	SrceRect.w = DestRect.w;
	SrceRect.h = DestRect.h;
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(pTexture, OPA);
	SDL_SetTextureColorMod(pTexture, Color.ch.red, Color.ch.green, Color.ch.blue);
	SDL_RenderCopy(pDC->m_pRenderer, pTexture, &SrceRect, &DestRect);
	pDC->CompositeEnd(&ApplyRect, pDrawLabel->m_BlendMode);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_FontGlyphKey_t EGSDLContext::CreateFontGlyphKey(const EG_Font_t *pFont, uint32_t Char)
{
	EG_FontGlyphKey_t Key;
	//  VERY IMPORTANT! Padding between members is uninitialized, so we have to wipe them manually 
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_FONT_GLYPH;
	Key.pFont = pFont;
	Key.Character = Char;
	return Key;
}

#endif
