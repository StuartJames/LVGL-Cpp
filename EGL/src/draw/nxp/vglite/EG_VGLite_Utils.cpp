/**
 * MIT License
 *
 * Copyright 2022, 2023 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next paragraph)
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#include "draw/nxp/vglite/EG_VGLite_Utils.h"

#if EG_USE_GPU_NXP_VG_LITE
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

static inline void VG_InvalidateCache(void);

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EG_VGliteRun(void)
{
	VG_InvalidateCache();
	VG_LITE_ERR_RETURN_INV(vg_lite_flush(), "Flush failed.");
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EG_VGlitePremultSwizzle(vg_lite_color_t *pVGColor32, EG_Color32_t EG_Color32, EG_OPA_t OPA, VGLiteBufferFormat_e vg_col_format)
{
	EG_Color32_t Color32Premul = EG_Color32;
	if(OPA <= (EG_OPA_t)EG_OPA_MAX) {
		/* Only pre-multiply color if hardware pre-multiplication is not present */
		if(!vg_lite_query_feature(gcFEATURE_BIT_VG_HW_PREMULTIPLY)) {
			Color32Premul.ch.red = (uint8_t)(((uint16_t)EG_Color32.ch.red * OPA) >> 8);
			Color32Premul.ch.green = (uint8_t)(((uint16_t)EG_Color32.ch.green * OPA) >> 8);
			Color32Premul.ch.blue = (uint8_t)(((uint16_t)EG_Color32.ch.blue * OPA) >> 8);
		}
		Color32Premul.ch.alpha = OPA;
	}
	switch(vg_col_format) {
		case VG_LITE_BGRA8888:
			*pVGColor32 = Color32Premul.full;
			break;
		case VG_LITE_RGBA8888:
			*pVGColor32 = ((uint32_t)Color32Premul.ch.red << 24) | ((uint32_t)Color32Premul.ch.green << 16) |
				((uint32_t)Color32Premul.ch.blue << 8) | (uint32_t)Color32Premul.ch.alpha;
			break;
		case VG_LITE_ABGR8888:
			*pVGColor32 = ((uint32_t)Color32Premul.ch.alpha << 24) | ((uint32_t)Color32Premul.ch.blue << 16) |
				((uint32_t)Color32Premul.ch.green << 8) | (uint32_t)Color32Premul.ch.red;
			break;
		case VG_LITE_ARGB8888:
			*pVGColor32 = ((uint32_t)Color32Premul.ch.alpha << 24) | ((uint32_t)Color32Premul.ch.red << 16) |
				((uint32_t)Color32Premul.ch.green << 8) | (uint32_t)Color32Premul.ch.blue;
			break;
		default:
			return EG_RES_INVALID;
	}
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

VGLiteBlend_e EG_VGlightGetBlendMode(EG_BlendMode_e m_BlendMode)
{
VGLiteBlend_e vg_blend_mode;

	switch(m_BlendMode) {
		case EG_BLEND_MODE_ADDITIVE:
			vg_blend_mode = VG_LITE_BLEND_ADDITIVE;
			break;
		case EG_BLEND_MODE_SUBTRACTIVE:
			vg_blend_mode = VG_LITE_BLEND_SUBTRACT;
			break;
		case EG_BLEND_MODE_MULTIPLY:
			vg_blend_mode = VG_LITE_BLEND_MULTIPLY;
			break;
		case EG_BLEND_MODE_REPLACE:
			vg_blend_mode = VG_LITE_BLEND_NONE;
			break;
		default:
			vg_blend_mode = VG_LITE_BLEND_SRC_OVER;
			break;
	}
	return vg_blend_mode;
}

//////////////////////////////////////////////////////////////////////////////////////

static inline void VG_InvalidateCache(void)
{
  EGDisplay *pDisp = GetRefreshingDisplay();
	if(pDisp->m_pDriver->CleanDcacheCB)
  pDisp->m_pDriver->CleanDcacheCB(pDisp->m_pDriver);
}

#endif