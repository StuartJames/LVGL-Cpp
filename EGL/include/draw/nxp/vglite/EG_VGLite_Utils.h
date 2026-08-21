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

#if EG_USE_GPU_NXP_VG_LITE
#include "extra/others/VGLite.h"
#include "../../sw/EG_SoftContext.h"
#include "misc/EG_Log.h"

#ifndef EG_GPU_NXP_VG_LITE_LOG_ERRORS
/** Enable logging of VG-Lite errors (\see EG_LOG_ERROR)*/
#define EG_GPU_NXP_VG_LITE_LOG_ERRORS 1
#endif

#ifndef EG_GPU_NXP_VG_LITE_LOG_TRACES
/** Enable logging of VG-Lite traces (\see EG_LOG_ERROR)*/
#define EG_GPU_NXP_VG_LITE_LOG_TRACES 0
#endif

/* The optimal Bezier control point offset for radial unit
 * see: https://spencermortensen.com/articles/bezier-circle/
 **/
#define BEZIER_OPTIM_CIRCLE 0.551915024494f

/* Draw lines for control points of Bezier curves */
#define BEZIER_DBG_CONTROL_POINTS 0

// Enable scissor and set the clipping box.
static inline void EG_VGliteSetScissor(const EGRect *pClipRect);

static inline void EG_VGliteDisableScissor(void);

EG_Result_t EG_VGlitePremultSwizzle(vg_lite_color_t *pVGColor32, EG_Color32_t EG_Color32, EG_OPA_t OPA, VGLiteBufferFormat_e vg_col_format);

VGLiteBlend_e EG_VGlightGetBlendMode(EG_BlendMode_e m_BlendMode);

EG_Result_t EG_VGliteRun(void); // Clear cache and flush command to VG-Lite.

//////////////////////////////////////////////////////////////////////////////////////

#define VG_LITE_COND_STOP(cond, txt)  \
	do {                                \
		if(cond) {                        \
			EG_LOG_ERROR("%s. STOP!", txt); \
			for(;;)                         \
				;                             \
		}                                 \
	} while(0)

#if EG_GPU_NXP_VG_LITE_LOG_ERRORS
#define VG_LITE_ERR_RETURN_INV(err, fmt, ...) \
	do {                                        \
		if(err != VG_LITE_SUCCESS) {              \
			EG_LOG_ERROR(fmt " (err = %d)",         \
									 err, ##__VA_ARGS__);       \
			return EG_RES_INVALID;                      \
		}                                         \
	} while(0)
#else
#define VG_LITE_ERR_RETURN_INV(err, fmt, ...) \
	do {                                        \
		if(err != VG_LITE_SUCCESS) {              \
			return EG_RES_INVALID;                      \
		}                                         \
	} while(0)
#endif

#if EG_GPU_NXP_VG_LITE_LOG_TRACES
#define VG_LITE_LOG_TRACE(fmt, ...) \
	do {                              \
		EG_LOG(fmt, ##__VA_ARGS__);     \
	} while(0)

#define VG_LITE_RETURN_INV(fmt, ...)  \
	do {                                \
		EG_LOG_ERROR(fmt, ##__VA_ARGS__); \
		return EG_RES_INVALID;                \
	} while(0)
#else
#define VG_LITE_LOG_TRACE(fmt, ...) \
	do {                              \
	} while(0)
#define VG_LITE_RETURN_INV(fmt, ...) \
	do {                               \
		return EG_RES_INVALID;               \
	} while(0)
#endif 

//////////////////////////////////////////////////////////////////////////////////////

static inline void EG_VGliteSetScissor(const EGRect *pClipRect)
{
	vg_lite_enable_scissor();
	vg_lite_set_scissor((int32_t)pClipRect->GetX1(), (int32_t)pClipRect->GetY1(),
											(int32_t)pClipRect->GetWidth(),	(int32_t)pClipRect->GetHeight());
}

//////////////////////////////////////////////////////////////////////////////////////

static inline void EG_VGliteDisableScissor(void)
{
	vg_lite_disable_scissor();
}

#endif
