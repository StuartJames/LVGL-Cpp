/*
 *                LEGL 2025-2026 HydraSystems.
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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "draw/sw/EG_DrawSoftGradient.h"
#include "misc/lv_gc.h"
#include "misc/EG_Types.h"

/////////////////////////////////////////////////////////////////////////////////

#if _DITHER_GRADIENT
#define GRAD_CM(r, g, b) EG_COLOR_MAKE32(r, g, b)
#define GRAD_CONV(t, x) t.full = EG_ColorTo32(x)
#else
#define GRAD_CM(r, g, b) EG_COLOR_MAKE(r, g, b)
#define GRAD_CONV(t, x) t = x
#endif

#undef ALIGN
#if defined(EG_ARCH_64)
#define ALIGN(X) (((X) + 7) & ~7)
#else
#define ALIGN(X) (((X) + 3) & ~3)
#endif

#if EG_GRAD_CACHE_DEF_SIZE != 0 && EG_GRAD_CACHE_DEF_SIZE < 256
#error "EG_GRAD_CACHE_DEF_SIZE is too small"
#endif

/////////////////////////////////////////////////////////////////////////////////

static EG_GradCacheItem_t* NextCached(EG_GradCacheItem_t *pItem);

typedef EG_Result_t (*op_cache_t)(EG_GradCacheItem_t *pGradItem, void *pContext);
static EG_Result_t IterateCache(op_cache_t Func, void *pContext, EG_GradCacheItem_t **ppOut);
static size_t GetGradientCacheSize(EG_GradCacheItem_t *pGradItem);
static EG_GradCacheItem_t* AllocateGradient(const EG_GradDescriptor_t *pGradient, EG_Coord_t Width, EG_Coord_t Height);
static EG_Result_t CompareGradientLife(EG_GradCacheItem_t *pGradItem, void *pContext);
static EG_Result_t RemoveOldestGradient(EG_GradCacheItem_t *pGradItem, void *pContext);
static EG_Result_t FindGradient(EG_GradCacheItem_t *pGradItem, void *pContext);
static void FreeGradient(EG_GradCacheItem_t *pGradItem);
static uint32_t ComputeKey(const EG_GradDescriptor_t *pGradient, EG_Coord_t Width, EG_Coord_t Height);

static size_t CacheSize = 0;
static uint8_t *pCacheEnd = 0;

/*union PtrMod {
	const void *pPtr;
	const uint32_t value;
};*/

/////////////////////////////////////////////////////////////////////////////////

static uint32_t ComputeKey(const EG_GradDescriptor_t *pGradient, EG_Coord_t Size, EG_Coord_t Width)
{
// union PtrMod v;
//	v.pPtr = pGradient;

	return ((uint32_t)pGradient ^ Size ^ (Width >> 1)); // Yes, this is correct, it's like a hash that changes if the width changes
}

/////////////////////////////////////////////////////////////////////////////////

static size_t GetGradientCacheSize(EG_GradCacheItem_t *pGradItem)
{
	size_t Size = ALIGN(sizeof(*pGradItem)) + ALIGN(pGradItem->AllocatedSize * sizeof(EG_Color_t));
#if _DITHER_GRADIENT
	Size += ALIGN(pGradItem->Size * sizeof(EG_Color32_t));
#if EG_DITHER_ERROR_DIFFUSION == 1
	Size += ALIGN(pGradItem->Width * sizeof(EG_SColor24_t));
#endif
#endif
	return Size;
}

/////////////////////////////////////////////////////////////////////////////////

static EG_GradCacheItem_t* NextCached(EG_GradCacheItem_t *pGradItem)
{
	if(CacheSize == 0) return nullptr;
	if(pGradItem == nullptr) return (EG_GradCacheItem_t *)EG_GC_ROOT(_lv_grad_cache_mem);
	size_t Size = GetGradientCacheSize(pGradItem);
	// Compute the Size for this cache item
	if((uint8_t *)pGradItem + Size >= pCacheEnd) return nullptr;
	else return (EG_GradCacheItem_t *)((uint8_t *)pGradItem + Size);
}

/////////////////////////////////////////////////////////////////////////////////

static EG_Result_t IterateCache(op_cache_t Func, void *pContext, EG_GradCacheItem_t **ppOut)
{
	EG_GradCacheItem_t *pGrad = NextCached(NULL);
	while(pGrad != nullptr && pGrad->Life) {
		if((*Func)(pGrad, pContext) == EG_RES_OK) {
			if(ppOut != NULL) *ppOut = pGrad;
			return EG_RES_OK;
		}
		pGrad = NextCached(pGrad);
	}
	return EG_RES_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////

static EG_Result_t CompareGradientLife(EG_GradCacheItem_t *pGradItem, void *pLife)
{
	uint32_t *pMinLife = (uint32_t *)pLife;
	if(pGradItem->Life < *pMinLife) *pMinLife = pGradItem->Life;
	return EG_RES_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////

static void FreeGradient(EG_GradCacheItem_t *pGradItem)
{
	size_t Size = GetGradientCacheSize(pGradItem);
	size_t next_items_size = (size_t)(pCacheEnd - (uint8_t *)pGradItem) - Size;
	pCacheEnd -= Size;
	if(next_items_size) {
		uint8_t *old = (uint8_t *)pGradItem;
		EG_CopyMem(pGradItem, ((uint8_t *)pGradItem) + Size, next_items_size);
		//  Then need to fix all internal pointers too 
		while((uint8_t *)pGradItem != pCacheEnd) {
			pGradItem->pMap = (EG_Color_t *)(((uint8_t *)pGradItem->pMap) - Size);
#if _DITHER_GRADIENT
			pGradItem->pHighMap = (EG_Color32_t *)(((uint8_t *)pGradItem->pHighMap) - Size);
#if EG_DITHER_ERROR_DIFFUSION == 1
			pGradItem->pError = (EG_SColor24_t *)(((uint8_t *)pGradItem->pError) - Size);
#endif
#endif
			pGradItem = (EG_GradCacheItem_t *)(((uint8_t *)pGradItem) + GetGradientCacheSize(pGradItem));
		}
		EG_ZeroMem(old + next_items_size, Size);
	}
}

/////////////////////////////////////////////////////////////////////////////////

static EG_Result_t RemoveOldestGradient(EG_GradCacheItem_t *pGradItem, void *pContext)
{
	uint32_t *min_life = (uint32_t *)pContext;
	if(pGradItem->Life == *min_life) {
		// Found, let's kill it
		FreeGradient(pGradItem);
		return EG_RES_OK;
	}
	return EG_RES_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////

static EG_Result_t FindGradient(EG_GradCacheItem_t *pGradItem, void *pContext)
{
	uint32_t *k = (uint32_t *)pContext;
	if(pGradItem->Key == *k) return EG_RES_OK;
	return EG_RES_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////

static EG_GradCacheItem_t* AllocateGradient(const EG_GradDescriptor_t *pGradItem, EG_Coord_t Width, EG_Coord_t Height)
{
	EG_Coord_t Size = pGradItem->dir == EG_GRAD_DIR_HOR ? Width : Height;
 //  The pMap is being used horizontally (width) unless no dithering is selected where it's used vertically 
 	EG_Coord_t map_size = EG_MAX(Width, Height);
	size_t req_size = ALIGN(sizeof(EG_GradCacheItem_t)) + ALIGN(map_size * sizeof(EG_Color_t));
#if _DITHER_GRADIENT
	req_size += ALIGN(Size * sizeof(EG_Color32_t));
#if EG_DITHER_ERROR_DIFFUSION == 1
	req_size += ALIGN(Width * sizeof(EG_SColor24_t));
#endif
#endif

	size_t act_size = (size_t)(pCacheEnd - EG_GC_ROOT(_lv_grad_cache_mem));
	EG_GradCacheItem_t *pItem = NULL;
	if(req_size + act_size < CacheSize) {
		pItem = (EG_GradCacheItem_t *)pCacheEnd;
		pItem->UnCached = 0;
	}
	else {
		// Need to evict items from cache until we find enough space to allocate this one 
		if(req_size <= CacheSize) {
			while(act_size + req_size > CacheSize) {
				uint32_t oldest_life = UINT32_MAX;
				IterateCache(&CompareGradientLife, &oldest_life, NULL);
				IterateCache(&RemoveOldestGradient, &oldest_life, NULL);
				act_size = (size_t)(pCacheEnd - EG_GC_ROOT(_lv_grad_cache_mem));
			}
			pItem = (EG_GradCacheItem_t *)pCacheEnd;
			pItem->UnCached = 0;
		}
		else {
			// The cache is too small. Allocate the item manually and free it later.
			pItem = (EG_GradCacheItem_t *)EG_AllocMem(req_size);
			EG_ASSERT_MALLOC(pItem);
			if(pItem == NULL) return NULL;
			pItem->UnCached = 1;
		}
	}

	pItem->Key = ComputeKey(pGradItem, Size, Width);
	pItem->Life = 1;
	pItem->Filled = 0;
	pItem->AllocatedSize = map_size;
	pItem->Size = Size;
	if(pItem->UnCached) {
		uint8_t *p = (uint8_t *)pItem;
		pItem->pMap = (EG_Color_t *)(p + ALIGN(sizeof(*pItem)));
#if _DITHER_GRADIENT
		pItem->hmap = (EG_Color32_t *)(p + ALIGN(sizeof(*pItem)) + ALIGN(map_size * sizeof(EG_Color_t)));
#if EG_DITHER_ERROR_DIFFUSION == 1
		pItem->error_acc = (EG_SColor24_t *)(p + ALIGN(sizeof(*pItem)) + ALIGN(Size * sizeof(EG_GradientColor_t)) +
																				ALIGN(map_size * sizeof(EG_Color_t)));
		pItem->Width = Width;
#endif
#endif
	}
	else {
		pItem->pMap = (EG_Color_t *)(pCacheEnd + ALIGN(sizeof(*pItem)));
#if _DITHER_GRADIENT
		pItem->hmap = (EG_Color32_t *)(pCacheEnd + ALIGN(sizeof(*pItem)) + ALIGN(map_size * sizeof(EG_Color_t)));
#if EG_DITHER_ERROR_DIFFUSION == 1
		pItem->error_acc = (EG_SColor24_t *)(pCacheEnd + ALIGN(sizeof(*pItem)) + ALIGN(Size * sizeof(EG_GradientColor_t)) +
																				ALIGN(map_size * sizeof(EG_Color_t)));
		pItem->Width = Width;
#endif
#endif
		pCacheEnd += req_size;
	}
	return pItem;
}

/////////////////////////////////////////////////////////////////////////////////

void EG_GradientFreeCache(void)
{
	EG_FreeMem(EG_GC_ROOT(_lv_grad_cache_mem));
	EG_GC_ROOT(_lv_grad_cache_mem) = pCacheEnd = NULL;
	CacheSize = 0;
}

/////////////////////////////////////////////////////////////////////////////////

void EG_GradientSetCacheSize(size_t max_bytes)
{
	EG_FreeMem(EG_GC_ROOT(_lv_grad_cache_mem));
	pCacheEnd = (uint8_t *)EG_AllocMem(max_bytes);
	EG_GC_ROOT(_lv_grad_cache_mem) = (uint8_t *)EG_AllocMem(max_bytes);
	EG_ASSERT_MALLOC(EG_GC_ROOT(_lv_grad_cache_mem));
	EG_ZeroMem(EG_GC_ROOT(_lv_grad_cache_mem), max_bytes);
	CacheSize = max_bytes;
}

/////////////////////////////////////////////////////////////////////////////////

EG_GradCacheItem_t* EG_GetGradient(const EG_GradDescriptor_t *pGradient, EG_Coord_t Width, EG_Coord_t Height)
{
	//  No gradient, no cache 
	if(pGradient->dir == EG_GRAD_DIR_NONE) return nullptr;
	//  Step 0: Check if the cache exist (else create it) 
	static bool inited = false;
	if(!inited) {
		EG_GradientSetCacheSize(EG_GRAD_CACHE_DEF_SIZE);
		inited = true;
	}
	//  Step 1: Search cache for the given Key 
	EG_Coord_t Size = pGradient->dir == EG_GRAD_DIR_HOR ? Width : Height;
	uint32_t Key = ComputeKey(pGradient, Size, Width);
	EG_GradCacheItem_t *pItem = nullptr;
	if(IterateCache(&FindGradient, &Key, &pItem) == EG_RES_OK) {
		pItem->Life++; //  Don't forget to bump the counter 
		return pItem;
	}
	//  Step 2: Need to allocate an item for it 
	pItem = AllocateGradient(pGradient, Width, Height);
	if(pItem == nullptr) {
		EG_LOG_WARN("Faild to allcoate item for the gradient");
		return pItem;
	}
//  Step 3: Fill it with the gradient, as expected 
#if _DITHER_GRADIENT
	for(EG_Coord_t i = 0; i < pItem->Size; i++) {
		pItem->hmap[i] = EG_GradientCalculate(g, pItem->Size, i);
	}
#if EG_DITHER_ERROR_DIFFUSION == 1
	EG_ZeroMem(pItem->error_acc, Width * sizeof(EG_SColor24_t));
#endif
#else
	for(EG_Coord_t i = 0; i < pItem->Size; i++) {
		pItem->pMap[i] = EG_GradientCalculate(pGradient, pItem->Size, i);
	}
#endif
	return pItem;
}

/////////////////////////////////////////////////////////////////////////////////

EG_GradientColor_t EG_ATTRIBUTE_FAST_MEM EG_GradientCalculate(const EG_GradDescriptor_t *dsc, EG_Coord_t range, EG_Coord_t frac)
{
	EG_GradientColor_t tmp;
	EG_Color32_t one, two;
	// Clip out-of-bounds first
	int32_t min = (dsc->stops[0].frac * range) >> 8;
	if(frac <= min) {
		GRAD_CONV(tmp, dsc->stops[0].color);
		return tmp;
	}
	int32_t max = (dsc->stops[dsc->stops_count - 1].frac * range) >> 8;
	if(frac >= max) {
		GRAD_CONV(tmp, dsc->stops[dsc->stops_count - 1].color);
		return tmp;
	}
	// Find the 2 closest stop now
	int32_t d = 0;
	for(uint8_t i = 1; i < dsc->stops_count; i++) {
		int32_t cur = (dsc->stops[i].frac * range) >> 8;
		if(frac <= cur) {
			one.full = EG_ColorTo32(dsc->stops[i - 1].color);
			two.full = EG_ColorTo32(dsc->stops[i].color);
			min = (dsc->stops[i - 1].frac * range) >> 8;
			max = (dsc->stops[i].frac * range) >> 8;
			d = max - min;
			break;
		}
	}
	EG_ASSERT(d != 0);
	// Then interpolate
	frac -= min;
	EG_OPA_t mix = (frac * 255) / d;
	EG_OPA_t imix = 255 - mix;
	EG_GradientColor_t r = GRAD_CM(LV_UDIV255(two.ch.red * mix + one.ch.red * imix), LV_UDIV255(two.ch.green * mix + one.ch.green * imix),
															LV_UDIV255(two.ch.blue * mix + one.ch.blue * imix));
	return r;
}

/////////////////////////////////////////////////////////////////////////////////

void EG_GradientCleanup(EG_GradCacheItem_t *grad)
{
	if(grad->UnCached) {
		EG_FreeMem(grad);
	}
}
