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

#include "draw/EG_DrawRect.h"
#include "draw/EG_DrawImage.h"
#include "draw/EG_DrawLabel.h"
#include "draw/EG_DrawMask.h"
#include "core/EG_Refresh.h"
#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawBackground(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
EGRect Rect;

  EGSDLContext *pDC = (EGSDLContext*)pDrawRect->m_pContext;
  const EGRect *pClipRect = pDC->m_pClipRect;
  bool HasContent = Rect.Intersect(pRect, pClipRect);  // pRect will be translated so pRect will start at (0,0)
  if(HasContent) {   // Shadows and outlines will also draw in extended area
    if(pDrawRect->m_pBackImageSource) pDC->DrawBackgroundImage(pRect, &Rect, pDrawRect);
    else pDC->DrawBackgroundColor(pRect, &Rect, pDrawRect);
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawBackgroundColor(const EGRect *pRect, const EGRect *pFillRect, const EGDrawRect *pDrawRect)
{
SDL_Color Color;
SDL_Rect Rect;

  ColorToSDLColor(&pDrawRect->m_BackgroundColor, &Color);
  SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(m_pRenderer, Color.r, Color.g, Color.b, pDrawRect->m_BackgroundOPA);
  RectToSDLRect(pFillRect, &Rect);
  SDL_RenderFillRect(m_pRenderer, &Rect);
  SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawBackgroundImage(const EGRect *pRect, const EGRect *pFillRect, const EGDrawRect *pDrawRect)
{
  SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 0);
  SDL_Rect Rect;
  RectToSDLRect(pFillRect, &Rect);
  SDL_RenderFillRect(m_pRenderer, &Rect);
  SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
  DrawRect(pDrawRect, pRect);
}

#endif
