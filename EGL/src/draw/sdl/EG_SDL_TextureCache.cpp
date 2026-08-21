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

typedef struct {
	EG_SDL_CacheKeyMagic_e Magic;
} temp_texture_key_t;

typedef struct {
	int32_t Width;
	int32_t Height;
} temp_texture_userdata_t;

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::TextureCacheInit(void)
{
	m_pTextureCache = EG_LRUCreate(EG_GPU_SDL_LRU_SIZE, 65536,	(EG_LRU_Free_t*)DrawCacheFreeValue, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::TextureCacheDeinit(void)
{
	EG_LRUDelete(m_pTextureCache);
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::TextureCacheGet(const void *pKey, size_t KeyLength, bool *pFound)
{
	return TextureCacheGetWithExtData(pKey, KeyLength, pFound, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::TextureCacheGetWithExtData(const void *pKey, size_t KeyLength, bool *pFound, void **ppExtData)
{
	DrawCacheValue_t *pValue = DrawCacheGetEntry(pKey, KeyLength, pFound);
	if(!pValue) return nullptr;
	if(ppExtData) {
		*ppExtData = pValue->pExtData;
	}
	return pValue->pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSDLContext::TextureCachePut(const void *pKey, size_t KeyLength, SDL_Texture *pTexture)
{
	return TextureCachePutAdvanced(pKey, KeyLength, pTexture, nullptr, nullptr, EG_DRAW_SDL_CACHE_FLAG_NONE);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSDLContext::TextureCachePutAdvanced(const void *pKey, size_t KeyLength, SDL_Texture *pTexture, void *pExtData,
           void ExtDataFree(void *), EG_SDL_CacheFlag_e Flags)
{
	EG_LRU_t *pLRU = m_pTextureCache;
	DrawCacheValue_t *pValue = (DrawCacheValue_t*)SDL_malloc(sizeof(DrawCacheValue_t));
	pValue->pTexture = pTexture;
	pValue->pExtData = pExtData;
	pValue->pExtDataFree = ExtDataFree;
	pValue->Flags = Flags;
	if(!pTexture) return EG_LRUSet(pLRU, pKey, KeyLength, pValue, 1) == EG_LRU_OK;
	if(Flags & EG_DRAW_SDL_CACHE_FLAG_MANAGED) {
		// Managed pTexture doesn't count into cache size 
		EG_LOG_INFO("cache pTexture %p", pTexture);
		return EG_LRUSet(pLRU, pKey, KeyLength, pValue, 1) == EG_LRU_OK;
	}
	Uint32 Format;
	int access, width, height;
	if(SDL_QueryTexture(pTexture, &Format, &access, &width, &height) != 0) {
		return false;
	}
	EG_LOG_INFO("cache pTexture %p, %d*%d@%dbpp", pTexture, width, height, SDL_BITSPERPIXEL(Format));
	return EG_LRUSet(pLRU, pKey, KeyLength, pValue, width * height * SDL_BITSPERPIXEL(Format) / 8) == EG_LRU_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_CacheKeyHeadImage_t* EGSDLContext::CreateTextureImageKey(const void *pSrce, int32_t FrameID, size_t *pSize)
{
	EG_CacheKeyHeadImage_t Header;
	// VERY IMPORTANT! Padding between members is uninitialized, so we have to wipe them manually
	SDL_memset(&Header, 0, sizeof(Header));
	Header.Magic = EG_GPU_CACHE_KEY_MAGIC_IMG;
	Header.Type = EGDrawImage::GetType(pSrce);
	Header.FrameID = FrameID;
	void *pKey;
	size_t KeySize;
	if(Header.Type == EG_IMG_SRC_FILE || Header.Type == EG_IMG_SRC_SYMBOL) {
		size_t SrceLength = SDL_strlen((char*)pSrce);
		KeySize = sizeof(Header) + SrceLength;
		pKey = SDL_malloc(KeySize);
		SDL_memcpy(pKey, &Header, sizeof(Header));
		SDL_memcpy((void*)((char*)pKey + sizeof(Header)), pSrce, SrceLength);		// Copy string content as pKey pValue
	}
	else {
		KeySize = sizeof(Header) + sizeof(void *);
		pKey = SDL_malloc(KeySize);
		SDL_memcpy(pKey, &Header, sizeof(Header));
		SDL_memcpy((void*)((char*)pKey + sizeof(Header)), &pSrce, sizeof(void *));	// Copy address number as pKey pValue
	}
	*pSize = KeySize;
	return (EG_CacheKeyHeadImage_t *)pKey;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawCacheFreeValue(DrawCacheValue_t *pValue)
{
	if(pValue->pTexture && !(pValue->Flags & EG_DRAW_SDL_CACHE_FLAG_MANAGED)) {
		EG_LOG_INFO("destroy pTexture %p", pValue->pTexture);
		SDL_DestroyTexture(pValue->pTexture);
	}
	if(pValue->pExtDataFree) pValue->pExtDataFree(pValue->pExtData);
	SDL_free(pValue);
}

///////////////////////////////////////////////////////////////////////////////////////

DrawCacheValue_t* EGSDLContext::DrawCacheGetEntry(const void *pKey, size_t KeyLength, bool *pFound)
{
	EG_LRU_t *pLRU = m_pTextureCache;
	DrawCacheValue_t *pValue = nullptr;
	EG_LRUGet(pLRU, pKey, KeyLength, (void **)&pValue);
	if(!pValue) {
		if(pFound) *pFound = false;
		return nullptr;
	}
	if(pFound) *pFound = true;
	return pValue;
}

#endif
