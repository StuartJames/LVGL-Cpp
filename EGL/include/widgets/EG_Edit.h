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

#if EG_USE_TEXTAREA != 0

// Testing of dependencies
#if EG_USE_LABEL == 0
#error "lv_ta: lv_label is required. Enable it in EG_Config.h (EG_USE_LABEL 1)"
#endif

#include "../core/EG_Object.h"
#include "EG_Label.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_TEXTAREA_CURSOR_LAST (0x7FFF) // Put the cursor after the last character

EG_EXPORT_CONST_INT(EG_TEXTAREA_CURSOR_LAST);

enum {
  EG_PART_TEXTAREA_PLACEHOLDER = EG_PART_CUSTOM_FIRST,
};

extern const EG_ClassType_t c_EditClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGEdit : public EGLabel
{
public:
                      EGEdit(void);
                      EGEdit(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_EditClass);
                      ~EGEdit(void);
  virtual void        Configure(void);
  void                AddChar(uint32_t Chr);
  void                AddText(const char *pText);
  void                DeleteChar(void);
  void                DeleteCharForward(void);
  void                SetEditText(const char *pText);
  void                SetPromptText(const char *pText);
  void                SetCursorPosition(int32_t Position);
  void                SetCursorClickPos(bool Enable);
  void                SetPasswordMode(bool Enable);
  void                SetPasswordBullet(const char *pBullet);
  void                SetOneLineMode(bool Enable);
  void                SetValidChars(const char *pList){ m_pValidChars = pList; };
  void                SetNumericMode(bool Enable = true);
  void                SetTextLimit(uint32_t Count){ m_TextLimit = Count; };
  void                SetInsertReplace(const char *pText){ m_pInsertReplace = pText; };
  void                SetSelectMode(bool Enable);
  void                SetPasswordShowTime(uint16_t Time){ m_PasswordShowTime = Time; };
  void                SetAlignment(EG_TextAlignment_t Alignment);
  const char*         GetEditText(void);
  const char*         GetPromptText(void);
  uint32_t            GetCursorPosition(void){ return m_Cursor.Position; };
  bool                GetCursorClickPos(void){ return m_Cursor.ClickEnable ? true : false; };
  bool                GetPasswordMode(void){ return (m_PasswordMode == 1U) ? true : false; };
  const char*         GetPasswordBullet(void);
  bool                GetOneLineMode(void){ return (m_OneLineMode == 1U) ? true : false; };
  const char*         GetValidChars(void){ return m_pValidChars; };
  uint32_t            GetTextLimit(void){ return m_TextLimit; };
  bool                IsSelected(void);
  bool                GetSelection(void);
  uint16_t            GetPasswordShowTime(void);
  void                ClearSelection(void);
  void                CursorRight(void);
  void                CursorLeft(void);
  void                CursorDown(void);
  void                CursorUp(void);
  void                Event(EGEvent *pEvent);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void         CursorBlinkAnimateCB(void *pObj, int32_t Show);
  static void         PWDCharHiderAnimate(void *pObj, int32_t X);
  static void         PWDCharHiderAnimateEnd(EGAnimate *pAnimate);

  char               *m_pPromptText;      // Place holder label. only visible if text is an empty string
  char               *m_pPasswordTemp;    // Used to store the original text in password mode
  char               *m_pPasswordBullet;  // Replacement characters displayed in password mode
  const char         *m_pValidChars;      // Only these characters will be accepted. NULL: accept all
  uint32_t            m_TextLimit;        // The max. number of characters. 0: no limit
  uint16_t            m_PasswordShowTime; // Time to show characters in password mode before change them to '*'
  EGRect              m_BlinkRect;
  struct {
      EG_Coord_t      ValidX;             // Used when stepping up/down to a shorter line. (Used by the library)
      uint32_t        Position;           // The current cursor position (0: before 1st letter; 1: before 2nd letter ...)
      EGRect          Rect;               // Cursor area relative to the Text Area
      uint32_t        Index;              // Byte index of the letter after (on) the cursor
      uint8_t         Show : 1;           // Cursor is visible now or not (Handled by the library)
      uint8_t         ClickEnable : 1;    // 1: Enable positioning the cursor by clicking the text area
  } m_Cursor;
#if EG_LABEL_TEXT_SELECTION
  uint32_t            m_SelectStart;  // Temporary values for text selection
  uint32_t            m_SelectEnd;
  uint8_t             m_SelectInProgress : 1; // User is in process of selecting
  uint8_t             m_SelectEnable : 1;      // Text can be selected on this text area
#endif
  uint8_t             m_PasswordMode : 1; // Replace characters with '*'
  uint8_t             m_OneLineMode : 1; // One line mode (ignore line breaks)

private:
  void                DrawPromp(EGEvent *pEvent);
  void                DrawCursor(EGEvent *pEvent);
  void                PasswordConcealer(void);
  bool                IsValid(uint32_t Chr);
  void                StartCursorBlink(void);
  void                RefreshCursorArea(void);
  EG_Result_t         InsertHandler(const char *pText);
  void                AutoConceal(void);
  bool                IsValidNonPrintable(const uint32_t Chr);
  void                IncrementCursorPosition(uint32_t Count);
  void                UpdateCursorPositionOnClick(EGEvent *pEvent);

  static const char  *m_pInsertReplace;

};

///////////////////////////////////////////////////////////////////////////////////////

inline void EGEdit::IncrementCursorPosition(uint32_t Count)
{
	SetCursorPosition(m_Cursor.Position + Count);	// Move the cursor after the new character
}

///////////////////////////////////////////////////////////////////////////////////////

 inline bool EGEdit::IsValidNonPrintable(const uint32_t Chr)
{
	if(Chr == '\0' || Chr == '\n' || Chr == '\r') return true;
	return false;
}

#endif
