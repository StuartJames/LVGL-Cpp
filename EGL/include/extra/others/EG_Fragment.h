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

#pragma once

#include "EG_IntrnlConfig.h"

#if EG_USE_FRAGMENT

#include "core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////

class EGFragment;

///////////////////////////////////////////////////////////////////////////////

class EGFragmentExec
{
public:
	                    EGFragmentExec(void){};
	                    EGFragmentExec(EGFragment *pParent);
	                    ~EGFragmentExec(void);
  void                CreateObj(void);
  void                DeleteObj(void);
  void                Add(EGFragment *pFragment, EGObject *pContainer);
  void                Remove(EGFragment *pFragment);
  void                Push(EGFragment *pFragment, EGObject *pContainer);
  bool                Pop(void);
  void                Replace(EGFragment *pFragment, EGObject *pContainer);
  size_t              GetStackSize(void);
  EGFragment*         GetTop(void);
  EGFragment*         FindByContainer(const EGObject *pContainer);
  EGFragment*         GetParentFragment(void);

  static bool         SendEvent(EGFragmentExec *pManager, int Code, void *pExtData);

private:
	EGFragment*         AttachFragment(EGFragment *pFragment,	EGObject *pContainer);

	EGFragment          *m_pParent;
	EGList              m_Attached;      // Linked list to store attached fragments
	EGList              m_Stack;         // Linked list to store fragments in stack
};

///////////////////////////////////////////////////////////////////////////////

class EGFragment
{
public:
                            EGFragment(void *pArgs);
  virtual                   ~EGFragment(void);
  EGObject*                 Create(EGObject *pContainer);
  void                      Destroy(void);
  void                      Recreate(void);
  EGFragmentExec*           GetManager(void);
  const EGObject*           GetContainer(void);
  EGFragment*               GetParent(void);

  virtual void              Attached(void){}; // Fragment attached to manager
  virtual void              Detached(void){};	// Fragment detached from manager
  virtual EGObject*         CreateContent(EGObject *pContainer){ return nullptr; };
  virtual void              ContentCreated(EGObject *pObj){};
  virtual void              DeletingContent(EGObject *pObj){};// Called before objects in the fragment will be deleted.
  virtual void              ContentDeleted(EGObject *pObj){};   // Called when the object created by fragment received `EG_EVENT_DELETE` event

  static bool               EventCB(int Code, void *pExtData){ return false; };
  static void               DeleteAssertionCB(EGEvent *event);


	bool                      m_Managed;	        // this fragment is managed.
  EGFragmentExec            *m_pManager;
	EGFragmentExec            *m_pChildManager;
	EGObject                  *m_pContainer;	    // Container object the fragment adding view to
	bool                      m_ContentCreated;       // true between `create_obj_cb` and `obj_deleted_cb`
	bool                      m_DestroyingContent;	  // true before `lv_fragment_del_obj` is called. Don't touch any object if this is true
	bool                      m_InStack;          // true if this fragment is in navigation stack that can be popped

private:
	EGObject                  *m_pContent;

};

///////////////////////////////////////////////////////////////////////////////

#endif
