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

#include "extra/libs/gifdec.h"

///////////////////////////////////////////////////////////////////////////////////////

#define MY_CLASS &EG_GifClass

const EG_ClassType_t EG_GifClass = {
  .pBaseClassType = c_Imageclass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_INHERIT,
};

///////////////////////////////////////////////////////////////////////////////////////

EGImageGIF::EGImageGIF(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &EG_GifClass*/)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGImageGIF::~EGImageGIF(void)
{
	InvalidateImageCacheSource(&m_ImageBuffer);
	if(m_pGif) gd_close_gif(m_pGif);
	EGTimer::Delete(m_pTimer);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::Configure(void)
{
  EGImage::Configure();
	m_pGif = nullptr;
	m_pTimer = EGTimer::Create(NextFrameTaskCB, 10, this);
	m_pTimer->Pause();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::SetSource(const void *pSource)
{
	if(m_pGif) {                         // Close previous gif if any
		InvalidateImageCacheSource(&m_ImageBuffer);
		gd_close_gif(m_pGif);
		m_pGif = nullptr;
		m_ImageBuffer.m_pData = nullptr;
	}
	if(EGDrawImage::GetType(pSource) == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pSource;
		m_pGif = gd_open_gif_data(pImageBuffer->m_pData);
	}
	else if(EGDrawImage::GetType(pSource) == EG_IMG_SRC_FILE) {
		m_pGif = gd_open_gif_file((char*)pSource);
	}
	if(m_pGif == nullptr) {
		EG_LOG_WARN("Could't load the source");
		return;
	}
	m_ImageBuffer.m_pData = m_pGif->canvas;
	m_ImageBuffer.m_Header.AlwaysZero = 0;
	m_ImageBuffer.m_Header.ColorFormat = EG_IMG_CF_TRUE_COLOR_ALPHA;
	m_ImageBuffer.m_Header.Height = m_pGif->height;
	m_ImageBuffer.m_Header.Width = m_pGif->width;
	m_LastCall = EG_GetTick();
	EGImage::SetSource(&m_ImageBuffer);
	m_pTimer->Resume();
	m_pTimer->Reset();
	NextFrameTaskCB(m_pTimer);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::Restart(void)
{
	gd_rewind(m_pGif);
	m_pTimer->Resume();
	m_pTimer->Reset();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageGIF::NextFrameTaskCB(EGTimer *pTimer)
{
	EGImageGIF *pGif = (EGImageGIF*)pTimer->m_pUserData;
	uint32_t elaps = EG_TickElapse(pGif->m_LastCall);
	if(elaps < pGif->m_pGif->gce.delay * 10) return;
	pGif->m_LastCall = EG_GetTick();
	int has_next = gd_get_frame(pGif->m_pGif);
	if(has_next == 0) {
		EG_Result_t res = EGEvent::EventSend(pGif, EG_EVENT_READY, nullptr);		// It was the last repeat
		pTimer->Pause();
		if(res != EG_FS_RES_OK) return;
	}
	gd_render_frame(pGif->m_pGif, (uint8_t *)pGif->m_ImageBuffer.m_pData);
	InvalidateImageCacheSource(pGif->GetSource());
	pGif->Invalidate();
}

#endif
