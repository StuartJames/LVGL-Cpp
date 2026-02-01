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

#include "extra/others/EG_Fragment.h"

#if EG_USE_FRAGMENT

///////////////////////////////////////////////////////////////////////////////

EGFragment::EGFragment(void *pArgs) :
  m_Managed(false),
  m_pManager(nullptr),
  m_pChildManager(nullptr),
  m_pContainer(nullptr),
  m_ContentCreated(false),
  m_DestroyingContent(false),
  m_InStack(false),
  m_pContent(nullptr)
{
	m_pChildManager = new EGFragmentExec(this);
}

///////////////////////////////////////////////////////////////////////////////

EGFragment::~EGFragment(void)
{
	if(m_Managed) {
		m_pManager->Remove(this);
		return;
	}
	if(m_pContent) Destroy();
  if(m_pChildManager != nullptr) delete m_pChildManager;
}

///////////////////////////////////////////////////////////////////////////////

EGObject* EGFragment::Create(EGObject *pContainer)
{
	m_DestroyingContent = false;
	EGObject *pObj = CreateContent(pContainer);
	EG_ASSERT_NULL(pObj);
	m_pContent = pObj;
	m_pChildManager->CreateObj();
  m_ContentCreated = true;
  if(m_Managed)	EGEvent::AddEventCB(pObj, DeleteAssertionCB, EG_EVENT_DELETE, NULL);
	ContentCreated(pObj);
	return m_pContent;
}

///////////////////////////////////////////////////////////////////////////////

void EGFragment::Destroy(void)
{
	if(m_pChildManager) m_pChildManager->DeleteObj();
	if(m_Managed) {
		if(!m_ContentCreated) return;
    m_DestroyingContent = true;
		bool Removed = EGEvent::RemoveEventCB(m_pContent, DeleteAssertionCB);
		EG_ASSERT(Removed);
	}
	EG_ASSERT_NULL(m_pContent);
	DeletingContent(m_pContent);
	EGObject::Delete(m_pContent);
	ContentDeleted(m_pContent);
	m_ContentCreated = false;
	m_pContent = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void EGFragment::Recreate(void)
{
	EG_ASSERT(m_Managed == true);
	Destroy();
	Create(m_pContainer);
}

///////////////////////////////////////////////////////////////////////////////

EGFragmentExec* EGFragment::GetManager(void)
{
	EG_ASSERT(m_Managed == true);
	return m_pManager;
}

///////////////////////////////////////////////////////////////////////////////

const EGObject* EGFragment::GetContainer(void)
{
	EG_ASSERT(m_Managed == true);
	return (EGObject*)m_pContainer;
}

///////////////////////////////////////////////////////////////////////////////

EGFragment* EGFragment::GetParent(void)
{
	EG_ASSERT(m_Managed == true);
	return m_pManager->GetParentFragment();
}

///////////////////////////////////////////////////////////////////////////////

void EGFragment::DeleteAssertionCB(EGEvent *pEvent)
{
	EG_UNUSED(pEvent);
	EG_ASSERT_MSG(0, "Please delete objects with Delete");
}

#endif 
