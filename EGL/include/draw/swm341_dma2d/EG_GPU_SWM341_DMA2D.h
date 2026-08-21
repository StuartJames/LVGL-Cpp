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

#include "misc/EG_Color.h"
#include "hal/EG_HALDisplay.h"
#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_GPU_SWM341_DMA2D

#include "SWM341.h"

#define EG_SWM341_DMA2D_ARGB8888 0
#define EG_SWM341_DMA2D_RGB888 1
#define EG_SWM341_DMA2D_RGB565 2


///////////////////////////////////////////////////////////////////////////////////////

class EGSWM341Context : public EGSoftContext
{
public:
                  EGSWM341Context() : EGSoftContext(){};
  virtual         ~EGSWM341Context(){};
  void            InitialiseContext(void);

  static void     InitialiseDMA2d(void);
  static void     WaitForFinish(void);
  static void     Blend(EGBlendBase *pBlend);
  static void     DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSrceBuffer, EG_ImageColorFormat_t ColorFormat);

private:
  void            Fill(EG_Color_t * pDestBuffer, int32_t DestStep, const EGRect *pFillRect, EG_Color_t Color);
  void            Map(EG_Color_t * pDestBuffer, const EGRect *pDestRect, int32_t DestStep, const EG_Color_t * pSrceBuffer, int32_t SrceStep, EG_OPA_t OPA);

};

#endif