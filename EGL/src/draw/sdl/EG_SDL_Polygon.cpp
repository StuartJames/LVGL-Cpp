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

void EGSDLContext::DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGPoint *pPoints, uint16_t PointCount)
{
	if((PointCount < 3) || (pPoints == nullptr)) return;
  EGSDLContext *pDC = (EGSDLContext*)pDrawPolygon->m_pContext;
	MaskPolygonParam_t pParam;
	DrawMaskSetPolygon(&pParam, pPoints, PointCount);
	if(pParam.Poly.Count < 3) {
		DrawMaskFreeParam(&pParam);
		return;
	}
	EGRect PolyRect(EG_COORD_MAX, EG_COORD_MAX, EG_COORD_MIN, EG_COORD_MIN);
	for(uint16_t i = 0; i < PointCount; i++) {
		PolyRect.SetX1(EG_MIN(PolyRect.GetX1(), pParam.Poly.pVertices[i].m_X));
		PolyRect.SetY1(EG_MIN(PolyRect.GetY1(), pParam.Poly.pVertices[i].m_Y));
		PolyRect.SetX2(EG_MAX(PolyRect.GetX2(), pParam.Poly.pVertices[i].m_X));
		PolyRect.SetY2(EG_MAX(PolyRect.GetY2(), pParam.Poly.pVertices[i].m_Y));
	}
	EGRect DrawRect;
	bool IsCommon = DrawRect.Intersect(&PolyRect, pDC->m_pClipRect);
	if(!IsCommon) {
		DrawMaskFreeParam(&pParam);
		return;
	}
	int16_t MaskID = DrawMaskAdd(&pParam, nullptr);
	int32_t Width = DrawRect.GetWidth();
  int32_t Height = DrawRect.GetHeight();
	bool InCache = false;
	SDL_Texture *pTexture = pDC->CompositeGetTexture(EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_STREAM1, Width, Height,	&InCache);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	pDC->PolyDumpMasks(pTexture, &DrawRect);
	DrawMaskRemoveID(MaskID);
	DrawMaskFreeParam(&pParam);
	SDL_Rect SrceRect = {0, 0, Width, Height}, DestRect;
	RectToSDLRect(&DrawRect, &DestRect);
	SDL_Color Color;
	ColorToSDLColor(&pDrawPolygon->m_FillColor, &Color);
	SDL_SetTextureColorMod(pTexture, Color.r, Color.g, Color.b);
	SDL_SetTextureAlphaMod(pTexture, pDrawPolygon->m_FillOPA);
	SDL_RenderCopy(pDC->m_pRenderer, pTexture, &SrceRect, &DestRect);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::PolyDumpMasks(SDL_Texture *pTexture, const EGRect *pRect)
{
	int32_t Width = pRect->GetWidth(), Height = pRect->GetHeight();
	SDL_assert(Width > 0 && Height > 0);
	SDL_Rect Rect = {0, 0, Width, Height};
	uint8_t *pPixels;
	int Pitch;
	if(SDL_LockTexture(pTexture, &Rect, (void **)&pPixels, &Pitch) != 0) return;

	EG_OPA_t *pLineBuffer = (EG_OPA_t*)EG_GetBufferMem(Rect.w);
	for(int32_t y = 0; y < Rect.h; y++) {
		EG_SetMemFF(pLineBuffer, Rect.w);
		int32_t AbsX = (int32_t)pRect->GetX1();
    int32_t AbsY = (int32_t)(y + pRect->GetY1());
    int32_t Length = (int32_t)Rect.w;
		DrawMaskRes_t Result = DrawMaskApply(pLineBuffer, AbsX, AbsY, Length);
		if(Result == EG_DRAW_MASK_RESULT_TRANSP) {
			EG_ZeroMem(&pPixels[y * Pitch], 4 * Rect.w);
		}
		else if(Result == EG_DRAW_MASK_RESULT_FULL_COVER) {
			EG_SetMemFF(&pPixels[y * Pitch], 4 * Rect.w);
		}
		else {
			for(int x = 0; x < Rect.w; x++) {
				uint8_t *pixel = &pPixels[y * Pitch + x * 4];
				*pixel = pLineBuffer[x];
				pixel[1] = pixel[2] = pixel[3] = 0xFF;
			}
		}
	}
	EG_ReleaseBufferMem(pLineBuffer);
	SDL_UnlockTexture(pTexture);
}

#endif
