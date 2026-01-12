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

#if EG_USE_BAR != 0

#include "../core/EG_Object.h"
#include "../misc/EG_Animate.h"
#include "EG_Button.h"
#include "EG_Label.h"

///////////////////////////////////////////////////////////////////////////////////////

enum {
  EG_BAR_MODE_NORMAL,
  EG_BAR_MODE_SYMMETRICAL,
  EG_BAR_MODE_RANGE
};
typedef uint8_t EG_BarMode_e;

typedef struct {
  EGObject *pBar;
  int32_t   AnimstionStart;
  int32_t   AnimationEnd;
  int32_t   AnimationState;
} EG_BarAnimation_t;

typedef enum {
  EG_BAR_DRAW_PART_INDICATOR,    // The indicator
} lv_bar_draw_part_type_t;

extern const EG_ClassType_t c_BarClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGBar : public EGObject
{
public:
                    EGBar(void);
                    EGBar(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_BarClass);
                    ~EGBar(void);
  virtual void      Configure(void);
  void              SetValue(int32_t value, EG_AnimateEnable_e Animate);
  void              SetStartValue(int32_t start_value, EG_AnimateEnable_e Animate);
  void              SetRange(int32_t min, int32_t max);
  void              SetMode(EG_BarMode_e mode);
  int32_t           GetValue(void) const;
  int32_t           GetStartValue(void) const;
  int32_t           GetMinimumValue(void) const;
  int32_t           GetMaximumValue(void) const;
  EG_BarMode_e      GetMode(void) const;

  static void       Event(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void       Animate(void *pValue, int32_t value);
  static void       AnimateEnd(EGAnimate *pAnimation);

  int32_t           m_CurrentValue;          // Current value of the bar
  int32_t           m_MinimumValue;          // Minimum value of the bar
  int32_t           m_MaximumValue;          // Maximum value of the bar
  int32_t           m_StartValue;        // Start value of the bar
  EGRect            m_IndicatorRect;         // Save the indicator area. Might be used by derived types
  EG_BarAnimation_t m_CurrentValueAnimation;
  EG_BarAnimation_t m_StartValueAnimation;
  EG_BarMode_e      m_Mode : 2;           // Type of bar

private:
  void              DrawIndicator(EGEvent *pEvent);
  void              SetValueWithAnimation(int32_t new_value, int32_t *value_ptr, EG_BarAnimation_t *anim_info, EG_AnimateEnable_e en);
  void              InitialiseAnimmation(EG_BarAnimation_t *bar_anim);

};

#endif
