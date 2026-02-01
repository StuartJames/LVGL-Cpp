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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "draw/sw/EG_SoftContext.h"
#include "hal/EG_HALDisplay.h"
#include "misc/EG_Rect.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

EGLayerContext* EGSoftContext::DrawLayerCreate(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags)
{
	if(EG_COLOR_SCREEN_TRANSP == 0 && (Flags & EG_DRAW_LAYER_FLAG_HAS_ALPHA)) {
		EG_LOG_WARN("Rendering this widget needs EG_COLOR_SCREEN_TRANSP 1");
		return nullptr;
	}
	uint32_t PixelSize = Flags & EG_DRAW_LAYER_FLAG_HAS_ALPHA ? EG_IMG_PX_SIZE_ALPHA_BYTE : sizeof(EG_Color_t);
	if(Flags & EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE) {
		pDrawLayer->m_BufferSizeBytes = EG_LAYER_SIMPLE_BUF_SIZE;
		uint32_t FullSize = pDrawLayer->m_FullRect.GetSize() * PixelSize;
		if(pDrawLayer->m_BufferSizeBytes > FullSize) pDrawLayer->m_BufferSizeBytes = FullSize;
		pDrawLayer->m_pLayerBuffer = EG_AllocMem(pDrawLayer->m_BufferSizeBytes);
		if(pDrawLayer->m_pLayerBuffer == nullptr) {
			EG_LOG_WARN("Cannot allocate %" EG_PRIu32 " bytes for layer buffer. Allocating %" EG_PRIu32 " bytes instead. (Reduced performance)",
									(uint32_t)pDrawLayer->m_BufferSizeBytes, (uint32_t)EG_LAYER_SIMPLE_FALLBACK_BUF_SIZE * PixelSize);
			pDrawLayer->m_BufferSizeBytes = EG_LAYER_SIMPLE_FALLBACK_BUF_SIZE;
			pDrawLayer->m_pLayerBuffer = EG_AllocMem(pDrawLayer->m_BufferSizeBytes);
			if(pDrawLayer->m_pLayerBuffer == nullptr) {
				return nullptr;
			}
		}
		pDrawLayer->m_ActiveRect = pDrawLayer->m_FullRect;
		pDrawLayer->m_ActiveRect.SetY2(pDrawLayer->m_FullRect.GetY1());
		EG_Coord_t w = pDrawLayer->m_ActiveRect.GetWidth();
		pDrawLayer->m_MaxRowWithAlpha = pDrawLayer->m_BufferSizeBytes / w / EG_IMG_PX_SIZE_ALPHA_BYTE;
		pDrawLayer->m_MaxRowWithoutAlpha = pDrawLayer->m_BufferSizeBytes / w / sizeof(EG_Color_t);
	}
	else {
		pDrawLayer->m_ActiveRect = pDrawLayer->m_FullRect;
		pDrawLayer->m_BufferSizeBytes = pDrawLayer->m_FullRect.GetSize() * PixelSize;
		pDrawLayer->m_pLayerBuffer = EG_AllocMem(pDrawLayer->m_BufferSizeBytes);
		EG_ZeroMem(pDrawLayer->m_pLayerBuffer, pDrawLayer->m_BufferSizeBytes);
		pDrawLayer->m_HasAlpha = Flags & EG_DRAW_LAYER_FLAG_HAS_ALPHA ? 1 : 0;
		if(pDrawLayer->m_pLayerBuffer == nullptr) {
			return nullptr;
		}
		pDrawLayer->m_pContext->m_pDrawBuffer = pDrawLayer->m_pLayerBuffer;
		pDrawLayer->m_pContext->m_pDrawRect = &pDrawLayer->m_ActiveRect;
		pDrawLayer->m_pContext->m_pClipRect = &pDrawLayer->m_ActiveRect;
		EGDisplay *pDisplay = GetRefreshingDisplay();
		pDisplay->m_pDriver->m_ScreenTransparent = Flags & EG_DRAW_LAYER_FLAG_HAS_ALPHA ? 1 : 0;
	}
	return pDrawLayer;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawLayerAdjust(EGLayerContext *pDrawLayer,	 EGDrawLayerFlags_e Flags)
{
	EGDisplay *pDisplay = GetRefreshingDisplay();
	if(Flags & EG_DRAW_LAYER_FLAG_HAS_ALPHA) {
		EG_ZeroMem(pDrawLayer->m_pLayerBuffer, pDrawLayer->m_BufferSizeBytes);
		pDrawLayer->m_HasAlpha = 1;
		pDisplay->m_pDriver->m_ScreenTransparent = 1;
	}
	else {
		pDrawLayer->m_HasAlpha = 0;
		pDisplay->m_pDriver->m_ScreenTransparent = 0;
	}
	pDrawLayer->m_pContext->m_pDrawBuffer = pDrawLayer->m_pLayerBuffer;
	pDrawLayer->m_pContext->m_pDrawRect = &pDrawLayer->m_ActiveRect;
	pDrawLayer->m_pContext->m_pClipRect = &pDrawLayer->m_ActiveRect;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawLayerBlend(EGLayerContext *pDrawLayer, EGDrawImage *pImage)
{
	EGImageBuffer Image;
	Image.m_pData = (uint8_t*)pDrawLayer->m_pContext->m_pDrawBuffer;
	Image.m_Header.AlwaysZero = 0;
	Image.m_Header.Width = pDrawLayer->m_pContext->m_pDrawRect->GetWidth();
	Image.m_Header.Height = pDrawLayer->m_pContext->m_pDrawRect->GetHeight();
	Image.m_Header.ColorFormat = pDrawLayer->m_HasAlpha ? EG_COLOR_FORMAT_NATIVE_ALPHA : EG_COLOR_FORMAT_NATIVE;
	pDrawLayer->m_pContext->m_pDrawBuffer = pDrawLayer->m_Original.pBuffer;	// Restore the original draw_ctx
	pDrawLayer->m_pContext->m_pDrawRect = pDrawLayer->m_Original.pBuferArea;
	pDrawLayer->m_pContext->m_pClipRect = pDrawLayer->m_Original.pClipRect;
	EGDisplay *pDisplay = GetRefreshingDisplay();
	pDisplay->m_pDriver->m_ScreenTransparent = pDrawLayer->m_Original.ScreenTransparent;
	pImage->Draw(pDrawLayer->m_pContext, &pDrawLayer->m_ActiveRect, &Image);	// Blend the layer
	pDrawLayer->m_pContext->WaitForFinish();
	InvalidateImageCacheSource(&Image);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawLayerDestroy(EGLayerContext *pDrawLayer)
{
	EG_FreeMem(pDrawLayer->m_pLayerBuffer);
}

