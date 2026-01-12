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

#include "draw/sw/EG_DrawSoftBlend.h"
#include "misc/EG_Math.h"
#include "hal/EG_HALDisplay.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

#define FILL_NORMAL_MASK_PX(Color)                     \
	if(*pMask == EG_OPA_COVER) *pDestBuffer = Color;     \
	else *pDestBuffer = EG_ColorMix(Color, *pDestBuffer, *pMask); \
	pMask++;                                             \
	pDestBuffer++;

#define MAP_NORMAL_MASK_PX(x)                                           \
	if(*pMaskTempX) {                                                     \
		if(*pMaskTempX == EG_OPA_COVER) pDestBuffer[x] = pSourceBuffer[x];  \
		else pDestBuffer[x] = EG_ColorMix(pSourceBuffer[x], pDestBuffer[x], *pMaskTempX); \
	}                                                                     \
	pMaskTempX++;

//////////////////////////////////////////////////////////////////////////////////////

EGSoftBlend::EGSoftBlend(const EGSoftContext *pContext) :
  m_pRect(nullptr),
  m_pSourceBuffer(nullptr), 
  m_Color(EG_ColorBlack()),
  m_pMaskBuffer(nullptr),
  m_MaskResult(EG_DRAW_MASK_RES_UNKNOWN),
  m_pMaskRect(nullptr),
  m_OPA(EG_OPA_COVER),
  m_BlendMode(EG_BLEND_MODE_NORMAL),
  m_pContext(pContext)
{
  BlendProc = EGSoftBlend::BlendBasic;
}

//////////////////////////////////////////////////////////////////////////////////////

EGSoftBlend::~EGSoftBlend(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftBlend::DoBlend(void)
{
	if(m_OPA <= EG_OPA_MIN) return;	// Do not draw transparent things
  EGRect Rect;
	if(!Rect.Intersect(m_pRect, m_pContext->m_pClipRect)) return;
	m_pContext->WaitForFinish();
	BlendProc(this);
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::BlendBasic(EGSoftBlend *pBlend)
{
EG_OPA_t *pMask;
const EGDrawContext *pContext = pBlend->m_pContext;

	if(pBlend->m_pMaskBuffer == nullptr) pMask = nullptr;
	if(pBlend->m_pMaskBuffer && pBlend->m_MaskResult == EG_DRAW_MASK_RES_TRANSP) return;
	else if(pBlend->m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER)	pMask = nullptr;
	else pMask = pBlend->m_pMaskBuffer;
	EG_Coord_t DestStep = pContext->m_pDrawRect->GetWidth();
	EGRect DestRect;
	if(!DestRect.Intersect(pBlend->m_pRect, pContext->m_pClipRect)) return;
	EGDisplay *pDisplay = GetRefreshingDisplay();
	EG_Color_t *pDestBuffer = (EG_Color_t*)pContext->m_pDrawBuffer;
	if(pDisplay->m_pDriver->SetPixelCB == nullptr) {
		if(pDisplay->m_pDriver->m_ScreenTransparent == 0) {
			pDestBuffer += DestStep * (DestRect.GetY1() - pContext->m_pDrawRect->GetY1()) + (DestRect.GetX1() - pContext->m_pDrawRect->GetX1());
		}
		else {
			uint8_t *pDestBuffer8 = (uint8_t *)pDestBuffer;			// With EG_COLOR_DEPTH 16 it means ARGB8565 (3 bytes format)
			pDestBuffer8 += DestStep * (DestRect.GetY1() - pContext->m_pDrawRect->GetY1()) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			pDestBuffer8 += (DestRect.GetX1() - pContext->m_pDrawRect->GetX1()) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			pDestBuffer = (EG_Color_t *)pDestBuffer8;
		}
	}
	const EG_Color_t *pSourceBuffer = pBlend->m_pSourceBuffer;
	EG_Coord_t SourceStep;
	if(pSourceBuffer) {
		SourceStep = pBlend->m_pRect->GetWidth();
		pSourceBuffer += SourceStep * (DestRect.GetY1() - pBlend->m_pRect->GetY1()) + (DestRect.GetX1() - pBlend->m_pRect->GetX1());
	}
	else SourceStep = 0;
	EG_Coord_t MaskStep = 0;
	if(pMask) {	// Round the values in the mask if anti-aliasing is disabled
		if(pDisplay->m_pDriver->m_Antialiasing == 0) {
			const int32_t MaskSize = pBlend->m_pMaskRect->GetSize();
			for(int32_t i = 0; i < MaskSize; i++) {
				pMask[i] = pMask[i] > 128 ? EG_OPA_COVER : EG_OPA_TRANSP;
			}
		}
		MaskStep = pBlend->m_pMaskRect->GetWidth();
		pMask += MaskStep * (DestRect.GetY1() - pBlend->m_pMaskRect->GetY1()) + (DestRect.GetX1() - pBlend->m_pMaskRect->GetX1());
	}
	DestRect.Move(-pContext->m_pDrawRect->GetX1(), -pContext->m_pDrawRect->GetY1());
	if(pDisplay->m_pDriver->SetPixelCB) {
		if(pBlend->m_pSourceBuffer == nullptr) FillSetPixel(pBlend, pDestBuffer, &DestRect, DestStep, pBlend->m_Color, pBlend->m_OPA, pMask, MaskStep);
		else MapSetPixel(pBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pBlend->m_OPA, pMask, MaskStep);
	}
#if EG_COLOR_SCREEN_TRANSP
	else if(pDisplay->m_pDriver->m_ScreenTransparent) {
		if(pBlend->pSourceBuffer == nullptr) {
			FillaRGB(pBlend, pDestBuffer, &DestRect, DestStep, pBlend->m_Color, pBlend->m_OPA, pMask, MaskStep);
		}
		else {
			MapaRGB(pBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pBlend->m_OPA, pMask, MaskStep, pBlend->m_BlendMode);
		}
	}
#endif
	else if(pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL) {
		if(pBlend->m_pSourceBuffer == nullptr) {
			FillNormal(pBlend, pDestBuffer, &DestRect, DestStep, pBlend->m_Color, pBlend->m_OPA, pMask, MaskStep);
		}
		else {
			MapNormal(pBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pBlend->m_OPA, pMask, MaskStep);
		}
	}
	else {
#if EG_DRAW_COMPLEX
		if(pBlend->m_pSourceBuffer == nullptr) {
			FillBlended(pBlend, pDestBuffer, &DestRect, DestStep, pBlend->m_Color, pBlend->m_OPA, pMask, MaskStep, pBlend->m_BlendMode);
		}
		else {
			MapBlended(pBlend, pDestBuffer, &DestRect, DestStep, pSourceBuffer, SourceStep, pBlend->m_OPA, pMask, MaskStep, pBlend->m_BlendMode);
		}
#endif
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftBlend::FillSetPixel(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, EG_Coord_t DestStep,
												EG_Color_t Color, EG_OPA_t OPA, const EG_OPA_t *pMask, EG_Coord_t mask_stide)
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

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::FillNormal(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect,	EG_Coord_t DestStep,
                           EG_Color_t Color, EG_OPA_t OPA, const EG_OPA_t *pMask, EG_Coord_t MaskStep)
{
int32_t x, y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	if(pMask == nullptr) {	// No mask
//    ESP_LOGI("[Blend ]", "Buffer no mask:%p", (void*)pDestBuffer);
		if(OPA >= EG_OPA_MAX) {
			for(y = 0; y < Height; y++) {
				EG_ColorFill(pDestBuffer, Color, Width);
				pDestBuffer += DestStep;
			}
		}
		else {		// Has opacity
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
	else {	// Masked
//    ESP_LOGI("[Blend ]", "Buffer masked:%p", (void*)pDestBuffer);
#if EG_COLOR_DEPTH == 16
		uint32_t c32 = Color.full + ((uint32_t)Color.full << 16);
#endif
		if(OPA >= EG_OPA_MAX) {		// Only the mask matters
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
		else {		// With opacity
			EG_Color_t last_dest_color;	// Buffer the result color to avoid recalculating the same color
			EG_Color_t last_res_color;
			EG_OPA_t last_mask = EG_OPA_TRANSP;
			last_dest_color.full = pDestBuffer[0].full;
			last_res_color.full = pDestBuffer[0].full;
			EG_OPA_t opa_tmp = EG_OPA_TRANSP;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					if(*pMask) {
						if(*pMask != last_mask) opa_tmp = *pMask == EG_OPA_COVER ? OPA : (uint32_t)((uint32_t)(*pMask) * OPA) >> 8;
						if(*pMask != last_mask || last_dest_color.full != pDestBuffer[x].full) {
							if(opa_tmp == EG_OPA_COVER)	last_res_color = Color;
							else last_res_color = EG_ColorMix(Color, pDestBuffer[x], opa_tmp);
							last_mask = *pMask;
							last_dest_color.full = pDestBuffer[x].full;
						}
						pDestBuffer[x] = last_res_color;
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
												 EG_Coord_t DestStep, EG_Color_t Color, EG_OPA_t OPA, const EG_OPA_t *pMask, EG_Coord_t MaskStep,
												 EG_BlendMode_e blend_mode)
{
int32_t x, y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	EG_Color_t (*BlendFunc)(EGSoftBlend *, EG_Color_t, EG_Color_t, EG_OPA_t);
	switch(blend_mode) {
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
			EG_LOG_WARN("fill_blended: unsupported blend mode");
			return;
	}
	// Simple fill (maybe with opacity), no masking
	if(pMask == nullptr) {
		EG_Color_t last_dest_color = pDestBuffer[0];
		EG_Color_t last_res_color = BlendFunc(pBlend, Color, pDestBuffer[0], OPA);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(last_dest_color.full != pDestBuffer[x].full) {
					last_dest_color = pDestBuffer[x];
					last_res_color = BlendFunc(pBlend, Color, pDestBuffer[x], OPA);
				}
				pDestBuffer[x] = last_res_color;
			}
			pDestBuffer += DestStep;
		}
	}
	// Masked
	else {
		// Buffer the result color to avoid recalculating the same color
 		EG_Color_t last_dest_color;
		EG_Color_t last_res_color;
		EG_OPA_t last_mask = EG_OPA_TRANSP;
		last_dest_color = pDestBuffer[0];
		EG_OPA_t opa_tmp = pMask[0] >= EG_OPA_MAX ? OPA : (uint32_t)((uint32_t)pMask[0] * OPA) >> 8;
		last_res_color = BlendFunc(pBlend, Color, last_dest_color, opa_tmp);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x] == 0) continue;
				if(pMask[x] != last_mask || last_dest_color.full != pDestBuffer[x].full) {
					opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : (uint32_t)((uint32_t)pMask[x] * OPA) >> 8;
					last_res_color = BlendFunc(pBlend, Color, pDestBuffer[x], opa_tmp);
					last_mask = pMask[x];
					last_dest_color.full = pDestBuffer[x].full;
				}
				pDestBuffer[x] = last_res_color;
			}
			pDestBuffer += DestStep;
			pMask += MaskStep;
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftBlend::MapSetPixel(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, EG_Coord_t DestStep,
											 const EG_Color_t *pSourceBuffer, EG_Coord_t SourceStep, EG_OPA_t OPA,
											 const EG_OPA_t *pMask, EG_Coord_t MaskStep)
{
int32_t x;
int32_t y;

	EGDisplay *pDisplay = GetRefreshingDisplay();
	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	if(pMask == nullptr) {
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				pDisplay->m_pDriver->SetPixelCB(pDisplay->m_pDriver, (uint8_t*)pDestBuffer, DestStep, pDestRect->GetX1() + x, pDestRect->GetY1() + y, pSourceBuffer[x],
																OPA);
			}
			pSourceBuffer += SourceStep;
		}
	}
	else {
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x]) {
					pDisplay->m_pDriver->SetPixelCB(pDisplay->m_pDriver, (uint8_t*)pDestBuffer, DestStep, pDestRect->GetX1() + x, pDestRect->GetY1() + y, pSourceBuffer[x],
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
																						 EG_Coord_t DestStep, const EG_Color_t *pSourceBuffer,
																						 EG_Coord_t SourceStep, EG_OPA_t OPA, const EG_OPA_t *pMask,
																						 EG_Coord_t MaskStep)

{
int32_t x;
int32_t y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	// Simple fill (maybe with opacity), no masking
	if(pMask == nullptr) {
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
	// Masked
	else {
		// Only the mask matters
		if(OPA > EG_OPA_MAX) {
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
		// Handle opa and mask values too
		else {
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

#if LV_COLOR_SCREEN_TRANSP

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::FillaRGB(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestArea, EG_Coord_t DestStride,
                          EG_Color_t color, EG_OPA_t OPA,	const EG_OPA_t *pMask, EG_Coord_t MaskStride)
{
int32_t x, y;

	uint8_t *dest_buf8 = (uint8_t *)pDestBuffer;
	int32_t Width = pDestArea->GetWidth();
	int32_t Height = pDestArea->GetHeight();
	uint8_t ctmp[EG_IMG_PX_SIZE_ALPHA_BYTE];
	EG_CopyMem(ctmp, &color, sizeof(EG_Color_t));
	ctmp[EG_IMG_PX_SIZE_ALPHA_BYTE - 1] = OPA;
	// No mask
	if(pMask == NULL) {
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
		// Has opacity
		else {
			uint8_t *dest_buf8_row = dest_buf8;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					set_px_argb(dest_buf8, color, OPA);
					dest_buf8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
				}
				dest_buf8_row += DestStride * EG_IMG_PX_SIZE_ALPHA_BYTE;
				dest_buf8 = dest_buf8_row;
			}
		}
	}
	// Masked
	else {	// Only the mask matters
		if(OPA >= EG_OPA_MAX) {
			uint8_t *dest_buf8_row = dest_buf8;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					set_px_argb(dest_buf8, color, *pMask);
					pMask++;
					dest_buf8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
				}
				dest_buf8_row += DestStride * EG_IMG_PX_SIZE_ALPHA_BYTE;
				dest_buf8 = dest_buf8_row;
			}
		}
		// With opacity
		else {			// Buffer the result color to avoid recalculating the same color
			EG_OPA_t last_mask = EG_OPA_TRANSP;
			EG_OPA_t opa_tmp = EG_OPA_TRANSP;

			uint8_t *dest_buf8_row = dest_buf8;
			for(y = 0; y < Height; y++) {
				for(x = 0; x < Width; x++) {
					if(*pMask) {
						if(*pMask != last_mask) opa_tmp = (*pMask == EG_OPA_COVER) ? OPA : (uint32_t)((uint32_t)(*pMask) * OPA) >> 8;

						set_px_argb(dest_buf8, color, opa_tmp);
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

void EG_ATTRIBUTE_FAST_MEM EGSoftBlend::MapaRGB(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect,
																					 EG_Coord_t DestStep, const EG_Color_t *pSourceBuffer,
																					 EG_Coord_t SourceStep, EG_OPA_t OPA, const EG_OPA_t *pMask,
																					 EG_Coord_t MaskStep, EG_BlendMode_e blend_mode)

{
	uint8_t *pDestBuffer8 = (uint8_t *)pDestBuffer;

	int32_t Width = GetWidth(pDestRect);
	int32_t Height = GetHeight(pDestRect);

	int32_t x;
	int32_t y;

	EG_Color_t (*blend_fp)(EG_Color_t, EG_Color_t, EG_OPA_t);
	switch(blend_mode) {
		case EG_BLEND_MODE_ADDITIVE:
			blend_fp = color_blend_true_color_additive;
			break;
		case EG_BLEND_MODE_SUBTRACTIVE:
			blend_fp = color_blend_true_color_subtractive;
			break;
		case EG_BLEND_MODE_MULTIPLY:
			blend_fp = color_blend_true_color_multiply;
			break;
		default:
			blend_fp = nullptr;
	}

	// Simple fill (maybe with opacity), no masking
	if(pMask == nullptr) {
		if(OPA >= EG_OPA_MAX) {
			if(blend_fp == nullptr && EG_COLOR_DEPTH == 32) {
				for(y = 0; y < Height; y++) {
					EG_CopyMem(pDestBuffer, pSourceBuffer, Width * sizeof(EG_Color_t));
					pDestBuffer += DestStep;
					pSourceBuffer += SourceStep;
				}
			}
			else {
				uint8_t *dest_buf8_row = pDestBuffer8;
				for(y = 0; y < Height; y++) {
					if(blend_fp == nullptr) {
						for(x = 0; x < Width; x++) {
							set_px_argb(pDestBuffer8, pSourceBuffer[x], EG_OPA_COVER);
							pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
						}
					}
					else {
						for(x = 0; x < Width; x++) {
							set_px_argb_blend(pDestBuffer8, pSourceBuffer[x], EG_OPA_COVER, blend_fp);
							pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
						}
					}

					dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
					pDestBuffer8 = dest_buf8_row;
					pSourceBuffer += SourceStep;
				}
			}
		}
		// No mask but opacity
		else {
			uint8_t *dest_buf8_row = pDestBuffer8;
			for(y = 0; y < Height; y++) {
				if(blend_fp == nullptr) {
					for(x = 0; x < Width; x++) {
						set_px_argb(pDestBuffer8, pSourceBuffer[x], OPA);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				else {
					for(x = 0; x < Width; x++) {
						set_px_argb_blend(pDestBuffer8, pSourceBuffer[x], OPA, blend_fp);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}

				dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
				pDestBuffer8 = dest_buf8_row;
				pSourceBuffer += SourceStep;
			}
		}
	}
	// Masked
	else {
		// Only the mask matters
		if(OPA > EG_OPA_MAX) {
			uint8_t *dest_buf8_row = pDestBuffer8;
			for(y = 0; y < Height; y++) {
				if(blend_fp == nullptr) {
					for(x = 0; x < Width; x++) {
						set_px_argb(pDestBuffer8, pSourceBuffer[x], pMask[x]);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				else {
					for(x = 0; x < Width; x++) {
						set_px_argb_blend(pDestBuffer8, pSourceBuffer[x], pMask[x], blend_fp);
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				dest_buf8_row += DestStep * EG_IMG_PX_SIZE_ALPHA_BYTE;
				pDestBuffer8 = dest_buf8_row;
				pSourceBuffer += SourceStep;
				pMask += MaskStep;
			}
		}
		// Handle opa and mask values too
		else {
			uint8_t *dest_buf8_row = pDestBuffer8;
			for(y = 0; y < Height; y++) {
				if(blend_fp == nullptr) {
					for(x = 0; x < Width; x++) {
						if(pMask[x]) {
							EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
							set_px_argb(pDestBuffer8, pSourceBuffer[x], opa_tmp);
						}
						pDestBuffer8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
					}
				}
				else {
					for(x = 0; x < Width; x++) {
						if(pMask[x]) {
							EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
							set_px_argb_blend(pDestBuffer8, pSourceBuffer[x], opa_tmp, blend_fp);
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

void EGSoftBlend::MapBlended(EGSoftBlend *pBlend, EG_Color_t *pDestBuffer, const EGRect *pDestRect, EG_Coord_t DestStep,
												const EG_Color_t *pSourceBuffer, EG_Coord_t SourceStep, EG_OPA_t OPA,
												const EG_OPA_t *pMask, EG_Coord_t MaskStep, EG_BlendMode_e blend_mode)
{
EG_Color_t (*BlendFunc)(EGSoftBlend *pBlend, EG_Color_t, EG_Color_t, EG_OPA_t);
int32_t x, y;

	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	switch(blend_mode) {
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
			EG_LOG_WARN("fill_blended: unsupported blend mode");
			return;
	}

	EG_Color_t last_dest_color;
	EG_Color_t last_src_color;
	// Simple fill (maybe with opacity), no masking
	if(pMask == nullptr) {
		last_dest_color = pDestBuffer[0];
		last_src_color = pSourceBuffer[0];
		EG_Color_t last_res_color = BlendFunc(pBlend, last_src_color, last_dest_color, OPA);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(last_src_color.full != pSourceBuffer[x].full || last_dest_color.full != pDestBuffer[x].full) {
					last_dest_color = pDestBuffer[x];
					last_src_color = pSourceBuffer[x];
					last_res_color = BlendFunc(pBlend, last_src_color, last_dest_color, OPA);
				}
				pDestBuffer[x] = last_res_color;
			}
			pDestBuffer += DestStep;
			pSourceBuffer += SourceStep;
		}
	}
	else {	// Masked
		last_dest_color = pDestBuffer[0];
		last_src_color = pSourceBuffer[0];
		EG_OPA_t last_opa = pMask[0] >= EG_OPA_MAX ? OPA : ((OPA * pMask[0]) >> 8);
		EG_Color_t last_res_color = BlendFunc(pBlend, last_src_color, last_dest_color, last_opa);
		for(y = 0; y < Height; y++) {
			for(x = 0; x < Width; x++) {
				if(pMask[x] == 0) continue;
				EG_OPA_t opa_tmp = pMask[x] >= EG_OPA_MAX ? OPA : ((OPA * pMask[x]) >> 8);
				if(last_src_color.full != pSourceBuffer[x].full || last_dest_color.full != pDestBuffer[x].full || last_opa != opa_tmp) {
					last_dest_color = pDestBuffer[x];
					last_src_color = pSourceBuffer[x];
					last_opa = opa_tmp;
					last_res_color = BlendFunc(pBlend, last_src_color, last_dest_color, last_opa);
				}
				pDestBuffer[x] = last_res_color;
			}
			pDestBuffer += DestStep;
			pSourceBuffer += SourceStep;
			pMask += MaskStep;
		}
	}
}

#endif

