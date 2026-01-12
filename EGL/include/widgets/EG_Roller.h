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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

#if EG_USE_ROLLER != 0

#if EG_USE_LABEL == 0 // Testing of dependencies
#error "lv_roller: lv_label is required. Enable it in EG_Config.h (EG_USE_ROLLER 1)"
#endif

#include "../core/EG_Object.h"
#include "EG_Label.h"

///////////////////////////////////////////////////////////////////////////////////////

enum {
    EG_ROLLER_MODE_NORMAL,    //  Normal mode (roller ends at the end of the options).
    EG_ROLLER_MODE_INFINITE,  //  Infinite mode (roller can be scrolled forever).
};

typedef uint8_t EG_RollerMode_t;

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_RollerClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGRoller : public EGObject
{
public:
                      EGRoller(void);
                      EGRoller(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_RollerClass);
  virtual void        Configure(void);

  void                SetItems(const char *pItems, EG_RollerMode_t Mode);
  void                SetSelected(uint16_t SelectedItem, EG_AnimateEnable_e Enable);
  void                SetVisibleRowCount(uint8_t RowCount);
  uint16_t            GetSelected(void);
  void                GetSelectedText(char *pBuffer, uint32_t BufferSize);
  const char*         GetItems(void);
  uint16_t            GetItemCount(void);
  void                Event(EGEvent *pEvent);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void         LabelEventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void         ScrollAnimateEndCB(EGAnimate *pAnimate);
  static void         SetAnimateY(void *pObj, int32_t v);

  uint16_t            m_ItemCount;          //  Number of options
  uint16_t            m_CurrentItemIndex;   //  Index of the current option
  uint16_t            m_FocusedItemIndex;   //  Store the original index on focus
  EG_RollerMode_t     m_Mode : 1;
  uint32_t            m_IsMoved : 1;

private:
  void                DrawMain(EGEvent *pEvent);
  void                DrawLabel(EGEvent *pEvent, EGLabel *pLabel);
  void                GetSelectionRect(EGRect *pRect);
  void                RefreshPosition(EG_AnimateEnable_e Animate);
  EG_Result_t         ReleaseHandler(void);
  void                Normalize(void);
  EGLabel*            GetLabel(void);
  EG_Coord_t          GetSelectedLabelWidth(void);

};


#endif 