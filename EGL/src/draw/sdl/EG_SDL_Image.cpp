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

#include "draw/EG_DrawImage.h"
#include "draw/EG_ImageCache.h"
#include "draw/EG_DrawMask.h"

#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGSDLContext::DrawImage(EGDrawImage *pDrawImage, const EGRect *pRect, const void *pSourceBuffer)
{
size_t KeySize;

  EGSDLContext *pDC = (EGSDLContext*)pDrawImage->m_pContext;
  const EGRect *pClip = pDC->m_pClipRect;
  EG_CacheKeyHeadImage_t *Key = CreateTextureImageKey(pSourceBuffer, pDrawImage->m_FrameID, &KeySize);
	bool TextureFound = false;
	EG_SDL_ImageHeader_t *pHeader = nullptr;
	SDL_Texture *pTexture = TextureCacheGetWithExtData(Key, KeySize, &TextureFound, (void **)&pHeader);
	bool InCache = false;
	if(!TextureFound) pDC->LoadImageTexture(Key, KeySize, pSourceBuffer, pDrawImage->m_FrameID, &pTexture, &pHeader, &InCache);
	else InCache = true;
	SDL_free(Key);
	if(!pTexture || !pHeader) return EG_RES_INVALID;
	EGRect ScaledRect;
	EGImageBuffer::GetTransformedRect(&ScaledRect, pRect->GetWidth(), pRect->GetHeight(), 0, pDrawImage->m_Scale, &pDrawImage->m_Pivot);
	ScaledRect.Move(pRect->GetX1(), pRect->GetY1());
	int32_t Radius = 0;
	EGRect TempRect = ScaledRect;
  EGRect TempClip = *pClip;
  EGRect ApplyRect;	// pRect will be translated so pRect will start at (0,0)
	bool HasComposite = false;
	if(!pDC->CheckMaskSimpleRadius(&TempRect, &Radius)) {
		HasComposite = pDC->CompositeBegin(&ScaledRect, pClip, nullptr, pDrawImage->m_BlendMode,	&TempRect, &TempClip, &ApplyRect);
	}
	pDC->TransformAreasOffset(HasComposite, &ApplyRect, &TempRect, &TempClip);
	SDL_Rect ClipRect, DrawRect;
	RectToSDLRect(&TempClip, &ClipRect);
	RectToSDLRect(&TempRect, &DrawRect);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	if(Radius > 0) pDC->DrawImageRounded(pTexture, pHeader, pDrawImage, &TempRect, &TempClip, Radius);
	else pDC->DrawImageSimple(pTexture, pHeader, pDrawImage, &TempRect, &TempClip);
	pDC->CompositeEnd(&ApplyRect, pDrawImage->m_BlendMode);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		if(!pHeader->Managed) {
			SDL_DestroyTexture(pTexture);
		}
		EG_FreeMem(pHeader);
	}
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::CalcDrawPart(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader, const EGRect *pRect,
  const EGRect *pClip, SDL_Rect *pClippedSrce, SDL_Rect *pClippedDest)
{
double x = 0, y = 0, Width, Height;

	if(SDL_RectEmpty(&pHeader->Rect)) {
		Uint32 Format = 0;
		int Access = 0, TempWidth, TempHeight;
		SDL_QueryTexture(pTexture, &Format, &Access, &TempWidth, &TempHeight);
		Width = TempWidth;
		Height = TempHeight;
	}
	else {
		x = pHeader->Rect.x;
		y = pHeader->Rect.y;
		Width = pHeader->Rect.w;
		Height = pHeader->Rect.h;
	}
	if(pClip) {
		EGRect ClippedRect;
		ClippedRect.Intersect(pRect, pClip);
		RectToSDLRect(&ClippedRect, pClippedDest);
	}
	else RectToSDLRect(pRect, pClippedDest);
	int32_t RectWidth = pRect->GetWidth();
  int32_t RectHeight = pRect->GetHeight();
	pClippedSrce->x = (int)(x + (pClippedDest->x - pRect->GetX1()) * Width / RectWidth);
	pClippedSrce->y = (int)(y + (pClippedDest->y - pRect->GetY1()) * Height / RectHeight);
	pClippedSrce->w = (int)(Width - (RectWidth - pClippedDest->w) * Width / RectWidth);
	pClippedSrce->h = (int)(Height - (RectHeight - pClippedDest->h) * Height / RectHeight);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSDLContext::LoadImageTexture(EG_CacheKeyHeadImage_t *pKey, size_t KeySize, const void *pSrce,
  int32_t FrameID, SDL_Texture **ppTexture, EG_SDL_ImageHeader_t **ppHeader, bool *pInCache)
{
	ImageCacheEntry_t *pCachedImage = ImageCacheOpen(pSrce, EG_ColorWhite(), FrameID);
	EG_SDL_CacheFlag_e Flags = EG_DRAW_SDL_CACHE_FLAG_NONE;
	SDL_Rect Rect;
	SDL_memset(&Rect, 0, sizeof(SDL_Rect));
	if(pCachedImage) {
		EGImageDecoder *pDecoder = pCachedImage->DecoderDSC.pDecoder;
		if(pDecoder->m_pExtData && SDL_memcmp(pDecoder->m_pExtData, EG_DRAW_SDL_DEC_DSC_TEXTURE_HEAD, 8) == 0) {
			EG_SDL_Dec_ExtData_t *pExtData = (EG_SDL_Dec_ExtData_t *)pDecoder->m_pExtData;
			*ppTexture = pExtData->pTexture;
			Rect = pExtData->Rect;
			if(pExtData->TextureManaged) {
				Flags = (EG_SDL_CacheFlag_e)(Flags | EG_DRAW_SDL_CACHE_FLAG_MANAGED); // probably no point in doing this
			}
			pExtData->TextureReferenced = true;
		}
		else *ppTexture = UploadImageTexture(&pCachedImage->DecoderDSC);
#if EG_IMAGE_CACHE_DEF_SIZE == 0
    pDecoder->Close(&pCachedImage->DecoderDSC);
#endif
	}
	if(ppTexture && pCachedImage) {
		*ppHeader = (EG_SDL_ImageHeader_t*)EG_AllocMem(sizeof(EG_SDL_ImageHeader_t));
		SDL_memcpy(&(*ppHeader)->Base, &pCachedImage->DecoderDSC.Header, sizeof(EG_ImageHeader_t));
		(*ppHeader)->Rect = Rect;
		(*ppHeader)->Managed = (Flags & EG_DRAW_SDL_CACHE_FLAG_MANAGED) != 0;
		*pInCache = TextureCachePutAdvanced(pKey, KeySize, *ppTexture, *ppHeader, SDL_free, Flags);
		return true;
	}
	else {
		*pInCache = TextureCachePut(pKey, KeySize, nullptr);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::UploadImageTexture(ImageDecoderDescriptor_t *pDecoderDSC)
{
	if(!pDecoderDSC->pImageData) {
		return UploadImageTextureFallback(pDecoderDSC);
	}
	bool chroma_keyed = pDecoderDSC->Header.ColorFormat == (uint32_t)EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED;
	int Height = (int)pDecoderDSC->Header.Height;
	int Width = (int)pDecoderDSC->Header.Width;
	void *pData = (void *)pDecoderDSC->pImageData;
	Uint32 rmask = 0x00FF0000;
	Uint32 gmask = 0x0000FF00;
	Uint32 bmask = 0x000000FF;
	Uint32 amask = 0xFF000000;
	if(chroma_keyed) amask = 0x00;
	SDL_Surface *pSurface = SDL_CreateRGBSurfaceFrom(pData, Width, Height, EG_COLOR_DEPTH, Width * EG_COLOR_DEPTH / 8,	rmask, gmask, bmask, amask);
	SDL_SetColorKey(pSurface, chroma_keyed, EG_ColorTo32(EG_COLOR_CHROMA_KEY));
	SDL_Texture *pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
	SDL_FreeSurface(pSurface);
	return pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::UploadImageTextureFallback(ImageDecoderDescriptor_t *pDecoderDSC)
{
	int32_t Height = (int32_t)pDecoderDSC->Header.Height;
	int32_t Width = (int32_t)pDecoderDSC->Header.Width;
	uint8_t *pData = (uint8_t*)EG_GetBufferMem(Width * Height * sizeof(EG_Color_t));
	for(int32_t y = 0; y < Height; y++) {
		pDecoderDSC->pDecoder->ReadLine(pDecoderDSC, 0, y, Width, &pData[y * Width * sizeof(EG_Color_t)]);
	}
	Uint32 rmask = 0x00FF0000;
	Uint32 gmask = 0x0000FF00;
	Uint32 bmask = 0x000000FF;
	Uint32 amask = 0xFF000000;
	SDL_Surface *pSurface = SDL_CreateRGBSurfaceFrom(pData, Width, Height, EG_COLOR_DEPTH, Width * EG_COLOR_DEPTH / 8, rmask, gmask, bmask, amask);
	SDL_SetColorKey(pSurface, SDL_TRUE, EG_ColorTo32(EG_COLOR_CHROMA_KEY));
	SDL_Texture *pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
	SDL_FreeSurface(pSurface);
	EG_ReleaseBufferMem(pData);
	return pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

// Check if there is only one Radius mask
bool EGSDLContext::CheckMaskSimpleRadius(const EGRect *pRect, int32_t *pRadius)
{
	if(DrawMaskGetCount() != 1) return false;
	for(uint8_t i = 0; i < _EG_MASK_MAX_NUM; i++) {
		MaskCommonDiscrpt_t *pMask = (MaskCommonDiscrpt_t*)EG_GC_ROOT(EG_DrawMaskArray[i]).pMask;
		if(pMask->Type == EG_DRAW_MASK_TYPE_RADIUS) {
			MaskRadiusParam_t *pRadiusMask = (MaskRadiusParam_t *)pMask;
			if(pRadiusMask->Radius.Outer) return false;
			if(!pRadiusMask->Radius.Area.IsEqualTo(pRect)) return false;
			*pRadius = pRadiusMask->Radius.Radius;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawImageSimple(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader, const EGDrawImage *pDrawImage, 
                                  const EGRect *pRect, const EGRect *pClip)
{
	ApplyRecolorOPA(pTexture, pDrawImage);
	SDL_Point Pivot = {.x = pDrawImage->m_Pivot.m_X, .y = pDrawImage->m_Pivot.m_Y};
	if(pDrawImage->m_Angle != 0) { //Image needs to be rotated, so we have to use pClip rect which is slower
		SDL_Rect ClipRect;		// No Radius, set pClip here
		RectToSDLRect(pClip, &ClipRect);
		SDL_RenderSetClipRect(m_pRenderer, &ClipRect);
	}
	SDL_Rect SrceRect, DestRect;
	CalcDrawPart(pTexture, pHeader, pRect, pClip, &SrceRect, &DestRect);
	SDL_RenderCopyEx(m_pRenderer, pTexture, &SrceRect, &DestRect, pDrawImage->m_Angle, &Pivot, SDL_FLIP_NONE);
	if(pDrawImage->m_Angle != 0) {
		SDL_RenderSetClipRect(m_pRenderer, nullptr);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawImageRounded(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader,
  const EGDrawImage *pDrawImage, const EGRect *pRect, const EGRect *pClip, int32_t Radius)
{
	const int Width = pRect->GetWidth(), Height = pRect->GetHeight();
	int32_t RealRadius = EG_MIN3(Radius, Width, Height);
	bool InCache = false;
	SDL_Texture *pFrag = GetImageRoundedFrag(pTexture, pHeader, Width, Height, RealRadius, &InCache);
	ApplyRecolorOPA(pFrag, pDrawImage);
	RectBackgroundFragDrawCorners(pFrag, RealRadius, pRect, pClip, true);
	ApplyRecolorOPA(pTexture, pDrawImage);
	SDL_Rect SrceRect, DestRect;
	// Draw 3 PartRects 
	EGRect TempClip, PartRect;
	CalcDrawPart(pTexture, pHeader, pRect, nullptr, &SrceRect, &DestRect);
	for(int i = Width > Height ? ROUNDED_IMAGE_PART_LEFT : ROUNDED_IMAGE_PART_TOP, j = i + 3; i <= j; i++) {
		switch(i) {
			case ROUNDED_IMAGE_PART_LEFT:
        PartRect.Set(pRect->GetX1(), pRect->GetY1() + Radius, pRect->GetX1() + Radius - 1, pRect->GetY2() - Radius);
				break;
			case ROUNDED_IMAGE_PART_HCENTER:
        PartRect.Set(pRect->GetX1() + Radius, pRect->GetY1(), pRect->GetX2() - Radius, pRect->GetY2());
				break;
			case ROUNDED_IMAGE_PART_RIGHT:
        PartRect.Set(pRect->GetX2() - Radius + 1, pRect->GetY1() + Radius, pRect->GetX2(), pRect->GetY2() - Radius);
				break;
			case ROUNDED_IMAGE_PART_TOP:
        PartRect.Set(pRect->GetX1() + Radius, pRect->GetY1(), pRect->GetX2() - Radius, pRect->GetY1() + Radius - 1);
				break;
			case ROUNDED_IMAGE_PART_VCENTER:
        PartRect.Set(pRect->GetX1() + Radius, pRect->GetY2() - Radius + 1, pRect->GetX2() - Radius, pRect->GetY2());
				break;
			case ROUNDED_IMAGE_PART_BOTTOM:
        PartRect.Set(pRect->GetX1(), pRect->GetY1() + Radius, pRect->GetX2(), pRect->GetY2() - Radius);
				break;
			default:
				break;
		}
		if(!TempClip.Intersect(&PartRect, pClip)) continue;
		SDL_Rect ClipRect;
		RectToSDLRect(&TempClip, &ClipRect);
		SDL_RenderSetClipRect(m_pRenderer, &ClipRect);
		SDL_RenderCopy(m_pRenderer, pTexture, &SrceRect, &DestRect);
	}
	SDL_RenderSetClipRect(m_pRenderer, nullptr);

	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pFrag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::ApplyRecolorOPA(SDL_Texture *pTexture, const EGDrawImage *pDrawImage)
{
	if(pDrawImage->m_RecolorOPA > EG_OPA_TRANSP) {
		// Draw with mixed recolor 
		EG_Color_t Recolor = EG_ColorMix(pDrawImage->m_Recolor, EG_ColorWhite(), pDrawImage->m_RecolorOPA);
		SDL_SetTextureColorMod(pTexture, Recolor.ch.red, Recolor.ch.green, Recolor.ch.blue);
	}
	else {		// Draw with no recolor 
		SDL_SetTextureColorMod(pTexture, 0xFF, 0xFF, 0xFF);
	}
	SDL_SetTextureAlphaMod(pTexture, pDrawImage->m_OPA);
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::GetImageRoundedFrag(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader, int Width, int Height, int32_t Radius, bool *pInCache)
{
	EG_ImageRoundedKey_t Key = CreateRoundedKey(pTexture, Width, Height, Radius);
	bool InCache = false;
	SDL_Texture *pMaskFrag = RectBackgroundGetFrag(Radius, &InCache);
	SDL_Texture *pImageFrag = TextureCacheGet(&Key, sizeof(Key), nullptr);
	if(pImageFrag == nullptr) {
		const int32_t FullFragSize = Radius * 2 + 3;
		pImageFrag = SDL_CreateTexture(m_pRenderer, EG_DRAW_SDL_TEXTURE_FORMAT, SDL_TEXTUREACCESS_TARGET, FullFragSize, FullFragSize);
		SDL_assert(pImageFrag);
		SDL_SetTextureBlendMode(pImageFrag, SDL_BLENDMODE_BLEND);
		SDL_Texture *old_target = SDL_GetRenderTarget(m_pRenderer);
		SDL_SetRenderTarget(m_pRenderer, pImageFrag);
		SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 0);
		// SDL_RenderClear is not working properly, so we overwrite the target with solid color 
		SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
		SDL_RenderFillRect(m_pRenderer, nullptr);
		SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);

		EGRect pRect(0, 0, Width - 1, Height - 1);
		EGRect FragRect(0, 0, FullFragSize - 1, FullFragSize - 1);
		RectBackgroundFragDrawCorners(pMaskFrag, Radius, &FragRect, nullptr, false);

		SDL_SetTextureAlphaMod(pTexture, 0xFF);
		SDL_SetTextureColorMod(pTexture, 0xFF, 0xFF, 0xFF);
#if EG_GPU_SDL_CUSTOM_BLEND_MODE
		SDL_BlendMode BlendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ZERO,
																													SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_DST_ALPHA,
																													SDL_BLENDFACTOR_ZERO, SDL_BLENDOPERATION_ADD);
		SDL_SetTextureBlendMode(pTexture, BlendMode);
#else
		SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_MOD);
#endif
		SDL_Rect SrceRect, ClipRect, DestRect = {0, 0, Radius, Radius};
		ClipRect.w = ClipRect.h = Radius;
		for(int i = 0; i <= ROUNDED_IMAGE_CORNER_BOTTOM_LEFT; i++) {
			switch(i) {
				case ROUNDED_IMAGE_CORNER_TOP_LEFT:
					ClipRect.x = 0;
					ClipRect.y = 0;
					FragRect.Align(&pRect, EG_ALIGN_TOP_LEFT, 0, 0);
					break;
				case ROUNDED_IMAGE_CORNER_TOP_RIGHT:
					ClipRect.x = FullFragSize - Radius;
					ClipRect.y = 0;
					FragRect.Align(&pRect, EG_ALIGN_TOP_RIGHT, 0, 0);
					break;
				case ROUNDED_IMAGE_CORNER_BOTTOM_RIGHT:
					ClipRect.x = FullFragSize - Radius;
					ClipRect.y = FullFragSize - Radius;
					FragRect.Align(&pRect, EG_ALIGN_BOTTOM_RIGHT, 0, 0);
					break;
				case ROUNDED_IMAGE_CORNER_BOTTOM_LEFT:
					ClipRect.x = 0;
					ClipRect.y = FullFragSize - Radius;
					FragRect.Align(&pRect, EG_ALIGN_BOTTOM_LEFT, 0, 0);
					break;
				default:
					break;
			}
			CalcDrawPart(pTexture, pHeader, &pRect, nullptr, &SrceRect, &DestRect);
			SDL_RenderSetClipRect(m_pRenderer, &ClipRect);
			SDL_RenderCopy(m_pRenderer, pTexture, &SrceRect, &DestRect);
		}
		SDL_RenderSetClipRect(m_pRenderer, nullptr);
		SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(m_pRenderer, old_target);
		*pInCache = TextureCachePut(&Key, sizeof(Key), pImageFrag);
	}
	else {
		*pInCache = true;
	}
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pMaskFrag);
	}
	return pImageFrag;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ImageRoundedKey_t EGSDLContext::CreateRoundedKey(const SDL_Texture *pTexture, int32_t Width, int32_t Height, int32_t Radius)
{
EG_ImageRoundedKey_t Key;

	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_IMAGE_ROUNDED_CORNERS;
	Key.pTexture = pTexture;
	Key.Width = Width;
	Key.Height = Height;
	Key.Radius = Radius;
	return Key;
}

#endif
