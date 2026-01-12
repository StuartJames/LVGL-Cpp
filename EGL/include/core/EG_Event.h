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

#pragma once

#include <stdbool.h>
#include "EG_Object.h"
#include "EG_InputDevice.h"

///////////////////////////////////////////////////////////////////////////////////////

class EGInputDevice;
struct EG_EventDiscriptor_t;

///////////////////////////////////////////////////////////////////////////////////////

typedef enum : uint16_t {
    EG_EVENT_ALL = 0,

    // Input device events
    EG_EVENT_PRESSED,             // (01) The object has been pressed
    EG_EVENT_PRESSING,            // (02) The object is being pressed (called continuously while pressing)
    EG_EVENT_PRESS_LOST,          // (03) The object is still being pressed but slid cursor/finger off of the object 
    EG_EVENT_SHORT_CLICKED,       // (04) The object was pressed for a short period of time, then released it. Not called if scrolled.
    EG_EVENT_LONG_PRESSED,        // (05) Object has been pressed for at least `long_press_time`.  Not called if scrolled.
    EG_EVENT_LONG_PRESSED_REPEAT, // (06) Called after `long_press_time` in every `long_press_repeat_time` ms.  Not called if scrolled.
    EG_EVENT_CLICKED,             // (07) Called on release if not scrolled (regardless to long press)
    EG_EVENT_RELEASED,            // (08) Called in every cases when the object has been released
    EG_EVENT_SCROLL_BEGIN,        // (09) Scrolling begins. The event parameter is a pointer to the animation of the scroll. Can be modified
    EG_EVENT_SCROLL_END,          // (10) Scrolling ends
    EG_EVENT_SCROLL,              // (11) Scrolling
    EG_EVENT_GESTURE,             // (12) A gesture is detected. Get the gesture with `EGInputDevice::GetActive()->GetGestureDirection();` 
    EG_EVENT_KEY,                 // (13) A key is sent to the object. Get the key with `EGInputDevice::GetActive()->GetKey();`
    EG_EVENT_FOCUSED,             // (14) The object is focused
    EG_EVENT_DEFOCUSED,           // (15) The object is defocused
    EG_EVENT_LEAVE,               // (16) The object is defocused but still selected
    EG_EVENT_HIT_TEST,            // (17) Perform advanced hit-testing

    // Drawing events
    EG_EVENT_COVER_CHECK,        // (18) Check if the object fully covers an area. The event parameter is `EG_CoverCheckInfo_t *`.
    EG_EVENT_REFR_EXT_DRAW_SIZE, // (19) Get the required extra draw area around the object (e.g. for shadow). The event parameter is `EG_Coord_t *` to store the size.
    EG_EVENT_DRAW_MAIN_BEGIN,    // (20) Starting the main drawing phase
    EG_EVENT_DRAW_MAIN,          // (21) Perform the main drawing
    EG_EVENT_DRAW_MAIN_END,      // (22) Finishing the main drawing phase
    EG_EVENT_DRAW_POST_BEGIN,    // (23) Starting the post draw phase (when all children are drawn)
    EG_EVENT_DRAW_POST,          // (24) Perform the post draw phase (when all children are drawn)
    EG_EVENT_DRAW_POST_END,      // (25) Finishing the post draw phase (when all children are drawn)
    EG_EVENT_DRAW_PART_BEGIN,    // (26) Starting to draw a part. The event parameter is `EG_obj_draw_dsc_t *`. 
    EG_EVENT_DRAW_PART_END,      // (27) Finishing to draw a part. The event parameter is `EG_obj_draw_dsc_t *`. 

    // Special events
    EG_EVENT_VALUE_CHANGED,       // (28) The object's value has changed (i.e. slider moved)
    EG_EVENT_INSERT,              // (29) A text is inserted to the object. The event data is `char *` being inserted.
    EG_EVENT_REFRESH,             // (30) Notify the object to refresh something on it (for the user)
    EG_EVENT_READY,               // (31) A process has finished
    EG_EVENT_CANCEL,              // (32) A process has been cancelled 

    // Other events
    EG_EVENT_DELETE,              // (33) Object is being deleted
    EG_EVENT_CHILD_CHANGED,       // (34) Child was removed, added, or its size, position were changed 
    EG_EVENT_CHILD_CREATED,       // (35) Child was created, always bubbles up to all parents
    EG_EVENT_CHILD_DELETED,       // (36) Child was deleted, always bubbles up to all parents
    EG_EVENT_SCREEN_UNLOAD_START, // (37) A screen unload started, fired immediately when scr_load is called
    EG_EVENT_SCREEN_LOAD_START,   // (38) A screen load started, fired when the screen change delay is expired
    EG_EVENT_SCREEN_LOADED,       // (39) A screen was loaded
    EG_EVENT_SCREEN_UNLOADED,     // (40) A screen was unloaded
    EG_EVENT_SIZE_CHANGED,        // (41) Object coordinates/size have changed
    EG_EVENT_STYLE_CHANGED,       // (42) Object's style has changed
    EG_EVENT_LAYOUT_CHANGED,      // (43) The children position has changed due to a layout recalculation
    EG_EVENT_GET_SELF_SIZE,       // (44) Get the internal size of a widget

    _EG_EVENT_LAST,               // (45) Number of default events

    EG_EVENT_PREPROCESS = 0x80,   // This is a flag that can be set with an event so it's processed before the class default event processing 
} EG_EventCode_e;

typedef void (*EG_EventCB_t)(EGEvent *pEvent);

// Used as the event parameter of ::EG_EVENT_HIT_TEST to check if an `point` can click the object or not.
typedef struct {
    const EGPoint *pPoint;        // A point relative to screen to check if it can click the object or not
    bool Result;                  // true: `point` can click the object; false: it cannot
} EG_HitTestState_t;

// Used as the event parameter of ::EG_EVENT_COVER_CHECK to check if an area is covered by the object or not.
typedef struct {
    EG_CoverResult_e Result;
    const EGRect    *pRect;
} EG_CoverCheckInfo_t;

typedef struct EG_EventDiscriptor_t {
    EG_EventCB_t  EventCB;
    void          *pExtParam;
    EG_EventCode_e Filter : 8;
} EG_EventDiscriptor_t;

///////////////////////////////////////////////////////////////////////////////////////

class EGEvent 
{
public:
                          EGEvent(void);
  EG_Result_t             Pump(const EG_ClassType_t *pClass);
  EGObject*               GetTarget(void){ return m_pTarget; };
  EGObject*               GetCurrentTarget(void){ return m_pCurrentTarget; };
  EG_EventCode_e          GetCode(void){ return (EG_EventCode_e)((int)m_EventCode & ~EG_EVENT_PREPROCESS);};
  void*                   GetParam(void){ return m_pParam; };
  void*                   GetExtParam(void){ return m_pExtParam; };
  bool                    ShouldBubble(void);
  void                    StopBubling(void){ m_StopBubbling = 1; };
  void                    StopProcessing(void){ m_StopProcessing = 1; };
  uint32_t                RegisterID(void);
  bool                    RemoveEventCBWithUserData(EGObject *pObj, EG_EventCB_t EventCB, const void *pParam);
  bool                    RemoveEventDiscriptor(EGObject *pObj, struct EG_EventDiscriptor_t *pEventDiscriptor);
  void*                   GetEventExtParam(EGObject *pObj, EG_EventCB_t EventCB);
  EGInputDevice*          GetInputDevice(void);
  EGDrawContext*          GetDrawPartDiscriptor(void);
  EGDrawContext*          GetDrawContext(void);
  const EGRect*           GetOldSize(void);
  uint32_t                GetKey(void);
  EGAnimate*              GetScrollAnimation(void);
  void                    SetExtDrawSize(EG_Coord_t Size);
  EGPoint*                GetSelfSizeInfo(void);
  EG_HitTestState_t*      GetHitTestInfo(void);
  const EGRect*           GetCoverArea(void);
  void                    SetCoverResult(EG_CoverResult_e Result);

  static EG_EventDiscriptor_t* AddEventCB(EGObject *pObj, EG_EventCB_t EventCB, EG_EventCode_e Filter, void *pParam);
  static bool             RemoveEventCB(EGObject *pObj, EG_EventCB_t EventCB);
  static EG_Result_t      EventSend(EGObject *pObj, EG_EventCode_e EventCode, void *pParam);
  static void             MarkDeleted(EGObject *pObj);

  EGObject               *m_pTarget;
  EGObject               *m_pCurrentTarget;
  EG_EventCode_e          m_EventCode;
  void                   *m_pExtParam;
  void                   *m_pParam;
  EGEvent                *m_pPrevious;
  uint8_t                 m_Deleted : 1;
  uint8_t                 m_StopProcessing : 1;
  uint8_t                 m_StopBubbling : 1;

private:

  EG_EventDiscriptor_t*   GetDiscriptor(const EGObject *pObj, uint32_t Id);
  EG_Result_t             SendCore(void);

  static uint32_t         m_LastID;
  static EGEvent         *m_pEventHead;

};


