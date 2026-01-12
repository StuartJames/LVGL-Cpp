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

#include "extra/widgets/EG_Keyboard.h"
#if EG_USE_KEYBOARD

#include "widgets/EG_Edit.h"
#include "misc/EG_Assert.h"

#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////

#define KEYBOARD_CLASS &c_KeyboardClass
#define LV_KB_BTN(width) EG_BTNMATRIX_CTRL_POPOVER | width

const EG_ClassType_t c_KeyboardClass = {
  .pBaseClassType = &c_ButtonMatrixClass,
	.pEventCB = nullptr,
	.WidthDef = _EG_PCT(100),
	.HeightDef = _EG_PCT(50),
	.IsEditable = 1,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
  .pExtData = NULL,
#endif
};

static const char *const default_kb_map_lc[] = {"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", EG_SYMBOL_BACKSPACE, "\n",
																								"ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", EG_SYMBOL_NEW_LINE, "\n",
																								"_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
																								EG_SYMBOL_KEYBOARD, EG_SYMBOL_LEFT, " ", EG_SYMBOL_RIGHT, EG_SYMBOL_OK, ""};

static const EG_ButMatrixCtrl_t default_kb_ctrl_lc_map[] = {
	EG_KEYBOARD_CTRL_BTN_FLAGS | 5, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), EG_BTNMATRIX_CTRL_CHECKED | 7,
	EG_KEYBOARD_CTRL_BTN_FLAGS | 6, LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), EG_BTNMATRIX_CTRL_CHECKED | 7,
	EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1),
	EG_KEYBOARD_CTRL_BTN_FLAGS | 2, EG_BTNMATRIX_CTRL_CHECKED | 2, 6, EG_BTNMATRIX_CTRL_CHECKED | 2, EG_KEYBOARD_CTRL_BTN_FLAGS | 2};

static const char *const default_kb_map_uc[] = {"1#", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", EG_SYMBOL_BACKSPACE, "\n",
																								"abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", EG_SYMBOL_NEW_LINE, "\n",
																								"_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":", "\n",
																								EG_SYMBOL_KEYBOARD, EG_SYMBOL_LEFT, " ", EG_SYMBOL_RIGHT, EG_SYMBOL_OK, ""};

static const EG_ButMatrixCtrl_t default_kb_ctrl_uc_map[] = {
	EG_KEYBOARD_CTRL_BTN_FLAGS | 5, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), EG_BTNMATRIX_CTRL_CHECKED | 7,
	EG_KEYBOARD_CTRL_BTN_FLAGS | 6, LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), LV_KB_BTN(3), EG_BTNMATRIX_CTRL_CHECKED | 7,
	EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | LV_KB_BTN(1),
	EG_KEYBOARD_CTRL_BTN_FLAGS | 2, EG_BTNMATRIX_CTRL_CHECKED | 2, 6, EG_BTNMATRIX_CTRL_CHECKED | 2, EG_KEYBOARD_CTRL_BTN_FLAGS | 2};

static const char *const default_kb_map_spec[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", EG_SYMBOL_BACKSPACE, "\n",
																									"abc", "+", "&", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
																									"\\", "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'", "\n",
																									EG_SYMBOL_KEYBOARD, EG_SYMBOL_LEFT, " ", EG_SYMBOL_RIGHT, EG_SYMBOL_OK, ""};

static const EG_ButMatrixCtrl_t default_kb_ctrl_spec_map[] = {
	LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), EG_BTNMATRIX_CTRL_CHECKED | 2,
	EG_KEYBOARD_CTRL_BTN_FLAGS | 2, LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
	LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1), LV_KB_BTN(1),
	EG_KEYBOARD_CTRL_BTN_FLAGS | 2, EG_BTNMATRIX_CTRL_CHECKED | 2, 6, EG_BTNMATRIX_CTRL_CHECKED | 2, EG_KEYBOARD_CTRL_BTN_FLAGS | 2};

static const char *const default_kb_map_num[] = {"1", "2", "3", EG_SYMBOL_KEYBOARD, "\n",
																								 "4", "5", "6", EG_SYMBOL_OK, "\n",
																								 "7", "8", "9", EG_SYMBOL_BACKSPACE, "\n",
																								 "+/-", "0", ".", EG_SYMBOL_LEFT, EG_SYMBOL_RIGHT, ""};

static const EG_ButMatrixCtrl_t default_kb_ctrl_num_map[] = {
	1, 1, 1, EG_KEYBOARD_CTRL_BTN_FLAGS | 2,
	1, 1, 1, EG_KEYBOARD_CTRL_BTN_FLAGS | 2,
	1, 1, 1, 2,
	1, 1, 1, 1, 1};

static const char **kb_map[9] = {
	(const char **)default_kb_map_lc,
	(const char **)default_kb_map_uc,
	(const char **)default_kb_map_spec,
	(const char **)default_kb_map_num,
	(const char **)default_kb_map_lc,
	(const char **)default_kb_map_lc,
	(const char **)default_kb_map_lc,
	(const char **)default_kb_map_lc,
	(const char **)NULL,
};

static const EG_ButMatrixCtrl_t *kb_ctrl[9] = {
	default_kb_ctrl_lc_map,
	default_kb_ctrl_uc_map,
	default_kb_ctrl_spec_map,
	default_kb_ctrl_num_map,
	default_kb_ctrl_lc_map,
	default_kb_ctrl_lc_map,
	default_kb_ctrl_lc_map,
	default_kb_ctrl_lc_map,
	NULL,
};

///////////////////////////////////////////////////////////////////////////////////////

EGKeyboard::EGKeyboard(void) : EGButtonMatrix(),
 m_pEditCtrl(nullptr),
 m_Mode(EG_KEYBOARD_MODE_TEXT_LOWER),
 m_Popovers(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGKeyboard::EGKeyboard(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_LedClass*/) : EGButtonMatrix(),
 m_pEditCtrl(nullptr),
 m_Mode(EG_KEYBOARD_MODE_TEXT_LOWER),
 m_Popovers(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::Configure(void)
{
  EGButtonMatrix::Configure();
	ClearFlag(EG_OBJ_FLAG_CLICK_FOCUSABLE);
	m_pEditCtrl = NULL;
	m_Mode = EG_KEYBOARD_MODE_TEXT_LOWER;
	m_Popovers = 0;
	Align(EG_ALIGN_BOTTOM_MID, 0, 0);
	EGEvent::AddEventCB(this, EGKeyboard::EventCB, EG_EVENT_VALUE_CHANGED, nullptr);
	SetStyleBaseDirection(EG_BASE_DIR_LTR, 0);
	UpdateMap();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::SetEditCtrl(EGEdit *pEditCtrl)
{
//	if(pEditCtrl){
		EG_ASSERT_OBJ(pEditCtrl, &c_EditClass);
//	}
	if(m_pEditCtrl){	// Hide the cursor of the old Text area if cursor management is enabled
		ClearState(EG_STATE_FOCUSED);
	}
	m_pEditCtrl = pEditCtrl;
	if(m_pEditCtrl){	// Show the cursor of the new Text area if cursor management is enabled
		AddFlag(EG_STATE_FOCUSED);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::SetMode(EG_KeyboardMode_e Mode)
{
	if(m_Mode == Mode) return;
	m_Mode = Mode;
	UpdateMap();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::SetPopoverModes(bool Enable)
{
	if(m_Popovers == Enable) return;
	m_Popovers = Enable;
	UpdateCtrlMap();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::SetMaps(EG_KeyboardMode_e Mode, const char *pMap[], const EG_ButMatrixCtrl_t CtrlMap[])
{
	kb_map[Mode] = pMap;
	kb_ctrl[Mode] = CtrlMap;
	UpdateMap();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::EventCB(EGEvent *pEvent)
{
	EGKeyboard *pKeyboard = (EGKeyboard*)pEvent->GetTarget();
  pKeyboard->Event(pEvent); // Dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::Event(EGEvent *pEvent)
{

	if(GetSelected() == EG_BTNMATRIX_BTN_NONE) return;
	const char *pText = GetButtonText(GetSelected());
	if(pText == NULL) return;
	if(strcmp(pText, "abc") == 0) {
		m_Mode = EG_KEYBOARD_MODE_TEXT_LOWER;
		SetMap(kb_map[EG_KEYBOARD_MODE_TEXT_LOWER]);
		UpdateCtrlMap();
		return;
	}
	if(strcmp(pText, "ABC") == 0) {
		m_Mode = EG_KEYBOARD_MODE_TEXT_UPPER;
		SetMap(kb_map[EG_KEYBOARD_MODE_TEXT_UPPER]);
		UpdateCtrlMap();
		return;
	}
	if(strcmp(pText, "1#") == 0) {
		m_Mode = EG_KEYBOARD_MODE_SPECIAL;
		SetMap(kb_map[EG_KEYBOARD_MODE_SPECIAL]);
		UpdateCtrlMap();
		return;
	}
	if(strcmp(pText, EG_SYMBOL_CLOSE) == 0 || strcmp(pText, EG_SYMBOL_KEYBOARD) == 0) {
		EG_Result_t res = EGEvent::EventSend(this, EG_EVENT_CANCEL, NULL);
		if(res != EG_RES_OK) return;
		if(m_pEditCtrl) {
			res = EGEvent::EventSend(m_pEditCtrl, EG_EVENT_CANCEL, NULL);
			if(res != EG_RES_OK) return;
		}
		return;
	}
	if(strcmp(pText, EG_SYMBOL_OK) == 0) {
		EG_Result_t res = EGEvent::EventSend(this, EG_EVENT_READY, NULL);
		if(res != EG_RES_OK) return;
		if(m_pEditCtrl) {
			res = EGEvent::EventSend(m_pEditCtrl, EG_EVENT_READY, NULL);
			if(res != EG_RES_OK) return;
		}
		return;
	}
	// Add the characters to the text area if set
	if(m_pEditCtrl == NULL) return;
	if((strcmp(pText, "Enter") == 0) || (strcmp(pText, EG_SYMBOL_NEW_LINE) == 0)) {
		m_pEditCtrl->AddChar('\n');
		if(m_pEditCtrl->GetOneLineMode()) {
			EG_Result_t res = EGEvent::EventSend(m_pEditCtrl, EG_EVENT_READY, NULL);
			if(res != EG_RES_OK) return;
		}
	}
	else if(strcmp(pText, EG_SYMBOL_LEFT) == 0) {
		m_pEditCtrl->CursorLeft();
	}
	else if(strcmp(pText, EG_SYMBOL_RIGHT) == 0) {
		m_pEditCtrl->CursorRight();
	}
	else if(strcmp(pText, EG_SYMBOL_BACKSPACE) == 0) {
		m_pEditCtrl->DeleteChar();
	}
	else if(strcmp(pText, "+/-") == 0) {
		uint16_t cur = m_pEditCtrl->GetCursorPosition();
		const char *ta_pText = m_pEditCtrl->GetText();
		if(ta_pText[0] == '-') {
			m_pEditCtrl->SetCursorPosition(1);
			m_pEditCtrl->DeleteChar();
			m_pEditCtrl->AddChar('+');
			m_pEditCtrl->SetCursorPosition(cur);
		}
		else if(ta_pText[0] == '+') {
			m_pEditCtrl->SetCursorPosition(1);
			m_pEditCtrl->DeleteChar();
			m_pEditCtrl->AddChar('-');
			m_pEditCtrl->SetCursorPosition(cur);
		}
		else {
			m_pEditCtrl->SetCursorPosition(0);
			m_pEditCtrl->AddChar('-');
			m_pEditCtrl->SetCursorPosition(cur + 1);
		}
	}
	else m_pEditCtrl->AddText(pText);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::UpdateMap(void)
{
	SetMap(kb_map[m_Mode]);
	UpdateCtrlMap();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGKeyboard::UpdateCtrlMap(void)
{
	if(m_Popovers) {// Apply the current control map (already includes EG_BTNMATRIX_CTRL_POPOVER flags)
		SetControlMap(kb_ctrl[m_Mode]);
	}
	else {		                              // Make a copy of the current control map
		EG_ButMatrixCtrl_t *pCtrlMap = (EG_ButMatrixCtrl_t*)EG_AllocMem(m_ButtonCount * sizeof(EG_ButMatrixCtrl_t));
		EG_CopyMem(pCtrlMap, kb_ctrl[m_Mode], sizeof(EG_ButMatrixCtrl_t) * m_ButtonCount);
		for(uint16_t i = 0; i < m_ButtonCount; i++) {		// Remove all EG_BTNMATRIX_CTRL_POPOVER flags
			pCtrlMap[i] &= (~EG_BTNMATRIX_CTRL_POPOVER);
		}
		SetControlMap(pCtrlMap);		// Apply new control map and clean up
		EG_FreeMem(pCtrlMap);
	}
}

#endif
