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

#include "draw/nxp/vglite/EG_VGLite_Context.h"

#if EG_USE_GPU_NXP_VG_LITE
#include "draw/nxp/vglite/EG_VGLite_Buffer.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteContext::DrawLineGPU(const EGPoint *pPoint1, const EGPoint *pPoint2, const EGRect *pClipRect, const EGDrawLine *pDrawLine)
{
VGLiteError_e Res = VG_LITE_SUCCESS;
VGLitePath_t Path;
vg_lite_color_t VGColor; /* vglite takes ABGR */
VGLiteBuffer_t *VGBuffer = EG_VGliteGetDestBuffer();
VGLiteCapStyle_e CapStyle = (pDrawLine->m_RoundStart || pDrawLine->m_RoundEnd) ? VG_LITE_CAP_ROUND : VG_LITE_CAP_BUTT;
VGLiteJoinStyle_e JoinStyle = (pDrawLine->m_RoundStart || pDrawLine->m_RoundEnd) ? VG_LITE_JOIN_ROUND : VG_LITE_JOIN_MITER;

	bool IsDashed = (pDrawLine->m_DashWidth && pDrawLine->m_DashGap);
	vg_lite_float_t StrokeDashPattern[2] = {0, 0};
	uint32_t StrokeDashCount = 0;
	vg_lite_float_t StrokeDashPhase = 0;
	if(IsDashed) {
		StrokeDashPattern[0] = (vg_lite_float_t)pDrawLine->m_DashWidth;
		StrokeDashPattern[1] = (vg_lite_float_t)pDrawLine->m_DashGap;
		StrokeDashCount = sizeof(StrokeDashPattern) / sizeof(vg_lite_float_t);
		StrokeDashPhase = (vg_lite_float_t)pDrawLine->m_DashWidth / 2;
	}
	// Choose vglite blend mode based on given lvgl blend mode
	VGLiteBlend_e BlendMode = EG_VGlightGetBlendMode(pDrawLine->m_BlendMode);
	// Init path
	int32_t Width = pDrawLine->m_Width;
	int32_t LinePath[] = {// VG line path
    VLC_OP_MOVE, pPoint1->m_X, pPoint1->m_Y,
    VLC_OP_LINE, pPoint2->m_X, pPoint2->m_Y,
    VLC_OP_END
  };

	Res = vg_lite_init_path(&Path, VG_LITE_S32, VG_LITE_HIGH, sizeof(LinePath), LinePath,
        (vg_lite_float_t)pClipRect->GetX1(), (vg_lite_float_t)pClipRect->GetY1(),
        ((vg_lite_float_t)pClipRect->GetX2()) + 1.0f, ((vg_lite_float_t)pClipRect->GetY2()) + 1.0f);
	VG_LITE_ERR_RETURN_INV(Res, "Init path failed.");
	VGLiteMatrix_t Matrix;
	vg_lite_identity(&Matrix);
	EG_Color32_t Color32 = {.full = EG_ColorTo32(pDrawLine->m_Color)}; /*Convert color to RGBA8888*/
	VGLiteBufferFormat_e ColorFormat = EG_COLOR_DEPTH == 16 ? VG_LITE_BGRA8888 : VG_LITE_ABGR8888;
	if(EG_VGlitePremultSwizzle(&VGColor, Color32, pDrawLine->m_OPA, ColorFormat) != EG_RES_OK) VG_LITE_RETURN_INV("Premultiplication and swizzle failed.");
	// Draw line
	Res = vg_lite_set_draw_path_type(&Path, VG_LITE_DRAW_STROKE_PATH);
	VG_LITE_ERR_RETURN_INV(Res, "Set draw path type failed.");
	Res = vg_lite_set_stroke(&Path, CapStyle, JoinStyle, Width, 8, StrokeDashPattern, StrokeDashCount, StrokeDashPhase, VGColor);
	VG_LITE_ERR_RETURN_INV(Res, "Set stroke failed.");
	Res = vg_lite_update_stroke(&Path);
	VG_LITE_ERR_RETURN_INV(Res, "Update stroke failed.");
	EG_VGliteSetScissor(pClipRect);
	Res = vg_lite_draw(VGBuffer, &Path, VG_LITE_FILL_NON_ZERO, &Matrix, BlendMode, VGColor);
	VG_LITE_ERR_RETURN_INV(Res, "Draw line failed.");
	if(EG_VGliteRun() != EG_RES_OK) VG_LITE_RETURN_INV("Run failed.");
  EG_VGliteDisableScissor();
	Res = vg_lite_clear_path(&Path);
	VG_LITE_ERR_RETURN_INV(Res, "Clear path failed.");
	return EG_RES_OK;
}

#endif
