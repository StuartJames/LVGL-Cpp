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

#if EG_USE_SPINBOX

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_TEXTAREA == 0
#error "Spinbox: TextArea is required. Enable it in EG_Config.h (EG_USE_TEXTAREA  1) "
#endif

#define EG_SPINBOX_MAX_DIGIT_COUNT 10

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_SpinboxClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGSpinBox : public EGEdit
{
public:
                    EGSpinBox(void);
                    EGSpinBox(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_SpinboxClass);
  virtual void      Configure(void);
  void              SetValue(int32_t Value);
  void              SetRollover(bool Enable);
  void              SetDigitFormat(uint8_t DigitCount, uint8_t SeparatorPosition);
  void              SetStep(uint32_t Step);
  void              SetRange(int32_t MinRange, int32_t MaxRange);
  void              SetCursorPos(uint8_t Pos);
  void              SetStepDirection(EG_DirType_e Direction);
  bool              GetRollover(void){ return m_Rollover; };
  int32_t           GetValue(void){ return m_Value; };
  int32_t           GetStep(void){ return m_Step; };
  void              StepNext(void);
  void              StepPrev(void);
  void              Increment(void);
  void              Decrement(void);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  int32_t           m_Value;
  int32_t           m_MaxRange;
  int32_t           m_MinRange;
  int32_t           m_Step;
  uint16_t          m_DigitCount : 4;
  uint16_t          m_DecimalPointPos : 4;   // if 0, there is no separator and the number is an integer
  uint16_t          m_Rollover : 1;        // Set to true for rollover functionality
  uint16_t          m_StepDir : 2;  // the direction the digit will step on encoder button press when editing

private:
  void              UpdateValue(void);
};


#endif