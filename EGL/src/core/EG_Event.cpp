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

#include "core/EG_Event.h"

///////////////////////////////////////////////////////////////////////////////////////

#if EG_LOG_TRACE_EVENT
#define EVENT_TRACE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define EVENT_TRACE(...)
#endif

uint32_t EGEvent::m_LastID = _EG_EVENT_LAST;
EGEvent *EGEvent::m_pEventHead = nullptr;

///////////////////////////////////////////////////////////////////////////////////////

EGEvent::EGEvent(void) :
  m_pTarget(nullptr),
  m_pCurrentTarget(nullptr),
  m_EventCode(EG_EVENT_ALL),
  m_pExtParam(nullptr),
  m_pParam(nullptr),
  m_pPrevious(nullptr),
  m_Deleted(0),
  m_StopProcessing(0),
  m_StopBubbling(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGEvent::Pump(const EG_ClassType_t *pClassType)
{
const EG_ClassType_t *pNextClass;

	if(pClassType == nullptr)	pNextClass = m_pCurrentTarget->m_pClass;    // the intial class
	else pNextClass = pClassType->pBaseClassType;                             // sub-classes
	// Find a base class with the event handler callback set
	while(pNextClass && pNextClass->pEventCB == nullptr) pNextClass = pNextClass->pBaseClassType;
	if(pNextClass == nullptr) return EG_RES_OK;
	if(pNextClass->pEventCB == nullptr) return EG_RES_OK;
	m_pExtParam = nullptr;      // Call the actual event callback
 	EG_LOG_TRACE("Event:%p, Base:%p", (void*)this, (void*)pClassType);
//  ESP_LOGI("[Event ]", "Base Event:%p, Code:%d", (void*)this, m_EventCode);
	pNextClass->pEventCB(pNextClass, this);	  // Widget type specific event function
	EG_Result_t Result = (m_Deleted) ? EG_RES_INVALID: EG_RES_OK;	// Stop if the object is deleted
 	EG_LOG_TRACE("Event base");
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_EventCode_e EGEvent::GenerateNewID(void)
{
	m_LastID++;
	return (EG_EventCode_e)m_LastID;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEvent::MarkDeleted(EGObject *pObj)
{
EGEvent *pEvent = m_pEventHead;

	while(pEvent) {
		if(pEvent->m_pCurrentTarget == pObj || pEvent->m_pTarget == pObj) pEvent->m_Deleted = 1;
		pEvent = pEvent->m_pPrevious;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

struct EG_EventDiscriptor_t* EGEvent::AddEventCB(EGObject *pObj, EG_EventCB_t EventCB, EG_EventCode_e Filter, void *pParam)
{
	pObj->AllocateAttribute();
	pObj->m_pAttributes->EventDescriptorCount++;
	pObj->m_pAttributes->pEventDescriptor = (EG_EventDiscriptor_t*)EG_ReallocMem(pObj->m_pAttributes->pEventDescriptor, pObj->m_pAttributes->EventDescriptorCount * sizeof(EG_EventDiscriptor_t));
	EG_ASSERT_MALLOC(pObj->m_pAttributes->pEventDescriptor);
	pObj->m_pAttributes->pEventDescriptor[pObj->m_pAttributes->EventDescriptorCount - 1].EventCB = EventCB;
	pObj->m_pAttributes->pEventDescriptor[pObj->m_pAttributes->EventDescriptorCount - 1].Filter = Filter;
	pObj->m_pAttributes->pEventDescriptor[pObj->m_pAttributes->EventDescriptorCount - 1].pExtParam = pParam;
	return &pObj->m_pAttributes->pEventDescriptor[pObj->m_pAttributes->EventDescriptorCount - 1];
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEvent::RemoveEventCB(EGObject *pObj, EG_EventCB_t EventCB)
{
int32_t i = 0;

	if(pObj->m_pAttributes == nullptr) return false;
	for(i = 0; i < pObj->m_pAttributes->EventDescriptorCount; i++) {
		if(EventCB == nullptr || pObj->m_pAttributes->pEventDescriptor[i].EventCB == EventCB) {
			for(; i < (pObj->m_pAttributes->EventDescriptorCount - 1); i++) {	// Shift the remaining event handlers forward
				pObj->m_pAttributes->pEventDescriptor[i] = pObj->m_pAttributes->pEventDescriptor[i + 1];
			}
			pObj->m_pAttributes->EventDescriptorCount--;
			pObj->m_pAttributes->pEventDescriptor = (EG_EventDiscriptor_t *)EG_ReallocMem(pObj->m_pAttributes->pEventDescriptor, pObj->m_pAttributes->EventDescriptorCount * sizeof(EG_EventDiscriptor_t));
			EG_ASSERT_MALLOC(pObj->m_pAttributes->pEventDescriptor);
			return true;
		}
	}
	return false;	//No event handler found
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEvent::RemoveEventCBWithUserData(EGObject *pObj, EG_EventCB_t EventCB, const void *pParam)
{
	if(pObj->m_pAttributes == nullptr) return false;

	int32_t i = 0;
	for(i = 0; i < pObj->m_pAttributes->EventDescriptorCount; i++) {
		if((EventCB == nullptr || pObj->m_pAttributes->pEventDescriptor[i].EventCB == EventCB) &&
			 pObj->m_pAttributes->pEventDescriptor[i].pExtParam == pParam) {	// shift the remaining event handlers forward
			for(; i < (pObj->m_pAttributes->EventDescriptorCount - 1); i++) {
				pObj->m_pAttributes->pEventDescriptor[i] = pObj->m_pAttributes->pEventDescriptor[i + 1];
			}
			pObj->m_pAttributes->EventDescriptorCount--;
			pObj->m_pAttributes->pEventDescriptor = (EG_EventDiscriptor_t *)EG_ReallocMem(pObj->m_pAttributes->pEventDescriptor,
																								pObj->m_pAttributes->EventDescriptorCount * sizeof(EG_EventDiscriptor_t));
			EG_ASSERT_MALLOC(pObj->m_pAttributes->pEventDescriptor);
			return true;
		}
	}
	return false;	// No event handler found
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEvent::RemoveEventDiscriptor(EGObject *pObj, struct EG_EventDiscriptor_t *pEventDescriptor)
{
	if(pObj->m_pAttributes == nullptr) return false;
	int32_t i = 0;
	for(i = 0; i < pObj->m_pAttributes->EventDescriptorCount; i++) {
		if(&pObj->m_pAttributes->pEventDescriptor[i] == pEventDescriptor) {	// Shift the remaining event handlers forward
			for(; i < (pObj->m_pAttributes->EventDescriptorCount - 1); i++) {
				pObj->m_pAttributes->pEventDescriptor[i] = pObj->m_pAttributes->pEventDescriptor[i + 1];
			}
			pObj->m_pAttributes->EventDescriptorCount--;
			pObj->m_pAttributes->pEventDescriptor = (EG_EventDiscriptor_t *)EG_ReallocMem(pObj->m_pAttributes->pEventDescriptor,
																							pObj->m_pAttributes->EventDescriptorCount * sizeof(EG_EventDiscriptor_t));
			EG_ASSERT_MALLOC(pObj->m_pAttributes->pEventDescriptor);
			return true;
		}
	}
	return false;	// No event handler found
}

///////////////////////////////////////////////////////////////////////////////////////

EG_EventDiscriptor_t* EGEvent::GetDiscriptor(const EGObject *pObj, uint32_t Id)
{
	if(pObj->m_pAttributes == nullptr) return nullptr;
	if(Id >= pObj->m_pAttributes->EventDescriptorCount) return nullptr;
	return &pObj->m_pAttributes->pEventDescriptor[Id];
}

///////////////////////////////////////////////////////////////////////////////////////

void* EGEvent::GetEventExtParam(EGObject *pObj, EG_EventCB_t EventCB)
{
	if(pObj->m_pAttributes == nullptr) return nullptr;
	for(int32_t i = 0; i < pObj->m_pAttributes->EventDescriptorCount; i++) {
		if(EventCB == pObj->m_pAttributes->pEventDescriptor[i].EventCB) return pObj->m_pAttributes->pEventDescriptor[i].pExtParam;
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

EGInputDevice* EGEvent::GetInputDevice(void)
{
	if(m_EventCode == EG_EVENT_PRESSED ||
		 m_EventCode == EG_EVENT_PRESSING ||
		 m_EventCode == EG_EVENT_PRESS_LOST ||
		 m_EventCode == EG_EVENT_SHORT_CLICKED ||
		 m_EventCode == EG_EVENT_LONG_PRESSED ||
		 m_EventCode == EG_EVENT_LONG_PRESSED_REPEAT ||
		 m_EventCode == EG_EVENT_CLICKED ||
		 m_EventCode == EG_EVENT_RELEASED ||
		 m_EventCode == EG_EVENT_SCROLL_BEGIN ||
		 m_EventCode == EG_EVENT_SCROLL_END ||
		 m_EventCode == EG_EVENT_SCROLL ||
		 m_EventCode == EG_EVENT_GESTURE ||
		 m_EventCode == EG_EVENT_KEY ||
		 m_EventCode == EG_EVENT_FOCUSED ||
		 m_EventCode == EG_EVENT_DEFOCUSED ||
		 m_EventCode == EG_EVENT_LEAVE) {
		return (EGInputDevice*)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EGDrawContext* EGEvent::GetDrawPartDiscriptor(void)
{
	if(m_EventCode == EG_EVENT_DRAW_PART_BEGIN || m_EventCode == EG_EVENT_DRAW_PART_END) {
		return (EGDrawContext*)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EGDrawContext* EGEvent::GetDrawContext(void)
{
	if(m_EventCode == EG_EVENT_DRAW_MAIN ||
		 m_EventCode == EG_EVENT_DRAW_MAIN_BEGIN ||
		 m_EventCode == EG_EVENT_DRAW_MAIN_END ||
		 m_EventCode == EG_EVENT_DRAW_POST ||
		 m_EventCode == EG_EVENT_DRAW_POST_BEGIN ||
		 m_EventCode == EG_EVENT_DRAW_POST_END) {
		return (EGDrawContext*)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

const EGRect* EGEvent::GetOldSize(void)
{
	if(m_EventCode == EG_EVENT_SIZE_CHANGED) {
		return (EGRect *)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGEvent::GetKey(void)
{
	if(m_EventCode == EG_EVENT_KEY) {
		uint32_t *pKey = (uint32_t *)GetParam();
		if(pKey)	return *pKey;
		else return 0;
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EGAnimate* EGEvent::GetScrollAnimation(void)
{
	if(m_EventCode == EG_EVENT_SCROLL_BEGIN) {
		return (EGAnimate*)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEvent::SetExtDrawSize(EG_Coord_t Size)
{
	if(m_EventCode == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		EG_Coord_t *pCurrentSize = (EG_Coord_t *)GetParam();
		*pCurrentSize = EG_MAX(*pCurrentSize, Size);
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EGPoint* EGEvent::GetSelfSizeInfo()
{
	if(m_EventCode == EG_EVENT_GET_SELF_SIZE) {
		return (EGPoint*)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_HitTestState_t* EGEvent::GetHitTestInfo()
{
	if(m_EventCode == EG_EVENT_HIT_TEST) {
		return (EG_HitTestState_t *)GetParam();
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

const EGRect* EGEvent::GetCoverArea()
{
	if(m_EventCode == EG_EVENT_COVER_CHECK) {
		EG_CoverCheckInfo_t *pParam = (EG_CoverCheckInfo_t *)GetParam();
		return pParam->pRect;
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEvent::SetCoverResult(EG_CoverResult_e Result)
{
	if(m_EventCode == EG_EVENT_COVER_CHECK) {
		EG_CoverCheckInfo_t *pParam = (EG_CoverCheckInfo_t *)GetParam();
		if(Result > pParam->Result) pParam->Result = Result; // Save only "stronger" results
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGEvent::SendCore(void)
{
uint32_t i = 0;

	EVENT_TRACE("Core event %d to %p with %p param", m_EventCode, (void *)m_pCurrentTarget, m_pParam);
//	ESP_LOGI("[Event ]", "Core event %d to %p with %p param", m_EventCode, (void *)m_pCurrentTarget, m_pParam);
	EGInputDevice *pActiveInput = EGInputDevice::GetActive();	// Call the input device's feedback callback if set
	if(pActiveInput) {
		if(pActiveInput->m_pDriver->FeedbackCB) pActiveInput->m_pDriver->FeedbackCB(pActiveInput->m_pDriver, m_EventCode);
		if(m_StopProcessing) return EG_RES_OK;
		if(m_Deleted) return EG_RES_INVALID;
	}
	EG_Result_t Result = EG_RES_OK;
	EG_EventDiscriptor_t *pEventDescriptor = GetDiscriptor(m_pCurrentTarget, 0);
	while(pEventDescriptor && Result == EG_RES_OK) {
		if(pEventDescriptor->EventCB && ((pEventDescriptor->Filter & EG_EVENT_PREPROCESS) == EG_EVENT_PREPROCESS) &&
        (pEventDescriptor->Filter == (EG_EVENT_ALL | EG_EVENT_PREPROCESS) ||
			  (pEventDescriptor->Filter & ~EG_EVENT_PREPROCESS) == m_EventCode)) {
			m_pExtParam = pEventDescriptor->pExtParam;
			pEventDescriptor->EventCB(this);
			if(m_StopProcessing) return EG_RES_OK;
			if(m_Deleted) return EG_RES_INVALID;			// Stop if the object is deleted
		}
		i++;
		pEventDescriptor = GetDiscriptor(m_pCurrentTarget, i);
	}
 	Result = Pump(nullptr); // Pump the event through the current and parent classes
	pEventDescriptor = (Result == EG_RES_INVALID) ? nullptr : GetDiscriptor(m_pCurrentTarget, 0);
	i = 0;
	while(pEventDescriptor && Result == EG_RES_OK) {
		if(pEventDescriptor->EventCB && ((pEventDescriptor->Filter & EG_EVENT_PREPROCESS) == 0) &&
        (pEventDescriptor->Filter == EG_EVENT_ALL || pEventDescriptor->Filter == m_EventCode)) {
			m_pExtParam = pEventDescriptor->pExtParam;
			pEventDescriptor->EventCB(this);
			if(m_StopProcessing) return EG_RES_OK;
			if(m_Deleted) return EG_RES_INVALID;			// Stop if the object is deleted
		}
		i++;
		pEventDescriptor = GetDiscriptor(m_pCurrentTarget, i);
	}
	if(Result == EG_RES_OK && m_pCurrentTarget->GetParent() && ShouldBubble()){
		m_pCurrentTarget = m_pCurrentTarget->GetParent();
		Result = SendCore();
		if(Result != EG_RES_OK) return EG_RES_INVALID;
	}
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGEvent::EventSend(EGObject *pObj, EG_EventCode_e EventCode, void *pParam)
{
	if(pObj == nullptr) return EG_RES_OK;
	EGEvent Event;
	Event.m_pTarget = pObj;
	Event.m_pCurrentTarget = pObj;
	Event.m_EventCode = EventCode;
	Event.m_pExtParam = nullptr;
	Event.m_pParam = pParam;
	Event.m_Deleted = 0;
	Event.m_StopProcessing = 0;
	Event.m_StopBubbling = 0;

	// Build a simple linked list from the objects used in the events
  // It's important to know if this object was deleted by a nested event called from this `event_cb`.
	Event.m_pPrevious = m_pEventHead;       // plug in the new event
	m_pEventHead = &Event;
	EG_Result_t Result = Event.SendCore();  // Send the event
	m_pEventHead = Event.m_pPrevious;	      // event has been actioned Remove it from the list
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEvent::ShouldBubble()
{
	if(m_StopBubbling) return false;
	switch(m_EventCode) {	        //Event codes that always bubble
		case EG_EVENT_CHILD_CREATED:
		case EG_EVENT_CHILD_DELETED:
			return true;
		default:
			break;
	}
	// Check other codes only if bubbling is enabled
	if(m_pCurrentTarget->HasFlagSet(EG_OBJ_FLAG_EVENT_BUBBLE) == false) return false;
	switch(m_EventCode){
		case EG_EVENT_HIT_TEST:
		case EG_EVENT_COVER_CHECK:
		case EG_EVENT_REFR_EXT_DRAW_SIZE:
		case EG_EVENT_DRAW_MAIN_BEGIN:
		case EG_EVENT_DRAW_MAIN:
		case EG_EVENT_DRAW_MAIN_END:
		case EG_EVENT_DRAW_POST_BEGIN:
		case EG_EVENT_DRAW_POST:
		case EG_EVENT_DRAW_POST_END:
		case EG_EVENT_DRAW_PART_BEGIN:
		case EG_EVENT_DRAW_PART_END:
		case EG_EVENT_REFRESH:
		case EG_EVENT_DELETE:
		case EG_EVENT_CHILD_CREATED:
		case EG_EVENT_CHILD_DELETED:
		case EG_EVENT_CHILD_CHANGED:
		case EG_EVENT_SIZE_CHANGED:
		case EG_EVENT_STYLE_CHANGED:
		case EG_EVENT_GET_SELF_SIZE:
			return false;
		default:
			return true;
	}
}

