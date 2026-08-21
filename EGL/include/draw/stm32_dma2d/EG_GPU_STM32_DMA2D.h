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

#if EG_USE_GPU_STM32_DMA2D

#include "stm32F7xx_hal.h"

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
#define EG_STM32_DMA2D_USE_M7_CACHE
#endif

///////////////////////////////////////////////////////////////////////////////////////

enum DMA2DColorFormat_e {
	ARGB8888 = 0x0,
	RGB888 = 0x01,
	RGB565 = 0x02,
	ARGB1555 = 0x03,
	ARGB4444 = 0x04,
	A8 = 0x09,
	UNSUPPORTED = 0xff,
};

///////////////////////////////////////////////////////////////////////////////////////

class EGSTM32Context : public EGSoftContext
{
public:
                      EGSTM32Context() : EGSoftContext(){};
  virtual             ~EGSTM32Context(){};
  void                InitialiseContext(void);

  static void         InitialiseDMA2D(void);
  static void         Blend(EGBlendBase *pBlend);
  static void         DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
  static EG_Result_t  DrawImage(const EGDrawImage *pDrawImage, const EGRect *pSrceRect, const void *pSrceBuffer);
  static void         BufferCopy(void *pDestBuffer, int32_t DestStep, EGRect *pDestRect, void *pSrceBuffer, int32_t SrceStep, EGRect *pSrceRect);


private:
  void                InvalidateCache(uint32_t Address, int32_t Offset, int32_t Width, int32_t Height, uint8_t PixelSize);
  void                CopyBuffer(EG_Color_t *pDestBuffer, int32_t DestStep, EGRect *pDestRect, EG_Color_t *pSourceBuffer, int32_t SourceStep, EGPoint *pSourceOffset);
  void                BlendFill(const EG_Color_t *pDestBuffer, int32_t DestStep, const EGRect *pDrawRect, EG_Color_t Color, EG_OPA_t OPA);
  void                BlendPaint(const EG_Color_t *pDestBuffer, int32_t DestStep, const EGRect *pDrawRect, const EG_OPA_t *pMaskBuffer,
                                          int32_t MaskStep, const EGPoint *pMaskOffset, EG_Color_t Color, EG_OPA_t OPA);
  static void         BlendMap(const EG_Color_t *pDestBuffer, int32_t DestStep, const EGRect *pDrawRect, const void *pSrceBuffer, int32_t SrceStep,
                                          const EGPoint *pSrceOffset, EG_OPA_t OPA, DMA2DColorFormat_e SrceCF, bool IgnoreSrceAlpha);
  static void         WaitTransferFinish(EGDisplayDriver *pDriver);
  static void         StartTransfer(void);
  static void         CleanCache(uint32_t Address, int32_t Offset, int32_t Width, int32_t Height, uint8_t PixelSize);
  static DMA2DColorFormat_e  ColorToDMA2dColor(EG_ImageColorFormat_t ColorFormat);

#if defined(EG_STM32_DMA2D_TEST)
  bool                InitialiseDWT(void);
  void                ResetDWT(void);
  uint32_t            GetDWT_us(void);
#endif

  static bool         m_IsDMAInProgess; // indicates whether DMA2D transfer is in progress

};

///////////////////////////////////////////////////////////////////////////////////////
#endif