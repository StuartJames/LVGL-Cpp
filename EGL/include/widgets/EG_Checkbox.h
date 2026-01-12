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

#include "../EG_IntrnlConfig.h"
#include "../core/EG_Object.h"

#if EG_USE_CHECKBOX != 0

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_CheckboxClass;

typedef enum {
    LV_CHECKBOX_DRAW_PART_BOX,    // The tick box
} EG_CheckboxDrawPartType_e;

///////////////////////////////////////////////////////////////////////////////////////

class EGCheckbox : public EGObject
{
public:
                    EGCheckbox(void);
                    EGCheckbox(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_CheckboxClass);
                    ~EGCheckbox(void);
  virtual void      Configure(void);
  void              SetText(const char *pText);
  void              SetStaticText(const char *pText);
  const char*       GetText(void);
  void              Event(EGEvent *pEvent);
  void              Draw(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  const char       *m_pText;
  uint32_t          m_StaticText : 1;
};




#endif 

