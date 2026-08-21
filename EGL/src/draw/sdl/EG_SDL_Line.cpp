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

#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

#define ROUND_START 0x01
#define ROUND_END 0x02

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
  EGSDLContext *pDC = (EGSDLContext*)pDrawLine->m_pContext;
	int32_t X1 = pPoint1->m_X, X2 = pPoint2->m_X, Y1 = pPoint1->m_Y, Y2 = pPoint2->m_Y;
	double Length = SDL_sqrt(SDL_pow(X2 - X1, 2) + SDL_pow(Y2 - Y1, 2));
	if(Length - (long)Length > 0.5) {
		Length = (long)Length + 1;
	}
	double Angle = SDL_atan2(Y2 - Y1, X2 - X1) * 180 / M_PI;
	EG_LineKey_t Key = pDC->CreateLineKey(pDrawLine, (int32_t)Length);
	SDL_Texture *pTexture = TextureCacheGet(&Key, sizeof(Key), nullptr);
	if(!pTexture) {
		pTexture = pDC->CreateLineTexture(pDrawLine, (int32_t)Length);
		TextureCachePut(&Key, sizeof(Key), pTexture);
	}
	EGRect DrawRect(X1, Y1, X2, Y2);
	const EGRect *pClip = pDC->m_pClipRect;
	SDL_Rect Rect, Clip;
	RectToSDLRect(&DrawRect, &Rect);
	RectToSDLRect(pClip, &Clip);
	EGRect TempRect = DrawRect, TempClip = *pClip, ApplyRect;
	EGRect Extension(pDrawLine->m_Width / 2, pDrawLine->m_Width / 2, pDrawLine->m_Width / 2, pDrawLine->m_Width / 2);
	pDC->CompositeBegin(&DrawRect, pClip, &Extension, pDrawLine->m_BlendMode, &TempRect, &TempClip,	&ApplyRect);
	SDL_Color Color;
	ColorToSDLColor(&pDrawLine->m_Color, &Color);
	SDL_SetTextureColorMod(pTexture, Color.r, Color.g, Color.b);
	SDL_SetTextureAlphaMod(pTexture, pDrawLine->m_OPA);
	SDL_Rect SrceRect = {0, 0, (int)Length + pDrawLine->m_Width + 2, pDrawLine->m_Width + 2};
	SDL_Rect DestRect = {TempRect.GetX1() - 1 - pDrawLine->m_Width / 2, TempRect.GetY1() - 1, SrceRect.w, SrceRect.h};
	SDL_Point Centre = {1 + pDrawLine->m_Width / 2, 1 + pDrawLine->m_Width / 2};
	SDL_Rect ClipRect;
	RectToSDLRect(&TempClip, &ClipRect);
	if(!SDL_RectEquals(&ClipRect, &DestRect) || Angle != 0) {
		SDL_RenderSetClipRect(pDC->m_pRenderer, &ClipRect);
	}
	SDL_RenderCopyEx(pDC->m_pRenderer, pTexture, &SrceRect, &DestRect, Angle, &Centre, SDL_FLIP_NONE);
	SDL_RenderSetClipRect(pDC->m_pRenderer, nullptr);
	pDC->CompositeEnd(&ApplyRect, pDrawLine->m_BlendMode);
}

///////////////////////////////////////////////////////////////////////////////////////

EG_LineKey_t EGSDLContext::CreateLineKey(const EGDrawLine *pDrawLine, int32_t Length)
{
EG_LineKey_t Key;

  EG_ZeroMem(&Key, sizeof(EG_LineKey_t));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_LINE;
	Key.Length = Length;
	Key.Width = pDrawLine->m_Width;
	Key.Round = (pDrawLine->m_RoundStart ? ROUND_START : 0) | (pDrawLine->m_RoundEnd ? ROUND_END : 0);
	return Key;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::CreateLineTexture(const EGDrawLine *pDrawLine, int32_t Length)
{
	SDL_Texture *pTexture = SDL_CreateTexture(m_pRenderer, EG_DRAW_SDL_TEXTURE_FORMAT, SDL_TEXTUREACCESS_TARGET,
																					 Length + pDrawLine->m_Width + 2, pDrawLine->m_Width + 2);
	SDL_Texture *pTarget = SDL_GetRenderTarget(m_pRenderer);
	SDL_SetRenderTarget(m_pRenderer, pTexture);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(m_pRenderer, 0xFF, 0xFF, 0xFF, 0x0);
	/* SDL_RenderClear is not working properly, so we overwrite the pTarget with solid Color */
	SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(m_pRenderer, nullptr);
	SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(m_pRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_Rect LineRect = {1 + pDrawLine->m_Width / 2, 1, Length, pDrawLine->m_Width};
	SDL_RenderFillRect(m_pRenderer, &LineRect);
	if(pDrawLine->m_RoundStart || pDrawLine->m_RoundEnd) {
		MaskRadiusParam_t Radius;
		EGRect RoundRect = {0, 0, pDrawLine->m_Width - 1, pDrawLine->m_Width - 1};
		DrawMaskSetRadius(&Radius, &RoundRect, EG_RADIUS_CIRCLE, false);
		int16_t MaskID = DrawMaskAdd(&Radius, nullptr);
		SDL_Texture *pRoundTexture = MaskDumpTexture(&RoundRect, &MaskID, 1);
		DrawMaskRemoveID(MaskID);
		SDL_Rect round_src = {0, 0, pDrawLine->m_Width, pDrawLine->m_Width};
		SDL_Rect round_dst = {LineRect.x - pDrawLine->m_Width / 2, 1, pDrawLine->m_Width, pDrawLine->m_Width};
		SDL_RenderCopy(m_pRenderer, pRoundTexture, &round_src, &round_dst);
		round_dst.x = LineRect.w + pDrawLine->m_Width / 2;
		SDL_RenderCopy(m_pRenderer, pRoundTexture, &round_src, &round_dst);
		SDL_DestroyTexture(pRoundTexture);
	}
	SDL_SetRenderTarget(m_pRenderer, pTarget);
	return pTexture;
}

#endif
