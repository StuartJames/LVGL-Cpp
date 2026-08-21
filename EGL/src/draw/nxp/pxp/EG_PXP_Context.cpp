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

#include "draw/nxp/pxp/EG_PXP_Context.h"

#if EG_USE_GPU_NXP_PXP

#include "draw/nxp/pxp/EG_PXP_Blend.h"

//////////////////////////////////////////////////////////////////////

/* Minimum area (in pixels) for PXP blit/fill processing. */
#ifndef EG_GPU_NXP_PXP_SIZE_LIMIT
#define EG_GPU_NXP_PXP_SIZE_LIMIT 5000
#endif

//////////////////////////////////////////////////////////////////////

void EGPXPContext::InitialiseContext(void)
{
  EGSoftContext::InitialiseContext();   // call the base class
	DrawImageDecodedProc = DrawImageDecoded;
	BlendProc = Blend;
	WaitForFinishProc = WaitForFinish;
	CopyBufferProc = BufferCopy;
}

//////////////////////////////////////////////////////////////////////

void EGPXPContext::WaitForFinish(void)
{
	PXP_WaitGPU();
	SoftWaitForFinish();
}

//////////////////////////////////////////////////////////////////////

void EGPXPContext::Blend(EGBlendBase *pBlend)
{
	if(pBlend->m_OPA <= (EG_OPA_t)EG_OPA_MIN) return;
	if(NeedARGB8565Support()) {
		EGSoftBlend::BlendBasic(pBlend);
		return;
	}

	EGRect BlendRect;
	// Let's get the blend area which is the intersection of the area to draw and the clip area
  EGPXPContext *pDC = (EGPXPContext*)pBlend->m_pContext;
	if(!BlendRect.Intersect(pBlend->m_pRect, pDC->m_pClipRect)) return; // Fully clipped, nothing to do
	BlendRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1()); // Make the blend area relative to the buffer
	if(pBlend->m_pMaskBuffer != nullptr || pBlend->m_BlendMode != EG_BLEND_MODE_NORMAL || BlendRect.GetSize() < EG_GPU_NXP_PXP_SIZE_LIMIT) {
		EGSoftBlend::BlendBasic(pBlend);
		return;
	}
	EG_Color_t *pDestBuffer = (EG_Color_t*)pDC->m_pDrawBuffer;	// Fill/Blend only non masked, normal blended
	int32_t DestStep = pDC->m_pDrawRect->GetWidth();
	const EG_Color_t *pSourceBuffer = pBlend->m_pSourceBuffer;
	if(pSourceBuffer == nullptr) {
		EGPXPBlend::Fill(pDestBuffer, &BlendRect, DestStep, pBlend->m_Color, pBlend->m_OPA);
	}
	else {
		EGRect SourceRect;
		SourceRect.SetX1(BlendRect.GetX1() - (pBlend->m_pRect->GetX1() - pDC->m_pDrawRect->GetX1()));
		SourceRect.SetY1(BlendRect.GetY1() - (pBlend->m_pRect->GetY1() - pDC->m_pDrawRect->GetY1()));
		SourceRect.SetX2(SourceRect.GetX1() + pBlend->m_pRect->GetWidth() - 1);
		SourceRect.SetY2(SourceRect.GetY1() + pBlend->m_pRect->GetHeight() - 1);
		int32_t SourceStep = pBlend->m_pRect->GetWidth();
		EGPXPBlend::Blit(pDestBuffer, &BlendRect, DestStep, pSourceBuffer, &SourceRect, SourceStep,	pBlend->m_OPA, EG_DISP_ROT_NONE);
	}
}

//////////////////////////////////////////////////////////////////////

void EGPXPContext::DrawImageDecoded(const EGDrawImage *pImage, const EGRect *pRect, const uint8_t *pSourceMap, EG_ImageColorFormat_t cf)
{
	if(pImage->m_OPA <= (EG_OPA_t)EG_OPA_MIN) return;
	if(NeedARGB8565Support()) {
		EGSoftContext::DrawImageDecoded(pImage, pRect, pSourceMap, cf);
		return;
	}
	const EG_Color_t *pSourceBuffer = (const EG_Color_t *)pSourceMap;
	if(!pSourceBuffer) {
		EGSoftContext::DrawImageDecoded(pImage, pRect, pSourceMap, cf);
		return;
	}
	EGRect RelRect(pRect);
  EGPXPContext *pDC = (EGPXPContext*)pImage->m_pContext;
	RelRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	EGRect RelClip(pDC->m_pClipRect);
	RelClip.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	bool HasScale = (pImage->m_Scale.IsScaled());
	bool HasRotation = (pImage->m_Angle != 0);
	bool Unsupported = false;
	EGRect BlendRect;
	if(HasRotation) BlendRect.Copy(&RelRect);
	else if(!BlendRect.Intersect(&RelRect, &RelClip)) return; // Fully clipped, nothing to do
	bool HasMask = HasAnyDrawMask(&BlendRect);
	int32_t SourceWidth = pRect->GetWidth();
	int32_t SourceHeight = pRect->GetHeight();
	if(HasRotation) { // PXP can only rotate at 90x angles.
		if(pImage->m_Angle % 900) {
			PXP_LOG_TRACE("Rotation angle %d is not supported. PXP can rotate only 90x angle.", pImage->m_Angle);
			Unsupported = true;
		}
		/* PXP is set to process 16x16 blocks to optimize the system for memory
     * bandwidth and image processing time.
     * The output engine essentially truncates any output pixels after the
     * desired number of pixels has been written.
     * When rotating a source image and the output is not divisible by the block
     * size, the incorrect pixels could be truncated and the final output image
     *  can look shifted.*/
		if(SourceWidth % 16 || SourceHeight % 16) {
			PXP_LOG_TRACE("Rotation is not supported for image w/o alignment to block size 16x16.");
			Unsupported = true;
		}
	}
	if(HasMask || HasScale || Unsupported || (BlendRect.GetSize() < EG_GPU_NXP_PXP_SIZE_LIMIT)
#if EG_COLOR_DEPTH != 32
		|| pImage->HasAlpha(cf)
#endif
			) {
        EGSoftContext::DrawImageDecoded(pImage, pRect, pSourceMap, cf);
		return;
	}
	EG_Color_t *DestBuffer = (EG_Color_t*)pDC->m_pDrawBuffer;
	int32_t DestStep = pDC->m_pDrawRect->GetWidth();
	EGRect SourceRect;
	SourceRect.SetX1(BlendRect.GetX1() - (pRect->GetX1() - pDC->m_pDrawRect->GetX1()));
	SourceRect.SetY1(BlendRect.GetY1() - (pRect->GetY1() - pDC->m_pDrawRect->GetY1()));
	SourceRect.SetX2(SourceRect.GetX1() + SourceWidth - 1);
	SourceRect.SetY2(SourceRect.GetY1() + SourceHeight - 1);
	int32_t SourceStep = pRect->GetWidth();
	EGPXPBlend::BlitTransform(DestBuffer, &BlendRect, DestStep, pSourceBuffer, &SourceRect, SourceStep,	pImage, cf);
}

//////////////////////////////////////////////////////////////////////

void EGPXPContext::BufferCopy(void *pDestBuffer, int32_t DestStep, EGRect *pDestRect,
																		void *pSourceBuffer, int32_t SourceStep, EGRect *pSourceRect)
{
	if(pDestRect->GetSize() < EG_GPU_NXP_PXP_SIZE_LIMIT) {
		EGSoftContext::BufferCopy(pDestBuffer, DestStep, pDestRect, pSourceBuffer, SourceStep, pSourceRect);
		return;
	}
	EGPXPBlend::BufferCopy((EG_Color_t*)pDestBuffer, pDestRect, DestStep, (EG_Color_t*)pSourceBuffer, pSourceRect, SourceStep);
}

#endif
