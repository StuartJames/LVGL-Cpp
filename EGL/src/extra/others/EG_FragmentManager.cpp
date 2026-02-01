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


///////////////////////////////////////////////////////////////////////////////

EGFragmentExec::EGFragmentExec(EGFragment *pParent)
{
	m_pParent = pParent;
  m_Attached.Initialise();  
  m_Stack.Initialise();        
}

///////////////////////////////////////////////////////////////////////////////

EGFragmentExec::~EGFragmentExec(void)
{
POSITION Pos = nullptr;
EGFragment *pFragment;

	for(pFragment = (EGFragment*)m_Attached.GetTail(Pos); pFragment != nullptr; pFragment = (EGFragment*)m_Attached.GetPrev(Pos)){
    pFragment->Destroy();
    pFragment->Detached();
	  pFragment->m_Managed = false;
	}
  m_Attached.RemoveAll();
  m_Stack.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////

void EGFragmentExec::CreateObj(void)
{
POSITION Pos = nullptr;
EGFragment *pFragment = nullptr;

	EGFragment *pTop = (EGFragment*)m_Stack.GetTail();
	for(pFragment = (EGFragment*)m_Attached.GetTail(Pos); pFragment != nullptr; pFragment = (EGFragment*)m_Attached.GetPrev(Pos)){
		if(pFragment->m_InStack && pTop != pFragment) 	continue;		// Only create obj for top item in stack
    pFragment->Create(pFragment->m_pContainer);
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGFragmentExec::DeleteObj(void)
{
POSITION Pos = nullptr;
EGFragment *pFragment = nullptr;

	for(pFragment = (EGFragment*)m_Attached.GetTail(Pos); pFragment != nullptr; pFragment = (EGFragment*)m_Attached.GetPrev(Pos)){
  	pFragment->Destroy();
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGFragmentExec::Add(EGFragment *pFragment, EGObject *pContainer)
{
	AttachFragment(pFragment, pContainer);
	if(!m_pParent || m_pParent->m_ContentCreated) pFragment->Create(pFragment->m_pContainer);
}

///////////////////////////////////////////////////////////////////////////////

void EGFragmentExec::Remove(EGFragment *pFragment)
{
POSITION Pos = nullptr;
EGFragment *pPrevious = nullptr, *pItem = nullptr;
bool WasTop = false;

	EG_ASSERT_NULL(pFragment);
	EG_ASSERT(pFragment->m_Managed = true);
	EG_ASSERT(pFragment->m_pManager == this);
	if(pFragment->m_InStack) {
		EGFragment *pStackTop = (EGFragment*)m_Stack.GetTail();
	  for(pItem = (EGFragment*)m_Stack.GetTail(Pos); pItem != nullptr; pItem = (EGFragment*)m_Stack.GetPrev(Pos)){
			if(pItem == pFragment) {
				WasTop = (pStackTop == pItem) ? true : false;  // was it the active fragment
				EGFragment *pStackPrev = (EGFragment*)m_Stack.GetPrev(Pos);
				if(!pStackPrev) break;
				pPrevious = pStackPrev;
				break;
			}
		}       // if it was found remove it from the stack
		if(pItem) if((Pos = m_Stack.Find(pItem)) != nullptr) m_Stack.RemoveAt(Pos);
	}
	pFragment->Destroy();
  pFragment->Detached();
	pFragment->m_Managed = false;
  if((Pos = m_Attached.Find(pFragment)) != nullptr) delete (EGFragment*)m_Attached.RemoveAt(Pos);
	if(pPrevious && WasTop) pPrevious->Create(pPrevious->m_pContainer);
}

///////////////////////////////////////////////////////////////////////////////

void EGFragmentExec::Push(EGFragment *pFragment, EGObject *pContainer)
{
  EGFragment *pStackTop = (EGFragment*)m_Stack.GetTail();
  if(pStackTop != nullptr) {
	  pStackTop->Destroy();
	}
	AttachFragment(pFragment, pContainer);
	pFragment->m_InStack = true;
  m_Stack.AddTail(pFragment);	// Add fragment to the top of the stack
	pFragment->Create(pFragment->m_pContainer);
}

///////////////////////////////////////////////////////////////////////////////

bool EGFragmentExec::Pop(void)
{
  EGFragment *pFragment = (EGFragment*)m_Stack.GetTail();
	if(pFragment == nullptr) return false;
	Remove(pFragment);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void EGFragmentExec::Replace(EGFragment *pFragment, EGObject *pContainer)
{
	EGFragment *pTop = FindByContainer(pContainer);
	if(pTop != nullptr) Remove(pTop);
	Add(pFragment, pContainer);
}

///////////////////////////////////////////////////////////////////////////////

bool EGFragmentExec::SendEvent(EGFragmentExec *pManager, int Code, void *pExtData)
{
EGFragment *pFragment = nullptr;
POSITION Pos = nullptr;

  for(pFragment = (EGFragment*)pManager->m_Attached.GetTail(Pos); pFragment != nullptr; pFragment = (EGFragment*)pManager->m_Attached.GetPrev(Pos)){
		if(!pFragment->m_ContentCreated || pFragment->m_DestroyingContent) continue;
		if(SendEvent(pFragment->m_pChildManager, Code, pExtData)) return true;
		return pFragment->EventCB(Code, pExtData);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

size_t EGFragmentExec::GetStackSize(void)
{
	return m_Stack.GetSize();
}

///////////////////////////////////////////////////////////////////////////////

EGFragment* EGFragmentExec::GetTop(void)
{
  EGFragment *pFragment = (EGFragment*)m_Stack.GetTail();
	return pFragment;
}

///////////////////////////////////////////////////////////////////////////////

EGFragment* EGFragmentExec::FindByContainer(const EGObject *pContainer)
{
POSITION Pos = nullptr;
EGFragment *pFragment;

  for(pFragment = (EGFragment*)m_Attached.GetTail(Pos); pFragment != nullptr; pFragment = (EGFragment*)m_Attached.GetPrev(Pos)){
		if(pFragment->m_pContainer == pContainer) return pFragment;
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

EGFragment* EGFragmentExec::GetParentFragment(void)
{
	return m_pParent;
}

///////////////////////////////////////////////////////////////////////////////

EGFragment* EGFragmentExec::AttachFragment(EGFragment *pFragment,	EGObject *pContainer)
{
	EG_ASSERT(pFragment);
	EG_ASSERT(pFragment->m_pManager == nullptr);
  m_Attached.AddTail(pFragment);
	pFragment->m_pManager = this;
	pFragment->m_pContainer = pContainer;
	pFragment->m_Managed = true;
	pFragment->Attached();
	return pFragment;
}

#endif 
