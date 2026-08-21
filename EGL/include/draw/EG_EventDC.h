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

#include "EG_DeviceContext.h"
#include "core/EG_ObjClass.h"

/////////////////////////////////////////////////////////////////////////////


// Cover check results.
enum EG_CoverResult_e : uint8_t {
    EG_COVER_RES_COVER      = 0,
    EG_COVER_RES_NOT_COVER  = 1,
    EG_COVER_RES_MASKED     = 2,
};

enum EG_LayerType_e : uint8_t {
    EG_LAYER_TYPE_NONE,
    EG_LAYER_TYPE_SIMPLE,
    EG_LAYER_TYPE_TRANSFORM,
};

/////////////////////////////////////////////////////////////////////////////

class EGEventDC
{
public:
                        EGEventDC();
                        EGEventDC(EGDeviceContext *pDC, const EG_ClassType_t *pClass = nullptr, uint32_t Type = 0, uint32_t Part = 0);

  EGDeviceContext         *m_pContext;             // Draw context
  const EG_ClassType_t    *m_pClass;               // The class that sent the event
  uint32_t                 m_Type;                 // The type of part being draw. Element of `EG_<NAME>_DRAW_PART_TYPE_t`
  EGRect                  *m_pRect;                // The area of the part being drawn
  EGDrawRect              *m_pDrawRect;            // A draw class that can be modified to changed what will be drawn. Set only for rectangle-like parts
  EGDrawLabel             *m_pDrawLabel;           // A draw class that can be modified to changed what will be drawn. Set only for text-like parts
  EGDrawLine              *m_pDrawLine;            // A draw class that can be modified to changed what will be drawn. Set only for line-like parts
  EGDrawImage             *m_pDrawImage;           // A draw class that can be modified to changed what will be drawn. Set only for image-like parts
  EGDrawArc               *m_pDrawArc;             // A draw class that can be modified to changed what will be drawn. Set only for arc-like parts
  EGDrawPolygon           *m_pDrawPoly;            // A draw class that can be modified to changed what will be drawn. Set only for polygon-like parts
  const EGPoint           *m_pPoint1;              // A point calculated during drawing. E.g. a point of chart or the center of an arc.
  const EGPoint           *m_pPoint2;              // A point calculated during drawing. E.g. a point of chart.
  char                    *m_pText;                // A text calculated during drawing. Can be modified. E.g. tick labels on a chart axis.
  uint32_t                 m_TextLength;           // Size of the text buffer containing null-terminated text string calculated during drawing.
  uint32_t                 m_Part;                 // The current part for which the event is sent
  uint32_t                 m_Index;                // The index of the part. E.g. a button's index on button matrix or table cell index.
  int32_t                  m_Radius;               // E.g. the radius of an arc (not the corner radius).
  int32_t                  m_Value;                // A value calculated during drawing. E.g. Chart's tick line value.
  const void              *m_pSubPart;             // A pointer the identifies something in the part. E.g. chart series.
};


