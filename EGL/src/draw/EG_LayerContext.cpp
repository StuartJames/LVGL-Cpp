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

#include "draw/EG_DeviceContext.h"
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
}
//////////////////////////////////////////////////////////////////////////////////////

EGLayerContext* EGLayerContext::Create(EGDeviceContext *pDC, const EGRect *pLayerRect, EGDrawLayerFlags_e Flags)
{
	if(pDC->LayerIntialiseProc == nullptr) return nullptr;
  EGLayerContext *pDrawLayer = new EGLayerContext;
	EGDisplay *pDisplay = GetRefreshingDisplay();
  pDrawLayer->m_pContext = pDC;
	pDrawLayer->m_Original.pBuffer = pDC->m_pDrawBuffer;
	pDrawLayer->m_Original.pBuferArea = pDC->m_pDrawRect;
	pDrawLayer->m_Original.pClipRect = pDC->m_pClipRect;
	pDrawLayer->m_Original.ScreenTransparent = pDisplay->m_pDriver->m_ScreenTransparent;
	pLayerRect->Copy(&pDrawLayer->m_FullRect);
	if(!pDC->LayerIntialiseProc(pDrawLayer, Flags)){
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

void EGLayerContext::Blend(EGDrawImage *pDrawImage)
{
	if(m_pContext->LayerBlendProc) m_pContext->LayerBlendProc(this, pDrawImage);
}

