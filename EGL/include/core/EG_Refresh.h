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

#pragma once

#include "EG_Object.h"
#include <stdbool.h>
#include "misc/EG_Timer.h"

/////////////////////////////////////////////////////////////////////////////////////

#define EG_REFR_TASK_PRIO EG_TASK_PRIO_MID

class EGDisplayDriver;
extern EGDrawRect *g_pDrawRect;

/////////////////////////////////////////////////////////////////////////////////////

void            RefreshInitialise(void);
void            RefreshNow(EGDisplay *pDisplay);
#if EG_USE_PERF_MONITOR
void            ResetFPSCounter(void);
uint32_t        GetAverageFPS(void);
#endif

void            RedrawObject(EGDrawContext *pContext, EGObject *pObj);
void            InvalidateRect(EGDisplay *pDisplay, const EGRect *pRect);
EGDisplay*      GetRefreshingDisplay(void);
void            SetRefreshingDisplay(EGDisplay *pDisplay);
void            RefreshTimerCB(EGTimer *pTimer);


