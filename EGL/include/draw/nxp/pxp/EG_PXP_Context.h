/**
 * MIT License
 *
 * Copyright 2022, 2023 NXP
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

#include "../../sw/EG_SoftContext.h"
#include "../../sw/EG_DrawSoftBlend.h"

#if EG_COLOR_DEPTH != 32
#include "core/EG_Refresh.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////

class EGPXPContext : public EGSoftContext
{
public:
              EGPXPContext() : EGSoftContext(){};
  virtual    ~EGPXPContext(){};
  void        InitialiseContext(void);

  static void WaitForFinish(void);
  static void Blend(EGBlendBase *pBlend);
  static void DrawImageDecoded(const EGDrawImage *pImage, const EGRect *pRect, const uint8_t *pSourceMap, EG_ImageColorFormat_t ColorFormat);
  static void BufferCopy(void *pDestBuffer, int32_t DestStep, EGRect *pDestRect, void *pSourceBuffer, int32_t SourceStep, EGRect *pSourceRect);
  static bool NeedARGB8565Support();

private:
  EG_Result_t PXP_InitialiseGPU(void);
};

//////////////////////////////////////////////////////////////////////

/* During rendering, LVGL might initializes new draw_ctxs and start drawing into
 * a separate buffer (called layer). If the content to be rendered has "holes",
 * e.g. rounded corner, LVGL temporarily sets the disp_drv.screen_transp flag.
 * It means the renderers should draw into an ARGB buffer.
 * With 32 bit color depth it's not a big problem but with 16 bit color depth
 * the target pixel format is ARGB8565 which is not supported by the GPU.
 * In this case, the PXP callbacks should fallback to SW rendering. */
inline bool EGPXPContext::NeedARGB8565Support()
{
#if EG_COLOR_DEPTH != 32
  EGDisplay *pDisp = GetRefreshingDisplay();
	if(pDisp->m_pDriver->m_ScreenTransparent == 1) return true;
#endif
	return false;
}

#endif

