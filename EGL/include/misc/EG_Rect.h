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
#include "EG_Point.h"

////////////////////////////////////////////////////////////////////////////////

typedef enum EG_AlignType_e{
  EG_ALIGN_DEFAULT = 0,       // (00)
  EG_ALIGN_TOP_LEFT,          // (01)
  EG_ALIGN_TOP_MID,           // (02)
  EG_ALIGN_TOP_RIGHT,         // (03)
  EG_ALIGN_BOTTOM_LEFT,       // (04)
  EG_ALIGN_BOTTOM_MID,        // (05)
  EG_ALIGN_BOTTOM_RIGHT,      // (06)
  EG_ALIGN_LEFT_MID,          // (07)
  EG_ALIGN_RIGHT_MID,         // (08)
  EG_ALIGN_CENTER,            // (09)
  EG_ALIGN_OUT_TOP_LEFT,      // (10)
  EG_ALIGN_OUT_TOP_MID,       // (11)
  EG_ALIGN_OUT_TOP_RIGHT,     // (12)
  EG_ALIGN_OUT_BOTTOM_LEFT,   // (13)
  EG_ALIGN_OUT_BOTTOM_MID,    // (14)
  EG_ALIGN_OUT_BOTTOM_RIGHT,  // (15)
  EG_ALIGN_OUT_LEFT_TOP,      // (16)
  EG_ALIGN_OUT_LEFT_MID,      // (17)
  EG_ALIGN_OUT_LEFT_BOTTOM,   // (18)
  EG_ALIGN_OUT_RIGHT_TOP,     // (19)
  EG_ALIGN_OUT_RIGHT_MID,     // (20)
  EG_ALIGN_OUT_RIGHT_BOTTOM,  // (21)
} EG_AlignType_e;

typedef enum : uint8_t{
	EG_DIR_NONE   = 0x00,
	EG_DIR_LEFT   = (1 << 0),
	EG_DIR_RIGHT  = (1 << 1),
	EG_DIR_TOP    = (1 << 2),
	EG_DIR_BOTTOM = (1 << 3),
	EG_DIR_HOR    = EG_DIR_LEFT | EG_DIR_RIGHT,
	EG_DIR_VER    = EG_DIR_TOP | EG_DIR_BOTTOM,
	EG_DIR_ALL    = EG_DIR_HOR | EG_DIR_VER,
} EG_DirType_e;

typedef uint8_t EG_DirType_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGRect
{
public:
                      EGRect(void);
	                    EGRect(const EGRect &InRect);
	                    EGRect(const EGRect *pInRect);
	                    EGRect(EG_Coord_t x1, EG_Coord_t y1, EG_Coord_t x2, EG_Coord_t y2);
	void                Set(EG_Coord_t x1, EG_Coord_t y1, EG_Coord_t x2, EG_Coord_t y2);
	void                SetWidth(EG_Coord_t w);
	void                SetHeight(EG_Coord_t h);
	void                SetPosition(EG_Coord_t x, EG_Coord_t y);
	uint32_t            GetSize(void) const;
  EG_Coord_t          GetWidth(void) const; 
  EG_Coord_t          GetHeight(void) const;
  EGPoint             BottomLeft(void) const;
  EGPoint             TopRight(void) const;
  EGPoint             Center(void) const;
  void                Copy(EGRect *pDest) const;
	void                Move(EG_Coord_t OffsetX, EG_Coord_t OffsetY);
	void                Inflate(EG_Coord_t Width, EG_Coord_t Height);
	void                Inflate(EG_Coord_t Left, EG_Coord_t Right, EG_Coord_t Top, EG_Coord_t Bottom);
	void                Deflate(EG_Coord_t Width, EG_Coord_t Height);
	void                Deflate(EG_Coord_t Left, EG_Coord_t Right, EG_Coord_t Top, EG_Coord_t Bottom);
  void                Normalise(void);
	bool                Intersect(const EGRect *pRect);
	bool                Intersect(const EGRect *pRectA, const EGRect *pRectB);
	void                Join(EGRect *pJoined, const EGRect *pRect);
	bool                IsPointIn(const EGPoint *pPoint, EG_Coord_t Radius) const;
	bool                IsOn(const EGRect *pRect) const;
	bool                IsInside(const EGRect *pRect, EG_Coord_t Radius) const;
	bool                IsOutside(const EGRect *pRect, EG_Coord_t Radius) const;
	bool                IsEqualTo(const EGRect *a, const EGRect *b);
	int8_t              Difference(EGRect *pResult, const EGRect *pRect);
	void                Align(EGRect *pRectToAlign, EG_AlignType_e AlignType, EG_Coord_t OffsetX, EG_Coord_t OffsetY);
  void                SetX1(EG_Coord_t x){ m_X1 = x; };
  void                SetY1(EG_Coord_t y){ m_Y1 = y; };
  void                SetX2(EG_Coord_t x){ m_X2 = x; };
  void                SetY2(EG_Coord_t y){ m_Y2 = y; };
  EG_Coord_t          GetX1() const { return m_X1;};
  EG_Coord_t          GetY1() const { return m_Y1;};
  EG_Coord_t          GetX2() const { return m_X2;};
  EG_Coord_t          GetY2() const { return m_Y2;};
  void                IncX1(EG_Coord_t n){ m_X1 += n;};
  void                IncY1(EG_Coord_t n){ m_Y1 += n;};
  void                IncX2(EG_Coord_t n){ m_X2 += n;};
  void                IncY2(EG_Coord_t n){ m_Y2 += n;};
  void                DecX1(EG_Coord_t n){ m_X1 -= n;};
  void                DecY1(EG_Coord_t n){ m_Y1 -= n;};
  void                DecX2(EG_Coord_t n){ m_X2 -= n;};
  void                DecY2(EG_Coord_t n){ m_Y2 -= n;};
  void                operator = (const EGRect &rval);
	void                operator += (const EGRect rval);
	void                operator -= (const EGRect rval);
	void                operator++ (void);
	void                operator-- (void);

private:
  bool                PointWithinCircle(const EGPoint *pPoint);

  EG_Coord_t          m_X1;
	EG_Coord_t          m_Y1;
	EG_Coord_t          m_X2;
	EG_Coord_t          m_Y2;
};

//////////////////////////////////////////////////////////////////////////////////

// Convert a percentage value to `EG_Coord_t`.
// Percentage values are stored in special range
inline EG_Coord_t EG_PCT(EG_Coord_t x)
{
	return _EG_PCT(x);
}

//////////////////////////////////////////////////////////////////////////////////

inline void EGRect::Copy(EGRect *pRect) const
{
	pRect->m_X1 = m_X1;
	pRect->m_Y1 = m_Y1;
	pRect->m_X2 = m_X2;
	pRect->m_Y2 = m_Y2;
}

//////////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t EGRect::GetWidth(void) const
{
	return (m_X2 - m_X1 + 1);
}

//////////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t EGRect::GetHeight(void) const
{
	return (m_Y2 - m_Y1 + 1);
}

//////////////////////////////////////////////////////////////////////////////////

inline EGPoint EGRect::BottomLeft(void) const
{
	return EGPoint(m_X1, m_Y1);
}

//////////////////////////////////////////////////////////////////////////////////

inline EGPoint EGRect::TopRight(void) const
{
	return EGPoint(m_X2, m_Y2);
}
//////////////////////////////////////////////////////////////////////////////////

inline EGPoint EGRect::Center(void) const
{
	return EGPoint((m_X2 - m_X1) / 2, (m_Y2 - m_Y1) / 2);
}

