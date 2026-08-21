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

#pragma once

#include "EG_IntrnlConfig.h"

#if EG_USE_RLOTTIE

#include "core/EG_Object.h"
#include "widgets/EG_Image.h"

///////////////////////////////////////////////////////////////////////////////////////

enum EG_RlottieCtrl_e {
	EG_RLOTTIE_CTRL_FORWARD = 0,
	EG_RLOTTIE_CTRL_BACKWARD = 1,
	EG_RLOTTIE_CTRL_PAUSE = 2,
	EG_RLOTTIE_CTRL_PLAY = 0, /* Yes, play = 0 is the default mode */
	EG_RLOTTIE_CTRL_LOOP = 8,
};

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_RLottieClass;
struct Lottie_Animation_S;    // definition in lottieanimation_capi.c

///////////////////////////////////////////////////////////////////////////////////////

class EGRLottie : public EGImage
{
public:
                    EGRLottie(void) : EGImage(){};
                    EGRLottie(EGObject *pParent, int32_t Width, int32_t Height, const char *pPath,
                       bool IsPath = false, const EG_ClassType_t *pClassCnfg = &c_RLottieClass);
                    ~EGRLottie();
  virtual void      Configure(void);

  void              SetPlayMode(const EG_RlottieCtrl_e Ctrl);
  void              SetCurrentFrame(const size_t Frame);

  static void       NextFrameCB(EGTimer *pTimer);

	struct Lottie_Animation_S *m_pAnimation;
	EGTimer          *m_pTimer;  //task;
	EGImageBuffer     m_ImageBuffer;
	size_t            m_TotalFrames;
	size_t            m_CurrentFrame;
	size_t            m_Framerate;
	uint32_t         *m_pAllocatedBuffer;
	size_t            m_AllocatedBufferSize;
	size_t            m_ScanlineWidth;
	EG_RlottieCtrl_e  m_PlayControl;
	size_t            m_DestFrame;

private:
#if EG_COLOR_DEPTH == 16
  static void       ConvertToRGBA5658(uint32_t *pix, const size_t width, const size_t height);
#endif

  int32_t           m_Width;
  int32_t           m_Height;
  const char        *m_pRLottieObj;
  const char        *m_pPath;

};


#endif