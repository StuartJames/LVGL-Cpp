/**
 * MIT License
 *
 * Copyright 2023 NXP
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

#include "draw/nxp/vglite/EG_VGLite_Buffer.h"

#if EG_USE_GPU_NXP_VG_LITE

/*********************
 *      DEFINES
 *********************/

#if EG_COLOR_DEPTH == 16
#define VG_LITE_PX_FMT VG_LITE_RGB565
#elif EG_COLOR_DEPTH == 32
#define VG_LITE_PX_FMT VG_LITE_BGRA8888
#else
#error Only 16bit and 32bit color depth are supported. Set EG_COLOR_DEPTH to 16 or 32.
#endif

//////////////////////////////////////////////////////////////////////////////////////

static inline void EG_VGliteSetDestBuffer(const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step);
static inline void EG_VGliteSetBufferPtr(VGLiteBuffer_t *pVGBuffer, const EG_Color_t *pBuffer);

//////////////////////////////////////////////////////////////////////////////////////

static VGLiteBuffer_t DestVGBuffer;
static VGLiteBuffer_t SrceVGBuffer;

//////////////////////////////////////////////////////////////////////////////////////

void EG_VGliteInitBuffer(const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step)
{
	EG_VGliteSetDestBuffer(pBuffer, pRect, Step);
}

//////////////////////////////////////////////////////////////////////////////////////

VGLiteBuffer_t* EG_VGliteGetDestBuffer(void)
{
	return &DestVGBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////

VGLiteBuffer_t* EG_VGliteGetSrceBuffer(void)
{
	return &SrceVGBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_VGliteSetDestBufferPtr(const EG_Color_t *pBuffer)
{
	EG_VGliteSetBufferPtr(&DestVGBuffer, pBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_VGliteSetSrceBuffferPtr(const EG_Color_t *pBuffer)
{
	EG_VGliteSetBufferPtr(&SrceVGBuffer, pBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_VGliteSetSrceBuffer(const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step)
{
	if(SrceVGBuffer.memory != (void*)pBuffer)
  EG_VGliteSetBuffer(&SrceVGBuffer, pBuffer, pRect, Step);
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_VGliteSetBuffer(VGLiteBuffer_t *pVGBuffer, const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step)
{
	pVGBuffer->format = VG_LITE_PX_FMT;
	pVGBuffer->tiled = VG_LITE_LINEAR;
	pVGBuffer->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
	pVGBuffer->transparency_mode = VG_LITE_IMAGE_OPAQUE;
	pVGBuffer->width = (int32_t)pRect->GetWidth();
	pVGBuffer->height = (int32_t)pRect->GetHeight();
	pVGBuffer->stride = (int32_t)(Step) * sizeof(EG_Color_t);
	EG_SetMem(&pVGBuffer->yuv, 0, sizeof(pVGBuffer->yuv));
	pVGBuffer->memory = (void *)pBuffer;
	pVGBuffer->address = (uint32_t)pVGBuffer->memory;
	pVGBuffer->handle = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////

static inline void EG_VGliteSetDestBuffer(const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step)
{
	EG_VGliteSetBuffer(&DestVGBuffer, pBuffer, pRect, Step);
}

//////////////////////////////////////////////////////////////////////////////////////

static inline void EG_VGliteSetBufferPtr(VGLiteBuffer_t *pVGBuffer, const EG_Color_t *pBuffer)
{
	pVGBuffer->memory = (void *)pBuffer;
	pVGBuffer->address = (uint32_t)pVGBuffer->memory;
}

#endif
