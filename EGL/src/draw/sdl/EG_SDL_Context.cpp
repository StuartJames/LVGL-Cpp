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

//SDL_Renderer   *EGSDLContext::m_pRenderer = nullptr;
EG_LRU_t       *EGSDLContext::m_pTextureCache = nullptr;
SDL_Texture    *EGSDLContext::m_pMask = nullptr;
SDL_Texture    *EGSDLContext::m_pComposition = nullptr;
bool            EGSDLContext::m_CompositionCached = false;
SDL_Texture    *EGSDLContext::m_pTargetBackup = nullptr;
uint8_t         EGSDLContext::m_TransformCount = 0;

///////////////////////////////////////////////////////////////////////////////////////

EGSDLContext::~EGSDLContext(void)
{
	TextureCacheDeinit();
	UtilsDeinit();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::InitialiseContext(EGDisplayDriver *pDriver)
{
	DrawArcProc = DrawArc;
	DrawRectProc = DrawRect;
	DrawBackgroundProc = DrawBackground;
	DrawCharacterProc = DrawCharacter;
  DrawImageProc = DrawImage;
	DrawLineProc = DrawLine;
	DrawPolygonProc = DrawPolygon;
	LayerIntialiseProc = DrawLayerCreate;
	LayerBlendProc = DrawLayerBlend;
	LayerDestroyProc = DrawLayerDestroy;
	m_pRenderer = ((EG_SDL_DriverParam_t*)pDriver->m_pExtData)->pRenderer;
	UtilsInit();
	TextureCacheInit();
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::CreateScreenTexture(int32_t Horizontal, int32_t Vertical)
{
	SDL_Texture *pTexture = SDL_CreateTexture(m_pRenderer, EG_DRAW_SDL_TEXTURE_FORMAT, SDL_TEXTUREACCESS_TARGET, Horizontal, Vertical);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	return pTexture;
}

#endif
