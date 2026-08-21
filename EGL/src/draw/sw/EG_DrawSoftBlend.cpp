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

#include "draw/sw/EG_DrawSoftBlend.h"
#include "misc/EG_Math.h"
#include "hal/EG_HALDisplay.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

#define FILL_NORMAL_MASK_PX(Color)                           \
	if(*pMask == EG_OPA_COVER)                                 \
		*pDestBuffer = Color;                                    \
	else                                                       \
		*pDestBuffer = EG_ColorMix(Color, *pDestBuffer, *pMask); \
	pMask++;                                                   \
	pDestBuffer++;

#define MAP_NORMAL_MASK_PX(x)                                                      \
	if(*pMaskTempX) {                                                                \
		if(*pMaskTempX == EG_OPA_COVER)                                                \
			pDestBuffer[x] = pSourceBuffer[x];                                           \
		else                                                                           \
			pDestBuffer[x] = EG_ColorMix(pSourceBuffer[x], pDestBuffer[x], *pMaskTempX); \
	}                                                                                \
	pMaskTempX++;

//////////////////////////////////////////////////////////////////////////////////////

EGSoftBlend::EGSoftBlend(EGSoftContext *pDC) : EGBlendBase(pDC)
{
	((EGDeviceContext*)pDC)->BlendProc = BlendBasic;
}

//////////////////////////////////////////////////////////////////////////////////////

EGSoftBlend::~EGSoftBlend(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftBlend::DoBlend(void)
{
	if(m_OPA <= EG_OPA_MIN) return;  // Do not draw transparent things
	EGRect Rect;
	if(!Rect.Intersect(m_pRect, m_pContext->m_pClipRect)) return;
	m_pContext->WaitForFinish();
	m_pContext->BlendProc(this);
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::BlendBasic(EGBlendBase *pBlend)
{
EGRect DestRect;
EG_OPA_t *pMask;
const EGDeviceContext *pDC = pBlend->m_pContext;
EGSoftBlend *pSoftBlend = (EGSoftBlend*)pBlend;

	if(pSoftBlend->m_pMaskBuffer == nullptr) pMask = nullptr;
	if(pSoftBlend->m_pMaskBuffer && pSoftBlend->m_MaskResult == EG_DRAW_MASK_RESULT_TRANSP) return;
	else if(pSoftBlend->m_MaskResult == EG_DRAW_MASK_RESULT_FULL_COVER) pMask = nullptr;
	else pMask = pSoftBlend->m_pMaskBuffer;
	int32_t DestStep = pDC->m_pDrawRect->GetWidth();
	if(!DestRect.Intersect(pSoftBlend->m_pRect, pDC->m_pClipRect)) return;
	EGDisplay *pDisplay = GetRefreshingDisplay();
	EG_Color_t *pDestBuffer = (EG_Color_t *)pDC->m_pDrawBuffer;
	if(pDisplay->m_pDriver->SetPixelCB == nullptr) {
		if(pDisplay->m_pDriver->m_ScreenTransparent == 0) {
			pDestBuffer += DestStep * (DestRect.GetY1() - pDC->m_pDrawRect->GetY1()) + (DestRect.GetX1() - pDC->m_pDrawRect->GetX1());
		}
		else {
			uint8_t *pDestBuffer8 = (uint8_t *)pDestBuffer;  // With EG_COLOR_DEPTH 16 it means ARGB8565 (3 bytes format)
			pDestBuffer8 += DestStep * (DestRect.GetY1() - pDC->m_pDrawRect->GetY1()) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			pDestBuffer8 += (DestRect.GetX1() - pDC->m_pDrawRect->GetX1()) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			pDestBuffer = (EG_Color_t *)pDestBuffer8;
		}
	}
	const EG_Color_t *pSourceBuffer = pSoftBlend->m_pSourceBuffer;
	int32_t SourceStep;
	if(pSourceBuffer) {
		SourceStep = pSoftBlend->m_pRect->GetWidth();
		pSourceBuffer += SourceStep * (DestRect.GetY1() - pSoftBlend->m_pRect->GetY1()) + (DestRect.GetX1() - pSoftBlend->m_pRect->GetX1());
	}
	else SourceStep = 0;
	int32_t MaskStep = 0;
	if(pMask) {  // Round the values in the mask if anti-aliasing is disabled
		if(pDisplay->m_pDriver->m_AntiAliasing == 0) {
			const int32_t MaskSize = pSoftBlend->m_pMaskRect->GetSize();
			for(int32_t i = 0; i < MaskSize; i++) {
				pMask[i] = pMask[i] > 128 ? EG_OPA_COVER : EG_OPA_TRANSP;
			}
		}
		MaskStep = pSoftBlend->m_pMaskRect->GetWidth();
		pMask += MaskStep * (DestRect.GetY1() - pSoftBlend->m_pMaskRect->GetY1()) + (DestRect.GetX1() - pSoftBlend->m_pMaskRect->GetX1());
	}
	DestRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	if(pDisplay->m_pDriver->SetPixelCB) {
		if(pSoftBlend->m_pSourceBuffer == nullptr)
			FillSetPixel(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSoftBlend->m_Color, pSoftBlend->m_OPA, pMask, MaskStep);
		else
			MapSetPixel(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pSoftBlend->m_OPA, pMask, MaskStep);
	}
#if EG_COLOR_SCREEN_TRANSP
	else if(pDisplay->m_pDriver->m_ScreenTransparent) {
		if(pSoftBlend->m_pSourceBuffer == nullptr) {
			FillARGB(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSoftBlend->m_Color, pSoftBlend->m_OPA, pMask, MaskStep);
		}
		else {
			MapARGB(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pSoftBlend->m_OPA, pMask, MaskStep, pSoftBlend->m_BlendMode);
		}
	}
#endif
	else if(pSoftBlend->m_BlendMode == EG_BLEND_MODE_NORMAL) {
		if(pSoftBlend->m_pSourceBuffer == nullptr) {
			FillNormal(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSoftBlend->m_Color, pSoftBlend->m_OPA, pMask, MaskStep);
		}
		else {
			MapNormal(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pSoftBlend->m_OPA, pMask, MaskStep);
		}
	}
	else {
#if EG_DRAW_COMPLEX
		if(pSoftBlend->m_pSourceBuffer == nullptr) {
			FillBlended(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSoftBlend->m_Color, pSoftBlend->m_OPA, pMask, MaskStep, pSoftBlend->m_BlendMode);
		}
		else {
			MapBlended(pSoftBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pSoftBlend->m_OPA, pMask, MaskStep, pSoftBlend->m_BlendMode);
		}
#endif
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftBlend::FillSetPixel(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep,
															 EG_Color_t Color, EG_OPA_t OPA, const EG_OPA_t *pMask, int32_t mask_stide)
{
int32_t x, y;

	EGDisplay *pDisplay = GetRefreshingDisplay();
	if(pMask == nullptr) {
		for(y = pDestRect->GetY1(); y <= pDestRect->GetY2(); y++) {
			for(x = pDestRect->GetX1(); x <= pDestRect->GetX2(); x++) {
				pDisplay->m_pDriver->SetPixelCB(pDisplay->m_pDriver, (uint8_t *)pDestBuffer, DestStep, x, y, Color, OPA);
			}
		}
	}
	else {
		int32_t Width = pDestRect->GetWidth();
		int32_t Height = pDestRect->GetHeight();
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x]) {
					pDisplay->m_pDriver->SetPixelCB(pDisplay->m_pDriver, (uint8_t *)pDestBuffer, DestStep, pDestRect->GetX1() + x, pDestRect->GetY1() + y, Color,
																					(uint32_t)((uint32_t)OPA * pMask[x]) >> 8);
				}
			}
			pMask += mask_stide;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::FillNormal(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep,
																									 EG_Color_t Color, EG_OPA_t OPA, const EG_OPA_t *pMask, int32_t MaskStep)
{
int32_t x, y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	if(pMask == nullptr) {  // No mask
//    ESP_LOGI("[Blend ]", "Buffer no mask:%p", (void*)pDestBuffer);
		if(OPA >= EG_OPA_MAX) {
			for(y = 0; y < Height; y++) {
				EG_ColorFill(pDestBuffer, Color, Width);
				pDestBuffer += DestStep;
			}
		}
		else {  // Has opacity
			EG_Color_t LastDestColor = EG_ColorBlack();
			EG_Color_t LastColorMix = EG_ColorMix(Color, LastDestColor, OPA);
#if EG_COLOR_MIX_ROUND_OFS == 0 && EG_COLOR_DEPTH == 16
			/*EG_ColorMix work with an optimized algorithm with 16 bit color depth.
       *However, it introduces some rounded error on opa.
       *Introduce the same error here too to make EG_ColorPreMultiply produces the same result */
			OPA = (uint32_t)((uint32_t)OPA + 4) >> 3;
			OPA = OPA << 3;
#endif
			uint16_t ColorPremult[3];
			EG_ColorPreMultiply(Color, OPA, ColorPremult);
			EG_OPA_t InvertedOPA = 255 - OPA;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					if(LastDestColor.full != pDestBuffer[x].full) {
						LastDestColor = pDestBuffer[x];
						LastColorMix = EG_ColorMixPreMultiply(ColorPremult, pDestBuffer[x], InvertedOPA);
					}
					pDestBuffer[x] = LastColorMix;
				}
				pDestBuffer += DestStep;
			}
		}
	}
	else {  // Masked
//    ESP_LOGI("[Blend ]", "Buffer masked:%p", (void*)pDestBuffer);
#if EG_COLOR_DEPTH == 16
		uint32_t c32 = Color.full + ((uint32_t)Color.full << 16);
#endif
		if(OPA >= EG_OPA_MAX) {  // Only the mask matters
//    ESP_LOGI("[Blend ]", "Masked");
			int32_t x_end4 = Width - 4;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width && ((eg_uintptr_t)(pMask)&0x3); x++) {
					FILL_NORMAL_MASK_PX(Color)
				}
				for(; x <= x_end4; x += 4) {
					uint32_t mask32 = *((uint32_t *)pMask);
					if(mask32 == 0xFFFFFFFF) {
#if EG_COLOR_DEPTH == 16
						if((eg_uintptr_t)pDestBuffer & 0x3) {
							*(pDestBuffer + 0) = Color;
							uint32_t *d = (uint32_t *)(pDestBuffer + 1);
							*d = c32;
							*(pDestBuffer + 3) = Color;
						}
						else {
							uint32_t *d = (uint32_t *)pDestBuffer;
							*d = c32;
							*(d + 1) = c32;
						}
#else
						pDestBuffer[0] = Color;
						pDestBuffer[1] = Color;
						pDestBuffer[2] = Color;
						pDestBuffer[3] = Color;
#endif
						pDestBuffer += 4;
						pMask += 4;
					}
					else if(mask32) {
						FILL_NORMAL_MASK_PX(Color)
						FILL_NORMAL_MASK_PX(Color)
						FILL_NORMAL_MASK_PX(Color)
						FILL_NORMAL_MASK_PX(Color)
					}
					else {
						pMask += 4;
						pDestBuffer += 4;
					}
				}
				for(; x < Width; x++) {
					FILL_NORMAL_MASK_PX(Color)
				}
				pDestBuffer += (DestStep - Width);
				pMask += (MaskStep - Width);
			}
		}
		else {                         // With opacity
			EG_Color_t DestColor;  // Buffer the result color to avoid recalculating the same color
			EG_Color_t ResultColor;
			EG_OPA_t last_mask = EG_OPA_TRANSP;
			DestColor.full = pDestBuffer[0].full;
			ResultColor.full = pDestBuffer[0].full;
			EG_OPA_t opa_tmp = EG_OPA_TRANSP;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					if(*pMask) {
						if(*pMask != last_mask) opa_tmp = *pMask == EG_OPA_COVER ? OPA : (uint32_t)((uint32_t)(*pMask) * OPA) >> 8;
						if(*pMask != last_mask || DestColor.full != pDestBuffer[x].full) {
							if(opa_tmp == EG_OPA_COVER)
								ResultColor = Color;
							else
								ResultColor = EG_ColorMix(Color, pDestBuffer[x], opa_tmp);
							last_mask = *pMask;
							DestColor.full = pDestBuffer[x].full;
						}
						pDestBuffer[x] = ResultColor;
					}
					pMask++;
				}
				pDestBuffer += DestStep;
				pMask += (MaskStep - Width);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX
void EGSoftBlend::FillBlended(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect,
															int32_t DestStep, EG_Color_t Color, EG_OPA_t OPA, const EG_OPA_t *pMask, int32_t MaskStep,
															EG_BlendMode_e BlendMode)
{
int32_t x, y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	EG_Color_t (*BlendFunc)(EGSoftBlend *, EG_Color_t, EG_Color_t, EG_OPA_t);
	switch((uint8_t)BlendMode) {
		case EG_BLEND_MODE_ADDITIVE:
			BlendFunc = BlendTrueColorAdditive;
			break;
		case EG_BLEND_MODE_SUBTRACTIVE:
			BlendFunc = BlendTrueColorSubtractive;
			break;
		case EG_BLEND_MODE_MULTIPLY:
			BlendFunc = BlendTrueColorMultiply;
			break;
		default:
			EG_LOG_WARN("FillBlended: unsupported blend mode");
			return;
	}
	if(pMask == nullptr) {	// Simple fill (maybe with opacity), no masking
		EG_Color_t DestColor = pDestBuffer[0];
		EG_Color_t ResultColor = BlendFunc(pBlend, Color, pDestBuffer[0], OPA);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(DestColor.full != pDestBuffer[x].full) {
					DestColor = pDestBuffer[x];
					ResultColor = BlendFunc(pBlend, Color, pDestBuffer[x], OPA);
				}
				pDestBuffer[x] = ResultColor;
			}
			pDestBuffer += DestStep;
		}
	}
	else {	// Masked
		EG_Color_t DestColor;		// Buffer the result color to avoid recalculating the same color
		EG_Color_t ResultColor;
		EG_OPA_t last_mask = EG_OPA_TRANSP;
		DestColor = pDestBuffer[0];
		EG_OPA_t opa_tmp = pMask[0] >= EG_OPA_MAX ? OPA : (uint32_t)((uint32_t)pMask[0] * OPA) >> 8;
		ResultColor = BlendFunc(pBlend, Color, DestColor, opa_tmp);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x] == 0) continue;
				if(pMask[x] != last_mask || DestColor.full != pDestBuffer[x].full) {
					opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : (uint32_t)((uint32_t)pMask[x] * OPA) >> 8;
					ResultColor = BlendFunc(pBlend, Color, pDestBuffer[x], opa_tmp);
					last_mask = pMask[x];
					DestColor.full = pDestBuffer[x].full;
				}
				pDestBuffer[x] = ResultColor;
			}
			pDestBuffer += DestStep;
			pMask += MaskStep;
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftBlend::MapSetPixel(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep,
															const EG_Color_t *pSourceBuffer, int32_t SourceStep, EG_OPA_t OPA,
															const EG_OPA_t *pMask, int32_t MaskStep)
{
int32_t x, y;

	EGDisplay *pDisplay = GetRefreshingDisplay();
	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	if(pMask == nullptr) {
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				pDisplay->m_pDriver->SetPixelCB(pDisplay->m_pDriver, (uint8_t *)pDestBuffer, DestStep, pDestRect->GetX1() + x, pDestRect->GetY1() + y, pSourceBuffer[x], OPA);
			}
			pSourceBuffer += SourceStep;
		}
	}
	else {
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x]) {
					pDisplay->m_pDriver->SetPixelCB(pDisplay->m_pDriver, (uint8_t *)pDestBuffer, DestStep, pDestRect->GetX1() + x, pDestRect->GetY1() + y, pSourceBuffer[x],
																					(uint32_t)((uint32_t)OPA * pMask[x]) >> 8);
				}
			}
			pMask += MaskStep;
			pSourceBuffer += SourceStep;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::MapNormal(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect,
																									int32_t DestStep, const EG_Color_t *pSourceBuffer,
																									int32_t SourceStep, EG_OPA_t OPA, const EG_OPA_t *pMask,
																									int32_t MaskStep)
{
int32_t x, y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	if(pMask == nullptr) {	// Simple fill (maybe with opacity), no masking
		if(OPA >= EG_OPA_MAX) {
			for(y = 0; y < Height; y++) {
				EG_CopyMem(pDestBuffer, pSourceBuffer, Width * sizeof(EG_Color_t));
				pDestBuffer += DestStep;
				pSourceBuffer += SourceStep;
			}
		}
		else {
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					pDestBuffer[x] = EG_ColorMix(pSourceBuffer[x], pDestBuffer[x], OPA);
				}
				pDestBuffer += DestStep;
				pSourceBuffer += SourceStep;
			}
		}
	}
	else {	// Masked
		if(OPA > EG_OPA_MAX) {		// Only the mask matters
			int32_t x_end4 = Width - 4;
			for(y = 0; y < Height; y++) {
				const EG_OPA_t *pMaskTempX = pMask;
#if 0
        for(x = 0; x < Width; x++) {
            MAP_NORMAL_MASK_PX(x);
        }
#else
				for(x = 0; x < Width && ((eg_uintptr_t)pMaskTempX & 0x3); x++) {
					MAP_NORMAL_MASK_PX(x)
				}
				uint32_t *mask32 = (uint32_t *)pMaskTempX;
				for(; x < x_end4; x += 4) {
					if(*mask32) {
						if((*mask32) == 0xFFFFFFFF) {
							pDestBuffer[x] = pSourceBuffer[x];
							pDestBuffer[x + 1] = pSourceBuffer[x + 1];
							pDestBuffer[x + 2] = pSourceBuffer[x + 2];
							pDestBuffer[x + 3] = pSourceBuffer[x + 3];
						}
						else {
							pMaskTempX = (const EG_OPA_t *)mask32;
							MAP_NORMAL_MASK_PX(x)
							MAP_NORMAL_MASK_PX(x + 1)
							MAP_NORMAL_MASK_PX(x + 2)
							MAP_NORMAL_MASK_PX(x + 3)
						}
					}
					mask32++;
				}
				pMaskTempX = (const EG_OPA_t *)mask32;
				for(; x < Width; x++) {
					MAP_NORMAL_MASK_PX(x)
				}
#endif
				pDestBuffer += DestStep;
				pSourceBuffer += SourceStep;
				pMask += MaskStep;
			}
		}
		else {		// Handle opa and mask values too
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					if(pMask[x]) {
						EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
						pDestBuffer[x] = EG_ColorMix(pSourceBuffer[x], pDestBuffer[x], opa_tmp);
					}
				}
				pDestBuffer += DestStep;
				pSourceBuffer += SourceStep;
				pMask += MaskStep;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_SCREEN_TRANSP
void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::FillARGB(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestArea, int32_t DestStride,
																								 EG_Color_t color, EG_OPA_t OPA, const EG_OPA_t *pMask, int32_t MaskStride)
{
int32_t x, y;

	uint8_t *dest_buf8 = (uint8_t *)pDestBuffer;
	int32_t Width = pDestArea->GetWidth();
	int32_t Height = pDestArea->GetHeight();
	uint8_t ctmp[EG_IMG_PX_SIZE_ALPHA_BYTE];
	EG_CopyMem(ctmp, &color, sizeof(EG_Color_t));
	ctmp[EG_IMG_PX_SIZE_ALPHA_BYTE - 1] = OPA;
	if(pMask == NULL) {	// No mask
		if(OPA >= EG_OPA_MAX) {
			for(x = 0; x < Width; x++) {
				EG_CopyMem(dest_buf8, ctmp, EG_IMG_PX_SIZE_ALPHA_BYTE);
				dest_buf8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
			}
			dest_buf8 += (DestStride - Width) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			for(y = 1; y < Height; y++) {
				EG_CopyMem(dest_buf8, (uint8_t *)pDestBuffer, Width * EG_IMG_PX_SIZE_ALPHA_BYTE);
				dest_buf8 += DestStride * EG_IMG_PX_SIZE_ALPHA_BYTE;
			}
		}
		else {		// Has opacity
			uint8_t *dest_buf8_row = dest_buf8;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					SetPixelARGB(dest_buf8, color, OPA);
					dest_buf8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
				}
				dest_buf8_row += DestStride * EG_IMG_PX_SIZE_ALPHA_BYTE;
				dest_buf8 = dest_buf8_row;
			}
		}
	}
	else {  	// Masked
		if(OPA >= EG_OPA_MAX) {    // Only the mask matters
			uint8_t *dest_buf8_row = dest_buf8;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					SetPixelARGB(dest_buf8, color, *pMask);
					pMask++;
					dest_buf8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
				}
				dest_buf8_row += DestStride * EG_IMG_PX_SIZE_ALPHA_BYTE;
				dest_buf8 = dest_buf8_row;
			}
		}
		else {  		// With opacity
			EG_OPA_t last_mask = EG_OPA_TRANSP;  // Buffer the result color to avoid recalculating the same color
			EG_OPA_t opa_tmp = EG_OPA_TRANSP;
			uint8_t *dest_buf8_row = dest_buf8;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					if(*pMask) {
						if(*pMask != last_mask) opa_tmp = (*pMask == EG_OPA_COVER) ? OPA : (uint32_t)((uint32_t)(*pMask) * OPA) >> 8;
						SetPixelARGB(dest_buf8, color, opa_tmp);
					}
					dest_buf8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					pMask++;
				}
				dest_buf8_row += DestStride * EG_IMG_PX_SIZE_ALPHA_BYTE;
				dest_buf8 = dest_buf8_row;
				pMask += (MaskStride - Width);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::MapARGB(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect,
																								int32_t DestStep, const EG_Color_t *pSourceBuffer,
																								int32_t SourceStep, EG_OPA_t OPA, const EG_OPA_t *pMask,
																								int32_t MaskStep, EG_BlendMode_e BlendMode)
{
int32_t x, y;
uint8_t *pDestBuffer8 = (uint8_t *)pDestBuffer;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	EG_Color_t (*BlendFunc)(EGSoftBlend *, EG_Color_t, EG_Color_t, EG_OPA_t);
	switch((uint8_t)BlendMode) {
		case EG_BLEND_MODE_ADDITIVE:
			BlendFunc = BlendTrueColorAdditive;
			break;
		case EG_BLEND_MODE_SUBTRACTIVE:
			BlendFunc = BlendTrueColorSubtractive;
			break;
		case EG_BLEND_MODE_MULTIPLY:
			BlendFunc = BlendTrueColorMultiply;
			break;
		default:
			BlendFunc = nullptr;
	}
	if(pMask == nullptr) {  // Simple fill (maybe with opacity), no masking
		if(OPA >= EG_OPA_MAX) {
			if(BlendFunc == nullptr && EG_COLOR_DEPTH == 32) {
				for(y = 0; y < Height; y++) {
					EG_CopyMem(pDestBuffer, pSourceBuffer, Width * sizeof(EG_Color_t));
					pDestBuffer += DestStep;
					pSourceBuffer += SourceStep;
				}
			}
			else {
				uint8_t *dest_buf8_row = pDestBuffer8;
				for(y = 0; y < Height; y++) {
					if(BlendFunc == nullptr) {
						for(x = 0; x < Width; x++) {
							SetPixelARGB(pDestBuffer8, pSourceBuffer[x], EG_OPA_COVER);
							pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
						}
					}
					else {
						for(x = 0; x < Width; x++) {
							SetPixelBlendedARGB(pBlend, pDestBuffer8, pSourceBuffer[x], EG_OPA_COVER, BlendFunc);
							pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
						}
					}

					dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
					pDestBuffer8 = dest_buf8_row;
					pSourceBuffer += SourceStep;
				}
			}
		}
		else {  // No mask but opacity
			uint8_t *dest_buf8_row = pDestBuffer8;
			for(y = 0; y < Height; y++) {
				if(BlendFunc == nullptr) {
					for(x = 0; x < Width; x++) {
						SetPixelARGB(pDestBuffer8, pSourceBuffer[x], OPA);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				else {
					for(x = 0; x < Width; x++) {
						SetPixelBlendedARGB(pBlend, pDestBuffer8, pSourceBuffer[x], OPA, BlendFunc);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}

				dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
				pDestBuffer8 = dest_buf8_row;
				pSourceBuffer += SourceStep;
			}
		}
	}
	else {	// Masked
		if(OPA > EG_OPA_MAX) {		// Only the mask matters
			uint8_t *dest_buf8_row = pDestBuffer8;
			for(y = 0; y < Height; y++) {
				if(BlendFunc == nullptr) {
					for(x = 0; x < Width; x++) {
						SetPixelARGB(pDestBuffer8, pSourceBuffer[x], pMask[x]);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				else {
					for(x = 0; x < Width; x++) {
						SetPixelBlendedARGB(pBlend, pDestBuffer8, pSourceBuffer[x], pMask[x], BlendFunc);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
				pDestBuffer8 = dest_buf8_row;
				pSourceBuffer += SourceStep;
				pMask += MaskStep;
			}
		}
		else {		// Handle opa and mask values too
			uint8_t *dest_buf8_row = pDestBuffer8;
			for(y = 0; y < Height; y++) {
				if(BlendFunc == nullptr) {
					for(x = 0; x < Width; x++) {
						if(pMask[x]) {
							EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
							SetPixelARGB(pDestBuffer8, pSourceBuffer[x], opa_tmp);
						}
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				else {
					for(x = 0; x < Width; x++) {
						if(pMask[x]) {
							EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
							SetPixelBlendedARGB(pBlend, pDestBuffer8, pSourceBuffer[x], opa_tmp, BlendFunc);
						}
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
				pDestBuffer8 = dest_buf8_row;
				pSourceBuffer += SourceStep;
				pMask += MaskStep;
			}
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX
void EGSoftBlend::MapBlended(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep,
														 const EG_Color_t *pSourceBuffer, int32_t SourceStep, EG_OPA_t OPA,
														 const EG_OPA_t *pMask, int32_t MaskStep, EG_BlendMode_e BlendMode)
{
int32_t x, y;
EG_Color_t (*BlendFunc)(EGSoftBlend * pBlend, EG_Color_t, EG_Color_t, EG_OPA_t);

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	switch((uint8_t)BlendMode) {
		case EG_BLEND_MODE_ADDITIVE:
			BlendFunc = BlendTrueColorAdditive;
			break;
		case EG_BLEND_MODE_SUBTRACTIVE:
			BlendFunc = BlendTrueColorSubtractive;
			break;
		case EG_BLEND_MODE_MULTIPLY:
			BlendFunc = BlendTrueColorMultiply;
			break;
		default:
			EG_LOG_WARN("MapBlended: unsupported blend mode");
			return;
	}
	EG_Color_t DestColor;
	EG_Color_t SrceColor;
	if(pMask == nullptr) {	// Simple fill (maybe with opacity), no masking
		DestColor = pDestBuffer[0];
		SrceColor = pSourceBuffer[0];
		EG_Color_t ResultColor = BlendFunc(pBlend, SrceColor, DestColor, OPA);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(SrceColor.full != pSourceBuffer[x].full || DestColor.full != pDestBuffer[x].full) {
					DestColor = pDestBuffer[x];
					SrceColor = pSourceBuffer[x];
					ResultColor = BlendFunc(pBlend, SrceColor, DestColor, OPA);
				}
				pDestBuffer[x] = ResultColor;
			}
			pDestBuffer += DestStep;
			pSourceBuffer += SourceStep;
		}
	}
	else {  // Masked
		DestColor = pDestBuffer[0];
		SrceColor = pSourceBuffer[0];
		EG_OPA_t LastOPA = pMask[0] >= EG_OPA_MAX ? OPA : ((OPA * pMask[0]) >> 8);
		EG_Color_t ResultColor = BlendFunc(pBlend, SrceColor, DestColor, LastOPA);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x] == 0) continue;
				EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
				if(SrceColor.full != pSourceBuffer[x].full || DestColor.full != pDestBuffer[x].full || LastOPA != opa_tmp) {
					DestColor = pDestBuffer[x];
					SrceColor = pSourceBuffer[x];
					LastOPA = opa_tmp;
					ResultColor = BlendFunc(pBlend, SrceColor, DestColor, LastOPA);
				}
				pDestBuffer[x] = ResultColor;
			}
			pDestBuffer += DestStep;
			pSourceBuffer += SourceStep;
			pMask += MaskStep;
		}
	}
}

#endif
