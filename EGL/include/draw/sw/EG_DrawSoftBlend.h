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

//#pragma once

#ifndef __EGSOFTBLEND__
#define __EGSOFTBLEND__

#include "../../misc/EG_Color.h"
#include "../../misc/EG_Rect.h"
#include "../../misc/EG_Style.h"
#include "EG_SoftContext.h"
#include "../EG_DrawMask.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGSoftBlend
{
public:
                    EGSoftBlend(const EGSoftContext *pContext);
                    ~EGSoftBlend(void);
  void              DoBlend(void);

  void              (*BlendProc)(EGSoftBlend *pBlend);

  static void       BlendBasic(EGSoftBlend *pBlend);
  static void       FillSetPixel(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *blend_area, EG_Coord_t dest_stride,
									        EG_Color_t color, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stide);
  static void       FillNormal(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride,
                          EG_Color_t color, EG_OPA_t opa,	const EG_OPA_t *mask, EG_Coord_t mask_stride);
  static void       MapSetPixel(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride,	const EG_Color_t *src_buf, 
                          EG_Coord_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride);
  static void       MapNormal(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, const EG_Color_t *src_buf,
									        EG_Coord_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride);

#if EG_DRAW_COMPLEX
  static  void      FillBlended(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, 
                          EG_Color_t color, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride, EG_BlendMode_e blend_mode);
  static void       MapBlended(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, const EG_Color_t *src_buf,
                          EG_Coord_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride, EG_BlendMode_e blend_mode);
  static EG_Color_t BlendTrueColorAdditive(EGSoftBlend *pBlend, EG_Color_t ForeColor, EG_Color_t BackColor, EG_OPA_t opa);
  static EG_Color_t BlendTrueColorSubtractive(EGSoftBlend *pBlend, EG_Color_t ForeColor, EG_Color_t BackColor, EG_OPA_t opa);
  static EG_Color_t BlendTrueColorMultiply(EGSoftBlend *pBlend, EG_Color_t ForeColor, EG_Color_t BackColor, EG_OPA_t opa);
#endif

#if EG_COLOR_SCREEN_TRANSP
  static void       FillARGB(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride,
                  EG_Color_t color, EG_OPA_t opa,	const EG_OPA_t *mask, EG_Coord_t mask_stride);
  static void       MapARGB(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, const EG_Color_t *src_buf,
								  EG_Coord_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride, EG_BlendMode_e blend_mode);
  static void       SetPixelARGB(uint8_t *buf, EG_Color_t Color, EG_OPA_t OPA);
  static void       SetPixelBlendedARGB(EGSoftBlend *pBlend, uint8_t *pBuffer, EG_Color_t Color, EG_OPA_t OPA, EG_Color_t (*BlendFunc)(EGSoftBlend *, EG_Color_t, EG_Color_t, EG_OPA_t));
#endif 

  const EGRect         *m_pRect;          // The area with absolute coordinates to draw on 
  const EG_Color_t     *m_pSourceBuffer;  // Pointer to an image to blend. If set `fill_color` is ignored 
  EG_Color_t            m_Color;          // Fill color
  EG_OPA_t             *m_pMaskBuffer;    // NULL if ignored, or an alpha mask to apply on `blend_area`
  DrawMaskRes_t         m_MaskResult;     // The result of the previous mask operation 
  const EGRect         *m_pMaskRect;      // The area of `mask_buf` with absolute coordinates
  EG_OPA_t              m_OPA;            // The overall opacity
  EG_BlendMode_e        m_BlendMode;      // E.g. EG_BLEND_MODE_ADDITIVE
  const EGSoftContext  *m_pContext;

};

//////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX

//////////////////////////////////////////////////////////////////////////////////////

inline EG_Color_t EGSoftBlend::BlendTrueColorAdditive(EGSoftBlend *pBlend, EG_Color_t ForeColor, EG_Color_t BackColor, EG_OPA_t OPA)
{
	if(OPA <= EG_OPA_MIN) return BackColor;
	uint32_t tmp;
#if EG_COLOR_DEPTH == 1
	tmp = BackColor.full + ForeColor.full;
	ForeColor.full = EG_MIN(tmp, 1);
#else
	tmp = BackColor.ch.red + ForeColor.ch.red;
#if EG_COLOR_DEPTH == 8
	ForeColor.ch.red = EG_MIN(tmp, 7);
#elif EG_COLOR_DEPTH == 16
	ForeColor.ch.red = EG_MIN(tmp, 31);
#elif EG_COLOR_DEPTH == 32
	ForeColor.ch.red = EG_MIN(tmp, 255);
#endif
#if EG_COLOR_DEPTH == 8
	tmp = BackColor.ch.green + ForeColor.ch.green;
	ForeColor.ch.green = EG_MIN(tmp, 7);
#elif EG_COLOR_DEPTH == 16
#if EG_COLOR_16_SWAP == 0
	tmp = BackColor.ch.green + ForeColor.ch.green;
	ForeColor.ch.green = EG_MIN(tmp, 63);
#else
	tmp = (BackColor.ch.green_h << 3) + BackColor.ch.green_l + (ForeColor.ch.green_h << 3) + ForeColor.ch.green_l;
	tmp = EG_MIN(tmp, 63);
	ForeColor.ch.green_h = tmp >> 3;
	ForeColor.ch.green_l = tmp & 0x7;
#endif
#elif EG_COLOR_DEPTH == 32
	tmp = BackColor.ch.green + ForeColor.ch.green;
	ForeColor.ch.green = EG_MIN(tmp, 255);
#endif
	tmp = BackColor.ch.blue + ForeColor.ch.blue;
#if EG_COLOR_DEPTH == 8
	ForeColor.ch.blue = EG_MIN(tmp, 4);
#elif EG_COLOR_DEPTH == 16
	ForeColor.ch.blue = EG_MIN(tmp, 31);
#elif EG_COLOR_DEPTH == 32
	ForeColor.ch.blue = EG_MIN(tmp, 255);
#endif
#endif
	if(OPA == EG_OPA_COVER) return ForeColor;
	return EG_ColorMix(ForeColor, BackColor, OPA);
}

//////////////////////////////////////////////////////////////////////////////////////

inline EG_Color_t EGSoftBlend::BlendTrueColorSubtractive(EGSoftBlend *pBlend, EG_Color_t ForeColor, EG_Color_t BackColor, EG_OPA_t OPA)
{
	if(OPA <= EG_OPA_MIN) return BackColor;
	int32_t tmp;
	tmp = BackColor.ch.red - ForeColor.ch.red;
	ForeColor.ch.red = EG_MAX(tmp, 0);
#if EG_COLOR_16_SWAP == 0
	tmp = BackColor.ch.green - ForeColor.ch.green;
	ForeColor.ch.green = EG_MAX(tmp, 0);
#else
	tmp = (BackColor.ch.green_h << 3) + BackColor.ch.green_l + (ForeColor.ch.green_h << 3) + ForeColor.ch.green_l;
	tmp = EG_MAX(tmp, 0);
	ForeColor.ch.green_h = tmp >> 3;
	ForeColor.ch.green_l = tmp & 0x7;
#endif
	tmp = BackColor.ch.blue - ForeColor.ch.blue;
	ForeColor.ch.blue = EG_MAX(tmp, 0);
	if(OPA == EG_OPA_COVER) return ForeColor;
	return EG_ColorMix(ForeColor, BackColor, OPA);
}

//////////////////////////////////////////////////////////////////////////////////////

inline EG_Color_t EGSoftBlend::BlendTrueColorMultiply(EGSoftBlend *pBlend, EG_Color_t ForeColor, EG_Color_t BackColor, EG_OPA_t OPA)
{
	if(OPA <= EG_OPA_MIN) return BackColor;
#if EG_COLOR_DEPTH == 32
	ForeColor.ch.red = (ForeColor.ch.red * BackColor.ch.red) >> 8;
	ForeColor.ch.green = (ForeColor.ch.green * BackColor.ch.green) >> 8;
	ForeColor.ch.blue = (ForeColor.ch.blue * BackColor.ch.blue) >> 8;
#elif EG_COLOR_DEPTH == 16
	ForeColor.ch.red = (ForeColor.ch.red * BackColor.ch.red) >> 5;
	ForeColor.ch.blue = (ForeColor.ch.blue * BackColor.ch.blue) >> 5;
	EG_COLOR_SET_G(ForeColor, (EG_COLOR_GET_G(ForeColor) * EG_COLOR_GET_G(BackColor)) >> 6);
#elif EG_COLOR_DEPTH == 8
	ForeColor.ch.red = (ForeColor.ch.red * BackColor.ch.red) >> 3;
	ForeColor.ch.green = (ForeColor.ch.green * BackColor.ch.green) >> 3;
	ForeColor.ch.blue = (ForeColor.ch.blue * BackColor.ch.blue) >> 2;
#endif
	if(OPA == EG_OPA_COVER) return ForeColor;
	return EG_ColorMix(ForeColor, BackColor, OPA);
}

#endif

//////////////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_SCREEN_TRANSP

inline void EGSoftBlend::SetPixelARGB(uint8_t *pBuffer, EG_Color_t Color, EG_OPA_t OPA)
{
EG_Color_t BackColor;
EG_Color_t ResultColor;
EG_OPA_t BackOPA = pBuffer[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];

#if LV_COLOR_DEPTH == 8
	BackColor.full = pBuffer[0];
	lv_color_mix_with_alpha(BackColor, BackOPA, color, opa, &ResultColor, &pBuffer[1]);
	if(pBuffer[1] <= LV_OPA_MIN) return;
	pBuffer[0] = ResultColor.full;
#elif EG_COLOR_DEPTH == 16
	BackColor.full = pBuffer[0] + (pBuffer[1] << 8);
	EG_ColorMixWithAlpha(BackColor, BackOPA, Color, OPA, &ResultColor, &pBuffer[2]);
	if(pBuffer[2] <= EG_OPA_MIN) return;
	pBuffer[0] = ResultColor.full & 0xff;
	pBuffer[1] = ResultColor.full >> 8;
#elif EG_COLOR_DEPTH == 32
	BackColor = *((EG_Color_t *)pBuffer);
	lv_color_mix_with_alpha(BackColor, BackOPA, color, opa, &ResultColor, &pBuffer[3]);
	if(pBuffer[3] <= LV_OPA_MIN) return;
	pBuffer[0] = ResultColor.ch.blue;
	pBuffer[1] = ResultColor.ch.green;
	pBuffer[2] = ResultColor.ch.red;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

inline void EGSoftBlend::SetPixelBlendedARGB(EGSoftBlend *pBlend, uint8_t *pBuffer, EG_Color_t Color, EG_OPA_t OPA, EG_Color_t (*BlendFunc)(EGSoftBlend *, EG_Color_t, EG_Color_t, EG_OPA_t))
{
EG_Color_t DestColor;
EG_Color_t SrceColor;
EG_Color_t ResultColor;
EG_Color_t BackColor;
uint32_t LastOPA = 0xffff;  // Set to an invalid value for first


// Get the BG color
#if LV_COLOR_DEPTH == 8
	if(pBuffer[1] <= LV_OPA_MIN) return;
	BackColor.full = pBuffer[0];
#elif EG_COLOR_DEPTH == 16
	if(pBuffer[2] <= EG_OPA_MIN) return;
	BackColor.full = pBuffer[0] + (pBuffer[1] << 8);
#elif EG_COLOR_DEPTH == 32
	if(pBuffer[3] <= LV_OPA_MIN) return;
	BackColor = *((EG_Color_t *)pBuffer);
#endif
	// Get the result color
//	if(DestColor.full != BackColor.full || SrceColor.full != Color.full || LastOPA != OPA) {
	if(LastOPA != OPA) {
		DestColor = BackColor;
		SrceColor = Color;
		LastOPA = OPA;
		ResultColor = BlendFunc(pBlend, SrceColor, DestColor, LastOPA);
	}
// Set the result color
#if LV_COLOR_DEPTH == 8
	pBuffer[0] = ResultColor.full;
#elif EG_COLOR_DEPTH == 16
	pBuffer[0] = ResultColor.full & 0xff;
	pBuffer[1] = ResultColor.full >> 8;
#elif EG_COLOR_DEPTH == 32
	pBuffer[0] = ResultColor.ch.blue;
	pBuffer[1] = ResultColor.ch.green;
	pBuffer[2] = ResultColor.ch.red;
#endif
}

#endif


#endif
