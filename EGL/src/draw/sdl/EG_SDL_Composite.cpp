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

//#include "misc/lv_gc.h"
#include "core/EG_Refresh.h"
#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

bool EGSDLContext::CompositeBegin(const EGRect *pRectIn, const EGRect *pClipIn, const EGRect *pExtRect, EG_BlendMode_e BlendMode, 
                    EGRect *pRectOut, EGRect *pClipOut, EGRect *pApplyRect)
{
EGRect Rect = *pRectIn;

	Rect.Normalise();
	if(pExtRect) Rect.Inflate(pExtRect);
	if(!pApplyRect->Intersect(&Rect, pClipIn)) return false;
	bool HasMask = HasAnyDrawMask(pApplyRect);
	const bool DrawMask = HasMask && EG_GPU_SDL_CUSTOM_BLEND_MODE;
	const bool DrawBlend = BlendMode != EG_BLEND_MODE_NORMAL;
	if(DrawMask || DrawBlend) {
		EG_ASSERT(m_pMask == nullptr && m_pComposition == nullptr && m_pTargetBackup == nullptr);

		int32_t Width = pApplyRect->GetWidth();
    int32_t Height = pApplyRect->GetHeight();
		m_pComposition = CompositeGetTexture(EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TARGET0, Width, Height, &m_CompositionCached);
		// Don't need to worry about integral overflow
		int32_t OffsetX = (int32_t)-pApplyRect->GetX1();
    int32_t OffsetY = (int32_t)-pApplyRect->GetY1();
		// Offset draw area to start with (0,0) of pRect
		pRectOut->Move(OffsetX, OffsetY);
		pClipOut->Move(OffsetX, OffsetY);
		m_pTargetBackup = SDL_GetRenderTarget(m_pRenderer);
		SDL_SetRenderTarget(m_pRenderer, m_pComposition);
		SDL_SetRenderDrawColor(m_pRenderer, 255, 255, 255, 0);
		// SDL_RenderClear is not working properly, so we overwrite the target with solid color
		SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
		SDL_RenderFillRect(m_pRenderer, nullptr);
		SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
#if EG_GPU_SDL_CUSTOM_BLEND_MODE
		m_pMask = CompositeGetTexture(EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_STREAM0, Width, Height, &m_CompositionCached);
		DumpMasks(m_pMask, pApplyRect);
#endif
	}
	else if(HasMask) {
		// Fallback mask handling. This will at least make bars looks better
		for(uint8_t i = 0; i < _EG_MASK_MAX_NUM; i++) {
			MaskCommonDiscrpt_t *pCommon = (MaskCommonDiscrpt_t*)EG_GC_ROOT(EG_DrawMaskArray[i]).pMask;
			if(pCommon == nullptr) continue;
			switch(pCommon->Type) {
				case EG_DRAW_MASK_TYPE_RADIUS: {
					const MaskRadiusParam_t *pParam = (const MaskRadiusParam_t*)pCommon;
					if(pParam->Radius.Radius) break;
					pClipOut->Intersect(pApplyRect, &pParam->Radius.Area);
					break;
				}
				default:
					break;
			}
		}
	}
	return HasMask;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::CompositeEnd(const EGRect *pApplyRect, EG_BlendMode_e BlendMode)
{
	SDL_Rect SrceRect = {0, 0, pApplyRect->GetWidth(), pApplyRect->GetHeight()};
#if EG_GPU_SDL_CUSTOM_BLEND_MODE
	if(m_pMask) {
		SDL_BlendMode Mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE,
																										SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO,
																										SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
		SDL_SetTextureBlendMode(m_pMask, Mode);
		SDL_RenderCopy(m_pRenderer, m_pMask, &SrceRect, &SrceRect);
	}
#endif

	// Shapes are drawn on composite layer when mask or blend Mode is present
	if(m_pComposition) {
		SDL_Rect DestRect;
		RectToSDLRect(pApplyRect, &DestRect);
		SDL_SetRenderTarget(m_pRenderer, m_pTargetBackup);
		switch(BlendMode) {
			case EG_BLEND_MODE_NORMAL:
				SDL_SetTextureBlendMode(m_pComposition, SDL_BLENDMODE_BLEND);
				break;
			case EG_BLEND_MODE_ADDITIVE:
				SDL_SetTextureBlendMode(m_pComposition, SDL_BLENDMODE_ADD);
				break;
#if EG_GPU_SDL_CUSTOM_BLEND_MODE
			case EG_BLEND_MODE_SUBTRACTIVE: {
				SDL_BlendMode Mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE,
																												SDL_BLENDOPERATION_SUBTRACT, SDL_BLENDFACTOR_ONE,
																												SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_SUBTRACT);
				SDL_SetTextureBlendMode(m_pComposition, Mode);
				break;
			}
			case EG_BLEND_MODE_MULTIPLY: {
				SDL_BlendMode Mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_COLOR,
																												SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO,
																												SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDOPERATION_ADD);
				SDL_SetTextureBlendMode(m_pComposition, Mode);
				break;
			}
#endif
			default:
				EG_LOG_WARN("Doesn't support blend Mode %d", BlendMode);
				SDL_SetTextureBlendMode(m_pComposition, SDL_BLENDMODE_BLEND);
				// Unsupported
				break;
		}
		SDL_RenderCopy(m_pRenderer, m_pComposition, &SrceRect, &DestRect);
		if(!m_CompositionCached) {
			EG_LOG_WARN("Texture is not cached, this will impact performance.");
			SDL_DestroyTexture(m_pComposition);
		}
	}
	m_pMask = m_pComposition = m_pTargetBackup = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::CompositeGetTexture(EG_SDL_CompositeTextures_e ID, int32_t Width, int32_t Height, bool *pTextureCached)
{
	EGPoint *pTextSize = nullptr;
	CompositeKey_t MaskKey = MaskKeyCreate(ID);
	SDL_Texture *pResult = TextureCacheGetWithExtData(&MaskKey, sizeof(CompositeKey_t), nullptr,	(void **)&pTextSize);
	if(pResult == nullptr || pTextSize->m_X < Width || pTextSize->m_Y < Height) {
		int32_t Size = NextPowOf2(EG_MAX(Width, Height));
		int Access = SDL_TEXTUREACCESS_STREAMING;
		if(ID >= EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TRANSFORM0) Access = SDL_TEXTUREACCESS_TARGET;
		else if(ID >= EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TARGET0) Access = SDL_TEXTUREACCESS_TARGET;
		pResult = SDL_CreateTexture(m_pRenderer, EG_DRAW_SDL_TEXTURE_FORMAT, Access, Size, Size);
		pTextSize = new EGPoint;
		pTextSize->m_X = pTextSize->m_Y = Size;
		bool InCache = TextureCachePutAdvanced(&MaskKey, sizeof(CompositeKey_t), pResult, pTextSize, EG_FreeMem, EG_DRAW_SDL_CACHE_FLAG_NONE);
		if(!InCache) delete pTextSize;
		if(pTextureCached != nullptr) *pTextureCached = InCache;
	}
	else if(pTextureCached != nullptr) *pTextureCached = true;
	return pResult;
}

///////////////////////////////////////////////////////////////////////////////////////

CompositeKey_t EGSDLContext::MaskKeyCreate(EG_SDL_CompositeTextures_e Type)
{
	CompositeKey_t Key;
	// VERY IMPORTANT! Padding between members is uninitialized, so we have to wipe them manually
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_MASK;
	Key.Type = Type;
	return Key;
}

///////////////////////////////////////////////////////////////////////////////////////

int32_t EGSDLContext::NextPowOf2(int32_t Value)
{
	int32_t n = 128;
	while(n < Value && n < 16384) {
		n = n << 1;
	}
	return n;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DumpMasks(SDL_Texture *pTexture, const EGRect *pRect)
{
	int32_t Width = pRect->GetWidth();
  int32_t Height = pRect->GetHeight();
	SDL_assert(Width > 0 && Height > 0);
	SDL_Rect Rect = {0, 0, Width, Height};
	uint8_t *pPixels;
	int Pitch;
	if(SDL_LockTexture(pTexture, &Rect, (void **)&pPixels, &Pitch) != 0) return;

	EG_OPA_t *pBuffer = (EG_OPA_t*)EG_GetBufferMem(Rect.w);
	for(int32_t y = 0; y < Rect.h; y++) {
		EG_SetMemFF(pBuffer, Rect.w);
		int32_t AbsX = (int32_t)pRect->GetX1();
    int32_t AbsY = (int32_t)(y + pRect->GetY1());
    int32_t Length = (int32_t)Rect.w;
		DrawMaskRes_t Result;
		Result = DrawMaskApply(pBuffer, AbsX, AbsY, Length);
		if(Result == EG_DRAW_MASK_RESULT_TRANSP) {
			EG_ZeroMem(&pPixels[y * Pitch], 4 * Rect.w);
		}
		else if(Result == EG_DRAW_MASK_RESULT_FULL_COVER) {
			EG_SetMemFF(&pPixels[y * Pitch], 4 * Rect.w);
		}
		else {
			for(int x = 0; x < Rect.w; x++) {
				const size_t Index = y * Pitch + x * 4;
				pPixels[Index] = pBuffer[x];
				pPixels[Index + 1] = pPixels[Index + 2] = pPixels[Index + 3] = 0xFF;
			}
		}
	}
	EG_ReleaseBufferMem(pBuffer);
	SDL_UnlockTexture(pTexture);
}

#endif
