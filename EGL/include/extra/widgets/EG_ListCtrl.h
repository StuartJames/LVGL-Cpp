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

#include "core/EG_Object.h"
#include "extra/layouts/EG_Flex.h"

#if EG_USE_LIST

///////////////////////////////////////////////////////////////////////////////////////

class EGButton;
class EGLabel;

extern const EG_ClassType_t c_ListClass;
extern const EG_ClassType_t c_ListTextClass;
extern const EG_ClassType_t c_ListButtonClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGListCtrl : public EGObject
{
public:
                      EGListCtrl(void) : EGObject(){};                   
                      EGListCtrl(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ListClass);
  EGLabel*            AddText(const char *pText);
  EGButton*           AddButton(const void *pIcon, const char *pText);
  const char*         GetButtonText(EGObject *pButton);
 
};

#endif 