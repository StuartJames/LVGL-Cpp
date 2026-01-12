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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "EGL.h"

//////////////////////////////////////////////////////////////////////

#define EG_NO_TASK_READY EG_NO_TIMER_READY
#define EG_INDEV_STATE_REL EG_INDEV_STATE_RELEASED
#define EG_INDEV_STATE_PR EG_INDEV_STATE_PRESSED
#define EG_OBJ_FLAG_SNAPABLE EG_OBJ_FLAG_SNAPPABLE /*Fixed typo*/

//////////////////////////////////////////////////////////////////////

static inline EG_ATTRIBUTE_TIMER_HANDLER uint32_t EG_TaskHandler(void)
{
	return EGTimer::TimerHandler();
}

//////////////////////////////////////////////////////////////////////

// Move the object to the foreground. It will look as though it was created as the last child of its parent.
static inline void EG_MoveToForeground(EGObject *pObj)
{
	EGObject *pParent = pObj->GetParent();
	pObj->MoveToIndex(pParent->GetChildCount() - 1);
}

//////////////////////////////////////////////////////////////////////

// Move the object to the background. It will look as though it was created as the first child of its parent.
static inline void EG_MoveToBackground(EGObject *pObj)
{
	pObj->MoveToIndex(0);
}

//////////////////////////////////////////////////////////////////////

static inline uint32_t EG_GetChildID(const EGObject *pObj)
{
	return pObj->GetIndex();
}
