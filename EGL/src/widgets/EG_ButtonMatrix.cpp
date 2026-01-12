/*
 *        Copyright (Key) 2025-2026 HydraSystems..
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

#include "widgets/EG_ButtonMatrix.h"
#if EG_USE_BTNMATRIX != 0

#include "misc/EG_Assert.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Group.h"
#include "draw/EG_DrawContext.h"
#include "core/EG_Refresh.h"
#include "misc/EG_Text.h"
#include "misc/lv_txt_ap.h"

///////////////////////////////////////////////////////////////////////////////////////

#define BUTTONMATRIX_CLASS &c_ButtonMatrixClass

#define BTN_EXTRA_CLICK_AREA_MAX (EG_DPI_DEF / 10)
#define EG_BTNMATRIX_WIDTH_MASK 0x000F


static const char *lv_btnmatrix_def_map[] = {"Btn1", "Btn2", "Btn3", "\n", "Btn4", "Btn5", ""};

const EG_ClassType_t c_ButtonMatrixClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGButtonMatrix::EventCB,
	.WidthDef = EG_DPI_DEF * 2,
	.HeightDef = EG_DPI_DEF,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
	.pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGButtonMatrix::EGButtonMatrix(void) : EGObject()
{
	m_ButtonCount = 0;
	m_RowCount = 0;
	m_SelectID = EG_BTNMATRIX_BTN_NONE;
	m_pButtonRects = nullptr;
	m_pControlBits = nullptr;
	m_ppMap = nullptr;
	m_OneCheck = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

EGButtonMatrix::EGButtonMatrix(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_ButtonMatrixClass*/) : EGObject()
{
	m_ButtonCount = 0;
	m_RowCount = 0;
	m_SelectID = EG_BTNMATRIX_BTN_NONE;
	m_pButtonRects = nullptr;
	m_pControlBits = nullptr;
	m_ppMap = nullptr;
	m_OneCheck = 0;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGButtonMatrix::~EGButtonMatrix(void)
{
  m_ButtonCount = 0;
	EG_FreeMem(m_pButtonRects);
	EG_FreeMem(m_pControlBits);
	m_pButtonRects = nullptr;
	m_pControlBits = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::Configure(void)
{
  EGObject::Configure();
	m_ButtonCount = 0;
	m_RowCount = 0;
	m_SelectID = EG_BTNMATRIX_BTN_NONE;
	m_pButtonRects = nullptr;
	m_pControlBits = nullptr;
	m_ppMap = nullptr;
	m_OneCheck = 0;
	SetMap(lv_btnmatrix_def_map);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetMap(const char *pMap[])
{
	if(pMap == nullptr) return;
	AllocateMap(pMap);	// Analyze the map and create the required number of buttons
	m_ppMap = pMap;
	EG_BaseDirection_e Direction = GetStyleBaseDirection(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);	// Set size and positions of the buttons
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PadRow = GetStylePadRow(EG_PART_MAIN);
	EG_Coord_t PadColumn = GetStylePadColumn(EG_PART_MAIN);
	EG_Coord_t MaxWidth = GetContentWidth();
	EG_Coord_t MaxHeight = GetContentHeight();
	EG_Coord_t MaxHeightNoGap = MaxHeight - (PadRow * (m_RowCount - 1));	// Calculate the position of each Row

	// Count the units and the buttons in a line (A button can be 1,2,3... unit wide)
	uint32_t txt_tot_i = 0; // Act. index in the str map
	uint32_t btn_tot_i = 0; // Act. index of button areas
	const char **ppMapRow = pMap;

	// Count the units and the buttons in a line
	uint32_t Row;
	for(Row = 0; Row < m_RowCount; Row++) {
		uint16_t unit_cnt = 0; // Number of units in a Row
		uint16_t ButtonCount = 0;  // Number of buttons in a Row
		// Count the buttons and units in this Row
		while(ppMapRow[ButtonCount] && strcmp(ppMapRow[ButtonCount], "\n") != 0 && ppMapRow[ButtonCount][0] != '\0') {
			unit_cnt += GetButtonWidth(m_pControlBits[btn_tot_i + ButtonCount]);
			ButtonCount++;
		}
		if(ButtonCount == 0) {		// Only deal with the non empty lines
			ppMapRow = &ppMapRow[ButtonCount + 1]; // Set the map to the next Row
			continue;
		}
		EG_Coord_t RowY1 = PadTop + (MaxHeightNoGap * Row) / m_RowCount + Row * PadRow;
		EG_Coord_t RowY2 = PadTop + (MaxHeightNoGap * (Row + 1)) / m_RowCount + Row * PadRow - 1;

		// Set the button size and positions
		EG_Coord_t max_w_no_gap = MaxWidth - (PadColumn * (ButtonCount - 1));
		if(max_w_no_gap < 0) max_w_no_gap = 0;

		uint32_t row_unit_cnt = 0; // The current unit position in the Row
		uint32_t btn;
		for(btn = 0; btn < ButtonCount; btn++, btn_tot_i++, txt_tot_i++) {
			uint32_t btn_u = GetButtonWidth(m_pControlBits[btn_tot_i]);
			EG_Coord_t btn_x1 = (max_w_no_gap * row_unit_cnt) / unit_cnt + btn * PadColumn;
			EG_Coord_t btn_x2 = (max_w_no_gap * (row_unit_cnt + btn_u)) / unit_cnt + btn * PadColumn - 1;
			if(Direction == EG_BASE_DIR_RTL) {			// If RTL start from the right
				EG_Coord_t tmp = btn_x1;
				btn_x1 = btn_x2;
				btn_x2 = tmp;
				btn_x1 = MaxWidth - btn_x1;
				btn_x2 = MaxWidth - btn_x2;
			}
			btn_x1 += PadLeft;
			btn_x2 += PadLeft;
			m_pButtonRects[btn_tot_i].Set(btn_x1, RowY1, btn_x2, RowY2);
			row_unit_cnt += btn_u;
		}
		ppMapRow = &ppMapRow[ButtonCount + 1]; // Set the map to the next line
	}
	// Popovers in the top Row will draw outside the widget and the extended draw size...
  // depends on the Row height which may have changed when setting the new map
	RefreshExtDrawSize();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetControlMap(const EG_ButMatrixCtrl_t CtrlMap[])
{
	EG_CopyMem(m_pControlBits, CtrlMap, sizeof(EG_ButMatrixCtrl_t) * m_ButtonCount);
	SetMap(m_ppMap);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetSelected(uint16_t ButtonID)
{
	if(ButtonID >= m_ButtonCount && ButtonID != EG_BTNMATRIX_BTN_NONE) return;
	InvalidateButton(m_SelectID);
	m_SelectID = ButtonID;
	InvalidateButton(ButtonID);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetControl(uint16_t ButtonID, EG_ButMatrixCtrl_t Control)
{
	if(ButtonID >= m_ButtonCount) return;
	if(m_OneCheck && (Control & EG_BTNMATRIX_CTRL_CHECKED)) {
		ClearControlAll(EG_BTNMATRIX_CTRL_CHECKED);
	}
	//  If we hide a button if all buttons are now hidden hide the whole button matrix to make focus behave correctly 
	if(Control & EG_BTNMATRIX_CTRL_HIDDEN) {
		bool all_buttons_hidden = true;
		if(m_ButtonCount > 1) {
			for(uint16_t Index = 0; Index < m_ButtonCount; Index++) {
				if(Index == ButtonID) continue;
				if(!(m_pControlBits[Index] & EG_BTNMATRIX_CTRL_HIDDEN)) all_buttons_hidden = false;
			}
		}
		if(all_buttons_hidden) AddFlag(EG_OBJ_FLAG_HIDDEN);
	}
	m_pControlBits[ButtonID] |= Control;
	InvalidateButton(ButtonID);
	if(Control & EG_BTNMATRIX_CTRL_POPOVER) RefreshExtDrawSize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::ClearControl(uint16_t ButtonID, EG_ButMatrixCtrl_t Control)
{
	if(ButtonID >= m_ButtonCount) return;
	//  If all buttons were hidden the whole button matrix is hidden so we need to check and remove hidden flag if present 
	if(Control & EG_BTNMATRIX_CTRL_HIDDEN) if(HasFlagSet(EG_OBJ_FLAG_HIDDEN)) ClearFlag(EG_OBJ_FLAG_HIDDEN);
	m_pControlBits[ButtonID] &= (~Control);
	InvalidateButton(ButtonID);
	if(Control & EG_BTNMATRIX_CTRL_POPOVER) RefreshExtDrawSize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetControlAll(EG_ButMatrixCtrl_t Control)
{
	for(uint16_t i = 0; i < m_ButtonCount; i++) SetControl(i, Control);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::ClearControlAll(EG_ButMatrixCtrl_t Control)
{
	for(uint16_t i = 0; i < m_ButtonCount; i++) ClearControl(i, Control);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetButtonWidth(uint16_t ButtonID, uint8_t width)
{
	if(ButtonID >= m_ButtonCount) return;
	m_pControlBits[ButtonID] &= (~EG_BTNMATRIX_WIDTH_MASK);
	m_pControlBits[ButtonID] |= (EG_BTNMATRIX_WIDTH_MASK & width);
	SetMap(m_ppMap);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::SetOneChecked(bool State)
{
	m_OneCheck = State;
	MakeOneButtonChecked(0);	// If more than one button is toggled only the first one should be
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGButtonMatrix::GetButtonText(uint16_t ButtonID)
{
uint16_t TextIndex = 0, ButtonIndex = 0;

	if(ButtonID == EG_BTNMATRIX_BTN_NONE) return nullptr;
	if(ButtonID > m_ButtonCount) return nullptr;
	// Get the text of the selected button using the text map. Skip empty strings
	while(ButtonIndex != ButtonID) {
		ButtonIndex++;
		TextIndex++;
		if(strcmp(m_ppMap[TextIndex], "\n") == 0) TextIndex++;
	}
	if(ButtonIndex == m_ButtonCount) return nullptr;
	return m_ppMap[TextIndex];
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::HasControl(uint16_t ButtonID, EG_ButMatrixCtrl_t Control)
{
	if(ButtonID >= m_ButtonCount) return false;
	return ((m_pControlBits[ButtonID] & Control) == Control) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(BUTTONMATRIX_CLASS) != EG_RES_OK) return;// Call the ancestor's event handler
	EGButtonMatrix *pMatrix = (EGButtonMatrix*)pEvent->GetTarget();
  pMatrix->Event(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::Event(EGEvent *pEvent)
{
EG_Result_t Result;
EGPoint Point;

	EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_REFR_EXT_DRAW_SIZE: {
      if(HasPopoversInTopRow()) {	// reserve one Row worth of extra space to account for popovers in the top Row
        EG_Coord_t Size = m_RowCount > 0 ? GetContentHeight() / m_RowCount : 0;
        pEvent->SetExtDrawSize(Size);
      }
      break;
    }
    case EG_EVENT_STYLE_CHANGED:{
      SetMap(m_ppMap);
      break;
    }
    case EG_EVENT_SIZE_CHANGED:{
      SetMap(m_ppMap);
      break;
    }
    case EG_EVENT_PRESSED: {
      void *pParam = pEvent->GetParam();
       InvalidateButton(m_SelectID);
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      if(InputType == EG_INDEV_TYPE_POINTER || InputType == EG_INDEV_TYPE_BUTTON) {
        ((EGInputDevice *)pParam)->GetPoint(&Point);		// Search the pressed area
        uint16_t PressedButton = GetFromPoint(&Point);
        // Handle the case where there is no button there
        m_SelectID = EG_BTNMATRIX_BTN_NONE;
        if(PressedButton != EG_BTNMATRIX_BTN_NONE) {
          if(IsInactive(m_pControlBits[PressedButton]) == false &&
            IsHidden(m_pControlBits[PressedButton]) == false) {
            m_SelectID = PressedButton;
            InvalidateButton(m_SelectID); // Invalidate the new area
          }
        }
        else {
          m_SelectID = EG_BTNMATRIX_BTN_NONE;
        }
      }
      if(m_SelectID != EG_BTNMATRIX_BTN_NONE) {
        if(IsClickTrig(m_pControlBits[m_SelectID]) == false &&
          IsPopover(m_pControlBits[m_SelectID]) == false &&
          IsInactive(m_pControlBits[m_SelectID]) == false &&
          IsHidden(m_pControlBits[m_SelectID]) == false) {
          uint32_t b = m_SelectID;
          Result = EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, &b);
          if(Result != EG_RES_OK) return;
        }
      }
      break;
    }
    case EG_EVENT_PRESSING: {
      void *pParam = pEvent->GetParam();
      uint16_t PressedButton = EG_BTNMATRIX_BTN_NONE;
      EGInputDevice *pInput = EGInputDevice::GetActive();		// Search the pressed area
      EG_InDeviceType_e InputType = pInput->GetType();
      if(InputType == EG_INDEV_TYPE_ENCODER || InputType == EG_INDEV_TYPE_KEYPAD) return;
      pInput->GetPoint(&Point);
      PressedButton = GetFromPoint(&Point);
      if(PressedButton != m_SelectID) {		// Invalidate to old and the new areas
        if(m_SelectID != EG_BTNMATRIX_BTN_NONE) {
          InvalidateButton(m_SelectID);
        }
        m_SelectID = PressedButton;
        ((EGInputDevice *)pParam)->ResetLongPress(); // Start the log press time again on the new button
        if(PressedButton != EG_BTNMATRIX_BTN_NONE && IsInactive(m_pControlBits[PressedButton]) == false &&
          IsHidden(m_pControlBits[PressedButton]) == false) {
          InvalidateButton(PressedButton);
          // Send VALUE_CHANGED for the newly pressed button
          if(IsClickTrig(m_pControlBits[PressedButton]) == false &&
            IsPopover(m_pControlBits[PressedButton]) == false) {
            Result = EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, (uint32_t*)&PressedButton);
            if(Result != EG_RES_OK) return;
          }
        }
      }
      break;
    }
    case EG_EVENT_RELEASED: {
      if(m_SelectID != EG_BTNMATRIX_BTN_NONE) {
        if(IsCheckable(m_pControlBits[m_SelectID]) &&        // Toggle the button if enabled
          !IsInactive(m_pControlBits[m_SelectID])) {
          if(GetChecked(m_pControlBits[m_SelectID]) && !m_OneCheck) {
            m_pControlBits[m_SelectID] &= (~EG_BTNMATRIX_CTRL_CHECKED);
          }
          else m_pControlBits[m_SelectID] |= EG_BTNMATRIX_CTRL_CHECKED;
          if(m_OneCheck) MakeOneButtonChecked(m_SelectID);
        }
        if((IsClickTrig(m_pControlBits[m_SelectID]) == true ||
            IsPopover(m_pControlBits[m_SelectID]) == true) &&
            IsInactive(m_pControlBits[m_SelectID]) == false &&
            IsHidden(m_pControlBits[m_SelectID]) == false) {
          Result = EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, (uint32_t*)&m_SelectID);
          if(Result != EG_RES_OK) return;
        }
      }
      InvalidateButton(m_SelectID);		// Invalidate to old pressed area;
      break;
    }
    case EG_EVENT_LONG_PRESSED_REPEAT: {
      if(m_SelectID != EG_BTNMATRIX_BTN_NONE) {
        if(IsRepeatDisabled(m_pControlBits[m_SelectID]) == false &&
          IsInactive(m_pControlBits[m_SelectID]) == false &&
          IsHidden(m_pControlBits[m_SelectID]) == false) {
          Result = EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, (uint32_t*)&m_SelectID);
          if(Result != EG_RES_OK) return;
        }
      }
      break;
    }
    case EG_EVENT_PRESS_LOST: {
      InvalidateButton(m_SelectID);
      m_SelectID = EG_BTNMATRIX_BTN_NONE;
      break;
    }
    case EG_EVENT_FOCUSED: {
      if(m_ButtonCount == 0) return;
      EGInputDevice *pInput = (EGInputDevice *)pEvent->GetParam();
      EG_InDeviceType_e InputType = pInput->GetType();
      if(pInput == nullptr) {		// If not focused by an input device assume the last input device
        pInput = EGInputDevice::GetNext(nullptr);
        InputType = pInput->GetType();
      }
      EGGroup *pGroup = (EGGroup*)GetGroup();
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
      if(m_SelectID == EG_BTNMATRIX_BTN_NONE) {		// Focus the first button if there is not selected button
        if(InputType == EG_INDEV_TYPE_KEYPAD || (InputType == EG_INDEV_TYPE_ENCODER && IsEditing)) {
          uint32_t Index = 0;
          if(m_OneCheck) {
            while(IsHidden(m_pControlBits[Index]) || IsInactive(m_pControlBits[Index]) ||
              IsChecked(m_pControlBits[Index]) == false) Index++;
          }
          else {
            while(IsHidden(m_pControlBits[Index]) || IsInactive(m_pControlBits[Index])) Index++;
          }
          m_SelectID = Index;
        }
        else {
          m_SelectID = EG_BTNMATRIX_BTN_NONE;
        }
      }
      break;
    }
    case EG_EVENT_DEFOCUSED:
    case EG_EVENT_LEAVE: {
      if(m_SelectID != EG_BTNMATRIX_BTN_NONE) InvalidateButton(m_SelectID);
      m_SelectID = EG_BTNMATRIX_BTN_NONE;
      break;
    }
    case EG_EVENT_KEY: {
      InvalidateButton(m_SelectID);
      char Key = *((char *)pEvent->GetParam());
      switch(Key){
        case EG_KEY_RIGHT: {
          if(m_SelectID == EG_BTNMATRIX_BTN_NONE)	m_SelectID = 0;
          else m_SelectID++;
          if(m_SelectID >= m_ButtonCount) m_SelectID = 0;
          uint16_t StartID = m_SelectID;
          while(IsHidden(m_pControlBits[m_SelectID]) || IsInactive(m_pControlBits[m_SelectID])) {
            m_SelectID++;
            if(m_SelectID >= m_ButtonCount) m_SelectID = 0;
            if(m_SelectID == StartID) {
              m_SelectID = EG_BTNMATRIX_BTN_NONE;
              break;
            }
          }
          break;
        }
        case EG_KEY_LEFT: {
          if(m_SelectID == EG_BTNMATRIX_BTN_NONE) m_SelectID = 0;
          if(m_SelectID == 0)	m_SelectID = m_ButtonCount - 1;
          else if(m_SelectID > 0)	m_SelectID--;
          uint16_t StartID = m_SelectID;
          while(IsHidden(m_pControlBits[m_SelectID]) || IsInactive(m_pControlBits[m_SelectID])) {
            if(m_SelectID > 0) m_SelectID--;
            else m_SelectID = m_ButtonCount - 1;
            if(m_SelectID == StartID) {
              m_SelectID = EG_BTNMATRIX_BTN_NONE;
              break;
            }
          }
          break;
        }
        case EG_KEY_DOWN: {
          EG_Coord_t ColumnGap = GetStylePadColumn(EG_PART_MAIN);
          if(m_SelectID == EG_BTNMATRIX_BTN_NONE) {        // Find the area below the current
            m_SelectID = 0;
            while(IsHidden(m_pControlBits[m_SelectID]) || IsInactive(m_pControlBits[m_SelectID])) {
              m_SelectID++;
              if(m_SelectID >= m_ButtonCount) {
                m_SelectID = EG_BTNMATRIX_BTN_NONE;
                break;
              }
            }
          }
          else {
            uint16_t Index;
            EG_Coord_t PressPoint = m_pButtonRects[m_SelectID].GetX1() + (m_pButtonRects[m_SelectID].GetWidth() >> 1);
            for(Index = m_SelectID; Index < m_ButtonCount; Index++) {
              if(m_pButtonRects[Index].GetY1() > m_pButtonRects[m_SelectID].GetY1() &&
                PressPoint >= m_pButtonRects[Index].GetX1() &&
                PressPoint <= m_pButtonRects[Index].GetX2() + ColumnGap &&
                IsInactive(m_pControlBits[Index]) == false &&
                IsHidden(m_pControlBits[Index]) == false) {
                break;
              }
            }
            if(Index < m_ButtonCount) m_SelectID = Index;
          }
          break;
        }
        case EG_KEY_UP: {
          EG_Coord_t ColumnGap = GetStylePadColumn(EG_PART_MAIN);
          if(m_SelectID == EG_BTNMATRIX_BTN_NONE) {			// Find the area below the current
            m_SelectID = 0;
            while(IsHidden(m_pControlBits[m_SelectID]) || IsInactive(m_pControlBits[m_SelectID])) {
              m_SelectID++;
              if(m_SelectID >= m_ButtonCount) {
                m_SelectID = EG_BTNMATRIX_BTN_NONE;
                break;
              }
            }
          }
          else {
            int16_t Index;
            EG_Coord_t PressPoint = m_pButtonRects[m_SelectID].GetX1() + (m_pButtonRects[m_SelectID].GetWidth() >> 1);
            for(Index = m_SelectID; Index >= 0; Index--) {
              if(m_pButtonRects[Index].GetY1() < m_pButtonRects[m_SelectID].GetY1() &&
                PressPoint >= m_pButtonRects[Index].GetX1() - ColumnGap &&
                PressPoint <= m_pButtonRects[Index].GetX2() &&
                IsInactive(m_pControlBits[Index]) == false &&
                IsHidden(m_pControlBits[Index]) == false) {
                break;
              }
            }
            if(Index >= 0) m_SelectID = Index;
          }
          break;
        }
        default: break;
      }
      InvalidateButton(m_SelectID);
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      Draw(pEvent);
      break;
    }
    default: break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::Draw(EGEvent *pEvent)
{
EGRect ObjRect(m_Rect), ButtonRect;
uint16_t ButtonIndex = 0, TextIndex = 0;
EGDrawRect ActiveDrawRect, DefaultDrawRect;
EGDrawLabel ActivDrawLabel, DefaultDrawLabel;

	if(m_ButtonCount == 0) return;
	EGDrawContext *pDrawContext = pEvent->GetDrawContext();
	m_SkipTransition = 1;
	EGState_t OriginalState = m_State;
	m_State = EG_STATE_DEFAULT;
	m_SkipTransition = 1;
	InititialseDrawRect(EG_PART_ITEMS, &DefaultDrawRect);
	InititialseDrawLabel(EG_PART_ITEMS, &DefaultDrawLabel);
	m_SkipTransition = 0;
	m_State = OriginalState;
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_MAIN);
#if EG_USE_ARABIC_PERSIAN_CHARS
	const size_t txt_ap_size = 256;
	char *txt_ap = EG_GetBufferMem(txt_ap_size);
#endif
	EGDrawDiscriptor PartDrawDiscriptor;
	InitDrawDescriptor(&PartDrawDiscriptor, pDrawContext);
	PartDrawDiscriptor.m_Part = EG_PART_ITEMS;
	PartDrawDiscriptor.m_pClass = m_pClass;
	PartDrawDiscriptor.m_Type = EG_BTNMATRIX_DRAW_PART_BTN;
	PartDrawDiscriptor.m_pDrawRect = &ActiveDrawRect;
	PartDrawDiscriptor.m_pDrawLabel = &ActivDrawLabel;
	for(ButtonIndex = 0; ButtonIndex < m_ButtonCount; ButtonIndex++, TextIndex++) {
		while(strcmp(m_ppMap[TextIndex], "\n") == 0) TextIndex++;		// Search the next valid text in the map
		if(IsHidden(m_pControlBits[ButtonIndex])) continue;		// Skip hidden buttons
		EGState_t State = EG_STATE_DEFAULT;		// Get the m_State of the button
		if(GetChecked(m_pControlBits[ButtonIndex])) State |= EG_STATE_CHECKED;
		if(IsInactive(m_pControlBits[ButtonIndex]))	State |= EG_STATE_DISABLED;
		else if(ButtonIndex == m_SelectID) {
			if(OriginalState & EG_STATE_PRESSED) State |= EG_STATE_PRESSED;
			if(OriginalState & EG_STATE_FOCUSED) State |= EG_STATE_FOCUSED;
			if(OriginalState & EG_STATE_FOCUS_KEY) State |= EG_STATE_FOCUS_KEY;
			if(OriginalState & EG_STATE_EDITED) State |= EG_STATE_EDITED;
		}
		m_pButtonRects[ButtonIndex].Copy(&ButtonRect);		// Get the button's area
		ButtonRect.IncX1(ObjRect.GetX1());
		ButtonRect.IncY1(ObjRect.GetY1());
		ButtonRect.IncX2(ObjRect.GetX1());
		ButtonRect.IncY2(ObjRect.GetY1());
		if(State == EG_STATE_DEFAULT) {		// Set up the draw descriptors
			ActiveDrawRect = DefaultDrawRect;
			ActivDrawLabel = DefaultDrawLabel;
		}
		else {		// In other cases get the styles directly without caching them
			m_State = State;
			m_SkipTransition = 1;
			ActiveDrawRect.Reset();
			ActivDrawLabel.Reset();
			InititialseDrawRect(EG_PART_ITEMS, &ActiveDrawRect);
			InititialseDrawLabel(EG_PART_ITEMS, &ActivDrawLabel);
			m_State = OriginalState;
			m_SkipTransition = 0;
		}
		if(IsRecolor(m_pControlBits[ButtonIndex])) ActivDrawLabel.m_Flag |= EG_TEXT_FLAG_RECOLOR;
		else ActivDrawLabel.m_Flag &= ~EG_TEXT_FLAG_RECOLOR;
		PartDrawDiscriptor.m_pRect = &ButtonRect;
		PartDrawDiscriptor.m_Index = ButtonIndex;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
		if(ActiveDrawRect.m_BorderSide & EG_BORDER_SIDE_INTERNAL) {		// Remove borders on the edges if `EG_BORDER_SIDE_INTERNAL`
			ActiveDrawRect.m_BorderSide = EG_BORDER_SIDE_FULL;
			if(ButtonRect.GetX1() == m_Rect.GetX1() + PadLeft) ActiveDrawRect.m_BorderSide &= ~EG_BORDER_SIDE_LEFT;
			if(ButtonRect.GetX2() == m_Rect.GetX2() - PadRight) ActiveDrawRect.m_BorderSide &= ~EG_BORDER_SIDE_RIGHT;
			if(ButtonRect.GetY1() == m_Rect.GetY1() + PadTop) ActiveDrawRect.m_BorderSide &= ~EG_BORDER_SIDE_TOP;
			if(ButtonRect.GetY2() == m_Rect.GetY2() - PadBottom) ActiveDrawRect.m_BorderSide &= ~EG_BORDER_SIDE_BOTTOM;
		}
    EG_Coord_t ButtonHeight = ButtonRect.GetHeight();
		if((State & EG_STATE_PRESSED) && (m_pControlBits[ButtonIndex] & EG_BTNMATRIX_CTRL_POPOVER)) {
			ButtonRect.DecY1(ButtonRect.GetHeight());			// Push up the upper boundary of the btn area to create the popover
		}
		ActiveDrawRect.Draw(pDrawContext, &ButtonRect);		// Draw the background
		const EG_Font_t *pFont = ActivDrawLabel.m_pFont;		// Calculate the size of the text
		EG_Coord_t Kerning = ActivDrawLabel.m_Kerning;
		EG_Coord_t LineSpace = ActivDrawLabel.m_LineSpace;
		const char *pString = m_ppMap[TextIndex];
#if EG_USE_ARABIC_PERSIAN_CHARS
		size_t len_ap = _lv_txt_ap_calc_bytes_cnt(pString);		// Get the size of the Arabic text and process it
		if(len_ap < txt_ap_size) {
			_lv_txt_ap_proc(pString, txt_ap);
			pString = txt_ap;
		}
#endif
		EGPoint StringSize;
		EG_GetTextSize(&StringSize, pString, pFont, Kerning,	LineSpace, ObjRect.GetWidth(), ActivDrawLabel.m_Flag);
		ButtonRect.IncX1((ButtonRect.GetWidth() - StringSize.m_X) / 2);
		ButtonRect.IncY1((ButtonRect.GetHeight() - StringSize.m_Y) / 2);
		ButtonRect.SetX2(ButtonRect.GetX1() + StringSize.m_X);
		ButtonRect.SetY2(ButtonRect.GetY1() + StringSize.m_Y);
		if((State & EG_STATE_PRESSED) && (m_pControlBits[ButtonIndex] & EG_BTNMATRIX_CTRL_POPOVER)) {
			ButtonRect.DecY1(ButtonHeight / 2);			// Push up the button text into the popover
			ButtonRect.DecY2(ButtonHeight / 2);
		}
		ActivDrawLabel.Draw(pDrawContext, &ButtonRect, pString, nullptr);		// Draw the text
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
	}
	m_SkipTransition = 0;
#if EG_USE_ARABIC_PERSIAN_CHARS
	EG_ReleaseBufferMem(txt_ap);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::AllocateMap(const char **ppMap)
{
uint16_t i = 0, ButtonCount = 0;	// Count the buttons in the map

	m_RowCount = 1;
	while(ppMap[i] && ppMap[i][0] != '\0') {
		if(strcmp(ppMap[i], "\n") != 0) ButtonCount++; // Do not count line breaks
		else m_RowCount++;
		i++;
	}
	if(ButtonCount == m_ButtonCount) return;	// Do not allocate memory for the same amount of buttons
	if(m_pButtonRects != nullptr) {
		EG_FreeMem(m_pButtonRects);
		m_pButtonRects = nullptr;
	}
	if(m_pControlBits != nullptr) {
		EG_FreeMem(m_pControlBits);
		m_pControlBits = nullptr;
	}
	m_pButtonRects = (EGRect *)EG_AllocMem(sizeof(EGRect) * ButtonCount);
	EG_ASSERT_MALLOC(m_pButtonRects);
	m_pControlBits = (EG_ButMatrixCtrl_t *)EG_AllocMem(sizeof(EG_ButMatrixCtrl_t) * ButtonCount);
	EG_ASSERT_MALLOC(m_pControlBits);
	if(m_pButtonRects == nullptr || m_pControlBits == nullptr) ButtonCount = 0;
	EG_ZeroMem(m_pControlBits, sizeof(EG_ButMatrixCtrl_t) * ButtonCount);
	m_ButtonCount = ButtonCount;
}

///////////////////////////////////////////////////////////////////////////////////////

uint8_t EGButtonMatrix::GetButtonWidth(EG_ButMatrixCtrl_t ControlBits)
{
	uint8_t Width = ControlBits & EG_BTNMATRIX_WIDTH_MASK;
	return Width != 0 ? Width : 1;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsHidden(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_HIDDEN) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsChecked(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_CHECKED) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsRepeatDisabled(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_NO_REPEAT) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsInactive(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_DISABLED) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsClickTrig(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_CLICK_TRIG) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsPopover(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_POPOVER) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsCheckable(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_CHECKABLE) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::GetChecked(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_CHECKED) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::IsRecolor(EG_ButMatrixCtrl_t ControlBits)
{
	return (ControlBits & EG_BTNMATRIX_CTRL_RECOLOR) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGButtonMatrix::GetFromPoint(EGPoint *pPoint)
{
uint16_t i;

	EGRect ObjRect = m_Rect;
	EG_Coord_t Width = GetWidth();
	EG_Coord_t Height = GetHeight();
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t PadRow = GetStylePadRow(EG_PART_MAIN);
	EG_Coord_t PadColumn = GetStylePadColumn(EG_PART_MAIN);
	PadRow = (PadRow / 2) + 1 + (PadRow & 1);	// Get the half gap. Button look larger with this value. (+1 for rounding error)
	PadColumn = (PadColumn / 2) + 1 + (PadColumn & 1);
	PadRow = EG_MIN(PadRow, BTN_EXTRA_CLICK_AREA_MAX);
	PadColumn = EG_MIN(PadColumn, BTN_EXTRA_CLICK_AREA_MAX);
	PadRight = EG_MIN(PadRight, BTN_EXTRA_CLICK_AREA_MAX);
	PadTop = EG_MIN(PadTop, BTN_EXTRA_CLICK_AREA_MAX);
	PadBottom = EG_MIN(PadBottom, BTN_EXTRA_CLICK_AREA_MAX);
	for(i = 0; i < m_ButtonCount; i++) {
		EGRect ButtonRect = m_pButtonRects[i];
		if(ButtonRect.GetX1() <= PadLeft)	ButtonRect.IncX1(ObjRect.GetX1() - EG_MIN(PadLeft, BTN_EXTRA_CLICK_AREA_MAX));
		else ButtonRect.IncX1(ObjRect.GetX1() - PadColumn);
		if(ButtonRect.GetY1() <= PadTop)	ButtonRect.IncY1(ObjRect.GetY1() - EG_MIN(PadTop, BTN_EXTRA_CLICK_AREA_MAX));
		else ButtonRect.IncY1(ObjRect.GetY1() - PadRow);
		if(ButtonRect.GetX2() >= Width - PadRight - 2) ButtonRect.IncX2(ObjRect.GetX1() + EG_MIN(PadRight, BTN_EXTRA_CLICK_AREA_MAX)); // -2 for rounding error
		else ButtonRect.IncX2(ObjRect.GetX1() + PadColumn);
		if(ButtonRect.GetY2() >= Height - PadBottom - 2)	ButtonRect.IncY2(ObjRect.GetY1() + EG_MIN(PadBottom, BTN_EXTRA_CLICK_AREA_MAX)); // -2 for rounding error
		else ButtonRect.IncY2(ObjRect.GetY1() + PadRow);
	if(ButtonRect.IsPointIn(pPoint, 0) != false) break;
	}
	if(i == m_ButtonCount) i = EG_BTNMATRIX_BTN_NONE;
	return i;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::InvalidateButton(uint16_t Index)
{
	if((Index == EG_BTNMATRIX_BTN_NONE) || (Index >= m_ButtonCount)) return;
	EGRect ButtonRect(m_pButtonRects[Index]);
	EGRect ObjRect;
	m_Rect.Copy(&ObjRect);
	/*The buttons might have outline and shadow so make the invalidation larger with the gaps between the buttons.
     *It assumes that the outline or shadow is smaller than the gaps*/
	EG_Coord_t RowGap = GetStylePadRow(EG_PART_MAIN);
	EG_Coord_t ColumnGap = GetStylePadColumn(EG_PART_MAIN);
	EG_Coord_t DPI = EGDisplay::GetDPI(GetDisplay());	// Be sure to have a minimal extra space if Row/ColumnGap is small
	RowGap = EG_MAX(RowGap, DPI / 10);
	ColumnGap = EG_MAX(ColumnGap, DPI / 10);
	ButtonRect.IncX1(ObjRect.GetX1() - RowGap);	// Convert relative coordinates to absolute
	ButtonRect.IncY1(ObjRect.GetY1() - ColumnGap);
	ButtonRect.IncX2(ObjRect.GetX1() + RowGap);
	ButtonRect.IncY2(ObjRect.GetY1() + ColumnGap);
	if((Index == m_SelectID) && (m_pControlBits[Index] & EG_BTNMATRIX_CTRL_POPOVER)) {
		ButtonRect.DecY1(ButtonRect.GetHeight());	// Push up the upper boundary of the btn area to also invalidate the popover
	}
	InvalidateArea(&ButtonRect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGButtonMatrix::MakeOneButtonChecked(uint16_t Index)
{
	bool ToggleState = HasControl(Index, EG_BTNMATRIX_CTRL_CHECKED);	// Save whether the button was toggled
	ClearControlAll(EG_BTNMATRIX_CTRL_CHECKED);
	if(ToggleState) SetControl(Index, EG_BTNMATRIX_CTRL_CHECKED);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGButtonMatrix::HasPopoversInTopRow(void)
{
	if(m_RowCount <= 0)	return false;
	const char **ppMapRow = m_ppMap;
	uint16_t ButtonCount = 0;
	while(ppMapRow[ButtonCount] && strcmp(ppMapRow[ButtonCount], "\n") != 0 && ppMapRow[ButtonCount][0] != '\0') {
		if(IsPopover(m_pControlBits[ButtonCount])) {
			return true;
		}
		ButtonCount++;
	}
	return false;
}

#endif
