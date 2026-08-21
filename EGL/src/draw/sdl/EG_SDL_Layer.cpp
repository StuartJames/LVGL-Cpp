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

#include "core/EG_Refresh.h"
#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

EG_SDLLayerContext::EG_SDLLayerContext() : EGLayerContext(),
  m_pOrigTarget(nullptr),
  m_pTarget(nullptr),
  m_InCache(false),
  m_Flags(EG_DRAW_LAYER_FLAG_NONE)
{
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSDLContext::DrawLayerCreate(EGLayerContext *pDrawLayer,	EGDrawLayerFlags_e Flags)
{
  EG_SDLLayerContext *pSDLLayer = (EG_SDLLayerContext*)pDrawLayer;
  EGSDLContext *pDC = (EGSDLContext*)pSDLLayer->m_pContext;
	pSDLLayer->m_Flags = Flags;
	pSDLLayer->m_pOrigTarget = SDL_GetRenderTarget(pDC->m_pRenderer);
	int32_t Width = pDrawLayer->m_FullRect.GetWidth();
	int32_t Height = pDrawLayer->m_FullRect.GetHeight();
	EG_SDL_CompositeTextures_e TextureID = (EG_SDL_CompositeTextures_e)(EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TRANSFORM0 + m_TransformCount);
	pSDLLayer->m_pTarget = pDC->CompositeGetTexture(TextureID, Width, Height, &pSDLLayer->m_InCache);
	pSDLLayer->m_TargetRect.x = 0;
	pSDLLayer->m_TargetRect.y = 0;
	pSDLLayer->m_TargetRect.w = Width;
	pSDLLayer->m_TargetRect.h = Height;
	pDrawLayer->m_MaxRowWithAlpha = Height;
	pDrawLayer->m_MaxRowWithoutAlpha = Height;
	SDL_SetTextureBlendMode(pSDLLayer->m_pTarget, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(pDC->m_pRenderer, pSDLLayer->m_pTarget);
	// SDL_RenderClear is not working properly, so we overwrite the target with solid color */
	SDL_SetRenderDrawBlendMode(pDC->m_pRenderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(pDC->m_pRenderer, 0, 0, 0, 0);
	SDL_RenderFillRect(pDC->m_pRenderer, nullptr);
	SDL_SetRenderDrawBlendMode(pDC->m_pRenderer, SDL_BLENDMODE_BLEND);
	// Set proper drawing context for transform layer */
	m_TransformCount += 1;
	pSDLLayer->m_Original.pBuferArea = &pDrawLayer->m_FullRect;
	pSDLLayer->m_Original.pClipRect = &pDrawLayer->m_FullRect;
	return pDrawLayer;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawLayerBlend(EGLayerContext *pDrawLayer, EGDrawImage *pImage)
{
  EG_SDLLayerContext *pSDLLayer = (EG_SDLLayerContext*)pDrawLayer;
  EGSDLContext *pDC = (EGSDLContext*)pSDLLayer->m_pContext;
	SDL_Rect TransRect;
	if(pSDLLayer->m_Flags & EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE) {
		ScaleToSDLRect(&pDrawLayer->m_FullRect, &TransRect, pImage->m_Scale, &pImage->m_Pivot);
	}
	else {
		ScaleToSDLRect(&pDrawLayer->m_FullRect, &TransRect, pImage->m_Scale, &pImage->m_Pivot);
	}
	SDL_SetRenderTarget(pDC->m_pRenderer, pSDLLayer->m_pOrigTarget);
	// Render off-screen texture, transformed
	SDL_Rect ClipRect;
	RectToSDLRect(pDrawLayer->m_Original.pClipRect, &ClipRect);
	SDL_Point center = {.x = pImage->m_Pivot.m_X, .y = pImage->m_Pivot.m_Y};
	SDL_RenderSetClipRect(pDC->m_pRenderer, &ClipRect);
	SDL_SetTextureAlphaMod(pSDLLayer->m_pTarget, pImage->m_OPA);
	SDL_RenderCopyEx(pDC->m_pRenderer, pSDLLayer->m_pTarget, &pSDLLayer->m_TargetRect, &TransRect, pImage->m_Angle, &center, SDL_FLIP_NONE);
	SDL_RenderSetClipRect(pDC->m_pRenderer, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawLayerDestroy(EGLayerContext *pDrawLayer)
{
  EG_SDLLayerContext *pSDLLayer = (EG_SDLLayerContext*)pDrawLayer;
	if(!pSDLLayer->m_InCache && pSDLLayer->m_pTarget != nullptr) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pSDLLayer->m_pTarget);
	}
	m_TransformCount -= 1;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::TransformAreasOffset(bool HasComposite, EGRect *pApplyRect,	EGRect *pRect, EGRect *pClip) const
{
	if(m_TransformCount == 0)	return;
	EGRect *pDrawRect = m_pDrawRect;
	pRect->Move(-pDrawRect->GetX1(), -pDrawRect->GetY1());
	pClip->Move(-pDrawRect->GetX1(), -pDrawRect->GetY1());
	if(HasComposite) {
		pApplyRect->Move(-pDrawRect->GetX1(), -pDrawRect->GetY1());
	}
}

#endif
