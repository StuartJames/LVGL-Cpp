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
#include "../core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_PolygonClass;

///////////////////////////////////////////////////////////////////////////////////////

// `type` field in `EG_DrawPartDescriptor_t` if `class_p = EGPolygon class`
// Used in `EG_EVENT_DRAW_PART_BEGIN` and `EG_EVENT_DRAW_PART_END`
enum EG_PolyDrawPartType_e{
  EG_POLY_DRAW_PART_FOREGROUND,
} ;

///////////////////////////////////////////////////////////////////////////////////////

class EGPolygon : public EGObject
{
public:
                    EGPolygon(void);
                    EGPolygon(EGObject *pParent, uint32_t PointCount, const EG_ClassType_t *pClassCnfg = &c_PolygonClass);
                    ~EGPolygon(void);
  virtual void      Configure(void);
  void              SetRotation(uint16_t rotation);
  void              SetVertices(EGPoint *pVertices, uint32_t VerticesCount = 3);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

private:
  void              CalculateVertices(bool Reset = true);
  bool              m_Recalculate;
  bool              m_ExtVertices;
  uint16_t          m_Rotation;
  EGPoint           *m_pVertices;
  uint32_t          m_VerticesCount;
};


