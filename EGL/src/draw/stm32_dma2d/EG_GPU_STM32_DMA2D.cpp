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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#include "draw/stm32_dma2d/EG_GPU_STM32_DMA2D.h"
#include "core/EG_Refresh.h"

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_GPU_STM32_DMA2D

#if EG_COLOR_16_SWAP
// Note: DMA2D red/blue swap (RBS) works for all color modes
#define RBS_BIT 1U
#else
#define RBS_BIT 0U
#endif

#define CACHE_ROW_SIZE 32U  // cache row size in Bytes

// For code/implementation discussion refer to https://github.com/lvgl/lvgl/issues/3714#issuecomment-1365187036
// astyle --options=lvgl/scripts/code-format.cfg --ignore-exclude-errors lvgl/pSrceBuffer/draw/stm32_dma2d/*.c lvgl/pSrceBuffer/draw/stm32_dma2d/*.h

#if EG_COLOR_DEPTH == 16
const DMA2DColorFormat_e EG2DColorFormat = RGB565;
#elif EG_COLOR_DEPTH == 32
const DMA2DColorFormat_e EG2DColorFormat = ARGB8888;
#else
#error "Cannot use DMA2D with EG_COLOR_DEPTH other than 16 or 32"
#endif

///////////////////////////////////////////////////////////////////////////////////////

bool EGSTM32Context::m_IsDMAInProgess = false;  // indicates whether DMA2D transfer *initiated here* is in progress

///////////////////////////////////////////////////////////////////////////////////////

// Turn on the peripheral and set output color mode, this only needs to be done once
void EGSTM32Context::InitialiseDMA2D(void)
{
// Enable DMA2D clock
#if defined(STM32F4) || defined(STM32F7) || defined(STM32U5)
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;  // enable DMA2D
	// wait for hardware access to complete
	__asm volatile("DSB\n");
	volatile uint32_t temp = RCC->AHB1ENR;
	EG_UNUSED(temp);
#elif defined(STM32H7)
	RCC->AHB3ENR |= RCC_AHB3ENR_DMA2DEN;
	// wait for hardware access to complete
	__asm volatile("DSB\n");
	volatile uint32_t temp = RCC->AHB3ENR;
	EG_UNUSED(temp);
#else
#warning "EGL failed to enable the clock of DMA2D"
#endif
	// AHB master timer configuration
	DMA2D->AMTCR = 0;  // AHB bus guaranteed dead time disabled
#if defined(EG_STM32_DMA2D_TEST)
	InitialiseDWT();  // init µs timer
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::InitialiseContext(void)
{
  EGSoftContext::InitialiseContext();   // call the base class
	BlendProc = Blend;
	DrawImageDecodedProc = DrawImageDecoded;
//  DrawImageProc = DrawImage;
  CopyBufferProc = BufferCopy;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::Blend(EGBlendBase *pBlend)
{
	if(pBlend->m_BlendMode != EG_BLEND_MODE_NORMAL) {
		EGSoftBlend::BlendBasic(pBlend);
		return;
	}
  EGSTM32Context *pDC = (EGSTM32Context*)pBlend->m_pContext;
	// Note: x1 must be zero. Otherwise, there is no way to correctly calculate DestStep.
	// EG_ASSERT_MSG(m_pDrawRect->x1 == 0); // critical?
	// Both draw buffer start address and buffer size *must* be 32-byte aligned since draw buffer cache is being invalidated.
	// uint32_t drawBufferLength = m_pDrawRect->GetSize() * sizeof(EG_Color_t);
	// EG_ASSERT_MSG(drawBufferLength % CACHE_ROW_SIZE == 0); // critical, but this is not the way to test it
	// EG_ASSERT_MSG((uint32_t)m_pDrawBuffer % CACHE_ROW_SIZE == 0, "draw_ctx.buf is not 32B aligned"); // critical?

	if(pBlend->m_pSourceBuffer != nullptr) {
		// For performance reasons, both source buffer start address and buffer size *should* be 32-byte aligned since source buffer cache is being cleaned.
		// uint32_t srcBufferLength = pBlend->m_pRect->GetSize() * sizeof(EG_Color_t);
		// EG_ASSERT_MSG(srcBufferLength % CACHE_ROW_SIZE == 0); // FIXME: assert fails (performance, non-critical)
		// EG_ASSERT_MSG((uint32_t)pBlend->m_pSourceBuffer % CACHE_ROW_SIZE == 0); // FIXME: assert fails (performance, non-critical)
	}

	EGRect DrawRect;
	if(!DrawRect.Intersect(pBlend->m_pRect, pDC->m_pClipRect)) return;
	// + m_pDrawRect has the entire draw buffer location
	// + draw_ctx->clip_area has the current draw buffer location
	// + pBlend->m_pRect has the location of the area intended to be painted - image etc.
	// + DrawRect has the area actually being painted
	// All coordinates are relative to the screen.

	const EG_OPA_t *pMask = pBlend->m_pMaskBuffer;
	if(pBlend->m_pMaskBuffer && pBlend->m_MaskResult == EG_DRAW_MASK_RESULT_TRANSP) return;
	else if(pBlend->m_MaskResult == EG_DRAW_MASK_RESULT_FULL_COVER)	pMask = nullptr;
	int32_t DestStep = pDC->m_pDrawRect->GetWidth();
	if(pMask != nullptr) {
		// For performance reasons, both mask buffer start address and buffer size *should* be 32-byte aligned since mask buffer cache is being cleaned.
		// uint32_t srcBufferLength = pBlend->m_pMaskRect->GetSize() * sizeof(EG_OPA_t);
		// EG_ASSERT_MSG(srcBufferLength % CACHE_ROW_SIZE == 0); // FIXME: assert fails (performance, non-critical)
		// EG_ASSERT_MSG((uint32_t)pMask % CACHE_ROW_SIZE == 0); // FIXME: assert fails (performance, non-critical)
		int32_t MaskStep = pBlend->m_pMaskRect->GetWidth();
		EGPoint MaskOffset = pBlend->m_pMaskRect->GetOffset(&DrawRect);  // mask offset in relation to DrawRect
		if(pBlend->m_pSourceBuffer == nullptr) {  // 93.5%
			DrawRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
			pDC->BlendPaint((EG_Color_t*)pDC->m_pDrawBuffer, DestStep, &DrawRect, pMask, MaskStep, &MaskOffset, pBlend->m_Color, pBlend->m_OPA);
		}
		else {  // 0.2%
						// note: (x)RGB pBlend->m_pSourceBuffer does not carry alpha channel bytes,
						// alpha channel bytes are carried in pBlend->m_pMaskBuffer
#if EG_COLOR_DEPTH == 32
			int32_t SrceStep = pBlend->m_pRect->GetWidth();
			EGPoint SrceOffset = pBlend->m_pRect->GetOffset( &DrawRect);  // source image offset in relation to DrawRect
			int32_t DrawWidth = DrawRect.GetWidth();
			int32_t DrawHeight = DrawRect.GetHeight();
			// merge mask alpha bytes with pSrceBuffer RGB bytes
			// TODO: optimize by reading 4 or 8 mask bytes at a time
			pMask += (MaskStep * MaskOffset.y) + MaskOffset.x;
			EG_Color_t *pSourceBuffer = (EG_Color_t *)pBlend->m_pSourceBuffer;
			pSourceBuffer += (SrceStep * SrceOffset.y) + SrceOffset.x;
			uint16_t mask_buffer_offset = MaskStep - DrawWidth;
			uint16_t src_buffer_offset = SrceStep - DrawWidth;
			while(DrawHeight > 0) {
				DrawHeight--;
				for(uint16_t x = 0; x < DrawWidth; x++) {
					(*pSourceBuffer).ch.alpha = *pMask;
					pSourceBuffer++;
					pMask++;
				}
				pMask += mask_buffer_offset;
				pSourceBuffer += src_buffer_offset;
			}
			DrawRect.Move(-m_pDrawRect->GetX1(), -m_pDrawRect->GetY1());  // translate the screen draw area to the origin of the buffer area
			BlendMap((EG_Color_t*)m_pDrawBuffer, DestStep, &DrawRect, pBlend->m_pSourceBuffer, SrceStep, &SrceOffset, pBlend->m_OPA, ARGB8888, false);
#else
			// Note: 16-bit bitmap hardware blending with mask and background is possible, but requires a temp 24 or 32-bit buffer to combine bitmap with mask first.
			EGSoftBlend::BlendBasic(pBlend);  // (e.g. Shop Items)
// clean cache after software drawing - this does not help since this is not the only place where buffer is written without dma2d
// int32_t DrawWidth = DrawRect.GetWidth();
// int32_t DrawHeight = DrawRect.GetHeight();
// uint32_t dest_address = (uint32_t)(m_pDrawBuffer + (DestStep * DrawRect.y1) + DrawRect.x1);
// EGSTM32Context::CleanCache(dest_address, DestStep - DrawWidth, DrawWidth, DrawHeight, sizeof(EG_Color_t));
#endif
		}
	}
	else {
		if(pBlend->m_pSourceBuffer == nullptr) {  // 6.1%
			DrawRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());  // translate the screen draw area to the origin of the buffer area
			pDC->BlendFill((EG_Color_t*)pDC->m_pDrawBuffer, DestStep, &DrawRect, pBlend->m_Color, pBlend->m_OPA);
		}
		else {  // 0.2%
			int32_t SrceStep = pBlend->m_pRect->GetWidth();
			EGPoint SrceOffset = pBlend->m_pRect->GetOffset(&DrawRect);  // source image offset in relation to DrawRect
			DrawRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());  // translate the screen draw area to the origin of the buffer area
			pDC->BlendMap((EG_Color_t*)pDC->m_pDrawBuffer, DestStep, &DrawRect, pBlend->m_pSourceBuffer, SrceStep, &SrceOffset, pBlend->m_OPA, EG2DColorFormat, true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

// See: https://github.com/lvgl/lvgl/issues/3714#issuecomment-1331710788
void EGSTM32Context::BufferCopy(void *pDestBuffer, int32_t DestStep, EGRect *pDestRect, void *pSrceBuffer, int32_t SrceStep, EGRect *pSrceRect)
{
	// Both draw buffer start address and buffer size *must* be 32-byte aligned since draw buffer cache is being invalidated.
	// uint32_t drawBufferLength = m_pDrawRect->GetSize() * sizeof(EG_Color_t);
	// EG_ASSERT_MSG(drawBufferLength % CACHE_ROW_SIZE == 0); // critical, but this is not the way to test it
	// EG_ASSERT_MSG((uint32_t)m_pDrawBuffer % CACHE_ROW_SIZE == 0, "draw_ctx.buf is not 32B aligned"); // critical?
	// FIXME:
	// 1. Both pSourceBuffer and pDestBuffer pixel size *must* be known to use DMA2D.
	// 2. Verify both buffers start addresses and lengths are 32-byte (cache row size) aligned.
	EGPoint SrceOffset = pSrceRect->GetOffset(pDestRect);
	// FIXME: use pDestRect->Move(-dest_area->x1, -dest_area->y1) here ?
	// TODO: It is assumed that pDestBuffer and pSourceBuffer buffers are of EG_Color_t type. Verify it, this assumption may be incorrect.
	BlendMap((const EG_Color_t *)pDestBuffer, DestStep, pDestRect, (const EG_Color_t *)pSrceBuffer, SrceStep, &SrceOffset, 0xff, EG2DColorFormat, true);
	// TODO: Investigate if output buffer cache needs to be invalidated. It depends on what the destination buffer is and how it is used next - by dma2d or not.
	WaitTransferFinish(nullptr);  // TODO: is this line needed here?
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat)
{
  EGSTM32Context *pDC = (EGSTM32Context*)pDrawImage->m_pContext;
  EGRect DrawRect(pDC->m_pClipRect);
  bool HasMask = HasAnyDrawMask(pDC->m_pDrawRect);
  bool Transform = ((pDrawImage->m_Angle != 0) || pDrawImage->m_Scale.IsScaled());
	const DMA2DColorFormat_e BitmapColorFormat = ColorToDMA2dColor(ColorFormat);
	const bool IgnoreAlpha = (ColorFormat == EG_COLOR_FORMAT_RGBX8888);
	if(!HasMask && !Transform && BitmapColorFormat != UNSUPPORTED && pDrawImage->m_RecolorOPA == EG_OPA_TRANSP) {
		// simple bitmap blending, optionally with supported color format conversion - handle directly by dma2d
		int32_t DestStep = pDC->m_pDrawRect->GetWidth();
		int32_t SrceStep = pRect->GetWidth();
		EGPoint SrceOffset = pRect->GetOffset(&DrawRect);  // source image offset in relation to DrawRect
		DrawRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
		pDC->BlendMap((EG_Color_t*)pDC->m_pDrawBuffer, DestStep, &DrawRect, pSourceBuffer, SrceStep, &SrceOffset, pDrawImage->m_OPA, BitmapColorFormat, IgnoreAlpha);
	}
	else {
		// the rest are complex cases which require additional image transformations
		EGSoftContext::DrawImageDecoded(pDrawImage, pRect, pSourceBuffer, ColorFormat);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

DMA2DColorFormat_e EGSTM32Context::ColorToDMA2dColor(EG_ImageColorFormat_t ColorFormat)
{
	switch(ColorFormat) {
		case EG_COLOR_FORMAT_RGBA8888:			// note: EG_COLOR_FORMAT_RGBA8888 is actually ARGB8888
			return ARGB8888;
		case EG_COLOR_FORMAT_RGBX8888:			// note: EG_IMG_CF_RGBX8888 is actually XRGB8888
			return ARGB8888;
		case EG_COLOR_FORMAT_RGB565:
			return RGB565;
		case EG_COLOR_FORMAT_NATIVE:
			return EG2DColorFormat;
		case EG_COLOR_FORMAT_NATIVE_ALPHA:
#if EG_COLOR_DEPTH == 16
			// bitmap color format is 24b ARGB8565 - dma2d unsupported
			return UNSUPPORTED;
#elif EG_COLOR_DEPTH == 32
			return ARGB8888;
#else
			// unknown bitmap color format
			return UNSUPPORTED;
#endif
		default:
			return UNSUPPORTED;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGSTM32Context::DrawImage(const EGDrawImage *pDrawImage, const EGRect *pSrceRect, const void *pSrceBuffer)
{
	//if(pSrceBuffer->GetType() != EG_IMG_SRC_VARIABLE) return EG_RES_INVALID;
	return EG_RES_INVALID;
	if(pDrawImage->m_OPA <= EG_OPA_MIN) return EG_RES_OK;
  EGSTM32Context *pDC = (EGSTM32Context*)pDrawImage->m_pContext;
	const EGImageBuffer *pImage = (EGImageBuffer*)pSrceBuffer;
	const DMA2DColorFormat_e BitmapColorFormat = ColorToDMA2dColor((EG_ImageColorFormat_t)pImage->m_Header.ColorFormat);
	const bool IgnoreBitmapAlpha = (pImage->m_Header.ColorFormat == EG_COLOR_FORMAT_RGBX8888);
	if(BitmapColorFormat == UNSUPPORTED || pDrawImage->m_Angle != 0 || pDrawImage->m_Scale.IsScaled()) {
		return EG_RES_INVALID;  // sorry, dma2d can handle this
	}

	// FIXME: handle dsc.pivot, dsc.recolor, dsc.blend_mode
	// FIXME: pSrceBuffer pixel size *must* be known to use DMA2D
	// FIXME: If image is drawn by SW, then output cache needs to be cleaned next. Currently it is not possible.
	// Both draw buffer start address and buffer size *must* be 32-byte aligned since draw buffer cache is being invalidated.
	//uint32_t drawBufferLength = m_pDrawRect->GetSize() * sizeof(EG_Color_t);
	//EG_ASSERT_MSG(drawBufferLength % CACHE_ROW_SIZE == 0); // critical, but this is not the way to test it
	//EG_ASSERT_MSG((uint32_t)m_pDrawBuffer % CACHE_ROW_SIZE == 0, "draw_ctx.buf is not 32B aligned"); // critical?

	// For performance reasons, both source buffer start address and buffer size *should* be 32-byte aligned since source buffer cache is being cleaned.
	//uint32_t srcBufferLength = lv_area_get_size(src_area) * sizeof(EG_Color_t); // TODO: verify pSrceBuffer pixel size = sizeof(EG_Color_t)
	//EG_ASSERT_MSG(srcBufferLength % CACHE_ROW_SIZE == 0); // FIXME: assert fails (performance, non-critical)
	//EG_ASSERT_MSG((uint32_t)pSrceBuffer % CACHE_ROW_SIZE == 0); // FIXME: assert fails (performance, non-critical)

	EGRect DrawRect;
	if(!DrawRect.Intersect(pSrceRect, pDC->m_pClipRect)) return EG_RES_OK;
	int32_t DestStep = pDC->m_pDrawRect->GetWidth();
	EGPoint SrceOffset = pSrceRect->GetOffset(&DrawRect);  // source image offset in relation to DrawRect
	DrawRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
	pDC->BlendMap((EG_Color_t*)pDC->m_pDrawBuffer, DestStep, &DrawRect, pImage->m_pData, pImage->m_Header.Width, &SrceOffset, pDrawImage->m_OPA, BitmapColorFormat, IgnoreBitmapAlpha);
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

// Fills DrawRect with specified color.
void EGSTM32Context::BlendFill(const EG_Color_t *pDestBuffer, int32_t DestStep, const EGRect *pDrawRect, EG_Color_t Color, EG_OPA_t OPA)
{
	EG_ASSERT_MSG(!m_IsDMAInProgess, "dma2d transfer has not finished");  // critical
	int32_t DrawWidth = pDrawRect->GetWidth();
	int32_t DrawHeight = pDrawRect->GetHeight();
	WaitTransferFinish(nullptr);
	if(OPA >= EG_OPA_MAX) {
		DMA2D->CR = 0x3UL << DMA2D_CR_MODE_Pos;  // Register-to-memory (no FG nor BG, only output stage active)
		DMA2D->OPFCCR = EG2DColorFormat;
#if defined(DMA2D_OPFCCR_RBS_Pos)
		DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
		DMA2D->OMAR = (uint32_t)(pDestBuffer + (DestStep * pDrawRect->GetY1()) + pDrawRect->GetX1());
		DMA2D->OOR = DestStep - DrawWidth;  // out buffer offset
// Note: unlike FGCOLR and BGCOLR, OCOLR bits must match DMA2D_OUTPUT_COLOR, alpha can be specified
#if RBS_BIT		// swap red/blue bits
		DMA2D->OCOLR = (Color.ch.blue << 11) | (Color.ch.green_l << 5 | Color.ch.green_h << 8) | (Color.ch.red);
#else
		DMA2D->OCOLR = Color.full;
#endif
	}
	else {
		DMA2D->CR = 0x2UL << DMA2D_CR_MODE_Pos;  // Memory-to-memory with blending (FG and BG fetch with PFC and blending)

		DMA2D->FGPFCCR = A8;
		DMA2D->FGPFCCR |= (OPA << DMA2D_FGPFCCR_ALPHA_Pos);
		// Alpha Mode 1: Replace original foreground image alpha channel value by FGPFCCR.ALPHA
		DMA2D->FGPFCCR |= (0x1UL << DMA2D_FGPFCCR_AM_Pos);
		//DMA2D->FGPFCCR |= (RBS_BIT << DMA2D_FGPFCCR_RBS_Pos);

		// Note: in Alpha Mode 1 FGMAR and FGOR are not used to supply foreground A8 bytes,
		// those bytes are replaced by constant ALPHA defined in FGPFCCR
		DMA2D->FGMAR = (uint32_t)pDestBuffer;
		DMA2D->FGOR = DestStep;
		DMA2D->FGCOLR = EG_ColorTo32(Color) & 0x00ffffff;  // swap FGCOLR R/B bits if FGPFCCR.RBS (RBS_BIT) bit is set

		DMA2D->BGPFCCR = EG2DColorFormat;
#if defined(DMA2D_BGPFCCR_RBS_Pos)
		DMA2D->BGPFCCR |= (RBS_BIT << DMA2D_BGPFCCR_RBS_Pos);
#endif
		DMA2D->BGMAR = (uint32_t)(pDestBuffer + (DestStep * pDrawRect->GetY1()) + pDrawRect->GetX1());
		DMA2D->BGOR = DestStep - DrawWidth;
		DMA2D->BGCOLR = 0;  // used in A4 and A8 modes only
		CleanCache(DMA2D->BGMAR, DMA2D->BGOR, DrawWidth, DrawHeight, sizeof(EG_Color_t));
		DMA2D->OPFCCR = EG2DColorFormat;
#if defined(DMA2D_OPFCCR_RBS_Pos)
		DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
		DMA2D->OMAR = DMA2D->BGMAR;
		DMA2D->OOR = DMA2D->BGOR;
		DMA2D->OCOLR = 0;
	}
	// PL - pixel per lines (14 bit), NL - number of lines (16 bit)
	DMA2D->NLR = (DrawWidth << DMA2D_NLR_PL_Pos) | (DrawHeight << DMA2D_NLR_NL_Pos);
	StartTransfer();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::BlendMap(const EG_Color_t *pDestBuffer, int32_t DestStep, const EGRect *pDrawRect, const void *pSrceBuffer, int32_t SrceStep,
  const EGPoint *pSrceOffset, EG_OPA_t OPA, DMA2DColorFormat_e SrceCF, bool IgnoreSrceAlpha)
{
	EG_ASSERT_MSG(!m_IsDMAInProgess, "dma2d transfer has not finished");  // critical
	if(OPA <= EG_OPA_MIN || SrceCF == UNSUPPORTED) return;
	int32_t DrawWidth = pDrawRect->GetWidth();
	int32_t DrawHeight = pDrawRect->GetHeight();
	bool BitmapHasOpacity = !IgnoreSrceAlpha && (SrceCF == ARGB8888 || SrceCF == ARGB1555 || SrceCF == ARGB4444);
	if(OPA >= EG_OPA_MAX) OPA = 0xff;
	uint8_t SrceBpp;  // source bytes per pixel
	switch(SrceCF) {
		case ARGB8888:
			SrceBpp = 4;
			break;
		case RGB888:
			SrceBpp = 3;
			break;
		case RGB565:
		case ARGB1555:
		case ARGB4444:
			SrceBpp = 2;
			break;
		default:
			EG_LOG_ERROR("unsupported color format");
			return;
	}
	WaitTransferFinish(nullptr);
	DMA2D->FGPFCCR = SrceCF;
	if(OPA == 0xff && !BitmapHasOpacity) {		// no need to blend
		if(SrceCF == EG2DColorFormat) {     			// no need to convert pixel format (PFC) either
			DMA2D->CR = 0x0UL;
		}
		else {
			DMA2D->CR = 0x1UL << DMA2D_CR_MODE_Pos;  // Memory-to-memory with PFC (FG fetch only with FG PFC active)
		}
		// Alpha Mode 0: No modification of the foreground image alpha channel value
	}
	else {
		// blend
		DMA2D->CR = 0x2UL << DMA2D_CR_MODE_Pos;  // Memory-to-memory with blending (FG and BG fetch with PFC and blending)
		DMA2D->FGPFCCR |= (OPA << DMA2D_FGPFCCR_ALPHA_Pos);
		if(BitmapHasOpacity) {
			// Alpha Mode 2: Replace original foreground image alpha channel value by FGPFCCR.ALPHA multiplied with original alpha channel value
			DMA2D->FGPFCCR |= (0x2UL << DMA2D_FGPFCCR_AM_Pos);
		}
		else {
			// Alpha Mode 1: Replace original foreground image alpha channel value by FGPFCCR.ALPHA
			DMA2D->FGPFCCR |= (0x1UL << DMA2D_FGPFCCR_AM_Pos);
		}
	}
#if defined(DMA2D_FGPFCCR_RBS_Pos)
	DMA2D->FGPFCCR |= (RBS_BIT << DMA2D_FGPFCCR_RBS_Pos);
#endif
	DMA2D->FGMAR = ((uint32_t)pSrceBuffer) + SrceBpp * ((SrceStep * pSrceOffset->m_Y) + pSrceOffset->m_X);
	DMA2D->FGOR = SrceStep - DrawWidth;
	DMA2D->FGCOLR = 0;  // used in A4 and A8 modes only
	CleanCache(DMA2D->FGMAR, DMA2D->FGOR, DrawWidth, DrawHeight, SrceBpp);
	DMA2D->OPFCCR = EG2DColorFormat;
#if defined(DMA2D_OPFCCR_RBS_Pos)
	DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
	DMA2D->OMAR = (uint32_t)(pDestBuffer + (DestStep * pDrawRect->GetY1()) + pDrawRect->GetX1());
	DMA2D->OOR = DestStep - DrawWidth;
	DMA2D->OCOLR = 0;
	if(OPA != 0xff || BitmapHasOpacity) {
		// use background (BG*) registers
		DMA2D->BGPFCCR = EG2DColorFormat;
#if defined(DMA2D_BGPFCCR_RBS_Pos)
		DMA2D->BGPFCCR |= (RBS_BIT << DMA2D_BGPFCCR_RBS_Pos);
#endif
		DMA2D->BGMAR = DMA2D->OMAR;
		DMA2D->BGOR = DMA2D->OOR;
		DMA2D->BGCOLR = 0;  // used in A4 and A8 modes only
		CleanCache(DMA2D->BGMAR, DMA2D->BGOR, DrawWidth, DrawHeight, sizeof(EG_Color_t));
	}
	// PL - pixel per lines (14 bit), NL - number of lines (16 bit)
	DMA2D->NLR = (DrawWidth << DMA2D_NLR_PL_Pos) | (DrawHeight << DMA2D_NLR_NL_Pos);
	StartTransfer();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::BlendPaint(const EG_Color_t *pDestBuffer, int32_t DestStep, const EGRect *pDrawRect, const EG_OPA_t *pMaskBuffer,
  int32_t MaskStep, const EGPoint *pMaskOffset, EG_Color_t Color, EG_OPA_t OPA)
{
	EG_ASSERT_MSG(!m_IsDMAInProgess, "dma2d transfer has not finished");  // critical
	int32_t DrawWidth = pDrawRect->GetWidth();
	int32_t DrawHeight = pDrawRect->GetHeight();
	WaitTransferFinish(nullptr);
	DMA2D->CR = 0x2UL << DMA2D_CR_MODE_Pos;  // Memory-to-memory with blending (FG and BG fetch with PFC and blending)
	DMA2D->FGPFCCR = A8;
	if(OPA < EG_OPA_MAX) {
		DMA2D->FGPFCCR |= (OPA << DMA2D_FGPFCCR_ALPHA_Pos);
		DMA2D->FGPFCCR |= (0x2UL << DMA2D_FGPFCCR_AM_Pos);  // Alpha Mode: Replace original foreground image alpha channel value by FGPFCCR.ALPHA multiplied with original alpha channel value
	}
	//DMA2D->FGPFCCR |= (RBS_BIT << DMA2D_FGPFCCR_RBS_Pos);
	DMA2D->FGMAR = (uint32_t)(pMaskBuffer + (MaskStep * pMaskOffset->m_Y) + pMaskOffset->m_X);
	DMA2D->FGOR = MaskStep - DrawWidth;
	DMA2D->FGCOLR = EG_ColorTo32(Color) & 0x00ffffff;  // swap FGCOLR R/B bits if FGPFCCR.RBS (RBS_BIT) bit is set
	CleanCache(DMA2D->FGMAR, DMA2D->FGOR, DrawWidth, DrawHeight, sizeof(EG_OPA_t));
	DMA2D->BGPFCCR = EG2DColorFormat;
#if defined(DMA2D_BGPFCCR_RBS_Pos)
	DMA2D->BGPFCCR |= (RBS_BIT << DMA2D_BGPFCCR_RBS_Pos);
#endif
	DMA2D->BGMAR = (uint32_t)(pDestBuffer + (DestStep * pDrawRect->GetY1()) + pDrawRect->GetX1());
	DMA2D->BGOR = DestStep - DrawWidth;
	DMA2D->BGCOLR = 0;  // used in A4 and A8 modes only
	CleanCache(DMA2D->BGMAR, DMA2D->BGOR, DrawWidth, DrawHeight, sizeof(EG_Color_t));
	DMA2D->OPFCCR = EG2DColorFormat;
#if defined(DMA2D_OPFCCR_RBS_Pos)
	DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
	DMA2D->OMAR = DMA2D->BGMAR;
	DMA2D->OOR = DMA2D->BGOR;
	DMA2D->OCOLR = 0;
	// PL - pixel per lines (14 bit), NL - number of lines (16 bit)
	DMA2D->NLR = (DrawWidth << DMA2D_NLR_PL_Pos) | (DrawHeight << DMA2D_NLR_NL_Pos);
	StartTransfer();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::CopyBuffer(EG_Color_t *pDestBuffer, int32_t DestStep, EGRect *pDestRect, EG_Color_t *pSourceBuffer, int32_t SourceStep, EGPoint *pSourceOffset)
{
	EG_ASSERT_MSG(!m_IsDMAInProgess, "dma2d transfer has not finished");  // critical
	int32_t DrawWidth = pDestRect->GetWidth();
	int32_t DrawHeight = pDestRect->GetHeight();

	WaitTransferFinish(nullptr);

	DMA2D->CR = 0x0UL;  // Memory-to-memory (FG fetch only)

	DMA2D->FGPFCCR = EG2DColorFormat;
#if defined(DMA2D_FGPFCCR_RBS_Pos)
	DMA2D->FGPFCCR |= (RBS_BIT << DMA2D_FGPFCCR_RBS_Pos);
#endif
	DMA2D->FGMAR = (uint32_t)(pSourceBuffer + (SourceStep * pSourceOffset->m_Y) + pSourceOffset->m_X);
	DMA2D->FGOR = SourceStep - DrawWidth;
	DMA2D->FGCOLR = 0;  // used in A4 and A8 modes only
	CleanCache(DMA2D->FGMAR, DMA2D->FGOR, DrawWidth, DrawHeight, sizeof(EG_Color_t));
	// Note BG* registers do not need to be set up since BG is not used
	DMA2D->OPFCCR = EG2DColorFormat;
#if defined(DMA2D_OPFCCR_RBS_Pos)
	DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
	DMA2D->OMAR = (uint32_t)(pDestBuffer + (DestStep * pDestRect->GetY1()) + pDestRect->GetX1());
	DMA2D->OOR = DestStep - DrawWidth;
	DMA2D->OCOLR = 0;
	// PL - pixel per lines (14 bit), NL - number of lines (16 bit)
	DMA2D->NLR = (DrawWidth << DMA2D_NLR_PL_Pos) | (DrawHeight << DMA2D_NLR_NL_Pos);
	StartTransfer();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::StartTransfer(void)
{
	EG_ASSERT_MSG(!m_IsDMAInProgess, "dma2d transfer has not finished");
	m_IsDMAInProgess = true;
	DMA2D->IFCR = 0x3FU;  // trigger ISR flags reset
												// Note: cleaning output buffer cache is needed only when buffer may be misaligned or adjacent area may have been drawn in sw-fashion, e.g. using lv_draw_sw_blend_basic()
#if EG_COLOR_DEPTH == 16
  CleanCache(DMA2D->OMAR, DMA2D->OOR, (DMA2D->NLR & DMA2D_NLR_PL_Msk) >> DMA2D_NLR_PL_Pos,
																	 (DMA2D->NLR & DMA2D_NLR_NL_Msk) >> DMA2D_NLR_NL_Pos, sizeof(EG_Color_t));
#endif
	DMA2D->CR |= DMA2D_CR_START;
	// Note: for some reason mask buffer gets damaged during transfer if waiting is postponed
	WaitTransferFinish(nullptr);  // FIXME: this line should not be needed here, but it is
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::WaitTransferFinish(EGDisplayDriver *pDriver)
{
	if(pDriver && pDriver->WaitCB) {
		while((DMA2D->CR & DMA2D_CR_START) != 0U) {
			pDriver->WaitCB(pDriver);
		}
	}
	else {
		while((DMA2D->CR & DMA2D_CR_START) != 0U);
	}
	__IO uint32_t isrFlags = DMA2D->ISR;
	if(isrFlags & DMA2D_ISR_CEIF) {
		EG_LOG_ERROR("DMA2D config error");
	}

	if(isrFlags & DMA2D_ISR_TEIF) {
		EG_LOG_ERROR("DMA2D transfer error");
	}
	DMA2D->IFCR = 0x3FU;  // trigger ISR flags reset
	if(m_IsDMAInProgess) {
		// invalidate output buffer cached memory ONLY after DMA2D transfer
		//EGSTM32Context:: CleanCache(DMA2D->OMAR, DMA2D->OOR, (DMA2D->NLR & DMA2D_NLR_PL_Msk) >> DMA2D_NLR_PL_Pos, (DMA2D->NLR & DMA2D_NLR_NL_Msk) >> DMA2D_NLR_NL_Pos, sizeof(EG_Color_t));
		m_IsDMAInProgess = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

// Cortex-M7 DCache present
void EGSTM32Context::InvalidateCache(uint32_t Address, int32_t Offset, int32_t Width, int32_t Height, uint8_t PixelSize)
{
#if defined(EG_STM32_DMA2D_USE_M7_CACHE)
	if(((SCB->CCR) & SCB_CCR_DC_Msk) == 0) return;    // L1 data cache is disabled
	uint16_t Step = PixelSize * (Width + Offset);  // in bytes
	uint16_t ll = PixelSize * Width;                 // line length in bytes
	uint32_t n = 0;                                   // address of the next cache row after the last invalidated row
	int32_t h = 0;
	__DSB();
	while(h < Height) {
		uint32_t a = Address + (h * Step);
		uint32_t e = a + ll;  // end address, address of the first byte after the current line
		a &= ~(CACHE_ROW_SIZE - 1U);
		if(a < n) a = n;  // prevent the previous last cache row from being invalidated again
		while(a < e) {
			SCB->DCIMVAC = a;
			a += CACHE_ROW_SIZE;
		}
		n = a;
		h++;
	};
	__DSB();
	__ISB();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSTM32Context::CleanCache(uint32_t Address, int32_t Offset, int32_t Width, int32_t Height, uint8_t PixelSize)
{
#if defined(EG_STM32_DMA2D_USE_M7_CACHE)
	if(((SCB->CCR) & SCB_CCR_DC_Msk) == 0) return;    // L1 data cache is disabled
	uint16_t Step = PixelSize * (Width + Offset);  // in bytes
	uint16_t ll = PixelSize * Width;                 // line length in bytes
	uint32_t n = 0;                                   // address of the next cache row after the last cleaned row
	int32_t h = 0;
	__DSB();
	while(h < Height) {
		uint32_t a = Address + (h * Step);
		uint32_t e = a + ll;  // end address, address of the first byte after the current line
		a &= ~(CACHE_ROW_SIZE - 1U);
		if(a < n) a = n;  // prevent the previous last cache row from being cleaned again
		while(a < e) {
			SCB->DCCMVAC = a;
			a += CACHE_ROW_SIZE;
		}
		n = a;
		h++;
	};
	__DSB();
	__ISB();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

#if defined(EG_STM32_DMA2D_TEST)
// initialize µs timer
bool EGSTM32Context::InitialiseDWT(void)
{
	CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;	// disable TRC
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	// enable TRC

#if defined(__CORTEX_M) && (__CORTEX_M == 7U)
	DWT->LAR = 0xC5ACCE55;
#endif
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;	// disable clock cycle counter
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;	// enable  clock cycle counter
	DWT->CYCCNT = 0;	// reset the clock cycle counter value
	__ASM volatile("NOP");	// 3 NO OPERATION instructions
	__ASM volatile("NOP");
	__ASM volatile("NOP");
	if(DWT->CYCCNT) {    // check if clock cycle counter has started
		return true;  // clock cycle counter started
	}
	else {
		return false;  // clock cycle counter not started
	}
}

///////////////////////////////////////////////////////////////////////////////////////

// get elapsed µs since reset
uint32_t EGSTM32Context::GetDWT_us(void)
{
	uint32_t us = (DWT->CYCCNT * 1000000) / HAL_RCC_GetHCLKFreq();
	return us;
}

///////////////////////////////////////////////////////////////////////////////////////

// reset µs timer
void EGSTM32Context::ResetDWT(void)
{
	DWT->CYCCNT = 0;
}
#endif
#endif
