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

#include "draw/EG_DrawContext.h"
#include "draw/EG_DrawArc.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

EGLayerContext::EGLayerContext(void) :
  m_pContext(nullptr),
  m_MaxRowWithAlpha(0),
  m_MaxRowWithoutAlpha(0),
  m_pLayerBuffer(nullptr),
  m_BufferSizeBytes(0),
  m_HasAlpha(0)
{
  m_Original.pClipRect = nullptr;
  m_Original.pBuferArea = nullptr;
  m_Original.pBuffer = nullptr;
  m_Original.ScreenTransparent = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

EGLayerContext::~EGLayerContext()
{
  m_pContext->WaitForFinish();
	m_pContext->m_pDrawBuffer = m_Original.pBuffer;
	m_pContext->m_pDrawRect = m_Original.pBuferArea;
	m_pContext->m_pClipRect = m_Original.pClipRect;
	EGDisplay *pDisplay = GetRefreshingDisplay();
	pDisplay->m_pDriver->m_ScreenTransparent = m_Original.ScreenTransparent;
	if(m_pContext->LayerDestroyProc) m_pContext->LayerDestroyProc(this);
//	EG_FreeMem(layer_ctx);
}
//////////////////////////////////////////////////////////////////////////////////////

EGLayerContext* EGLayerContext::Create(EGDrawContext *pContext, const EGRect *pLayerArea, EGDrawLayerFlags_e Flags)
{
	if(pContext->IntialiseLayerProc == nullptr) return nullptr;
  EGLayerContext *pDrawLayer = new EGLayerContext;
	EGDisplay *pDisplay = GetRefreshingDisplay();
  pDrawLayer->m_pContext = pContext;
	pDrawLayer->m_Original.pBuffer = pContext->m_pDrawBuffer;
	pDrawLayer->m_Original.pBuferArea = pContext->m_pDrawRect;
	pDrawLayer->m_Original.pClipRect = pContext->m_pClipRect;
	pDrawLayer->m_Original.ScreenTransparent = pDisplay->m_pDriver->m_ScreenTransparent;
	pLayerArea->Copy(&pDrawLayer->m_FullRect);
	pContext->IntialiseLayerProc(pDrawLayer, Flags);
	if(pDrawLayer->m_pLayerBuffer == nullptr) {
		delete pDrawLayer;
    return nullptr;
	}
	return pDrawLayer;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGLayerContext::Adjust(EGDrawLayerFlags_e Flags)
{
	if(m_pContext->LayerAdjustProc) m_pContext->LayerAdjustProc(this, Flags);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGLayerContext::Blend(EGDrawImage *draw_dsc)
{
	if(m_pContext->LayerBlendProc) m_pContext->LayerBlendProc(this, draw_dsc);
}

