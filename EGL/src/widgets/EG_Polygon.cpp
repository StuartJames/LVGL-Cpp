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

#include "widgets/EG_Polygon.h"
#include "misc/EG_Assert.h"
#include "misc/lv_txt_ap.h"
#include "core/EG_Group.h"
#include "draw/EG_DrawContext.h"
#include "math.h"

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_PolygonClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGPolygon::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGPolygon::EGPolygon(void) : EGObject(),
	m_Recalculate(true),
	m_ExtVertices(false),
	m_Rotation(0),
  m_pVertices(nullptr),
  m_VerticesCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGPolygon::EGPolygon(EGObject *pParent, uint32_t PointCount, const EG_ClassType_t *pClassCnfg /*= &c_PolygonClass*/) : EGObject(),
	m_Recalculate(true),
	m_ExtVertices(false),
	m_Rotation(0),
  m_pVertices(nullptr),
  m_VerticesCount(PointCount)
{
	m_VerticesCount = (m_VerticesCount < 3) ? 3 : m_VerticesCount;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGPolygon::~EGPolygon(void)
{
	if(m_pVertices != nullptr) delete[] m_pVertices;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPolygon::Configure(void)
{
  EGObject::Configure();
	SetScrollbarMode(EG_SCROLLBAR_MODE_OFF);
 	SetStyleRadius(0, EG_PART_MAIN);
	ClearFlag(EG_OBJ_FLAG_CLICKABLE);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPolygon::SetRotation(uint16_t rotation)
{
	m_Rotation = rotation;
	m_Recalculate = true;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPolygon::SetVertices(EGPoint *pVertices, uint32_t VerticesCount /*= 3*/)
{
	if(m_pVertices != nullptr) delete[] m_pVertices;
	if(pVertices != nullptr){
		m_pVertices = pVertices;
		m_ExtVertices = true;
	}
	else m_ExtVertices = false;
	m_VerticesCount = VerticesCount;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPolygon::CalculateVertices(bool Reset /*= true*/)
{
EGRect Rect;

  if(m_ExtVertices) return;
	if(m_pVertices != nullptr) delete[] m_pVertices;
	m_pVertices = new EGPoint[m_VerticesCount];
	GetContentArea(&Rect);
	EG_Coord_t Radius = Rect.GetMinAxis() / 2;
	EGPoint Center(Rect.BottomLeft());
	Center.Offset(Radius, Radius);
	double Segment = 2 * EG_PI / m_VerticesCount;
	for(long i = 0; i < m_VerticesCount; i++){
  	double Angle = (Segment * i) + EG_DEG2RAD(-90 + m_Rotation);
	  m_pVertices[i].Set(Center.m_X + Radius * cos(Angle), Center.m_Y + Radius * sin(Angle));
	}
	if(Reset) m_Recalculate = false;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPolygon::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(&c_PolygonClass) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGPolygon *pPolygon = (EGPolygon*)pEvent->GetTarget();
  pPolygon->Event(pEvent); // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPolygon::Event(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
		case EG_EVENT_REFR_EXT_DRAW_SIZE:{
      EG_Coord_t PolygonWidth = GetStyleBorderWidth(EG_PART_MAIN);
      EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();
      if(*pSize < PolygonWidth) *pSize = PolygonWidth;
			break;
		}
		case EG_EVENT_GET_SELF_SIZE:{
      EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
			CalculateVertices(false);
      EG_Coord_t Width = 0, Height = 0;
      if(m_VerticesCount > 0) {
				for(uint16_t i = 0; i < m_VerticesCount; i++) {
					Width = EG_MAX(m_pVertices[i].m_X, Width);
					Height = EG_MAX(m_pVertices[i].m_Y, Height);
				}
				EG_Coord_t PolygonWidth = GetStyleBorderWidth(EG_PART_MAIN);
				Width += PolygonWidth;
				Height += PolygonWidth;
				pPoint->m_X = Width;
				pPoint->m_Y = Height;
			}
			break;
		}
    case EG_EVENT_DRAW_MAIN:{
			if(m_Recalculate) CalculateVertices();
			EGDrawContext *pDrawContext = pEvent->GetDrawContext();
			EGDrawDiscriptor PartDrawDiscriptor;
			InitDrawDescriptor(&PartDrawDiscriptor, pDrawContext);
			EGDrawPolygon DrawPolygon;
			InititialseDrawPoly(EG_PART_MAIN, &DrawPolygon);
			PartDrawDiscriptor.m_Part = EG_PART_MAIN;
			PartDrawDiscriptor.m_pClass = &c_PolygonClass;
			PartDrawDiscriptor.m_Type = EG_POLY_DRAW_PART_FOREGROUND;
			PartDrawDiscriptor.m_pDrawPoly = &DrawPolygon;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
			DrawPolygon.Draw(pDrawContext, m_pVertices, m_VerticesCount);
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
      break;
    }
    default:{
      break;
    }
	}
}

