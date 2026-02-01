/*
 *                EGL 2025-2026 HydraSystems.
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

#include "hal/EG_HALTick.h"
#include <stddef.h>

///////////////////////////////////////////////////////////////////////

#if EG_TICK_CUSTOM == 1
#include EG_TICK_CUSTOM_INCLUDE
#endif

#if !EG_TICK_CUSTOM
static uint32_t sys_time = 0;
static volatile uint8_t tick_irq_flag;
#endif

///////////////////////////////////////////////////////////////////////

#if !EG_TICK_CUSTOM

void EG_ATTRIBUTE_TICK_INC EG_IncrementTick(uint32_t tick_period) // You have to call this function periodically
{
	tick_irq_flag = 0;
	sys_time += tick_period;
}

#endif

///////////////////////////////////////////////////////////////////////

uint32_t EG_GetTick(void)  // Get the elapsed milliseconds since start up
{
#if EG_TICK_CUSTOM == 0

	/*If `EG_IncrementTick` is called from an interrupt while `sys_time` is read the result might be corrupted.
     *This loop detects if `EG_IncrementTick` was called while reading `sys_time`.
     *If `tick_irq_flag` was cleared in `EG_IncrementTick` try to read again until `tick_irq_flag` remains `1`.*/
	uint32_t result;
	do{
		tick_irq_flag = 1;
		result = sys_time;
	}
  while(!tick_irq_flag);  // Continue until see a non interrupted cycle
	return result;
#else
	return EG_TICK_CUSTOM_SYS_TIME_EXPR;
#endif
}

///////////////////////////////////////////////////////////////////////

uint32_t EG_TickElapse(uint32_t prev_tick)  // Get the elapsed milliseconds since a previous time stamp
{
	uint32_t act_time = EG_GetTick();
	if(act_time >= prev_tick) {	              // If there is no overflow in sys_time simple subtract
		prev_tick = act_time - prev_tick;
	}
	else {
		prev_tick = UINT32_MAX - prev_tick + 1;
		prev_tick += act_time;
	}
	return prev_tick;
}

