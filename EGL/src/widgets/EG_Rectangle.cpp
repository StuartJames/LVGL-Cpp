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

#include "widgets/EG_Rectangle.h"
#include "misc/EG_Assert.h"
#include "misc/lv_txt_ap.h"
#include "core/EG_Group.h"
#include "draw/EG_DrawContext.h"

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_RectangleClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGRectangle::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGRectangle::EGRectangle(void) : EGObject()
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGRectangle::EGRectangle(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_CheckboxClass*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGRectangle::~EGRectangle(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRectangle::Configure(void)
{
  EGObject::Configure();
	SetScrollbarMode(EG_SCROLLBAR_MODE_OFF);
 	SetStyleRadius(0, EG_PART_MAIN);
	ClearFlag(EG_OBJ_FLAG_CLICKABLE);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRectangle::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(&c_RectangleClass) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGRectangle *pRectangle = (EGRectangle*)pEvent->GetTarget();
  pRectangle->Event(pEvent); // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRectangle::Event(EGEvent *pEvent)
{
	EG_UNUSED(pEvent);
//  EG_EventCode_e Code = pEvent->GetCode();
}

