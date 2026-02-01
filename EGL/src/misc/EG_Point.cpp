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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "misc/EG_Point.h"
#include "misc/EG_Math.h"

int32_t  EGPoint::m_SIN = 0;
int32_t  EGPoint::m_COS = 0;
int32_t  EGPoint::m_PreviousAngle = INT32_MIN;

////////////////////////////////////////////////////////////////////////////////

EGPoint::EGPoint() :
  m_X(0),
	m_Y(0)
{
}

//////////////////////////////////////////////////////////////////////////////////

EGPoint::EGPoint(const EGPoint &InPoint)
{
	m_X = InPoint.m_X;
	m_Y = InPoint.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

EGPoint::EGPoint(const EGPoint *pInPoint)
{
	m_X = pInPoint->m_X;
	m_Y = pInPoint->m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

EGPoint::EGPoint(EG_Coord_t X, EG_Coord_t Y)
{
  m_X = X;
	m_Y = Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::Set(EG_Coord_t X, EG_Coord_t Y)
{
  m_X = X;
	m_Y = Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::operator = (const EGPoint &rval)
{
  m_X = rval.m_X;
  m_Y = rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::operator += (const EGPoint rval)
{
  m_X += rval.m_X;
  m_Y += rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::operator -= (const EGPoint rval)
{
  m_X -= rval.m_X;
  m_Y -= rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::operator++ (void)
{
  m_X += 1;
  m_Y += 1;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::operator-- (void)
{
  m_X -= 1;
  m_Y -= 1;
}

//////////////////////////////////////////////////////////////////////////////////

void EGPoint::Offset(EG_Coord_t X, EG_Coord_t Y)
{
  m_X += X;
  m_Y += Y;
}
//////////////////////////////////////////////////////////////////////////////////

void EGPoint::Offset(const EGPoint *pPoint)
{
  m_X += pPoint->m_X;
  m_Y += pPoint->m_Y;
}

////////////////////////////////////////////////////////////////////////////////

void EGPoint::PointTransform(int32_t Angle, int32_t ScaleX, uint32_t ScaleY, const EGPoint *pPivot, bool ZoomFirst /*= false*/)
{
	if((Angle == 0) && (ScaleX == 256) && (ScaleY == 256)) return;
	m_X -= pPivot->m_X;
	m_Y -= pPivot->m_Y;
	if(Angle == 0) {
		m_X = (((int32_t)(m_X) * ScaleX) >> 8) + pPivot->m_X;
		m_Y = (((int32_t)(m_Y) * ScaleY) >> 8) + pPivot->m_Y;
		return;
	}
	if(m_PreviousAngle != Angle){
		int32_t AngleLimited = Angle;
		if(AngleLimited > 3600) AngleLimited -= 3600;
		if(AngleLimited < 0) AngleLimited += 3600;
		int32_t AngleLow = AngleLimited / 10;
		int32_t AngleHigh = AngleLow + 1;
		int32_t AngleRemain = AngleLimited - (AngleLow * 10);
		int32_t s1 = EG_TrigoSin(AngleLow);
		int32_t s2 = EG_TrigoSin(AngleHigh);
		int32_t c1 = EG_TrigoSin(AngleLow + 90);
		int32_t c2 = EG_TrigoSin(AngleHigh + 90);
		m_SIN = (s1 * (10 - AngleRemain) + s2 * AngleRemain) / 10;
		m_COS = (c1 * (10 - AngleRemain) + c2 * AngleRemain) / 10;
		m_SIN = m_SIN >> (EG_TRIGO_SHIFT - _EG_TRANSFORM_TRIGO_SHIFT);
		m_COS = m_COS >> (EG_TRIGO_SHIFT - _EG_TRANSFORM_TRIGO_SHIFT);
		m_PreviousAngle = Angle;
	}
	int32_t x = m_X;
	int32_t y = m_Y;
	if((ScaleX == 256) && (ScaleY == 256)) {
		m_X = ((m_COS * x - m_SIN * y) >> _EG_TRANSFORM_TRIGO_SHIFT) + pPivot->m_X;
		m_Y = ((m_SIN * x + m_COS * y) >> _EG_TRANSFORM_TRIGO_SHIFT) + pPivot->m_Y;
	}
	else{
    if(ZoomFirst){
      x *= ScaleX;
      y *= ScaleY;
      m_X = (((m_COS * x - m_SIN * y)) >> (_EG_TRANSFORM_TRIGO_SHIFT + 8)) + pPivot->m_X;
      m_Y = (((m_SIN * x + m_COS * y)) >> (_EG_TRANSFORM_TRIGO_SHIFT + 8)) + pPivot->m_Y;
    }
    else{
  		m_X = (((m_COS * x - m_SIN * y) * ScaleX) >> (_EG_TRANSFORM_TRIGO_SHIFT + 8)) + pPivot->m_X;
	  	m_Y = (((m_SIN * x + m_COS * y) * ScaleY) >> (_EG_TRANSFORM_TRIGO_SHIFT + 8)) + pPivot->m_Y;
    }
	}
}

