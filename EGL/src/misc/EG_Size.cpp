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

#include "misc/EG_Size.h"

//////////////////////////////////////////////////////////////////////////////////
// EGSize //
////////////////////////////////////////////////////////////////////////////////

EGSize::EGSize() :
  m_X(0),
	m_Y(0)
{
}

//////////////////////////////////////////////////////////////////////////////////

EGSize::EGSize(const EGSize &InSize)
{
	m_X = InSize.m_X;
	m_Y = InSize.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

EGSize::EGSize(const EGSize *pInSize)
{
	m_X = pInSize->m_X;
	m_Y = pInSize->m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

EGSize::EGSize(int32_t X, int32_t Y)
{
  m_X = X;
	m_Y = Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::Set(int32_t X, int32_t Y)
{
  m_X = X;
	m_Y = Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::operator = (const EGSize &rval)
{
  m_X = rval.m_X;
  m_Y = rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::operator += (const EGSize rval)
{
  m_X += rval.m_X;
  m_Y += rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::operator -= (const EGSize rval)
{
  m_X -= rval.m_X;
  m_Y -= rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::operator *= (const EGSize rval)
{
  m_X *= rval.m_X;
  m_Y *= rval.m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::operator++ (void)
{
  m_X += 1;
  m_Y += 1;
}

//////////////////////////////////////////////////////////////////////////////////

void EGSize::operator-- (void)
{
  m_X -= 1;
  m_Y -= 1;
}

//////////////////////////////////////////////////////////////////////////////////

bool EGSize::operator == (const EGSize &rval) const
{
  return ((m_X == rval.m_X) && (m_Y == rval.m_Y)) ? true : false;  
}

//////////////////////////////////////////////////////////////////////////////////
// EGScale //
//////////////////////////////////////////////////////////////////////////////////

EGScale::EGScale(void) : EGSize()
{
  m_X = EG_SCALE_NONE;
	m_Y = EG_SCALE_NONE;
}

//////////////////////////////////////////////////////////////////////////////////

bool EGScale::IsScaled(void) const
{
  return ((m_X != EG_SCALE_NONE) || (m_Y != EG_SCALE_NONE)) ? true : false;  
}

//////////////////////////////////////////////////////////////////////////////////

void EGScale::Normalise(void)
{
  if(m_X == 0) m_X = 1;
  if(m_Y == 0) m_Y = 1;  
}

//////////////////////////////////////////////////////////////////////////////////

void EGScale::Negate(void)
{
  m_X = (256 * 256) / m_X;
  m_Y = (256 * 256) / m_Y;
}

//////////////////////////////////////////////////////////////////////////////////

int32_t EGScale::Maximum(void)
{
  return ((m_X > m_Y)) ? m_X : m_Y;  
}

//////////////////////////////////////////////////////////////////////////////////

int32_t EGScale::Minimum(void)
{
  return ((m_X <= m_Y)) ? m_X : m_Y;  
}


