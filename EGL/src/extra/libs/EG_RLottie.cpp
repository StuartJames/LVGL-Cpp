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

#include "extra/libs/EG_RLottie.h"

#if EG_USE_RLOTTIE

#include <rlottie_capi.h>

///////////////////////////////////////////////////////////////////////////////////////

#define RLOTTIE_CLASS &c_RLottieClass
#define LV_ARGB32 32
#define LV_ARGB32 32

const EG_ClassType_t c_RLottieClass = {
	.base_class = &c_ImageClass,
	.pEventCB = EGImage::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
	.pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGRLottie::EGRLottie(EGObject *pParent, int32_t Width, int32_t Height, const char *pPath,
   bool IsPath /*= false*/, const EG_ClassType_t *pClassCnfg /*= RLOTTIE_CLASS*/) : EGImage()
{
	m_Width = Width;
	m_Height = Height;
  if(IsPath){
	  m_pPath = pPath;
	  m_pRLottieObj = nullptr;
  }
  else{
	  m_pRLottieObj = pPath;
	  m_pPath = nullptr;
  }
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRLottie::Configure()
{
	if(m_pRLottieObj) m_pAnimation= lottie_animation_from_data(m_pRLottieObj, m_pRLottieObj, "");
	else if(m_pPath) m_pAnimation= lottie_animation_from_file(m_pPath);
	if(m_pAnimation== nullptr) {
    EG_LOG_WARN("The aniamtion can't be opened");
		return;
	}
	m_TotalFrames = lottie_animation_get_totalframe(m_pAnimation);
	m_Framerate = (size_t)lottie_animation_get_framerate(m_pAnimation);
	m_CurrentFrame = 0;
	m_ScanlineWidth = m_Width * LV_ARGB32 / 8;
	size_t Size = (m_Width * m_Height * LV_ARGB32 / 8);
	m_pAllocatedBuffer = (uint32_t*)EG_AllocMem(Size);
	if(m_pAllocatedBuffer != nullptr) {
		m_AllocatedBufferSize = Size;
		memset(m_pAllocatedBuffer, 0, m_AllocatedBufferSize);
	}
	m_ImageBuffer.m_Header.AlwaysZero = 0;
	m_ImageBuffer.m_Header.ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;
	m_ImageBuffer.m_Header.Height = m_Height;
	m_ImageBuffer.m_Header.Width = m_Width;
	m_ImageBuffer.m_pData = (uint8_t*)m_pAllocatedBuffer;
	m_ImageBuffer.m_DataSize = m_AllocatedBufferSize;
	SetSource(&m_ImageBuffer);
	m_PlayControl = (EG_RlottieCtrl_e)(EG_RLOTTIE_CTRL_FORWARD | EG_RLOTTIE_CTRL_PLAY | EG_RLOTTIE_CTRL_LOOP);
	m_DestFrame = m_TotalFrames; // invalid destination frame so it's possible to pause on frame 0
	m_pTimer = EGTimer::Create(NextFrameCB, 1000 / m_Framerate, this);
	UpdateLayout();
}

///////////////////////////////////////////////////////////////////////////////////////

EGRLottie::~EGRLottie()
{
	if(m_pAnimation) {
		lottie_animation_destroy(m_pAnimation);
		m_pAnimation= 0;
		m_CurrentFrame = 0;
		m_Framerate = 0;
		m_ScanlineWidth = 0;
		m_TotalFrames = 0;
	}
	if(m_pTimer) {
		EGTimer::Delete(m_pTimer);
		m_pTimer = nullptr;
		m_PlayControl = EG_RLOTTIE_CTRL_FORWARD;
		m_DestFrame = 0;
	}
	InvalidateImageCacheSource(&m_ImageBuffer);
	if(m_pAllocatedBuffer) {
		EG_FreeMem(m_pAllocatedBuffer);
		m_pAllocatedBuffer = nullptr;
		m_AllocatedBufferSize = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRLottie::SetPlayMode(const EG_RlottieCtrl_e Ctrl)
{
	m_PlayControl = Ctrl;
	if(m_pTimer && (m_DestFrame != m_CurrentFrame || (m_PlayControl & EG_RLOTTIE_CTRL_PAUSE) == EG_RLOTTIE_CTRL_PLAY)) {
		m_pTimer->Resume();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRLottie::SetCurrentFrame(const size_t Frame)
{
	m_CurrentFrame = Frame < m_TotalFrames ? Frame : m_TotalFrames - 1;
}

///////////////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_DEPTH == 16
void EGRLottie::ConvertToRGBA5658(uint32_t *pPixel, const size_t Width, const size_t Height)
{
	/* rlottie draws in ARGB32 format, but EGL only deal with RGB565 format with (optional 8 bit alpha channel)
       so convert in place here the received buffer to EGL format. */
	uint8_t *pDest = (uint8_t *)pPixel;
	uint32_t *pSrce = pPixel;
	for(size_t y = 0; y < Height; y++) {
		/* Convert a 4 bytes per pixel in format ARGB to R5G6B5A8 format
            naive way:
                        r = ((c & 0xFF0000) >> 19)
                        g = ((c & 0xFF00) >> 10)
                        b = ((c & 0xFF) >> 3)
                        rgb565 = (r << 11) | (g << 5) | b
                        a = c >> 24;
            That's 3 mask, 6 bitshift and 2 or operations

            A bit better:
                        r = ((c & 0xF80000) >> 8)
                        g = ((c & 0xFC00) >> 5)
                        b = ((c & 0xFF) >> 3)
                        rgb565 = r | g | b
                        a = c >> 24;
            That's 3 mask, 3 bitshifts and 2 or operations */
		for(size_t x = 0; x < Width; x++) {
			uint32_t in = pSrce[x];
#if EG_COLOR_16_SWAP == 0
			uint16_t r = (uint16_t)(((in & 0xF80000) >> 8) | ((in & 0xFC00) >> 5) | ((in & 0xFF) >> 3));
#else
			// We want: rrrr rrrr GGGg gggg bbbb bbbb => gggb bbbb rrrr rGGG
			uint16_t r = (uint16_t)(((in & 0xF80000) >> 16) | ((in & 0xFC00) >> 13) | ((in & 0x1C00) << 3) | ((in & 0xF8) << 5));
#endif
			EG_CopyMem(pDest, &r, sizeof(r));
			pDest[sizeof(r)] = (uint8_t)(in >> 24);
			pDest += EG_IMG_PX_SIZE_ALPHA_BYTE;
		}
		pSrce += Width;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////////////

void EGRLottie::NextFrameCB(EGTimer *pTimer)
{
	EGRLottie *pRLotie = (EGRLottie*)pTimer->m_pParam;
	if((pRLotie->m_PlayControl & EG_RLOTTIE_CTRL_PAUSE) == EG_RLOTTIE_CTRL_PAUSE) {
		if(pRLotie->m_CurrentFrame == pRLotie->m_DestFrame) {
			// Pause the timer too when it has run once to avoid CPU consumption
      pTimer->Pause();
			return;
		}
		pRLotie->m_DestFrame = pRLotie->m_CurrentFrame;
	}
	else {
		if((pRLotie->m_PlayControl & EG_RLOTTIE_CTRL_BACKWARD) == EG_RLOTTIE_CTRL_BACKWARD) {
			if(pRLotie->m_CurrentFrame > 0)
				--pRLotie->m_CurrentFrame;
			else { // Looping ?
				if((pRLotie->m_PlayControl & EG_RLOTTIE_CTRL_LOOP) == EG_RLOTTIE_CTRL_LOOP)
        pRLotie->m_CurrentFrame = pRLotie->m_TotalFrames - 1;
				else {
					EGEvent::EventSend(pRLotie, EG_EVENT_READY, nullptr);
					pTimer->Pause();
					return;
				}
			}
		}
		else {
			if(pRLotie->m_CurrentFrame < pRLotie->m_TotalFrames)
				++pRLotie->m_CurrentFrame;
			else { // Looping ?
				if((pRLotie->m_PlayControl & EG_RLOTTIE_CTRL_LOOP) == EG_RLOTTIE_CTRL_LOOP)
        pRLotie->m_CurrentFrame = 0;
				else {
					EGEvent::EventSend(pRLotie, EG_EVENT_READY, nullptr);
					pTimer->Pause();
					return;
				}
			}
		}
	}
	lottie_animation_render(pRLotie->m_pAnimation, pRLotie->m_CurrentFrame,	pRLotie->m_pAllocatedBuffer,
		pRLotie->m_ImageBuffer.m_Header.Width, pRLotie->m_ImageBuffer.m_Header.Height,	pRLotie->m_ScanlineWidth);
#if EG_COLOR_DEPTH == 16
  ConvertToRGBA5658(pRLotie->m_pAllocatedBuffer, pRLotie->m_ImageBuffer.m_Header.Width, pRLotie->m_ImageBuffer.m_Header.Height);
#endif
  pRLotie->Invalidate();
}

#endif
