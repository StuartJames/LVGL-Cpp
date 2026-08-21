/**
 * MIT License
 *
 * Copyright 2020, 2022, 2023 NXP
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

#include "draw/nxp/pxp/EG_PXP_GPU_OSA.h"

#if EG_USE_GPU_NXP_PXP && EG_USE_GPU_NXP_PXP_AUTO_INIT

#include "misc/EG_Log.h"
#include "fsl_pxp.h"

#if defined(SDK_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#endif

#if defined(__ZEPHYR__)
#include <zephyr/kernel.h>
#endif

//////////////////////////////////////////////////////////////////////

static EG_Result_t PXP_GPU_InterruptInit(void);
static void PXP_GPU_InterruptDeinit(void);
static void PXP_GPU_Run(void);
static void PXP_GPU_Wait(void);

#if defined(SDK_OS_FREE_RTOS)
static SemaphoreHandle_t s_PXP_IdleSemaphore;
#endif
#if defined(__ZEPHYR__)
static K_SEM_DEFINE(s_PXP_IdleSemaphore, 0, 1);
#endif
static volatile bool s_pxpIdle;

static PXP_FuncConfig_t PXP_DefaultConfig = {
	.PXP_InitInterrupt = PXP_GPU_InterruptInit,
	.PXP_DeintInterrupt = PXP_GPU_InterruptDeinit,
	.PXP_Run = PXP_GPU_Run,
	.PXP_Wait = PXP_GPU_Wait,
};

//////////////////////////////////////////////////////////////////////

void PXP_IRQHandler(void)
{
#if defined(SDK_OS_FREE_RTOS)
	BaseType_t TaskAwake = pdFALSE;
#endif
	if(kPXP_CompleteFlag & PXP_GetStatusFlags(EG_GPU_NXP_PXP_ID)) {
		PXP_ClearStatusFlags(EG_GPU_NXP_PXP_ID, kPXP_CompleteFlag);
#if defined(SDK_OS_FREE_RTOS)
		xSemaphoreGiveFromISR(s_PXP_IdleSemaphore, &TaskAwake);
		portYIELD_FROM_ISR(TaskAwake);
#elif defined(__ZEPHYR__)
		k_sem_give(&s_PXP_IdleSemaphore);
#else
		s_pxpIdle = true;
#endif
	}
}

//////////////////////////////////////////////////////////////////////

PXP_FuncConfig_t* PXP_GetFunctions(void)
{
	return &PXP_DefaultConfig;
}

//////////////////////////////////////////////////////////////////////

static EG_Result_t PXP_GPU_InterruptInit(void)
{
#if defined(SDK_OS_FREE_RTOS)
	s_PXP_IdleSemaphore = xSemaphoreCreateBinary();
	if(s_PXP_IdleSemaphore == NULL) return EG_RES_INV;
	NVIC_SetPriority(EG_GPU_NXP_PXP_IRQ_ID, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
#endif
	s_pxpIdle = true;
	NVIC_EnableIRQ(EG_GPU_NXP_PXP_IRQ_ID);
	return EG_RES_OK;
}

//////////////////////////////////////////////////////////////////////

static void PXP_GPU_InterruptDeinit(void)
{
	NVIC_DisableIRQ(EG_GPU_NXP_PXP_IRQ_ID);
#if defined(SDK_OS_FREE_RTOS)
	vSemaphoreDelete(s_PXP_IdleSemaphore);
#elif defined(__ZEPHYR__)
	k_sem_reset(&s_PXP_IdleSemaphore);
#endif
}

//////////////////////////////////////////////////////////////////////

// Start PXP job.
static void PXP_GPU_Run(void)
{
  s_pxpIdle = false;
	PXP_EnableInterrupts(EG_GPU_NXP_PXP_ID, kPXP_CompleteInterruptEnable);
	PXP_Start(EG_GPU_NXP_PXP_ID);
}

//////////////////////////////////////////////////////////////////////

// Wait for PXP completion.
static void PXP_GPU_Wait(void)
{
#if defined(SDK_OS_FREE_RTOS) || defined(__ZEPHYR__)
	if(s_pxpIdle == true)	return;	// Return if PXP was never started, otherwise the semaphore will lock forever.
#endif
#if defined(SDK_OS_FREE_RTOS)
	if(xSemaphoreTake(s_PXP_IdleSemaphore, portMAX_DELAY) == pdTRUE)	s_pxpIdle = true;
#elif defined(__ZEPHYR__)
	if(k_sem_take(&s_PXP_IdleSemaphore, K_FOREVER) == 0)	s_pxpIdle = true;
#else
	while(s_pxpIdle == false) {
	}
#endif
}

#endif
