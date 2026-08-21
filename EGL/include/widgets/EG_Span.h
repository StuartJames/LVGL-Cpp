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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include "../core/EG_Object.h"

#if EG_USE_SPAN != 0

///////////////////////////////////////////////////////////////////////////////////////

#ifndef EG_SPAN_SNIPPET_STACK_SIZE
#define EG_SPAN_SNIPPET_STACK_SIZE 64
#endif

///////////////////////////////////////////////////////////////////////////////////////

class EGSpanGroup;

enum EG_SpanOverflow_e{
    EG_SPAN_OVERFLOW_CLIP,
    EG_SPAN_OVERFLOW_ELLIPSIS,
} ;

enum EG_SpanMode_e{
    EG_SPAN_MODE_FIXED,     //  fixed the obj size
    EG_SPAN_MODE_EXPAND,    //  Expand the object size to the text size
    EG_SPAN_MODE_BREAK,     //  Keep width, break the too long lines and expand height
} ;

typedef struct {
  char        *pText;           //  a pointer to display text 
  EGSpanGroup *pSpangroup;      //  a pointer to spangroup 
  EGStyle     Style;            //  display text style 
  uint8_t     IsStaticText : 1; //  the text is static flag 
} EG_Span_t;

typedef struct {
	EG_Span_t       *pSpan;
	const char      *pText;
	const EG_Font_t *pFont;
	uint16_t        Bytes;
	int32_t      TextWidth;
	int32_t      LineHeight;
	int32_t      Kerning;
} EG_Snippet_t;

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_SpanGroupClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGSpanGroup : public EGObject
{
public:
                      EGSpanGroup(void);
                      EGSpanGroup(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_SpanGroupClass);
                      ~EGSpanGroup(void);
  virtual void        Configure(void);
  EG_Span_t*          NewSpan(void);
  void                DeleteSpan(EG_Span_t *pSpan);
  void                SetSpanText(EG_Span_t *pSpan, const char *pText);
  void                SetSpanTextStatic(EG_Span_t *pSpan, const char *pText);
  void                SetAlignment(EG_TextAlignment_t Align);
  void                SetOverflow(EG_SpanOverflow_e Overflow);
  void                SetIndent(int32_t Indent);
  void                SetMode(EG_SpanMode_e Mode);
  void                SetLines(int32_t Lines);
  EG_Span_t*          GetSpan(int32_t Index);
  uint32_t            GetSpanCount(void);
  EG_TextAlignment_t  GetAlignment(void);
  EG_SpanOverflow_e   GetOverflow(void){ return (EG_SpanOverflow_e)m_Overflow; };
  int32_t             GetIndent(void){ return m_Indent; };
  EG_SpanMode_e       GetMode(void){ return (EG_SpanMode_e)m_Mode; };
  int32_t             GetLines(void){ return m_Lines; };
  int32_t             GetMaxLineHeight(void);
  uint32_t            GetExpandWidth(uint32_t MaxWidth);
  int32_t             GetExpandHeight(int32_t Width);
  void                RefreshMode(void);
  void                Event(EGEvent *pEvent);


  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  int32_t             m_Lines;
  int32_t             m_Indent;         //  first line indent
  int32_t             m_CacheWidth;     //  the cache automatically calculates the width
  int32_t             m_CacheHeight;    
  EGList              m_SpanList;
  uint8_t             m_Mode : 2;
  uint8_t             m_Overflow : 1;
  uint8_t             m_Refresh : 1;    

private:
  void                DrawMain(EGEvent *pEvent);
  const EG_Font_t*    GetSpanStyleFont(EG_Span_t *pSpan);
  int32_t             GetSpanStyleKerning(EG_Span_t *pSpan);
  EG_Color_t          GetSpanStyleColor(EG_Span_t *pSpan);
  EG_OPA_t            GetSpanStyleOPA(EG_Span_t *pSpan);
  EG_BlendMode_e      GetSpanStyleBlendMode(EG_Span_t *pSpan);
  EG_TextDecor_e      GetSpanStyleDecoration(EG_Span_t *pSpan);
  inline void         CheckSpanText(const char **ppText);
  bool                GetSnippetInfo(const char *txt, const EG_Font_t *font, int32_t letter_space,
  												 int32_t max_width, EG_TextFlag_t flag, int32_t *use_width,
  												 uint32_t *end_ofs);
  void                ClearSnippet(void);
  uint16_t            GetSnippetCount(void);
  void                PushSnippet(EG_Snippet_t *pItem);
  EG_Snippet_t*       GetSnippet(uint16_t Index);
  int32_t             ConvertIndentPCT(int32_t Width);
  void                RefreshSpanSize(void);


};

///////////////////////////////////////////////////////////////////////////////////////

inline void EGSpanGroup::CheckSpanText(const char **ppText)
{
	if(*ppText == nullptr) {
		*ppText = "";
		EG_LOG_ERROR("error Span text == NULL");
	}
}


#endif