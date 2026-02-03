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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */


#include "draw/EG_DrawImage.h"
#include "draw/EG_ImageCache.h"
#include "hal/EG_HALDisplay.h"
#include "misc/EG_Log.h"
#include "core/EG_Refresh.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Math.h"

//////////////////////////////////////////////////////////////////////////////////////

EGDrawImage::EGDrawImage(void) : 
  m_pContext(nullptr),
  m_Angle(0),
  m_Zoom(EG_SCALE_NONE),
  m_Pivot(0, 0),
  m_Recolor(EG_ColorBlack()),
  m_OPA(EG_OPA_COVER),
  m_BlendMode(EG_BLEND_MODE_NORMAL),
  m_FrameID(0)
{
	m_Antialias = EG_COLOR_DEPTH > 8 ? 1 : 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawImage::Draw(const EGDrawContext *pDrawContext, const EGRect *pRect, const void *pSource)
{
	EG_Result_t res = EG_RES_INVALID;

	if(pSource == nullptr) {
		EG_LOG_WARN("Image draw: Source is NULL");
		ShowError(pRect, "No\nData");
		return;
	}
	if(m_OPA <= EG_OPA_MIN) return;
  m_pContext = pDrawContext;
	if(pDrawContext->DrawImageProc) {
		res = pDrawContext->DrawImageProc(this, pRect, pSource);
	}
	if(res != EG_RES_OK) {
		res = PreDraw(pRect, pSource);
	}
	if(res != EG_RES_OK) {
		EG_LOG_WARN("Image draw error");
		ShowError(pRect, "No\nData");
	}
}

//////////////////////////////////////////////////////////////////////////////////////

uint8_t EGDrawImage::GetPixelSize(EG_ImageColorFormat_t ColorFormat)
{
	uint8_t px_size = 0;

	switch((uint8_t)ColorFormat) {
		case EG_COLOR_FORMAT_UNKNOWN:
		case EG_COLOR_FORMAT_RAW:
			px_size = 0;
			break;
		case EG_COLOR_FORMAT_NATIVE:
		case EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED:
			px_size = EG_COLOR_SIZE;
			break;
		case EG_COLOR_FORMAT_NATIVE_ALPHA:
			px_size = EG_IMG_PX_SIZE_ALPHA_BYTE << 3;
			break;
		case EG_COLOR_FORMAT_INDEXED_1BIT:
		case EG_COLOR_FORMAT_ALPHA_1BIT:
			px_size = 1;
			break;
		case EG_COLOR_FORMAT_INDEXED_2BIT:
		case EG_COLOR_FORMAT_ALPHA_2BIT:
			px_size = 2;
			break;
		case EG_COLOR_FORMAT_INDEXED_4BIT:
		case EG_COLOR_FORMAT_ALPHA_4BIT:
			px_size = 4;
			break;
		case EG_COLOR_FORMAT_INDEXED_8BIT:
		case EG_COLOR_FORMAT_ALPHA_8BIT:
			px_size = 8;
			break;
		default:
			px_size = 0;
			break;
	}
	return px_size;
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGDrawImage::IsChromaKeyed(EG_ImageColorFormat_t ColorFormat)
{
	switch(ColorFormat) {
		case EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED:
		case EG_COLOR_FORMAT_RAW_CHROMA_KEYED:
			return true;
			break;
		default:
			break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGDrawImage::HasAlpha(EG_ImageColorFormat_t ColorFormat)
{
	switch(ColorFormat) {
		case EG_COLOR_FORMAT_NATIVE_ALPHA:
		case EG_COLOR_FORMAT_RAW_ALPHA:
		case EG_COLOR_FORMAT_INDEXED_1BIT:
		case EG_COLOR_FORMAT_INDEXED_2BIT:
		case EG_COLOR_FORMAT_INDEXED_4BIT:
		case EG_COLOR_FORMAT_INDEXED_8BIT:
		case EG_COLOR_FORMAT_ALPHA_1BIT:
		case EG_COLOR_FORMAT_ALPHA_2BIT:
		case EG_COLOR_FORMAT_ALPHA_4BIT:
		case EG_COLOR_FORMAT_ALPHA_8BIT:
			return true;
			break;
		default:
			break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_ImageSource_t EGDrawImage::GetType(const void *pSource)
{
	EG_ImageSource_t SourceType = EG_IMG_SRC_UNKNOWN;

	if(pSource == nullptr) return SourceType;
	const uint8_t *pUint8 = (uint8_t *)pSource;
#if EG_BIG_ENDIAN_SYSTEM  // The first or fourth byte depending on platform endianess shows the type of the image source
//	if(pUint8[3] >= 0x20 && pUint8[3] <= 0x7F) {
#else
	if(pUint8[0] >= 0x20 && pUint8[0] <= 0x7F) {
#endif
		SourceType = EG_IMG_SRC_FILE;  // If it's an ASCII character then it's file name
	}
#if EG_BIG_ENDIAN_SYSTEM
//	else if(pUint8[3] >= 0x80) {
#else
	else if(pUint8[0] >= 0x80) {
#endif
		SourceType = EG_IMG_SRC_SYMBOL;  // Symbols begins after 0x7F
	}
	else {
		SourceType = EG_IMG_SRC_VARIABLE;  // `lv_img_dsc_t` is draw to the first byte < 0x20
	}
	if(EG_IMG_SRC_UNKNOWN == SourceType) {
		EG_LOG_WARN("lv_img_src_get_type: unknown image type");
	}
	return SourceType;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawImage::DrawDecoded(const EGRect *pRect, const uint8_t *pMap, EG_ImageColorFormat_t ColorFormat)
{
	if(m_pContext->DrawImageDecodedProc == nullptr) return;
	m_pContext->DrawImageDecodedProc(this, pRect, pMap, ColorFormat);
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EG_ATTRIBUTE_FAST_MEM EGDrawImage::PreDraw(const EGRect *pRect, const void *pSource)
{
	if(m_OPA <= EG_OPA_MIN) return EG_RES_OK;
	ImageCacheEntry_t *pCachedDescriptor = ImageCacheOpen(pSource, m_Recolor, m_FrameID);
	if(pCachedDescriptor == nullptr) return EG_RES_INVALID;
	EG_ImageColorFormat_t ColorFormat;
	if(IsChromaKeyed((EG_ImageColorFormat_t)pCachedDescriptor->DecoderDSC.Header.ColorFormat))	ColorFormat = EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED;
	else if(EG_COLOR_FORMAT_ALPHA_8BIT == pCachedDescriptor->DecoderDSC.Header.ColorFormat)	ColorFormat = EG_COLOR_FORMAT_ALPHA_8BIT;
	else if(EG_COLOR_FORMAT_RGB565A8 == pCachedDescriptor->DecoderDSC.Header.ColorFormat)	ColorFormat = EG_COLOR_FORMAT_RGB565A8;
	else if(HasAlpha((EG_ImageColorFormat_t)pCachedDescriptor->DecoderDSC.Header.ColorFormat))	ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;
	else ColorFormat = EG_COLOR_FORMAT_NATIVE;
	if(ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT) {
		if(m_Angle || m_Zoom != EG_SCALE_NONE) {
			ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;			//  resume normal method
			pCachedDescriptor->DecoderDSC.pImageData = nullptr;
		}
	}
	if(pCachedDescriptor->DecoderDSC.pErrorMsg != nullptr) {
		EG_LOG_WARN("Image draw error");
		ShowError(pRect, pCachedDescriptor->DecoderDSC.pErrorMsg);
	}
	else if(pCachedDescriptor->DecoderDSC.pImageData) {	// The decoder gave the entire uncompressed image.
		EGRect MapRectRotate(pRect);
		if(m_Angle || m_Zoom != EG_SCALE_NONE) {
			int32_t Width = pRect->GetWidth();
			int32_t Height = pRect->GetHeight();
			EGImageBuffer::GetTransformedRect(&MapRectRotate, Width, Height, m_Angle, m_Zoom, &m_Pivot);
			MapRectRotate.Move(pRect->GetX1(), pRect->GetY1());
		}
		EGRect CommonClip;          // Common area of mask and area
		if(!CommonClip.Intersect(m_pContext->m_pClipRect, &MapRectRotate)) {  // Out of mask. There is nothing to draw so the image is drawn successfully.
			DrawCleanup(pCachedDescriptor);
			return EG_RES_OK;
		}
		const EGRect *pOriginalClip = m_pContext->m_pClipRect;
		m_pContext->m_pClipRect = &CommonClip;
		DrawDecoded(pRect, pCachedDescriptor->DecoderDSC.pImageData, ColorFormat);
		m_pContext->m_pClipRect = pOriginalClip;
	}
	else {                              // The whole uncompressed image is not available. Try to read it LineArea-by-LineArea
		EGRect MapRect;  // Common area of mask and area
		if(!MapRect.Intersect(m_pContext->m_pClipRect, pRect)) {   // Out of mask. There is nothing to draw so the image is drawn successfully.
			DrawCleanup(pCachedDescriptor);
			return EG_RES_OK;
		}
		int32_t width = MapRect.GetWidth();
		uint8_t *pBuffer = (uint8_t *)EG_GetBufferMem(MapRect.GetWidth() * EG_IMG_PX_SIZE_ALPHA_BYTE);  // +1 because of the possible alpha byte
		const EGRect *pOriginalClip = m_pContext->m_pClipRect;
		EGRect LineArea(MapRect);
		LineArea.SetHeight(1);
		int32_t x = MapRect.GetX1() - pRect->GetX1();
		int32_t y = MapRect.GetY1() - pRect->GetY1();
		int32_t row;
		EG_Result_t read_res;
		for(row = MapRect.GetY1(); row <= MapRect.GetY2(); row++) {
			EGRect MaskLine;
			if(!MaskLine.Intersect(pOriginalClip, &LineArea)) continue;
			read_res = pCachedDescriptor->DecoderDSC.pDecoder->ReadLine(&pCachedDescriptor->DecoderDSC, x, y, width, pBuffer);
			if(read_res != EG_RES_OK) {
				pCachedDescriptor->DecoderDSC.pDecoder->Close(&pCachedDescriptor->DecoderDSC);
				EG_LOG_WARN("Image draw can't read the LineArea");
				EG_ReleaseBufferMem(pBuffer);
				DrawCleanup(pCachedDescriptor);
				m_pContext->m_pClipRect = pOriginalClip;
				return EG_RES_INVALID;
			}
			m_pContext->m_pClipRect = &MaskLine;
			DrawDecoded(&LineArea, pBuffer, ColorFormat);
			LineArea.IncY1(1);
			LineArea.IncY2(1);
			y++;
		}
		m_pContext->m_pClipRect = pOriginalClip;
		EG_ReleaseBufferMem(pBuffer);
	}
	DrawCleanup(pCachedDescriptor);
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawImage::ShowError(const EGRect *pRect, const char *pMsg)
{
	EGDrawRect MessageBox;
	MessageBox.m_BackgroundColor = EG_ColorWhite();
	MessageBox.Draw(m_pContext, pRect);
	EGDrawLabel MessageLabel;
	MessageLabel.Draw(m_pContext, pRect, pMsg, nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawImage::DrawCleanup(ImageCacheEntry_t *pCache)
{
// Automatically close images with no caching
#if EG_IMG_CACHE_DEF_SIZE == 0
	pCache->DecoderDSC.pDecoder->Close(&pCache->DecoderDSC);
#else
	EG_UNUSED(cache);
#endif
}
