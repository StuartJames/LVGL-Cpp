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

#pragma once

#include "EG_Object.h"
#include "../hal/EG_HALInputDevice.h"
#include "EG_Group.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGTimer;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGInputDevice
{
// Base Section //
public:
                          EGInputDevice();
  virtual                 ~EGInputDevice();
  EG_InDeviceType_e       GetType(void); // Get the type of an input device
  void                    ResetLongPress(void); // Reset the long press state of an input device
  void                    SetCursor(EGObject *pObj); // Set a cursor for a pointer input device (for EG_INPUT_TYPE_POINTER and EG_INPUT_TYPE_BUTTON)
  void                    SetGroup(EGGroup *pGroup); // Set a destination group for a keypad input device (for EG_INDEV_TYPE_KEYPAD)
  void                    SetButtonPoints(const EGPoint Points[]); // Set the an array of points for EG_INDEV_TYPE_BUTTON.
  EGGroup*                GetGroup(void){ return m_pGroup; }; // Get destination group
  void                    GetPoint(EGPoint *pPoint); // Get the last point of an input device (for EG_INDEV_TYPE_POINTER and EG_INDEV_TYPE_BUTTON)
  EG_DirType_e            GetGestureDirection(void); // Get the current gesture direct
  uint32_t                GetKey(void); // Get the last pressed key of an input device (for EG_INDEV_TYPE_KEYPAD)
  EG_DirType_e            GetScrollDirection(void); // Check the current scroll direction of an input device (for EG_INDEV_TYPE_POINTER and
  EGObject*               GetScrollObj(void); // Get the currently scrolled object (for EG_INDEV_TYPE_POINTER and
  void                    GetVector(EGPoint *pPoint); // Get the movement vector of an input device (for EG_INDEV_TYPE_POINTER and
  void                    WaitRelease(void); // Do nothing until the next release
  EGObject*               SearchObject(EGObject *pObj, EGPoint *pPoint);  // Search the most top, clickable object by a point
  void                    InitialiseDriver(EGInputDriver *pDriver); // Initialize an input device driver with default values.
  bool                    UpdateDriver(EGInputDriver *pDriver); // Update the driver in run time.
  void                    Read(EG_InputData_t *pData); // Read data from an input device.

  static void             ReadTimerCB(EGTimer *pTimer); // Called periodically to read the input devices
  static void             Enable(EGInputDevice *pDevice, bool Flag = true); // Enable or disable one or all input devices (default enabled)
  static EGInputDevice*   GetActive(void); // Get the currently processed input device. Can be used in action functions too.
  static EGObject*        GetActiveObj(void); // Gets a pointer to the currently active object in the currently processed input device.
  static EGTimer*         GetReadTimer(EGInputDevice *pIndev); // Get a pointer to the indev read timer to
  static EGInputDevice*   RegisterDriver(EGInputDriver *pDriver); // Register an initialized input device driver.
  static void             UnloadDevice(EGInputDevice *pDevice); // Remove input device.
  static EGInputDevice*   GetNext(EGInputDevice *pInputDevice); // Get the next input device.
  static void             Reset(EGInputDevice *pInputDev, EGObject *pObj); // Reset one or all input devices

  EGInputDriver          *m_pDriver;
  EG_ProcessedInput_t     m_Process;

  static EGList           m_InDeviceList;         // Linked list to store the input devices

private:
  void                    ReadProcess(void);
  void                    PointerProcess(EG_InputData_t *pData);
  void                    KeypadProcess(EG_InputData_t *pData);
  void                    EncoderProcess(EG_InputData_t *pData);
  void                    ButtonProcess(EG_InputData_t *pData);
  void                    PressProcess(EG_ProcessedInput_t *pProcess);
  void                    ReleaseProcess(EG_ProcessedInput_t *pProcess);
  void                    ResetQueryHandler(void);
  void                    ClickFocus(EG_ProcessedInput_t *pProcess);
  void                    Gesture(EG_ProcessedInput_t *pProcess);
  bool                    ResetCheck(EG_ProcessedInput_t *pProcess);

  EGTimer                *m_pReadTimer;          
  EGObject               *m_pCursor;        // Cursor for EG_INPUT_TYPE_POINTER
  EGGroup                *m_pGroup;         // Keypad destination group

  static EGInputDevice   *m_pActiveDevice;
  static EGObject        *m_pActiveObj;
  const EGPoint          *m_pButtonPoints;  // Array of points assigned to the button ()screen will be pressed here by the buttons

// Scroll Section //
public:
  EG_Coord_t              ScrollThrowPredict(EG_DirType_e Direction); // Predict where would a scroll throw end

  static void             ScrollGetSnapDist(EGObject *pObj, EGPoint *pPoint); // Get the distance of the nearest snap point
  static void             ScrollHandler(EG_ProcessedInput_t *pProcess); // Handle scrolling. Called by LVGL during input device processing
  static void             ScrollThrowHandler(EG_ProcessedInput_t *pProcess); // Handle throwing after scrolling. Called by LVGL during input device processing

private:
  static EGObject*        FindScrollObj(EG_ProcessedInput_t *pProcess);
  static void             InitScrollLimits(EG_ProcessedInput_t *pProcess);
  static EG_Coord_t       FindSnapPointX(EGObject *pObj, EG_Coord_t min, EG_Coord_t max, EG_Coord_t ofs);
  static EG_Coord_t       FindSnapPointY(EGObject *pObj, EG_Coord_t min, EG_Coord_t max, EG_Coord_t ofs);
  static void             ScrollLimitDifference(EG_ProcessedInput_t *pProcess, EG_Coord_t * diff_x, EG_Coord_t * diff_y);
  static EG_Coord_t       ScrollThrowPredictY(EG_ProcessedInput_t *pProcess);
  static EG_Coord_t       ScrollThrowPredictX(EG_ProcessedInput_t *pProcess);
  static EG_Coord_t       ElasticDifference(EGObject *pObj, EG_Coord_t diff, EG_Coord_t scroll_start, EG_Coord_t scroll_end, EG_DirType_e dir);

};
