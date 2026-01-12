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

#include "core/EG_Object.h"
#include "core/EG_Theme.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::Attach(EGObject *pObject, EGObject *pParent, const EG_ClassType_t *pClassCnfg)
{
  pObject->m_pClass = pClassCnfg;
  pObject->SetDirectParent(pParent);
	if(pParent == nullptr) {	// Create a screen
//   	ESP_LOGI("[Object]", "Attach: Screen.");
		EG_TRACE_OBJ_CREATE("Creating a screen");
		EGDisplay *pDispay = EGDisplay::GetDefault();
		if(pDispay == nullptr){
			EG_LOG_WARN("No display created yet. No place to assign the new screen");
			return;
		}
		if(pDispay->m_pScreens == nullptr) {
			pDispay->m_pScreens = (EGObject**)EG_AllocMem(sizeof(EGObject*));
			pDispay->m_pScreens[0] = pObject;
			pDispay->m_ScreenCount = 1;
		}
		else {
			pDispay->m_ScreenCount++;
			pDispay->m_pScreens = (EGObject**)EG_ReallocMem(pDispay->m_pScreens, sizeof(EGObject*) * pDispay->m_ScreenCount);
			pDispay->m_pScreens[pDispay->m_ScreenCount - 1] = pObject;
		}
		pObject->m_Rect.Set(0, 0, pDispay->GetHorizontalRes() - 1, pDispay->GetVerticalRes() - 1);		// Set coordinates to full screen size
	}
	else {	// Create a normal object
//   	ESP_LOGI("[Object]", "Attach: Object.");
		EG_TRACE_OBJ_CREATE("Creating normal object %p", (void*)pObject);
		if(pParent->m_pAttributes == nullptr) {
			pParent->AllocateAttribute();
		}
		if(pParent->m_pAttributes->ppChildren == nullptr) {
			pParent->m_pAttributes->ppChildren = (EGObject**)EG_AllocMem(sizeof(EGObject*));
			pParent->m_pAttributes->ppChildren[0] = pObject;
			pParent->m_pAttributes->ChildCount = 1;
		}
		else {
			pParent->m_pAttributes->ChildCount++;
			pParent->m_pAttributes->ppChildren = (EGObject**)EG_ReallocMem(pParent->m_pAttributes->ppChildren, sizeof(EGObject*) * pParent->m_pAttributes->ChildCount);
			pParent->m_pAttributes->ppChildren[pParent->m_pAttributes->ChildCount - 1] = pObject;
		}
	}
	EG_TRACE_OBJ_CREATE("...Finished");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::Initialise(void)
{
// 	ESP_LOGI("[Object]", "Initialising: %p", (void*)this);
 	MarkLayoutDirty();
	EGObject::EnableStyleRefresh(false);
	EGTheme::ThemeApply(this);
	Configure();
	EGObject::EnableStyleRefresh(true);
	RefreshStyle(EG_PART_ANY, EG_STYLE_PROP_ANY);
	RefreshSelfSize();
	EGGroup *pDefGroup = EGGroup::GetDefault();
	if((pDefGroup != nullptr) && IsGroupDef()) pDefGroup->AddObject(this);
	EGObject *pParent = GetParent();
	if(pParent){
		EGEvent::EventSend(pParent, EG_EVENT_CHILD_CHANGED, this);		// Call the ancestor's event handler to the parent to...
		EGEvent::EventSend(pParent, EG_EVENT_CHILD_CREATED, this);   // notify it about the new child. Also triggers layout update
		Invalidate();		// Invalidate the area
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGObject::IsEditable(void) const
{
const EG_ClassType_t *pClass = m_pClass;

	while(pClass && pClass->IsEditable == EG_OBJ_CLASS_EDITABLE_INHERIT) pClass = pClass->pBaseClassType;
	if(pClass == nullptr) return false;
	return pClass->IsEditable == EG_OBJ_CLASS_EDITABLE_TRUE ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGObject::IsGroupDef(void) const
{
const EG_ClassType_t *pClass = m_pClass;

  while(pClass && pClass->GroupDef == EG_OBJ_CLASS_GROUP_DEF_INHERIT) pClass = pClass->pBaseClassType;	// Find a base in which group_def is set
	if(pClass == nullptr) return false;
	return pClass->GroupDef == EG_OBJ_CLASS_GROUP_DEF_TRUE ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
