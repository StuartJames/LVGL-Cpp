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

#include "fsl_cache.h"
#include "fsl_pxp.h"

#include "misc/EG_Log.h"

//////////////////////////////////////////////////////////////////////

#define EG_GPU_NXP_PXP_ID         PXP // PXP module instance to use
#define EG_GPU_NXP_PXP_IRQ_ID     PXP_IRQn // PXP interrupt line ID

#ifndef EG_GPU_NXP_PXP_LOG_ERRORS
#define EG_GPU_NXP_PXP_LOG_ERRORS 1   // Enable logging of PXP errors (\see EG_LOG_ERROR)
#endif

#ifndef EG_GPU_NXP_PXP_LOG_TRACES
#define EG_GPU_NXP_PXP_LOG_TRACES 0   // Enable logging of PXP errors (\see EG_LOG_ERROR)
#endif

// NXP PXP device configuration - call-backs used for interrupt init/wait/deinit.
typedef struct {
	EG_Result_t (*PXP_InitInterrupt)(void);	// Callback for PXP interrupt initialization
	void (*PXP_DeintInterrupt)(void);	  // Callback for PXP interrupt de-initialization
	void (*PXP_Run)(void);	              // Callback for PXP start
	void (*PXP_Wait)(void);	              // Callback for waiting of PXP completion
} PXP_FuncConfig_t;

//////////////////////////////////////////////////////////////////////

/* Reset and initialize PXP device. This function should be called as a part
 * of display init sequence.
 *
 * @retval EG_RES_OK PXP init completed
 * @retval EG_RES_INV Error occurred (\see EG_GPU_NXP_PXP_LOG_ERRORS) */
EG_Result_t PXP_InitialiseGPU(void);

void PXP_DeinitGPU(void); // Disable PXP device. Should be called during display deinit sequence.
void PXP_ResetGPU(void);  // Reset PXP device.
void PXP_RunGPU(void);    // Clear cache and start PXP.
void PXP_WaitGPU(void);   // Wait for PXP completion.

//////////////////////////////////////////////////////////////////////

#define PXP_COND_STOP(cond, txt)      \
	do {                                \
		if(cond) {                        \
			EG_LOG_ERROR("%s. STOP!", txt); \
			for(;;)                         \
				;                             \
		}                                 \
	} while(0)

#if EG_GPU_NXP_PXP_LOG_ERRORS
#define PXP_RETURN_INV(fmt, ...)      \
	do {                                \
		EG_LOG_ERROR(fmt, ##__VA_ARGS__); \
		return EG_RES_INVALID;                \
	} while(0)
#else
#define PXP_RETURN_INV(fmt, ...) \
	do {                           \
		return EG_RES_INVALID;           \
	} while(0)
#endif

#if EG_GPU_NXP_PXP_LOG_TRACES
#define PXP_LOG_TRACE(fmt, ...) \
	do {                          \
		EG_LOG(fmt, ##__VA_ARGS__); \
	} while(0)
#else
#define PXP_LOG_TRACE(fmt, ...) \
	do {                          \
	} while(0)
#endif

#endif
