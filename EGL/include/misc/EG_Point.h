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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include <stdbool.h>
#include <stdint.h>
#include "EG_Types.h"

////////////////////////////////////////////////////////////////////////////////

class EGPoint
{
public:
                      EGPoint();
	                    EGPoint(const EGPoint &InPoint);
	                    EGPoint(const EGPoint *pInPoint);
	                    EGPoint(EG_Coord_t X, EG_Coord_t Y);
	void                Set(EG_Coord_t X, EG_Coord_t Y);
  void                operator = (const EGPoint &rval);
	void                operator += (const EGPoint rval);
	void                operator -= (const EGPoint rval);
	void                operator++ (void);
	void                operator-- (void);
  void                Offset(EG_Coord_t X, EG_Coord_t Y);
  void                Offset(const EGPoint *pPoint);
  void                PointTransform(int32_t Angle, int32_t Zoom, const EGPoint *pPivot);

	EG_Coord_t          m_X;
	EG_Coord_t          m_Y;
	static int32_t      m_SIN;
	static int32_t      m_COS;
  static int32_t      m_PreviousAngle;

};