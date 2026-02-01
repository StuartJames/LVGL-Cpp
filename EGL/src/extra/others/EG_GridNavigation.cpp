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

#include "extra/others/EG_GridNavigation.h"
#if EG_USE_GRIDNAV

#include "misc/EG_Assert.h"
#include "misc/EG_Math.h"
#include "core/EG_InputDevice.h"

///////////////////////////////////////////////////////////////////////////////

void EGGridNav::Install(EGObject *pObj, EG_GridNavControl_e m_Control)
{
	Remove(pObj); // Be sure not to add gridnav twice
  EGGridNav *pGridNav = new EGGridNav;
	pGridNav->m_Control = m_Control;
	pGridNav->m_pFocused = NULL;
	EGEvent::AddEventCB(pObj, EventCB, EG_EVENT_ALL, pGridNav);
	pObj->ClearFlag(EG_OBJ_FLAG_SCROLL_WITH_ARROW);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridNav::Remove(EGObject *pObj)
{
	EGGridNav *pGridNav = (EGGridNav*)EGEvent::GetEventExtParam(pObj, EventCB);
  if(pGridNav == nullptr) return; // no gridnav on this object 
  delete pGridNav;
	EGEvent::RemoveEventCB(pObj, EventCB);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridNav::SetFocused(EGObject *pObj, EGObject *pToFocus, EG_AnimateEnable_e Animate)
{
	EG_ASSERT_NULL(pToFocus);
	EGGridNav *pGridNav = (EGGridNav*)EGEvent::GetEventExtParam(pObj, EventCB);
	if(pGridNav == nullptr) {
		EG_LOG_WARN("No gridnav container found");
		return;
	}
	if(pGridNav->IsFocusable(pToFocus) == false) {
		EG_LOG_WARN("The object to focus is not focusable");
		return;
	}
	pGridNav->m_pFocused->ClearState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
	pToFocus->AddState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
	pToFocus->ScrollToView(Animate);
	pGridNav->m_pFocused = pToFocus;
}

///////////////////////////////////////////////////////////////////////////////

void EGGridNav::EventCB(EGEvent *pEvent)
{
	EGObject *pObj = pEvent->GetCurrentTarget();
	EGGridNav *pGridNav = (EGGridNav*)pEvent->GetExtParam();
	EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_KEY:{
      uint32_t ChildCount = pObj->GetChildCount();
      if(ChildCount == 0) return;
      if(pGridNav->m_pFocused == nullptr) pGridNav->m_pFocused = pGridNav->FindFirstFocusable(pObj);
      if(pGridNav->m_pFocused == nullptr) return;
      uint32_t Key = pEvent->GetKey();
      EGObject *pGuess = nullptr;
      switch(Key){
        case EG_KEY_RIGHT: {
          if((pGridNav->m_Control & EG_GRIDNAV_CTRL_SCROLL_FIRST) && pGridNav->m_pFocused->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE) &&
              pGridNav->m_pFocused->GetScrollRight() > 0) {
            EG_Coord_t d = pGridNav->m_pFocused->GetWidth() / 4;
            if(d <= 0) d = 1;
            pGridNav->m_pFocused->ScrollByBounded(-d, 0, EG_ANIM_ON);
          }
          else {
            pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_RIGHT);
            if(pGuess == NULL) {
              if(pGridNav->m_Control & EG_GRIDNAV_CTRL_ROLLOVER) {
                pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_NEXT_ROW_FIRST_ITEM);
                if(pGuess == NULL) pGuess = pGridNav->FindFirstFocusable(pObj);
              }
              else ((EGGroup*)pObj->GetGroup())->FocusNext();
            }
          }
          break;
        }
        case EG_KEY_LEFT: {
          if((pGridNav->m_Control & EG_GRIDNAV_CTRL_SCROLL_FIRST) && pGridNav->m_pFocused->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE) &&
              pGridNav->m_pFocused->GetScrollLeft() > 0) {
            EG_Coord_t d = pGridNav->m_pFocused->GetWidth() / 4;
            if(d <= 0) d = 1;
            pGridNav->m_pFocused->ScrollByBounded(d, 0, EG_ANIM_ON);
          }
          else {
            pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_LEFT);
            if(pGuess == NULL) {
              if(pGridNav->m_Control & EG_GRIDNAV_CTRL_ROLLOVER) {
                pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_PREV_ROW_LAST_ITEM);
                if(pGuess == NULL) pGuess = pGridNav->FindLastFocusable(pObj);
              }
              else ((EGGroup*)pObj->GetGroup())->FocusPrevious();
            }
          }
          break;
        }
        case EG_KEY_DOWN: {
          if((pGridNav->m_Control & EG_GRIDNAV_CTRL_SCROLL_FIRST) && pGridNav->m_pFocused->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE) &&
              pGridNav->m_pFocused->GetScrollBottom() > 0) {
            EG_Coord_t d = pGridNav->m_pFocused->GetHeight() / 4;
            if(d <= 0) d = 1;
            pGridNav->m_pFocused->ScrollByBounded(0, -d, EG_ANIM_ON);
          }
          else {
            pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_BOTTOM);
            if(pGuess == NULL) {
              if(pGridNav->m_Control & EG_GRIDNAV_CTRL_ROLLOVER) {
                pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_FIRST_ROW);
              }
              else {
                ((EGGroup*)pObj->GetGroup())->FocusNext();
              }
            }
          }
          break;
        }
        case EG_KEY_UP: {
          if((pGridNav->m_Control & EG_GRIDNAV_CTRL_SCROLL_FIRST) && pGridNav->m_pFocused->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE) &&
              pGridNav->m_pFocused->GetScrollTop() > 0) {
            EG_Coord_t d = pGridNav->m_pFocused->GetHeight() / 4;
            if(d <= 0) d = 1;
            pGridNav->m_pFocused->ScrollByBounded(0, d, EG_ANIM_ON);
          }
          else {
            pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_TOP);
            if(pGuess == NULL) {
              if(pGridNav->m_Control & EG_GRIDNAV_CTRL_ROLLOVER) {
                pGuess = pGridNav->FindChild(pObj, pGridNav->m_pFocused, FIND_LAST_ROW);
              }
              else {
                ((EGGroup*)pObj->GetGroup())->FocusPrevious();
              }
            }
          }
          break;
        }
        default:{
          if(((EGGroup*)pObj->GetGroup())->GetFocused() == pObj) {
            EGEvent::EventSend(pGridNav->m_pFocused, EG_EVENT_KEY, &Key);
          }
          break;
        }
      }
      if(pGuess && pGuess != pGridNav->m_pFocused) {
        pGridNav->m_pFocused->ClearState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
        pGuess->AddState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
        pGuess->ScrollToView(EG_ANIM_ON);
        pGridNav->m_pFocused = pGuess;
      }
      break;
    }
    case EG_EVENT_FOCUSED: {
      if(pGridNav->m_pFocused == NULL) pGridNav->m_pFocused = pGridNav->FindFirstFocusable(pObj);
      if(pGridNav->m_pFocused) {
        pGridNav->m_pFocused->AddState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
        pGridNav->m_pFocused->ClearState(EG_STATE_PRESSED); /*Be sure the focuses pObj is not stuck in pressed state*/
        pGridNav->m_pFocused->ScrollToView(EG_ANIM_OFF);
      }
      break;
    }
    case EG_EVENT_DEFOCUSED: {
      if(pGridNav->m_pFocused) {
        pGridNav->m_pFocused->ClearState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
      }
      break;
    }
    case EG_EVENT_CHILD_CREATED: {
      EGObject *pTarget = pEvent->GetTarget();
      if(pTarget->GetParent() == pObj) {
        if(pGridNav->m_pFocused == NULL) {
          pGridNav->m_pFocused = pTarget;
          if(pObj->HasState(EG_STATE_FOCUSED)) {
            pTarget->AddState(EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
            pTarget->ScrollToView(EG_ANIM_OFF);
          }
        }
      }
      break;
    }
    case EG_EVENT_CHILD_DELETED: {
      /*vThis event bubble, so be sure this object's child was deleted.
          *As we don't know which object was deleted we can't make the next focused.
          *So make the first object focused*/
      EGObject *pTarget = pEvent->GetTarget();
      if(pTarget == pObj) {
        pGridNav->m_pFocused = pGridNav->FindFirstFocusable(pObj);
      }
      break;
    }
    case EG_EVENT_DELETE: {
      pGridNav->Remove(pObj);
      break;
    }
    case EG_EVENT_PRESSED:
    case EG_EVENT_PRESSING:
    case EG_EVENT_PRESS_LOST:
    case EG_EVENT_LONG_PRESSED:
    case EG_EVENT_LONG_PRESSED_REPEAT:
    case EG_EVENT_CLICKED:
    case EG_EVENT_RELEASED: {
      if(((EGGroup*)pObj->GetGroup())->GetFocused() == pObj) {  
        EG_InDeviceType_e Type = EGInputDevice::GetActive()->GetType();  // Forward press/release related event too
        if(Type == EG_INDEV_TYPE_ENCODER || Type == EG_INDEV_TYPE_KEYPAD) {
          EGEvent::EventSend(pGridNav->m_pFocused, Code, EGInputDevice::GetActive());
        }
      }
      break;
    }
    default: break;
  }
}

///////////////////////////////////////////////////////////////////////////////

EGObject* EGGridNav::FindChild(EGObject *pObj, EGObject *pStartChild, EG_FindMode_e Mode)
{
	EG_Coord_t StartX = GetCenterX(pStartChild);
	EG_Coord_t StartY = GetCenterY(pStartChild);
	uint32_t Count = pObj->GetChildCount();
	EGObject *pGuess = nullptr;
	EG_Coord_t GuessErrorX = EG_COORD_MAX;
	EG_Coord_t GuessErrorY = EG_COORD_MAX;
	EG_Coord_t HalfHeight = pStartChild->GetHeight() / 2;
	EG_Coord_t FullHeight = pObj->GetHeight() + pObj->GetScrollTop() + pObj->GetScrollBottom();
	uint32_t i;
	for(i = 0; i < Count; i++) {
		EGObject *pChild = pObj->GetChild(i);
		if(pChild == pStartChild) continue;
		if(IsFocusable(pChild) == false) continue;
		EG_Coord_t ErrorX = 0;
		EG_Coord_t ErrorY = 0;
		switch(Mode) {
			case FIND_LEFT:
				ErrorX = GetCenterX(pChild) - StartX;
				ErrorY = GetCenterY(pChild) - StartY;
				if(ErrorX >= 0) continue;             /*It's on the right*/
				if(EG_ABS(ErrorY) > HalfHeight) continue; /*Too far*/
				break;
			case FIND_RIGHT:
				ErrorX = GetCenterX(pChild) - StartX;
				ErrorY = GetCenterY(pChild) - StartY;
				if(ErrorX <= 0) continue;             /*It's on the left*/
				if(EG_ABS(ErrorY) > HalfHeight) continue; /*Too far*/
				break;
			case FIND_TOP:
				ErrorX = GetCenterX(pChild) - StartX;
				ErrorY = GetCenterY(pChild) - StartY;
				if(ErrorY >= 0) continue; /*It's on the bottom*/
				break;
			case FIND_BOTTOM:
				ErrorX = GetCenterX(pChild) - StartX;
				ErrorY = GetCenterY(pChild) - StartY;
				if(ErrorY <= 0) continue; /*It's on the top*/
				break;
			case FIND_NEXT_ROW_FIRST_ITEM:
				ErrorY = GetCenterY(pChild) - StartY;
				if(ErrorY <= 0) continue; /*It's on the top*/
				ErrorX = pChild->GetX();
				break;
			case FIND_PREV_ROW_LAST_ITEM:
				ErrorY = GetCenterY(pChild) - StartY;
				if(ErrorY >= 0) continue; /*It's on the bottom*/
				ErrorX = pObj->m_Rect.GetX2() - pChild->m_Rect.GetX2();
				break;
			case FIND_FIRST_ROW:
				ErrorX = GetCenterX(pChild) - StartX;
				ErrorY = pChild->GetY();
				break;
			case FIND_LAST_ROW:
				ErrorX = GetCenterX(pChild) - StartX;
				ErrorY = FullHeight - pChild->GetY();
		}
		if(pGuess == NULL || (ErrorY * ErrorY + ErrorX * ErrorX < GuessErrorY * GuessErrorY + GuessErrorX * GuessErrorX)) {
			pGuess = pChild;
			GuessErrorX = ErrorX;
			GuessErrorY = ErrorY;
		}
	}
	return pGuess;
}

///////////////////////////////////////////////////////////////////////////////

EGObject* EGGridNav::FindFirstFocusable(EGObject *pObj)
{
	uint32_t Count = pObj->GetChildCount();
	for(int32_t i = 0; i < Count; i++) {
		EGObject *pChild = pObj->GetChild(i);
		if(IsFocusable(pChild)) return pChild;
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

EGObject* EGGridNav::FindLastFocusable(EGObject *pObj)
{
	uint32_t Count = pObj->GetChildCount();
	for(int32_t i = Count - 1; i >= 0; i--) {
		EGObject *pChild = pObj->GetChild(i);
		if(IsFocusable(pChild)) return pChild;
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

bool EGGridNav::IsFocusable(EGObject *pObj)
{
	if(pObj->HasFlagSet(EG_OBJ_FLAG_HIDDEN)) return false;
	if(pObj->HasFlagSet(EG_OBJ_FLAG_CLICKABLE | EG_OBJ_FLAG_CLICK_FOCUSABLE)) return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGGridNav::GetCenterX(EGObject *pObj)
{
	return pObj->m_Rect.GetX1() + pObj->m_Rect.GetWidth() / 2;
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGGridNav::GetCenterY(EGObject *pObj)
{
	return pObj->m_Rect.GetY1() + pObj->m_Rect.GetHeight() / 2;
}

///////////////////////////////////////////////////////////////////////////////

#endif 
