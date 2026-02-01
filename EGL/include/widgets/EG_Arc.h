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

#if EG_USE_ARC != 0

#include "../core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////////////

enum {
    EG_ARC_MODE_NORMAL,
    EG_ARC_MODE_SYMMETRICAL,
    EG_ARC_MODE_REVERSE
};
typedef uint8_t EG_ArcMode_e;

/**
 * `type` field in `EG_DrawPartDiscriptor_t` if `class_p = lv_arc_class`
 * Used in `EG_EVENT_DRAW_PART_BEGIN` and `EG_EVENT_DRAW_PART_END`
 */
typedef enum {
    EG_ARC_DRAW_PART_BACKGROUND,    //  The background arc
    EG_ARC_DRAW_PART_FOREGROUND,    //  The foreground arc
    EG_ARC_DRAW_PART_KNOB,          //  The knob
} lv_arc_draw_part_type_t;

extern const EG_ClassType_t c_ArcClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGArc : public EGObject
{
public:
                    EGArc(void);
                    EGArc(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ArcClass);
  virtual void      Configure(void);
  void              SetStartAngle(uint16_t Start);
  void              SetEndAngle(uint16_t End);
  void              SetAngles(uint16_t Start, uint16_t End);
  void              SetBackgroundStartAngle(uint16_t Start);
  void              SetBackgroundEndAngle(uint16_t End);
  void              SetBackgroundAngles(uint16_t Start, uint16_t End);
  void              SetRotation(uint16_t Rotation);
  void              SetMode(EG_ArcMode_e Type);
  void              SetValue(int16_t Value);
  void              SetRange(int16_t Min, int16_t Max);
  void              SetChangeRate(uint16_t Rate);
  uint16_t          GetAngleStart(void);
  uint16_t          GetAngleEnd(void);
  uint16_t          GetBackgroundAngleStart(void);
  uint16_t          GetBackgroundAngleEnd(void);
  int16_t           GetValue(void);
  int16_t           GetMinValue(void);
  int16_t           GetMaxValue(void);
  EG_ArcMode_e      GetMode(void);
  void              AlignToAngle(EGObject *pObj, EG_Coord_t Offset);
  void              RotateToAngle(EGObject * pObj, EG_Coord_t Offset);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  uint16_t          m_Rotation;
  uint16_t          m_IndicAngleStart;
  uint16_t          m_IndicAngleEnd;
  uint16_t          m_BackgroundAngleStart;
  uint16_t          m_BackgroundAngleEnd;
  int16_t           m_Value;                  // Current value of the arc
  int16_t           m_MinimumValue;           // Minimum value of the arc
  int16_t           m_MaximumValue;           // Maximum value of the arc
  uint32_t          m_Dragging          : 1;
  uint32_t          m_Type              : 2;
  uint32_t          m_CloseToMinimum    : 1;  // 1: the last pressed angle was closer to minimum end
  uint32_t          m_InOut             : 1;  //  1: The click was within the background arc angles. 0: Click outside 
  uint32_t          m_ChangeRate;             // Drag angle rate of change of the arc (degrees/sec)
  uint32_t          m_LastTick;               // Last dragging event timestamp of the arc
  int16_t           m_LastAngle;              // Last dragging angle of the arc

private:
  void              Draw(EGEvent *pEvent);
  void              InvalidateArcArea(uint16_t StartAngle, uint16_t EndAngle, EGPart_t Part);
  void              InvalidateKnobArea();
  void              GetCenter(EGPoint *center, EG_Coord_t *arc_r);
  EG_Coord_t        GetAngle();
  void              GetKnobArea(const EGPoint *pCenter, EG_Coord_t Radius, EGRect *pKnobRect);
  void              ValueUpdate(void);
  EG_Coord_t        KnobGetExtraSize(void);
  bool              IsAngleWithinBackgroundBounds(const uint32_t angle, const uint32_t tolerance_deg);

};

#endif 