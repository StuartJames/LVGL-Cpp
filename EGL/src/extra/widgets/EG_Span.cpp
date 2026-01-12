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

#include "extra/widgets/EG_Span.h"

#if EG_USE_SPAN != 0

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

#define SPAN_CLASS &lv_spangroup_class

///////////////////////////////////////////////////////////////////////////////////////

struct _snippet_stack {
	EG_Snippet_t stack[EG_SPAN_SNIPPET_STACK_SIZE];
	uint16_t index;
};

///////////////////////////////////////////////////////////////////////////////////////

static struct _snippet_stack snippet_stack;

const EG_ClassType_t lv_spangroup_class = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGSpanGroup::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,   
  .GroupDef = 0,  
#if EG_USE_USER_DATA
  .pExtData = NULL,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGSpanGroup::EGSpanGroup(void) : EGObject(),
  m_Lines(0),
  m_Indent(0),
  m_CacheWidth(0),
  m_CacheHeight(0),
  m_Mode(0),
  m_Overflow(0),
  m_Refresh(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGSpanGroup::EGSpanGroup(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_SpangroupClass*/) : EGObject(),
  m_Lines(0),
  m_Indent(0),
  m_CacheWidth(0),
  m_CacheHeight(0),
  m_Mode(0),
  m_Overflow(0),
  m_Refresh(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGSpanGroup::~EGSpanGroup(void)
{
	EG_Span_t *pSpan = (EG_Span_t*)m_SpanList.GetHead();
	while(pSpan) {
		m_SpanList.RemoveHead();
		if(pSpan->pText && pSpan->IsStaticText == 0) {
			EG_FreeMem(pSpan->pText);
		}
		pSpan->Style.Reset();
		EG_FreeMem(pSpan);
		pSpan = (EG_Span_t*)m_SpanList.GetHead();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::Configure(void)
{
	EGObject::Configure();
  m_Indent = 0;
	m_Lines = -1;
	m_Mode = EG_SPAN_MODE_EXPAND;
	m_Overflow = EG_SPAN_OVERFLOW_CLIP;
	m_CacheWidth = 0;
	m_CacheHeight = 0;
	m_Refresh = 1;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Span_t* EGSpanGroup::NewSpan(void)
{
  EG_Span_t *pSpan = (EG_Span_t*)EG_AllocMem(sizeof(EG_Span_t));
	EG_ASSERT_MALLOC(pSpan);
	m_SpanList.AddTail(pSpan);
	pSpan->Style.Initialise();
	pSpan->pText = (char *)"";
	pSpan->IsStaticText = 1;
	pSpan->pSpangroup = this;
	RefreshSpanSize();
	return pSpan;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::DeleteSpan(EG_Span_t *pDelSpan)
{
EG_Span_t *pSpan;

  POSITION Pos = m_SpanList.GetHeadPosition();
  while(Pos != nullptr){
	  pSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
		if(pSpan == pDelSpan) {
			m_SpanList.RemoveAt(Pos);
			if(pSpan->pText && pSpan->IsStaticText == 0) {
				EG_FreeMem(pSpan->pText);
			}
			pSpan->Style.Reset();
			EG_FreeMem(pSpan);
			break;
		}
	}
	RefreshSpanSize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetSpanText(EG_Span_t *pSpan, const char *pText)
{
	if(pSpan == NULL || pText == NULL) return;
	if(pSpan->pText == NULL || pSpan->IsStaticText == 1) {
		pSpan->pText = (char*)EG_AllocMem(strlen(pText) + 1);
	}
	else {
		pSpan->pText = (char*)EG_ReallocMem(pSpan->pText, strlen(pText) + 1);
	}
	pSpan->IsStaticText = 0;
	strcpy(pSpan->pText, pText);
	RefreshSpanSize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetSpanTextStatic(EG_Span_t *pSpan, const char *pText)
{
	if(pSpan == NULL || pText == NULL) return;
	if(pSpan->pText && pSpan->IsStaticText == 0) {
		EG_FreeMem(pSpan->pText);
	}
	pSpan->IsStaticText = 1;
	pSpan->pText = (char *)pText;
	RefreshSpanSize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetAlignment(EG_TextAlignment_t Align)
{
	SetStyleTextAlign(Align, EG_PART_MAIN);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetOverflow(EG_SpanOverflow_e Overflow)
{
	if(m_Overflow == Overflow) return;
	m_Overflow = Overflow;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetIndent(EG_Coord_t Indent)
{
	if(m_Indent == Indent) return;
	m_Indent = Indent;
	RefreshSpanSize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetMode(EG_SpanMode_e Mode)
{
	m_Mode = Mode;
	RefreshMode();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::SetLines(int32_t Lines)
{
	m_Lines = Lines;
	RefreshMode();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Span_t* EGSpanGroup::GetSpan(int32_t Index)
{
POSITION Pos;
int32_t CurrentIndex = 0;

	bool TraverseForwards = (Index >= 0);	// If using a negative index, start from the tail and use cur -1 to indicate the end
	EG_Span_t *pCurrentSpan = (EG_Span_t*)m_SpanList.GetHead(Pos);
	if(!TraverseForwards) {
		CurrentIndex = -1;
		pCurrentSpan = (EG_Span_t*)m_SpanList.GetTail(Pos);
	}
	while(pCurrentSpan != NULL) {
		if(CurrentIndex == Index) return pCurrentSpan;
		if(TraverseForwards) {
			pCurrentSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
			CurrentIndex++;
		}
		else {
			pCurrentSpan = (EG_Span_t*)m_SpanList.GetTail(Pos);
			CurrentIndex--;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGSpanGroup::GetSpanCount(void)
{
	return m_SpanList.GetCount();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_TextAlignment_t EGSpanGroup::GetAlignment(void)
{
	return GetStyleTextAlign(EG_PART_MAIN);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::RefreshMode(void)
{
	if(m_Mode == EG_SPAN_MODE_EXPAND) {
		SetWidth(EG_SIZE_CONTENT);
		SetHeight(EG_SIZE_CONTENT);
	}
	else if(m_Mode == EG_SPAN_MODE_BREAK) {
		if(GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT) SetWidth(100);
		SetHeight(EG_SIZE_CONTENT);
	}
	else if(m_Mode == EG_SPAN_MODE_FIXED) {
		//  use this mode, The user needs to set the size. 
		//  This is just to prevent an infinite loop. 
		if(GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT) SetWidth(100);
		if(GetStyleHeight(EG_PART_MAIN) == EG_SIZE_CONTENT) {
			EG_Coord_t Width = GetStyleWidth(EG_PART_MAIN);
			if(EG_COORD_IS_PCT(Width)) Width = 100;
			EG_Coord_t Height = GetExpandHeight(Width);
			SetContentHeight(Height);
		}
	}
	RefreshSpanSize();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGSpanGroup::GetMaxLineHeight(void)
{
EG_Coord_t MaxLineHeight = 0;
EG_Span_t *pSpan;

  POSITION Pos = m_SpanList.GetHeadPosition();
  while(Pos != nullptr){
	  pSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
		const EG_Font_t *pFont = GetSpanStyleFont(pSpan);
		EG_Coord_t LineHeight = EG_FontGetLineHeight(pFont);
		if(LineHeight > MaxLineHeight) MaxLineHeight = LineHeight;
	}
	return MaxLineHeight;
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGSpanGroup::GetExpandWidth(uint32_t MaxWidth)
{
EG_Span_t *pSpan;
EG_Coord_t Kerning = 0;

  if(m_SpanList.GetHead() == nullptr) return 0;
	uint32_t Width = EG_COORD_IS_PCT(m_Indent) ? 0 : m_Indent;
  POSITION Pos = m_SpanList.GetHeadPosition();
  while(Pos != nullptr){
	  pSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
		const EG_Font_t *pFont = GetSpanStyleFont(pSpan);
		Kerning = GetSpanStyleKerning(pSpan);
		uint32_t j = 0;
		const char *pText = pSpan->pText;
		CheckSpanText(&pText);
		while(pText[j] != '\0') {
			if(MaxWidth > 0 && Width >= MaxWidth) {
				return MaxWidth;
			}
			uint32_t Chr = EG_TextDecodeNext(pText, &j);
			uint32_t NextChr = EG_TextDecodeNext(&pText[j], NULL);
			uint16_t ChrWidth = EG_FontGetGlyphWidth(pFont, Chr, NextChr);
			Width = Width + ChrWidth + Kerning;
		}
	}
	return Width - Kerning;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGSpanGroup::GetExpandHeight(EG_Coord_t Width)
{
POSITION Pos;

	if((m_SpanList.GetHead(Pos) == NULL) || (Width <= 0)) return 0;
	//  init draw variable 
	EG_TextFlag_t TextFlag = EG_TEXT_FLAG_NONE;
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t MaxWidth = Width;
	EG_Coord_t Indent = ConvertIndentPCT(MaxWidth);
	EG_Coord_t MaxW = MaxWidth - Indent; //  first line need minus indent 

	//  Rect of draw pSpan-txt 
	EGPoint TextPos;
	TextPos.m_Y = 0;
	TextPos.m_X = 0 + Indent; //  first line need add indent 

	EG_Span_t *pSpan = (EG_Span_t*)m_SpanList.GetHead(Pos);
	const char *pText = pSpan->pText;
	CheckSpanText(&pText);
	uint32_t CurrentOffset = 0;
	EG_Snippet_t Snippet; //  use to save pSpan info and push it to stack 
	memset(&Snippet, 0, sizeof(Snippet));
	int32_t LineCount = 0;
	int32_t Lines = m_Lines < 0 ? INT32_MAX : m_Lines;
	//  the loop control how many lines need to draw 
	while(pSpan) {
		int SnippetCount = 0;
		EG_Coord_t MaxLineHeight = 0; //  the max height of span-font when a line have a lot of span 
		while(1) {		//  the loop control to find a line and push the relevant pSpan info into stack  
			if(pText[CurrentOffset] == '\0') {			//  switch to the next pSpan when current is end 
				pSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
				if(pSpan == NULL) break;
				pText = pSpan->pText;
				CheckSpanText(&pText);
				CurrentOffset = 0;
				continue;				//  maybe also pText[CurrentOffset] == '\0' 
			}
			if(CurrentOffset == 0) {			//  init span info to Snippet. 
				Snippet.pSpan = pSpan;
				Snippet.pFont = GetSpanStyleFont(pSpan);
				Snippet.Kerning = GetSpanStyleKerning(pSpan);
				Snippet.LineHeight = EG_FontGetLineHeight(Snippet.pFont) + LineSpace;
			}
			uint32_t NextOffset = 0;			//  get current span text line info 
			EG_Coord_t UseWidth = 0;
			bool isfill = GetSnippetInfo(&pText[CurrentOffset], Snippet.pFont, Snippet.Kerning, MaxW, TextFlag, &UseWidth, &NextOffset);
			if(isfill && NextOffset > 0 && SnippetCount > 0) {			//  break word deal Width 
				if(MaxW < UseWidth) {
					break;
				}
				uint32_t TempOffset = NextOffset;
				uint32_t Chr = EG_TextDecodePrevious(&pText[CurrentOffset], &TempOffset);
				if(!(Chr == '\0' || Chr == '\n' || Chr == '\r' || EG_TextIsBreak(Chr))) {
					TempOffset = 0;
					Chr = EG_TextDecodeNext(&pText[CurrentOffset + NextOffset], &TempOffset);
					if(!(Chr == '\0' || Chr == '\n' || Chr == '\r' || EG_TextIsBreak(Chr))) break;
				}
			}
			Snippet.pText = &pText[CurrentOffset];
			Snippet.Bytes = NextOffset;
			Snippet.TextWidth = UseWidth;
			CurrentOffset += NextOffset;
			if(MaxLineHeight < Snippet.LineHeight) MaxLineHeight = Snippet.LineHeight;
			SnippetCount++;
			MaxW = MaxW - UseWidth - Snippet.Kerning;
			if(isfill || MaxW <= 0) break;
		}
		TextPos.m_X = 0;		//  next line init 
		TextPos.m_Y += MaxLineHeight;
		MaxW = MaxWidth;
		LineCount += 1;
		if(LineCount >= Lines) break;

	}
	TextPos.m_Y -= LineSpace;
	return TextPos.m_Y;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(SPAN_CLASS) != EG_RES_OK) return;// Call the ancestor's event handler
	EGSpanGroup *pSpanGroup = (EGSpanGroup*)pEvent->GetTarget();
  pSpanGroup->Event(pEvent);  // Dereference once

}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_DRAW_MAIN: {
      DrawMain(pEvent);
      break;
    }
    case EG_EVENT_STYLE_CHANGED:
    case EG_EVENT_SIZE_CHANGED: {
      RefreshSpanSize();
      break;
    }
    case EG_EVENT_GET_SELF_SIZE: {
      EG_Coord_t Width = 0;
      EG_Coord_t height = 0;
      EGPoint *pSize = (EGPoint*)pEvent->GetParam();
      if(m_Mode == EG_SPAN_MODE_EXPAND) {
        if(m_Refresh) {
          m_CacheWidth = (EG_Coord_t)GetExpandWidth(0);
          m_CacheHeight = GetMaxLineHeight();
          m_Refresh = 0;
        }
        Width = m_CacheWidth;
        height = m_CacheHeight;
      }
      else if(m_Mode == EG_SPAN_MODE_BREAK) {
        Width = GetContentWidth();
        if(pSize->m_Y >= 0) {
          if(Width != m_CacheWidth || m_Refresh) {
            height = GetExpandHeight(Width);
            m_CacheWidth = Width;
            m_CacheHeight = height;
            m_Refresh = 0;
          }
          else {
            height = m_CacheHeight;
          }
        }
      }
      else if(m_Mode == EG_SPAN_MODE_FIXED) {
        Width = pSize->m_X >= 0 ? GetContentWidth() : 0;
        height = pSize->m_Y >= 0 ? GetContentHeight() : 0;
      }
      pSize->m_X = EG_MAX(pSize->m_X, Width);
      pSize->m_Y = EG_MAX(pSize->m_Y, height);
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSpanGroup::GetSnippetInfo(const char *txt, const EG_Font_t *pFont,
															 EG_Coord_t Kerning, EG_Coord_t MaxWidth, EG_TextFlag_t flag,
															 EG_Coord_t *UseWidth, uint32_t *end_ofs)
{
	if(txt == NULL || txt[0] == '\0') {
		*end_ofs = 0;
		*UseWidth = 0;
		return false;
	}

	uint32_t ofs = EG_GetNextTextLine(txt, pFont, Kerning, MaxWidth, UseWidth, flag);
	*end_ofs = ofs;

	if(txt[ofs] == '\0' && *UseWidth < MaxWidth) {
		return false;
	}
	else {
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::PushSnippet(EG_Snippet_t *pItem)
{
	if(snippet_stack.index < EG_SPAN_SNIPPET_STACK_SIZE) {
		memcpy(&snippet_stack.stack[snippet_stack.index], pItem, sizeof(EG_Snippet_t));
		snippet_stack.index++;
	}
	else {
		EG_LOG_ERROR("pSpan draw stack overflow, please set EG_SPAN_SNIPPET_STACK_SIZE too larger");
	}
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGSpanGroup::GetSnippetCount(void)
{
	return snippet_stack.index;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Snippet_t* EGSpanGroup::GetSnippet(uint16_t Index)
{
	return &snippet_stack.stack[Index];
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::ClearSnippet(void)
{
	snippet_stack.index = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

const EG_Font_t* EGSpanGroup::GetSpanStyleFont(EG_Span_t *pSpan)
{
const EG_Font_t *pFont;
EG_StyleValue_t Value;

	if(pSpan->Style.GetProperty(EG_STYLE_TEXT_FONT, &Value) != EG_RES_OK) pFont = GetStyleTextFont(EG_PART_MAIN);
	else pFont = (const EG_Font_t *)Value.pPtr;
	return pFont;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGSpanGroup::GetSpanStyleKerning(EG_Span_t *pSpan)
{
EG_Coord_t Kerning;
EG_StyleValue_t Value;

	if(pSpan->Style.GetProperty(EG_STYLE_TEXT_LETTER_SPACE, &Value) != EG_RES_OK) {
		Kerning = GetStyleTextKerning(EG_PART_MAIN);
	}
	else Kerning = (EG_Coord_t)Value.Number;
	return Kerning;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGSpanGroup::GetSpanStyleColor(EG_Span_t *pSpan)
{
EG_StyleValue_t Value;

	if(pSpan->Style.GetProperty(EG_STYLE_TEXT_COLOR, &Value) != EG_RES_OK) {
		Value.Color = GetStyleTextColor(EG_PART_MAIN);
	}
	return Value.Color;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_OPA_t EGSpanGroup::GetSpanStyleOPA(EG_Span_t *pSpan)
{
EG_OPA_t OPA;
	EG_StyleValue_t Value;
	if(pSpan->Style.GetProperty(EG_STYLE_TEXT_OPA, &Value) != EG_RES_OK) {
		OPA = (EG_OPA_t)GetStyleTextOPA(EG_PART_MAIN);
	}
	else OPA = (EG_OPA_t)Value.Number;
	return OPA;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_BlendMode_e EGSpanGroup::GetSpanStyleBlendMode(EG_Span_t *pSpan)
{
EG_BlendMode_e Mode;
EG_StyleValue_t Value;

	if(pSpan->Style.GetProperty(EG_STYLE_BLEND_MODE, &Value) != EG_RES_OK) {
		Mode = (EG_BlendMode_e)GetStyleBlendMode(EG_PART_MAIN);
	}
	else {
		Mode = (EG_BlendMode_e)Value.Number;
	}
	return Mode;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_TextDecor_e EGSpanGroup::GetSpanStyleDecoration(EG_Span_t *pSpan)
{
EG_TextDecor_e Decor;
EG_StyleValue_t Value;

	if(pSpan->Style.GetProperty(EG_STYLE_TEXT_DECOR, &Value) != EG_RES_OK) {
		Decor = (EG_TextDecor_e)GetStyleTextDecoration(EG_PART_MAIN);
	}
	else Decor = (EG_TextDecor_e)Value.Number;
	return Decor;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGSpanGroup::ConvertIndentPCT(EG_Coord_t Width)
{
	EG_Coord_t Indent = m_Indent;
	if(EG_COORD_IS_PCT(m_Indent)) {
		if(m_Mode == EG_SPAN_MODE_EXPAND) {
			Indent = 0;
		}
		else {
			Indent = (Width * EG_COORD_GET_PCT(m_Indent)) / 100;
		}
	}

	return Indent;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::DrawMain(EGEvent *pEvent)
{
POSITION Pos;
	EGRect Rect;

	EGDrawContext *pContext = pEvent->GetDrawContext();
	GetContentArea(&Rect);
	if(m_SpanList.IsEmpty()) return;	//  return if no spans
	EGRect ClipRect;
	if(!ClipRect.Intersect(&Rect, pContext->m_pClipRect)) return; //  return if no draw area 
	const EGRect *clip_area_ori = pContext->m_pClipRect;
	pContext->m_pClipRect = &ClipRect;
	EG_TextFlag_t TextFlag = EG_TEXT_FLAG_NONE;	//  init draw variable 
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t MaxWidth = Rect.GetWidth();
	EG_Coord_t Indent = ConvertIndentPCT(MaxWidth);
	EG_Coord_t MaxW = MaxWidth - Indent; //  first line need minus indent 
	EG_OPA_t obj_opa = GetOPARecursive(this, EG_PART_MAIN);
	EGPoint TextPos;	//  Rect of draw pSpan-txt 
	TextPos.m_Y = Rect.GetY1();
	TextPos.m_X = Rect.GetX1() + Indent; //  first line need add indent 
	EG_Span_t *pSpan = (EG_Span_t*)m_SpanList.GetHead(Pos);
	const char *pText = pSpan->pText;
	CheckSpanText(&pText);
	uint32_t CurrentOffset = 0;
	EG_Snippet_t Snippet; //  use to save pSpan info and push it to stack 
	EG_ZeroMem(&Snippet, sizeof(Snippet));
	EGDrawLabel DrawLabel;
	bool IsFirstLine = true;
	//  the loop control how many lines need to draw 
	while(pSpan) {
		bool is_end_line = false;
		bool ellipsis_valid = false;
		EG_Coord_t MaxLineHeight = 0;   //  the max height of span-font when a line have a lot of pSpan 
		EG_Coord_t max_baseline = 0; // baseline of the highest pSpan
		ClearSnippet();
		while(1) {		//  the loop control to find a line and push the relevant pSpan info into stack  
			if(pText[CurrentOffset] == '\0') {			//  switch to the next pSpan when current is end 
				pSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
				if(pSpan == NULL) break;
				pText = pSpan->pText;
				CheckSpanText(&pText);
				CurrentOffset = 0;
				continue;				//  maybe also pText[CurrentOffset] == '\0' 
			}
			if(CurrentOffset == 0) {			//  init pSpan info to Snippet. 
				Snippet.pSpan = pSpan;
				Snippet.pFont = GetSpanStyleFont(pSpan);
				Snippet.Kerning = GetSpanStyleKerning(pSpan);
				Snippet.LineHeight = EG_FontGetLineHeight(Snippet.pFont) + LineSpace;
			}
			uint32_t NextOffset = 0;			//  get current pSpan text line info 
			EG_Coord_t UseWidth = 0;
			bool isfill = GetSnippetInfo(&pText[CurrentOffset], Snippet.pFont, Snippet.Kerning, MaxW, TextFlag, &UseWidth, &NextOffset);
			if(isfill) {
				if(NextOffset > 0 && GetSnippetCount() > 0) {
					//  To prevent infinite loops, the EG_GetNextTextLine() may return incomplete words, 
					//  This phenomenon should be avoided when lv_get_snippet_cnt() > 0 
					if(MaxW < UseWidth) break;
					uint32_t TempOffset = NextOffset;
					uint32_t Chr = EG_TextDecodePrevious(&pText[CurrentOffset], &TempOffset);
					if(!(Chr == '\0' || Chr == '\n' || Chr == '\r' || EG_TextIsBreak(Chr))) {
						TempOffset = 0;
						Chr = EG_TextDecodeNext(&pText[CurrentOffset + NextOffset], &TempOffset);
						if(!(Chr == '\0' || Chr == '\n' || Chr == '\r' || EG_TextIsBreak(Chr))) {
							break;
						}
					}
				}
			}
			Snippet.pText = &pText[CurrentOffset];
			Snippet.Bytes = NextOffset;
			Snippet.TextWidth = UseWidth;
			CurrentOffset += NextOffset;
			if(MaxLineHeight < Snippet.LineHeight) {
				MaxLineHeight = Snippet.LineHeight;
				max_baseline = Snippet.pFont->BaseLine;
			}
			PushSnippet(&Snippet);
			MaxW = MaxW - UseWidth - Snippet.Kerning;
			if(isfill || MaxW <= 0) {
				break;
			}
		}
		uint16_t item_cnt = GetSnippetCount();		//  start current line deal with 
		if(item_cnt == 0) { //  break if stack is empty 
			break;
		}
		{		//  Whether the current line is the end line and does overflow processing 
			EG_Snippet_t *pLastSnippet = GetSnippet(item_cnt - 1);
			EG_Coord_t NextLineHeight = pLastSnippet->LineHeight;
			if(pLastSnippet->pText[pLastSnippet->Bytes] == '\0') {
				NextLineHeight = 0;
				EG_Span_t *pNextSpan = (EG_Span_t*)m_SpanList.GetNext(Pos);
				if(pNextSpan) { //  have the next line 
					NextLineHeight = EG_FontGetLineHeight(GetSpanStyleFont(pNextSpan)) + LineSpace;
				}
			}
			if(TextPos.m_Y + MaxLineHeight + NextLineHeight - LineSpace > Rect.GetY2() + 1) { //  for overflow if is end line. 
				if(pLastSnippet->pText[pLastSnippet->Bytes] != '\0') {
					pLastSnippet->Bytes = strlen(pLastSnippet->pText);
					pLastSnippet->TextWidth = EG_GetTextWidth(pLastSnippet->pText, pLastSnippet->Bytes, pLastSnippet->pFont,
																								 pLastSnippet->Kerning, TextFlag);
				}
				ellipsis_valid = m_Overflow == EG_SPAN_OVERFLOW_ELLIPSIS ? true : false;
				is_end_line = true;
			}
		}
		if(TextPos.m_Y + MaxLineHeight < ClipRect.GetY1()) {		// Go to the first visible line
			goto Next_line_init;
		}
	{
		EG_TextAlignment_t align = GetStyleTextAlign(EG_PART_MAIN);		//  align deal with 
		if(align == EG_TEXT_ALIGN_CENTER || align == EG_TEXT_ALIGN_RIGHT) {
			EG_Coord_t align_ofs = 0;
			EG_Coord_t txts_w = IsFirstLine ? Indent : 0;
			for(int i = 0; i < item_cnt; i++) {
				EG_Snippet_t *pSnippet = GetSnippet(i);
				txts_w = txts_w + pSnippet->TextWidth + pSnippet->Kerning;
			}
			txts_w -= GetSnippet(item_cnt - 1)->Kerning;
			align_ofs = MaxWidth > txts_w ? MaxWidth - txts_w : 0;
			if(align == EG_TEXT_ALIGN_CENTER) {
				align_ofs = align_ofs >> 1;
			}
			TextPos.m_X += align_ofs;
		}
		for(int i = 0; i < item_cnt; i++) {
			EG_Snippet_t *pSnippet = GetSnippet(i);
			const char *bidi_txt = pSnippet->pText;			//  bidi deal with:todo 
			EGPoint pos;
			pos.m_X = TextPos.m_X;
			pos.m_Y = TextPos.m_Y + MaxLineHeight - pSnippet->LineHeight - (max_baseline - pSnippet->pFont->BaseLine);
			DrawLabel.m_Color = GetSpanStyleColor(pSnippet->pSpan);
			DrawLabel.m_OPA = GetSpanStyleOPA(pSnippet->pSpan);
			DrawLabel.m_pFont = GetSpanStyleFont(pSnippet->pSpan);
			DrawLabel.m_BlendMode = GetSpanStyleBlendMode(pSnippet->pSpan);
			if(obj_opa < EG_OPA_MAX) {
				DrawLabel.m_OPA = (uint16_t)((uint16_t)DrawLabel.m_OPA * obj_opa) >> 8;
			}
			uint32_t txt_bytes = pSnippet->Bytes;
			uint16_t dot_letter_w = 0;			//  overflow 
			uint16_t dot_width = 0;
			if(ellipsis_valid) {
				dot_letter_w = EG_FontGetGlyphWidth(pSnippet->pFont, '.', '.');
				dot_width = dot_letter_w * 3;
			}
			EG_Coord_t ellipsis_width = Rect.GetX1() + MaxWidth - dot_width;

			uint32_t j = 0;
			while(j < txt_bytes) {
				if(pos.m_X > ClipRect.GetX2()) break;				//  skip invalid fields 
				uint32_t Chr = EG_TextDecodeNext(bidi_txt, &j);
				uint32_t NextChr = EG_TextDecodeNext(&bidi_txt[j], NULL);
				int32_t ChrWidth = EG_FontGetGlyphWidth(pSnippet->pFont, Chr, NextChr);
				if(pos.m_X + ChrWidth + pSnippet->Kerning < ClipRect.GetX1()) {				//  skip invalid fields 
					if(ChrWidth > 0) {
						pos.m_X = pos.m_X + ChrWidth + pSnippet->Kerning;
					}
					continue;
				}
				if(ellipsis_valid && pos.m_X + ChrWidth + pSnippet->Kerning > ellipsis_width) {
					for(int ell = 0; ell < 3; ell++) {
						DrawLabel.DrawChar(pContext, &pos, '.');
						pos.m_X = pos.m_X + dot_letter_w + pSnippet->Kerning;
					}
					if(pos.m_X <= ellipsis_width) {
						pos.m_X = ellipsis_width + 1;
					}
					break;
				}
				else {
					DrawLabel.DrawChar(pContext, &pos, Chr);
					if(ChrWidth > 0) {
						pos.m_X = pos.m_X + ChrWidth + pSnippet->Kerning;
					}
				}
			}
			EG_TextDecor_e decor = GetSpanStyleDecoration(pSnippet->pSpan);	//  draw decorations 
			if(decor != EG_TEXT_DECOR_NONE) {
				EGDrawLine DrawLine;
				DrawLine.m_Color = DrawLabel.m_Color;
				DrawLine.m_Width = DrawLabel.m_pFont->UnderlineThickness ? pSnippet->pFont->UnderlineThickness : 1;
				DrawLine.m_OPA = DrawLabel.m_OPA;
				DrawLine.m_BlendMode = DrawLabel.m_BlendMode;
				if(decor & EG_TEXT_DECOR_STRIKETHROUGH) {
					EGPoint Point1,	Point2;
					Point1.m_X = TextPos.m_X;
					Point1.m_Y = pos.m_Y + ((pSnippet->LineHeight - LineSpace) >> 1) + (DrawLine.m_Width >> 1);
					Point2.m_X = pos.m_X;
					Point2.m_Y = Point1.m_Y;
					DrawLine.Draw(pContext, &Point1, &Point2);
				}
				if(decor & EG_TEXT_DECOR_UNDERLINE) {
					EGPoint Point1;
					EGPoint Point2;
					Point1.m_X = TextPos.m_X;
					Point1.m_Y = pos.m_Y + pSnippet->LineHeight - LineSpace - pSnippet->pFont->BaseLine - pSnippet->pFont->UnderlinePosition;
					Point2.m_X = pos.m_X;
					Point2.m_Y = Point1.m_Y;
					DrawLine.Draw(pContext, &Point1, &Point2);
				}
			}
			TextPos.m_X = pos.m_X;
		}
	}
Next_line_init:

		//  next line init 
		IsFirstLine = false;
		TextPos.m_X = Rect.GetX1();
		TextPos.m_Y += MaxLineHeight;
		if(is_end_line || TextPos.m_Y > ClipRect.GetY2() + 1) {
			pContext->m_pClipRect = clip_area_ori;
			return;
		}
		MaxW = MaxWidth;
	}
	pContext->m_pClipRect = clip_area_ori;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpanGroup::RefreshSpanSize(void)
{
	Invalidate();
	RefreshSelfSize();
}

#endif
