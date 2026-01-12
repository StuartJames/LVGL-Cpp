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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"
#include "draw/EG_ImageCache.h"
#include "hal/EG_HALDisplay.h"
#include "misc/EG_Log.h"
#include "core/EG_Refresh.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Math.h"

//////////////////////////////////////////////////////////////////////////////////////

#define MAX_BUF_SIZE (uint32_t) GetRefreshingDisplay()->GetHorizontalRes()

static void ConvertCB(const EGRect *pDestRect, const void *pSourceBuffer, EG_Coord_t SourceWidth, EG_Coord_t SourceHeight,
											 EG_Coord_t SourceStride, const EGDrawImage *pDrawImage, EG_ImageColorFormat_t ColorFormat, EG_Color_t *pColorBuffer, EG_OPA_t *pOpaBuffer);

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat)
{
  EGSoftContext *pContext = (EGSoftContext*)pDrawImage->m_pContext;
  const EGRect *pClipRect = pContext->m_pClipRect;	// Use the clip area as draw area
	EGRect DrawRect(pClipRect);
	bool MaskAny = HasAnyDrawMask(&DrawRect);
	bool DoTransform = ((pDrawImage->m_Angle != 0) || (pDrawImage->m_Zoom != EG_IMG_ZOOM_NONE)) ? true : false;
	EGRect BlendRect;
	EGSoftBlend BlendObj(pContext);
	BlendObj.m_OPA = pDrawImage->m_OPA;
	BlendObj.m_BlendMode = pDrawImage->m_BlendMode;
	BlendObj.m_pRect = &BlendRect;
	// The simplest case just copy the pixels into the draw_buf
	if(!MaskAny && !DoTransform && ColorFormat == EG_IMG_CF_TRUE_COLOR && pDrawImage->m_RecolorOPA == EG_OPA_TRANSP) {
		BlendObj.m_pSourceBuffer = (const EG_Color_t *)pSourceBuffer;
		BlendObj.m_pRect = pRect;
		BlendObj.DoBlend();
	}
	else if(!MaskAny && !DoTransform && ColorFormat == EG_IMG_CF_ALPHA_8BIT) {
		EGRect ClippedRect;
		if(!ClippedRect.Intersect(pRect, pClipRect)) return;
		BlendObj.m_pMaskBuffer = (EG_OPA_t *)pSourceBuffer;
		BlendObj.m_pMaskRect = pRect;
		BlendObj.m_pSourceBuffer = nullptr;
		BlendObj.m_Color = pDrawImage->m_Recolor;
		BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
		BlendObj.m_pRect = pRect;
		BlendObj.DoBlend();
	}
#if EG_COLOR_DEPTH == 16
	else if(!MaskAny && !DoTransform && ColorFormat == EG_IMG_CF_RGB565A8 && pDrawImage->m_RecolorOPA == EG_OPA_TRANSP) {
		EG_Coord_t SourceWidth = pRect->GetWidth();
		EG_Coord_t SourceHeight = pRect->GetHeight();
		BlendObj.m_pSourceBuffer = (const EG_Color_t *)pSourceBuffer;
		BlendObj.m_pMaskBuffer = (EG_OPA_t *)pSourceBuffer;
		BlendObj.m_pMaskBuffer += sizeof(EG_Color_t) * SourceWidth * SourceHeight;
		BlendObj.m_pRect = pRect;
		BlendObj.m_pMaskRect = pRect;
		BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
		BlendObj.DoBlend();
	}
#endif
	else {	// In the other cases every pixel need to be checked one-by-one
		pContext->m_pClipRect->Copy(&BlendRect);
		EG_Coord_t SourceWidth = pRect->GetWidth();
		EG_Coord_t SourceHeight = pRect->GetHeight();
		EG_Coord_t BlendHeight = BlendRect.GetHeight();
		EG_Coord_t BlendWidth = BlendRect.GetWidth();
		uint32_t MaxBufferSize = MAX_BUF_SIZE;
		uint32_t BlendSize = BlendRect.GetSize();
		uint32_t BufferWidth = BlendWidth;
		uint32_t BufferHeight;
		if(BlendSize <= MaxBufferSize) BufferHeight = BlendHeight;
		else BufferHeight = MaxBufferSize / BlendWidth;  // Round to full lines
		uint32_t BufferSize = BufferWidth * BufferHeight;		// Create buffers and masks
		EG_Color_t *pBufferRGB = (EG_Color_t*)EG_GetBufferMem(BufferSize * sizeof(EG_Color_t));
		EG_OPA_t *pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(BufferSize);
		BlendObj.m_pMaskBuffer = pMaskBuffer;
		BlendObj.m_pMaskRect = &BlendRect;
		BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
		BlendObj.m_pSourceBuffer = pBufferRGB;
		EG_Coord_t LastY = BlendRect.GetY2();
		BlendRect.SetY2(BlendRect.GetY1() + BufferHeight - 1);
		DrawMaskRes_t DefMaskResult = (pDrawImage->m_Angle || (ColorFormat != EG_IMG_CF_TRUE_COLOR) || (pDrawImage->m_Zoom != EG_IMG_ZOOM_NONE)) ?
			                              EG_DRAW_MASK_RES_CHANGED : EG_DRAW_MASK_RES_FULL_COVER;
		BlendObj.m_MaskResult = DefMaskResult;
		while(BlendRect.GetY1() <= LastY) {
			// Apply transformations if any or separate the channels
			EGRect TransformRect(BlendRect);
			TransformRect.Move(-pRect->GetX1(), -pRect->GetY1());
			if(DoTransform) {
				Transform(pDrawImage->m_pContext, &TransformRect, pSourceBuffer, SourceWidth, SourceHeight, SourceWidth,
													pDrawImage, ColorFormat, pBufferRGB, pMaskBuffer);
			}
			else {
				ConvertCB(&TransformRect, pSourceBuffer, SourceWidth, SourceHeight, SourceWidth, pDrawImage, ColorFormat, pBufferRGB, pMaskBuffer);
			}
			if(pDrawImage->m_RecolorOPA > EG_OPA_MIN) {			// Apply recolor
				uint16_t premult_v[3];
				EG_OPA_t RecolorOPA = pDrawImage->m_RecolorOPA;
				EG_Color_t Recolor = pDrawImage->m_Recolor;
				EG_ColorPreMultiply(Recolor, RecolorOPA, premult_v);
				RecolorOPA = 255 - RecolorOPA;
				for(uint32_t i = 0; i < BufferSize; i++) pBufferRGB[i] = EG_ColorMixPreMultiply(premult_v, pBufferRGB[i], RecolorOPA);
			}
#if EG_DRAW_COMPLEX		
			if(MaskAny) {	// Apply the masks if any
				EG_OPA_t *TempMaskBuffer = pMaskBuffer;
				for(EG_Coord_t y = BlendRect.GetY1(); y <= BlendRect.GetY2(); y++) {
					DrawMaskRes_t LineMaskResult = DrawMaskApply(TempMaskBuffer, BlendRect.GetX1(), y, BlendWidth);
					if(LineMaskResult == EG_DRAW_MASK_RES_TRANSP) {
						EG_ZeroMem(TempMaskBuffer, BlendWidth);
						BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
					}
					else if(LineMaskResult == EG_DRAW_MASK_RES_CHANGED) {
						BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
					}
					TempMaskBuffer += BlendWidth;
				}
			}
#endif		
			BlendObj.DoBlend();
			BlendRect.SetY1(BlendRect.GetY2() + 1);			// Process the the next lines
			BlendRect.SetY2(BlendRect.GetY1() + BufferHeight - 1);
			if(BlendRect.GetY2() > LastY) BlendRect.SetY2(LastY);
		}
		EG_ReleaseBufferMem(pMaskBuffer);
		EG_ReleaseBufferMem(pBufferRGB);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

//  Separate the image channels to RGB and Alpha to match EG_COLOR_DEPTH settings
static void ConvertCB(const EGRect *pDestRect, const void *pSourceBuffer, EG_Coord_t SourceWidth, EG_Coord_t SourceHeight,
											 EG_Coord_t SourceStride, const EGDrawImage *pDrawImage, EG_ImageColorFormat_t ColorFormat, EG_Color_t *pColorBuffer, EG_OPA_t *pOpaBuffer)
{
	EG_UNUSED(pDrawImage);
	EG_UNUSED(SourceHeight);
	EG_UNUSED(SourceWidth);

	const uint8_t *pTemp8 = (const uint8_t *)pSourceBuffer;
	EG_Coord_t y;
	EG_Coord_t x;

	if(ColorFormat == EG_IMG_CF_TRUE_COLOR || ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED) {
		uint32_t PixelCount = pDestRect->GetSize();
		EG_SetMemFF(pOpaBuffer, PixelCount);
		pTemp8 += (SourceStride * pDestRect->GetY1() * sizeof(EG_Color_t)) + pDestRect->GetX1() * sizeof(EG_Color_t);
		uint32_t DestWidth = pDestRect->GetWidth();
		uint32_t dest_w_byte = DestWidth * sizeof(EG_Color_t);

		EG_Coord_t SourceStrideBytes = SourceStride * sizeof(EG_Color_t);
		EG_Color_t *pColorBufferTmp = pColorBuffer;
		for(y = pDestRect->GetY1(); y <= pDestRect->GetY2(); y++) {
			EG_CopyMem(pColorBufferTmp, pTemp8, dest_w_byte);
			pTemp8 += SourceStrideBytes;
			pColorBufferTmp += DestWidth;
		}
		// Make "holes" for with Chroma keying
		if(ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED) {
			EG_Color_t Check = EG_COLOR_CHROMA_KEY;
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
			uint8_t *pColorBufferU = (uint8_t *)pColorBuffer;
			uint8_t chk_v = Check.full;
#elif EG_COLOR_DEPTH == 16
			uint16_t *pColorBufferU = (uint16_t*)pColorBuffer;
			uint16_t chk_v = Check.full;
#elif EG_COLOR_DEPTH == 32
			uint32_t *pColorBufferU = (uint32_t *)pColorBuffer;
			uint32_t chk_v = Check.full;
#endif
			for(uint32_t i = 0; i < PixelCount; i++) {
				if(chk_v == pColorBufferU[i]) pOpaBuffer[i] = 0x00;
			}
		}
	}
	else if(ColorFormat == EG_IMG_CF_TRUE_COLOR_ALPHA) {
		pTemp8 += (SourceStride * pDestRect->GetY1() * EG_IMG_PX_SIZE_ALPHA_BYTE) + pDestRect->GetX1() * EG_IMG_PX_SIZE_ALPHA_BYTE;

		EG_Coord_t src_new_line_step_px = (SourceStride - pDestRect->GetWidth());
		EG_Coord_t src_new_line_step_byte = src_new_line_step_px * EG_IMG_PX_SIZE_ALPHA_BYTE;

		EG_Coord_t dest_h = pDestRect->GetHeight();
		EG_Coord_t DestWidth = pDestRect->GetWidth();
		for(y = 0; y < dest_h; y++) {
			for(x = 0; x < DestWidth; x++) {
				pOpaBuffer[x] = pTemp8[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
				pColorBuffer[x].full = *pTemp8;
#elif EG_COLOR_DEPTH == 16
				pColorBuffer[x].full = *pTemp8 + ((*(pTemp8 + 1)) << 8);
#elif EG_COLOR_DEPTH == 32
				pColorBuffer[x] = *((EG_Color_t *)pTemp8);
				pColorBuffer[x].ch.alpha = 0xff;
#endif
				pTemp8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
			}
			pColorBuffer += DestWidth;
			pOpaBuffer += DestWidth;
			pTemp8 += src_new_line_step_byte;
		}
	}
	else if(ColorFormat == EG_IMG_CF_RGB565A8) {
		pTemp8 += (SourceStride * pDestRect->GetY1() * sizeof(EG_Color_t)) + pDestRect->GetX1() * sizeof(EG_Color_t);

		EG_Coord_t SourceStrideBytes = SourceStride * sizeof(EG_Color_t);

		EG_Coord_t dest_h = pDestRect->GetHeight();
		EG_Coord_t DestWidth = pDestRect->GetWidth();
		for(y = 0; y < dest_h; y++) {
			EG_CopyMem(pColorBuffer, pTemp8, DestWidth * sizeof(EG_Color_t));
			pColorBuffer += DestWidth;
			pTemp8 += SourceStrideBytes;
		}

		pTemp8 = (const uint8_t *)pSourceBuffer;
		pTemp8 += sizeof(EG_Color_t) * SourceWidth * SourceHeight;
		pTemp8 += SourceStride * pDestRect->GetY1() + pDestRect->GetX1();
		for(y = 0; y < dest_h; y++) {
			EG_CopyMem(pOpaBuffer, pTemp8, DestWidth);
			pOpaBuffer += DestWidth;
			pTemp8 += SourceStride;
		}
	}
}
