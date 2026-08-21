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

#pragma once

#include "../../../EG_IntrnlConfig.h"

#if EG_USE_GPU_NXP_VG_LITE
#include "extra/others/VGLite.h"
#include "../../sw/EG_SoftContext.h"

// Init vglite destination buffer. It will be done once per frame.
void EG_VGliteInitBuffer(const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step);

// Get vglite destination buffer pointer.
VGLiteBuffer_t* EG_VGliteGetDestBuffer(void);

// Get vglite source buffer pointer.
VGLiteBuffer_t* EG_VGliteGetSrceBuffer(void);

// Set vglite destination buffer address only.
void EG_VGliteSetDestBufferPtr(const EG_Color_t *pBuffer);

// Set vglite source buffer address only.
void EG_VGliteSetSrceBuffferPtr(const EG_Color_t *pBuffer);

// Set vglite source buffer. It will be done only if buffer addreess is different.
void EG_VGliteSetSrceBuffer(const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step);

// Set vglite buffer.
void EG_VGliteSetBuffer(VGLiteBuffer_t *pVGbuffer, const EG_Color_t *pBuffer, const EGRect *pRect, int32_t Step);

#endif
