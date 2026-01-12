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
 static  void       FillBlended(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, 
                          EG_Color_t color, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride, EG_BlendMode_e blend_mode);
  static void       MapBlended(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, const EG_Color_t *src_buf,
                          EG_Coord_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride, EG_BlendMode_e blend_mode);
  static EG_Color_t BlendTrueColorAdditive(EGSoftBlend *pBlend, EG_Color_t fg, EG_Color_t bg, EG_OPA_t opa);
  static EG_Color_t BlendTrueColorSubtractive(EGSoftBlend *pBlend, EG_Color_t fg, EG_Color_t bg, EG_OPA_t opa);
  static EG_Color_t BlendTrueColorMultiply(EGSoftBlend *pBlend, EG_Color_t fg, EG_Color_t bg, EG_OPA_t opa);
#endif

#if EG_COLOR_SCREEN_TRANSP
  static void       FillaRGB(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride,
                  EG_Color_t color, EG_OPA_t opa,	const EG_OPA_t *mask, EG_Coord_t mask_stride);
  static void       MapaRGB(EGSoftBlend *pBlend, EG_Color_t *dest_buf, const EGRect *dest_area, EG_Coord_t dest_stride, const EG_Color_t *src_buf,
								  EG_Coord_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, EG_Coord_t mask_stride, EG_BlendMode_e blend_mode);
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

inline EG_Color_t EGSoftBlend::BlendTrueColorAdditive(EGSoftBlend *pBlend, EG_Color_t fg, EG_Color_t bg, EG_OPA_t OPA)
{
	if(OPA <= EG_OPA_MIN) return bg;
	uint32_t tmp;
#if EG_COLOR_DEPTH == 1
	tmp = bg.full + fg.full;
	fg.full = EG_MIN(tmp, 1);
#else
	tmp = bg.ch.red + fg.ch.red;
#if EG_COLOR_DEPTH == 8
	fg.ch.red = EG_MIN(tmp, 7);
#elif EG_COLOR_DEPTH == 16
	fg.ch.red = EG_MIN(tmp, 31);
#elif EG_COLOR_DEPTH == 32
	fg.ch.red = EG_MIN(tmp, 255);
#endif
#if EG_COLOR_DEPTH == 8
	tmp = bg.ch.green + fg.ch.green;
	fg.ch.green = EG_MIN(tmp, 7);
#elif EG_COLOR_DEPTH == 16
#if EG_COLOR_16_SWAP == 0
	tmp = bg.ch.green + fg.ch.green;
	fg.ch.green = EG_MIN(tmp, 63);
#else
	tmp = (bg.ch.green_h << 3) + bg.ch.green_l + (fg.ch.green_h << 3) + fg.ch.green_l;
	tmp = EG_MIN(tmp, 63);
	fg.ch.green_h = tmp >> 3;
	fg.ch.green_l = tmp & 0x7;
#endif
#elif EG_COLOR_DEPTH == 32
	tmp = bg.ch.green + fg.ch.green;
	fg.ch.green = EG_MIN(tmp, 255);
#endif
	tmp = bg.ch.blue + fg.ch.blue;
#if EG_COLOR_DEPTH == 8
	fg.ch.blue = EG_MIN(tmp, 4);
#elif EG_COLOR_DEPTH == 16
	fg.ch.blue = EG_MIN(tmp, 31);
#elif EG_COLOR_DEPTH == 32
	fg.ch.blue = EG_MIN(tmp, 255);
#endif
#endif
	if(OPA == EG_OPA_COVER) return fg;
	return EG_ColorMix(fg, bg, OPA);
}

//////////////////////////////////////////////////////////////////////////////////////

inline EG_Color_t EGSoftBlend::BlendTrueColorSubtractive(EGSoftBlend *pBlend, EG_Color_t fg, EG_Color_t bg, EG_OPA_t OPA)
{
	if(OPA <= EG_OPA_MIN) return bg;
	int32_t tmp;
	tmp = bg.ch.red - fg.ch.red;
	fg.ch.red = EG_MAX(tmp, 0);
#if EG_COLOR_16_SWAP == 0
	tmp = bg.ch.green - fg.ch.green;
	fg.ch.green = EG_MAX(tmp, 0);
#else
	tmp = (bg.ch.green_h << 3) + bg.ch.green_l + (fg.ch.green_h << 3) + fg.ch.green_l;
	tmp = EG_MAX(tmp, 0);
	fg.ch.green_h = tmp >> 3;
	fg.ch.green_l = tmp & 0x7;
#endif
	tmp = bg.ch.blue - fg.ch.blue;
	fg.ch.blue = EG_MAX(tmp, 0);
	if(OPA == EG_OPA_COVER) return fg;
	return EG_ColorMix(fg, bg, OPA);
}

//////////////////////////////////////////////////////////////////////////////////////

inline EG_Color_t EGSoftBlend::BlendTrueColorMultiply(EGSoftBlend *pBlend, EG_Color_t fg, EG_Color_t bg, EG_OPA_t OPA)
{
	if(OPA <= EG_OPA_MIN) return bg;
#if EG_COLOR_DEPTH == 32
	fg.ch.red = (fg.ch.red * bg.ch.red) >> 8;
	fg.ch.green = (fg.ch.green * bg.ch.green) >> 8;
	fg.ch.blue = (fg.ch.blue * bg.ch.blue) >> 8;
#elif EG_COLOR_DEPTH == 16
	fg.ch.red = (fg.ch.red * bg.ch.red) >> 5;
	fg.ch.blue = (fg.ch.blue * bg.ch.blue) >> 5;
	EG_COLOR_SET_G(fg, (EG_COLOR_GET_G(fg) * EG_COLOR_GET_G(bg)) >> 6);
#elif EG_COLOR_DEPTH == 8
	fg.ch.red = (fg.ch.red * bg.ch.red) >> 3;
	fg.ch.green = (fg.ch.green * bg.ch.green) >> 3;
	fg.ch.blue = (fg.ch.blue * bg.ch.blue) >> 2;
#endif
	if(OPA == EG_OPA_COVER) return fg;
	return EG_ColorMix(fg, bg, OPA);
}

#endif

#if EG_COLOR_SCREEN_TRANSP

inline void EGSoftBlend::SetPixelaRGB(EGSoftBlend *pBlend, uint8_t *buf, EG_Color_t color, EG_OPA_t OPA)
{
	EG_Color_t bg_color;
	EG_Color_t res_color;
	EG_OPA_t bg_opa = buf[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
#if EG_COLOR_DEPTH == 8
	bg_color.full = buf[0];
	EG_ColorMixWithAlpha(bg_color, bg_opa, color, OPA, &res_color, &buf[1]);
	if(buf[1] <= EG_OPA_MIN) return;
	buf[0] = res_color.full;
#elif EG_COLOR_DEPTH == 16
	bg_color.full = buf[0] + (buf[1] << 8);
	EG_ColorMixWithAlpha(bg_color, bg_opa, color, OPA, &res_color, &buf[2]);
	if(buf[2] <= EG_OPA_MIN) return;
	buf[0] = res_color.full & 0xff;
	buf[1] = res_color.full >> 8;
#elif EG_COLOR_DEPTH == 32
	bg_color = *((EG_Color_t *)buf);
	EG_ColorMixWithAlpha(bg_color, bg_opa, color, OPA, &res_color, &buf[3]);
	if(buf[3] <= EG_OPA_MIN) return;
	buf[0] = res_color.ch.blue;
	buf[1] = res_color.ch.green;
	buf[2] = res_color.ch.red;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

inline void EGSoftBlend::SetPixelaRGBBlend(EGSoftBlend *pBlend, uint8_t *buf, EG_Color_t color, EG_OPA_t OPA, EG_Color_t (*blend_fp)(EG_Color_t,
																																																					EG_Color_t, EG_OPA_t))
{
	static EG_Color_t last_dest_color;
	static EG_Color_t last_src_color;
	static EG_Color_t last_res_color;
	static uint32_t last_opa = 0xffff; // Set to an invalid value for first

	EG_Color_t bg_color;

// Get the BG color
#if EG_COLOR_DEPTH == 8
	if(buf[1] <= EG_OPA_MIN) return;
	bg_color.full = buf[0];
#elif EG_COLOR_DEPTH == 16
	if(buf[2] <= EG_OPA_MIN) return;
	bg_color.full = buf[0] + (buf[1] << 8);
#elif EG_COLOR_DEPTH == 32
	if(buf[3] <= EG_OPA_MIN) return;
	bg_color = *((EG_Color_t *)buf);
#endif
	// Get the result color
	if(last_dest_color.full != bg_color.full || last_src_color.full != color.full || last_opa != OPA) {
		last_dest_color = bg_color;
		last_src_color = color;
		last_opa = OPA;
		last_res_color = blend_fp(last_src_color, last_dest_color, last_opa);
	}
// Set the result color
#if EG_COLOR_DEPTH == 8
	buf[0] = last_res_color.full;
#elif EG_COLOR_DEPTH == 16
	buf[0] = last_res_color.full & 0xff;
	buf[1] = last_res_color.full >> 8;
#elif EG_COLOR_DEPTH == 32
	buf[0] = last_res_color.ch.blue;
	buf[1] = last_res_color.ch.green;
	buf[2] = last_res_color.ch.red;
#endif
}

#endif


#endif
