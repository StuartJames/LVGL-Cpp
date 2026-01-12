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

#include "core/EG_InputDevice.h"
#include "core/EG_Display.h"
#include "core/EG_Object.h"
#include "core/EG_Group.h"
#include "core/EG_Refresh.h"

#include "hal/EG_HALTick.h"
#include "misc/EG_Timer.h"
#include "misc/EG_Math.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#if EG_INDEV_DEF_SCROLL_THROW <= 0
#warning "EG_INDEV_DRAG_THROW must be greater than 0"
#endif

#if EG_LOG_TRACE_INDEV
#define INDEV_TRACE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define INDEV_TRACE(...)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDevice *EGInputDevice::m_pActiveDevice = nullptr;
EGObject      *EGInputDevice::m_pActiveObj = nullptr;
EGList         EGInputDevice::m_InDeviceList;        

///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDevice::EGInputDevice() :
  m_pDriver(nullptr),
  m_pReadTimer(nullptr),
  m_pCursor(nullptr),
  m_pGroup(nullptr)
{
  m_InDeviceList.Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDevice::~EGInputDevice()
{
	EG_ASSERT_NULL(m_pDriver);
	EG_ASSERT_NULL(m_pDriver->m_pReadTimer);
	EGTimer::Delete(m_pDriver->m_pReadTimer);	// Clean up the read timer first
  POSITION Pos = m_InDeviceList.Find(this);
	if(Pos != nullptr) m_InDeviceList.RemoveAt(Pos);	// Remove the input device from the list
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ReadTimerCB(EGTimer *pTimer)
{
	((EGInputDevice*)pTimer->m_pParam)->ReadProcess();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ReadProcess(void)
{
bool ContinueReading = false;

	INDEV_TRACE("begin");
	if(m_pDriver->m_pDisplay == nullptr) return; // Not assigned to any displays
	if(m_Process.Disabled || m_pDriver->m_pDisplay->m_pPrevoiusScreen != nullptr) return; // Input Disabled or screen animation active
	ResetQueryHandler();	// Handle reset query before processing the input
  m_pActiveDevice = this;
	EG_InputData_t Data;
	do{
		Read(&Data);		// Read the data
		ContinueReading = Data.ContinueReading;
		ResetQueryHandler();		// The active object might be deleted even in the read function
		m_pActiveObj = nullptr;
		m_Process.State = Data.State;
		if(Data.State == EG_INDEV_STATE_PRESSED) m_pDriver->m_pDisplay->m_LastActivityTime = EG_GetTick();	// Save the last activity time
		else if(m_pDriver->m_Type == EG_INDEV_TYPE_ENCODER && Data.EncoderSteps) {
			m_pDriver->m_pDisplay->m_LastActivityTime = EG_GetTick();
		}
    switch(m_pDriver->m_Type){
		  case EG_INDEV_TYPE_POINTER: {
        PointerProcess(&Data);
        break;
      }
      case EG_INDEV_TYPE_KEYPAD: {
        KeypadProcess(&Data);
        break;
      }
      case EG_INDEV_TYPE_ENCODER: {
        EncoderProcess(&Data);
        break;
      }
      case EG_INDEV_TYPE_BUTTON: {
        ButtonProcess(&Data);
        break;
      }
      default:
        break;
    }
		ResetQueryHandler();		// Handle reset query if it happened in during processing
  }
  while(ContinueReading);
  m_pActiveDevice = nullptr;
	m_pActiveObj = nullptr;
	INDEV_TRACE("finished");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::Enable(EGInputDevice *pDevice, bool Flag/* =true*/)
{
uint8_t enable = Flag ? 0 : 1;

  if(pDevice != nullptr) pDevice->m_Process.Disabled = enable;
	else {
		EGInputDevice *pInput = GetNext(nullptr);
		while(pInput) {
			pInput->m_Process.Disabled = enable;
			pInput = GetNext(pInput);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDevice* EGInputDevice::GetActive(void)
{
	return m_pActiveDevice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_InDeviceType_e EGInputDevice::GetType(void)
{
	return m_pDriver->m_Type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::Reset(EGInputDevice *pInputDev, EGObject *pObj)
{
	if(pInputDev) {
		pInputDev->m_Process.ResetQuery = 1;
		if(m_pActiveDevice == pInputDev) m_pActiveObj = nullptr;
		if(pInputDev->m_pDriver->m_Type == EG_INDEV_TYPE_POINTER || pInputDev->m_pDriver->m_Type == EG_INDEV_TYPE_KEYPAD) {
			if(pObj == nullptr || pInputDev->m_Process.Pointer.pLastPressedObj == pObj) {
				pInputDev->m_Process.Pointer.pLastPressedObj = nullptr;
			}
			if(pObj == nullptr || pInputDev->m_Process.Pointer.pActiveObj == pObj) {
				pInputDev->m_Process.Pointer.pActiveObj = nullptr;
			}
			if(pObj == nullptr || pInputDev->m_Process.Pointer.pLastObj == pObj) {
				pInputDev->m_Process.Pointer.pLastObj = nullptr;
			}
		}
	}
	else {
		EGInputDevice *pInput = GetNext(nullptr);
		while(pInput) {
			pInput->m_Process.ResetQuery = 1;
			if(pInput->m_pDriver->m_Type == EG_INDEV_TYPE_POINTER || pInput->m_pDriver->m_Type == EG_INDEV_TYPE_KEYPAD) {
				if(pObj == nullptr || pInput->m_Process.Pointer.pLastPressedObj == pObj) {
					pInput->m_Process.Pointer.pLastPressedObj = nullptr;
				}
				if(pObj == nullptr || pInput->m_Process.Pointer.pActiveObj == pObj) {
					pInput->m_Process.Pointer.pActiveObj = nullptr;
				}
				if(pObj == nullptr || pInput->m_Process.Pointer.pLastObj == pObj) {
					pInput->m_Process.Pointer.pLastObj = nullptr;
				}
			}
			pInput = GetNext(pInput);
		}
		m_pActiveObj = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ResetLongPress(void)
{
	m_Process.LongPressSent = 0;
	m_Process.LongPressRepeatTimestamp = EG_GetTick();
	m_Process.PressedTimestamp = EG_GetTick();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::SetCursor(EGObject *pObj)
{
	if(m_pDriver->m_Type != EG_INDEV_TYPE_POINTER) return;
	m_pCursor = pObj;
	m_pCursor->SetParent(EGDisplay::GetSystemLayer(m_pDriver->m_pDisplay));
	m_pCursor->SetPosition(m_Process.Pointer.ActivePoint.m_X, m_Process.Pointer.ActivePoint.m_Y);
	m_pCursor->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	m_pCursor->AddFlag(EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_FLOATING);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::SetGroup(EGGroup *pGroup)
{
	if(m_pDriver->m_Type == EG_INDEV_TYPE_KEYPAD || m_pDriver->m_Type == EG_INDEV_TYPE_ENCODER) {
		m_pGroup = pGroup;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::SetButtonPoints(const EGPoint Points[])
{
	if(m_pDriver->m_Type == EG_INDEV_TYPE_BUTTON){
		m_pButtonPoints = Points;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::GetPoint(EGPoint *pPoint)
{
	if(pPoint == nullptr) return;
	if(m_pDriver->m_Type != EG_INDEV_TYPE_POINTER && m_pDriver->m_Type != EG_INDEV_TYPE_BUTTON) {
		pPoint->m_X = -1;
		pPoint->m_Y = -1;
	}
	else {
		pPoint->m_X = m_Process.Pointer.ActivePoint.m_X;
		pPoint->m_Y = m_Process.Pointer.ActivePoint.m_Y;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_DirType_e EGInputDevice::GetGestureDirection(void)
{
	return m_Process.Pointer.GestureDirection;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t EGInputDevice::GetKey(void)
{
	if(m_pDriver->m_Type != EG_INDEV_TYPE_KEYPAD)	return 0;
	else return m_Process.Keypad.LastKey;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_DirType_e EGInputDevice::GetScrollDirection(void)
{
	if(m_pDriver->m_Type != EG_INDEV_TYPE_POINTER && m_pDriver->m_Type != EG_INDEV_TYPE_BUTTON) return EG_DIR_NONE;
	return m_Process.Pointer.ScrollDirection;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGObject* EGInputDevice::GetScrollObj(void)
{
	if(m_pDriver->m_Type != EG_INDEV_TYPE_POINTER && m_pDriver->m_Type != EG_INDEV_TYPE_BUTTON) return nullptr;
	return m_Process.Pointer.pScrollObj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::GetVector(EGPoint *pPoint)
{
	pPoint->m_X = 0;
	pPoint->m_Y = 0;
	if(m_pDriver->m_Type == EG_INDEV_TYPE_POINTER || m_pDriver->m_Type == EG_INDEV_TYPE_BUTTON) {
		pPoint->m_X = m_Process.Pointer.Vector.m_X;
		pPoint->m_Y = m_Process.Pointer.Vector.m_Y;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::WaitRelease(void)
{
	m_Process.WaitUntilRelease = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGObject* EGInputDevice::GetActiveObj(void)
{
	return m_pActiveObj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGTimer* EGInputDevice::GetReadTimer(EGInputDevice *pInDevice)
{
	if(!pInDevice) {
		EG_LOG_WARN("GetReadTimer: pInDevice was NULL");
		return nullptr;
	}
	return pInDevice->m_pReadTimer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGObject* EGInputDevice::SearchObject(EGObject *pObj, EGPoint *pPoint)
{
EGObject *pFound = nullptr;

	// If this obj is hidden the children are hidden too so return immediately
	if(pObj->HasFlagSet(EG_OBJ_FLAG_HIDDEN)) return nullptr;
	EGPoint Trans = *pPoint;
	pObj->TransformPoint(&Trans, false, true);
	bool WasHit = pObj->HitTest(&Trans);
	// If the point is on this object or has overflow visible check its children too
	if(pObj->m_Rect.IsPointIn(&Trans, 0) || pObj->HasFlagSet(EG_OBJ_FLAG_OVERFLOW_VISIBLE)) {
		uint32_t ChildCount = pObj->GetChildCount();
		for(int32_t i = ChildCount - 1; i >= 0; i--) {		// If a child matches use it
			EGObject *pChild = pObj->m_pAttributes->ppChildren[i];
			pFound = SearchObject(pChild, &Trans);
			if(pFound) return pFound;
		}
	}
	return (WasHit) ? pObj : nullptr;	// If this obj's hittest was ok use it
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::PointerProcess(EG_InputData_t *pInData)
{
	EGDisplay *m_pDisplay = m_pDriver->m_pDisplay;
	m_Process.Pointer.LastRawPoint = pInData->Point; 	// Save the raw points so they can be used again
	if(m_pDisplay->m_pDriver->m_Rotated == EG_DISP_ROT_180 || m_pDisplay->m_pDriver->m_Rotated == EG_DISP_ROT_270) {
		pInData->Point.m_X = m_pDisplay->m_pDriver->m_HorizontalRes - pInData->Point.m_X - 1;
		pInData->Point.m_Y = m_pDisplay->m_pDriver->m_VerticalRes - pInData->Point.m_Y - 1;
	}
	if(m_pDisplay->m_pDriver->m_Rotated == EG_DISP_ROT_90 || m_pDisplay->m_pDriver->m_Rotated == EG_DISP_ROT_270) {
    EG_Coord_t TempY = pInData->Point.m_Y;
		pInData->Point.m_Y = pInData->Point.m_X;
		pInData->Point.m_X = m_pDisplay->m_pDriver->m_VerticalRes - TempY - 1;
	}
	// Simple sanity check
	if(pInData->Point.m_X < 0) {
		EG_LOG_WARN("X is %d which is smaller than zero", (int)pInData->Point.m_X);
	}
	if(pInData->Point.m_X >= m_pDisplay->GetHorizontalRes()) {
		EG_LOG_WARN("X is %d which is greater than hor. res", (int)pInData->Point.m_X);
	}
	if(pInData->Point.m_Y < 0) {
		EG_LOG_WARN("Y is %d which is smaller than zero", (int)pInData->Point.m_Y);
	}
	if(pInData->Point.m_Y >= m_pDisplay->GetVerticalRes()) {
		EG_LOG_WARN("Y is %d which is greater than ver. res", (int)pInData->Point.m_Y);
	}
	// Move the cursor if set and moved
	if(m_pCursor != nullptr && (m_Process.Pointer.LastPoint.m_X != pInData->Point.m_X || m_Process.Pointer.LastPoint.m_Y != pInData->Point.m_Y)) {
		m_pCursor->SetPosition(pInData->Point.m_X, pInData->Point.m_Y);
	}
	m_Process.Pointer.ActivePoint = pInData->Point;
	if(m_Process.State == EG_INDEV_STATE_PRESSED) {
		PressProcess(&m_Process);
	}
	else {
		ReleaseProcess(&m_Process);
	}
	m_Process.Pointer.LastPoint = m_Process.Pointer.ActivePoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::KeypadProcess(EG_InputData_t *pInData)
{
	if(pInData->State == EG_INDEV_STATE_PRESSED && m_Process.WaitUntilRelease) return;
	if(m_Process.WaitUntilRelease) {
		m_Process.WaitUntilRelease = 0;
		m_Process.PressedTimestamp = 0;
		m_Process.LongPressSent = 0;
		m_Process.Keypad.LastState = EG_INDEV_STATE_RELEASED; // To skip the processing of release
	}
	EGGroup *pGroup = m_pGroup;
	if(pGroup == nullptr) return;
	m_pActiveObj = pGroup->GetFocused();
	if(m_pActiveObj == nullptr) return;
	bool Disabled = m_pActiveObj->HasState(EG_STATE_DISABLED);
	uint32_t PrevKey = m_Process.Keypad.LastKey;	// Save the last key to compare it with the current latter on RELEASE
	m_Process.Keypad.LastKey = pInData->Key;	// Save the last key. 
	// Save the previous State so we can detect State changes below and also set the last State now
	uint32_t PrevState = m_Process.Keypad.LastState;
	m_Process.Keypad.LastState = pInData->State;
	// Key press happened
	if(pInData->State == EG_INDEV_STATE_PRESSED && PrevState == EG_INDEV_STATE_RELEASED) {
		EG_LOG_INFO("%" EG_PRIu32 " key is pressed", pInData->Key);
		m_Process.PressedTimestamp = EG_GetTick();
		// Move the focus on NEXT
		if(pInData->Key == EG_KEY_NEXT) {
			pGroup->SetEditing(false); // Editing is not used by KEYPAD is be sure it is Disabled
			pGroup->FocusNext();
			if(ResetCheck(&m_Process)) return;
		}
		// Move the focus on PREV
		else if(pInData->Key == EG_KEY_PREV) {
			pGroup->SetEditing(false); // Editing is not used by KEYPAD is be sure it is Disabled
			pGroup->FocusPrevious();
			if(ResetCheck(&m_Process)) return;
		}
		else if(!Disabled) {
			// Simulate a press on the object if ENTER was pressed
			if(pInData->Key == EG_KEY_ENTER) {
				// Send the ENTER as a normal KEY
				pGroup->SendData(EG_KEY_ENTER);
				if(ResetCheck(&m_Process)) return;
				if(!Disabled) EGEvent::EventSend(m_pActiveObj, EG_EVENT_PRESSED, this);
				if(ResetCheck(&m_Process)) return;
			}
			else if(pInData->Key == EG_KEY_ESC) {
				// Send the ESC as a normal KEY
				pGroup->SendData(EG_KEY_ESC);
				if(ResetCheck(&m_Process)) return;
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_CANCEL, this);
				if(ResetCheck(&m_Process)) return;
			}
			// Just send other keys to the object (e.g. 'A' or `EG_GROUP_KEY_RIGHT`)
			else {
				pGroup->SendData(pInData->Key);
				if(ResetCheck(&m_Process)) return;
			}
		}
	}
	// Pressing
	else if(!Disabled && pInData->State == EG_INDEV_STATE_PRESSED && PrevState == EG_INDEV_STATE_PRESSED) {
		if(pInData->Key == EG_KEY_ENTER) {
			EGEvent::EventSend(m_pActiveObj, EG_EVENT_PRESSING, this);
			if(ResetCheck(&m_Process)) return;
		}

		// Long press time has elapsed?
		if(m_Process.LongPressSent == 0 && EG_TickElapse(m_Process.PressedTimestamp) > m_pDriver->m_LongPressTime) {
			m_Process.LongPressSent = 1;
			if(pInData->Key == EG_KEY_ENTER) {
				m_Process.LongPressRepeatTimestamp = EG_GetTick();
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_LONG_PRESSED, this);
				if(ResetCheck(&m_Process)) return;
			}
		}
		// Long press repeated time has elapsed?
		else if(m_Process.LongPressSent != 0 &&	EG_TickElapse(m_Process.LongPressRepeatTimestamp) > m_pDriver->m_LongPressRepeatTime) {
			m_Process.LongPressRepeatTimestamp = EG_GetTick();

			// Send LONG_PRESS_REP on ENTER
			if(pInData->Key == EG_KEY_ENTER) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_LONG_PRESSED_REPEAT, this);
				if(ResetCheck(&m_Process)) return;
			}
			// Move the focus on NEXT again
			else if(pInData->Key == EG_KEY_NEXT) {
				pGroup->SetEditing(false); // Editing is not used by KEYPAD is be sure it is Disabled
				pGroup->FocusPrevious();
				if(ResetCheck(&m_Process)) return;
			}
			// Move the focus on PREV again
			else if(pInData->Key == EG_KEY_PREV) {
				pGroup->SetEditing(false); // Editing is not used by KEYPAD is be sure it is Disabled
				pGroup->FocusPrevious();
				if(ResetCheck(&m_Process)) return;
			}
			// Just send other keys again to the object (e.g. 'A' or `EG_GROUP_KEY_RIGHT)
			else {
				pGroup->SendData(pInData->Key);
				if(ResetCheck(&m_Process)) return;
			}
		}
	}
	// Release happened
	else if(!Disabled && pInData->State == EG_INDEV_STATE_RELEASED && PrevState == EG_INDEV_STATE_PRESSED) {
		EG_LOG_INFO("%" EG_PRIu32 " key is released", pInData->Key);
		// The user might clear the key when it was released. Always release the pressed key
		pInData->Key = PrevKey;
		if(pInData->Key == EG_KEY_ENTER) {
			EGEvent::EventSend(m_pActiveObj, EG_EVENT_RELEASED, this);
			if(ResetCheck(&m_Process)) return;

			if(m_Process.LongPressSent == 0) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_SHORT_CLICKED, this);
				if(ResetCheck(&m_Process)) return;
			}

			EGEvent::EventSend(m_pActiveObj, EG_EVENT_CLICKED, this);
			if(ResetCheck(&m_Process)) return;
		}
		m_Process.PressedTimestamp = 0;
		m_Process.LongPressSent = 0;
	}
	m_pActiveObj = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::EncoderProcess(EG_InputData_t *pInData)
{
	if(pInData->State == EG_INDEV_STATE_PRESSED && m_Process.WaitUntilRelease) return;

	if(m_Process.WaitUntilRelease) {
		m_Process.WaitUntilRelease = 0;
		m_Process.PressedTimestamp = 0;
		m_Process.LongPressSent = 0;
		m_Process.Keypad.LastState = EG_INDEV_STATE_RELEASED; // To skip the processing of release
	}
	// Save the last keys before anything else. They need to be already saved if the function returns for any reason
	EG_InDeviceState_e LastState = m_Process.Keypad.LastState;
	m_Process.Keypad.LastState = pInData->State;
	m_Process.Keypad.LastKey = pInData->Key;
	EGGroup *pGroup = m_pGroup;
	if(pGroup == nullptr) return;
	m_pActiveObj = pGroup->GetFocused();
	if(m_pActiveObj == nullptr) return;
	if(pInData->State != EG_INDEV_STATE_RELEASED) {	// Process the steps they are valid only with released button
		pInData->EncoderSteps = 0;
	}
	// Refresh the focused object. It might change due to lv_group_focus_prev/next
	m_pActiveObj = pGroup->GetFocused();
	if(m_pActiveObj == nullptr) return;
	if(pInData->State == EG_INDEV_STATE_PRESSED && LastState == EG_INDEV_STATE_RELEASED) {	// Button press happened
		EG_LOG_INFO("pressed");
		m_Process.PressedTimestamp = EG_GetTick();
		if(pInData->Key == EG_KEY_ENTER) {
			bool editable_or_scrollable = m_pActiveObj->IsEditable() ||	m_pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE);
			if(pGroup->GetEditing() == true || editable_or_scrollable == false) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_PRESSED, this);
				if(ResetCheck(&m_Process)) return;
			}
		}
		else if(pInData->Key == EG_KEY_LEFT) {
			// emulate encoder left
			pInData->EncoderSteps--;
		}
		else if(pInData->Key == EG_KEY_RIGHT) {
			// emulate encoder right
			pInData->EncoderSteps++;
		}
		else if(pInData->Key == EG_KEY_ESC) {
			// Send the ESC as a normal KEY
			pGroup->SendData(EG_KEY_ESC);
			if(ResetCheck(&m_Process)) return;

			EGEvent::EventSend(m_pActiveObj, EG_EVENT_CANCEL, this);
			if(ResetCheck(&m_Process)) return;
		}
		// Just send other keys to the object (e.g. 'A' or `EG_GROUP_KEY_RIGHT`)
		else {
			pGroup->SendData(pInData->Key);
			if(ResetCheck(&m_Process)) return;
		}
	}
	// Pressing
	else if(pInData->State == EG_INDEV_STATE_PRESSED && LastState == EG_INDEV_STATE_PRESSED) {
		// Long press
		if(m_Process.LongPressSent == 0 && EG_TickElapse(m_Process.PressedTimestamp) > m_pDriver->m_LongPressTime) {
			m_Process.LongPressSent = 1;
			m_Process.LongPressRepeatTimestamp = EG_GetTick();

			if(pInData->Key == EG_KEY_ENTER) {
				bool editable_or_scrollable = m_pActiveObj->IsEditable() ||	m_pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE);

				// On enter long press toggle edit mode.
				if(editable_or_scrollable) {
					// Don't leave edit mode if there is only one object (nowhere to navigate)
					if(pGroup->GetObjectCount() > 1){
						EG_LOG_INFO("toggling edit mode");
						pGroup->SetEditing(pGroup->GetEditing() ? false : true); // Toggle edit mode on long press
						m_pActiveObj->ClearState(EG_STATE_PRESSED);             // Remove the pressed State manually
					}
				}
				// If not editable then just send a long press Call the ancestor's event handler
				else {
					EGEvent::EventSend(m_pActiveObj, EG_EVENT_LONG_PRESSED, this);
					if(ResetCheck(&m_Process)) return;
				}
			}

			m_Process.LongPressSent = 1;
		}
		// Long press repeated time has elapsed?
		else if(m_Process.LongPressSent != 0 && EG_TickElapse(m_Process.LongPressRepeatTimestamp) > m_pDriver->m_LongPressRepeatTime) {
			m_Process.LongPressRepeatTimestamp = EG_GetTick();

			if(pInData->Key == EG_KEY_ENTER) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_LONG_PRESSED_REPEAT, this);
				if(ResetCheck(&m_Process)) return;
			}
			else if(pInData->Key == EG_KEY_LEFT) {
				// emulate encoder left
				pInData->EncoderSteps--;
			}
			else if(pInData->Key == EG_KEY_RIGHT) {
				// emulate encoder right
				pInData->EncoderSteps++;
			}
			else {
				pGroup->SendData(pInData->Key);
				if(ResetCheck(&m_Process)) return;
			}
		}
	}
	// Release happened
	else if(pInData->State == EG_INDEV_STATE_RELEASED && LastState == EG_INDEV_STATE_PRESSED) {
		EG_LOG_INFO("released");

		if(pInData->Key == EG_KEY_ENTER) {
			bool editable_or_scrollable = m_pActiveObj->IsEditable() ||	m_pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE);

			// The button was released on a non-editable object. Just send enter
			if(editable_or_scrollable == false) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_RELEASED, this);
				if(ResetCheck(&m_Process)) return;

				if(m_Process.LongPressSent == 0) EGEvent::EventSend(m_pActiveObj, EG_EVENT_SHORT_CLICKED, this);
				if(ResetCheck(&m_Process)) return;

				EGEvent::EventSend(m_pActiveObj, EG_EVENT_CLICKED, this);
				if(ResetCheck(&m_Process)) return;
			}
			// An object is being edited and the button is released.
			else if(pGroup->GetEditing()) {
				// Ignore long pressed enter release because it comes from mode switch
				if(!m_Process.LongPressSent || pGroup->GetObjectCount() <= 1) {
					EGEvent::EventSend(m_pActiveObj, EG_EVENT_RELEASED, this);
					if(ResetCheck(&m_Process)) return;

					EGEvent::EventSend(m_pActiveObj, EG_EVENT_SHORT_CLICKED, this);
					if(ResetCheck(&m_Process)) return;

					EGEvent::EventSend(m_pActiveObj, EG_EVENT_CLICKED, this);
					if(ResetCheck(&m_Process)) return;

					pGroup->SendData(EG_KEY_ENTER);
					if(ResetCheck(&m_Process)) return;
				}
				else {
					m_pActiveObj->ClearState(EG_STATE_PRESSED); // Remove the pressed State manually
				}
			}
			// If the focused object is editable and now in navigate mode then on enter switch edit mode
			else if(!m_Process.LongPressSent) {
				EG_LOG_INFO("entering edit mode");
				pGroup->SetEditing(true); // Set edit mode
			}
		}

		m_Process.PressedTimestamp = 0;
		m_Process.LongPressSent = 0;
	}
	m_pActiveObj = nullptr;

	// if encoder steps or simulated steps via left/right keys
	if(pInData->EncoderSteps != 0) {
		// In edit mode send LEFT/RIGHT keys
		if(pGroup->GetEditing()) {
			EG_LOG_INFO("rotated by %+d (edit)", pInData->EncoderSteps);
			int32_t s;
			if(pInData->EncoderSteps < 0) {
				for(s = 0; s < -pInData->EncoderSteps; s++) {
					pGroup->SendData(EG_KEY_LEFT);
					if(ResetCheck(&m_Process)) return;
				}
			}
			else if(pInData->EncoderSteps > 0) {
				for(s = 0; s < pInData->EncoderSteps; s++) {
					pGroup->SendData(EG_KEY_RIGHT);
					if(ResetCheck(&m_Process)) return;
				}
			}
		}
		// In navigate mode focus on the next/prev objects
		else {
			EG_LOG_INFO("rotated by %+d (nav)", pInData->EncoderSteps);
			int32_t s;
			if(pInData->EncoderSteps < 0) {
				for(s = 0; s < -pInData->EncoderSteps; s++) {
					pGroup->FocusPrevious();
					if(ResetCheck(&m_Process)) return;
				}
			}
			else if(pInData->EncoderSteps > 0) {
				for(s = 0; s < pInData->EncoderSteps; s++) {
					pGroup->FocusNext();
					if(ResetCheck(&m_Process)) return;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ButtonProcess(EG_InputData_t *pInData)
{
static EG_InDeviceState_e PrevState = EG_INDEV_STATE_RELEASED;

	if(m_pButtonPoints == nullptr) {	// Die gracefully if btn_points is NULL
		EG_LOG_WARN("btn_points is NULL");
		return;
	}
	EG_Coord_t x = m_pButtonPoints[pInData->ButtonID].m_X;
	EG_Coord_t y = m_pButtonPoints[pInData->ButtonID].m_Y;
	if(PrevState != pInData->State) {
		if(pInData->State == EG_INDEV_STATE_PRESSED) {
			EG_LOG_INFO("button %" EG_PRIu32 " is pressed (x:%d y:%d)", pInData->ButtonID, x, y);
		}
		else {
			EG_LOG_INFO("button %" EG_PRIu32 " is released (x:%d y:%d)", pInData->ButtonID, x, y);
		}
	}
	if(pInData->State == EG_INDEV_STATE_PRESSED) {	// If a new point comes always make a release
		if(m_Process.Pointer.LastPoint.m_X != x ||
			 m_Process.Pointer.LastPoint.m_Y != y) {
			ReleaseProcess(&m_Process);
		}
	}
	if(ResetCheck(&m_Process)) return;
	m_Process.Pointer.ActivePoint.m_X = x;	// Save the new points
	m_Process.Pointer.ActivePoint.m_Y = y;
	if(pInData->State == EG_INDEV_STATE_PRESSED) PressProcess(&m_Process);
	else ReleaseProcess(&m_Process);
	if(ResetCheck(&m_Process)) return;
	m_Process.Pointer.LastPoint.m_X = m_Process.Pointer.ActivePoint.m_X;
	m_Process.Pointer.LastPoint.m_Y = m_Process.Pointer.ActivePoint.m_Y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

 void EGInputDevice::PressProcess(EG_ProcessedInput_t *pProcess)
{
	EG_LOG_INFO("pressed at x:%d y:%d", pProcess->Pointer.ActivePoint.m_X, pProcess->Pointer.ActivePoint.m_Y);
	m_pActiveObj = pProcess->Pointer.pActiveObj;
	if(pProcess->WaitUntilRelease != 0) return;
	EGDisplay *pDisplay = m_pDriver->m_pDisplay;
	bool new_obj_searched = false;
	if(m_pActiveObj == nullptr) {      // If there is no last object then search
		m_pActiveObj = SearchObject(EGDisplay::GetSystemLayer(pDisplay), &pProcess->Pointer.ActivePoint);
		if(m_pActiveObj == nullptr) m_pActiveObj = SearchObject(EGDisplay::GetTopLayer(pDisplay),	&pProcess->Pointer.ActivePoint);
		if(m_pActiveObj == nullptr) m_pActiveObj = SearchObject(EGDisplay::GetActiveScreen(pDisplay),	&pProcess->Pointer.ActivePoint);
		new_obj_searched = true;
	}
	// If there is last object but it is not scrolled and not protected also search
	else if(pProcess->Pointer.pScrollObj == nullptr && m_pActiveObj->HasFlagSet(EG_OBJ_FLAG_PRESS_LOCK) == false) {
		m_pActiveObj = SearchObject(EGDisplay::GetSystemLayer(pDisplay), &pProcess->Pointer.ActivePoint);
		if(m_pActiveObj == nullptr) m_pActiveObj = SearchObject(EGDisplay::GetTopLayer(pDisplay), &pProcess->Pointer.ActivePoint);
		if(m_pActiveObj == nullptr) m_pActiveObj = SearchObject(EGDisplay::GetActiveScreen(pDisplay),	&pProcess->Pointer.ActivePoint);
		new_obj_searched = true;
	}
	// The last object might have scroll throw. Stop it manually
	if(new_obj_searched && pProcess->Pointer.pLastObj) {
		pProcess->Pointer.ScrollThrowVector.m_X = 0;
		pProcess->Pointer.ScrollThrowVector.m_Y = 0;
		ScrollThrowHandler(pProcess);
		if(ResetCheck(pProcess)) return;
	}
	// If a new object was found reset some variables and send a pressed event handler
	if(m_pActiveObj != pProcess->Pointer.pActiveObj) {
		pProcess->Pointer.LastPoint.m_X = pProcess->Pointer.ActivePoint.m_X;
		pProcess->Pointer.LastPoint.m_Y = pProcess->Pointer.ActivePoint.m_Y;
		// If a new object found the previous was lost, so send a Call the ancestor's event handler
		if(pProcess->Pointer.pActiveObj != nullptr) {
			// Save the obj because in special cases `pActiveObj` can change in the Call the ancestor's event handler function
			EGObject *pLastObj = pProcess->Pointer.pActiveObj;
			EGEvent::EventSend(pLastObj, EG_EVENT_PRESS_LOST, this);
			if(ResetCheck(pProcess)) return;
		}
		pProcess->Pointer.pActiveObj = m_pActiveObj; // Save the pressed object
		pProcess->Pointer.pLastObj = m_pActiveObj;
		if(m_pActiveObj != nullptr) {
			// Save the time when the obj pressed to count long press time.
			pProcess->PressedTimestamp = EG_GetTick();
			pProcess->LongPressSent = 0;
			pProcess->Pointer.ScrollSum.m_X = 0;
			pProcess->Pointer.ScrollSum.m_Y = 0;
			pProcess->Pointer.ScrollDirection = EG_DIR_NONE;
			pProcess->Pointer.GestureDirection = EG_DIR_NONE;
			pProcess->Pointer.GestureSent = 0;
			pProcess->Pointer.GestureSum.m_X = 0;
			pProcess->Pointer.GestureSum.m_Y = 0;
			pProcess->Pointer.Vector.m_X = 0;
			pProcess->Pointer.Vector.m_Y = 0;
			EGEvent::EventSend(m_pActiveObj, EG_EVENT_PRESSED, this);		// Call the ancestor's event handler about the press
			if(ResetCheck(pProcess)) return;
			if(m_Process.WaitUntilRelease) return;
			ClickFocus(&m_Process);			// Handle focus
			if(ResetCheck(pProcess)) return;
		}
	}
	// Calculate the vector and apply a low pass filter: new value = 0.5 * old_value + 0.5 * new_value
	pProcess->Pointer.Vector.m_X = pProcess->Pointer.ActivePoint.m_X - pProcess->Pointer.LastPoint.m_X;
	pProcess->Pointer.Vector.m_Y = pProcess->Pointer.ActivePoint.m_Y - pProcess->Pointer.LastPoint.m_Y;

	pProcess->Pointer.ScrollThrowVector.m_X = (pProcess->Pointer.ScrollThrowVector.m_X + pProcess->Pointer.Vector.m_X) / 2;
	pProcess->Pointer.ScrollThrowVector.m_Y = (pProcess->Pointer.ScrollThrowVector.m_Y + pProcess->Pointer.Vector.m_Y) / 2;

	pProcess->Pointer.LastScrollThrowVector = pProcess->Pointer.ScrollThrowVector;

	if(m_pActiveObj) {
		EGEvent::EventSend(m_pActiveObj, EG_EVENT_PRESSING, this);
		if(ResetCheck(pProcess)) return;
		if(m_Process.WaitUntilRelease) return;
		ScrollHandler(pProcess);
		if(ResetCheck(pProcess)) return;
		Gesture(pProcess);
		if(ResetCheck(pProcess)) return;
		// If there is no scrolling then check for long press time
		if(pProcess->Pointer.pScrollObj == nullptr && pProcess->LongPressSent == 0) {
			// Call the ancestor's event handler about the long press if enough time elapsed
			if(EG_TickElapse(pProcess->PressedTimestamp) > m_pDriver->m_LongPressTime) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_LONG_PRESSED, this);
				if(ResetCheck(pProcess)) return;

				// Mark the Call the ancestor's event handler sending to do not send it again
				pProcess->LongPressSent = 1;

				// Save the long press time stamp for the long press repeat handler
				pProcess->LongPressRepeatTimestamp = EG_GetTick();
			}
		}

		// Send long press repeated Call the ancestor's event handler
		if(pProcess->Pointer.pScrollObj == nullptr && pProcess->LongPressSent == 1) {
			// Call the ancestor's event handler about the long press repeat if enough time elapsed
			if(EG_TickElapse(pProcess->LongPressRepeatTimestamp) > m_pDriver->m_LongPressRepeatTime) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_LONG_PRESSED_REPEAT, this);
				if(ResetCheck(pProcess)) return;
				pProcess->LongPressRepeatTimestamp = EG_GetTick();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ReleaseProcess(EG_ProcessedInput_t *pProcess)
{
	if(pProcess->WaitUntilRelease != 0) {
		EGEvent::EventSend(pProcess->Pointer.pActiveObj, EG_EVENT_PRESS_LOST, this);
		if(ResetCheck(pProcess)) return;
		pProcess->Pointer.pActiveObj = nullptr;
		pProcess->Pointer.pLastObj = nullptr;
		pProcess->PressedTimestamp = 0;
		pProcess->LongPressRepeatTimestamp = 0;
		pProcess->WaitUntilRelease = 0;
	}
	m_pActiveObj = pProcess->Pointer.pActiveObj;
	EGObject *pScrollObject = pProcess->Pointer.pScrollObj;
	if(m_pActiveObj) {	// Forget the act obj and send a released Call the ancestor's event handler
		EG_LOG_INFO("released");
		// Send RELEASE Call the ancestor's event handler and event
		EGEvent::EventSend(m_pActiveObj, EG_EVENT_RELEASED, this);
		if(ResetCheck(pProcess)) return;
		// Send CLICK if no scrolling
		if(pScrollObject == nullptr) {
			if(pProcess->LongPressSent == 0) {
				EGEvent::EventSend(m_pActiveObj, EG_EVENT_SHORT_CLICKED, this);
				if(ResetCheck(pProcess)) return;
			}
			EGEvent::EventSend(m_pActiveObj, EG_EVENT_CLICKED, this);
			if(ResetCheck(pProcess)) return;
		}
		pProcess->Pointer.pActiveObj = nullptr;
		pProcess->PressedTimestamp = 0;
		pProcess->LongPressRepeatTimestamp = 0;
		// Get the transformed vector with this object
		if(pScrollObject) {
			int16_t Angle = 0;
			int16_t Zoom = 256;
			EGPoint Pivot = {0, 0};
			EGObject *pParent = pScrollObject;
			while(pParent) {
				Angle += pParent->GetStyleTransformAngle(0);
				Zoom *= (pParent->GetStyleTransformZoom(0) / 256);
				pParent = pParent->GetParent();
			}

			if(Angle != 0 || Zoom != EG_IMG_ZOOM_NONE) {
				Angle = -Angle;
				Zoom = (256 * 256) / Zoom;
				pProcess->Pointer.ScrollThrowVector.PointTransform(Angle, Zoom, &Pivot);
				pProcess->Pointer.LastScrollThrowVector.PointTransform(Angle, Zoom, &Pivot);
			}
		}
	}

	// The reset can be set in the Call the ancestor's event handler function. 
	if(pScrollObject) {
		ScrollThrowHandler(pProcess);
		if(ResetCheck(pProcess)) return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ResetQueryHandler(void)
{
	if(m_Process.ResetQuery) {
		m_Process.Pointer.pActiveObj = nullptr;
		m_Process.Pointer.pLastObj = nullptr;
		m_Process.Pointer.pScrollObj = nullptr;
		m_Process.LongPressSent = 0;
		m_Process.PressedTimestamp = 0;
		m_Process.LongPressRepeatTimestamp = 0;
		m_Process.Pointer.ScrollSum.m_X = 0;
		m_Process.Pointer.ScrollSum.m_Y = 0;
		m_Process.Pointer.ScrollDirection = EG_DIR_NONE;
		m_Process.Pointer.ScrollThrowVector.m_X = 0;
		m_Process.Pointer.ScrollThrowVector.m_Y = 0;
		m_Process.Pointer.GestureSum.m_X = 0;
		m_Process.Pointer.GestureSum.m_Y = 0;
		m_Process.ResetQuery = 0;
		m_pActiveObj = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ClickFocus(EG_ProcessedInput_t *pProcess)
{
	if((m_pActiveObj->HasFlagSet(EG_OBJ_FLAG_CLICK_FOCUSABLE) == false) || (pProcess->Pointer.pLastPressedObj == m_pActiveObj)) {
		return;
	}
	EGGroup *pActiveGroup = (EGGroup*)m_pActiveObj->GetGroup();
	EGGroup *pPreviousGroup = (pProcess->Pointer.pLastPressedObj != nullptr) ? (EGGroup*)pProcess->Pointer.pLastPressedObj->GetGroup() : nullptr;
	if(pActiveGroup == pPreviousGroup) {	// If both the last and active obj. are in the same group (or have no group)
		if(pActiveGroup != nullptr) {		// The objects are in a group
			EGGroup::FocusObject(m_pActiveObj);
			if(ResetCheck(pProcess)) return;
		}
		else {		// The object are not in group
			if(pProcess->Pointer.pLastPressedObj) {
				EGEvent::EventSend(pProcess->Pointer.pLastPressedObj, EG_EVENT_DEFOCUSED, this);
				if(ResetCheck(pProcess)) return;
			}
			EGEvent::EventSend(m_pActiveObj, EG_EVENT_FOCUSED, this);
			if(ResetCheck(pProcess)) return;
		}
	}
	else {	// The object are not in the same group (in different groups or one has no group)
		if((pPreviousGroup == nullptr) && pProcess->Pointer.pLastPressedObj) { 
			EGEvent::EventSend(pProcess->Pointer.pLastPressedObj, EG_EVENT_DEFOCUSED, this);
			if(ResetCheck(pProcess)) return;
		}
		else {		// Focus on a non-group object
			if(pProcess->Pointer.pLastPressedObj) {
				if(pPreviousGroup == nullptr) {				// If the prev. object also wasn't in a group defocus it
					EGEvent::EventSend(pProcess->Pointer.pLastPressedObj, EG_EVENT_DEFOCUSED, this);
					if(ResetCheck(pProcess)) return;
				}
				else {		// If the previous object also was in a group at least "LEAVE" it instead of defocus
					EGEvent::EventSend(pProcess->Pointer.pLastPressedObj, EG_EVENT_LEAVE, this);
					if(ResetCheck(pProcess)) return;
				}
			}
		}
		if(pActiveGroup != nullptr) {		// Focus the object in its group
			EGGroup::FocusObject(m_pActiveObj);
			if(ResetCheck(pProcess)) return;
		}
		else {
			EGEvent::EventSend(m_pActiveObj, EG_EVENT_FOCUSED, this);
			if(ResetCheck(pProcess)) return;
		}
	}
	pProcess->Pointer.pLastPressedObj = m_pActiveObj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::Gesture(EG_ProcessedInput_t *pProcess)
{
	if(pProcess->Pointer.pScrollObj) return;
	if(pProcess->Pointer.GestureSent) return;

	EGObject *pGestureObj = pProcess->Pointer.pActiveObj;

	// If gesture parent is active check recursively the gesture attribute
	while(pGestureObj && pGestureObj->HasFlagSet(EG_OBJ_FLAG_GESTURE_BUBBLE)) {
		pGestureObj = pGestureObj->GetParent();
	}
	if(pGestureObj == nullptr) return;
	if((LV_ABS(pProcess->Pointer.Vector.m_X) < m_pDriver->m_GestureMinVelocity) &&
		  (LV_ABS(pProcess->Pointer.Vector.m_Y) < m_pDriver->m_GestureMinVelocity)) {
		pProcess->Pointer.GestureSum.m_X = 0;
		pProcess->Pointer.GestureSum.m_Y = 0;
	}
	// Count the movement by gesture
	pProcess->Pointer.GestureSum.m_X += pProcess->Pointer.Vector.m_X;
	pProcess->Pointer.GestureSum.m_Y += pProcess->Pointer.Vector.m_Y;
	if((LV_ABS(pProcess->Pointer.GestureSum.m_X) > m_pDriver->m_GestureLimit) ||
		  (LV_ABS(pProcess->Pointer.GestureSum.m_Y) > m_pDriver->m_GestureLimit)) {
		pProcess->Pointer.GestureSent = 1;

		if(LV_ABS(pProcess->Pointer.GestureSum.m_X) > LV_ABS(pProcess->Pointer.GestureSum.m_Y)) {
			if(pProcess->Pointer.GestureSum.m_X > 0)
				pProcess->Pointer.GestureDirection = EG_DIR_RIGHT;
			else
				pProcess->Pointer.GestureDirection = EG_DIR_LEFT;
		}
		else {
			if(pProcess->Pointer.GestureSum.m_Y > 0)
				pProcess->Pointer.GestureDirection = EG_DIR_BOTTOM;
			else
				pProcess->Pointer.GestureDirection = EG_DIR_TOP;
		}
		EGEvent::EventSend(pGestureObj, EG_EVENT_GESTURE, this);
		if(ResetCheck(pProcess)) return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGInputDevice::ResetCheck(EG_ProcessedInput_t *pProcess)
{
	if(pProcess->ResetQuery){
		m_pActiveObj = nullptr;
	}
	return pProcess->ResetQuery ? true : false;
}

