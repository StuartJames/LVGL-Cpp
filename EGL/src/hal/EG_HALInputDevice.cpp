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

#include "misc/EG_Assert.h"
#include "hal/EG_HALInputDevice.h"
#include "core/EG_InputDevice.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Misc.h"
#include "hal/EG_HALDisplay.h"

#if EG_LOG_TRACE_INDEV
#define INDEV_TRACE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define INDEV_TRACE(...)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::InitialiseDriver(EGInputDriver *pDriver)
{
	pDriver->m_Type = EG_INDEV_TYPE_NONE;
	pDriver->m_ScrollLimit = EG_INDEV_DEF_SCROLL_LIMIT;
	pDriver->m_ScrollThrow = EG_INDEV_DEF_SCROLL_THROW;
	pDriver->m_LongPressTime = EG_INDEV_DEF_LONG_PRESS_TIME;
	pDriver->m_LongPressRepeatTime = EG_INDEV_DEF_LONG_PRESS_REP_TIME;
	pDriver->m_GestureLimit = EG_INDEV_DEF_GESTURE_LIMIT;
	pDriver->m_GestureMinVelocity = EG_INDEV_DEF_GESTURE_MIN_VELOCITY;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDevice* EGInputDevice::RegisterDriver(EGInputDriver *pDriver)
{
	if(pDriver->m_pDisplay == nullptr) pDriver->m_pDisplay = EGDisplay::GetDefault();
	if(pDriver->m_pDisplay == nullptr){
		EG_LOG_WARN("DriverRegister: no display registered therefore can't attach the input device to a display");
		return nullptr;
	}
  EGInputDevice *pDevice = new EGInputDevice;
	if(!pDevice) return nullptr;
	pDevice->m_pDriver = pDriver;      // Attach the driver
	pDevice->m_Process.ResetQuery = 1;
  m_InDeviceList.AddHead(pDevice);  // save the device in the list then create periodic call back timer
	pDevice->m_pDriver->m_pReadTimer = EGTimer::Create(ReadTimerCB, EG_INDEV_DEF_READ_PERIOD, (void*)pDevice, false);
	return pDevice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::UnloadDevice(EGInputDevice *pDevice)
{
	EG_ASSERT_NULL(pDevice);
	EG_ASSERT_NULL(pDevice->m_pDriver);
	if(pDevice->m_pDriver->m_pReadTimer != nullptr) EGTimer::Delete(pDevice->m_pDriver->m_pReadTimer);
  delete pDevice->m_pDriver;	// Remove input device driver 
  POSITION Pos = m_InDeviceList.Find(pDevice);
  if(Pos != nullptr) m_InDeviceList.RemoveAt(Pos);  // remove the device from the list
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGInputDevice::UpdateDriver(EGInputDriver *pDriver)
{
	EG_ASSERT_NULL(pDriver);
	EG_ASSERT_NULL(m_pDriver);
	EG_ASSERT_NULL(m_pDriver->m_pReadTimer);
	if(pDriver->m_pDisplay == nullptr) pDriver->m_pDisplay = EGDisplay::GetDefault();
	if(pDriver->m_pDisplay == nullptr) {
		EG_LOG_WARN("UpdateDriver: no display registered therefore no changes made");
		m_Process.Disabled = true;
		return false;
	}
	EGTimer::Delete(m_pDriver->m_pReadTimer);
  delete m_pDriver;
	m_pDriver = pDriver;
	m_pDriver->m_pReadTimer = EGTimer::Create(ReadTimerCB, EG_INDEV_DEF_READ_PERIOD, this);
	m_Process.ResetQuery = 1;
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDevice* EGInputDevice::GetNext(EGInputDevice *pInputDevice)
{
	if(pInputDevice == nullptr) return (EGInputDevice*)m_InDeviceList.GetHead(nullptr);
	else return (EGInputDevice*)m_InDeviceList.GetNext(pInputDevice);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGInputDevice::Read(EG_InputData_t *pData)
{
  pData->Point.Set(0, 0);
  pData->Key = 0;
  pData->ButtonID = 0;
  pData->EncoderSteps = 0;
  pData->State = EG_INDEV_STATE_RELEASED;
  pData->ContinueReading = false;
	// For touchpad sometimes users don't set the last pressed coordinate on release. So be sure coordinates are initialized to the last point 
  if(m_pDriver->m_Type == EG_INDEV_TYPE_POINTER) pData->Point = m_Process.Pointer.LastRawPoint;
	else if(m_pDriver->m_Type == EG_INDEV_TYPE_KEYPAD) {	// Similarly set at least the last key in case of the user doesn't set it on release
		pData->Key = m_Process.Keypad.LastKey;
	}
	else if(m_pDriver->m_Type == EG_INDEV_TYPE_ENCODER) {	// For compatibility assume that used button was enter (encoder push)
		pData->Key = EG_KEY_ENTER;
	}
	if(m_pDriver->ReadCB != nullptr) {
		INDEV_TRACE("calling indev_read_cb");
		m_pDriver->ReadCB(m_pDriver, pData);
	}
	else EG_LOG_WARN("Read callback is not registered");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////// Input Driver //////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

EGInputDriver::EGInputDriver(void) :
  ReadCB(nullptr),
  FeedbackCB(nullptr),
  m_Type(EG_INDEV_TYPE_NONE),
  m_pDisplay(nullptr),
  m_pReadTimer(nullptr),
  m_pController(nullptr),
  m_ScrollLimit(0),
  m_ScrollThrow(0),
  m_GestureMinVelocity(0),
  m_GestureLimit(0),
  m_LongPressTime(0),
  m_LongPressRepeatTime(0)
{
}

