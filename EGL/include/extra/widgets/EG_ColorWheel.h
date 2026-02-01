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

#if EG_USE_COLORWHEEL

///////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    EG_COLORWHEEL_MODE_HUE,
    EG_COLORWHEEL_MODE_SATURATION,
    EG_COLORWHEEL_MODE_VALUE
} EG_ColorWheelMode_e;

extern const EG_ClassType_t c_ColorWheelClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGColorWheel : public EGObject
{
public:
                        EGColorWheel(void);
                        EGColorWheel(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ColorWheelClass);
  virtual void          Configure(void);
  bool                  SetHSV(EG_ColorHSV_t HSV);
  bool                  SetRGB(EG_Color_t Color);
  void                  SetMode(EG_ColorWheelMode_e Mode);
  void                  SetFixedMode(bool Fixed);
  EG_ColorHSV_t        GetHSV(void);
  EG_Color_t            GetRGB(void);
  EG_ColorWheelMode_e   GetMode(void);
  bool                  GetFixedMode(void);
  void                  Event(EGEvent *pEvent);

  static void           EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void           SetRecolor(bool Recolor){ m_CreateKnobRecolor = Recolor; };

  EG_ColorHSV_t        m_HSV;
  struct {
    EGPoint               Position;
    uint8_t               Recolor : 1;
  }                     m_Knob;
  uint32_t              m_LastClickTime;
  uint32_t              m_LastChangeTime;
  EGPoint               m_LastPressPoint;
  EG_ColorWheelMode_e   m_Mode      : 2;
  uint8_t               m_FixedMode : 1;

private:

  void                  DrawDiscGrad(EGEvent *pEvent);
  void                  DrawKnob(EGEvent *pEvent);
  void                  InvalidateKnob(void);
  EGRect                GetKnobArea(void);
  void                  NextColorMode(void);
  void                  RefreshKnobPosition(void);
  EG_Result_t           ResetDoubleClick(void);
  void                  FastHSV2RGB(uint16_t H, uint8_t S, uint8_t V, uint8_t *pR, uint8_t *pG, uint8_t *pB);
  EG_Color_t            AngleToModeColorFast(uint16_t Angle);
  uint16_t              GetAngle(void);

  static bool           m_CreateKnobRecolor;

};


#endif  