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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#include "draw/sw/EG_SoftContext.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Rect.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX

typedef struct {
	int32_t InX;
	int32_t InY;
	int32_t OutX;
	int32_t OutY;
	int32_t SinMa;
	int32_t CosMa;
	int32_t ScaleX;
	int32_t ScaleY;
	int32_t Angle;
	int32_t Pivot256X;
	int32_t Pivot256Y;
	EGPoint Pivot;
} PointTransform_t;

static void TransformPointUpscaled(PointTransform_t *t, int32_t xin, int32_t yin, int32_t *xout, int32_t *yout);

static void AlphaRGB_NoAntiAlias(const uint8_t *src, int32_t src_w, int32_t src_h, int32_t src_stride, int32_t xs_ups, 
                      int32_t ys_ups, int32_t xs_step, int32_t ys_step, int32_t x_end, EG_Color_t *cbuf, uint8_t *abuf);

static void RGB_NoAntiAlias(const uint8_t *src, int32_t src_w, int32_t src_h, int32_t src_stride, int32_t xs_ups, int32_t ys_ups,
                        int32_t xs_step, int32_t ys_step, int32_t x_end, EG_Color_t *cbuf, uint8_t *abuf, EG_ImageColorFormat_t ColorFormat);
#if EG_COLOR_DEPTH == 16
static void RGB565A8_NoAntiAlias(const uint8_t *src, int32_t src_w, int32_t src_h, int32_t src_stride, int32_t xs_ups, int32_t ys_ups,
                          int32_t xs_step, int32_t ys_step, int32_t x_end, EG_Color_t *cbuf, uint8_t *abuf);
#endif
static void AlphaRGB_AntiAlias(const uint8_t *src, int32_t src_w, int32_t src_h, int32_t src_stride, int32_t xs_ups, int32_t ys_ups,
                          int32_t xs_step, int32_t ys_step, int32_t x_end, EG_Color_t *cbuf, uint8_t *abuf, EG_ImageColorFormat_t ColorFormat);

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawTransform(const EGRect *pRect, const void *pSourceBuffer, int32_t SourceWidth,
     int32_t SourceHeight, int32_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat, EG_Color_t *pColorBuffer, EG_OPA_t *pBufferOPA)
{
	PointTransform_t tr_dsc;
	tr_dsc.Angle = -pImage->m_Angle;
	tr_dsc.ScaleX = (256 * 256) / pImage->m_Scale.m_X;
	tr_dsc.ScaleY = (256 * 256) / pImage->m_Scale.m_Y;
	tr_dsc.Pivot = pImage->m_Pivot;

	int32_t angle_low = tr_dsc.Angle / 10;
	int32_t angle_high = angle_low + 1;
	int32_t angle_rem = tr_dsc.Angle - (angle_low * 10);

	int32_t s1 = EG_TrigoSin(angle_low);
	int32_t s2 = EG_TrigoSin(angle_high);

	int32_t c1 = EG_TrigoSin(angle_low + 90);
	int32_t c2 = EG_TrigoSin(angle_high + 90);

	tr_dsc.SinMa = (s1 * (10 - angle_rem) + s2 * angle_rem) / 10;
	tr_dsc.CosMa = (c1 * (10 - angle_rem) + c2 * angle_rem) / 10;
	tr_dsc.SinMa = tr_dsc.SinMa >> (EG_TRIGO_SHIFT - 10);
	tr_dsc.CosMa = tr_dsc.CosMa >> (EG_TRIGO_SHIFT - 10);
	tr_dsc.Pivot256X = tr_dsc.Pivot.m_X * 256;
	tr_dsc.Pivot256Y = tr_dsc.Pivot.m_Y * 256;

	int32_t DestWidth = pRect->GetWidth();
	int32_t DestHeight = pRect->GetHeight();
	int32_t y;
	for(y = 0; y < DestHeight; y++) {
		int32_t xs1_ups, ys1_ups, xs2_ups, ys2_ups;
		TransformPointUpscaled(&tr_dsc, pRect->GetX1(), pRect->GetY1() + y, &xs1_ups, &ys1_ups);
		TransformPointUpscaled(&tr_dsc, pRect->GetX2(), pRect->GetY1() + y, &xs2_ups, &ys2_ups);
		int32_t xs_diff = xs2_ups - xs1_ups;
		int32_t ys_diff = ys2_ups - ys1_ups;
		int32_t xs_step_256 = 0;
		int32_t ys_step_256 = 0;
		if(DestWidth > 1) {
			xs_step_256 = (256 * xs_diff) / (DestWidth - 1);
			ys_step_256 = (256 * ys_diff) / (DestWidth - 1);
		}
		int32_t xs_ups = xs1_ups + 0x80;
		int32_t ys_ups = ys1_ups + 0x80;

		if(pImage->m_AntiAlias == 0) {
			switch(ColorFormat) {
				case EG_COLOR_FORMAT_NATIVE_ALPHA:
					AlphaRGB_NoAntiAlias((uint8_t*)pSourceBuffer, SourceWidth, SourceHeight, SourceStride, xs_ups, ys_ups, xs_step_256, ys_step_256, DestWidth, pColorBuffer, pBufferOPA);
					break;
				case EG_COLOR_FORMAT_NATIVE:
				case EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED:
					RGB_NoAntiAlias((uint8_t*)pSourceBuffer, SourceWidth, SourceHeight, SourceStride, xs_ups, ys_ups, xs_step_256, ys_step_256, DestWidth, pColorBuffer, pBufferOPA, ColorFormat);
					break;
#if EG_COLOR_DEPTH == 16
				case EG_COLOR_FORMAT_RGB565A8:
					RGB565A8_NoAntiAlias((uint8_t*)pSourceBuffer, SourceWidth, SourceHeight, SourceStride, xs_ups, ys_ups, xs_step_256, ys_step_256, DestWidth, pColorBuffer, pBufferOPA);
					break;
#endif
				default: break;
			}
		}
		else {
			AlphaRGB_AntiAlias((uint8_t*)pSourceBuffer, SourceWidth, SourceHeight, SourceStride, xs_ups, ys_ups, xs_step_256, ys_step_256, DestWidth, pColorBuffer, pBufferOPA, ColorFormat);
		}
		pColorBuffer += DestWidth;
		pBufferOPA += DestWidth;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

static void RGB_NoAntiAlias(const uint8_t *src, int32_t SourceWidth, int32_t SourceHeight, int32_t SourceStride,
											int32_t xs_ups, int32_t ys_ups, int32_t xs_step, int32_t ys_step,
											int32_t x_end, EG_Color_t *pColorBuffer, uint8_t *pBufferOPA, EG_ImageColorFormat_t ColorFormat)
{
	int32_t xs_ups_start = xs_ups;
	int32_t ys_ups_start = ys_ups;
	EGDisplay *pDisplay = GetRefreshingDisplay();
	EG_Color_t ck = pDisplay->m_pDriver->m_ColorChromaKey;

	EG_SetMemFF(pBufferOPA, x_end);

	int32_t x;
	for(x = 0; x < x_end; x++) {
		xs_ups = xs_ups_start + ((xs_step * x) >> 8);
		ys_ups = ys_ups_start + ((ys_step * x) >> 8);

		int32_t xs_int = xs_ups >> 8;
		int32_t ys_int = ys_ups >> 8;
		if(xs_int < 0 || xs_int >= SourceWidth || ys_int < 0 || ys_int >= SourceHeight) {
			pBufferOPA[x] = 0x00;
		}
		else {
#if EG_COLOR_DEPTH == 1 || EG_COLOR_DEPTH == 8
			const uint8_t *src_tmp = src;
			src_tmp += ys_int * SourceStride + xs_int;
			pColorBuffer[x].full = src_tmp[0];
#elif EG_COLOR_DEPTH == 16
			const EG_Color_t *src_tmp = (const EG_Color_t *)src;
			src_tmp += ys_int * SourceStride + xs_int;
			pColorBuffer[x] = *src_tmp;
#elif EG_COLOR_DEPTH == 32
			const uint8_t *src_tmp = src;
			src_tmp += (ys_int * SourceStride * sizeof(EG_Color_t)) + xs_int * sizeof(EG_Color_t);
			pColorBuffer[x].full = *((uint32_t *)src_tmp);
#endif
		}
		if(ColorFormat == EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED && pColorBuffer[x].full == ck.full) {
			pBufferOPA[x] = 0x00;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

static void AlphaRGB_NoAntiAlias(const uint8_t *src, int32_t SourceWidth, int32_t SourceHeight, int32_t SourceStride,
											 int32_t xs_ups, int32_t ys_ups, int32_t xs_step, int32_t ys_step,
											 int32_t x_end, EG_Color_t *pColorBuffer, uint8_t *pBufferOPA)
{
  int32_t xs_ups_start = xs_ups;
  int32_t ys_ups_start = ys_ups;
	for(int32_t x = 0; x < x_end; x++) {
		xs_ups = xs_ups_start + ((xs_step * x) >> 8);
		ys_ups = ys_ups_start + ((ys_step * x) >> 8);
		int32_t xs_int = xs_ups >> 8;
		int32_t ys_int = ys_ups >> 8;
		if(xs_int < 0 || xs_int >= SourceWidth || ys_int < 0 || ys_int >= SourceHeight) {
			pBufferOPA[x] = 0;
		}
		else {
			const uint8_t *src_tmp = src;
			src_tmp += (ys_int * SourceStride * EG_IMG_PX_SIZE_ALPHA_BYTE) + xs_int * EG_IMG_PX_SIZE_ALPHA_BYTE;
#if EG_COLOR_DEPTH == 1 || EG_COLOR_DEPTH == 8
			pColorBuffer[x].full = src_tmp[0];
#elif EG_COLOR_DEPTH == 16
			pColorBuffer[x].full = src_tmp[0] + (src_tmp[1] << 8);
#elif EG_COLOR_DEPTH == 32
			pColorBuffer[x].full = *((uint32_t *)src_tmp);
#endif
			pBufferOPA[x] = src_tmp[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_DEPTH == 16
static void RGB565A8_NoAntiAlias(const uint8_t *src, int32_t SourceWidth, int32_t SourceHeight, int32_t SourceStride,
													 int32_t xs_ups, int32_t ys_ups, int32_t xs_step, int32_t ys_step,
													 int32_t x_end, EG_Color_t *pColorBuffer, uint8_t *pBufferOPA)
{
	int32_t xs_ups_start = xs_ups;
	int32_t ys_ups_start = ys_ups;
	for(int32_t x = 0; x < x_end; x++) {
		xs_ups = xs_ups_start + ((xs_step * x) >> 8);
		ys_ups = ys_ups_start + ((ys_step * x) >> 8);
		int32_t xs_int = xs_ups >> 8;
		int32_t ys_int = ys_ups >> 8;
		if(xs_int < 0 || xs_int >= SourceWidth || ys_int < 0 || ys_int >= SourceHeight) {
			pBufferOPA[x] = 0;
		}
		else {
			const EG_Color_t *src_tmp = (const EG_Color_t *)src;
			src_tmp += ys_int * SourceStride + xs_int;
			pColorBuffer[x] = *src_tmp;
			const EG_OPA_t *a_tmp = src + SourceStride * SourceHeight * sizeof(EG_Color_t);
			a_tmp += ys_int * SourceStride + xs_int;
			pBufferOPA[x] = *a_tmp;
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

static void AlphaRGB_AntiAlias(const uint8_t *src, int32_t SourceWidth, int32_t SourceHeight, int32_t SourceStride,
														int32_t xs_ups, int32_t ys_ups, int32_t xs_step, int32_t ys_step,
														int32_t x_end, EG_Color_t *pColorBuffer, uint8_t *pBufferOPA, EG_ImageColorFormat_t ColorFormat)
{
	int32_t xs_ups_start = xs_ups;
	int32_t ys_ups_start = ys_ups;
	bool has_alpha;
	int32_t px_size;
	EG_Color_t ck = _EG_COLOR_ZERO_INITIALIZER;
	switch(ColorFormat) {
		case EG_COLOR_FORMAT_NATIVE:
			has_alpha = false;
			px_size = sizeof(EG_Color_t);
			break;
		case EG_COLOR_FORMAT_NATIVE_ALPHA:
			has_alpha = true;
			px_size = EG_IMG_PX_SIZE_ALPHA_BYTE;
			break;
		case EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED: {
			has_alpha = true;
			px_size = sizeof(EG_Color_t);
			EGDisplay *pDisplay = GetRefreshingDisplay();
			ck = pDisplay->m_pDriver->m_ColorChromaKey;
			break;
		}
#if EG_COLOR_DEPTH == 16
		case EG_COLOR_FORMAT_RGB565A8:
			has_alpha = true;
			px_size = sizeof(EG_Color_t);
			break;
#endif
		default:
			return;
	}
	for(int32_t x = 0; x < x_end; x++) {
		xs_ups = xs_ups_start + ((xs_step * x) >> 8);
		ys_ups = ys_ups_start + ((ys_step * x) >> 8);
		int32_t xs_int = xs_ups >> 8;
		int32_t ys_int = ys_ups >> 8;
		// Fully out of the image
		if(xs_int < 0 || xs_int >= SourceWidth || ys_int < 0 || ys_int >= SourceHeight) {
			pBufferOPA[x] = 0x00;
			continue;
		}
		// Get the direction the hor and ver neighbor `fract` range will be 0x00..0xFF and `next` (+/-1) indicates the direction
		int32_t xs_fract = xs_ups & 0xFF;
		int32_t ys_fract = ys_ups & 0xFF;
		int32_t x_next;
		int32_t y_next;
		if(xs_fract < 0x80) {
			x_next = -1;
			xs_fract = (0x7F - xs_fract) * 2;
		}
		else {
			x_next = 1;
			xs_fract = (xs_fract - 0x80) * 2;
		}
		if(ys_fract < 0x80) {
			y_next = -1;
			ys_fract = (0x7F - ys_fract) * 2;
		}
		else {
			y_next = 1;
			ys_fract = (ys_fract - 0x80) * 2;
		}
		const uint8_t *src_tmp = src;
		src_tmp += (ys_int * SourceStride * px_size) + xs_int * px_size;
		if(xs_int + x_next >= 0 &&
			 xs_int + x_next <= SourceWidth - 1 &&
			 ys_int + y_next >= 0 &&
			 ys_int + y_next <= SourceHeight - 1) {
			const uint8_t *px_base = src_tmp;
			const uint8_t *px_hor = src_tmp + x_next * px_size;
			const uint8_t *px_ver = src_tmp + y_next * SourceStride * px_size;
			EG_Color_t c_base;
			EG_Color_t c_ver;
			EG_Color_t c_hor;
			if(has_alpha) {
				EG_OPA_t a_base;
				EG_OPA_t a_ver;
				EG_OPA_t a_hor;
				if(ColorFormat == EG_COLOR_FORMAT_NATIVE_ALPHA) {
					a_base = px_base[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
					a_ver = px_ver[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
					a_hor = px_hor[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
				}
#if EG_COLOR_DEPTH == 16
				else if(ColorFormat == EG_COLOR_FORMAT_RGB565A8) {
					const EG_OPA_t *a_tmp = src + SourceStride * SourceHeight * sizeof(EG_Color_t);
					a_base = *(a_tmp + (ys_int * SourceStride) + xs_int);
					a_hor = *(a_tmp + (ys_int * SourceStride) + xs_int + x_next);
					a_ver = *(a_tmp + ((ys_int + y_next) * SourceStride) + xs_int);
				}
#endif
				else if(ColorFormat == EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED) {
					if(((EG_Color_t *)px_base)->full == ck.full ||
						 ((EG_Color_t *)px_ver)->full == ck.full ||
						 ((EG_Color_t *)px_hor)->full == ck.full) {
						pBufferOPA[x] = 0x00;
						continue;
					}
					else {
						a_base = 0xff;
						a_ver = 0xff;
						a_hor = 0xff;
					}
				}
				else {
					a_base = 0xff;
					a_ver = 0xff;
					a_hor = 0xff;
				}

				if(a_ver != a_base) a_ver = ((a_ver * ys_fract) + (a_base * (0x100 - ys_fract))) >> 8;
				if(a_hor != a_base) a_hor = ((a_hor * xs_fract) + (a_base * (0x100 - xs_fract))) >> 8;
				pBufferOPA[x] = (a_ver + a_hor) >> 1;

				if(pBufferOPA[x] == 0x00) continue;

#if EG_COLOR_DEPTH == 1 || EG_COLOR_DEPTH == 8
				c_base.full = px_base[0];
				c_ver.full = px_ver[0];
				c_hor.full = px_hor[0];
#elif EG_COLOR_DEPTH == 16
				c_base.full = px_base[0] + (px_base[1] << 8);
				c_ver.full = px_ver[0] + (px_ver[1] << 8);
				c_hor.full = px_hor[0] + (px_hor[1] << 8);
#elif EG_COLOR_DEPTH == 32
				c_base.full = *((uint32_t *)px_base);
				c_ver.full = *((uint32_t *)px_ver);
				c_hor.full = *((uint32_t *)px_hor);
#endif
			}
			/*No alpha channel -> RGB*/
			else {
				c_base = *((const EG_Color_t *)px_base);
				c_hor = *((const EG_Color_t *)px_hor);
				c_ver = *((const EG_Color_t *)px_ver);
				pBufferOPA[x] = 0xff;
			}

			if(c_base.full == c_ver.full && c_base.full == c_hor.full) {
				pColorBuffer[x] = c_base;
			}
			else {
				c_ver = EG_ColorMix(c_ver, c_base, ys_fract);
				c_hor = EG_ColorMix(c_hor, c_base, xs_fract);
				pColorBuffer[x] = EG_ColorMix(c_hor, c_ver, EG_OPA_50);
			}
		}
		/*Partially out of the image*/
		else {
#if EG_COLOR_DEPTH == 1 || EG_COLOR_DEPTH == 8
			pColorBuffer[x].full = src_tmp[0];
#elif EG_COLOR_DEPTH == 16
			pColorBuffer[x].full = src_tmp[0] + (src_tmp[1] << 8);
#elif EG_COLOR_DEPTH == 32
			pColorBuffer[x].full = *((uint32_t *)src_tmp);
#endif
			EG_OPA_t a;
			switch(ColorFormat) {
				case EG_COLOR_FORMAT_NATIVE_ALPHA:{
					a = src_tmp[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
					break;
        }
				case EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED:{
					a = pColorBuffer[x].full == ck.full ? 0x00 : 0xff;
					break;
        }
#if EG_COLOR_DEPTH == 16
				case EG_COLOR_FORMAT_RGB565A8:{
					a = *(src + SourceStride * SourceHeight * sizeof(EG_Color_t) + (ys_int * SourceStride) + xs_int);
					break;
        }
#endif
				default:{
					a = 0xff;
        }
			}

			if((xs_int == 0 && x_next < 0) || (xs_int == SourceWidth - 1 && x_next > 0)) {
				pBufferOPA[x] = (a * (0xFF - xs_fract)) >> 8;
			}
			else if((ys_int == 0 && y_next < 0) || (ys_int == SourceHeight - 1 && y_next > 0)) {
				pBufferOPA[x] = (a * (0xFF - ys_fract)) >> 8;
			}
			else {
				pBufferOPA[x] = 0x00;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

static void TransformPointUpscaled(PointTransform_t *t, int32_t xin, int32_t yin, int32_t *xout,
																		 int32_t *yout)
{
	if(t->Angle == 0 && t->ScaleX == EG_SCALE_NONE && t->ScaleY == EG_SCALE_NONE) {
		*xout = xin * 256;
		*yout = yin * 256;
		return;
	}

	xin -= t->Pivot.m_X;
	yin -= t->Pivot.m_Y;

	if(t->Angle == 0) {
		*xout = ((int32_t)(xin * t->ScaleX)) + (t->Pivot256X);
		*yout = ((int32_t)(yin * t->ScaleY)) + (t->Pivot256Y);
	}
	else if(t->ScaleX == EG_SCALE_NONE && t->ScaleY == EG_SCALE_NONE) {
		*xout = ((t->CosMa * xin - t->SinMa * yin) >> 2) + (t->Pivot256X);
		*yout = ((t->SinMa * xin + t->CosMa * yin) >> 2) + (t->Pivot256Y);
	}
	else {
		*xout = (((t->CosMa * xin - t->SinMa * yin) * t->ScaleX) >> 10) + (t->Pivot256X);
		*yout = (((t->SinMa * xin + t->CosMa * yin) * t->ScaleY) >> 10) + (t->Pivot256Y);
	}
}

#endif
