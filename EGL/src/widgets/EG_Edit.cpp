/* 
 *        Copyright (c) 2025-2026 HydraSystems..
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

#include "widgets/EG_Edit.h"
#if EG_USE_TEXTAREA != 0

#include <string.h>
#include "misc/EG_Assert.h"
#include "core/EG_Group.h"
#include "core/EG_Refresh.h"
#include "core/EG_InputDevice.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Animate.h"
#include "misc/EG_Text.h"
#include "misc/EG_Math.h"

///////////////////////////////////////////////////////////////////////////////////////

// Test configuration
#ifndef EG_TEXTAREA_DEF_CURSOR_BLINK_TIME
#define EG_TEXTAREA_DEF_CURSOR_BLINK_TIME 400  // ms
#endif

#ifndef EG_TEXTAREA_DEF_PWD_SHOW_TIME
#define EG_TEXTAREA_DEF_PWD_SHOW_TIME 1500  // ms
#endif

#define EG_TEXTAREA_PWD_BULLET_UNICODE 0x2022
#define IGNORE_KERNING '\0'

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_EditClass = {
  .pBaseClassType = &c_LabelClass,
	.pEventCB = EGEdit::EventCB,
	.WidthDef = EG_DPI_DEF * 2,
	.HeightDef = EG_DPI_DEF,
	.IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_EXT_DATA
	.pExtData = nullptr,
#endif
};

const char *EGEdit::m_pInsertReplace = nullptr;
const char NumericCharacters[] = "0123456789-.";

///////////////////////////////////////////////////////////////////////////////////////

EGEdit::EGEdit(void) : EGLabel(),
	m_pPromptText(nullptr),
	m_pPasswordTemp(nullptr),
	m_pPasswordBullet(nullptr),
	m_pValidChars(nullptr),
	m_TextLimit(0),
	m_PasswordShowTime(EG_TEXTAREA_DEF_PWD_SHOW_TIME),
#if EG_LABEL_TEXT_SELECTION
  m_SelectStart(EG_DRAW_LABEL_NO_TXT_SEL),
  m_SelectEnd(EG_DRAW_LABEL_NO_TXT_SEL),
  m_SelectInProgress(0), 
  m_SelectEnable(0),     
#endif
	m_PasswordMode(0),
	m_OneLineMode(0)
{
	m_Cursor.ValidX = 0;
	m_Cursor.Position = 1;
  m_Cursor.Index = 0;     
	m_Cursor.Show = 1;
	m_Cursor.ClickEnable = 1;
}

///////////////////////////////////////////////////////////////////////////////////////

EGEdit::EGEdit(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_EditClass*/) : EGLabel(),
	m_pPromptText(nullptr),
	m_pPasswordTemp(nullptr),
	m_pPasswordBullet(nullptr),
	m_pValidChars(nullptr),
	m_TextLimit(0),
	m_PasswordShowTime(EG_TEXTAREA_DEF_PWD_SHOW_TIME),
#if EG_LABEL_TEXT_SELECTION
  m_SelectStart(EG_DRAW_LABEL_NO_TXT_SEL),
  m_SelectEnd(EG_DRAW_LABEL_NO_TXT_SEL),
  m_SelectInProgress(0), 
  m_SelectEnable(0),     
#endif
	m_PasswordMode(0),
	m_OneLineMode(0)
{
	m_Cursor.ValidX = 0;
	m_Cursor.Position = 1;
  m_Cursor.Index = 0;     
	m_Cursor.Show = 1;
	m_Cursor.ClickEnable = 1;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGEdit::~EGEdit(void)
{
	if(m_pPasswordTemp != nullptr) {
		EG_FreeMem(m_pPasswordTemp);
		m_pPasswordTemp = nullptr;
	}
	if(m_pPasswordBullet != nullptr) {
		EG_FreeMem(m_pPasswordBullet);
		m_pPasswordBullet = nullptr;
	}
	if(m_pPromptText != nullptr) {
		EG_FreeMem(m_pPromptText);
		m_pPromptText = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::Configure(void)
{
  EGLabel::Configure();
	m_PasswordMode = 0;
	m_pPasswordTemp = nullptr;
	m_pPasswordBullet = nullptr;
	m_PasswordShowTime = EG_TEXTAREA_DEF_PWD_SHOW_TIME;
	m_pValidChars = nullptr;
	m_TextLimit = 0;
	m_Cursor.Show = 1;
	m_Cursor.Position = 1;
	m_Cursor.ClickEnable = 1;
	m_Cursor.ValidX = 0;
	m_OneLineMode = 0;
#if EG_LABEL_TEXT_SELECTION
	m_SelectEnable = 0;
#endif
	m_pPromptText = nullptr;
	SetWidth(EG_PCT(100));
	SetText("");
	AddFlag(EG_OBJ_FLAG_CLICKABLE);
	AddFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
	SetCursorPosition(0);
	StartCursorBlink();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::AddChar(uint32_t Char)
{
	if(m_OneLineMode && (Char == '\n' || Char == '\r')) {
		EG_LOG_INFO("Text area: line break ignored in one-line mode");
		return;
	}
	uint32_t u32Bufffer[2];
	u32Bufffer[0] = Char;
	u32Bufffer[1] = 0;
	const char *pCharBuffer = (char *)&u32Bufffer;
#if EG_BIG_ENDIAN_SYSTEM
	if(Char != 0) while(*ChrBuffer == 0) ++ChrBuffer;
#endif
	if(InsertHandler(pCharBuffer) != EG_RES_OK) return;
	uint32_t UniChr = EG_TextDecodeNext((const char *)&Char, nullptr);
	if(IsValid(UniChr) == false) {
		EG_LOG_INFO("Character is not accepted by the text area (too long text or not in the accepted list)");
		return;
	}
	if(m_PasswordMode) PasswordConcealer();   // Make sure all the current text contains only '*'
	if(m_pPromptText) {	                      // If the textarea is empty, invalidate it to hide the placeholder
		const char *pText = GetText();
		if(pText[0] == '\0') Invalidate();
	}
	InsertText(m_Cursor.Position, pCharBuffer);   // Insert the character
	ClearSelection();                             // Clear selection
	if(m_PasswordMode) {
		// +2: the new char + \0
		size_t realloc_size = strlen(m_pPasswordTemp) + strlen(pCharBuffer) + 1;
		m_pPasswordTemp = (char *)EG_ReallocMem(m_pPasswordTemp, realloc_size);
		EG_ASSERT_MALLOC(m_pPasswordTemp);
		if(m_pPasswordTemp == nullptr) return;
		EG_TextInsert(m_pPasswordTemp, m_Cursor.Position, (const char *)pCharBuffer);
		AutoConceal();		                          // Auto hide characters
	}
	IncrementCursorPosition(1);	// Move the cursor after the new character
	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::AddText(const char *pText)
{
	EG_ASSERT_NULL(pText);
	if(m_PasswordMode) PasswordConcealer();  // Make sure all the current text contains only '*'
	// Add the character one-by-one if not all characters are accepted or there is character limit.
	if(GetValidChars() || GetTextLimit()) {
		uint32_t i = 0;
		while(pText[i] != '\0') {
			uint32_t Char = EG_TextDecodeNext(pText, &i);
			AddChar(EG_TextUnicodeToEncoded(Char));
		}
		return;
	}
	EG_Result_t res = InsertHandler(pText);
	if(res != EG_RES_OK) return;
	if(m_pPromptText) {	// If the textarea is empty, invalidate it to hide the placeholder
		const char *txt_act = GetText();
		if(txt_act[0] == '\0') Invalidate();
	}
	InsertText(m_Cursor.Position, pText);	// Insert the text
	ClearSelection();
	if(m_PasswordMode) {
		size_t realloc_size = strlen(m_pPasswordTemp) + strlen(pText) + 1;
		m_pPasswordTemp = (char *)EG_ReallocMem(m_pPasswordTemp, realloc_size);
		EG_ASSERT_MALLOC(m_pPasswordTemp);
		if(m_pPasswordTemp == nullptr) return;
		EG_TextInsert(m_pPasswordTemp, m_Cursor.Position, pText);
		AutoConceal();		// Auto hide characters
	}
	IncrementCursorPosition(EG_TextEncodedGetLength(pText));	// Move the cursor after the new text

	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::DeleteChar(void)
{
	if(m_Cursor.Position == 0) return;
	char DeletBuffer[2] = {EG_KEY_DEL, '\0'};
	if(InsertHandler(DeletBuffer) != EG_RES_OK) return;
	char *label_txt = GetText();
	EG_TextCut(label_txt, m_Cursor.Position - 1, 1);	// Delete a character
	SetText(label_txt);	// Refresh the label
	ClearSelection();
	if(m_pPromptText) {	// If the textarea became empty, invalidate it to hide the placeholder
		const char *pText = GetText();
		if(pText[0] == '\0') Invalidate();
	}
	if(m_PasswordMode) {
		EG_TextCut(m_pPasswordTemp, m_Cursor.Position - 1, 1);
		m_pPasswordTemp = (char *)EG_ReallocMem(m_pPasswordTemp, strlen(m_pPasswordTemp) + 1);
		EG_ASSERT_MALLOC(m_pPasswordTemp);
		if(m_pPasswordTemp == nullptr) return;
	}
	IncrementCursorPosition(-1);	// Move the cursor to the place of the deleted character
	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::DeleteCharForward(void)
{
	uint32_t CurentPos = m_Cursor.Position;
	IncrementCursorPosition(1);
	if(CurentPos != m_Cursor.Position) DeleteChar();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetEditText(const char *pText)
{
	EG_ASSERT_NULL(pText);
	ClearSelection();	// Clear the existing selection
	// Add the character one-by-one if not all characters are accepted or there is character limit.
	if(GetValidChars() || GetTextLimit()) {
		SetText("");
		SetCursorPosition(EG_TEXTAREA_CURSOR_LAST);
		if(m_PasswordMode) {
			m_pPasswordTemp[0] = '\0';  // Clear the password too
		}
		uint32_t i = 0;
		while(pText[i] != '\0') {
			uint32_t Char = EG_TextDecodeNext(pText, &i);
			AddChar(EG_TextUnicodeToEncoded(Char));
		}
	}
	else {
		SetText(pText);
		SetCursorPosition(EG_TEXTAREA_CURSOR_LAST);
	}
	if(m_pPromptText) {	// If the textarea is empty, invalidate it to hide the placeholder
		const char *txt_act = GetText();
		if(txt_act[0] == '\0') Invalidate();
	}
	if(m_PasswordMode) {
		m_pPasswordTemp = (char *)EG_ReallocMem(m_pPasswordTemp, strlen(pText) + 1);
		EG_ASSERT_MALLOC(m_pPasswordTemp);
		if(m_pPasswordTemp == nullptr) return;
		strcpy(m_pPasswordTemp, pText);
		AutoConceal();		// Auto hide characters
	}
	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetPromptText(const char *pText)
{
	EG_ASSERT_NULL(pText);
	size_t txt_len = strlen(pText);
	if((txt_len == 0) && (m_pPromptText)) {
		EG_FreeMem(m_pPromptText);
		m_pPromptText = nullptr;
	}
	else {
		// Allocate memory for the placeholder_txt text
		// NOTE: Using special realloc behavior, malloc-like when data_p is NULL
		m_pPromptText = (char *)EG_ReallocMem(m_pPromptText, txt_len + 1);
		EG_ASSERT_MALLOC(m_pPromptText);
		if(m_pPromptText == nullptr) {
			EG_LOG_ERROR("couldn't allocate memory for prompt");
			return;
		}
		strcpy(m_pPromptText, pText);
		m_pPromptText[txt_len] = '\0';
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetCursorPosition(int32_t Position)
{
	if((uint32_t)m_Cursor.Position == (uint32_t)Position) return;
	uint32_t Length = EG_TextEncodedGetLength(GetText());
	if(Position < 0) Position = Length + Position;
	if(Position > (int32_t)Length || Position == EG_TEXTAREA_CURSOR_LAST) Position = Length;
	m_Cursor.Position = Position;
  UpdateLayout();	// Position the label to make the cursor visible
	EGPoint CursorPoint;
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	GetCharacterPosition(Position, &CursorPoint);
	// The text area needs to have it's final size to see if the cursor is out of the area or not
	EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);	// Check the PadTop
	if(CursorPoint.m_Y < GetScrollTop()) {
		ScrollToY(CursorPoint.m_Y, EG_ANIM_ON);
	}
	EG_Coord_t Height = GetContentHeight();	// Check the PadBottom
	if(CursorPoint.m_Y + FontHeight - GetScrollTop() > Height) {
		ScrollToY(CursorPoint.m_Y - Height + FontHeight, EG_ANIM_ON);
	}
	if(CursorPoint.m_X < GetScrollLeft()) {	// Check the PadLeft
		ScrollToX(CursorPoint.m_X, EG_ANIM_ON);
	}
	// Check the PadRight
	EG_Coord_t w = GetContentWidth();
	if(CursorPoint.m_X + FontHeight - GetScrollLeft() > w) {
		ScrollToX(CursorPoint.m_X - w + FontHeight, EG_ANIM_ON);
	}
	m_Cursor.ValidX = CursorPoint.m_X;
	StartCursorBlink();
	RefreshCursorArea();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetCursorClickPos(bool Enable)
{
	m_Cursor.ClickEnable = Enable ? 1U : 0U;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetNumericMode(bool Enable /*= true*/)
{
  if(Enable) m_pValidChars = NumericCharacters;
  else m_pValidChars = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetPasswordMode(bool Enable)
{
	if(m_PasswordMode == Enable) return;
	m_PasswordMode = Enable ? 1U : 0U;
	// Pwd mode is now enabled
	if(Enable) {
		char *pText = GetText();
		size_t len = strlen(pText);
		m_pPasswordTemp = (char *)EG_AllocMem(len + 1);
		EG_ASSERT_MALLOC(m_pPasswordTemp);
		if(m_pPasswordTemp == nullptr) return;
		strcpy(m_pPasswordTemp, pText);
		PasswordConcealer();
		ClearSelection();
	}
	else {	// Pwd mode is now disabled
		ClearSelection();
		SetText(m_pPasswordTemp);
		EG_FreeMem(m_pPasswordTemp);
		m_pPasswordTemp = nullptr;
	}
	RefreshCursorArea();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetPasswordBullet(const char *pBullet)
{
	EG_ASSERT_NULL(pBullet);
	if(!pBullet && (m_pPasswordBullet)) {
		EG_FreeMem(m_pPasswordBullet);
		m_pPasswordBullet = nullptr;
	}
	else {
		size_t txt_len = strlen(pBullet);
		// Allocate memory for the pwd_bullet text
		// NOTE: Using special realloc behavior, malloc-like when data_p is NULL
		m_pPasswordBullet = (char *)EG_ReallocMem(m_pPasswordBullet, txt_len + 1);
		EG_ASSERT_MALLOC(m_pPasswordBullet);
		if(m_pPasswordBullet == nullptr) {
			EG_LOG_ERROR("lv_textarea_set_password_bullet: couldn't allocate memory for bullet");
			return;
		}
		strcpy(m_pPasswordBullet, pBullet);
		m_pPasswordBullet[txt_len] = '\0';
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetOneLineMode(bool Enable)
{
	if(m_OneLineMode == Enable) return;
	m_OneLineMode = Enable ? 1U : 0U;
	EG_Coord_t Width = Enable ? EG_SIZE_CONTENT : EG_PCT(100);
	EG_Coord_t MinimumWidth = Enable ? EG_PCT(100) : 0;
	SetWidth(Width);
	SetStyleMinWidth(MinimumWidth, 0);
	if(Enable) SetHeight(EG_SIZE_CONTENT);
	else RemoveStyleProperty(EG_STYLE_HEIGHT, EG_PART_MAIN);
	ScrollTo(0, 0, EG_ANIM_OFF);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetSelectMode(bool Enable)
{
#if EG_LABEL_TEXT_SELECTION
	m_SelectEnable = Enable;
	if(!Enable) ClearSelection();
#else
	EG_UNUSED(Enable);   // Unused
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::SetAlignment(EG_TextAlignment_t Alignment)
{
	EG_LOG_WARN("Deprecated: use the normal text_align style property instead");
	SetStyleTextAlign(Alignment, 0);
	switch(Alignment) {
		default:
		case EG_TEXT_ALIGN_LEFT:
			Align(EG_ALIGN_TOP_LEFT, 0, 0);
			break;
		case EG_TEXT_ALIGN_RIGHT:
			Align(EG_ALIGN_TOP_RIGHT, 0, 0);
			break;
		case EG_TEXT_ALIGN_CENTER:
			Align(EG_ALIGN_TOP_MID, 0, 0);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGEdit::GetEditText(void)
{
	const char *pText;
	if(m_PasswordMode == 0) pText = GetText();
	else pText = m_pPasswordTemp;
	return pText;
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGEdit::GetPromptText(void)
{
	if(m_pPromptText) return m_pPromptText;
	return "";
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGEdit::GetPasswordBullet(void)
{
	if(m_pPasswordBullet) return m_pPasswordBullet;
	EG_FontGlyphProps_t Glyph;
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	// If the textarea's font has the bullet character use it else pFallback to "*"
	if(EG_FontGetGlyphProps(pFont, &Glyph, EG_TEXTAREA_PWD_BULLET_UNICODE, 0)) return EG_SYMBOL_BULLET;
	return "*";
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEdit::IsSelected(void)
{
#if EG_LABEL_TEXT_SELECTION
	if((GetSelectionStart() != EG_DRAW_LABEL_NO_TXT_SEL || GetSelectionEnd() != EG_DRAW_LABEL_NO_TXT_SEL)) {
		return true;
	}
	else return false;
#else
	return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEdit::GetSelection(void)
{
#if EG_LABEL_TEXT_SELECTION
	return m_SelectEnable;
#else
	return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGEdit::GetPasswordShowTime(void)
{
	return m_PasswordShowTime;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::ClearSelection(void)
{
#if EG_LABEL_TEXT_SELECTION
	if(GetSelectionStart() != EG_DRAW_LABEL_NO_TXT_SEL || GetSelectionEnd() != EG_DRAW_LABEL_NO_TXT_SEL) {
		SetSelectionStart(EG_DRAW_LABEL_NO_TXT_SEL);
		SetSelectionEnd(EG_DRAW_LABEL_NO_TXT_SEL);
	}
#else
	EG_UNUSED(obj);  // Unused
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::CursorRight(void)
{
	SetCursorPosition(m_Cursor.Position + 1);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::CursorLeft(void)
{
	if(m_Cursor.Position > 0) SetCursorPosition(m_Cursor.Position - 1);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::CursorDown(void)
{
	EGPoint Point;
  GetCharacterPosition(m_Cursor.Position, &Point);	// Get the position of the current character
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);	// Increment the y with one line and keep the valid x
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);
	Point.m_Y += FontHeight + LineSpace + 1;
	Point.m_X = m_Cursor.ValidX;
	if(Point.m_Y < GetHeight()) {	// Do not go below the last line
		uint32_t NewPosition = IsCharacterAt(&Point);		// Get the character index on the new cursor position and set it
		EG_Coord_t cur_valid_x_tmp = m_Cursor.ValidX;  // Cursor position set overwrites the valid position
		SetCursorPosition(NewPosition);
		m_Cursor.ValidX = cur_valid_x_tmp;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::CursorUp(void)
{
	EGPoint Point;
	GetCharacterPosition(m_Cursor.Position, &Point);	// Get the position of the current character
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);	// Decrement the y with one line and keep the valid x
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);
	Point.m_Y -= FontHeight + LineSpace - 1;
	Point.m_X = m_Cursor.ValidX;
	uint32_t NewPosition = GetCharacterAt(&Point);	// Get the character index on the new cursor position and set it
	EG_Coord_t cur_valid_x_tmp = m_Cursor.ValidX;  // Cursor position set overwrites the valid position
	SetCursorPosition(NewPosition);
	m_Cursor.ValidX = cur_valid_x_tmp;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(&c_EditClass) != EG_RES_OK) return;  // Call the ancestor's event handler
	((EGEdit*)pEvent->GetTarget())->Event(pEvent);  // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_FOCUSED: {
      StartCursorBlink();
      break;
    }
    case EG_EVENT_KEY: {
      uint32_t Char = *((uint32_t*)pEvent->GetParam());  // uint32_t because can be UTF-8
      switch(Char){
        case EG_KEY_RIGHT:
          CursorRight();
          break;
        case EG_KEY_LEFT:
          CursorLeft();
          break;
        case EG_KEY_UP:
          CursorUp();
          break;
        case EG_KEY_DOWN:
          CursorDown();
          break;
        case EG_KEY_BACKSPACE:
          DeleteChar();
          break;
        case EG_KEY_DEL:
          DeleteCharForward();
          break;
        case EG_KEY_HOME:
          SetCursorPosition(0);
          break;
        case EG_KEY_END:
          SetCursorPosition(EG_TEXTAREA_CURSOR_LAST);
          break;
        case EG_KEY_ENTER:
          if(GetOneLineMode()) EGEvent::EventSend(this, EG_EVENT_READY, nullptr);
          break;
        default:
          AddChar(Char);
          break;
      }
      break;
    }
    case EG_EVENT_PRESSED:
    case EG_EVENT_PRESSING:
    case EG_EVENT_PRESS_LOST:
    case EG_EVENT_RELEASED: {
      UpdateCursorPositionOnClick(pEvent);
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      DrawPromp(pEvent);
      break;
    }
    case EG_EVENT_DRAW_POST: {
      DrawCursor(pEvent);
      break;
    }
	  case EG_EVENT_STYLE_CHANGED:
    case EG_EVENT_SIZE_CHANGED: {
		  SetText(nullptr);
		  RefreshCursorArea();
		  StartCursorBlink();
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::UpdateCursorPositionOnClick(EGEvent *pEvent)
{
	EGInputDevice *pInput = EGInputDevice::GetActive();
	if((pInput == nullptr) || (m_Cursor.ClickEnable == 0)) return;
  EG_InDeviceType_e InputType = pInput->GetType();
	if(InputType == EG_INDEV_TYPE_KEYPAD || InputType == EG_INDEV_TYPE_ENCODER) return;
	EGRect LabelRect;
	m_Rect.Copy(&LabelRect);	
	EGPoint ActivePoint, ActiveVector;
	pInput->GetPoint(&ActivePoint);
	pInput->GetVector(&ActiveVector);
	if(ActivePoint.m_X < 0 || ActivePoint.m_Y < 0) return;  // Ignore event from keypad
	EGPoint rel_pos;
	rel_pos.m_X = ActivePoint.m_X - LabelRect.GetX1();
	rel_pos.m_Y = ActivePoint.m_Y - LabelRect.GetY1();
  EG_EventCode_e Code = pEvent->GetCode();
	EG_Coord_t LabelWidth = GetWidth();
	uint16_t char_id_at_click = 0;
#if EG_LABEL_TEXT_SELECTION
	bool click_outside_label = false;
	// Check if the click happened on the PadLeft side of the area outside the label
	if(rel_pos.m_X < 0) {
		char_id_at_click = 0;
		click_outside_label = true;
	}
	// Check if the click happened on the PadRight side of the area outside the label
	else if(rel_pos.m_X >= LabelWidth) {
		char_id_at_click = EG_TEXTAREA_CURSOR_LAST;
		click_outside_label = true;
	}
	else {
		char_id_at_click = GetCharacterAt(&rel_pos);
		click_outside_label = !IsCharacterAt(&rel_pos);
	}
	if(m_SelectEnable) {
		if(!m_SelectInProgress && !click_outside_label && Code == EG_EVENT_PRESSED) {
			// Input device just went down. Store the selection start position
			m_SelectStart = char_id_at_click;
			m_SelectEnd = EG_LABEL_TEXT_SELECTION_OFF;
			m_SelectInProgress = 1;
			ClearFlag(EG_OBJ_FLAG_SCROLL_CHAIN);
		}
		else if(m_SelectInProgress && Code == EG_EVENT_PRESSING) {
			// Input device may be moving. Store the end position
			m_SelectEnd = char_id_at_click;
		}
		else if(m_SelectInProgress && (Code == EG_EVENT_PRESS_LOST || Code == EG_EVENT_RELEASED)) {
			// Input device is released. Check if anything was selected.
			AddFlag(EG_OBJ_FLAG_SCROLL_CHAIN);
		}
	}
	if(m_SelectInProgress || Code == EG_EVENT_PRESSED) SetCursorPosition(char_id_at_click);
	if(m_SelectInProgress) {
		// If the selected area has changed then update the real values and
		if(m_SelectStart > m_SelectEnd) {		// Invalidate the text area.
			if(m_SelectionStart != m_SelectEnd || m_SelectionEnd != m_SelectStart) {
				m_SelectionStart = m_SelectEnd;
				m_SelectionEnd = m_SelectStart;
				Invalidate();
			}
		}
		else if(m_SelectStart < m_SelectEnd) {
			if(m_SelectionStart != m_SelectStart || m_SelectionEnd != m_SelectEnd) {
				m_SelectionStart = m_SelectStart;
				m_SelectionEnd = m_SelectEnd;
				Invalidate();
			}
		}
		else {
			if(m_SelectionStart != EG_DRAW_LABEL_NO_TXT_SEL || m_SelectionEnd != EG_DRAW_LABEL_NO_TXT_SEL) {
				m_SelectionStart = EG_DRAW_LABEL_NO_TXT_SEL;
				m_SelectionEnd = EG_DRAW_LABEL_NO_TXT_SEL;
				Invalidate();
			}
		}
		// Finish selection if necessary
		if(Code == EG_EVENT_PRESS_LOST || Code == EG_EVENT_RELEASED) {
			m_SelectInProgress = 0;
		}
	}
#else
	// Check if the click happened on the PadLeft side of the area outside the label
	if(rel_pos.m_X < 0) {
		char_id_at_click = 0;
	}
	// Check if the click happened on the PadRight side of the area outside the label
	else if(rel_pos.m_X >= LabelWidth) {
		char_id_at_click = EG_TEXTAREA_CURSOR_LAST;
	}
	else {
		char_id_at_click = lv_label_get_letter_on(m_pLabel, &rel_pos);
	}

	if(code == EG_EVENT_PRESSED) lv_textarea_set_cursor_pos(obj, char_id_at_click);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::CursorBlinkAnimateCB(EGAnimate *pAnimate, int32_t Show)
{
	EGEdit *pEdit = (EGEdit *)pAnimate->m_pItem;
	if(Show != pEdit->m_Cursor.Show) {
		pEdit->m_Cursor.Show = Show ? 1U : 0U;
		pEdit->InvalidateArea(&pEdit->m_BlinkRect);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::PWDCharHiderAnimate(EGAnimate *pAnimate, int32_t X)
{
	EG_UNUSED(pAnimate);
	EG_UNUSED(X);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::PWDCharHiderAnimateEnd(EGAnimate *pAnimate)
{
	EGEdit *pTextBox = (EGEdit*)pAnimate->m_pItem;
	pTextBox->PasswordConcealer();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::PasswordConcealer(void)
{
uint32_t i;

	if(m_PasswordMode == 0) return;
	char *pText = GetText();	// When m_pLabel is empty we get 0 back
	uint32_t enc_len = EG_TextEncodedGetLength(pText);
	if(enc_len == 0) return;
	const char *bullet = GetPasswordBullet();
	const size_t bullet_len = strlen(bullet);
	char *txt_tmp = (char *)EG_GetBufferMem(enc_len * bullet_len + 1);
	for(i = 0; i < enc_len; i++) {
		EG_CopyMem(&txt_tmp[i * bullet_len], bullet, bullet_len);
	}
	txt_tmp[i * bullet_len] = '\0';
	SetText(txt_tmp);
	EG_ReleaseBufferMem(txt_tmp);
	RefreshCursorArea();
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGEdit::IsValid(uint32_t Char)
{
uint32_t i = 0;

	if((m_TextLimit > 0) && (EG_TextEncodedGetLength(GetText()) >= m_TextLimit)) {	// Too many characters?
		return false;
	}
	if(m_pValidChars == nullptr || m_pValidChars[0] == '\0') return true;
	while(m_pValidChars[i] != '\0') {	// Accepted character?
		uint32_t a = EG_TextDecodeNext(m_pValidChars, &i);
		if(a == Char) return true;  // Accepted
	}
	return false;  // The character wasn't in the list
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::StartCursorBlink(void)
{
	uint32_t BlinkPeriod = GetStyleAnimationTime(EG_PART_CURSOR);
	if(BlinkPeriod == 0) {
		EGAnimate::Delete(this, CursorBlinkAnimateCB);
		m_Cursor.Show = 1;
	}
	else {
		EGAnimate Animate;
		Animate.SetItem(this);
		Animate.SetExcCB(CursorBlinkAnimateCB);
		Animate.SetTime(BlinkPeriod);
		Animate.SetPlaybackTime(BlinkPeriod);
		Animate.SetValues(1, 0);
		Animate.SetPathCB(EGAnimate::PathStep);
		Animate.SetRepeatCount(EG_ANIM_REPEAT_INFINITE);
		EGAnimate::Create(&Animate);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::RefreshCursorArea(void)
{
EGPoint CharPos;

	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	uint32_t BytePosition = EG_TextEncodedGetIndex(m_pText, m_Cursor.Position);
	uint32_t Char = EG_TextDecodeNext(&m_pText[BytePosition], nullptr);
	//  Character height and width
	const EG_Coord_t CharHeight = EG_FontGetLineHeight(pFont);
	// Set CharWidth (set not 0 on non printable but valid chars)
	uint32_t ValidChar = Char;
	if(IsValidNonPrintable(Char)) ValidChar = ' ';
	EG_Coord_t CharWidth = EG_FontGetGlyphWidth(pFont, ValidChar, IGNORE_KERNING);
	GetCharacterPosition(m_Cursor.Position, &CharPos);
	EG_TextAlignment_t Alignment = CalculateTextAlignment(EG_PART_MAIN, m_pText);
	// If the cursor is out of the text (far right) draw it on the next line
	if(((CharPos.m_X + m_Rect.GetX1()) + CharWidth > m_Rect.GetX2()) &&
   (m_OneLineMode == 0) && (Alignment != EG_TEXT_ALIGN_RIGHT)) {
	  EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
		CharPos.m_X = 0;
		CharPos.m_Y += CharHeight + LineSpace;
		if(Char != '\0') {
			BytePosition += EG_TextEncodedSize(&m_pText[BytePosition]);
			Char = EG_TextDecodeNext(&m_pText[BytePosition], nullptr);
		}
		ValidChar = Char;
		if(IsValidNonPrintable(Char)) ValidChar = ' ';
		CharWidth = EG_FontGetGlyphWidth(pFont, ValidChar, IGNORE_KERNING);
	}
	m_Cursor.Index = BytePosition;	// Save the byte position. It is required to draw `EG_CURSOR_BLOCK`
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_CURSOR);	// Calculate the cursor according to its type
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_CURSOR) + BorderWidth;
	EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_CURSOR) + BorderWidth;
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_CURSOR) + BorderWidth;
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_CURSOR) + BorderWidth;
	EGRect CursorRect(CharPos.m_X - PadLeft, CharPos.m_Y - PadTop, CharPos.m_X + PadRight + CharWidth - 1, CharPos.m_Y + PadBottom + CharHeight - 1);
	EGRect Rect(m_Cursor.Rect);	// Get the present poition
	Rect.Move(m_Rect.GetX1(), m_Rect.GetY1());
	InvalidateArea(&Rect);      // The old position will be redrawn
	m_Cursor.Rect = CursorRect; // Update the new position
	Rect = m_Cursor.Rect;
	Rect.Move(m_Rect.GetX1(), m_Rect.GetY1());
	InvalidateArea(&Rect);      // Redraw the new position
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGEdit::InsertHandler(const char *pText)
{
	m_pInsertReplace = nullptr;
	EGEvent::EventSend(this, EG_EVENT_INSERT, (char *)pText);

	//  Drop pText if insert replace is set to '\0'
	if(m_pInsertReplace && m_pInsertReplace[0] == '\0')
		return EG_RES_INVALID;

	if(m_pInsertReplace) {
		// Add the replaced text directly it's different from the original
		if(strcmp(m_pInsertReplace, pText)) {
			AddText(m_pInsertReplace);
			return EG_RES_INVALID;
		}
	}

	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::DrawPromp(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	const char *pText = GetText();

	// Draw the place holder
	if(pText[0] == '\0' && m_pPromptText && m_pPromptText[0] != 0) {
		EGDrawLabel DrawLabel;
		InititialseDrawLabel(EG_PART_TEXTAREA_PLACEHOLDER, &DrawLabel);
		if(m_OneLineMode) DrawLabel.m_Flag |= EG_TEXT_FLAG_EXPAND;
		EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
		EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
		EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
		EGRect LabelRect(m_Rect);
		LabelRect.Move(PadLeft + BorderWidth, PadTop + BorderWidth);
		DrawLabel.Draw(pContext, &LabelRect, m_pPromptText, nullptr);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::DrawCursor(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	const char *pText = GetText();
	if(m_Cursor.Show == 0) return;
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
  EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN) + BorderWidth;
  m_BlinkRect = m_Cursor.Rect;    // Blink rect is used in the cursor blink animation
	m_BlinkRect.Move(m_Rect.GetX1() + PadLeft, m_Rect.GetY1() + PadTop);
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_CURSOR, &DrawRect);
	DrawRect.Draw(pContext, &m_BlinkRect);
	BorderWidth = GetStyleBorderWidth(EG_PART_CURSOR);
	PadLeft = GetStylePadLeft(EG_PART_CURSOR) + BorderWidth;
	PadTop = GetStylePadTop(EG_PART_CURSOR) + BorderWidth;
	char Buffer[8] = {0};
	EG_CopyMem(Buffer, &pText[m_Cursor.Index], EG_TextEncodedSize(&pText[m_Cursor.Index]));
	EGRect CharRect(m_Cursor.Rect);	
	CharRect.IncX1(PadLeft);
	CharRect.IncY1(PadTop);
	/*Draw over the cursor only if the cursor has background or the character has different color than the original.
    Else the original character is drawn twice which makes it look bolder*/
	EG_Color_t TextColor = GetStyleTextColor(0);
	EGDrawLabel DrawLabel;
	InititialseDrawLabel(EG_PART_CURSOR, &DrawLabel);
	if(DrawRect.m_BackgroundOPA > EG_OPA_MIN || DrawLabel.m_Color.full != TextColor.full) {
		DrawLabel.Draw(pContext, &CharRect, Buffer, nullptr);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGEdit::AutoConceal(void)
{
	if(m_PasswordShowTime == 0) PasswordConcealer();
	else {
		EGAnimate Animate;
		Animate.SetItem(this);
		Animate.SetExcCB(PWDCharHiderAnimate);
		Animate.SetTime(m_PasswordShowTime);
		Animate.SetValues(0, 1);
		Animate.SetPathCB(EGAnimate::PathStep);
		Animate.SetEndCB(PWDCharHiderAnimateEnd);
		EGAnimate::Create(&Animate);
	}
}


#endif
