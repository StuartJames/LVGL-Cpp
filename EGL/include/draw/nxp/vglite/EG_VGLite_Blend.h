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


#if EG_USE_GPU_NXP_VG_LITE
#include "EG_VGLite_Utils.h"

//////////////////////////////////////////////////////////////////////////////////////

/**
 * Enable BLIT quality degradation workaround for RT595,
 * recommended for screen's dimension > 352 pixels.
 */
#define RT595_BLIT_WRKRND_ENABLED 1

/* Internal compound symbol */
#if(defined(CPU_MIMXRT595SFFOB) || defined(CPU_MIMXRT595SFFOB_cm33) ||  \
		defined(CPU_MIMXRT595SFFOC) || defined(CPU_MIMXRT595SFFOC_cm33)) && \
	RT595_BLIT_WRKRND_ENABLED
#define VG_LITE_BLIT_SPLIT_ENABLED 1
#else
#define VG_LITE_BLIT_SPLIT_ENABLED 0
#endif

//////////////////////////////////////////////////////////////////////////////////////

class EGVGLiteContext;

class EGVGLiteBlend : public EGBlendBase
{
public:
                  EGVGLiteBlend(const EGVGLiteContext *pDC);
                  ~EGVGLiteBlend(void);
  static EG_Result_t     Fill(EGRect *pDestRect, EG_Color_t Color, EG_OPA_t OPA);
#if VG_LITE_BLIT_SPLIT_ENABLED
  static EG_Result_t       BlitSplitGPU(EG_Color_t *pDestBuffer, EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrceBuffer,
                                       EGRect *pSrceRect, int32_t SrceStep,	EG_OPA_t OPA);
#else
  static EG_Result_t     BlitGPU(EGRect *pDestRect,	const EG_Color_t *pSrceBuffer, const EGRect *pSrceRect, int32_t SrceStep,	EG_OPA_t OPA);
  static EG_Result_t     BlitTransformGPU(const EGRect *pDestRect, const EGRect *pClipRect,	const EG_Color_t *pSrceBuffer, 
                                          const EGRect *pSrceRect, int32_t SrceStep, const EGDrawImage *pImage);

#endif
  static EG_Result_t     BufferCopyGPU(EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep,
															const EG_Color_t *pSrceBuffer, const EGRect *pSrceRect, int32_t SrceStep);

private:
  static EG_Result_t     Blit(const EGRect *pSrceRect, EG_OPA_t OPA);
  static EG_Result_t     CheckSrceAlignment(const EG_Color_t *pSrceBuffer, int32_t SrceStep);
  static inline void     SetTranslationMatrix(const EGRect *pDestRect);
  static inline void     SetTransformationMatrix(const EGRect *pDestRect, const EGDrawImage *pImage);
#if VG_LITE_BLIT_SPLIT_ENABLED
  static void            AlignX(EGRect *pRect, EG_Color_t **ppBuffer);
  static void            AlignY(EGRect *pRect, EG_Color_t **ppBuffer, int32_t stride);
  static EG_Result_t     BlitSplit(EG_Color_t *pDestBuffer, EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrceBuffer,
                            EGRect *pSrceRect, int32_t SrceStep, EG_OPA_t OPA);
#endif

  static VGLiteMatrix_t  m_VGMatrix;
};

//////////////////////////////////////////////////////////////////////////////////////

inline void EGVGLiteBlend::SetTranslationMatrix(const EGRect *pDestRect)
{
	vg_lite_identity(&m_VGMatrix);
	vg_lite_translate((vg_lite_float_t)pDestRect->GetX1(), (vg_lite_float_t)pDestRect->GetY1(), &m_VGMatrix);
}

//////////////////////////////////////////////////////////////////////////////////////

inline void EGVGLiteBlend::SetTransformationMatrix(const EGRect *pDestRect, const EGDrawImage *pImage)
{
	SetTranslationMatrix(pDestRect);
	bool HasScale = (pImage->m_Scale.IsScaled());
	bool HasRotation = (pImage->m_Angle != 0);
	vg_lite_translate(pImage->m_Pivot.m_X, pImage->m_Pivot.m_Y, &m_VGMatrix);
	if(HasRotation) vg_lite_rotate(pImage->m_Angle / 10.0f, &m_VGMatrix); // angle is 1/10 degree
	if(HasScale) {
		vg_lite_float_t ScaleX = 1.0f * pImage->m_Scale.m_X / EG_SCALE_NONE;
		vg_lite_float_t ScaleY = 1.0f * pImage->m_Scale.m_Y / EG_SCALE_NONE;
		vg_lite_scale(ScaleX, ScaleY, &m_VGMatrix);
	}
	vg_lite_translate(0.0f - pImage->m_Pivot.m_X, 0.0f - pImage->m_Pivot.m_Y, &m_VGMatrix);
}



#endif
