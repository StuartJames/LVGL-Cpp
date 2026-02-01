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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

/***************************
 * CURRENT VERSION OF LVGL
 ***************************/
#define EG_VERSION_MAJOR 8
#define EG_VERSION_MINOR 4
#define EG_VERSION_PATCH 0
#define EG_VERSION_INFO ""

#include "misc/EG_Log.h"
#include "misc/EG_Timer.h"
#include "misc/EG_Math.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Async.h"
#include "misc/EG_Timeline.h"
#include "misc/lv_printf.h"
#include "misc/EG_List.h"

#include "hal/EG_HAL.h"

#include "core/EG_Object.h"
#include "core/EG_Group.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Refresh.h"
#include "core/EG_Display.h"
#include "core/EG_Theme.h"

#include "font/EG_Font.h"
#include "font/EG_FontLoader.h"
#include "font/EG_FontFmtText.h"

#include "widgets/EG_Arc.h"
#include "widgets/EG_Button.h"
#include "widgets/EG_Image.h"
#include "widgets/EG_Label.h"
#include "widgets/EG_Line.h"
#include "widgets/EG_Table.h"
#include "widgets/EG_Checkbox.h"
#include "widgets/EG_Bar.h"
#include "widgets/EG_Slider.h"
#include "widgets/EG_ButtonMatrix.h"
#include "widgets/EG_DropDown.h"
#include "widgets/EG_Roller.h"
#include "widgets/EG_Edit.h"
#include "widgets/EG_Canvas.h"
#include "widgets/EG_Switch.h"

#include "draw/EG_DrawContext.h"
#include "draw/sw/EG_SoftContext.h"

#include "EG_APIMap.h"

#include "extra/EG_Extra.h"

#include "esp_log.h"

#define EG_VERSION_CHECK(x,y,z) (x == EG_VERSION_MAJOR && (y < EG_VERSION_MINOR || (y == EG_VERSION_MINOR && z <= EG_VERSION_PATCH)))

// Wrapper functions for VERSION macros

static inline int EG_VersionMajor(void)
{
    return EG_VERSION_MAJOR;
}

static inline int EG_VersionMinor(void)
{
    return EG_VERSION_MINOR;
}

static inline int EG_VersionPatch(void)
{
    return EG_VERSION_PATCH;
}

static inline const char *EG_VersionInfo(void)
{
    return EG_VERSION_INFO;
}


