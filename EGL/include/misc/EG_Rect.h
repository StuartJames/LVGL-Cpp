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
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include <stdbool.h>
#include <stdint.h>
#include "EG_Types.h"
#include "EG_Point.h"
#include "EG_Size.h"
#include "../misc/EG_Math.h"


////////////////////////////////////////////////////////////////////////////////

enum EG_AlignType_e{
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
};

enum EG_DirType_e{
	EG_DIR_NONE   = 0x00,
	EG_DIR_LEFT   = 0x01,
	EG_DIR_RIGHT  = 0x02,
	EG_DIR_TOP    = 0x04,
	EG_DIR_BOTTOM = 0x08,
	EG_DIR_HOR    = EG_DIR_LEFT | EG_DIR_RIGHT,
	EG_DIR_VER    = EG_DIR_TOP | EG_DIR_BOTTOM,
	EG_DIR_ALL    = EG_DIR_HOR | EG_DIR_VER,
};

typedef uint8_t EG_DirType_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGRect
{
public:
                      EGRect(void);
	                    EGRect(const EGRect &InRect);
	                    EGRect(const EGRect *pInRect);
	                    EGRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
  void                Zero(void);
	void                Set(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
	void                SetWidth(int32_t w);
	void                SetHeight(int32_t h);
	void                SetPosition(int32_t x, int32_t y);
	uint32_t            GetSize(void) const;
  int32_t             GetWidth(void) const;
  int32_t             GetHeight(void) const;
  int32_t             GetMinAxis(void) const;
  int32_t             GetMaxAxis(void) const;
  EGPoint             GetOffset(const EGRect *pRect) const;
  EGPoint             BottomLeft(void) const;
  EGPoint             TopRight(void) const;
  EGPoint             Center(void) const;
  void                Copy(EGRect *pDest) const;
	void                Inflate(const EGRect *pInRect);
	void                Inflate(int32_t Width, int32_t Height);
	void                Inflate(int32_t Left, int32_t Right, int32_t Top, int32_t Bottom);
	void                Deflate(int32_t Width, int32_t Height);
	void                Deflate(int32_t Left, int32_t Right, int32_t Top, int32_t Bottom);
	void                Move(int32_t OffsetX, int32_t OffsetY);
	void                Move(int32_t OffsetX, int32_t OffsetY, int32_t OffsetX2, int32_t OffsetY2);
  void                Normalise(void);
	bool                Intersect(const EGRect *pRect);
	bool                Intersect(const EGRect *pRectA, const EGRect *pRectB);
	void                Join(EGRect *pJoined, const EGRect *pRect);
	bool                IsPointIn(const EGPoint *pPoint, int32_t Radius) const;
	bool                IsOn(const EGRect *pRect) const;
	bool                IsInside(const EGRect *pRect, int32_t Radius) const;
	bool                IsOutside(const EGRect *pRect, int32_t Radius) const;
	bool                IsEqualTo(const EGRect *pRect);
	int8_t              Difference(EGRect *pResult, const EGRect *pRect);
	void                Align(EGRect *pRectToAlign, EG_AlignType_e AlignType, int32_t OffsetX, int32_t OffsetY) const;
  void                SetX1(int32_t x){ m_X1 = x; };
  void                SetY1(int32_t y){ m_Y1 = y; };
  void                SetX2(int32_t x){ m_X2 = x; };
  void                SetY2(int32_t y){ m_Y2 = y; };
  int32_t             GetX1() const { return m_X1;};
  int32_t             GetY1() const { return m_Y1;};
  int32_t             GetX2() const { return m_X2;};
  int32_t             GetY2() const { return m_Y2;};
  void                IncX1(int32_t n){ m_X1 += n;};
  void                IncY1(int32_t n){ m_Y1 += n;};
  void                IncX2(int32_t n){ m_X2 += n;};
  void                IncY2(int32_t n){ m_Y2 += n;};
  void                DecX1(int32_t n){ m_X1 -= n;};
  void                DecY1(int32_t n){ m_Y1 -= n;};
  void                DecX2(int32_t n){ m_X2 -= n;};
  void                DecY2(int32_t n){ m_Y2 -= n;};
  void                operator = (const EGRect &rval);
  bool                operator == (const EGRect &rval);
	void                operator += (const EGRect rval);
	void                operator -= (const EGRect rval);
	void                operator++ (void);
	void                operator-- (void);

private:
  bool                PointWithinCircle(const EGPoint *pPoint);

  int32_t             m_X1;
	int32_t             m_Y1;
	int32_t             m_X2;
	int32_t             m_Y2;
};

//////////////////////////////////////////////////////////////////////////////////

// Convert a percentage value to `int32_t`.
// Percentage values are stored in special range
inline int32_t EG_PCT(int32_t x)
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

inline int32_t EGRect::GetWidth(void) const
{
	return (m_X2 - m_X1 + 1);
}

//////////////////////////////////////////////////////////////////////////////////

inline int32_t EGRect::GetHeight(void) const
{
	return (m_Y2 - m_Y1 + 1);
}

//////////////////////////////////////////////////////////////////////////////////

inline EGPoint EGRect::GetOffset(const EGRect *pRect) const
{
	return EGPoint(m_X1 - pRect->m_X1, m_Y1 - pRect->m_Y1);
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

////////////////////////////////////////////////////////////////////////////////

inline int32_t EGRect::GetMinAxis(void) const
{
  return EG_MIN(m_X2 - m_X1, m_Y2 - m_Y1);
}

////////////////////////////////////////////////////////////////////////////////

inline int32_t EGRect::GetMaxAxis(void) const
{
  return EG_MAX(m_X2 - m_X1, m_Y2 - m_Y1);
}



