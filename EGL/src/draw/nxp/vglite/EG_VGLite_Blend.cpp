/**
 * MIT License
 *
 * Copyright 2020-2023 NXP
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

#include "draw/nxp/vglite/EG_VGLite_Blend.h"

#if EG_USE_GPU_NXP_VG_LITE
#include "draw/nxp/vglite/EG_VGLite_Buffer.h"
#include "draw/nxp/vglite/EG_VGLite_Utils.h"

//////////////////////////////////////////////////////////////////////////////////////


// Stride in px required by VG-Lite HW
#define EG_GPU_NXP_VG_LITE_STRIDE_ALIGN_PX 16U

#if VG_LITE_BLIT_SPLIT_ENABLED
// BLIT split threshold - BLITs with Width or Height higher than this value will be
// done in multiple steps. Value must be 16-aligned. Don't change.
#define EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR 352
#endif

//////////////////////////////////////////////////////////////////////////////////////

EGVGLiteBlend::EGVGLiteBlend(const EGVGLiteContext *pDC) : EGBlendBase((EGDeviceContext*)pDC)
{
}

//////////////////////////////////////////////////////////////////////////////////////

EGVGLiteBlend::~EGVGLiteBlend(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteBlend::Fill(EGRect *pDestRect, EG_Color_t Color, EG_OPA_t OPA)
{
VGLiteError_e Result = VG_LITE_SUCCESS;
EG_Color32_t Color32 = {.full = EG_ColorTo32(Color)}; // Convert color to RGBA8888
vg_lite_color_t VGColor;                               // vglite takes ABGR
VGLiteBuffer_t *pVGBuffer = EG_VGliteGetDestBuffer();
VGLiteBufferFormat_e ColorFormat = EG_COLOR_DEPTH == 16 ? VG_LITE_BGRA8888 : VG_LITE_ABGR8888;

	if(EG_VGlitePremultSwizzle(&VGColor, Color32, OPA, ColorFormat) != EG_RES_OK)
		VG_LITE_RETURN_INV("Premultiplication and swizzle failed.");
	if(OPA >= (EG_OPA_t)EG_OPA_MAX) { // Opaque fill
		VGLiteRectangle_t Rect = {
			.x = pDestRect->GetX1(),
			.y = pDestRect->GetY1(),
			.width = pDestRect->GetWidth(),
			.height = pDestRect->GetHeight()
    };

		Result = vg_lite_clear(pVGBuffer, &Rect, VGColor);
		VG_LITE_ERR_RETURN_INV(Result, "Clear failed.");
		if(EG_VGliteRun() != EG_RES_OK)
			VG_LITE_RETURN_INV("Run failed.");
	}
	else { // fill with transparency
		VGLitePath_t Path;
		int32_t PathData[] = {// VG rectangular path
      VLC_OP_MOVE, pDestRect->GetX1(), pDestRect->GetY1(),
      VLC_OP_LINE, pDestRect->GetX2() + 1, pDestRect->GetY1(),
      VLC_OP_LINE, pDestRect->GetX2() + 1, pDestRect->GetY2() + 1,
      VLC_OP_LINE, pDestRect->GetX1(), pDestRect->GetY2() + 1,
      VLC_OP_LINE, pDestRect->GetX1(), pDestRect->GetY1(),
      VLC_OP_END
    };
		Result = vg_lite_init_path(&Path, VG_LITE_S32, VG_LITE_LOW, sizeof(PathData), PathData,
														(vg_lite_float_t)pDestRect->GetX1(), (vg_lite_float_t)pDestRect->GetY1(),
														((vg_lite_float_t)pDestRect->GetX2()) + 1.0f, ((vg_lite_float_t)pDestRect->GetY2()) + 1.0f);
		VG_LITE_ERR_RETURN_INV(Result, "Init path failed.");
		VGLiteMatrix_t Matrix;
		vg_lite_identity(&Matrix);
		Result = vg_lite_draw(pVGBuffer, &Path, VG_LITE_FILL_EVEN_ODD, &Matrix, VG_LITE_BLEND_SRC_OVER, VGColor);
		VG_LITE_ERR_RETURN_INV(Result, "Draw rectangle failed.");
		if(EG_VGliteRun() != EG_RES_OK) VG_LITE_RETURN_INV("Run failed.");
		Result = vg_lite_clear_path(&Path);
		VG_LITE_ERR_RETURN_INV(Result, "Clear path failed.");
	}
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

#if VG_LITE_BLIT_SPLIT_ENABLED
EG_Result_t EGVGLiteBlend::BlitSplitGPU(EG_Color_t *pDestBuffer, EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrceBuffer,
  EGRect *pSrceRect, int32_t SrceStep,	EG_OPA_t OPA)
{
	EG_VGliteSetSrceBuffer(pSrceBuffer, pSrceRect, SrceStep);
	EG_Color_t *pOriginal = pDestBuffer;
	EG_Result_t Res = BlitSplit(pDestBuffer, pDestRect, DestStep, pSrceBuffer, pSrceRect, SrceStep, OPA);
	EG_VGliteSetDestBufferPtr(pOriginal);	// Restore the original dest_vgbuf memory address.
	return Res;
}

#else

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteBlend::BlitGPU(EGRect *pDestRect,	const EG_Color_t *pSrceBuffer, const EGRect *pSrceRect, int32_t SrceStep,	EG_OPA_t OPA)
{
	if(CheckSrceAlignment(pSrceBuffer, SrceStep) != EG_RES_OK) VG_LITE_RETURN_INV("Check src alignment failed.");
	EG_VGliteSetSrceBuffer(pSrceBuffer, pSrceRect, SrceStep);
	EG_VGliteSetScissor(pDestRect);
	SetTranslationMatrix(pDestRect);
	EG_Result_t Res = Blit(pSrceRect, OPA);
	EG_VGliteDisableScissor();
	return Res;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteBlend::BlitTransformGPU(const EGRect *pDestRect, const EGRect *pClipRect,	const EG_Color_t *pSrceBuffer, 
  const EGRect *pSrceRect, int32_t SrceStep, const EGDrawImage *pImage)
{
EG_Result_t Res = EG_RES_INVALID;

	if(CheckSrceAlignment(pSrceBuffer, SrceStep) != EG_RES_OK) VG_LITE_RETURN_INV("Check src alignment failed.");
	EG_VGliteSetSrceBuffer(pSrceBuffer, pSrceRect, SrceStep);	// Set pVGSrceBuffer structure.
	EG_VGliteSetScissor(pClipRect);
	SetTransformationMatrix(pDestRect, pImage);
	Res = Blit(pSrceRect, pImage->m_OPA);
	EG_VGliteDisableScissor();
	return Res;
}

#endif

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteBlend::BufferCopyGPU(EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep,
  const EG_Color_t *pSrceBuffer, const EGRect *pSrceRect, int32_t SrceStep)
{
VGLiteError_e Result = VG_LITE_SUCCESS;

	if(CheckSrceAlignment(pSrceBuffer, SrceStep) != EG_RES_OK) VG_LITE_RETURN_INV("Check src alignment failed.");
	VGLiteBuffer_t VGSrceBuffer, VGDestBuffer;
	EG_VGliteSetBuffer(&VGSrceBuffer, pSrceBuffer, pSrceRect, SrceStep);
	EG_VGliteSetBuffer(&VGDestBuffer, pDestBuffer, pDestRect, DestStep);
	uint32_t Rect[] = {
		(uint32_t)pSrceRect->GetX1(),                // start x
		(uint32_t)pSrceRect->GetY1(),                // start y
		(uint32_t)pSrceRect->GetWidth(),
		(uint32_t)pSrceRect->GetHeight()
	};
	EG_VGliteSetScissor(pDestRect);
	SetTranslationMatrix(pDestRect);
	Result = vg_lite_blit_rect(&VGDestBuffer, &VGSrceBuffer, (VGLiteRectangle_t*)Rect, &m_VGMatrix, VG_LITE_BLEND_NONE, 0xFFFFFFFFU, VG_LITE_FILTER_POINT);
	if(Result != VG_LITE_SUCCESS) {
		EG_LOG_ERROR("Blit rectangle failed.");
		EG_VGliteDisableScissor();
		return EG_RES_INVALID;
	}
	if(EG_VGliteRun() != EG_RES_OK) {
		EG_LOG_ERROR("Run failed.");
		EG_VGliteDisableScissor();
		return EG_RES_INVALID;
	}
	EG_VGliteDisableScissor();
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

#if VG_LITE_BLIT_SPLIT_ENABLED
EG_Result_t EGVGLiteBlend::BlitSplit(EG_Color_t *pDestBuffer, EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrceBuffer,
  EGRect *pSrceRect, int32_t SrceStep, EG_OPA_t OPA)
{
EG_Result_t Res = EG_RES_INVALID;

	VG_LITE_LOG_TRACE("Blit "
    "Area: ([%d,%d], [%d,%d]) -> ([%d,%d], [%d,%d]) | "
    "Size: ([%dx%d] -> [%dx%d]) | "
    "Addr: (0x%x -> 0x%x)",
    pSrceRect->GetX1(), pSrceRect->GetY1(), pSrceRect->GetX2(), pSrceRect->GetY2(),
    pDestRect->GetX1(), pDestRect->GetY1(), pDestRect->GetX2(), pDestRect->GetY2(),
    GetWidth(pSrceRect), GetHeight(pSrceRect),
    GetWidth(pDestRect), GetHeight(pDestRect),
    (uintptr_t)pSrceBuffer, (uintptr_t)pDestBuffer);

	// Stage 1: Move starting pointers as close as possible to [x1, y1], so coordinates are as small as possible.
	AlignX(pSrceRect, (EG_Color_t **)&pSrceBuffer);
	AlignY(pSrceRect, (EG_Color_t **)&pSrceBuffer, SrceStep);
	AlignX(pDestRect, (EG_Color_t **)&pDestBuffer);
	AlignY(pDestRect, (EG_Color_t **)&pDestBuffer, DestStep);

	// Stage 2: If we're in limit, do a single BLIT
	if((pSrceRect->GetX2() < EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR) && (pSrceRect->GetY2() < EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR)) {
		if(check_src_alignment(pSrceBuffer, SrceStep) != EG_RES_OK)	VG_LITE_RETURN_INV("Check src alignment failed.");
		EG_VGliteSetDestBufferPtr(pDestBuffer);	// Set new dest_vgbuf and pVGSrceBuffer memory addresses.
		EG_VGliteSetSrceBuffferPtr(pSrceBuffer);
		EG_VGliteSetScissor(pDestRect);
		SetTranslationMatrix(pSrceRect);
		Res = Blit(pSrceRect, OPA);
		EG_VGliteDisableScissor();
		VG_LITE_LOG_TRACE("Single "
      "Area: ([%d,%d], [%d,%d]) -> ([%d,%d], [%d,%d]) | "
      "Size: ([%dx%d] -> [%dx%d]) | "
      "Addr: (0x%x -> 0x%x) %s",
      pSrceRect->GetX1(), pSrceRect->GetY1(), pSrceRect->GetX2(), pSrceRect->GetY2(),
      pDestRect->GetX1(), pDestRect->GetY1(), pDestRect->GetX2(), pDestRect->GetY2(),
      pSrceRect->GetWidth(), pSrceRect->GetHeight(), pDestRect->GetWidth(), pDestRect->GetHeight(),
      (uintptr_t)pSrceBuffer, (uintptr_t)pDestBuffer,
      Res == EG_RES_OK ? "OK!" : "FAILED!");
		return Res;
	};
	// Stage 3: Split the BLIT into multiple tiles
	VG_LITE_LOG_TRACE("Split "
    "Area: ([%d,%d], [%d,%d]) -> ([%d,%d], [%d,%d]) | "
    "Size: ([%dx%d] -> [%dx%d]) | "
    "Addr: (0x%x -> 0x%x)",
    pSrceRect->GetX1(), pSrceRect->GetY1(), pSrceRect->GetX2(), pSrceRect->GetY2(),
    pDestRect->GetX1(), pDestRect->GetY1(), pDestRect->GetX2(), pDestRect->GetY2(),
    pSrceRect->GetWidth(), pSrceRect->GetHeight(), pDestRect->GetWidth(), pDestRect->GetHeight(),
    (uintptr_t)pSrceBuffer, (uintptr_t)pDestBuffer);
	int32_t Width = GetWidth(pSrceRect);
	int32_t Height = GetHeight(pSrceRect);
	// Number of tiles needed
	int TotalTilesX = (pSrceRect->GetX1() + Width + EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1) / EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR;
	int TotalTilesY = (pSrceRect->GetY1() + Height + EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1) /	EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR;
	// src and dst buffer shift against each other. Src buffer real data [0,0] may start actually at [3,0] in buffer, as the
  // buffer pointer has to be aligned, while dst buffer real data [0,0] may start at [1,0] in buffer. alignment may be different */
	int SrceShiftX = (pSrceRect->GetX1() > pDestRect->GetX1()) ? (pSrceRect->GetX1() - pDestRect->GetX1()) : 0;
	int DestShiftX = (pSrceRect->GetX1() < pDestRect->GetX1()) ? (pSrceRect->GetX1() - pDestRect->GetX1()) : 0;
	VG_LITE_LOG_TRACE("X shift: src: %d, dst: %d", SrceShiftX, DestShiftX);
	EG_Color_t *pDestTileBuffer;
	EGRect DestTileRect;
	const EG_Color_t *pSrceTileBuffer;
	EGRect SrceTileRect;
	for(int y = 0; y < TotalTilesY; y++) {
		SrceTileRect.SetY1(0); // no vertical alignment, always start from 0
		SrceTileRect.SetY2(Height - y * EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1);
		if(SrceTileRect.GetY2() >= EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR) {
			SrceTileRect.SetY2(EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1); // Should never happen
		}
		pSrceTileBuffer = pSrceBuffer + y * EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR * SrceStep;
		DestTileRect.SetY1(SrceTileRect.GetY1()); // y has no alignment, always in sync with src
		DestTileRect.SetY2(SrceTileRect.GetY2());
		pDestTileBuffer = pDestBuffer + y * EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR * DestStep;
		for(int x = 0; x < TotalTilesX; x++) {
			if(x == 0) {
				/* 1st tile is special - there may be a gap between buffer start pointer
        * and Rect.x1 value, as the pointer has to be aligned.
        * pSrceTileBuffer pointer - keep init value from Y-loop.
        * Also, 1st tile start is not shifted! shift is applied from 2nd tile */
				SrceTileRect.SetX1(pSrceRect->GetX1());
				DestTileRect.SetX1(pDestRect->GetX1());
			}
			else {
				SrceTileRect.SetX1(SrceShiftX);				// subsequent tiles always starts from 0, but shifted
				DestTileRect.SetX1(DestShiftX);
				pSrceTileBuffer += EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR; // and advance start pointer + 1 tile size
				pDestTileBuffer += EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR;
			}
			// Clip tile end coordinates
			SrceTileRect.SetX2(Width + pSrceRect->GetX1() - x * EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1);
			if(SrceTileRect.GetX2() >= EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR) {
				SrceTileRect.SetX2(EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1);
			}
			DestTileRect.SetX2(Width + pDestRect->GetX1() - x * EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1);
			if(DestTileRect.GetX2() >= EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR) {
				DestTileRect.SetX2(EG_GPU_NXP_VG_LITE_BLIT_SPLIT_THR - 1);
			}
			if(x < (TotalTilesX - 1)) {
				// And adjust end coords if shifted, but not for last tile!
				SrceTileRect.IncX2(SrceShiftX);
				DestTileRect.IncX2(DestShiftX);
			}
			if(check_src_alignment(pSrceTileBuffer, SrceStep) != EG_RES_OK)	VG_LITE_RETURN_INV("Check src alignment failed.");
			// Set new dest buffer and srce buffer memory addresses.
			EG_VGliteSetDestBufferPtr(pDestTileBuffer);
			EG_VGliteSetSrceBuffferPtr(pSrceTileBuffer);
			EG_VGliteSetScissor(&DestTileRect);
			SetTranslationMatrix(&DestTileRect);
			Res = Blit(&SrceTileRect, OPA);
			EG_VGliteDisableScissor();
			VG_LITE_LOG_TRACE("Tile [%d, %d] "
        "Area: ([%d,%d], [%d,%d]) -> ([%d,%d], [%d,%d]) | "
        "Size: ([%dx%d] -> [%dx%d]) | "
        "Addr: (0x%x -> 0x%x) %s",
        x, y,
        SrceTileRect.GetX1(), SrceTileRect.GetY1(), SrceTileRect.GetX2(), SrceTileRect.GetY2(),
        DestTileRect.GetX1(), DestTileRect.GetY1(), DestTileRect.GetX2(), DestTileRect.GetY2(),
        SrceTileRect.GetWidth(), SrceTileRect.GetHeight(),
        DestTileRect.GetWidth(), DestTileRect.GetHeight(),
        (uintptr_t)pSrceTileBuffer, (uintptr_t)pDestTileBuffer,
        Res == EG_RES_OK ? "OK!" : "FAILED!");
			if(Res != EG_RES_OK) return Res;
		}
	}
	return Res;
}

#endif

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteBlend::Blit(const EGRect *pSrceRect, EG_OPA_t OPA)
{
VGLiteError_e Result = VG_LITE_SUCCESS;
VGLiteBuffer_t *pVGDestBuffer = EG_VGliteGetDestBuffer();
VGLiteBuffer_t *pVGSrceBuffer = EG_VGliteGetSrceBuffer();
uint32_t Color;
VGLiteBlend_e Blend;
uint32_t Rect[] = {
  (uint32_t)pSrceRect->GetX1(),                // start x
  (uint32_t)pSrceRect->GetY1(),                // start y
  (uint32_t)pSrceRect->GetWidth(),
  (uint32_t)pSrceRect->GetHeight()
};

	if(OPA >= (EG_OPA_t)EG_OPA_MAX) {
		Color = 0xFFFFFFFFU;
		Blend = VG_LITE_BLEND_SRC_OVER;
		pVGSrceBuffer->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
	}
	else {
		if(vg_lite_query_feature(gcFEATURE_BIT_VG_HW_PREMULTIPLY)) { //was gcFEATURE_BIT_VG_PE_PREMULTIPLY
			Color = (OPA << 24) | 0x00FFFFFFU;
		}
		else {
			Color = (OPA << 24) | (OPA << 16) | (OPA << 8) | OPA;
		}
		Blend = VG_LITE_BLEND_SRC_OVER;
		pVGSrceBuffer->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
		pVGSrceBuffer->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
	}
	Result = vg_lite_blit_rect(pVGDestBuffer, pVGSrceBuffer, (VGLiteRectangle_t*)Rect, &m_VGMatrix, Blend, Color, VG_LITE_FILTER_POINT);
	VG_LITE_ERR_RETURN_INV(Result, "Blit rectangle failed.");
	if(EG_VGliteRun() != EG_RES_OK)	VG_LITE_RETURN_INV("Run failed.");
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// No alignment requirement for destination pixel buffer when using mode VG_LITE_LINEAR
EG_Result_t EGVGLiteBlend::CheckSrceAlignment(const EG_Color_t *pSrceBuffer, int32_t SrceStep)
{
	if((((uintptr_t)pSrceBuffer) % (uintptr_t)EG_ATTRIBUTE_MEM_ALIGN_SIZE) != (uintptr_t)0x0U)	// Test for pointer alignment
		VG_LITE_RETURN_INV("Src buffer ptr (0x%x) not aligned to 0x%x bytes.", (size_t)pSrceBuffer, EG_ATTRIBUTE_MEM_ALIGN_SIZE);
	if((SrceStep % (int32_t)EG_GPU_NXP_VG_LITE_STRIDE_ALIGN_PX) != 0x0U)	// Test for Step alignment
		VG_LITE_RETURN_INV("Src buffer Step (%d px) not aligned to %d px.", SrceStep, EG_GPU_NXP_VG_LITE_STRIDE_ALIGN_PX);
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////////////////////

#if VG_LITE_BLIT_SPLIT_ENABLED
void EGVGLiteBlend::AlignX(EGRect *pRect, EG_Color_t **ppBuffer)
{
	int alignedAreaStartPx = pRect->GetX1() - (pRect->GetX1() % (EG_ATTRIBUTE_MEM_ALIGN_SIZE / sizeof(EG_Color_t)));
	VG_LITE_COND_STOP(alignedAreaStartPx < 0, "Negative X alignment.");
	pRect->GetX1()-= alignedAreaStartPx;
	pRect->GetX2() -= alignedAreaStartPx;
	*ppBuffer += alignedAreaStartPx;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteBlend::AlignY(EGRect *pRect, EG_Color_t **ppBuffer, int32_t Step)
{
int LineToAlignMem;
int alignedAreaStartPy;

	// find how many lines of pixels will respect memory alignment requirement
	if((Step % (int32_t)EG_ATTRIBUTE_MEM_ALIGN_SIZE) == 0x0U) {
		alignedAreaStartPy = pRect->GetY1();
	}
	else {
		LineToAlignMem = EG_ATTRIBUTE_MEM_ALIGN_SIZE / (EG_GPU_NXP_VG_LITE_STRIDE_ALIGN_PX * sizeof(EG_Color_t));
		VG_LITE_COND_STOP(EG_ATTRIBUTE_MEM_ALIGN_SIZE % (EG_GPU_NXP_VG_LITE_STRIDE_ALIGN_PX * sizeof(EG_Color_t)),
											"Complex case: need gcd function.");
		alignedAreaStartPy = pRect->GetY1() - (pRect->GetY1() % LineToAlignMem);
		VG_LITE_COND_STOP(alignedAreaStartPy < 0, "Negative Y alignment.");
	}
	pRect->GetY1() -= alignedAreaStartPy;
	pRect->GetY2() -= alignedAreaStartPy;
	*ppBuffer += (uint32_t)(alignedAreaStartPy * Step);
}

#endif

#endif
