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


#include "core/EG_Object.h"
#include "core/EG_InputDevice.h"
#include "misc/EG_Animate.h"
#include "misc/lv_gc.h"
#include "misc/EG_Async.h"

/////////////////////////////////////////////////////////////////////////////

#define OBJ_CLASS &c_ObjectClass

/////////////////////////////////////////////////////////////////////////////

void EGObject::Delete(EGObject *pObj)
{
	EG_LOG_TRACE("begin (delete %p)", (void *)pObj);
	pObj->Invalidate();
	EGObject *pParent = pObj->GetParent();
	EGDisplay *pDisplay = nullptr;
	bool ActiveScreenDelete = false;
	if(pParent == nullptr) {
		pDisplay = pObj->GetDisplay();
		if(!pDisplay) return;             // Shouldn't happen
		if(pDisplay->m_pActiveScreen == pObj) ActiveScreenDelete = true;
	}
	DeleteCore(pObj);
	if(pParent) {	    // Call the ancestor's event handler to the parent to notify it about the child delete
		pParent->ScrollbarInvalidate();
		EGEvent::EventSend(pParent, EG_EVENT_CHILD_CHANGED, nullptr);
		EGEvent::EventSend(pParent, EG_EVENT_CHILD_DELETED, nullptr);
	}
	if(ActiveScreenDelete) {	// Handle if the active screen was deleted
		EG_LOG_WARN("the active screen was deleted");
		pDisplay->m_pActiveScreen = nullptr;
	}
	EG_ASSERT_MEM_INTEGRITY();
	EG_LOG_TRACE("finished (delete %p)", (void *)pObj);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::Clean(EGObject *pObj)
{
	EG_LOG_TRACE("begin (delete %p)", (void *)pObj);
	pObj->Invalidate();
	EGObject *pChild = pObj->GetChild(0);
	while(pChild) {
		Delete(pChild);
		pChild = pObj->GetChild(0);
	}
	pObj->ScrollTo(0, 0, EG_ANIM_OFF);	//Just to remove scroll animations if any
	if(pObj->m_pAttributes) {
		pObj->m_pAttributes->pScroll->m_X = 0;
		pObj->m_pAttributes->pScroll->m_Y = 0;
	}
	EG_ASSERT_MEM_INTEGRITY();
	EG_LOG_TRACE("finished (delete %p)", (void *)pObj);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::DeleteDelayed(EGObject *pObj, uint32_t Delay)
{
	EGAnimate Animate;
	Animate.SetItem(pObj);
	Animate.SetExcCB(nullptr);
	Animate.SetTime(1);
	Animate.SetDelay(Delay);
	Animate.SetEndCB(DeleteAnimationEndCB);
	EGAnimate::Create(&Animate);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::DeleteAsync(EGObject *pObj)
{
	EGAsyncFunc::Create(DeleteAsyncCB, pObj);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetParent(EGObject *pParent)
{
	if(m_pParent == nullptr) {
		EG_LOG_WARN("Can't set the parent of a screen");
		return;
	}
	if(pParent == nullptr) {
		EG_LOG_WARN("Can't set parent == NULL to an object");
		return;
	}
	Invalidate();
	pParent->AllocateAttribute();
	EGObject *pOldParent = GetParent();
	for(int32_t i = GetIndex(); i <= (int32_t)pOldParent->GetChildCount() - 2; i++) {	// Remove the object from the old parent's child list
		pOldParent->m_pAttributes->ppChildren[i] = pOldParent->m_pAttributes->ppChildren[i + 1];
	}
	pOldParent->m_pAttributes->ChildCount--;
	if(pOldParent->m_pAttributes->ChildCount) {
		pOldParent->m_pAttributes->ppChildren = (EGObject**)EG_ReallocMem(pOldParent->m_pAttributes->ppChildren, pOldParent->m_pAttributes->ChildCount * (sizeof(EGObject*)));
	}
	else {
		EG_FreeMem(pOldParent->m_pAttributes->ppChildren);
		pOldParent->m_pAttributes->ppChildren = nullptr;
	}
	pParent->m_pAttributes->ChildCount++;       	// Add the child to the new parent as the last (newest child)
	pParent->m_pAttributes->ppChildren = (EGObject**)EG_ReallocMem(pParent->m_pAttributes->ppChildren, pParent->m_pAttributes->ChildCount * (sizeof(EGObject*)));
	pParent->m_pAttributes->ppChildren[pParent->GetChildCount() - 1] = this;
	m_pParent = pParent;
	pOldParent->ScrollbarInvalidate();	// Notify the original parent because one of its ppChildren is lost
	EGEvent::EventSend(pOldParent, EG_EVENT_CHILD_CHANGED, this);
	EGEvent::EventSend(pOldParent, EG_EVENT_CHILD_DELETED, nullptr);
	/*Notify the new parent about the child*/
	EGEvent::EventSend(pParent, EG_EVENT_CHILD_CHANGED, this);
	EGEvent::EventSend(pParent, EG_EVENT_CHILD_CREATED, nullptr);
	MarkLayoutDirty();
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::MoveToIndex(int32_t Index)
{
	if(Index < 0) {
		Index = m_pParent->GetChildCount() + Index;
	}
	const int32_t OldIndex = GetIndex();
	EGObject *pParent = GetParent();
	if(Index < 0) return;
	if(Index >= (int32_t)pParent->GetChildCount()) return;
	if(Index == OldIndex) return;
	int32_t i = OldIndex;
	if(Index < OldIndex) {
		while(i > Index) {
			pParent->m_pAttributes->ppChildren[i] = pParent->m_pAttributes->ppChildren[i - 1];
			i--;
		}
	}
	else {
		while(i < Index) {
			pParent->m_pAttributes->ppChildren[i] = pParent->m_pAttributes->ppChildren[i + 1];
			i++;
		}
	}
	pParent->m_pAttributes->ppChildren[Index] = this;
	EGEvent::EventSend(pParent, EG_EVENT_CHILD_CHANGED, nullptr);
	pParent->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::Swap(EGObject *pObj1, EGObject *pObj2)
{
	EGObject *pParent1 = pObj1->GetParent();
	EGObject *pParent2 = pObj2->GetParent();
	uint_fast32_t Index1 = pObj1->GetIndex();
	uint_fast32_t Index2 = pObj2->GetIndex();
	EGEvent::EventSend(pParent2, EG_EVENT_CHILD_DELETED, pObj2);
	EGEvent::EventSend(pParent1, EG_EVENT_CHILD_DELETED, pObj1);
	pParent1->m_pAttributes->ppChildren[Index1] = pObj2;
	pParent2->m_pAttributes->ppChildren[Index2] = pObj1;
	EGEvent::EventSend(pParent1, EG_EVENT_CHILD_CHANGED, pObj2);
	EGEvent::EventSend(pParent1, EG_EVENT_CHILD_CREATED, pObj2);
	EGEvent::EventSend(pParent2, EG_EVENT_CHILD_CHANGED, pObj1);
	EGEvent::EventSend(pParent2, EG_EVENT_CHILD_CREATED, pObj1);
	pParent1->Invalidate();
	if(pParent1 != pParent2) pParent2->Invalidate();
	EGGroup::SwapObject(pObj1, pObj2);
}

/////////////////////////////////////////////////////////////////////////////

EGObject* EGObject::GetScreen(void)
{
EGObject *pObj = this;

//  EG_LOG_WARN("Obj:%p", (void *)pObj);
  do{
    if(pObj->m_pParent == nullptr) return pObj;
		pObj = pObj->m_pParent;
  }
  while(pObj != nullptr);
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////

EGDisplay* EGObject::GetDisplay(void)
{
const EGObject *pScreen;

	if(m_pParent == nullptr) pScreen = this; // this object is a screen
	else pScreen = GetScreen();           // get the screen of this object
	return EGDisplay::GetDisplay(pScreen);
}

/////////////////////////////////////////////////////////////////////////////

EGObject* EGObject::GetChild(int32_t Index)
{
uint32_t CheckIndex;

	if(m_pAttributes == nullptr) return nullptr;
	if(Index < 0){
		Index = m_pAttributes->ChildCount + Index;
		if(Index < 0) return nullptr;
		CheckIndex = (uint32_t)Index;
	}
	else	CheckIndex = Index;
	if(CheckIndex >= m_pAttributes->ChildCount) return nullptr;
	return m_pAttributes->ppChildren[Index];
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGObject::GetChildCount(void)
{
	if(m_pAttributes == nullptr) return 0;
	return m_pAttributes->ChildCount;
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGObject::GetIndex(void) const
{
	EGObject *pParent = m_pParent;
	if(pParent == nullptr) return 0;
	for(uint32_t i = 0; i < pParent->GetChildCount(); i++) {
		if(pParent->GetChild(i) == this) return i;
	}
	return 0xFFFFFFFF; // Shouldn't happen
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::TreeWalk(EGObject *pObj, ObjTreeWalkCB_t WalkCB, void *pUserData)
{
	WalkCore(pObj, WalkCB, pUserData);
}

/////////////////////////////////////////////////////////////////////////////
//   STATIC
/////////////////////////////////////////////////////////////////////////////

void EGObject::DeleteAsyncCB(void *pObj)
{
	EGObject::Delete((EGObject*)pObj);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::DeleteAnimationEndCB(EGAnimate *pAnimate)
{
	EGObject::Delete((EGObject*)pAnimate->m_pItem);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::DeleteCore(EGObject *pObj)
{
	// Let the user free the resources used in `EG_EVENT_DELETE`
	EG_Result_t Result = EGEvent::EventSend(pObj, EG_EVENT_DELETE, nullptr);
	if(Result == EG_RES_INVALID) return;
	pObj->m_IsBeingDeleted = 1;
	EGObject *pChild = pObj->GetChild(0);
	while(pChild){	    // Recursively delete the children
		DeleteCore(pChild);
		pChild = pObj->GetChild(0);
	}
	EGGroup *pGroup = (EGGroup*)pObj->GetGroup();
	EGInputDevice *pIndev = EGInputDevice::GetNext(nullptr);	// Reset all input devices if the object to delete is used
	while(pIndev){
		if((pIndev->m_Process.Pointer.pActiveObj == pObj) || (pIndev->m_Process.Pointer.pLastObj == pObj)){
			EGInputDevice::Reset(pIndev, pObj);
		}
		if(pIndev->m_Process.Pointer.pLastPressedObj == pObj) pIndev->m_Process.Pointer.pLastPressedObj = nullptr;
		if((pIndev->GetGroup() == pGroup) && (pObj == pIndev->GetActiveObj())) EGInputDevice::Reset(pIndev, pObj);
		pIndev = EGInputDevice::GetNext(pIndev);
	}
	if(pObj->m_pParent == nullptr) {	// Remove the screen for the screen list
		EGDisplay *pDisplay = pObj->GetDisplay();
		uint32_t i;
		for(i = 0; i < pDisplay->m_ScreenCount; i++) {		// Find the screen in the list
			if(pDisplay->m_pScreens[i] == pObj) break;
		}
		uint32_t id = i;
		for(i = id; i < pDisplay->m_ScreenCount - 1; i++) {
			pDisplay->m_pScreens[i] = pDisplay->m_pScreens[i + 1];
		}
		pDisplay->m_ScreenCount--;
		pDisplay->m_pScreens = (EGObject**)EG_ReallocMem(pDisplay->m_pScreens, pDisplay->m_ScreenCount * sizeof(EGObject*));
	}
	else{	// Remove the object from the child list of its parent
		uint32_t Index = pObj->GetIndex();
		for(uint32_t i = Index; i < pObj->m_pParent->m_pAttributes->ChildCount - 1; i++) {
			pObj->m_pParent->m_pAttributes->ppChildren[i] = pObj->m_pParent->m_pAttributes->ppChildren[i + 1];
		}
		pObj->m_pParent->m_pAttributes->ChildCount--;
		pObj->m_pParent->m_pAttributes->ppChildren = (EGObject**)EG_ReallocMem(pObj->m_pParent->m_pAttributes->ppChildren, pObj->m_pParent->m_pAttributes->ChildCount * sizeof(EGObject*));
	}
  delete pObj;
}

/////////////////////////////////////////////////////////////////////////////

EG_TreeWalkResult_e EGObject::WalkCore(EGObject *pObj, ObjTreeWalkCB_t TreeWalkCB, void *pUserData)
{
EG_TreeWalkResult_e Result = EG_TREE_WALK_NEXT;

	if(pObj == nullptr) {
		EGDisplay *pDisplay = EGDisplay::GetNext(nullptr);
		while(pDisplay) {
			for(uint32_t i = 0; i < pDisplay->m_ScreenCount; i++) {
				WalkCore(pDisplay->m_pScreens[i], TreeWalkCB, pUserData);
			}
			pDisplay = EGDisplay::GetNext(pDisplay);
		}
		return EG_TREE_WALK_END; // The value doesn't matter as it wasn't called recursively
	}
	Result = TreeWalkCB(pObj, pUserData);
	if(Result == EG_TREE_WALK_END) return EG_TREE_WALK_END;
	if(Result != EG_TREE_WALK_SKIP_CHILDREN) {
		for(uint32_t i = 0; i < pObj->GetChildCount(); i++) {
			Result = WalkCore(pObj->GetChild(i), TreeWalkCB, pUserData);
			if(Result == EG_TREE_WALK_END) return EG_TREE_WALK_END;
		}
	}
	return EG_TREE_WALK_NEXT;
}


