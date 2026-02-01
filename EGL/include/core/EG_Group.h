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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../EG_IntrnlConfig.h"

#include "../misc/EG_Types.h"
#include "../misc/lv_gc.h"
#include "EG_Object.h"

/////////////////////////////////////////////////////////////////////////////

class EGGroup;
class EGInputDevice;

/////////////////////////////////////////////////////////////////////////////

// Predefined keys to control the focused object via lv_group_send(group, c)
typedef enum EG_Key_e : uint8_t{
  EG_KEY_UP        = 17,  // 0x11
  EG_KEY_DOWN      = 18,  // 0x12
  EG_KEY_RIGHT     = 19,  // 0x13
  EG_KEY_LEFT      = 20,  // 0x14
  EG_KEY_ESC       = 27,  // 0x1B
  EG_KEY_DEL       = 127, // 0x7F
  EG_KEY_BACKSPACE = 8,   // 0x08
  EG_KEY_ENTER     = 10,  // 0x0A, '\n'
  EG_KEY_NEXT      = 9,   // 0x09, '\t'
  EG_KEY_PREV      = 11,  // 0x0B, '
  EG_KEY_HOME      = 2,   // 0x02, STX
  EG_KEY_END       = 3,   // 0x03, ETX
} EG_Key_e;


typedef void (*EG_GroupFocusCB_t)(EGGroup *pGroup);
typedef void (*EG_GroupEdgeCB_t)(EGGroup *pGroup, bool);

typedef enum EG_GroupRefocusPolicy_e : uint8_t {
    EG_GROUP_REFOCUS_POLICY_NEXT = 0,
    EG_GROUP_REFOCUS_POLICY_PREV = 1
} EG_GroupRefocusPolicy_e;

/////////////////////////////////////////////////////////////////////////////

class EGGroup
{
                      EGGroup(void); // private constructor
public:
  void                SetAsDefault(){ m_pDefaultGroup = this; };
  void                AddObject(EGObject *pObj);
  void                RemoveObject(EGObject *pObj);
  void                RemoveAllObjects(void);
  void                FocusNext(void);
  void                FocusPrevious(void);
  void                FocusFreeze(bool Freeze);
  EG_Result_t         SendData(uint32_t Data);
  void                SetFocusCB(EG_GroupFocusCB_t FocusCB);
  void                SetEdgeCB(EG_GroupEdgeCB_t RdgeCB);
  void                SetRefocusPolicy(EGGroup *pGroup, EG_GroupRefocusPolicy_e Policy);
  void                SetEditing(bool Edit);
  void                SetWrap(bool Wrap);
  EGObject*           GetFocused(void);
  EG_GroupFocusCB_t   GetFocusCB(void);
  EG_GroupEdgeCB_t    GetEdgeCB(void);
  bool                GetEditing(void);
  bool                GetWrap(void);
  uint32_t            GetObjectCount(void);

  static EGGroup*     Create(void);
  static void         Delete(EGGroup *pGroup);
  static void         SwapObject(EGObject *pObj1, EGObject *pObj2);
  static void         FocusObject(EGObject *pObj);
  static EGGroup*     GetDefault(void){ return m_pDefaultGroup; };

  EGList                  m_ObjectList;         // Linked list to store the objects in the group
  EGObject               *m_pFocusedObject;       // The object in focus
#if EG_USE_USER_DATA
  void                   *m_pUserData;
#endif
  uint8_t                 m_Frozen : 1;         // 1: can't focus to new object
  uint8_t                 m_Editing : 1;        // 1: Edit mode, 0: Navigate mode
  uint8_t                 m_RefocusPolicy : 1;  // 1: Focus prev if focused on deletion. 0: Focus next if focused on deletion.
  uint8_t                 m_Wrap : 1;           // 1: Focus next/prev can wrap at end of list. 0: Focus next/prev stops at end of list.

private:

  bool                FocusCore(bool Direction);
  void                Refocus(void);
  EGInputDevice*      GetIndev(void);

  EG_GroupFocusCB_t     m_FocusCB;            
  EG_GroupEdgeCB_t      m_EdgeCB;             

  static EGGroup          *m_pDefaultGroup;
  static EGList           m_GroupList;         // Linked list to store the groups

};
