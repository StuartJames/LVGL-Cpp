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

#include "../EG_IntrnlConfig.h"

#if EG_USE_SWITCH != 0

#include "../core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////////////

#define _EG_SWITCH_KNOB_EXT_AREA_CORRECTION 2 // Switch knob extra area correction factor 

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_SwitchClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGSwitch : public EGObject
{
public:
                        EGSwitch(void);
                        EGSwitch(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_SwitchClass);
                        ~EGSwitch(void);
  virtual void          Configure(void);
  void                  TriggerAnimate(void);

  static void           EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void           AnimateExecCB(EGAnimate *pAnimate, int32_t Value);
  static void           AnimateEndCB(EGAnimate *pAnimate);

  int32_t               m_AnimateState;

private:
  void                  DrawMain(EGEvent *pEvent);

};

#endif