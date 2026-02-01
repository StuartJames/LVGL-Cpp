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

#if EG_USE_LED

///////////////////////////////////////////////////////////////////////////////////////

#ifndef EG_LED_BRIGHT_MIN
# define EG_LED_BRIGHT_MIN 80
#endif

#ifndef EG_LED_BRIGHT_MAX
# define EG_LED_BRIGHT_MAX 255
#endif

///////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    EG_LED_DRAW_PART_RECTANGLE,    // The main rectangle
} EG_LedDrawPartType_e;

extern const EG_ClassType_t c_LedClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGLed : public EGObject
{
public:
                    EGLed(void) : EGObject(){};
                    EGLed(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_LedClass);
  virtual void      Configure(void);
  void              SetColor(EG_Color_t Color);
  void              SetBrightness(uint8_t Brightness);
  void              SetOn(void);
  void              SetOff(void);
  void              Toggle(void);
  uint8_t           GetBrightness(void);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  EG_Color_t        m_LedColor;
  uint8_t           m_Brightness;     // Current brightness of the LED (0..255)
};


#endif 