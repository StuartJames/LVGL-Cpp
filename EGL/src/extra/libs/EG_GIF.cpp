/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "extra/libs/EG_GIF.h"

#if EG_USE_GIF

#include "extra/libs/EG_GIFDecoder.h"

///////////////////////////////////////////////////////////////////////////////////////

#define GIF_CLASS &c_GifClass

const EG_ClassType_t c_GifClass = {
  .pBaseClassType = &c_ImageClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_INHERIT,
#if EG_USE_EXT_DATA
	.pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGImageGIF::EGImageGIF(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_GifClass*/) : EGImage(),
  m_pDecoder(nullptr),
  m_pTimer(nullptr),
  m_LastCall(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGImageGIF::~EGImageGIF(void)
{
	InvalidateImageCacheSource(&m_ImageBuffer);
	if(m_pDecoder) delete m_pDecoder;
	EGTimer::Delete(m_pTimer);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::Configure(void)
{
  EGImage::Configure();
	m_pTimer = EGTimer::Create(EGImageGIF::NextFrameCB, 10, this, true);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::SetSource(const void *pSource)
{
	m_pTimer->Pause();
	if(m_pDecoder) {                         // Close previous gif if any
		InvalidateImageCacheSource(&m_ImageBuffer);
		delete m_pDecoder;
		m_pDecoder = nullptr;
		m_ImageBuffer.m_pData = nullptr;
	}
	if(EGDrawImage::GetType(pSource) == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pSource;
		m_pDecoder = EGDecoderGIF::OpenData(pImageBuffer->m_pData);
	}
	else if(EGDrawImage::GetType(pSource) == EG_IMG_SRC_FILE) {
		m_pDecoder = EGDecoderGIF::OpenFile((char*)pSource);
	}
	if(m_pDecoder == nullptr) {
		EG_LOG_WARN("Could't load the source");
		return;
	}
	m_ImageBuffer.m_pData = m_pDecoder->m_pCanvas;
  m_ImageBuffer.m_DataSize = m_pDecoder->m_ImageSize;
	m_ImageBuffer.m_Header.AlwaysZero = 0;
	m_ImageBuffer.m_Header.ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;
	m_ImageBuffer.m_Header.Height = m_pDecoder->m_Height;
	m_ImageBuffer.m_Header.Width = m_pDecoder->m_Width;
	EGImage::SetSource(&m_ImageBuffer);
	m_LastCall = EG_GetTick();
	m_pTimer->Reset();
	m_pTimer->Resume();
	NextFrameCB(m_pTimer);    // render first frame
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::Restart(void)
{
	m_pDecoder->Rewind();
	m_pTimer->Reset();
	m_pTimer->Resume();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::NextFrameCB(EGTimer *pTimer)
{
	EGImageGIF *pGif = (EGImageGIF*)pTimer->m_pParam;
	uint32_t Elaps = EG_TickElapse(pGif->m_LastCall);
	if(Elaps < pGif->m_pDecoder->m_GlobalControlExt.Delay * 10) return;
	pGif->m_LastCall = EG_GetTick();
	int Next = pGif->m_pDecoder->GetFrame();
	if(Next == 0) {
		EG_Result_t Result = EGEvent::EventSend(pGif, EG_EVENT_READY, nullptr);		// It was the last repeat
		pTimer->Pause();
		if(Result != EG_FS_RES_OK) return;
	}
	pGif->m_pDecoder->RenderFrame((uint8_t *)pGif->m_ImageBuffer.m_pData);
	InvalidateImageCacheSource(pGif->GetSource());
	pGif->Invalidate();
}

#endif
