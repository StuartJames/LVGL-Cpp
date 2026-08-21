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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include <stdbool.h>
#include <stdint.h>
#include "EG_Types.h"

////////////////////////////////////////////////////////////////////////////////

class EGScale;

////////////////////////////////////////////////////////////////////////////////

class EGPoint
{
public:
                      EGPoint();
	                    EGPoint(const EGPoint &InPoint);
	                    EGPoint(const EGPoint *pInPoint);
	                    EGPoint(int32_t X, int32_t Y);
	void                Set(int32_t X, int32_t Y);
  void                operator = (const EGPoint &rval);
	void                operator += (const EGPoint rval);
	void                operator -= (const EGPoint rval);
	void                operator++ (void);
	void                operator-- (void);
  EGPoint             Add(int32_t X, int32_t Y);
  EGPoint             Sub(int32_t X, int32_t Y);
  void                Offset(int32_t X, int32_t Y);
  void                Offset(const EGPoint *pPoint);
  void                Swap(EGPoint *pPoint);
  EGPoint             Difference(const EGPoint *pPoint);
  void                PointTransform(int32_t Angle, EGScale Scale, const EGPoint *pPivot, bool ZoomFirst =false);

	int32_t             m_X;
	int32_t             m_Y;
	static int32_t      m_SIN;
	static int32_t      m_COS;
  static int32_t      m_PreviousAngle;

};