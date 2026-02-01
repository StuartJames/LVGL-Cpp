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

#if EG_USE_METER != 0

///////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX == 0
#error "lv_meter: Complex drawing is required. Enable it in EG_Config.h (EG_DRAW_COMPLEX 1)"
#endif

///////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    EG_METER_INDICATOR_TYPE_NEEDLE_IMG,
    EG_METER_INDICATOR_TYPE_NEEDLE_LINE,
    EG_METER_INDICATOR_TYPE_SCALE_LINES,
    EG_METER_INDICATOR_TYPE_ARC,
} EG_IndicatorType_e;

typedef enum {
    EG_METER_DRAW_PART_ARC,             // The arc indicator
    EG_METER_DRAW_PART_NEEDLE_LINE,     // The needle lines
    EG_METER_DRAW_PART_NEEDLE_IMG,      // The needle images
    EG_METER_DRAW_PART_TICK,            // The tick lines and labels
} lv_meter_draw_part_type_t;

///////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    EG_Color_t  TickColor;
    uint16_t    TickCount;
    uint16_t    TickLength;
    uint16_t    TickWidth;

    EG_Color_t  TickMajorColor;
    uint16_t    TickMajorNth;
    uint16_t    TickMajorLength;
    uint16_t    TickMajorWidth;

    int16_t     LabelGap;

    int32_t     Minimum;
    int32_t     Maximum;
    int16_t     RadiusMod;
    uint16_t    AngleRange;
    int16_t     Rotation;
} EG_MeterScale_t;

typedef struct {
  EG_MeterScale_t     *pScale;
  EG_IndicatorType_e  Type;
  EG_OPA_t            OPA;
  int32_t             StartValue;
  int32_t             EndValue;
  union {
    struct {
      const void      *pSource;
      EGPoint         Pivot;
    } NeedleImage;
    struct {
      uint16_t        Width;
      int16_t         RadiusMod;
      EG_Color_t      Color;
    } NeedleLine;
    struct {
      uint16_t        Width;
      const void      *Source;
      EG_Color_t      Color;
      int16_t         RadiusMod;
    } Arc;
    struct {
      int16_t         WidthMod;
      EG_Color_t      StartColor;
      EG_Color_t      EndColor;
      uint8_t         LocalGrad  : 1;
    } ScaleLines;
  } TypeData;
} EG_Indicator_t;

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_MeterClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGMeter : public EGObject
{
public:
                    EGMeter(void) : EGObject(){};
                    EGMeter(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_MeterClass);
                    ~EGMeter(void);
  virtual void      Configure(void);
  EG_MeterScale_t*  AddScale(void);
  void              SetScaleTicks(EG_MeterScale_t *pScale, uint16_t Count, uint16_t Width, uint16_t Length, EG_Color_t Color);
  void              SetScaleMajorTicks(EG_MeterScale_t *pScale, uint16_t Nth, uint16_t Width, uint16_t Length, EG_Color_t Color, int16_t LabelGap);
  void              SetScaleRange(EG_MeterScale_t *pScale, int32_t Minimum, int32_t Maximum, uint32_t AngleRange, uint32_t Rotation);
  EG_Indicator_t*   AddNeedleLine(EG_MeterScale_t *pScale, uint16_t Width, EG_Color_t Color, int16_t RadiusMod);
  EG_Indicator_t*   AddNeedleImage(EG_MeterScale_t *pScale, const void *pSource, EG_Coord_t PivotX, EG_Coord_t PivotY);
  EG_Indicator_t*   AddArc(EG_MeterScale_t *pScale, uint16_t Width, EG_Color_t Color, int16_t RadiusMod);
  EG_Indicator_t*   AddScaleLines(EG_MeterScale_t *pScale, EG_Color_t StartColor, EG_Color_t EndColor, bool Local, int16_t WidthMod);
  void              SetIndicatorValue(EG_Indicator_t *pIndicator, int32_t Value);
  void              SetIndicatorStartValue(EG_Indicator_t *pIndicator, int32_t Value);
  void              SetIndicatorEndValue(EG_Indicator_t *pIndicator, int32_t Value);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);


  EGList            m_ScaleList;
  EGList            m_IndicatorList;
private:
  void              DrawArcs(EGDrawContext *pContext, const EGRect *pScaleArea);
  void              DrawTicksLabels(EGDrawContext *pContext, const EGRect *pScaleArea);
  void              DrawNeedles(EGDrawContext *pContext, const EGRect *pScaleArea);
  void              InvArc(EG_Indicator_t *pIndicator, int32_t OldValue, int32_t NewValue);
  void              InvLine(EG_Indicator_t *pIndicator, int32_t Value);


};


#endif 
