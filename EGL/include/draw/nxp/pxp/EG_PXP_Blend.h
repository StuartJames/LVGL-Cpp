/**
 * MIT License
 *
 * Copyright 2020-2023 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next paragraph)
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

 #pragma once

 #include "../../../EG_IntrnlConfig.h"

#if EG_USE_GPU_NXP_PXP

#include "EG_PXP_Context.h"
#include "EG_PXP_GPU.h"
#include "hal/EG_HALDisplay.h"

///////////////////////////////////////////////////////////////////////////////////////

class EGPXPBlend : public EGBlendBase
{
public:
              EGPXPBlend(const EGPXPContext *pDC);
              ~EGPXPBlend(void);
  static void Fill(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep, EG_Color_t color, EG_OPA_t opa);
  static void Blit(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
                  const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
                  EG_OPA_t opa, EG_DisplayRotation_t Angle);
  static void BlitTransform(EG_Color_t *pDest, EGRect *pDestRect, int32_t DestStep,
                  const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
                  const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat);
  static void BufferCopy(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
                  const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep);

private:
  static void BlitOPA(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
                  const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
                  const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat);
  static void BlitCover(EG_Color_t *pDest, EGRect *pDestRect, int32_t DestStep,
                  const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
                  const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat);
  static void BlitColorFormat(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
                  const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
                  const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat);

};

#endif
