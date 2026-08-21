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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

#if EG_USE_SLIDER != 0

#if EG_USE_BAR == 0 // Testing of dependencies
#error "EGBar is required. Enable it in EG_Config.h (EG_USE_BAR 1)"
#endif

#include "../core/EG_Object.h"
#include "EG_Bar.h"

///////////////////////////////////////////////////////////////////////////////////////

enum {
    EG_SLIDER_MODE_NORMAL = EG_BAR_MODE_NORMAL,
    EG_SLIDER_MODE_SYMMETRICAL = EG_BAR_MODE_SYMMETRICAL,
    EG_SLIDER_MODE_RANGE = EG_BAR_MODE_RANGE
};
typedef uint8_t EG_SliderMode_e;

// `type` field in `EG_DrawPartDescriptor_t` if `class_p = c_SliderClass`
 enum EG_SliderDrawPartType_e{
    EG_SLIDER_DRAW_PART_KNOB,           // The main (right) knob's rectangle
    EG_SLIDER_DRAW_PART_KNOB_LEFT,      // The left knob's rectangle
} ;


extern const EG_ClassType_t c_SliderClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGSlider : public EGBar
{
public:
                      EGSlider(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_SliderClass);
  virtual void        Configure(void);
  void                Event(EGEvent *pEvent);
  void                PositionKnob(EGRect *knob_area, const int32_t knob_size, const bool hor);
  bool                IsHorizontal(void);
  EG_SliderMode_e     GetMode(void);
  bool                IsDragged(void);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  EGRect              m_LeftKnobRect;
  EGRect              m_RightKnobRect;
  int32_t            *m_pValueToSet;        // Which bar value to set
  uint8_t             m_IsDragging : 1;     // the slider is being dragged
  uint8_t             m_LeftKnobFocus : 1;  // with encoder now the right knob can be adjusted

private:
  void                DrawKnob(EGEvent *pEvent);

};

///////////////////////////////////////////////////////////////////////////////////////


inline EG_SliderMode_e EGSlider::GetMode(void)
{
    EG_SliderMode_e Mode = EGBar::GetMode();
    if(Mode == EG_BAR_MODE_SYMMETRICAL) return EG_SLIDER_MODE_SYMMETRICAL;
    else if(Mode == EG_BAR_MODE_RANGE) return EG_SLIDER_MODE_RANGE;
    else return EG_SLIDER_MODE_NORMAL;
}


#endif 