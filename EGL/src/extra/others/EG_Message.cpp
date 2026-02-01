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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "extra/others/EG_Message.h"

#if EG_USE_MSG
#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////

EG_EventCode_e EG_EVENT_MSG_RECEIVED;
EGList  EGMessageExec::m_MessageList;

///////////////////////////////////////////////////////////////////////////////

EGMessage::EGMessage(void) :
  m_ID(0),
  m_pExtData(nullptr),
  m_pPrivateData(nullptr),
  m_pPayload(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////
// MessageExec
///////////////////////////////////////////////////////////////////////////////

EGMessageExec::~EGMessageExec(void)
{
  m_MessageList.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////

void EGMessageExec::Initialise(void)
{
	EG_EVENT_MSG_RECEIVED = EGEvent::GenerateNewID();
}

///////////////////////////////////////////////////////////////////////////////

SubscribeDiscriptor_t* EGMessageExec::Subsribe(uint32_t MessageID, EG_MessageSubscribeCB_t SubscribeCB, void *pExtData)
{
  SubscribeDiscriptor_t *pSubscribe = (SubscribeDiscriptor_t*)EG_AllocMem(sizeof(SubscribeDiscriptor_t));
	EG_ASSERT_MALLOC(pSubscribe);
	if(pSubscribe == nullptr) return nullptr;
  m_MessageList.AddTail(pSubscribe);
	EG_ZeroMem(pSubscribe, sizeof(SubscribeDiscriptor_t));
	pSubscribe->MessageID = MessageID;
	pSubscribe->Callback = SubscribeCB;
	pSubscribe->pExtData = pExtData;
	return pSubscribe;
}

///////////////////////////////////////////////////////////////////////////////

SubscribeDiscriptor_t* EGMessageExec::SubsribeObj(uint32_t MessageID, EGObject *pObj, void *pExtData)
{
	SubscribeDiscriptor_t *pSubscribe = Subsribe(MessageID, (EG_MessageSubscribeCB_t)NotifyObjCB, pExtData);
	if(pSubscribe == nullptr) return nullptr;
	pSubscribe->pPrivateData = pObj;
	// If not added yet, add a delete event cb which automatically unsubcribes the object
	SubscribeDiscriptor_t *pSubEvent = (SubscribeDiscriptor_t*)EGEvent::GetEventExtParam(pObj, DeleteObjEventCB);
	if(pSubEvent == nullptr) {
		EGEvent::AddEventCB(pObj, DeleteObjEventCB, EG_EVENT_DELETE, pSubscribe);
	}
	return pSubscribe;
}

///////////////////////////////////////////////////////////////////////////////

void EGMessageExec::Unsubscribe(void *pSubscribe)
{
POSITION Pos;

	EG_ASSERT_NULL(pSubscribe);
  if((Pos = m_MessageList.Find(pSubscribe)) != 0) m_MessageList.RemoveAt(Pos);
	EG_FreeMem(pSubscribe);
}

///////////////////////////////////////////////////////////////////////////////

uint32_t EGMessageExec::UnsubscribeObj(uint32_t MessageID, EGObject *pObj)
{
uint32_t cnt = 0;

  POSITION Pos = m_MessageList.GetHeadPosition();
	while(Pos != nullptr){
    SubscribeDiscriptor_t *pSub = (SubscribeDiscriptor_t*)m_MessageList.GetNext(Pos);
		if(pSub->Callback == NotifyObjCB && (pSub->MessageID == LV_MSG_ID_ANY || pSub->MessageID == MessageID) && (pObj == nullptr || pSub->pPrivateData == pObj)) {
			Unsubscribe(pSub);
			cnt++;
		}
	}
	return cnt;
}

///////////////////////////////////////////////////////////////////////////////

EGMessage* EGMessageExec::GetMessage(EGEvent *pEvent)
{
	if(pEvent->m_EventCode == EG_EVENT_MSG_RECEIVED) {
		return (EGMessage*)pEvent->m_pParam;
	}
	else {
		EG_LOG_WARN("Not interpreted with this event code");
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGMessageExec::Notify(uint32_t MessageID, const void *pPayload)
{
  EGMessage Message;
	Message.m_ID = MessageID;
	Message.m_pPayload = pPayload;
	EGMessageExec::Distribute(&Message);
}

///////////////////////////////////////////////////////////////////////////////

void EGMessageExec::Distribute(EGMessage *pMessage)  // distribute to all subscribers
{
POSITION Pos;
SubscribeDiscriptor_t *pSub;

	for(pSub = (SubscribeDiscriptor_t*)m_MessageList.GetHead(Pos); pSub != nullptr; pSub = (SubscribeDiscriptor_t*)m_MessageList.GetNext(Pos)){
		if(pSub->MessageID == pMessage->m_ID && pSub->Callback) {
			pMessage->m_pExtData = pSub->pExtData;
			pMessage->m_pPrivateData = pSub->pPrivateData;
			pSub->Callback(pSub, pMessage);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGMessageExec::NotifyObjCB(void *pSubscribe, EGMessage *pMessage)
{
EG_UNUSED(pSubscribe);

	EGEvent::EventSend((EGObject*)pMessage->m_pPrivateData, EG_EVENT_MSG_RECEIVED, pMessage);
}

///////////////////////////////////////////////////////////////////////////////

void EGMessageExec::DeleteObjEventCB(EGEvent *pEvent)
{
SubscribeDiscriptor_t *pSub;

	EGObject *pObj = pEvent->GetTarget();
	POSITION Pos = m_MessageList.GetHeadPosition();
	while(Pos) {
	  pSub = (SubscribeDiscriptor_t*)m_MessageList.GetNext(Pos);
		if(pSub->pPrivateData == pObj) {
			Unsubscribe(pSub);
      Pos = m_MessageList.GetHeadPosition();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

#endif 
