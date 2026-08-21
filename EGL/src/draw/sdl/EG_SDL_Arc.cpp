/*
 *                EGL 2025-2026 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; withpRect even the implied warranty of
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
 * SJ    2026/07/20   8.6.0    Modified file layopRect & class naming
 *
 */

#include "EG_IntrnlConfig.h"

#if EG_USE_GPU_SDL

#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawArc(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius, uint16_t StartAngle, uint16_t EndAngle)
{
	EGSDLContext *pDC = (EGSDLContext*)pDrawArc->m_pContext;
  // -1 because the pCenter already belongs to the left/bottom part
	EGRect OuterRect(pCenter->m_X - Radius, pCenter->m_Y- Radius, pCenter->m_X + Radius - 1, pCenter->m_Y + Radius - 1);
	EGRect InnerRect(OuterRect);
	InnerRect.IncX1(pDrawArc->m_Width);
	InnerRect.IncY1(pDrawArc->m_Width);
	InnerRect.DecX2(pDrawArc->m_Width);
	InnerRect.DecY2(pDrawArc->m_Width);
	EGRect DrawRect;
	if(!DrawRect.Intersect(&OuterRect, pDC->m_pClipRect)) return;
	while(StartAngle >= 360) StartAngle -= 360;
	while(EndAngle >= 360) EndAngle -= 360;
	int16_t MaskIDs[3] = {EG_MASK_ID_INVALID, EG_MASK_ID_INVALID, EG_MASK_ID_INVALID}, MaskIDCount = 1;
	int16_t CapIDs[2] = {EG_MASK_ID_INVALID, EG_MASK_ID_INVALID};
	MaskRadiusParam_t MaskOutsideParam;
	DrawMaskSetRadius(&MaskOutsideParam, &OuterRect, EG_RADIUS_CIRCLE, false);
	MaskIDs[0] = DrawMaskAdd(&MaskOutsideParam, nullptr);

	MaskRadiusParam_t MaskInsideParam;
	if(InnerRect.GetWidth() > 0 && InnerRect.GetHeight() > 0) {
		DrawMaskSetRadius(&MaskInsideParam, &InnerRect, EG_RADIUS_CIRCLE, true);
		MaskIDs[1] = DrawMaskAdd(&MaskInsideParam, nullptr);
		MaskIDCount++;
	}

	MaskAngleParam_t MaskAngleParam;
	if((StartAngle - EndAngle) % 360) {
		DrawMaskSetAngle(&MaskAngleParam, pCenter, StartAngle, EndAngle);
		MaskIDs[2] = DrawMaskAdd(&MaskAngleParam, nullptr);
		MaskIDCount++;
	}

	MaskRadiusParam_t CapStartParam, CapEndParam;
	if(MaskIDCount == 3 && pDrawArc->m_Rounded) {
		EGRect StartRect, EndRect;
		pDC->GetCapRect((int16_t)StartAngle, pDrawArc->m_Width, Radius, pCenter, &StartRect);
		pDC->GetCapRect((int16_t)EndAngle, pDrawArc->m_Width, Radius, pCenter, &EndRect);
		DrawMaskSetRadius(&CapStartParam, &StartRect, pDrawArc->m_Width / 2, false);
		CapIDs[0] = DrawMaskAdd(&CapStartParam, nullptr);
		DrawMaskSetRadius(&CapEndParam, &EndRect, pDrawArc->m_Width / 2, false);
		CapIDs[1] = DrawMaskAdd(&CapEndParam, nullptr);
	}

	int32_t Width = DrawRect.GetWidth(), Height = DrawRect.GetHeight();
	bool TextureCached = false;
	SDL_Texture *pTexture = pDC->CompositeGetTexture(EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_STREAM1, Width, Height,	&TextureCached);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	pDC->ArcDumpMasks(pTexture, &DrawRect, MaskIDs, MaskIDCount, CapIDs[0] != EG_MASK_ID_INVALID ? CapIDs : nullptr);
	DrawMaskRemoveID(MaskIDs[0]);
	DrawMaskFreeParam(&MaskOutsideParam);
	if(MaskIDCount > 1) {
		DrawMaskRemoveID(MaskIDs[1]);
		DrawMaskFreeParam(&MaskInsideParam);
	}
	if(MaskIDCount > 2) {
		DrawMaskRemoveID(MaskIDs[2]);
		DrawMaskFreeParam(&MaskAngleParam);
	}
	if(CapIDs[0] != EG_MASK_ID_INVALID) {
		DrawMaskRemoveID(CapIDs[0]);
		DrawMaskRemoveID(CapIDs[1]);
		DrawMaskFreeParam(&CapStartParam);
		DrawMaskFreeParam(&CapEndParam);
	}
	SDL_Rect SrceRect = {0, 0, Width, Height}, DestRect;
	RectToSDLRect(&DrawRect, &DestRect);
	SDL_Color Color;
	ColorToSDLColor(&pDrawArc->m_Color, &Color);
	SDL_SetTextureColorMod(pTexture, Color.r, Color.g, Color.b);
	SDL_SetTextureAlphaMod(pTexture, pDrawArc->m_OPA);
	SDL_RenderCopy(pDC->m_pRenderer, pTexture, &SrceRect, &DestRect);

	if(!TextureCached) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::ArcDumpMasks(SDL_Texture *pTexture, const EGRect *pRect, const int16_t *pIDs, int16_t IDCount, const int16_t *pCaps)
{
	int32_t Width = pRect->GetWidth(), Height = pRect->GetHeight();
	SDL_assert(Width > 0 && Height > 0);
	SDL_Rect Rect = {0, 0, Width, Height};
	uint8_t *pPixels;
	int Pitch;
	if(SDL_LockTexture(pTexture, &Rect, (void **)&pPixels, &Pitch) != 0) return;

	EG_OPA_t *pBuffer = (EG_OPA_t*)EG_GetBufferMem(Rect.w);
	for(int32_t y = 0; y < Rect.h; y++) {
		EG_SetMemFF(pBuffer, Rect.w);
		int32_t AbsX = (int32_t)pRect->GetX1(), AbsY = (int32_t)(y + pRect->GetY1()), Length = (int32_t)Rect.w;
		DrawMaskRes_t Result = DrawMaskApplyIDs(pBuffer, AbsX, AbsY, Length, pIDs, IDCount);
		if(Result == EG_DRAW_MASK_RESULT_TRANSP) {
			EG_ZeroMem(&pPixels[y * Pitch], 4 * Rect.w);
		}
		else if(Result == EG_DRAW_MASK_RESULT_FULL_COVER) {
			EG_SetMemFF(&pPixels[y * Pitch], 4 * Rect.w);
		}
		else {
			for(int x = 0; x < Rect.w; x++) {
				uint8_t *pixel = &pPixels[y * Pitch + x * 4];
				*pixel = pBuffer[x];
				pixel[1] = pixel[2] = pixel[3] = 0xFF;
			}
		}
		if(pCaps) {
			for(int i = 0; i < 2; i++) {
				EG_SetMemFF(pBuffer, Rect.w);
				Result = DrawMaskApplyIDs(pBuffer, AbsX, AbsY, Length, &pCaps[i], 1);
				if(Result == EG_DRAW_MASK_RESULT_TRANSP) {
					// Ignore 
				}
				else if(Result == EG_DRAW_MASK_RESULT_FULL_COVER) {
					EG_SetMemFF(&pPixels[y * Pitch], 4 * Rect.w);
				}
				else {
					for(int x = 0; x < Rect.w; x++) {
						uint8_t *pixel = &pPixels[y * Pitch + x * 4];
						uint16_t old_opa = pBuffer[x] + *pixel;
						*pixel = EG_MIN(old_opa, 0xFF);
						pixel[1] = pixel[2] = pixel[3] = 0xFF;
					}
				}
			}
		}
	}
	EG_ReleaseBufferMem(pBuffer);
	SDL_UnlockTexture(pTexture);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::GetCapRect(int16_t Angle, int32_t Width, uint16_t Radius, const EGPoint *pCenter, EGRect *pRect)
{
const uint8_t ps = 8;
const uint8_t pa = 127;

	int32_t HalfWidth = Width / 2;
	uint8_t WidthCorr = (Width & 0x01) ? 0 : 1;
	int32_t CirX = ((Radius - HalfWidth) * EG_TrigoSin((int16_t)(90 - Angle))) >> (EG_TRIGO_SHIFT - ps);
	int32_t CirY = ((Radius - HalfWidth) * EG_TrigoSin(Angle)) >> (EG_TRIGO_SHIFT - ps);
	if(CirX > 0) {   // Actually the pCenter of the pixel need to be calculated so apply 1/2 px offset
		CirX = (CirX - pa) >> ps;
		pRect->SetX1(CirX - HalfWidth + WidthCorr);
		pRect->SetX2(CirX + HalfWidth);
	}
	else {
		CirX = (CirX + pa) >> ps;
		pRect->SetX1(CirX - HalfWidth);
		pRect->SetX2(CirX + HalfWidth - WidthCorr);
	}

	if(CirY > 0) {
		CirY = (CirY - pa) >> ps;
		pRect->SetY1(CirY - HalfWidth + WidthCorr);
		pRect->SetY2(CirY + HalfWidth);
	}
	else {
		CirY = (CirY + pa) >> ps;
		pRect->SetY1(CirY - HalfWidth);
		pRect->SetY2(CirY + HalfWidth - WidthCorr);
	}
	pRect->Move(pCenter->m_X, pCenter->m_Y);
}

#endif
