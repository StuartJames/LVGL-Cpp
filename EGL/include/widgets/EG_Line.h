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

#if EG_USE_LINE != 0

#include "../core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_LineClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGLine : public EGObject
{
public:
                    EGLine(void);
                    EGLine(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_LineClass);
                    ~EGLine();
  virtual void      Configure(void);
  void              SetPoints(const EGPoint Points[], uint16_t PointCount);
  void              SetInvertY(bool Enable);
  bool              GetInvertY(void);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);


  const EGPoint    *m_pPointArray;        // Pointer to an array with the points of the line
  uint16_t          m_PointCount;         // Number of points in 'point_array'
  uint8_t           m_InvertY : 1;        // 1: y == 0 will be on the bottom
};




#endif