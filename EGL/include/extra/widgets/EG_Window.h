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

#include "EGL.h"


///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_WindowClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGWindow : public EGObject
{
public:
                    EGWindow(void) : EGObject(){};
                    EGWindow(EGObject *pParent, EG_Coord_t HeaderHeight);
  virtual void      Configure(void);
  EGLabel*          AddTitle(const char *pText);
  EGButton*         AddButton(const void *pIcon, EG_Coord_t Width);
  EGObject*         GetHeader(void){ return GetChild(0); };
  EGObject*         GetContent(void){ return GetChild(1); };

  static EG_Coord_t create_header_height;
};


