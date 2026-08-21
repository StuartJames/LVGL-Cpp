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

#ifndef HAVE_SDL_CUSTOM_BLEND_MODE
#define HAVE_SDL_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
#endif

///////////////////////////////////////////////////////////////////////////////////////

EG_OPA_t* EGSDLContext::MaskDumpOPA(const EGRect *pRect, const int16_t *IDs, int16_t IDCount)
{
	SDL_assert(pRect->GetX2() >= pRect->GetX1());
	SDL_assert(pRect->GetY2() >= pRect->GetY1());
	int32_t Width = pRect->GetWidth();
  int32_t Height = pRect->GetHeight();
	EG_OPA_t *pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(Width * Height);
	for(int32_t y = 0; y < Height; y++) {
		EG_OPA_t *pLineBuffer = &pMaskBuffer[y * Width];
		EG_SetMemFF(pLineBuffer, Width);
		int32_t AbsX = (int32_t)pRect->GetX1();
    int32_t AbsY = (int32_t)(y + pRect->GetY1());
    int32_t Length = (int32_t)Width;
		DrawMaskRes_t Result;
		if(IDs)	Result = DrawMaskApplyIDs(pLineBuffer, AbsX, AbsY, Length, IDs, IDCount);
		else Result = DrawMaskApply(pLineBuffer, AbsX, AbsY, Length);
		if(Result == EG_DRAW_MASK_RESULT_TRANSP) EG_ZeroMem(pLineBuffer, Width);
	}
	return pMaskBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::MaskDumpTexture(const EGRect *pRect, const int16_t *IDs, int16_t IDCount)
{
	int32_t Width = pRect->GetWidth();
  int32_t Height = pRect->GetHeight();
	EG_OPA_t *pMaskBuffer = MaskDumpOPA(pRect, IDs, IDCount);
	SDL_Surface *pSurface = CreateOPASurface(pMaskBuffer, Width, Height, Width);
	EG_ReleaseBufferMem(pMaskBuffer);
	SDL_Texture *pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
	SDL_FreeSurface(pSurface);
	return pTexture;
}

#endif
