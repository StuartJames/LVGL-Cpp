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

#if EG_USE_LABEL != 0

#include <stdarg.h>
#include "core/EG_Object.h"
#include "font/EG_Font.h"
#include "font/EG_SymbolDef.h"
#include "misc/EG_Text.h"
#include "draw/EG_DrawContext.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_LABEL_WAIT_CHAR_COUNT        3
#define EG_LABEL_DOT_NUM 3
#define EG_LABEL_POS_LAST 0xFFFF
#define EG_LABEL_TEXT_SELECTION_OFF EG_DRAW_LABEL_NO_TXT_SEL

EG_EXPORT_CONST_INT(EG_LABEL_DOT_NUM);
EG_EXPORT_CONST_INT(EG_LABEL_POS_LAST);
EG_EXPORT_CONST_INT(EG_LABEL_TEXT_SELECTION_OFF);

///////////////////////////////////////////////////////////////////////////////////////

// Long mode behaviors. Used in 'lv_label_ext_t'
enum {
    EG_LABEL_LONG_WRAP,             //  Keep the object width, wrap the too long lines and expand the object height
    EG_LABEL_LONG_DOT,              //  Keep the size and write dots at the end if the text is too long
    EG_LABEL_LONG_SCROLL,           //  Keep the size and roll the text back and forth
    EG_LABEL_LONG_SCROLL_CIRCULAR,  //  Keep the size and roll the text circularly
    EG_LABEL_LONG_CLIP,             //  Keep the size and clip the text out of it
};

typedef uint8_t EG_LabelLongMode_e;

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_LabelClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGLabel : public EGObject
{
public:
                      EGLabel(void);
                      EGLabel(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_LabelClass);
                      ~EGLabel(void);
  virtual void        Configure(void);
  void                SetText(const char *pText);
  void                SetFormatText(const char *pFormat, ...) EG_FORMAT_ATTRIBUTE(2, 3);
  void                SetStaticText(const char *pText);
  void                SetLongMode(EG_LabelLongMode_e LongMode);
  void                SetRecolor(bool Enable);
  void                SetSelectionStart(uint32_t Index);
  void                SetSelectionEnd(uint32_t Index);
  char*               GetText(void);
  EG_LabelLongMode_e  GetLongMode(void);
  bool                GetRecolor(void);
  void                GetCharacterPosition(uint32_t Index, EGPoint *pPosition);
  uint32_t            GetCharacterAt(EGPoint *pPosition);
  bool                IsCharacterAt(EGPoint *pPosition);
  uint32_t            GetSelectionStart(void);
  uint32_t            GetSelectionEnd(void);
  void                InsertText(uint32_t Position, const char *pText);
  void                CutText(uint32_t Position, uint32_t Count);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void         SetAnimateOffsetX(void *pLabel, int32_t X);
  static void         SetAnimateOffsetY(void *pLabel, int32_t Y);

  char               *m_pText;
  union {
      char           *pTemp;            // Pointer to the allocated memory containing the character replaced by dots
      char            Temp[EG_LABEL_DOT_NUM + 1]; // Directly store the characters if <=4 characters
  } m_Dot;
  uint32_t            m_DotEnd;         // The real text length, used in dot mode

#if EG_LABEL_LONG_TXT_HINT
  EG_DrawLabelHint_t m_Hint;
#endif
#if EG_LABEL_TEXT_SELECTION
  uint32_t            m_SelectionStart;
  uint32_t            m_SelectionEnd;
#endif
  EGPoint             m_Offset;           // Text draw position offset*/
  EG_LabelLongMode_e  m_LongMode : 3;     // Determine what to do with the long texts*/
  uint8_t             m_StaticText : 1;   // Flag to indicate the text is static*/
  uint8_t             m_Recolor : 1;      // Enable in-line letter re-coloring*/
  uint8_t             m_Expand : 1;       // Ignore real width (used by the library with EG_LABEL_LONG_SCROLL)*/
  uint8_t             m_DotAllocated : 1; // 1: dot is allocated, 0: dot directly holds up to 4 chars*/

private:
  void                Event(EGEvent *pEvent);
  void                DrawMain(EGEvent *pEvent);
  void                RefreshText(void);
  void                RevertDots(void);
  bool                SetDotTemp(char *pData, uint32_t Length);
  char*               GetDotTemp(void);
  void                FreeDotTemp(void);

};

#endif