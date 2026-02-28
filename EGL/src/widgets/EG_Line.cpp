/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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

#include "widgets/EG_Line.h"

#if EG_USE_LINE != 0
#include "misc/EG_Assert.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Math.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_LineClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGLine::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGLine::EGLine(void) : EGObject(),
	m_pPointArray(nullptr),
  m_PointCount(0),
	m_InvertY(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGLine::EGLine(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_LineClass*/) : EGObject(),
	m_pPointArray(nullptr),
  m_PointCount(0),
	m_InvertY(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGLine::~EGLine(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLine::Configure(void)
{
  EGObject::Configure();
	m_PointCount = 0;
	m_pPointArray = nullptr;
	m_InvertY = 0;
	ClearFlag(EG_OBJ_FLAG_CLICKABLE);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLine::SetPoints(const EGPoint Points[], uint16_t PointCount)
{
	m_pPointArray = Points;
	m_PointCount = PointCount;
	RefreshSelfSize();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLine::SetInvertY(bool Enable)
{
	if(m_InvertY == Enable) return;
	m_InvertY = Enable ? 1U : 0U;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGLine::GetInvertY(void)
{
	return m_InvertY == 1U;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLine::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(&c_LineClass) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGLine *pLine = (EGLine*)pEvent->GetTarget();
  pLine->Event(pEvent); // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLine::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		EG_Coord_t line_width = GetStyleLineWidth(EG_PART_MAIN);	// The corner of the skew lines is out of the intended Rect
		EG_Coord_t *s = (EG_Coord_t*)pEvent->GetParam();
		if(*s < line_width) *s = line_width;
	}
	else if(Code == EG_EVENT_GET_SELF_SIZE) {
		if(m_PointCount == 0 || m_pPointArray == nullptr) return;
		EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
		EG_Coord_t Width = 0;
		EG_Coord_t Height = 0;
		for(uint16_t i = 0; i < m_PointCount; i++) {
			Width = EG_MAX(m_pPointArray[i].m_X, Width);
			Height = EG_MAX(m_pPointArray[i].m_Y, Height);
		}
		EG_Coord_t line_width = GetStyleLineWidth(EG_PART_MAIN);
		Width += line_width;
		Height += line_width;
		pPoint->m_X = Width;
		pPoint->m_Y = Height;
	}
	else if(Code == EG_EVENT_DRAW_MAIN) {
    EGDrawContext *pContext = pEvent->GetDrawContext();
		if(m_PointCount == 0 || m_pPointArray == nullptr) return;
		EGRect Rect(m_Rect);
		EG_Coord_t OffsetX = Rect.GetX1() - GetScrollX();
		EG_Coord_t OffsetY = Rect.GetY1() - GetScrollY();
		EG_Coord_t Height = GetHeight();
		EGDrawLine DrawLine;
		InititialseDrawLine(EG_PART_MAIN, &DrawLine);
		for(uint16_t i = 0; i < m_PointCount - 1; i++) {	// Read all points and draw the lines
    	EGPoint Point1, Point2;
			Point1.m_X = m_pPointArray[i].m_X + OffsetX;
			Point2.m_X = m_pPointArray[i + 1].m_X + OffsetX;
			if(m_InvertY == 0) {
				Point1.m_Y = m_pPointArray[i].m_Y + OffsetY;
				Point2.m_Y = m_pPointArray[i + 1].m_Y + OffsetY;
			}
			else {
				Point1.m_Y = Height - m_pPointArray[i].m_Y + OffsetY;
				Point2.m_Y = Height - m_pPointArray[i + 1].m_Y + OffsetY;
			}
			DrawLine.Draw(pContext, &Point1, &Point2);
			DrawLine.m_RoundStart = 0; // Draw the rounding only on the end points after the first pLine
		}
	}
}

#endif
