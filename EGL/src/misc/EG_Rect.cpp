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

#include "misc/EG_Point.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Math.h"

////////////////////////////////////////////////////////////////////////////////

EGRect::EGRect(void) :
  m_X1(0),
	m_Y1(0),
	m_X2(0),
	m_Y2(0)
{
}

//////////////////////////////////////////////////////////////////////////////////

EGRect::EGRect(const EGRect &InRect)
{
	m_X1 = InRect.m_X1;
	m_Y1 = InRect.m_Y1;
	m_X2 = InRect.m_X2;
	m_Y2 = InRect.m_Y2;
}

//////////////////////////////////////////////////////////////////////////////////

EGRect::EGRect(const EGRect *pInRect)
{
	m_X1 = pInRect->m_X1;
	m_Y1 = pInRect->m_Y1;
	m_X2 = pInRect->m_X2;
	m_Y2 = pInRect->m_Y2;
}

//////////////////////////////////////////////////////////////////////////////////

EGRect::EGRect(EG_Coord_t X1, EG_Coord_t Y1, EG_Coord_t X2, EG_Coord_t Y2)
{
  m_X1 = X1;
	m_Y1 = Y1;
	m_X2 = X2;
	m_Y2 = Y2;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::operator=(const EGRect &rval)
{
  m_X1 = rval.m_X1;
  m_X2 = rval.m_X2;
  m_Y1 = rval.m_Y1;
  m_Y2 = rval.m_Y2;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::operator += (const EGRect rval)
{
  m_X1 += rval.m_X1;
  m_X2 += rval.m_X2;
  m_Y1 += rval.m_Y1;
  m_Y2 += rval.m_Y2;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::operator -= (const EGRect rval)
{
  m_X1 -= rval.m_X1;
  m_X2 -= rval.m_X2;
  m_Y1 -= rval.m_Y1;
  m_Y2 -= rval.m_Y2;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::operator++ (void)
{
  m_X1 += 1;
  m_X2 += 1;
  m_Y1 += 1;
  m_Y2 += 1;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::operator-- (void)
{
  m_X1 -= 1;
  m_X2 -= 1;
  m_Y1 -= 1;
  m_Y2 -= 1;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Set(EG_Coord_t X1, EG_Coord_t Y1, EG_Coord_t X2, EG_Coord_t Y2)
{
	m_X1 = X1;
	m_Y1 = Y1;
	m_X2 = X2;
	m_Y2 = Y2;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::SetWidth(EG_Coord_t Width)
{
	m_X2 = m_X1 + Width - 1;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::SetHeight(EG_Coord_t Height)
{
	m_Y2 = m_Y1 + Height - 1;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::SetPosition(EG_Coord_t PosX, EG_Coord_t PosY)
{
	EG_Coord_t Width = GetWidth();
	EG_Coord_t Height = GetHeight();
	m_X1 = PosX;
	m_Y1 = PosY;
	SetWidth(Width);
	SetHeight(Height);
}

////////////////////////////////////////////////////////////////////////////////

uint32_t EGRect::GetSize() const
{
	return (uint32_t)(m_X2 - m_X1 + 1) * (m_Y2 - m_Y1 + 1);
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Inflate(EG_Coord_t Width, EG_Coord_t Height)
{
	m_X1 -= Width;
	m_X2 += Width;
	m_Y1 -= Height;
	m_Y2 += Height;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::Inflate(EG_Coord_t Left, EG_Coord_t Right, EG_Coord_t Top, EG_Coord_t Bottom)
{
	m_X1 -= Left;
	m_X2 += Right;
	m_Y1 -= Top;
	m_Y2 += Bottom;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Deflate(EG_Coord_t Width, EG_Coord_t Height)
{
	m_X1 += Width;
	m_X2 -= Width;
	m_Y1 += Height;
	m_Y2 -= Height;
}

//////////////////////////////////////////////////////////////////////////////////

void EGRect::Deflate(EG_Coord_t Left, EG_Coord_t Right, EG_Coord_t Top, EG_Coord_t Bottom)
{
	m_X1 += Left;
	m_X2 -= Right;
	m_Y1 += Top;
	m_Y2 -= Bottom;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Move(EG_Coord_t OffsetX, EG_Coord_t OffsetY)
{
	m_X1 += OffsetX;
	m_X2 += OffsetX;
	m_Y1 += OffsetY;
	m_Y2 += OffsetY;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Normalise(void)
{
	if(m_X1 > m_X2){
    EG_Coord_t Temp = m_X1;
    m_X1 = m_X2;
    m_X2 = Temp;
  } 
	if(m_Y1  > m_Y2){
    EG_Coord_t Temp = m_Y1;
    m_Y1 = m_Y2;
    m_Y2 = Temp;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::Intersect(const EGRect *pRect)
{
	// Get the smaller area from 'this' and 'pRect'
	m_X1 = EG_MAX(m_X1, pRect->m_X1);
	m_Y1 = EG_MAX(m_Y1, pRect->m_Y1);
	m_X2 = EG_MIN(m_X2, pRect->m_X2);
	m_Y2 = EG_MIN(m_Y2, pRect->m_Y2);
	// If m_X1 or m_Y1 greater than m_X2 or m_Y2 then the areas union is empty
	if((m_X1 > m_X2) || (m_Y1 > m_Y2)) return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::Intersect(const EGRect *pRectA, const EGRect *pRectB)
{
	// Get the smaller area from 'this' and 'pRect'
	m_X1 = EG_MAX(pRectA->m_X1, pRectB->m_X1);
	m_Y1 = EG_MAX(pRectA->m_Y1, pRectB->m_Y1);
	m_X2 = EG_MIN(pRectA->m_X2, pRectB->m_X2);
	m_Y2 = EG_MIN(pRectA->m_Y2, pRectB->m_Y2);
	// If m_X1 or m_Y1 greater than m_X2 or m_Y2 then the areas union is empty
	if((m_X1 > m_X2) || (m_Y1 > m_Y2)) return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Join(EGRect *pJoined, const EGRect *pRect)
{
	pJoined->m_X1 = EG_MIN(m_X1, pRect->m_X1);
	pJoined->m_Y1 = EG_MIN(m_Y1, pRect->m_Y1);
	pJoined->m_X2 = EG_MAX(m_X2, pRect->m_X2);
	pJoined->m_Y2 = EG_MAX(m_Y2, pRect->m_Y2);
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::IsPointIn(const EGPoint *pPoint, EG_Coord_t Radius) const
{
bool IsInRect = false;	

	if((pPoint->m_X >= m_X1 && pPoint->m_X <= m_X2) && ((pPoint->m_Y >= m_Y1 && pPoint->m_Y <= m_Y2))) { // First check the basic area
		IsInRect = true;
	}
	if(!IsInRect)	return false;
	if(Radius <= 0) return true;	// Now handle potential rounded rectangles
	EG_Coord_t Width = GetWidth() / 2;
	EG_Coord_t Height = GetHeight() / 2;
	EG_Coord_t MaxRadius = EG_MIN(Width, Height);
	if(Radius > MaxRadius)	Radius = MaxRadius;
	EGRect CornerArea;	// Check if it's in one of the corners
	CornerArea.m_X1 = m_X1;	// Top left
	CornerArea.m_X2 = m_X1 + Radius;
	CornerArea.m_Y1 = m_Y1;
	CornerArea.m_Y2 = m_Y1 + Radius;
	if(CornerArea.IsPointIn(pPoint, 0)) {
		CornerArea.m_X2 += Radius;
		CornerArea.m_Y2 += Radius;
		return CornerArea.PointWithinCircle(pPoint);
	}
	CornerArea.m_Y1 = m_Y2 - Radius;	// Bottom left
	CornerArea.m_Y2 = m_Y2;
	if(CornerArea.IsPointIn(pPoint, 0)) {
		CornerArea.m_X2 += Radius;
		CornerArea.m_Y1 -= Radius;
		return CornerArea.PointWithinCircle(pPoint);
	}
	CornerArea.m_X1 = m_X2 - Radius;	// Bottom right
	CornerArea.m_X2 = m_X2;
	if(CornerArea.IsPointIn(pPoint, 0)) {
		CornerArea.m_X1 -= Radius;
		CornerArea.m_Y1 -= Radius;
		return CornerArea.PointWithinCircle(pPoint);
	}
	CornerArea.m_Y1 = m_Y1;	// Top right
	CornerArea.m_Y2 = m_Y1 + Radius;
	if(CornerArea.IsPointIn(pPoint, 0)) {
		CornerArea.m_X1 -= Radius;
		CornerArea.m_Y2 += Radius;
		return CornerArea.PointWithinCircle(pPoint);
	}
	return true;	//Not within corners
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::IsOn(const EGRect *pRect) const
{
	if((m_X1 <= pRect->m_X2) && (m_X2 >= pRect->m_X1) && (m_Y1 <= pRect->m_Y2) && (m_Y2 >= pRect->m_Y1)) return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::IsInside(const EGRect *pRect, EG_Coord_t Radius) const
{
EGPoint pPoint;	
bool IsIn = false;

	if(m_X1 >= pRect->m_X1 && m_Y1 >= pRect->m_Y1 && m_X2 <= pRect->m_X2 && m_Y2 <= pRect->m_Y2) IsIn = true;
  if(!IsIn) return false;
	if(Radius == 0) return true;
	pPoint.m_X = m_X1;
	pPoint.m_Y = m_Y1;
	if(IsPointIn(&pPoint, Radius) == false) return false;
	pPoint.m_X = m_X2;
	pPoint.m_Y = m_Y1;
	if(IsPointIn(&pPoint, Radius) == false) return false;
	pPoint.m_X = m_X1;
	pPoint.m_Y = m_Y2;
	if(IsPointIn(&pPoint, Radius) == false) return false;
	pPoint.m_X = m_X2;
	pPoint.m_Y = m_Y2;
	if(IsPointIn(&pPoint, Radius) == false) return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::IsOutside(const EGRect *pRect, EG_Coord_t Radius) const
{
EGPoint pPoint;

	if(m_X2 < pRect->m_X1 || m_Y2 < pRect->m_Y1 || m_X1 > pRect->m_X2 || m_Y1 > pRect->m_Y2) return true;
	if(Radius == 0) return false;
	pPoint.m_X = m_X1;	// Check if the corner points are outside the Radius or not
	pPoint.m_Y = m_Y1;
	if(IsPointIn(&pPoint, Radius)) return false;
	pPoint.m_X = m_X2;
	pPoint.m_Y = m_Y1;
	if(IsPointIn(&pPoint, Radius)) return false;
	pPoint.m_X = m_X1;
	pPoint.m_Y = m_Y2;
	if(IsPointIn(&pPoint, Radius)) return false;
	pPoint.m_X = m_X2;
	pPoint.m_Y = m_Y2;
	if(IsPointIn(&pPoint, Radius)) return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::IsEqualTo(const EGRect *a, const EGRect *pRect)
{
	return m_X1 == pRect->m_X1 && m_X2 == pRect->m_X2 && m_Y1 == pRect->m_Y1 && m_Y2 == pRect->m_Y2;
}

////////////////////////////////////////////////////////////////////////////////

void EGRect::Align(EGRect *pRectToAlign, EG_AlignType_e AlignType, EG_Coord_t OffsetX, EG_Coord_t OffsetY)
{
EG_Coord_t x, y;

	switch(AlignType) {
		case EG_ALIGN_CENTER:
			x = GetWidth() / 2 - pRectToAlign->GetWidth() / 2;
			y = GetHeight() / 2 - pRectToAlign->GetHeight() / 2;
			break;

		case EG_ALIGN_TOP_LEFT:
			x = 0;
			y = 0;
			break;
		case EG_ALIGN_TOP_MID:
			x = GetWidth() / 2 - pRectToAlign->GetWidth() / 2;
			y = 0;
			break;

		case EG_ALIGN_TOP_RIGHT:
			x = GetWidth() - pRectToAlign->GetWidth();
			y = 0;
			break;

		case EG_ALIGN_BOTTOM_LEFT:
			x = 0;
			y = GetHeight() - pRectToAlign->GetHeight();
			break;
		case EG_ALIGN_BOTTOM_MID:
			x = GetWidth() / 2 - pRectToAlign->GetWidth() / 2;
			y = GetHeight() - pRectToAlign->GetHeight();
			break;

		case EG_ALIGN_BOTTOM_RIGHT:
			x = GetWidth() - pRectToAlign->GetWidth();
			y = GetHeight() - pRectToAlign->GetHeight();
			break;

		case EG_ALIGN_LEFT_MID:
			x = 0;
			y = GetHeight() / 2 - pRectToAlign->GetHeight() / 2;
			break;

		case EG_ALIGN_RIGHT_MID:
			x = GetWidth() - pRectToAlign->GetWidth();
			y = GetHeight() / 2 - pRectToAlign->GetHeight() / 2;
			break;

		case EG_ALIGN_OUT_TOP_LEFT:
			x = 0;
			y = -pRectToAlign->GetHeight();
			break;

		case EG_ALIGN_OUT_TOP_MID:
			x = GetWidth() / 2 - pRectToAlign->GetWidth() / 2;
			y = -pRectToAlign->GetHeight();
			break;

		case EG_ALIGN_OUT_TOP_RIGHT:
			x = GetWidth() - pRectToAlign->GetWidth();
			y = -pRectToAlign->GetHeight();
			break;

		case EG_ALIGN_OUT_BOTTOM_LEFT:
			x = 0;
			y = GetHeight();
			break;

		case EG_ALIGN_OUT_BOTTOM_MID:
			x = GetWidth() / 2 - pRectToAlign->GetWidth() / 2;
			y = GetHeight();
			break;

		case EG_ALIGN_OUT_BOTTOM_RIGHT:
			x = GetWidth() - pRectToAlign->GetWidth();
			y = GetHeight();
			break;

		case EG_ALIGN_OUT_LEFT_TOP:
			x = -pRectToAlign->GetWidth();
			y = 0;
			break;

		case EG_ALIGN_OUT_LEFT_MID:
			x = -pRectToAlign->GetWidth();
			y = GetHeight() / 2 - pRectToAlign->GetHeight() / 2;
			break;

		case EG_ALIGN_OUT_LEFT_BOTTOM:
			x = -pRectToAlign->GetWidth();
			y = GetHeight() - pRectToAlign->GetHeight();
			break;

		case EG_ALIGN_OUT_RIGHT_TOP:
			x = GetWidth();
			y = 0;
			break;

		case EG_ALIGN_OUT_RIGHT_MID:
			x = GetWidth();
			y = GetHeight() / 2 - pRectToAlign->GetHeight() / 2;
			break;

		case EG_ALIGN_OUT_RIGHT_BOTTOM:
			x = GetWidth();
			y = GetHeight() - pRectToAlign->GetHeight();
			break;
		default:
			x = 0;
			y = 0;
			break;
	}
	x += m_X1;
	y += m_Y1;
	EG_Coord_t Width = pRectToAlign->GetWidth();
	EG_Coord_t Height = pRectToAlign->GetHeight();
	pRectToAlign->m_X1 = x + OffsetX;
	pRectToAlign->m_Y1 = y + OffsetY;
	pRectToAlign->m_X2 = pRectToAlign->m_X1 + Width - 1;
	pRectToAlign->m_Y2 = pRectToAlign->m_Y1 + Height - 1;
}

////////////////////////////////////////////////////////////////////////////////

/** Get resulting sub areas after removing the common parts of two areas from the first area
 * @param pResult pointer to an array of areas with a count of 4, the resulting areas will be stored here
 * @param pRect pointer to the area to compare
 * @return number of results or -1 if no intersect */
int8_t EGRect::Difference(EGRect *pResult, const EGRect *pRect)
{
int8_t Count = 0;	// Result counter

	if(!IsOn(pRect)) return -1;	// Areas have no common parts
	if(IsInside(pRect, 0)) return 0;	// No remaining areas after removing common parts
	EG_Coord_t Width = GetWidth() - 1; // Get required information
	EG_Coord_t Height = GetHeight() - 1;
	EG_Coord_t Rect = pRect->m_Y1 - m_Y1;	// Compute top rectangle
	if(Rect > 0) {
		pResult[Count].m_X1 = m_X1;
		pResult[Count].m_Y1 = m_Y1;
		pResult[Count].m_X2 = m_X2;
		pResult[Count++].m_Y2 = m_Y1 + Rect;
	}
	Rect = Height - (pRect->m_Y2 - m_Y1);	// Compute the bottom rectangle
	if(Rect > 0 && pRect->m_Y2 < m_Y2) {
		pResult[Count].m_X1 = m_X1;
		pResult[Count].m_Y1 = pRect->m_Y2;
		pResult[Count].m_X2 = m_X2;
		pResult[Count++].m_Y2 = pRect->m_Y2 + Rect;
	}
	EG_Coord_t Y1 = pRect->m_Y1 > m_Y1 ? pRect->m_Y1 : m_Y1;	// Compute side height
	EG_Coord_t Y2 = pRect->m_Y2 < m_Y2 ? pRect->m_Y2 : m_Y2;
	EG_Coord_t Side = Y2 - Y1;
	Rect = pRect->m_X1 - m_X1;	// Compute the left rectangle
	if(Rect > 0 && Side > 0) {
		pResult[Count].m_X1 = m_X1;
		pResult[Count].m_Y1 = Y1;
		pResult[Count].m_X2 = m_X1 + Rect;
		pResult[Count++].m_Y2 = Y1 + Side;
	}
	Rect = Width - (pRect->m_X2 - m_X1);	// Compute the right rectangle
	if(Rect > 0) {
		pResult[Count].m_X1 = pRect->m_X2;
		pResult[Count].m_Y1 = Y1;
		pResult[Count].m_X2 = pRect->m_X2 + Rect;
		pResult[Count++].m_Y2 = Y1 + Side;
	}
	return Count;	// Return number of results
}

////////////////////////////////////////////////////////////////////////////////

bool EGRect::PointWithinCircle(const EGPoint *pPoint)
{
	EG_Coord_t Radius = (m_X2 - m_X1) / 2;
	EG_Coord_t cx = m_X1 + Radius;	// Circle center
	EG_Coord_t cy = m_Y1 + Radius;
	EG_Coord_t px = pPoint->m_X - cx;	// Simplify the code by moving everything to (0, 0)
	EG_Coord_t py = pPoint->m_Y - cy;
	uint32_t RadiusSqrd = Radius * Radius;
	uint32_t Distance = (px * px) + (py * py);
	if(Distance <= RadiusSqrd) return true;
	return false;
}

