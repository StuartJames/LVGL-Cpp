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

#if EG_USE_CHART != 0

///////////////////////////////////////////////////////////////////////////////////////

// Default value of points. Can be used to not draw a point
#if EG_USE_LARGE_COORD
#define EG_CHART_POINT_NONE (INT32_MAX)
#else
#define EG_CHART_POINT_NONE (INT16_MAX)
#endif
EG_EXPORT_CONST_INT(EG_CHART_POINT_NONE);

///////////////////////////////////////////////////////////////////////////////////////

enum {
    EG_CHART_TYPE_NONE,     // Don't draw the series
    EG_CHART_TYPE_LINE,     // Connect the points with lines
    EG_CHART_TYPE_BAR,      // Draw columns
    EG_CHART_TYPE_SCATTER,  // Draw points and lines in 2D (x,y coordinates)
};
typedef uint8_t EG_ChartType_e;

//  Chart update mode for `lv_chart_set_next`
enum {
    EG_CHART_UPDATE_MODE_SHIFT,     // Shift old data to the left and add the new one the right
    EG_CHART_UPDATE_MODE_CIRCULAR,  // Add the new data in a circular way
};
typedef uint8_t EG_ChartMode_e;

//  Enumeration of the axis' 
enum {
    EG_CHART_AXIS_PRIMARY_Y     = 0x00,
    EG_CHART_AXIS_SECONDARY_Y   = 0x01,
    EG_CHART_AXIS_PRIMARY_X     = 0x02,
    EG_CHART_AXIS_SECONDARY_X   = 0x04,
    _EG_CHART_AXIS_LAST
};
typedef uint8_t EG_ChartAxis_e;

//  Used in `EG_EVENT_DRAW_PART_BEGIN` and `EG_EVENT_DRAW_PART_END` 
typedef enum {
    EG_CHART_DRAW_PART_DIV_LINE_INIT,    // Used before/after drawn the div lines
    EG_CHART_DRAW_PART_DIV_LINE_HOR,     // Used for each horizontal division lines
    EG_CHART_DRAW_PART_DIV_LINE_VER,     // Used for each vertical division lines
    EG_CHART_DRAW_PART_LINE_AND_POINT,   // Used on line and scatter charts for lines and points
    EG_CHART_DRAW_PART_BAR,              // Used on bar charts for the rectangles
    EG_CHART_DRAW_PART_CURSOR,           // Used on cursor lines and points
    EG_CHART_DRAW_PART_TICK_LABEL,       // Used on tick lines and labels
} EG_ChartDrawPartTypr_e;

///////////////////////////////////////////////////////////////////////////////////////

//  Descriptor a chart series
typedef struct EG_CharSeries_t{
    EG_CharSeries_t(void) : pPointsX(nullptr), pPointsY(nullptr), StartPoint(0), Hidden(0),
                            ExtBufferUsedX(0), ExtBufferUsedY(0), AxisSecX(0), AxisSecY(0){};
    EG_Coord_t *pPointsX;
    EG_Coord_t *pPointsY;
    EG_Color_t  Color;
    uint16_t    StartPoint;
    uint8_t     Hidden : 1;
    uint8_t     ExtBufferUsedX : 1;
    uint8_t     ExtBufferUsedY : 1;
    uint8_t     AxisSecX : 1;
    uint8_t     AxisSecY : 1;
} EG_CharSeries_t;

typedef struct EG_CharCursor_t{
    EG_CharCursor_t(void) : PointIndex(0), pSeries(nullptr), Direction(EG_DIR_NONE), PositionSet(0){};
    EGPoint       Position;
    EG_Coord_t    PointIndex;
    EG_Color_t    Color;
    EG_CharSeries_t *pSeries;
    EG_DirType_e  Direction;
    uint8_t       PositionSet: 1; // 1: Position is set; 0: PointIndex is set
} EG_CharCursor_t;

typedef struct EG_CharTickDSC_t{
    EG_CharTickDSC_t(void) : MajorLength(0), MinorLength(0), DrawSize(0), MinorCount(0), MajorCount(0), LabelEnable(0){};
    EG_Coord_t  MajorLength;
    EG_Coord_t  MinorLength;
    EG_Coord_t  DrawSize;
    uint32_t    MinorCount  : 15;
    uint32_t    MajorCount  : 15;
    uint32_t    LabelEnable : 1;
} EG_CharTickDSC_t;

extern const EG_ClassType_t c_ChartClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGChart : public EGObject
{
public:
                    EGChart(void);
                    EGChart(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ChartClass);
                    ~EGChart(void);
  virtual void      Configure(void);
  void              SetType(EG_ChartType_e Type);
  void              SetPointCount(uint16_t Count);
  void              SetRange(EG_ChartAxis_e Axis, EG_Coord_t Min, EG_Coord_t Max);
  void              SetUpdateMode(EG_ChartMode_e UpdateMode);
  void              SetDivLineCount(uint8_t HorizontalDiv, uint8_t VerticalDiv);
  void              SetZoomX(uint16_t ZoomX);
  void              SetZoomY(uint16_t ZoomY);
  uint16_t          GetZoomX(void);
  uint16_t          GetZoomY(void);
  void              SetAxisTick(EG_ChartAxis_e Axis, EG_Coord_t MajorLength, EG_Coord_t MinorLength,
                              EG_Coord_t MajorCount, EG_Coord_t MinorCount, bool LabelEnable, EG_Coord_t DrawSize);
  EG_ChartType_e    GetType(void);
  uint16_t          GetPointCount(void);
  uint16_t          GetStartPointX(EG_CharSeries_t *pSeries);
  void              GetPointPosByIndex(EG_CharSeries_t *pSeries, uint16_t Index, EGPoint *pPoint);
  void              Refresh(void);
  EG_CharSeries_t*  AddSeries(EG_Color_t Color, EG_ChartAxis_e Axis);
  void              RemoveSeries(EG_CharSeries_t *pSeries);
  void              HideSeries(EG_CharSeries_t *pSeries, bool Hide);
  void              SetSeriesColor(EG_CharSeries_t *pSeries, EG_Color_t Color);
  void              SetStartPointX(EG_CharSeries_t *pSeries, uint16_t Index);
  EG_CharSeries_t*  GetSeriesNext(const EG_CharSeries_t *pSeries);
  EG_CharCursor_t*  AddCursor(EG_Color_t Color, EG_DirType_e Direction);
  void              SetCursorPos(EG_CharCursor_t *pCursor, EGPoint *pPosition);
  void              SetCursorPoint(EG_CharCursor_t *pCursor, EG_CharSeries_t *pSeries, uint16_t PointIndex);
  EGPoint           GetCursorPoint(EG_CharCursor_t *pCursor);
  void              SetAllValue(EG_CharSeries_t *pSeries, EG_Coord_t Value);
  void              SetNextValue(EG_CharSeries_t *pSeries, EG_Coord_t Value);
  void              SetNextValues(EG_CharSeries_t *pSeries, EG_Coord_t ValueX, EG_Coord_t ValueY);
  void              SetValueByIndex(EG_CharSeries_t *pSeries, uint16_t Index, EG_Coord_t Value);
  void              SetValuesByIndex(EG_CharSeries_t *pSeries, uint16_t Index, EG_Coord_t ValueX, EG_Coord_t ValueY);
  void              SetExtArrayY(EG_CharSeries_t *pSeries, EG_Coord_t Array[]);
  void              SetExtArrayX(EG_CharSeries_t *pSeries, EG_Coord_t Array[]);
  EG_Coord_t*       GetArrayY(EG_CharSeries_t *pSeries);
  EG_Coord_t*       GetArrayX(EG_CharSeries_t *pSeries);
  uint32_t          GetSelectPoint(void);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  EGList            m_Series;     // Linked list for the series (stores EG_CharSeries_t)
  EGList            m_Cursor;     // Linked list for the cursors (stores EG_CharCursor_t)
  EG_CharTickDSC_t  m_Tick[4];
  EG_Coord_t        m_MinY[2];
  EG_Coord_t        m_MaxY[2];
  EG_Coord_t        m_MinX[2];
  EG_Coord_t        m_MaxX[2];
  EG_Coord_t        m_SelectPointIndex;
  uint16_t          m_HorizontalDivCount;       // Number of horizontal division lines
  uint16_t          m_VerticalDivCount;         // Number of vertical division lines
  uint16_t          m_PointCount;               // Point number in a data line
  uint16_t          m_ZoomX;
  uint16_t          m_ZoomY;
  EG_ChartType_e    m_ChartType  : 3; // Line or column chart
  EG_ChartMode_e    m_UpdateMode : 1;

private:
  void              DrawTickLines(EGDrawContext *pContext);
  void              DrawLineSeries(EGDrawContext *pContext);
  void              DrawScatterSeries(EGDrawContext *pContext);
  void              DrawBarSeries(EGDrawContext *pContext);
  void              DrawCursors(EGDrawContext *pContext);
  void              DrawAxisY(EGDrawContext *pContext, EG_ChartAxis_e Axis);
  void              DrawAxisX(EGDrawContext *pContext, EG_ChartAxis_e Axis);
  void              DrawAxis(EGDrawContext *pContext);
  uint32_t          GetIndexFromX(EG_Coord_t X);
  void              InvalidatePoint(uint16_t Index);
  void              AllocateNewPoints(EG_CharSeries_t *pSeries, uint32_t Count, EG_Coord_t **pPoints);
  EG_CharTickDSC_t* GetTickDSC(EG_ChartAxis_e Axis);

};

#endif 