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

#pragma once

#include "EG_IntrnlConfig.h"

#include <stdint.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////

#ifndef EG_ATTRIBUTE_TICK_INC
#define EG_ATTRIBUTE_TICK_INC
#endif

#if !EG_TICK_CUSTOM
void /* EG_ATTRIBUTE_TICK_INC */ EG_IncrementTick(uint32_t tick_period);
#endif
uint32_t EG_GetTick(void);
uint32_t EG_TickElapse(uint32_t prev_tick);

