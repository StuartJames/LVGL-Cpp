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
#include <math.h>
#include "draw/nxp/vglite/EG_VGLite_Blend.h"
#include "draw/nxp/vglite/EG_VGLite_Buffer.h"

#if EG_COLOR_DEPTH != 32
#include "core/EG_Refresh.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////

/* Minimum area (in pixels) for VG-Lite blit/fill processing. */
#ifndef EG_GPU_NXP_VG_LITE_SIZE_LIMIT
#define EG_GPU_NXP_VG_LITE_SIZE_LIMIT 5000
#endif

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::InitialiseContext(void)
{
  EGSoftContext::InitialiseContext();   // call the base class
	InitBufferProc = InitBuffer;
	WaitForFinishProc = WaitForFinish;
	BlendProc = EGVGLiteContext::Blend;
	DrawImageDecodedProc = DrawImageDecoded;
	DrawArcProc = DrawArc;
	DrawLineProc = DrawLine;
	DrawRectProc = DrawRect;
	CopyBufferProc = BufferCopy;
}

///////////////////////////////////////////////////////////////////////////// /////////

void EGVGLiteContext::InitBuffer(EGDeviceContext *pDC)
{
	EG_VGliteInitBuffer((EG_Color_t*)pDC->m_pDrawBuffer, pDC->m_pDrawRect, pDC->m_pDrawRect->GetWidth());
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::WaitForFinish(void)
{
	vg_lite_finish();
	SoftWaitForFinish();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::Blend(EGBlendBase *pBlend)
{
EGRect BlendRect;

	if(pBlend->m_OPA <= (EG_OPA_t)EG_OPA_MIN) return;
	if(NeedARGB8565Support()) {
		EGSoftBlend::BlendBasic(pBlend);
		return;
	}
  EGVGLiteContext *pDC = (EGVGLiteContext*)pBlend->m_pContext;
	if(!BlendRect.Intersect(pBlend->m_pRect, pDC->m_pClipRect)) return; // Fully clipped, nothing to do
	BlendRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	bool Done = false;
	// Fill/Blend only non masked, normal blended
	if(pBlend->m_pMaskBuffer == nullptr && pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL && BlendRect.GetSize() >= EG_GPU_NXP_VG_LITE_SIZE_LIMIT) {
		const EG_Color_t *pSrceBuffer = pBlend->m_pSourceBuffer;
		if(pSrceBuffer == nullptr) {
			Done = (EGVGLiteBlend::Fill(&BlendRect, pBlend->m_Color, pBlend->m_OPA) == EG_RES_OK);
			if(!Done)	VG_LITE_LOG_TRACE("VG-Lite fill failed. Fallback.");
		}
		else {
			EGRect SrceRect;
			SrceRect.SetX1(BlendRect.GetX1() - (pBlend->m_pRect->GetX1() - pDC->m_pDrawRect->GetX1()));
			SrceRect.SetY1(BlendRect.GetY1() - (pBlend->m_pRect->GetY1() - pDC->m_pDrawRect->GetY1()));
			SrceRect.SetX2(SrceRect.GetX1() + pBlend->m_pRect->GetWidth() - 1);
			SrceRect.SetY2(SrceRect.GetY1() + pBlend->m_pRect->GetHeight() - 1);
			int32_t SrceStep = pBlend->m_pRect->GetWidth();
#if VG_LITE_BLIT_SPLIT_ENABLED
			EG_Color_t *pDestBuffer = m_pDrawBuffer;
			int32_t DestStep = m_pDrawRect.GetWidth();
			Done = (BlitSplitGPU(pDestBuffer, &BlendRect, DestStep, pSrceBuffer, &SrceRect, SrceStep, pBlend->m_OPA) == EG_RES_OK);
#else
			Done = (EGVGLiteBlend::BlitGPU(&BlendRect, pSrceBuffer, &SrceRect, SrceStep, pBlend->m_OPA) == EG_RES_OK);
#endif
			if(!Done)	VG_LITE_LOG_TRACE("VG-Lite blit failed. Fallback.");
		}
	}
	if(!Done) EGSoftBlend::BlendBasic((EGBlendBase*)pBlend);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat)
{
	if(pDrawImage->m_OPA <= (EG_OPA_t)EG_OPA_MIN) return;
	if(NeedARGB8565Support()) {
		EGSoftContext::DrawImageDecoded(pDrawImage, pRect, pSourceBuffer, ColorFormat);
		return;
	}
	const EG_Color_t *pSrceBuffer = (const EG_Color_t *)pSourceBuffer;
	if(!pSrceBuffer) {
		EGSoftContext::DrawImageDecoded(pDrawImage, pRect, pSourceBuffer, ColorFormat);
		return;
	}
  EGVGLiteContext *pDC = (EGVGLiteContext*)pDrawImage->m_pContext;
	EGRect RelRect(pRect);
	RelRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	EGRect RelClipRect(pDC->m_pClipRect);
	RelClipRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	EGRect BlendRect;
	bool HasTransform = pDrawImage->m_Angle != 0 || pDrawImage->m_Scale.IsScaled();
	if(HasTransform) BlendRect = RelRect;
	else if(!BlendRect.Intersect(&RelRect, &RelClipRect)) return; /*Fully clipped, nothing to do*/
	bool HasMask = HasAnyDrawMask(&BlendRect);
	bool HasRecolor = (pDrawImage->m_RecolorOPA != EG_OPA_TRANSP);
	bool Done = false;
	if(!HasMask && !HasRecolor && !pDrawImage->IsChromaKeyed(ColorFormat) && BlendRect.GetSize() >= EG_GPU_NXP_VG_LITE_SIZE_LIMIT
#if EG_COLOR_DEPTH != 32
		 && !EGDrawImage::HasAlpha(ColorFormat)
#endif
		) {
		EGRect SrceRect;
    SrceRect.SetX1(BlendRect.GetX1() - (pRect->GetX1() - pDC->m_pDrawRect->GetX1()));
    SrceRect.SetX2(SrceRect.GetX1() + pRect->GetWidth() - 1);
		SrceRect.SetY1(BlendRect.GetY1() - (pRect->GetY1() - pDC->m_pDrawRect->GetY1()));
		SrceRect.SetY2(SrceRect.GetY1() + pRect->GetHeight() - 1);
		int32_t SrceStep = pRect->GetWidth();

#if VG_LITE_BLIT_SPLIT_ENABLED
		EG_Color_t *pDestBuffer = draw_ctx->buf;
		int32_t DestStep = m_pDrawRect->GetWidth();
		if(HasTransform)	Done = false;	// VG-Lite blit split with transformation is not supported!
		else Done = (BlitSplitGPU(pDestBuffer, &BlendRect, DestStep, pSrceBuffer, &SrceRect, SrceStep, pDrawImage->OPA) == EG_RES_OK);
#else
		if(HasTransform)	Done = (EGVGLiteBlend::BlitTransformGPU(&BlendRect, &RelClipRect, pSrceBuffer, &SrceRect, SrceStep, pDrawImage) == EG_RES_OK);
		else Done = (EGVGLiteBlend::BlitGPU(&BlendRect, pSrceBuffer, &SrceRect, SrceStep, pDrawImage->m_OPA) == EG_RES_OK);
#endif
		if(!Done)	VG_LITE_LOG_TRACE("VG-Lite blit %sfailed. Fallback.", HasTransform ? "transform " : "");
	}
	if(!Done)	EGSoftContext::DrawImageDecoded(pDrawImage, pRect, pSourceBuffer, ColorFormat);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::BufferCopy(void *pDestBuffer, int32_t DestStep, EGRect *pDestRect, void *pSourceBuffer, int32_t SourceStep, EGRect *pSourceRect)
{
bool Done = false;

	if(pDestRect->GetSize() >= EG_GPU_NXP_VG_LITE_SIZE_LIMIT) {
		Done = EGVGLiteBlend::BufferCopyGPU((EG_Color_t*)pDestBuffer, pDestRect, DestStep, (EG_Color_t*)pSourceBuffer, pSourceRect, SourceStep);
		if(!Done)	VG_LITE_LOG_TRACE("VG-Lite buffer copy failed. Fallback.");
	}
	if(!Done) EGSoftContext::BufferCopy(pDestBuffer, DestStep, pDestRect, pSourceBuffer, SourceStep, pSourceRect);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::DrawLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
	if(pDrawLine->m_Width == 0)	return;
	if(pDrawLine->m_OPA <= (EG_OPA_t)EG_OPA_MIN) return;
	if(pPoint1->m_X == pPoint2->m_X && pPoint1->m_Y == pPoint2->m_Y) return;
	if(NeedARGB8565Support()) {
		EGSoftContext::DrawLine(pDrawLine, pPoint1, pPoint2);
		return;
	}
  EGVGLiteContext *pDC = (EGVGLiteContext*)pDrawLine->m_pContext;
	EGRect RelClipRect(EG_MIN(pPoint1->m_X, pPoint2->m_X) - pDrawLine->m_Width / 2, EG_MIN(pPoint1->m_Y, pPoint2->m_Y) - pDrawLine->m_Width / 2,
	                   EG_MAX(pPoint1->m_X, pPoint2->m_X) + pDrawLine->m_Width / 2, EG_MAX(pPoint1->m_Y, pPoint2->m_Y) + pDrawLine->m_Width / 2);
	EGRect ClipRect;
	if(!ClipRect.Intersect(&RelClipRect, pDC->m_pClipRect))	return; // Fully clipped, nothing to do
	RelClipRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	EGPoint RelPoint1(pPoint1->m_X - pDC->m_pDrawRect->GetX1(), pPoint1->m_Y - pDC->m_pDrawRect->GetY1());
	EGPoint RelPoint2(pPoint2->m_X - pDC->m_pDrawRect->GetX1(), pPoint2->m_Y - pDC->m_pDrawRect->GetY1());
	bool Done = false;
	bool HasMask = HasAnyDrawMask(&RelClipRect);
	if(!HasMask) {
		Done = (pDC->DrawLineGPU(&RelPoint1, &RelPoint2, &RelClipRect, pDrawLine) == EG_RES_OK);
		if(!Done)	VG_LITE_LOG_TRACE("VG-Lite draw line failed. Fallback.");
	}
	if(!Done) EGSoftContext::DrawLine(pDrawLine, pPoint1, pPoint2);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::DrawRect(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
	if(NeedARGB8565Support()) {
		EGSoftContext::DrawRect(pDrawRect, pRect);
		return;
	}
	EGDrawRect VGDrawRect;
  VGDrawRect = pDrawRect;
	VGDrawRect.m_BackgroundOPA = 0;
	VGDrawRect.m_BackImageOPA = 0;
	VGDrawRect.m_BorderOPA = 0;
	VGDrawRect.m_OutlineOPA = 0;
  EGVGLiteContext *pDC = (EGVGLiteContext*)pDrawRect->m_pContext;
#if EG_DRAW_COMPLEX
  EGSoftContext::DrawRect(&VGDrawRect, pRect);	// Draw the shadow with CPU
	VGDrawRect.m_ShadowOPA = 0;
#endif
	VGDrawRect.m_BackgroundOPA = pDrawRect->m_BackgroundOPA;	// Draw the background
	if(pDC->DrawBackground(&VGDrawRect, pRect) != EG_RES_OK)	EGSoftContext::DrawRect(&VGDrawRect, pRect);
	VGDrawRect.m_BackgroundOPA = 0;
	// Draw the background image using EGSoftContext::DrawImage callback gets called from EGSoftContext::DrawRect().
	VGDrawRect.m_BackImageOPA = pDrawRect->m_BackImageOPA;
	EGSoftContext::DrawRect(&VGDrawRect, pRect);
	VGDrawRect.m_BackImageOPA = 0;
	VGDrawRect.m_BorderOPA = pDrawRect->m_BorderOPA;	// Draw the border
	if(pDC->DrawBorder(&VGDrawRect, pRect) != EG_RES_OK)	EGSoftContext::DrawRect(&VGDrawRect, pRect);
	VGDrawRect.m_BorderOPA = 0;
	VGDrawRect.m_OutlineOPA = pDrawRect->m_OutlineOPA;	// Draw the outline
	if(pDC->DrawOutline(&VGDrawRect, pRect) != EG_RES_OK) EGSoftContext::DrawRect(&VGDrawRect, pRect);
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteContext::DrawBackground(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
	if(pDrawRect->m_BackgroundOPA <= (EG_OPA_t)EG_OPA_MIN)	return EG_RES_INVALID;
	EGRect RelRect(pRect);
	// If the border fully covers make the bg area 1px smaller to avoid artifacts on the corners
	if(pDrawRect->m_BorderWidth > 1 && pDrawRect->m_BorderOPA >= (EG_OPA_t)EG_OPA_MAX && pDrawRect->m_Radius != 0) {
		RelRect.Deflate((pDrawRect->m_BorderSide & EG_BORDER_SIDE_LEFT) ? 1 : 0,	(pDrawRect->m_BorderSide & EG_BORDER_SIDE_RIGHT) ? 1 : 0,
		                (pDrawRect->m_BorderSide & EG_BORDER_SIDE_TOP) ? 1 : 0, (pDrawRect->m_BorderSide & EG_BORDER_SIDE_BOTTOM) ? 1 : 0);
	}
	RelRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());
	EGRect RelClipRect(m_pClipRect);
	RelClipRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());
	EGRect ClipRect;
	if(!ClipRect.Intersect(&RelRect, &RelClipRect))	return EG_RES_OK; // Fully clipped, nothing to do
	bool HasMask = HasAnyDrawMask(&RelRect);
	EG_GradDirection_e GradDir = pDrawRect->m_BackgroundGrad.dir;
	EG_Color_t BackColor = (GradDir == (EG_GradDirection_e)EG_GRAD_DIR_NONE) ?	pDrawRect->m_BackgroundColor :	pDrawRect->m_BackgroundGrad.stops[0].color;
	if(BackColor.full == pDrawRect->m_BackgroundGrad.stops[1].color.full)	GradDir = EG_GRAD_DIR_NONE;
	// Most simple case: just a plain rectangle (no mask, no m_Radius, no gradient) shall be handled by draw_ctx->blend().
  // Complex case: gradient or m_Radius but no mask.
	if(!HasMask && ((pDrawRect->m_Radius != 0) || (GradDir != (EG_GradDirection_e)EG_GRAD_DIR_NONE))) {
		EG_Result_t Res = DrawBackgroundGPU(&RelRect, &RelClipRect, pDrawRect);
		if(Res != EG_RES_OK) VG_LITE_LOG_TRACE("VG-Lite draw bg failed. Fallback.");
		return Res;
	}
	return EG_RES_INVALID;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteContext::DrawBorder(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
	if(pDrawRect->m_BorderOPA <= (EG_OPA_t)EG_OPA_MIN)	return EG_RES_INVALID;
	if(pDrawRect->m_BorderWidth == 0) return EG_RES_INVALID;
	if(pDrawRect->m_BorderPost) return EG_RES_INVALID;
	if(pDrawRect->m_BorderSide != (EG_BorderSide_t)EG_BORDER_SIDE_FULL) return EG_RES_INVALID;
	int32_t m_BorderWidth = pDrawRect->m_BorderWidth;
	// Move border inwards to align with software rendered border
	EGRect RelRect(pRect->GetX1() + ceil(m_BorderWidth / 2.0f), pRect->GetY1() + ceil(m_BorderWidth / 2.0f),
                 pRect->GetX2() - floor(m_BorderWidth / 2.0f), pRect->GetY2() - floor(m_BorderWidth / 2.0f));
  RelRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());
	EGRect RelClipRect(m_pClipRect);
	RelClipRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());
	EGRect ClipRect;
	if(!ClipRect.Intersect(&RelRect, &RelClipRect))	return EG_RES_OK; // Fully clipped, nothing to do
	EG_Result_t Res = DrawBorderGeneric(&RelRect, &RelClipRect, pDrawRect, true);
	if(Res != EG_RES_OK) VG_LITE_LOG_TRACE("VG-Lite draw border failed. Fallback.");
	return Res;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGVGLiteContext::DrawOutline(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
	if(pDrawRect->m_BorderOPA <= (EG_OPA_t)EG_OPA_MIN) return EG_RES_INVALID;
	if(pDrawRect->m_BorderWidth == 0)	return EG_RES_INVALID;
	// Move outline outwards to align with software rendered outline
	int32_t OutlinePad = pDrawRect->m_OutlinePadding - 1;
	EGRect RelRect(pRect->GetX1() - OutlinePad - floor(pDrawRect->m_OutlineWidth / 2.0f), pRect->GetY1() - OutlinePad - floor(pDrawRect->m_OutlineWidth / 2.0f),
	                pRect->GetX2() + OutlinePad + ceil(pDrawRect->m_OutlineWidth / 2.0f), pRect->GetY2() + OutlinePad + ceil(pDrawRect->m_OutlineWidth / 2.0f));
  RelRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());
	EGRect RelClipRect(m_pClipRect);
	RelClipRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());
	EGRect ClipRect;
	if(!ClipRect.Intersect(&RelRect, &RelClipRect))	return EG_RES_OK; // Fully clipped, nothing to do
	EG_Result_t Res = DrawBorderGeneric(&RelRect, &RelClipRect, pDrawRect, false);
	if(Res != EG_RES_OK) VG_LITE_LOG_TRACE("VG-Lite draw outline failed. Fallback.");
	return Res;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGVGLiteContext::DrawArc(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle)
{
bool Done = false;

#if EG_DRAW_COMPLEX
	if(pDrawArc->m_OPA <= (EG_OPA_t)EG_OPA_MIN) return;
	if(pDrawArc->m_Width == 0)	return;
	if(StartAngle == EndAngle) return;
	if(NeedARGB8565Support()) {
		EGSoftContext::DrawArc(pDrawArc, pCenter, Radius, StartAngle, EndAngle);
		return;
	}
  EGVGLiteContext *pDC = (EGVGLiteContext*)pDrawArc->m_pContext;
	EGPoint RelCenter(pCenter->m_X - pDC->m_pDrawRect->GetX1(), pCenter->m_Y - pDC->m_pDrawRect->GetY1());
	EGRect RelClipRect(pDC->m_pClipRect);
	RelClipRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	bool HasMask = HasAnyDrawMask(&RelClipRect);
	if(!HasMask) {
		Done = (pDC->DrawArcGPU(&RelCenter, (int32_t)Radius, (int32_t)StartAngle, (int32_t)EndAngle, &RelClipRect, pDrawArc) == EG_RES_OK);
		if(!Done)	VG_LITE_LOG_TRACE("VG-Lite draw arc failed. Fallback.");
	}
#endif
	if(!Done) EGSoftContext::DrawArc(pDrawArc, pCenter, Radius, StartAngle, EndAngle);
}

#endif
