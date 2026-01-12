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

#if EG_USE_DROPDOWN != 0

// Testing of dependencies

#if EG_USE_LABEL == 0
#error "lv_dropdown: lv_label is required. Enable it in EG_Config.h (EG_USE_LABEL 1)"
#endif

#include "../widgets/EG_Label.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_DROPDOWN_POS_LAST 0xFFFF
EG_EXPORT_CONST_INT(EG_DROPDOWN_POS_LAST);

///////////////////////////////////////////////////////////////////////////////////////

extern const  EG_ClassType_t c_DropDownClass;
extern const  EG_ClassType_t c_DropDownListClass;

class EGDropDown;

///////////////////////////////////////////////////////////////////////////////////////

class EGDropDownList : public EGObject
{
public:
                    EGDropDownList(void);
                    EGDropDownList(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_DropDownListClass);
  virtual           ~EGDropDownList(void);
  virtual void      Configure(void);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  EGDropDown       *m_pDropDown;
  EGLabel          *m_pLabel;
  
private:
  void              Draw(EGEvent *pEvent);
  void              PressHandler(void);
  EG_Result_t       ReleaseHandler(void);
};

///////////////////////////////////////////////////////////////////////////////////////

class EGDropDown : public EGObject
{
public:
                    EGDropDown(void);
                    EGDropDown(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_DropDownClass);
  virtual           ~EGDropDown(void);
  virtual void      Configure(void);
  void              SetText(const char *pText);
  void              SetItems(const char *pItems);
  void              SetStaticItems(const char *pItems);
  void              AddItems(const char *pItems, uint32_t Index);
  void              ClearItems(void);
  void              SetSelectedIndex(uint16_t Index);
  void              SetDirection(EG_DirType_e Direction);
  void              SetSymbol(const void *pSymbol);
  void              SetHighlight(bool Enable);
  EGDropDownList*   GetList(void){ return m_pDropList; };
  const char*       GetText(void){ return m_pText; };
  const char*       GetItems(void){ return (m_pItems == nullptr) ? "" : m_pItems; };
  uint16_t          GetSelectedIndex(void){ return m_SelectedIndex; };
  uint16_t          GetItemCount(void){ return m_ItemCount; };
  void              GetSelectedText(char *pBuffer, uint32_t Size);
  int32_t           GetItemIndex(const char *pItem);
  const char*       GetSymbol(void);
  bool              GetHighlight(void);
  EG_DirType_e      GetDirection(void);
  void              Open(void);
  void              Close(void);
  bool              IsOpen(void);
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  EGDropDownList    *m_pDropList;       // The dropped down list
  const char        *m_pText;           // Text to display on the dropdown's button
  const void        *m_pSymbol;         // Arrow or other icon when the drop-down list is closed
  char              *m_pItems;          // Options in a '\n' separated list
  uint16_t          m_ItemCount;        // Number of options
  uint16_t          m_SelectedIndex;    // Index of the currently selected option
  uint16_t          m_FocusIndex;       // Store the original index on focus
  uint16_t          m_PressedIndex;     // Index of the currently pressed option
  EG_DirType_e      m_Direction   : 4;  // Direction in which the list should open
  uint8_t           m_StaticText  : 1;  // 1: Only a pointer is saved in `options`
  uint8_t           m_Highlight   : 1;  // 1: Make the selected option highlighted in the list

private:
  void              DrawMain(EGEvent *pEvent);
  void              DrawBox(EGDrawContext *pContext, uint16_t Id, EGState_t State);
  void              DrawBoxLabel(EGDrawContext *pContext, uint16_t ID, EGState_t State);
  uint16_t          GetIndexOnPoint(EG_Coord_t Y);
  EG_Result_t       ButtonReleaseHandler(void);
  void              PositionToSelected(void);
  EGLabel*          GetLabel(void);

  friend class EGDropDownList;

};

#endif 