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

#include "misc/EG_Assert.h"
#include "draw/EG_ImageCache.h"
#include "draw/EG_ImageDecoder.h"
#include "draw/EG_DrawImage.h"
#include "hal/EG_HALTick.h"
#include "misc/EG_Misc.h"

//namespace EG_ImageCache
//{

/////////////////////////////////////////////////////////////////////////////////

#define EG_IMG_CACHE_AGING 1         // Decrement Life with this value on every open
#define EG_IMG_CACHE_LIFE_GAIN 1     // Boost Life by this factor (multiply OpenDelay with this value)
#define EG_IMG_CACHE_LIFE_LIMIT 1000 // Don't let Life be greater than this because it requires a lot of time to "die" from values

#if EG_IMG_CACHE_DEF_SIZE
static bool ImageCacheMatch(const void *pSourceA, const void *pSourceB);
#endif

#if EG_IMG_CACHE_DEF_SIZE
static uint16_t EntryCount;
#endif

/////////////////////////////////////////////////////////////////////////////////

ImageCacheEntry_t* ImageCacheOpen(const void *pSource, EG_Color_t Color, int32_t FrameID)
{
	// Is the image cached?
	ImageCacheEntry_t *pCachedSource = nullptr;

#if EG_IMG_CACHE_DEF_SIZE
	if(EntryCount == 0) {
		EG_LOG_WARN("ImageCacheOpen: the cache size is 0");
		return nullptr;
	}
	ImageCacheEntry_t *pCache = EG_GC_ROOT(ImageCacheArray_t);
	// Decrement all lifes. Make the entries older
	uint16_t i;
	for(i = 0; i < EntryCount; i++) {
		if(pCache[i].Life > INT32_MIN + EG_IMG_CACHE_AGING) {
			pCache[i].Life -= EG_IMG_CACHE_AGING;
		}
	}
	for(i = 0; i < EntryCount; i++) {
		if(Color.full == pCache[i].DecoderDSC.Color.full &&
			 FrameID == pCache[i].DecoderDSC.FrameID &&
			 ImageCacheMatch(pSource, pCache[i].DecoderDSC.pSource)) {
			/* If opened increment its Life.
        Image difficult to open should live longer to keep avoid frequent their recaching.
        Therefore increase `Life` with `OpenDelay`*/
			pCachedSource = &pCache[i];
			pCachedSource->Life += pCachedSource->DecoderDSC.OpenDelay * EG_IMG_CACHE_LIFE_GAIN;
			if(pCachedSource->Life > EG_IMG_CACHE_LIFE_LIMIT) pCachedSource->Life = EG_IMG_CACHE_LIFE_LIMIT;
			EG_LOG_TRACE("image source found in the pCache");
			break;
		}
	}
	// The image is not cached then cache it now
	if(pCachedSource) return pCachedSource;
	// Find an entry to reuse. Select the entry with the least Life
	pCachedSource = &pCache[0];
	for(i = 1; i < EntryCount; i++) {
		if(pCache[i].Life < pCachedSource->Life) {
			pCachedSource = &pCache[i];
		}
	}
	// Close the decoder to reuse it if it was opened (has a valid source)
	if(pCachedSource->DecoderDSC.pSource) {
		pCachedSource->DecoderDSC.Decoder->Close();
		EG_LOG_INFO("image draw: cache miss, close and reuse an entry");
	}
	else {
		EG_LOG_INFO("image draw: cache miss, cached to an empty entry");
	}
#else
	pCachedSource = &EG_GC_ROOT(_lv_img_cache_single);
#endif
	// Open the image and measure the time to open
	uint32_t t_start = EG_GetTick();
	EG_Result_t open_res = pCachedSource->DecoderDSC.pDecoder->Open(&pCachedSource->DecoderDSC, pSource, Color, FrameID);
	if(open_res == EG_RES_INVALID) {
		EG_LOG_WARN("Image draw cannot open the image resource");
		EG_ZeroMem(pCachedSource, sizeof(ImageCacheEntry_t));
		pCachedSource->Life = INT32_MIN; // Make the empty entry very "weak" to force its use
		return nullptr;
	}
	pCachedSource->Life = 0;
	if(pCachedSource->DecoderDSC.OpenDelay == 0) {	// If `OpenDelay` was not set in the open function set it here
		pCachedSource->DecoderDSC.OpenDelay = EG_TickElapse(t_start);
	}
	if(pCachedSource->DecoderDSC.OpenDelay == 0) pCachedSource->DecoderDSC.OpenDelay = 1;
	return pCachedSource;
}

/////////////////////////////////////////////////////////////////////////////////

void SetImageCacheSize(uint16_t NewSlotNumber)
{
#if EG_IMG_CACHE_DEF_SIZE == 0
	EG_UNUSED(NewSlotNumber);
	EG_LOG_WARN("Can't change cache size because it's disabled by EG_IMG_CACHE_DEF_SIZE = 0");
#else
	if(EG_GC_ROOT(ImageCacheArray_t) != nullptr) {
		// Clean the cache before free it
		InvalidateImageCacheSource(nullptr);
		EG_FreeMem(EG_GC_ROOT(ImageCacheArray_t));
	}

	// Reallocate the cache
	EG_GC_ROOT(ImageCacheArray_t) = EG_AllocMem(sizeof(ImageCacheEntry_t) * new_entry_cnt);
	EG_ASSERT_MALLOC(EG_GC_ROOT(ImageCacheArray_t));
	if(EG_GC_ROOT(ImageCacheArray_t) == nullptr) {
		EntryCount = 0;
		return;
	}
	EntryCount = new_entry_cnt;

	// Clean the cache
	EG_ZeroMem(EG_GC_ROOT(ImageCacheArray_t), EntryCount * sizeof(ImageCacheEntry_t));
#endif
}

/////////////////////////////////////////////////////////////////////////////////

void InvalidateImageCacheSource(const void *pSource)
{
	EG_UNUSED(pSource);
#if EG_IMG_CACHE_DEF_SIZE
	ImageCacheEntry_t *pCache = EG_GC_ROOT(ImageCacheArray_t);

	uint16_t i;
	for(i = 0; i < EntryCount; i++) {
		if(pSource == nullptr || ImageCacheMatch(pSource, pCache[i].DecoderDSC.pSource)) {
			if(pCache[i].DecoderDSC.pSource != nullptr) {
				pCache[i].DecoderDSC.Decoder->Close();
			}
			EG_ZeroMem(&pCache[i], sizeof(ImageCacheEntry_t));
		}
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////

#if EG_IMG_CACHE_DEF_SIZE
static bool ImageCacheMatch(const void *pSourceA, const void *pSourceB)
{
	EG_ImageSource_e Type = ((EGDrawImage*)pSourceA)->GetType();
	if(Type == EG_IMG_SRC_VARIABLE)	return pSourceA == pSourceB;
	if(Type != EG_IMG_SRC_FILE)	return false;
	if(((EGDrawImage*)pSourceB)->GetType() != EG_IMG_SRC_FILE) return false;
	return strcmp(pSourceA, pSourceB) == 0;
}
#endif
//}