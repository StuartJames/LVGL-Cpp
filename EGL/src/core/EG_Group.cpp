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

#include "core/EG_Group.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

EGGroup *EGGroup::m_pDefaultGroup = nullptr;
EGList  EGGroup::m_GroupList;       

///////////////////////////////////////////////////////////////////////////////////////////////////

EGGroup::EGGroup(void) :
  m_pFocusedObject(nullptr),
  m_Frozen(0),
  m_Editing(0),
  m_RefocusPolicy(0),
  m_Wrap(0)
{ 
};

///////////////////////////////////////////////////////////////////////////////////////////////////

EGGroup* EGGroup::Create(void)
{
  EGGroup *pGroup = new EGGroup;
	if(pGroup == nullptr) return nullptr;
	m_GroupList.AddHead(pGroup);
	pGroup->m_FocusCB = nullptr;
	pGroup->m_EdgeCB = nullptr;
	pGroup->m_RefocusPolicy = EG_GROUP_REFOCUS_POLICY_PREV;
	pGroup->m_Wrap = 1;
  pGroup->m_ObjectList.Initialise();
#if EG_USE_USER_DATA
	pGroup->m_pUserData = nullptr;
#endif
	return pGroup;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::Delete(EGGroup *pGroup)
{
	if(pGroup->m_pFocusedObject != nullptr) {	// Defocus the currently focused object
		EGEvent::EventSend(pGroup->GetFocused(), EG_EVENT_DEFOCUSED, pGroup->GetIndev());
		pGroup->m_pFocusedObject->Invalidate();
	}
  POSITION Pos = pGroup->m_ObjectList.GetHeadPosition();
	while(Pos != nullptr){
    EGObject *pObj = (EGObject*)pGroup->m_ObjectList.GetNext(Pos);
  	if(pObj->m_pAttributes) pObj->m_pAttributes->pGroup = nullptr;	// Remove the objects from the group
	}
	EGInputDevice *pIndev = EGInputDevice::GetNext(nullptr);	// Remove the group from any indev devices 
	while(pIndev) {
		if(pIndev->GetGroup() == pGroup) {
			pIndev->SetGroup(nullptr);
		}
		pIndev = EGInputDevice::GetNext(pIndev);
	}
	if(m_pDefaultGroup == pGroup) m_pDefaultGroup = nullptr;
	pGroup->m_ObjectList.RemoveAll();
  if((Pos = m_GroupList.Find(pGroup)) != 0) m_GroupList.RemoveAt(Pos);
	delete pGroup;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::AddObject(EGObject *pObj)
{
	EG_LOG_TRACE("begin");
  POSITION Pos = m_ObjectList.GetHeadPosition();
	while(Pos != nullptr){
	  EGObject *pGroupObj = (EGObject*)m_ObjectList.GetNext(Pos);
		if(pGroupObj == pObj) {	// Do not add the object twice
			EG_LOG_INFO("the object is already added to this group");
			return;
		}
	}
	// If the object is already in a group and focused then refocus it
	EGGroup *pGroup = (EGGroup*)pObj->GetGroup();
	if(pGroup) {
		if(pObj->m_pAttributes->pGroup && (pObj->m_pAttributes->pGroup->m_pFocusedObject == pObj)) {
			pGroup->Refocus();
			EG_LOG_INFO("changing object's group");
		}
	}
	if(pGroup != nullptr) pGroup->RemoveObject(pObj);	// removed object from its current group
	if(pObj->m_pAttributes == nullptr) pObj->AllocateAttribute();
	pObj->m_pAttributes->pGroup = this;
  m_ObjectList.AddTail(pObj);
	if(m_ObjectList.GetCount() == 1) Refocus();	// If there is only one object in the linked list automatically activate it
	EG_LOG_TRACE("finished");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::SwapObject(EGObject *pObj1, EGObject *pObj2)
{
	EGGroup *pGroup1 = (EGGroup*)pObj1->GetGroup();
	EGGroup *pGroup2 = (EGGroup*)pObj2->GetGroup();
	if((pGroup1 != pGroup2) || (pGroup1 == nullptr)) return;
  POSITION Pos = pGroup1->m_ObjectList.GetHeadPosition();
  while(Pos != nullptr){
	  EGObject *pObj = (EGObject*)pGroup1->m_ObjectList.GetNext(Pos);
		if(pObj == pObj1)	pObj = pObj2;	// Do not add the object twice
		else if(pObj == pObj2) pObj = pObj1;
	}
	EGObject *pObj = pGroup1->GetFocused();
	if(pObj == pObj1)		pGroup1->FocusObject(pObj2);
	else if(pObj == pObj2) pGroup1->FocusObject(pObj1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::RemoveObject(EGObject *pObj)
{
	EG_LOG_TRACE("begin");
	if(m_pFocusedObject && m_pFocusedObject == pObj) {	// Focus on the next object
		if(m_Frozen) m_Frozen = 0;
		// If this is the only object in the group then focus to nothing.
		if((m_ObjectList.GetHead() == m_pFocusedObject) && (m_ObjectList.GetTail() == m_pFocusedObject)){
			EGEvent::EventSend(m_pFocusedObject, EG_EVENT_DEFOCUSED, GetIndev());
      m_pFocusedObject = nullptr;
		}
		else Refocus();	// otherwise focus to the next/prev object
	}
	/* If the focuses object is still the same then it was the only object in the group but it will
   * be deleted. Set the `obj_focus` to NULL to get back to the initial state of the group with
   * zero objects */
	if(m_pFocusedObject && m_pFocusedObject == pObj) m_pFocusedObject = nullptr;
  POSITION Pos = m_ObjectList.GetHeadPosition();	// Search the object and remove it from its group
  while(Pos != nullptr){
	  EGObject *pGroupObj = (EGObject*)m_ObjectList.GetAt(Pos);
		if(pGroupObj == pObj) {
			m_ObjectList.RemoveAt(Pos);
			if(pObj->m_pAttributes) pObj->m_pAttributes->pGroup = nullptr; // possibly a waste of time
			break;
		}
    m_ObjectList.GetNextPosition(Pos);
	}
	EG_LOG_TRACE("finished");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::RemoveAllObjects(void)
{
	if(m_pFocusedObject != nullptr) {	// Defocus the currently focused object.
		EGEvent::EventSend(m_pFocusedObject, EG_EVENT_DEFOCUSED, GetIndev());
		m_pFocusedObject->Invalidate();
		m_pFocusedObject = nullptr;
	}
  POSITION Pos = m_ObjectList.GetHeadPosition();
  while(Pos != nullptr){
	  EGObject *pObj = (EGObject*)m_ObjectList.GetNext(Pos);
		if(pObj->m_pAttributes) pObj->m_pAttributes->pGroup = nullptr;	// Remove the objects from the group.
	}
	m_ObjectList.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::FocusObject(EGObject *pObj)
{
	if(pObj == nullptr) return;
	EGGroup *pGroup = (EGGroup*)pObj->GetGroup();
	if(pGroup == nullptr) return;
	if(pGroup->m_Frozen != 0) return;
	pGroup->SetEditing(false);	// Edit mode must be removed.
//  POSITION Pos = pGroup->m_ObjectList.GetHeadPosition(); // not sure what this is supposed to achieve
//  while(Pos != nullptr){
//	  EGObject *pGroupObj = (EGObject*)pGroup->m_ObjectList.GetNext(Pos);
//		if(pGroupObj == pObj) {   // Unfocus presently focused item.
			if((pGroup->m_pFocusedObject != nullptr) && (pObj != pGroup->m_pFocusedObject)) { // Do not defocus if this is the same object
				if(EGEvent::EventSend(pGroup->m_pFocusedObject, EG_EVENT_DEFOCUSED, pGroup->GetIndev()) != EG_RES_OK) return;
				pGroup->m_pFocusedObject->Invalidate();
			}
			pGroup->m_pFocusedObject = pObj;
//			if(pGroup->m_pFocusedObject != nullptr) { // Focus new item.
 				if(pGroup->m_FocusCB) pGroup->m_FocusCB(pGroup);
				if(EGEvent::EventSend(pGroup->m_pFocusedObject, EG_EVENT_FOCUSED, pGroup->GetIndev()) != EG_RES_OK) return;
 				pGroup->m_pFocusedObject->Invalidate();
//			}
//			break;
//		}
//	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::FocusNext(void)
{
	bool FocusChanged = FocusCore(true);
	if(m_EdgeCB && !FocusChanged)	m_EdgeCB(this, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::FocusPrevious(void)
{
	bool FocusChanged = FocusCore(false);
	if(m_EdgeCB && !FocusChanged)	m_EdgeCB(this, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::FocusFreeze(bool Freeze)
{
	m_Frozen = (Freeze) ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGGroup::SendData(uint32_t Data)
{
	EGObject *pObj = GetFocused();
	if(pObj == nullptr) return EG_RES_OK;
	if(pObj->HasState(EG_STATE_DISABLED)) return EG_RES_OK;
	return EGEvent::EventSend(pObj, EG_EVENT_KEY, &Data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::SetFocusCB(EG_GroupFocusCB_t FocusCB)
{
	m_FocusCB = FocusCB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::SetEdgeCB(EG_GroupEdgeCB_t EdgeCB)
{
	m_EdgeCB = EdgeCB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::SetEditing(bool Edit)
{
uint8_t Enable = Edit ? 1 : 0;

	if(Enable == m_Editing) return; // Do not set the same mode again
	m_Editing = Enable;
	EGObject *pObj = GetFocused();
	if(pObj) {
		EG_Result_t res = EGEvent::EventSend(m_pFocusedObject, EG_EVENT_FOCUSED, GetIndev());
		if(res != EG_RES_OK) return;
		pObj->Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::SetRefocusPolicy(EGGroup *pGroup, EG_GroupRefocusPolicy_e Policy)
{
	m_RefocusPolicy = Policy & 0x01;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::SetWrap(bool Wrap)
{
	m_Wrap = (Wrap) ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGObject* EGGroup::GetFocused(void)
{
	if(m_pFocusedObject == nullptr) return nullptr;
	return m_pFocusedObject;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_GroupFocusCB_t EGGroup::GetFocusCB(void)
{
	return m_FocusCB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_GroupEdgeCB_t EGGroup::GetEdgeCB()
{
	return m_EdgeCB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGGroup::GetEditing(void)
{
	return m_Editing ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGGroup::GetWrap(void)
{
	return m_Wrap ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t EGGroup::GetObjectCount(void)
{
	return m_ObjectList.GetCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//   Private Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

void EGGroup::Refocus(void)
{
uint8_t TempWrap = m_Wrap;	// Refocus must temporarily allow wrapping to work correctly

  m_Wrap = 1;
	if(m_RefocusPolicy == EG_GROUP_REFOCUS_POLICY_NEXT)	FocusNext();
	else if(m_RefocusPolicy == EG_GROUP_REFOCUS_POLICY_PREV) FocusPrevious();
	m_Wrap = TempWrap;	// Restore wrap property
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGGroup::FocusCore(bool Forward)
{
bool FocusChanged = false;
bool CanMove = true;
bool CanBegin = true;
POSITION Pos = nullptr;

	if(m_Frozen) return FocusChanged;
	EGObject *pNextObj = m_pFocusedObject;
	EGObject *pSentinel = nullptr;
	for(;;) {
		if(pNextObj == nullptr) {
			if(m_Wrap || pSentinel == nullptr) {
				if(!CanBegin) return FocusChanged;
				if(Forward) pNextObj = (EGObject*)m_ObjectList.GetHead(Pos);
        else pNextObj = (EGObject*)m_ObjectList.GetTail(Pos);
				CanMove = false;
				CanBegin = false;
			}
			else return FocusChanged;// Currently focused object is the last/first in the group, keep it that way
		}
		if(pSentinel == nullptr) {
			pSentinel = pNextObj;
			if(pSentinel == nullptr) return FocusChanged; // Group is empty
		}
		if(CanMove) {
			if(Forward) pNextObj = (EGObject*)m_ObjectList.GetNext(Pos);
      else pNextObj = (EGObject*)m_ObjectList.GetPrev(Pos);
			// Give up if we walked the entire list and haven't found another visible object
			if(pNextObj == pSentinel) return FocusChanged;
		}
		CanMove = true;
		if(pNextObj == nullptr) continue;
		if(((EGObject*)pNextObj)->GetState() & EG_STATE_DISABLED) continue;
		// Hidden objects don't receive focus. If any parent is hidden, the object is also hidden)
		EGObject *pParent = pNextObj;
		while(pParent){
			if(pParent->HasFlagSet(EG_OBJ_FLAG_HIDDEN)) break;
			pParent = pParent->GetParent();
		}
		if((pParent != nullptr) && pParent->HasFlagSet(EG_OBJ_FLAG_HIDDEN)) continue;
		break;		// If we got here a good candidate is found
  }
  if(pNextObj == m_pFocusedObject) return FocusChanged; /*There's only one visible object and it's already focused*/
	if(m_pFocusedObject) {
		if(EGEvent::EventSend(m_pFocusedObject, EG_EVENT_DEFOCUSED, GetIndev()) != EG_RES_OK) return FocusChanged;
		m_pFocusedObject->Invalidate();
	}
	m_pFocusedObject = pNextObj;
	if(EGEvent::EventSend(m_pFocusedObject, EG_EVENT_FOCUSED, GetIndev()) != EG_RES_OK) return FocusChanged;
	m_pFocusedObject->Invalidate();
	if(m_FocusCB) m_FocusCB(this);
	FocusChanged = true;
	return FocusChanged;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

 EGInputDevice* EGGroup::GetIndev(void)
{
EGInputDevice *pIndevEncoder = nullptr;
EGInputDevice *pIndevGroup = nullptr;

	EGInputDevice *pIndev = EGInputDevice::GetNext(nullptr);
	while(pIndev != nullptr) {
		EG_InDeviceType_e InputType = pIndev->GetType();
		if(pIndev->GetGroup() == this) {
			if(InputType == EG_INDEV_TYPE_KEYPAD) return pIndev;	// Prefer KEYPAD
			if(InputType == EG_INDEV_TYPE_ENCODER) pIndevEncoder = pIndev;
			pIndevGroup = pIndev;
		}
		pIndev = EGInputDevice::GetNext(pIndev);
	}
	if(pIndevEncoder) return pIndevEncoder;
	if(pIndevGroup) return pIndevGroup;
	// In the lack of a better option use the first input device. (It can be NULL if there is no input device)
	return EGInputDevice::GetNext(nullptr);
}
