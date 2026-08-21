/**
 * MIT License
 *
 * Copyright 2021-2023 NXP
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

// Path data sizes for different elements
#define CUBIC_PATH_DATA_SIZE 7 // 1 opcode, 6 arguments
#define LINE_PATH_DATA_SIZE 3  // 1 opcode, 2 arguments
#define MOVE_PATH_DATA_SIZE 3  // 1 opcode, 2 arguments
#define END_PATH_DATA_SIZE 1   // 1 opcode, 0 arguments
/* Maximum possible rectangle Path size
 * is in the rounded rectangle case:
 * - 1 move for the Path start
 * - 4 cubics for the corners
 * - 4 lines for the sides
 * - 1 end for the Path end */
#define RECT_PATH_DATA_MAX_SIZE 1 * MOVE_PATH_DATA_SIZE + 4 * CUBIC_PATH_DATA_SIZE + 4 * LINE_PATH_DATA_SIZE + 1 * END_PATH_DATA_SIZE

static void CreateRectPathData(int32_t *PathData, uint32_t *PathDataSize,	int32_t Radius, const EGRect *pRect);

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteContext::DrawBackgroundGPU(const EGRect *pRect, const EGRect *pClipRect, const EGDrawRect *pDrawRect)
{
	VGLiteError_e Res = VG_LITE_SUCCESS;
	int32_t Width = pRect->GetWidth();
	int32_t Height = pRect->GetHeight();
	int32_t Radius = pDrawRect->m_Radius;
	vg_lite_color_t VGColor;
	VGLiteBuffer_t *VGBuffer = EG_VGliteGetDestBuffer();
	if(pDrawRect->m_Radius < 0)	return EG_RES_INVALID;
	int32_t PathData[RECT_PATH_DATA_MAX_SIZE];	// Init Path
	uint32_t PathDataSize;
	CreateRectPathData(PathData, &PathDataSize, Radius, pRect);
	VGLiteQuality_e PathQuality = pDrawRect->m_Radius > 0 ? VG_LITE_HIGH : VG_LITE_LOW;
	VGLitePath_t Path;
	Res = vg_lite_init_path(&Path, VG_LITE_S32, PathQuality, PathDataSize, PathData,
													(vg_lite_float_t)pClipRect->GetX1(), (vg_lite_float_t)pClipRect->GetY1(),
													((vg_lite_float_t)pClipRect->GetX2()) + 1.0f, ((vg_lite_float_t)pClipRect->GetY2()) + 1.0f);
	VG_LITE_ERR_RETURN_INV(Res, "Init Path failed.");
	VGLiteMatrix_t matrix;
	vg_lite_identity(&matrix);
	VGLiteMatrix_t *GradMatrix;
	vg_lite_linear_gradient_t Gradient;
	if(pDrawRect->m_BackgroundGrad.dir != (EG_GradDirection_e)EG_GRAD_DIR_NONE) {    // Init Color/Gradient 
		uint32_t Colors[2];
		uint32_t Stops[2];
		EG_Color32_t Color32[2];
		uint8_t StopCount = EG_MAX(pDrawRect->m_BackgroundGrad.StopCount, 2);		// Gradient setup
		for(uint8_t i = 0; i < StopCount; i++) {
			Color32[i].full = EG_ColorTo32(pDrawRect->m_BackgroundGrad.stops[i].color); // Convert color to RGBA8888
			Stops[i] = pDrawRect->m_BackgroundGrad.stops[i].frac;
			VGLiteBufferFormat_e ColorFormat = EG_COLOR_DEPTH == 16 ? VG_LITE_ABGR8888 : VG_LITE_ARGB8888;
			if(EG_VGlitePremultSwizzle((vg_lite_color_t*)&Colors[i], Color32[i], pDrawRect->m_BackgroundOPA, ColorFormat) != EG_RES_OK)
				VG_LITE_RETURN_INV("Premultiplication and swizzle failed.");
		}
		EG_SetMem(&Gradient, 0, sizeof(vg_lite_linear_gradient_t));
		Res = vg_lite_init_grad(&Gradient);
		VG_LITE_ERR_RETURN_INV(Res, "Init Gradient failed");
		Res = vg_lite_set_grad(&Gradient, StopCount, (vg_lite_uint32_t*)Colors, (vg_lite_uint32_t*)Stops);
		VG_LITE_ERR_RETURN_INV(Res, "Set Gradient failed.");
		Res = vg_lite_update_grad(&Gradient);
		VG_LITE_ERR_RETURN_INV(Res, "Update Gradient failed.");
		GradMatrix = vg_lite_get_grad_matrix(&Gradient);
		vg_lite_identity(GradMatrix);
		vg_lite_translate((float)pRect->GetX1(), (float)pRect->GetY1(), GradMatrix);
		if(pDrawRect->m_BackgroundGrad.dir == (EG_GradDirection_e)EG_GRAD_DIR_VER) {
			vg_lite_scale(1.0f, (float)Height / 256.0f, GradMatrix);
			vg_lite_rotate(90.0f, GradMatrix);
		}
		else { // EG_GRAD_DIR_HOR
			vg_lite_scale((float)Width / 256.0f, 1.0f, GradMatrix);
		}
	}
	EG_Color32_t bg_col32 = {.full = EG_ColorTo32(pDrawRect->m_BackgroundColor)}; /*Convert color to RGBA8888*/
	VGLiteBufferFormat_e ColorFormat = EG_COLOR_DEPTH == 16 ? VG_LITE_BGRA8888 : VG_LITE_ABGR8888;
	if(EG_VGlitePremultSwizzle(&VGColor, bg_col32, pDrawRect->m_BackgroundOPA, ColorFormat) != EG_RES_OK)
		VG_LITE_RETURN_INV("Premultiplication and swizzle failed.");
	EG_VGliteSetScissor(pClipRect);
	// Draw rectangle
	if(pDrawRect->m_BackgroundGrad.dir == (EG_GradDirection_e)EG_GRAD_DIR_NONE) {
		Res = vg_lite_draw(VGBuffer, &Path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_SRC_OVER, VGColor);
	}
	else {
		Res = vg_lite_draw_gradient(VGBuffer, &Path, VG_LITE_FILL_EVEN_ODD, &matrix, &Gradient, VG_LITE_BLEND_SRC_OVER);
	}
	VG_LITE_ERR_RETURN_INV(Res, "Draw Gradient failed.");
	if(EG_VGliteRun() != EG_RES_OK) VG_LITE_RETURN_INV("Run failed.");
	EG_VGliteDisableScissor();
	Res = vg_lite_clear_path(&Path);
	VG_LITE_ERR_RETURN_INV(Res, "Clear Path failed.");
	if(pDrawRect->m_BackgroundGrad.dir != (EG_GradDirection_e)EG_GRAD_DIR_NONE) {
		Res = vg_lite_clear_grad(&Gradient);
		VG_LITE_ERR_RETURN_INV(Res, "Clear Gradient failed.");
	}
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteContext::DrawBorderGeneric(const EGRect *pRect, const EGRect *pClipRect, const EGDrawRect *pDrawRect, bool Border)
{
	VGLiteError_e Res = VG_LITE_SUCCESS;
	vg_lite_color_t VGColor; // vglite takes ABGR
	int32_t Radius = pDrawRect->m_Radius;
	VGLiteBuffer_t *VGBuffer = EG_VGliteGetDestBuffer();
	if(Radius < 0) return EG_RES_INVALID;
	if(Border) {
		// Draw border - only has Radius if object has Radius*/
		int32_t BorderHalf = (int32_t)floor(pDrawRect->m_BorderWidth / 2.0f);
		if(Radius > BorderHalf) Radius = Radius - BorderHalf;
	}
	else {
		// Draw outline - always has Radius, leave the same Radius in the circle case
		int32_t OutlineHalf = (int32_t)ceil(pDrawRect->m_OutlineWidth / 2.0f);
		if(Radius < (int32_t)EG_RADIUS_CIRCLE - OutlineHalf) Radius = Radius + OutlineHalf;
	}
	VGLiteCapStyle_e CapStyle = (Radius) ? VG_LITE_CAP_ROUND : VG_LITE_CAP_BUTT;
	VGLiteJoinStyle_e JoinStyle = (Radius) ? VG_LITE_JOIN_ROUND : VG_LITE_JOIN_MITER;
	// Choose vglite blend mode based on given lvgl blend mode
	VGLiteBlend_e vglite_blend_mode = EG_VGlightGetBlendMode(pDrawRect->m_BlendMode);
	int32_t PathData[RECT_PATH_DATA_MAX_SIZE];	/*** Init Path ***/
	uint32_t PathDataSize;
	CreateRectPathData(PathData, &PathDataSize, Radius, pRect);
	VGLiteQuality_e PathQuality = pDrawRect->m_Radius > 0 ? VG_LITE_HIGH : VG_LITE_LOW;
	VGLitePath_t Path;
	Res = vg_lite_init_path(&Path, VG_LITE_S32, PathQuality, PathDataSize, PathData,
													(vg_lite_float_t)pClipRect->GetX1(), (vg_lite_float_t)pClipRect->GetY1(),
													((vg_lite_float_t)pClipRect->GetX2()) + 1.0f, ((vg_lite_float_t)pClipRect->GetY2()) + 1.0f);
	VG_LITE_ERR_RETURN_INV(Res, "Init Path failed.");
	VGLiteMatrix_t matrix;
	vg_lite_identity(&matrix);
	EG_OPA_t OPA;
	EG_Color32_t Color32;
	int32_t LineWidth;
	if(Border) {
		OPA = pDrawRect->m_BorderOPA;
		Color32.full = EG_ColorTo32(pDrawRect->m_BorderColor); // Convert color to RGBA8888
		LineWidth = pDrawRect->m_BorderWidth;
	}
	else {
		OPA = pDrawRect->m_OutlineOPA;
		Color32.full = EG_ColorTo32(pDrawRect->m_OutlineColor); // Convert color to RGBA8888
		LineWidth = pDrawRect->m_OutlineWidth;
	}
	VGLiteBufferFormat_e ColorFormat = EG_COLOR_DEPTH == 16 ? VG_LITE_BGRA8888 : VG_LITE_ABGR8888;
	if(EG_VGlitePremultSwizzle(&VGColor, Color32, OPA, ColorFormat) != EG_RES_OK) VG_LITE_RETURN_INV("Premultiplication and swizzle failed.");
	// Draw border
	Res = vg_lite_set_draw_path_type(&Path, VG_LITE_DRAW_STROKE_PATH);
	VG_LITE_ERR_RETURN_INV(Res, "Set draw Path type failed.");
	Res = vg_lite_set_stroke(&Path, CapStyle, JoinStyle, LineWidth, 8, NULL, 0, 0, VGColor);
	VG_LITE_ERR_RETURN_INV(Res, "Set stroke failed.");
	Res = vg_lite_update_stroke(&Path);
	VG_LITE_ERR_RETURN_INV(Res, "Update stroke failed.");
	EG_VGliteSetScissor(pClipRect);
	Res = vg_lite_draw(VGBuffer, &Path, VG_LITE_FILL_NON_ZERO, &matrix, vglite_blend_mode, VGColor);
	VG_LITE_ERR_RETURN_INV(Res, "Draw border failed.");
	if(EG_VGliteRun() != EG_RES_OK) VG_LITE_RETURN_INV("Run failed.");
	EG_VGliteDisableScissor();
	Res = vg_lite_clear_path(&Path);
	VG_LITE_ERR_RETURN_INV(Res, "Clear Path failed.");
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

static void CreateRectPathData(int32_t *PathData, uint32_t *PathDataSize,	int32_t Radius, const EGRect *pRect)
{
	int32_t rect_width = pRect->GetWidth();
	int32_t rect_height = pRect->GetHeight();
	// Get the final Radius. Can't be larger than the half of the shortest side
	int32_t shortest_side = EG_MIN(rect_width, rect_height);
	int32_t final_radius = EG_MIN(Radius, shortest_side / 2);
	// Path data element index
	uint8_t pidx = 0;
	if((Radius == (int32_t)EG_RADIUS_CIRCLE) && (rect_width == rect_height)) {
		// Get the control point offset for rounded cases
		int32_t cpoff = (int32_t)((float)final_radius * BEZIER_OPTIM_CIRCLE);
		// Circle case
		PathData[pidx++] = VLC_OP_MOVE;		// Starting point
		PathData[pidx++] = pRect->GetX1() + final_radius;
		PathData[pidx++] = pRect->GetY1();
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Top-right arc
		PathData[pidx++] = cpoff;
		PathData[pidx++] = 0;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = final_radius - cpoff;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Bottom-right arc*/
		PathData[pidx++] = 0;
		PathData[pidx++] = cpoff;
		PathData[pidx++] = cpoff - final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Bottom-left arc
		PathData[pidx++] = 0 - cpoff;
		PathData[pidx++] = 0;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = cpoff - final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Top-left arc*/
		PathData[pidx++] = 0;
		PathData[pidx++] = 0 - cpoff;
		PathData[pidx++] = final_radius - cpoff;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = VLC_OP_END;		// Ending point
	}
	else if(Radius > 0) {
		// Get the control point offset for rounded cases
		int32_t cpoff = (int32_t)((float)final_radius * BEZIER_OPTIM_CIRCLE);
		// Rounded rectangle case
		PathData[pidx++] = VLC_OP_MOVE;		// Starting point
		PathData[pidx++] = pRect->GetX1() + final_radius;
		PathData[pidx++] = pRect->GetY1();
		PathData[pidx++] = VLC_OP_LINE;		// Top side
		PathData[pidx++] = pRect->GetX2() - final_radius + 1;  // Extended for VGLite
		PathData[pidx++] = pRect->GetY1();
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Top-right corner
		PathData[pidx++] = cpoff;
		PathData[pidx++] = 0;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = final_radius - cpoff;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = VLC_OP_LINE;		// Right side
		PathData[pidx++] = pRect->GetX2() + 1;                 // Extended for VGLite
		PathData[pidx++] = pRect->GetY2() - final_radius + 1;  // Extended for VGLite
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Bottom-right corner*/
		PathData[pidx++] = 0;
		PathData[pidx++] = cpoff;
		PathData[pidx++] = cpoff - final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = VLC_OP_LINE;		// Bottom side
		PathData[pidx++] = pRect->GetX1() + final_radius;
		PathData[pidx++] = pRect->GetY2() + 1;  // Extended for VGLite
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Bottom-left corner
		PathData[pidx++] = 0 - cpoff;
		PathData[pidx++] = 0;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = cpoff - final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = VLC_OP_LINE;		// Left side*/
		PathData[pidx++] = pRect->GetX1();
		PathData[pidx++] = pRect->GetY1() + final_radius;
		PathData[pidx++] = VLC_OP_CUBIC_REL;		// Top-left corner
		PathData[pidx++] = 0;
		PathData[pidx++] = 0 - cpoff;
		PathData[pidx++] = final_radius - cpoff;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = final_radius;
		PathData[pidx++] = 0 - final_radius;
		PathData[pidx++] = VLC_OP_END;		// Ending point
	}
	else {
		// Non-rounded rectangle case
		PathData[pidx++] = VLC_OP_MOVE;		// Starting point
		PathData[pidx++] = pRect->GetX1();
		PathData[pidx++] = pRect->GetY1();
		PathData[pidx++] = VLC_OP_LINE;		// Top side
		PathData[pidx++] = pRect->GetX2() + 1;  // Extended for VGLite
		PathData[pidx++] = pRect->GetY1();
		PathData[pidx++] = VLC_OP_LINE;		// Right side
		PathData[pidx++] = pRect->GetX2() + 1;  // Extended for VGLite
		PathData[pidx++] = pRect->GetY2() + 1;  // Extended for VGLite
		PathData[pidx++] = VLC_OP_LINE;		// Bottom side
		PathData[pidx++] = pRect->GetX1();
		PathData[pidx++] = pRect->GetY2() + 1;  // Extended for VGLite
		PathData[pidx++] = VLC_OP_LINE;		// Left side
		PathData[pidx++] = pRect->GetX1();
		PathData[pidx++] = pRect->GetY1();
		PathData[pidx++] = VLC_OP_END;		// Ending point
	}
	*PathDataSize = pidx * sizeof(int32_t);	// Resulting Path size
}

#endif
