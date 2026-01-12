/*
 *                LEGL 2025-2026 HydraSystems.
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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include <stdint.h>
#include <string.h>
#include "core/EG_Object.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Refresh.h"
#include "core/EG_Group.h"
#include "core/EG_Display.h"
#include "core/EG_Theme.h"
#include "misc/EG_Assert.h"
#include "hal/EG_HAL.h"
#include "extra/EG_Extra.h"

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_GPU_STM32_DMA2D
#include "../draw/stm32_dma2d/lv_gpu_stm32_dma2d.h"
#endif

#if EG_USE_GPU_RA6M3_G2D
#include "../draw/renesas/lv_gpu_d2_ra6m3.h"
#endif

#if EG_USE_GPU_SWM341_DMA2D
#include "../draw/swm341_dma2d/lv_gpu_swm341_dma2d.h"
#endif

#if EG_USE_GPU_NXP_PXP && EG_USE_GPU_NXP_PXP_AUTO_INIT
#include "../draw/nxp/pxp/lv_gpu_nxp_pxp.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool EGInitialized = false;

extern EGDefTheme EGDefaultTheme;
extern EGDecoderBuiltIn DecoderBuiltIn;

///////////////////////////////////////////////////////////////////////////////////////////////////

void EG_Initialise(void)
{
	if(EGInitialized){	                      // Do nothing if already initialized
		EG_LOG_WARN("Already initialised");
		return;
	}
	EG_LOG_INFO("Starting initialisation");
	EG_InitMem();
	EGTimer::InitialiseCore();
  EGAnimate::InitialiseCore();
  EGImageDecoder::Register(&DecoderBuiltIn);
#if EG_USE_GPU_STM32_DMA2D
  lv_draw_stm32_dma2d_init();	              // Initialize DMA2D GPU
	#endif
#if EG_USE_GPU_RA6M3_G2D
	lv_draw_ra6m3_g2d_init();	                // Initialize G2D GPU
#endif
#if EG_USE_GPU_SWM341_DMA2D
	lv_draw_swm341_dma2d_init();	            // Initialize DMA2D GPU
#endif
#if EG_USE_GPU_NXP_PXP && EG_USE_GPU_NXP_PXP_AUTO_INIT
	PXP_COND_STOP(!lv_gpu_nxp_pxp_init(), "PXP init failed.");
#endif
	RefreshInitialise();	                        // Initialize the screen refresh system
#if EG_IMG_CACHE_DEF_SIZE
	lv_img_cache_set_size(EG_IMG_CACHE_DEF_SIZE);
#endif

  const char *pText = "Á";	                // Test if the IDE has UTF-8 encoding
	const uint8_t *pTextU8 = (uint8_t *)pText;
	if(pTextU8[0] != 0xc3 || pTextU8[1] != 0x81 || pTextU8[2] != 0x00) {
		EG_LOG_WARN("The strings have no UTF-8 encoding. Non-ASCII characters won't be displayed.");
	}
  uint32_t EndianessTest = 0x11223344;
	uint8_t *pEndianessTest = (uint8_t *)&EndianessTest;
	bool BigEndian = pEndianessTest[0] == 0x11 ? true : false;
	if(BigEndian) EG_ASSERT_MSG(EG_BIG_ENDIAN_SYSTEM == 1, "It's a big endian system but EG_BIG_ENDIAN_SYSTEM is not enabled in EG_Config.h");
	else EG_ASSERT_MSG(EG_BIG_ENDIAN_SYSTEM == 0, "It's a little endian system but EG_BIG_ENDIAN_SYSTEM is enabled in EG_Config.h");
#if EG_USE_ASSERT_MEM_INTEGRITY
	EG_LOG_WARN("Memory integrity checks are enabled via EG_USE_ASSERT_MEM_INTEGRITY which makes LVGL much slower");
#endif
#if EG_USE_ASSERT_OBJ
	EG_LOG_WARN("Object sanity checks are enabled via EG_USE_ASSERT_OBJ which makes LVGL much slower");
#endif
#if EG_USE_ASSERT_STYLE
	EG_LOG_WARN("Style sanity checks are enabled that uses more RAM");
#endif
#if EG_LOG_LEVEL == EG_LOG_LEVEL_TRACE
	EG_LOG_WARN("Log level is set to 'Trace' which makes LVGL much slower");
#endif

  EG_InitialiseExtra();
	EGInitialized = true;
	EG_LOG_TRACE("finished");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EG_Deinitialise(void)
{
    EG_GC_ClearRoots();
    EGDisplay::SetDefault(nullptr);
    EG_DeinitMem();
    EGInitialized = false;
    EG_LOG_INFO("lv_deinit done");

#if EG_USE_LOG
    EG_RegisterLogPrintCB(NULL);
#endif
}

