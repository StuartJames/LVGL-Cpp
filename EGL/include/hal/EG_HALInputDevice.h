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

#include "EG_IntrnlConfig.h"

#include <stdbool.h>
#include <stdint.h>
#include "misc/EG_Point.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Timer.h"

///////////////////////////////////////////////////////////////////////////////

#define EG_INDEV_DEF_SCROLL_LIMIT         10    // Drag threshold in pixels
#define EG_INDEV_DEF_SCROLL_THROW         10    // Drag throw slow-down in [%]. Greater value -> faster slow-down
#define EG_INDEV_DEF_LONG_PRESS_TIME      400   // Long press time in milliseconds. Time to send `EG_EVENT_LONG_PRESSSED`)
#define EG_INDEV_DEF_LONG_PRESS_REP_TIME  100   // Repeated trigger period in long press [ms]. Time between `EG_EVENT_LONG_PRESSED_REPEAT
#define EG_INDEV_DEF_GESTURE_LIMIT        50    // Gesture threshold in pixels
#define EG_INDEV_DEF_GESTURE_MIN_VELOCITY 3     // Gesture min velocity at release before swipe (pixels)

///////////////////////////////////////////////////////////////////////////////

class EGDisplay;
class EGObject;

///////////////////////////////////////////////////////////////////////////////

// Possible input device types
typedef enum : uint8_t {
    EG_INDEV_TYPE_NONE,    // Uninitialized state
    EG_INDEV_TYPE_POINTER, // Touch pad, mouse, external button
    EG_INDEV_TYPE_KEYPAD,  // Keypad or keyboard
    EG_INDEV_TYPE_BUTTON,  // External (hardware button) which is assigned to a specific point of the screen
    EG_INDEV_TYPE_ENCODER, // Encoder with only Left, Right turn and a Button
} EG_InDeviceType_e;


// States for input devices
typedef enum : uint8_t {
    EG_INDEV_STATE_RELEASED = 0,
    EG_INDEV_STATE_PRESSED
} EG_InDeviceState_e;

///////////////////////////////////////////////////////////////////////////////

// Data structure passed to an input driver to fill in
typedef struct EG_InputData_t{
    EG_InputData_t() : Key(0), ButtonID(0), EncoderSteps(0), State(EG_INDEV_STATE_RELEASED){};
    EGPoint     Point;            // For EG_INDEV_TYPE_POINTER the currently pressed point
    uint32_t    Key;              // For EG_INDEV_TYPE_KEYPAD the currently pressed key
    uint32_t    ButtonID;         // For EG_INDEV_TYPE_BUTTON the currently pressed button
    int16_t     EncoderSteps;     // For EG_INDEV_TYPE_ENCODER number of steps since the previous read
    EG_InDeviceState_e State;       // EG_INDEV_STATE_REL or EG_INDEV_STATE_PR
    bool        ContinueReading;  // If set to true, the read callback is invoked again
} EG_InputData_t;

///////////////////////////////////////////////////////////////////////////////

// Run time data of input devices. Internally used by the library, you should not need to touch it.
typedef struct EG_ProcessedInput_t {
  EG_ProcessedInput_t() : State(EG_INDEV_STATE_RELEASED), LongPressSent(0), ResetQuery(0), Disabled(0), WaitUntilRelease(0),
                          PressedTimestamp(0), LongPressRepeatTimestamp(0){};
  EG_InDeviceState_e State; // Current state of the input device.
  uint8_t LongPressSent     : 1;
  uint8_t ResetQuery        : 1;
  uint8_t Disabled          : 1;
  uint8_t WaitUntilRelease  : 1;
//  union {
    struct Pointer{                    // Pointer and button data
      Pointer() : pActiveObj(nullptr), pLastObj(nullptr), pScrollObj(nullptr), pLastPressedObj(nullptr),
                  ScrollDirection(EG_DIR_NONE), GestureDirection(EG_DIR_NONE), GestureSent(0){};
      EGPoint       ActivePoint; // Current point of input device.
      EGPoint       Point;
      EGPoint       LastPoint; // Last point of input device.
      EGPoint       LastRawPoint; // Last point read from read_cb. 
      EGPoint       Vector; // Difference between `act_point` and `last_point`.
      EGPoint       ScrollSum; // Count the dragged pixels to check EG_INDEV_DEF_SCROLL_LIMIT
      EGPoint       ScrollThrowVector;
      EGPoint       LastScrollThrowVector;
      EGPoint       ScrollThrowVectorOri;
      EGObject     *pActiveObj;      // The object being pressed
      EGObject     *pLastObj;     // The last object which was pressed
      EGObject     *pScrollObj;   // The object being scrolled
      EGObject     *pLastPressedObj; // The lastly pressed object
      EGRect        ScrollArea;
      EGPoint       GestureSum; // Count the gesture pixels to check EG_INDEV_DEF_GESTURE_LIMIT
      EG_DirType_e  ScrollDirection   : 4;
      EG_DirType_e  GestureDirection  : 4;
      uint8_t       GestureSent       : 1;
    } Pointer;
    struct Keypad{                        // Keypad data
      Keypad() : LastState(EG_INDEV_STATE_RELEASED), LastKey(0){};
      EG_InDeviceState_e  LastState;
      uint32_t            LastKey;
    } Keypad;
//  } Types;
  uint32_t PressedTimestamp;         // Pressed time stamp
  uint32_t LongPressRepeatTimestamp; // Long press repeat time stamp
} EG_ProcessedInput_t;

///////////////////////////////////////////////////////////////////////////////

// Initialized by the user and registered by 'EG_indev_add()'
class EGInputDriver
{
public:
                    EGInputDriver(void);
  void              (*ReadCB)(EGInputDriver *pDriver, EG_InputData_t *pData);    // Function pointer to read input device data.
  void              (*FeedbackCB)(EGInputDriver *pDriver, uint8_t);  // Called when an action happened on the input device. The second parameter is the event from `EGEvent`

  EG_InDeviceType_e    m_Type;               // Input device type
  EGDisplay           *m_pDisplay;           // Pointer to the assigned display
  EGTimer             *m_pReadTimer;         // Timer to periodically read the input device
  void                *m_pController;        // Hardware ctouch controller
  uint8_t              m_ScrollLimit;        // Number of pixels to slide before actually drag the object
  uint8_t              m_ScrollThrow;        // Drag throw slow-down in [%]. Greater value means faster slow-down
  uint8_t              m_GestureMinVelocity; // At least this difference should be between two points to evaluate as gesture
  uint8_t              m_GestureLimit;       // At least this difference should be to send a gesture
  uint16_t             m_LongPressTime;      // Long press time in milliseconds
  uint16_t             m_LongPressRepeatTime;// Repeated trigger period in long press [ms]

#if EG_USE_USER_DATA
  void                *m_pUserData;
#endif
} ;



