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

#include "draw/nxp/pxp/EG_PXP_GPU.h"

#if EG_USE_GPU_NXP_PXP

#include "draw/nxp/pxp/EG_PXP_GPU_OSA.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////

static PXP_FuncConfig_t *pPXP_Functions;
static inline void PXP_InvalidateCache(void);

//////////////////////////////////////////////////////////////////////

EG_Result_t PXP_InitialiseGPU(void)
{
#if EG_USE_GPU_NXP_PXP_AUTO_INIT
  pPXP_Functions = PXP_GetFunctions();
#endif

	if(!pPXP_Functions || !pPXP_Functions->PXP_DeintInterrupt || !pPXP_Functions->PXP_InitInterrupt || !pPXP_Functions->PXP_Run || !pPXP_Functions->PXP_Wait){
		PXP_RETURN_INV("PXP configuration error.");
  }
	PXP_Init(EG_GPU_NXP_PXP_ID);
	PXP_EnableCsc1(EG_GPU_NXP_PXP_ID, false);                     /*Disable CSC1, it is enabled by default.*/
	PXP_SetProcessBlockSize(EG_GPU_NXP_PXP_ID, kPXP_BlockSize16); /*Block size 16x16 for higher performance*/
	PXP_EnableInterrupts(EG_GPU_NXP_PXP_ID, kPXP_CompleteInterruptEnable);
	if(pPXP_Functions->PXP_InitInterrupt() != EG_RES_OK) {
		PXP_DisableInterrupts(EG_GPU_NXP_PXP_ID, kPXP_CompleteInterruptEnable);
		PXP_Deinit(EG_GPU_NXP_PXP_ID);
		PXP_RETURN_INV("PXP interrupt init failed.");
	}
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////

void PXP_DeinitGPU(void)
{
	pPXP_Functions->PXP_DeintInterrupt();
	PXP_DisableInterrupts(EG_GPU_NXP_PXP_ID, kPXP_CompleteInterruptEnable);
	PXP_Deinit(EG_GPU_NXP_PXP_ID);
}

//////////////////////////////////////////////////////////////////////

void PXP_ResetGPU(void)
{
	/* Wait for previous command to complete before resetting the registers. */
	PXP_WaitGPU();
	PXP_ResetControl(EG_GPU_NXP_PXP_ID);
	PXP_EnableCsc1(EG_GPU_NXP_PXP_ID, false);                     /*Disable CSC1, it is enabled by default.*/
	PXP_SetProcessBlockSize(EG_GPU_NXP_PXP_ID, kPXP_BlockSize16); /*Block size 16x16 for higher performance*/
}

//////////////////////////////////////////////////////////////////////

void PXP_RunGPU(void)
{
	PXP_InvalidateCache();
	pPXP_Functions->PXP_Run();
}

//////////////////////////////////////////////////////////////////////

void PXP_WaitGPU(void)
{
	pPXP_Functions->PXP_Wait();
}

//////////////////////////////////////////////////////////////////////

static inline void PXP_InvalidateCache(void)
{
  EGDisplay *pDisp = GetRefreshingDisplay();
	if(pDisp->m_pDriver->CleanDcacheCB)
  pDisp->m_pDriver->CleanDcacheCB(pDisp->m_pDriver);
}

#endif
