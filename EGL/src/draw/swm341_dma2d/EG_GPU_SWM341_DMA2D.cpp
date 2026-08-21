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

#include "draw/swm341_dma2d/EG_GPU_SWM341_DMA2D.h"
#include "core/EG_Refresh.h"

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_GPU_SWM341_DMA2D

#include EG_GPU_SWM341_DMA2D_INCLUDE

#if EG_COLOR_16_SWAP
#error "Can't use DMA2D with EG_COLOR_16_SWAP 1"
#endif

#if EG_COLOR_DEPTH == 8
#error "Can't use DMA2D with EG_COLOR_DEPTH == 8"
#endif

#if EG_COLOR_DEPTH == 16
#define EG_DMA2D_COLOR_FORMAT EG_SWM341_DMA2D_RGB565
#elif EG_COLOR_DEPTH == 32
#define EG_DMA2D_COLOR_FORMAT EG_SWM341_DMA2D_ARGB8888
#else
// Can't use GPU with other formats
#endif

///////////////////////////////////////////////////////////////////////////////////////

// Turn on the peripheral and set output color mode, this only needs to be Done once
void EGSWM341Context::InitialiseDMA2d(void)
{
	// Enable DMA2D clock
	SYS->CLKEN0 |= (1 << SYS_CLKEN0_DMA2D_Pos);
	DMA2D->CR &= ~DMA2D_CR_WAIT_Msk;
	DMA2D->CR |= (CyclesPerUs << DMA2D_CR_WAIT_Pos);
	DMA2D->IF = 0xFF;
	DMA2D->IE = (0 << DMA2D_IE_DONE_Pos);
	// set output colour mode
	DMA2D->L[DMA2D_LAYER_OUT].PFCCR = (EG_DMA2D_COLOR_FORMAT << DMA2D_PFCCR_CFMT_Pos);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSWM341Context::InitialiseContext()
{
  EGSoftContext::InitialiseContext();   // call the base class
	BlendProc = Blend;
//  DrawImageDecodedProc = DrawImageDecoded;
  WaitForFinishProc = WaitForFinish;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSWM341Context::Blend(EGBlendBase *pBlend)
{
EGRect BlendRect;

  EGSWM341Context *pDC = (EGSWM341Context*)pBlend->m_pContext;
	if(!BlendRect.Intersect(pBlend->m_pRect, pDC->m_pClipRect)) return;
	bool Done = false;
	if(pBlend->m_pMaskBuffer == NULL && pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL && BlendRect.GetSize() > 100) {
		int32_t DestStep = pDC->m_pDrawRect->GetWidth();
		EG_Color_t *pDestBuffer = (EG_Color_t*)pDC->m_pDrawBuffer;
		pDestBuffer += DestStep * (BlendRect.GetY1() - pDC->m_pDrawRect->GetY1()) + (BlendRect.GetX1() - pDC->m_pDrawRect->GetX1());
		const EG_Color_t *pSrceBuffer = pBlend->m_pSourceBuffer;
		if(pSrceBuffer) {
			EGSoftBlend::BlendBasic(pBlend);
			int32_t SrceStep = pBlend->m_pRect->GetWidth();
			pSrceBuffer += SrceStep * (BlendRect.GetY1() - pBlend->m_pRect->GetY1()) + (BlendRect.GetX1() - pBlend->m_pRect->GetX1());
			BlendRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
			pDC->Map(pDestBuffer, &BlendRect, DestStep, pSrceBuffer, SrceStep, pBlend->m_OPA);
			Done = true;
		}
		else if(pBlend->m_OPA >= EG_OPA_MAX) {
			BlendRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
			pDC->Fill(pDestBuffer, DestStep, &BlendRect, pBlend->m_Color);
			Done = true;
		}
	}
	if(!Done) EGSoftBlend::BlendBasic(pBlend);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSWM341Context::DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSrceBuffer, EG_ImageColorFormat_t ColorFormat)
{
	// TODO basic ARGB8888 image can be handles here
	EGSoftContext::DrawImageDecoded(pDrawImage, pRect, pSrceBuffer, ColorFormat);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSWM341Context::Fill(EG_Color_t * pDestBuffer, int32_t DestStep, const EGRect *pFillRect, EG_Color_t Color)
{
	// Simply fill an area
	int32_t FillWidth = pFillRect->GetWidth();
	int32_t FillHeight = pFillRect->GetHeight();

#if 1
	DMA2D->L[DMA2D_LAYER_OUT].COLOR = Color.full;

	DMA2D->L[DMA2D_LAYER_OUT].MAR = (uint32_t)pDestBuffer;
	DMA2D->L[DMA2D_LAYER_OUT].OR = DestStep - FillWidth;
	DMA2D->NLR = ((FillWidth - 1) << DMA2D_NLR_NPIXEL_Pos) | ((FillHeight - 1) << DMA2D_NLR_NLINE_Pos);
	DMA2D->CR &= ~DMA2D_CR_MODE_Msk;	// start transfer
	DMA2D->CR |= (3 << DMA2D_CR_MODE_Pos) |	(1 << DMA2D_CR_START_Pos);
#else
	for(uint32_t y = 0; y < FillHeight; y++) {
		for(uint32_t x = 0; x < FillWidth; x++) {
			pDestBuffer[y * DestStep + x] = Color;
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSWM341Context::Map(EG_Color_t * pDestBuffer, const EGRect *pDestRect, int32_t DestStep, const EG_Color_t * pSrceBuffer, int32_t SrceStep, EG_OPA_t OPA)
{
	// Simple copy
	int32_t DestWidth = pDestRect->GetWidth();
	int32_t DestHeight = pDestRect->GetHeight();
	if(OPA >= EG_OPA_MAX) {
#if 1
		// copy output colour mode, this register controls both input and output colour format
		DMA2D->L[DMA2D_LAYER_FG].MAR = (uint32_t)pSrceBuffer;
		DMA2D->L[DMA2D_LAYER_FG].OR = SrceStep - DestWidth;
		DMA2D->L[DMA2D_LAYER_FG].PFCCR = (EG_DMA2D_COLOR_FORMAT << DMA2D_PFCCR_CFMT_Pos);
		DMA2D->L[DMA2D_LAYER_OUT].MAR = (uint32_t)pDestBuffer;
		DMA2D->L[DMA2D_LAYER_OUT].OR = DestStep - DestWidth;
		DMA2D->NLR = ((DestWidth - 1) << DMA2D_NLR_NPIXEL_Pos) | ((DestHeight - 1) << DMA2D_NLR_NLINE_Pos);
		DMA2D->CR &= ~DMA2D_CR_MODE_Msk;		// start transfer
		DMA2D->CR |= (0 << DMA2D_CR_MODE_Pos) | (1 << DMA2D_CR_START_Pos);
#else
		EG_Color_t Buffer[1024];
		for(uint32_t y = 0; y < DestHeight; y++) {
			EG_CopyMem(Buffer, &pSrceBuffer[y * SrceStep], DestWidth * sizeof(EG_Color_t));
			EG_CopyMem(&pDestBuffer[y * DestStep], Buffer, DestWidth * sizeof(EG_Color_t));
		}
#endif
	}
	else {
		DMA2D->L[DMA2D_LAYER_FG].MAR = (uint32_t)pSrceBuffer;
		DMA2D->L[DMA2D_LAYER_FG].OR = SrceStep - DestWidth;
		DMA2D->L[DMA2D_LAYER_FG].PFCCR = (EG_DMA2D_COLOR_FORMAT << DMA2D_PFCCR_CFMT_Pos)
			| (2 << DAM2D_PFCCR_AMODE_Pos)	    // alpha mode 2, replace with foreground * alpha value
			| (OPA << DMA2D_PFCCR_ALPHA_Pos);		// alpha value
		DMA2D->L[DMA2D_LAYER_BG].MAR = (uint32_t)pDestBuffer;
		DMA2D->L[DMA2D_LAYER_BG].OR = DestStep - DestWidth;
		DMA2D->L[DMA2D_LAYER_BG].PFCCR = (EG_DMA2D_COLOR_FORMAT << DMA2D_PFCCR_CFMT_Pos);
		DMA2D->L[DMA2D_LAYER_OUT].MAR = (uint32_t)pDestBuffer;
		DMA2D->L[DMA2D_LAYER_OUT].OR = DestStep - DestWidth;
		DMA2D->NLR = ((DestWidth - 1) << DMA2D_NLR_NPIXEL_Pos) | ((DestHeight - 1) << DMA2D_NLR_NLINE_Pos);
		DMA2D->CR &= ~DMA2D_CR_MODE_Msk;		// start transfer
		DMA2D->CR |= (2 << DMA2D_CR_MODE_Pos) |	(1 << DMA2D_CR_START_Pos);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSWM341Context::WaitForFinish()
{
  EGDisplay *pDisp = GetRefreshingDisplay();
	if(pDisp->m_pDriver && pDisp->m_pDriver->WaitCB) {
		while(DMA2D->CR & DMA2D_CR_START_Msk) {
			pDisp->m_pDriver->WaitCB(pDisp->m_pDriver);
		}
	}
	else {
		while(DMA2D->CR & DMA2D_CR_START_Msk);
	}
	SoftWaitForFinish();
}

#endif
