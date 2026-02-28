/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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

#include "widgets/EG_Label.h"
#if EG_USE_LABEL != 0
#include "core/EG_Object.h"
#include "misc/EG_Assert.h"
#include "core/EG_Group.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Color.h"
#include "misc/EG_Math.h"
#include "misc/lv_bidi.h"
#include "misc/lv_txt_ap.h"
#include "misc/lv_printf.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_LABEL_DEF_SCROLL_SPEED (EGDisplay::GetDPI(GetDisplay()) / 3)
#define EG_LABEL_SCROLL_DELAY 300
#define EG_LABEL_DOT_END_INV 0xFFFFFFFF
#define EG_LABEL_HINT_HEIGHT_LIMIT 1024 // Enable "hint" to buffer info about labels larger than this. (Speed up drawing)

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_LabelClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGLabel::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGLabel::EGLabel(void) : EGObject(),
 	m_pText(nullptr),
	m_DotEnd(EG_LABEL_DOT_END_INV),
  m_SelectionStart(0),
  m_SelectionEnd(0),
	m_LongMode(EG_LABEL_LONG_WRAP),
	m_StaticText(0),
	m_Recolor(0),
	m_DotAllocated(0)
{
	m_Dot.pTemp = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

EGLabel::EGLabel(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= LABEL_CLASS*/) :
  EGObject(),
 	m_pText(nullptr),
	m_DotEnd(EG_LABEL_DOT_END_INV),
  m_SelectionStart(0),
  m_SelectionEnd(0),
	m_LongMode(EG_LABEL_LONG_WRAP),
	m_StaticText(0),
	m_Recolor(0),
	m_DotAllocated(0)
{
	m_Dot.pTemp = nullptr;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGLabel::~EGLabel(void)
{
	FreeDotTemp();
	if(!m_StaticText) EG_FreeMem(m_pText);
	m_pText = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::Configure(void)
{
  EG_LOG_INFO("[Label ]", "Configure.");
  EGObject::Configure();
	m_pText = nullptr;
	m_StaticText = 0;
	m_Recolor = 0;
	m_DotEnd = EG_LABEL_DOT_END_INV;
	m_LongMode = EG_LABEL_LONG_WRAP;
	m_Offset.m_X = 0;
	m_Offset.m_Y = 0;
#if EG_LABEL_LONG_TXT_HINT
	m_Hint.LineStart = -1;
	m_Hint.CoordY = 0;
	m_Hint.Y = 0;
#endif
#if EG_LABEL_TEXT_SELECTION
	m_SelectionStart = EG_DRAW_LABEL_NO_TXT_SEL;
	m_SelectionEnd = EG_DRAW_LABEL_NO_TXT_SEL;
#endif
	m_Dot.pTemp = nullptr;
	m_DotAllocated = 0;
	ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	SetLongMode(EG_LABEL_LONG_WRAP);
	SetText("Text");
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetText(const char *pText)
{
	Invalidate();
	if(pText == nullptr) pText = m_pText;	// If text is NULL then just refresh with the current text
	if(m_pText == pText && m_StaticText == 0) {
#if EG_USE_ARABIC_PERSIAN_CHARS// If set its own text then reallocate it (maybe its Size changed)
		size_t Length = _lv_txt_ap_calc_bytes_cnt(pText);		// Get the Size of the text and process it
		m_pText = EG_ReallocMem(m_pText, Length);
		EG_ASSERT_MALLOC(m_pText);
		if(m_pText == nullptr) return;
		_lv_txt_ap_proc(m_pText, m_pText);
#else
		m_pText = (char*)EG_ReallocMem(m_pText, strlen(m_pText) + 1);
#endif
		EG_ASSERT_MALLOC(m_pText);
		if(m_pText == nullptr) return;
	}
	else {
		if(m_pText != nullptr && m_StaticText == 0) {		// Free the old text
			EG_FreeMem(m_pText);
			m_pText = nullptr;
		}
#if EG_USE_ARABIC_PERSIAN_CHARS
		size_t Length = _lv_txt_ap_calc_bytes_cnt(pText);		// Get the Size of the text and process it
		m_pText = EG_AllocMem(Length);
		EG_ASSERT_MALLOC(m_pText);
		if(m_pText == nullptr) return;
		_lv_txt_ap_proc(pText, m_pText);
#else
		size_t Length = strlen(pText) + 1;		// Get the Size of the text
		m_pText = (char*)EG_AllocMem(Length);		// Allocate space for the new text
		EG_ASSERT_MALLOC(m_pText);
		if(m_pText == nullptr) return;
		strcpy(m_pText, pText);
#endif
		m_StaticText = 0;		// Now the text is dynamically allocated
	}
	RefreshText();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetFormatText(const char *pFormat, ...)
{
	EG_ASSERT_NULL(pFormat);
	Invalidate();
	if(pFormat == nullptr) {	// If text is NULL then refresh
		RefreshText();
		return;
	}
	if(m_pText != nullptr && m_StaticText == 0) {
		EG_FreeMem(m_pText);
		m_pText = nullptr;
	}
	va_list args;
	va_start(args, pFormat);
	m_pText = EG_TextFormat(pFormat, args);
	va_end(args);
	m_StaticText = 0; // Now the text is dynamically allocated
	RefreshText();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetStaticText(const char *pText)
{
	if(m_StaticText == 0 && m_pText != nullptr) {
		EG_FreeMem(m_pText);
		m_pText = nullptr;
	}
	if(pText != nullptr) {
		m_StaticText = 1;
		m_pText = (char *)pText;
	}
	RefreshText();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetLongMode(EG_LabelLongMode_e LongMode)
{
	EGAnimate::Delete(this, SetAnimateOffsetX);	// Delete the old animation (if exists)
  EGAnimate::Delete(this, SetAnimateOffsetY);
	m_Offset.m_X = 0;
	m_Offset.m_Y = 0;

	if(LongMode == EG_LABEL_LONG_SCROLL || LongMode == EG_LABEL_LONG_SCROLL_CIRCULAR || LongMode == EG_LABEL_LONG_CLIP) m_Expand = 1;
	else m_Expand = 0;
	if(m_LongMode == EG_LABEL_LONG_DOT && m_DotEnd != EG_LABEL_DOT_END_INV) {	// Restore the character under the dots
		RevertDots();
	}
	m_LongMode = LongMode;
	RefreshText();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetRecolor(bool Enable)
{
	if(m_Recolor == Enable) return;
	m_Recolor = Enable == false ? 0 : 1;
	RefreshText();	// Refresh the text because the potential color codes in text needs to be hidden or revealed
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetSelectionStart(uint32_t Index)
{
#if EG_LABEL_TEXT_SELECTION
	m_SelectionStart = Index;
	Invalidate();
#else
	EG_UNUSED(obj);   // Unused
	EG_UNUSED(index); // Unused
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetSelectionEnd(uint32_t Index)
{
#if EG_LABEL_TEXT_SELECTION
	m_SelectionEnd = Index;
	Invalidate();
#else
	EG_UNUSED(obj);   // Unused
	EG_UNUSED(index); // Unused
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

char* EGLabel::GetText(void)
{
	return m_pText;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_LabelLongMode_e EGLabel::GetLongMode(void)
{
	return m_LongMode;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGLabel::GetRecolor(void)
{
	return m_Recolor == 0 ? false : true;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::GetCharacterPosition(uint32_t Index, EGPoint *pPosition)
{
	EG_ASSERT_NULL(pPosition);
	const char *pText = GetText();
	EG_TextAlignment_t Align = CalculateTextAlignment(EG_PART_MAIN, pText);
	if(pText[0] == '\0') {
		pPosition->m_Y = 0;
		switch(Align) {
			case EG_TEXT_ALIGN_LEFT:
				pPosition->m_X = 0;
				break;
			case EG_TEXT_ALIGN_RIGHT:
				pPosition->m_X = GetContentWidth();
				break;
			case EG_TEXT_ALIGN_CENTER:
				pPosition->m_X = GetContentWidth() / 2;
				break;
		}
		return;
	}
	EGRect TextRect;
	GetContentArea(&TextRect);
	uint32_t LineStart = 0, NewLineStart = 0;
	EG_Coord_t MaxWidth = TextRect.GetWidth();
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
	EG_Coord_t CharHeight = EG_FontGetLineHeight(pFont);
	EG_Coord_t PosY = 0;
	EG_TextFlag_t Flags = EG_TEXT_FLAG_NONE;
	if(m_Recolor != 0) Flags |= EG_TEXT_FLAG_RECOLOR;
	if(m_Expand != 0) Flags |= EG_TEXT_FLAG_EXPAND;
	if(GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT && !m_WidthLayout) Flags |= EG_TEXT_FLAG_FIT;
	uint32_t ByteIndex = EG_TextEncodedGetIndex(pText, Index);
	while(pText[NewLineStart] != '\0') {	// Search the line of the index character;
		NewLineStart += EG_GetNextTextLine(&pText[LineStart], pFont, Kerning, MaxWidth, NULL, Flags);
		if(ByteIndex < NewLineStart || pText[NewLineStart] == '\0')	break; // The line of 'index' character begins at 'LineStart'
		PosY += CharHeight + LineSpace;
		LineStart = NewLineStart;
	}
	if(ByteIndex > 0) {	// If the last character is a line break then go to the next line
		if((pText[ByteIndex - 1] == '\n' || pText[ByteIndex - 1] == '\r') && pText[ByteIndex] == '\0') {
			PosY += CharHeight + LineSpace;
			LineStart = ByteIndex;
		}
	}
	const char *pBiDiText;
	uint32_t visual_byte_pos;
#if EG_USE_BIDI
	EG_BaseDirection_e BaseDirection = EG_GetStyleBaseDirection(obj, EG_PART_MAIN);
	if(BaseDirection == EG_BASE_DIR_AUTO) BaseDirection = _lv_bidi_detect_base_dir(pText);

	char *mutable_bidi_txt = NULL;
	// Handle Bidi
	if(NewLineStart == ByteIndex) {
		visual_byte_pos = (BaseDirection == EG_BASE_DIR_RTL) ? 0 : ByteIndex - LineStart;
		pBiDiText = &pText[LineStart];
	}
	else {
		uint32_t line_char_id = EG_TextEncodedGetPosition(&pText[LineStart], ByteIndex - LineStart);

		bool IsRtL;
		uint32_t visual_char_pos = _lv_bidi_get_visual_pos(&pText[LineStart], &mutable_bidi_txt, NewLineStart - LineStart,
																											 BaseDirection, line_char_id, &IsRtL);
		pBiDiText = mutable_bidi_txt;
		if(IsRtL) visual_char_pos++;

		visual_byte_pos = EG_TextEncodedGetIndex(pBiDiText, visual_char_pos);
	}
#else
	pBiDiText = &pText[LineStart];
	visual_byte_pos = ByteIndex - LineStart;
#endif
	// Calculate the x coordinate
	EG_Coord_t PosX = EG_GetTextWidth(pBiDiText, visual_byte_pos, pFont, Kerning, Flags);
	if(Index != LineStart) PosX += Kerning;
	if(Align == EG_TEXT_ALIGN_CENTER) {
		EG_Coord_t LineWidth = EG_GetTextWidth(pBiDiText, NewLineStart - LineStart, pFont, Kerning, Flags);
		PosX += TextRect.GetWidth() / 2 - LineWidth / 2;
	}
	else if(Align == EG_TEXT_ALIGN_RIGHT) {
		EG_Coord_t LineWidth = EG_GetTextWidth(pBiDiText, NewLineStart - LineStart, pFont, Kerning, Flags);
		PosX += TextRect.GetWidth() - LineWidth;
	}
	pPosition->m_X = PosX;
	pPosition->m_Y = PosY;
#if EG_USE_BIDI
	if(mutable_bidi_txt) EG_ReleaseBufferMem(mutable_bidi_txt);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGLabel::GetCharacterAt(EGPoint *pPosition)
{
	EG_ASSERT_NULL(pPosition);
	EGPoint Point;
	Point.m_X = pPosition->m_X - GetStylePadLeft(EG_PART_MAIN);
	Point.m_Y = pPosition->m_Y - GetStylePadTop(EG_PART_MAIN);
	EGRect TextRect;
	GetContentArea( &TextRect);
	const char *pText = GetText();
	uint32_t LineStart = 0;
	uint32_t NewLineStart = 0;
	EG_Coord_t MaxWidth = TextRect.GetWidth();
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
	EG_Coord_t CharHeight = EG_FontGetLineHeight(pFont);
	EG_Coord_t PosY = 0;
	EG_TextFlag_t Flags = EG_TEXT_FLAG_NONE;
	uint32_t logical_pos;
	char *pBiDiText;
	if(m_Recolor != 0) Flags |= EG_TEXT_FLAG_RECOLOR;
	if(m_Expand != 0) Flags |= EG_TEXT_FLAG_EXPAND;
	if(GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT && !m_WidthLayout) Flags |= EG_TEXT_FLAG_FIT;
	EG_TextAlignment_t Align = CalculateTextAlignment(EG_PART_MAIN, m_pText);
	while(pText[LineStart] != '\0') {	// Search the line of the index letter;
		NewLineStart += EG_GetNextTextLine(&pText[LineStart], pFont, Kerning, MaxWidth, NULL, Flags);
		if(Point.m_Y <= PosY + CharHeight) {
			// The line is found (stored in 'LineStart'). Include the NULL terminator in the last line
			uint32_t tmp = NewLineStart;
			uint32_t letter;
			letter = EG_TextDecodePrevious(pText, &tmp);
			if(letter != '\n' && pText[NewLineStart] == '\0') NewLineStart++;
			break;
		}
		PosY += CharHeight + LineSpace;
		LineStart = NewLineStart;
	}
#if EG_USE_BIDI
	pBiDiText = EG_GetBufferMem(NewLineStart - LineStart + 1);
	uint32_t txt_len = NewLineStart - LineStart;
	if(NewLineStart > 0 && pText[NewLineStart - 1] == '\0' && txt_len > 0) txt_len--;
	_lv_bidi_process_paragraph(pText + LineStart, pBiDiText, txt_len, EG_GetStyleBaseDirection(obj, EG_PART_MAIN), NULL, 0);
#else
	pBiDiText = (char *)pText + LineStart;
#endif
	EG_Coord_t PosX = 0;	// Calculate the x coordinate
	if(Align == EG_TEXT_ALIGN_CENTER) {
		EG_Coord_t LineWidth = EG_GetTextWidth(pBiDiText, NewLineStart - LineStart, pFont, Kerning, Flags);
		PosX += TextRect.GetWidth() / 2 - LineWidth / 2;
	}
	else if(Align == EG_TEXT_ALIGN_RIGHT) {
		EG_Coord_t LineWidth = EG_GetTextWidth(pBiDiText, NewLineStart - LineStart, pFont, Kerning, Flags);
		PosX += TextRect.GetWidth() - LineWidth;
	}
	EG_TextCommandState_t cmd_state = EG_TEXT_CMD_STATE_WAIT;
	uint32_t i = 0, i_act = 0;
	if(NewLineStart > 0) {
		while(i + LineStart < NewLineStart) {
			// Get the current letter and the next letter for kerning. Be careful 'i' already points to the next character
			uint32_t letter, letter_next;
			EG_TextDecode2(pBiDiText, &letter, &letter_next, &i);
			if((Flags & EG_TEXT_FLAG_RECOLOR) != 0) {			// Handle the recolor command
				if(EG_TextIsCommand(&cmd_state, pBiDiText[i]) != false) {
					continue; // Skip the letter if it is part of a command
				}
			}
			EG_Coord_t gw = EG_FontGetGlyphWidth(pFont, letter, letter_next);
			// Finish if the x position or the last char of the next line is reached
			if(Point.m_X < PosX + gw || i + LineStart == NewLineStart || pText[i_act + LineStart] == '\0') {
				i = i_act;
				break;
			}
			PosX += gw;
			PosX += Kerning;
			i_act = i;
		}
	}
#if EG_USE_BIDI
	// Handle Bidi
	uint32_t cid = EG_TextEncodedGetPosition(pBiDiText, i);
	if(pText[LineStart + i] == '\0') {
		logical_pos = i;
	}
	else {
		bool IsRtL;
		logical_pos = _lv_bidi_get_logical_pos(&pText[LineStart], NULL,
																					 txt_len, EG_GetStyleBaseDirection(obj, EG_PART_MAIN), cid, &IsRtL);
		if(IsRtL) logical_pos++;
	}
	EG_ReleaseBufferMem(pBiDiText);
#else
	logical_pos = EG_TextEncodedGetPosition(pBiDiText, i);
#endif

	return logical_pos + EG_TextEncodedGetPosition(pText, LineStart);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGLabel::IsCharacterAt(EGPoint *pPosition)
{
	EG_ASSERT_NULL(pPosition);

	EGRect TextRect;
	GetContentArea(&TextRect);
	const char *pText = GetText();
	uint32_t LineStart = 0;
	uint32_t NewLineStart = 0;
	EG_Coord_t MaxWidth = TextRect.GetWidth();
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
	EG_Coord_t CharHeight = EG_FontGetLineHeight(pFont);
	EG_TextAlignment_t Align = CalculateTextAlignment(EG_PART_MAIN, m_pText);
	EG_Coord_t PosY = 0;
	EG_TextFlag_t Flags = EG_TEXT_FLAG_NONE;
	if(m_Recolor != 0) Flags |= EG_TEXT_FLAG_RECOLOR;
	if(m_Expand != 0) Flags |= EG_TEXT_FLAG_EXPAND;
	if(GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT && !m_WidthLayout) Flags |= EG_TEXT_FLAG_FIT;
	while(pText[LineStart] != '\0') {	// Search the line of the index letter;
		NewLineStart += EG_GetNextTextLine(&pText[LineStart], pFont, Kerning, MaxWidth, NULL, Flags);
		if(pPosition->m_Y <= PosY + CharHeight) break; // The line is found (stored in 'LineStart')
		PosY += CharHeight + LineSpace;
		LineStart = NewLineStart;
	}
	EG_Coord_t PosX = 0;	// Calculate the x coordinate
	EG_Coord_t last_x = 0;
	if(Align == EG_TEXT_ALIGN_CENTER) {
		EG_Coord_t LineWidth = EG_GetTextWidth(&pText[LineStart], NewLineStart - LineStart, pFont, Kerning, Flags);
		PosX += TextRect.GetWidth() / 2 - LineWidth / 2;
	}
	else if(Align == EG_TEXT_ALIGN_RIGHT) {
		EG_Coord_t LineWidth = EG_GetTextWidth(&pText[LineStart], NewLineStart - LineStart, pFont, Kerning, Flags);
		PosX += TextRect.GetWidth() - LineWidth;
	}
	EG_TextCommandState_t cmd_state = EG_TEXT_CMD_STATE_WAIT;
	uint32_t i = LineStart;
	uint32_t i_current = i;
	uint32_t letter = '\0';
	uint32_t letter_next = '\0';
	if(NewLineStart > 0) {
		while(i <= NewLineStart - 1) {
			// Get the current letter and the next letter for kerning. Be careful 'i' already points to the next character
			EG_TextDecode2(pText, &letter, &letter_next, &i);
			if((Flags & EG_TEXT_FLAG_RECOLOR) != 0) {			// Handle the recolor command
				if(EG_TextIsCommand(&cmd_state, pText[i]) != false) {
					continue; // Skip the letter if it is part of a command
				}
			}
			last_x = PosX;
			PosX += EG_FontGetGlyphWidth(pFont, letter, letter_next);
			if(pPosition->m_X < PosX) {
				i = i_current;
				break;
			}
			PosX += Kerning;
			i_current = i;
		}
	}
	int32_t max_diff = EG_FontGetGlyphWidth(pFont, letter, letter_next) + Kerning + 1;
	return (pPosition->m_X >= (last_x - Kerning) && pPosition->m_X <= (last_x + max_diff));
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGLabel::GetSelectionStart(void)
{
#if EG_LABEL_TEXT_SELECTION
	return m_SelectionStart;

#else
	return EG_LABEL_TEXT_SELECTION_OFF;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t EGLabel::GetSelectionEnd(void)
{
#if EG_LABEL_TEXT_SELECTION
	return m_SelectionEnd;
#else
	return EG_LABEL_TEXT_SELECTION_OFF;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::InsertText(uint32_t Position, const char *pText)
{
	EG_ASSERT_NULL(pText);
	if(m_StaticText != 0) return;	// Can not append to static text
	Invalidate();
	size_t old_len = strlen(m_pText);	// Allocate space for the new text
	size_t ins_len = strlen(pText);
	size_t new_len = ins_len + old_len;
	m_pText = (char*)EG_ReallocMem(m_pText, new_len + 1);
	EG_ASSERT_MALLOC(m_pText);
	if(m_pText == nullptr) return;
	if(Position == EG_LABEL_POS_LAST) {
		Position = EG_TextEncodedGetLength(m_pText);
	}
	EG_TextInsert(m_pText, Position, pText);
	SetText(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::CutText(uint32_t Position, uint32_t Count)
{
	if(m_StaticText != 0) return;	// Can not remove from static text
	Invalidate();
	char *label_txt = GetText();
	EG_TextCut(label_txt, Position, Count);	// Delete the characters
	RefreshText();	// Refresh the label
}


///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(&c_LabelClass) != EG_RES_OK) return;  // Call the ancestor's event handler
	((EGLabel*)pEvent->GetTarget())->Event(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
//  if(Code == EG_EVENT_GET_SELF_SIZE)ESP_LOGI("[Label ]", "Got Event");
  switch(Code){
    case EG_EVENT_STYLE_CHANGED: {
      RevertDots();		// Revert dots for proper refresh
      RefreshText();
      break;
    }
    case EG_EVENT_REFR_EXT_DRAW_SIZE: {
    // Italic or other non-typical letters can be drawn of out of the object. It happens if BoxWidth
    // + m_OffsetX > adw_w in the glyph. To avoid this add some extra draw area. font_h / 4 is an empirical value. 
      const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
      EG_Coord_t LineHeight = EG_FontGetLineHeight(pFont);
      pEvent->SetExtDrawSize(LineHeight / 4);
      break;
    }
    case EG_EVENT_SIZE_CHANGED: {
      RevertDots();
      RefreshText();
      break;
    }
    case EG_EVENT_GET_SELF_SIZE: {
  		EGPoint Size;
      const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
      EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
      EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
      EG_TextFlag_t Flags = EG_TEXT_FLAG_NONE;
      if(m_Recolor != 0) Flags |= EG_TEXT_FLAG_RECOLOR;
      if(m_Expand != 0) Flags |= EG_TEXT_FLAG_EXPAND;
      EG_Coord_t Width = GetContentWidth();
      if((GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT) && !m_WidthLayout) Width = EG_COORD_MAX;
      else Width = GetContentWidth();
  		EG_GetTextSize(&Size, m_pText, pFont, Kerning, LineSpace, Width, Flags);
      EGPoint *pSize = (EGPoint*)pEvent->GetParam();
      pSize->m_X = EG_MAX(pSize->m_X, Size.m_X);
      pSize->m_Y = EG_MAX(pSize->m_Y, Size.m_Y);
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      DrawMain(pEvent);
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::DrawMain(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	EGRect TextRect;
	GetContentArea(&TextRect);
	EG_TextFlag_t Flags = EG_TEXT_FLAG_NONE;
	if(m_Recolor != 0) Flags |= EG_TEXT_FLAG_RECOLOR;
	if(m_Expand != 0) Flags |= EG_TEXT_FLAG_EXPAND;
	if((GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT) && !m_WidthLayout) Flags |= EG_TEXT_FLAG_FIT;
	EGDrawLabel DrawLabel;
	DrawLabel.m_OffsetX = m_Offset.m_X;
	DrawLabel.m_OffsetY = m_Offset.m_Y;
	DrawLabel.m_Flag = Flags;
	InititialseDrawLabel(EG_PART_MAIN, &DrawLabel);
	lv_bidi_calculate_align(&DrawLabel.m_Align, &DrawLabel.m_BidiDirection, m_pText);
	DrawLabel.m_SelectStart = m_SelectionStart;
	DrawLabel.m_SelectEnd = m_SelectionEnd;
	if(m_SelectionStart != EG_DRAW_LABEL_NO_TXT_SEL && m_SelectionEnd != EG_DRAW_LABEL_NO_TXT_SEL) {
		DrawLabel.m_SelectColor = GetStyleTextColorFiltered(EG_PART_SELECTED);
		DrawLabel.m_SelectBackColor = GetStyleBackColor(EG_PART_SELECTED);
	}
	/* In SCROLL and SCROLL_CIRCULAR mode the CENTER and RIGHT are pointless, so remove them.
     * (In addition, they will create misalignment in this situation)*/
	if((m_LongMode == EG_LABEL_LONG_SCROLL || m_LongMode == EG_LABEL_LONG_SCROLL_CIRCULAR) &&
		 (DrawLabel.m_Align == EG_TEXT_ALIGN_CENTER || DrawLabel.m_Align == EG_TEXT_ALIGN_RIGHT)) {
		EGPoint Size;
		EG_GetTextSize(&Size, m_pText, DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace,	EG_COORD_MAX, Flags);
		if(Size.m_X > TextRect.GetWidth()) {
			DrawLabel.m_Align = EG_TEXT_ALIGN_LEFT;
		}
	}
#if EG_LABEL_LONG_TXT_HINT
	EG_DrawLabelHint_t *pHint = &m_Hint;
	if(m_LongMode == EG_LABEL_LONG_SCROLL_CIRCULAR || TextRect.GetHeight() < EG_LABEL_HINT_HEIGHT_LIMIT) pHint = nullptr;
#else
	EG_DrawLabelHint_t *pHint = NULL;	// Just for compatibility
#endif
	EGRect TextClip;
	if(!TextClip.Intersect(&TextRect, pContext->m_pClipRect)) return;
	if(m_LongMode == EG_LABEL_LONG_WRAP) {
		EG_Coord_t ScrollTop = GetScrollTop();
		TextRect.Move(0, -ScrollTop);
		TextRect.SetY2(m_Rect.GetY2());
	}
	if(m_LongMode == EG_LABEL_LONG_SCROLL || m_LongMode == EG_LABEL_LONG_SCROLL_CIRCULAR) {
		const EGRect *pClipRect = pContext->m_pClipRect;
		pContext->m_pClipRect = &TextClip;
		DrawLabel.Draw(pContext, &TextRect, m_pText, pHint);
		pContext->m_pClipRect = pClipRect;
	}
	else {
		DrawLabel.Draw(pContext, &TextRect, m_pText, pHint);
	}
	const EGRect *pClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &TextClip;
	if(m_LongMode == EG_LABEL_LONG_SCROLL_CIRCULAR) {
		EGPoint Size;
		EG_GetTextSize(&Size, m_pText, DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace,	EG_COORD_MAX, Flags);
		if(Size.m_X > TextRect.GetWidth()) {	// Draw the text again on label to the original to make a circular effect 
			DrawLabel.m_OffsetX = m_Offset.m_X + Size.m_X +	EG_FontGetGlyphWidth(DrawLabel.m_pFont, ' ', ' ') * EG_LABEL_WAIT_CHAR_COUNT;
			DrawLabel.m_OffsetY = m_Offset.m_Y;
			DrawLabel.Draw(pContext, &TextRect, m_pText, pHint);
		}
		if(Size.m_Y > TextRect.GetHeight()) {// Draw the text again below the original to make a circular effect 
			DrawLabel.m_OffsetX = m_Offset.m_X;
			DrawLabel.m_OffsetY = m_Offset.m_Y + Size.m_Y + EG_FontGetLineHeight(DrawLabel.m_pFont);
			DrawLabel.Draw(pContext, &TextRect, m_pText, pHint);
		}
	}
	pContext->m_pClipRect = pClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::RefreshText(void)
{
	if(m_pText == nullptr) return;
#if EG_LABEL_LONG_TXT_HINT
	m_Hint.LineStart = -1; // The hint is invalid if the text changes
#endif
	EGRect TextRect;
	GetContentArea(&TextRect);
	EG_Coord_t MaxWidth = TextRect.GetWidth();
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_MAIN);
	EGPoint Size;	// Calc. the height and longest line
	EG_TextFlag_t Flags = EG_TEXT_FLAG_NONE;
	if(m_Recolor != 0) Flags |= EG_TEXT_FLAG_RECOLOR;
	if(m_Expand != 0) Flags |= EG_TEXT_FLAG_EXPAND;
	if(GetStyleWidth(EG_PART_MAIN) == EG_SIZE_CONTENT && !m_WidthLayout) Flags |= EG_TEXT_FLAG_FIT;
	EG_GetTextSize(&Size, m_pText, pFont, Kerning, LineSpace, MaxWidth, Flags);
	RefreshSelfSize();
	if(m_LongMode == EG_LABEL_LONG_SCROLL) {	// In scroll mode start an offset animation
		uint16_t AnimateSpeed = GetStyleAnimationSpeed(EG_PART_MAIN);
		if(AnimateSpeed == 0) AnimateSpeed = EG_LABEL_DEF_SCROLL_SPEED;
		EGAnimate Animate;
		Animate.SetItem(this);
		Animate.SetRepeatCount(EG_ANIM_REPEAT_INFINITE);
		Animate.SetPlaybackDelay( EG_LABEL_SCROLL_DELAY);
		Animate.SetRepeatDelay(Animate.m_PlaybackDelay);
		bool hor_anim = false;
		if(Size.m_X > TextRect.GetWidth()) {
#if EG_USE_BIDI
			int32_t start, end;
			EG_BaseDirection_e BaseDirection = EG_GetStyleBaseDirection(obj, EG_PART_MAIN);

			if(BaseDirection == EG_BASE_DIR_AUTO)
				BaseDirection = _lv_bidi_detect_base_dir(m_pText);

			if(BaseDirection == EG_BASE_DIR_RTL) {
				start = GetWidth(&TextRect) - Size.m_X;
				end = 0;
			}
			else {
				start = 0;
				end = GetWidth(&TextRect) - Size.m_X;
			}

			SetValues(&Animate, start, end);
#else
			Animate.SetValues(0, TextRect.GetWidth() - Size.m_X);
			Animate.SetExcCB(SetAnimateOffsetX);
#endif
			Animate.SetExcCB(SetAnimateOffsetX);
			EGAnimate *pCurrentAnimate = EGAnimate::Get(this, SetAnimateOffsetX);
			int32_t ActiveTime = 0;
			bool m_PlaybackNow = false;
			if(pCurrentAnimate) {
				ActiveTime = pCurrentAnimate->m_ActiveTime;
				m_PlaybackNow = pCurrentAnimate->m_PlaybackNow;
			}
			if(ActiveTime < Animate.m_Time) {
				Animate.m_ActiveTime= ActiveTime; // To keep the old position
				Animate.m_EarlyApply = 0;
				if(m_PlaybackNow) {
					Animate.m_PlaybackNow = 1;
					// Swap the start and end values
					int32_t tmp;
					tmp = Animate.m_StartValue;
					Animate.m_StartValue = Animate.m_EndValue;
					Animate.m_EndValue = tmp;
				}
			}

			Animate.SetTime(EGAnimate::SpeedToTime(AnimateSpeed, Animate.m_StartValue, Animate.m_EndValue));
			Animate.SetPlaybackTime(Animate.m_Time);
			EGAnimate::Create(&Animate);
			hor_anim = true;
		}
		else {
			EGAnimate::Delete(this, SetAnimateOffsetX);		// Delete the offset animation if not required
			m_Offset.m_X = 0;
		}

		if(Size.m_Y > TextRect.GetHeight() && hor_anim == false) {
			Animate.SetValues(0, TextRect.GetHeight() - Size.m_Y - (EG_FontGetLineHeight(pFont)));
			Animate.SetExcCB(SetAnimateOffsetY);

			EGAnimate *pCurrentAnimate = EGAnimate::Get(this, SetAnimateOffsetY);
			int32_t ActiveTime = 0;
			bool m_PlaybackNow = false;
			if(pCurrentAnimate) {
				ActiveTime = pCurrentAnimate->m_ActiveTime;
				m_PlaybackNow = pCurrentAnimate->m_PlaybackNow;
			}
			if(ActiveTime < Animate.m_Time) {
				Animate.m_ActiveTime= ActiveTime; // To keep the old position
				Animate.m_EarlyApply = 0;
				if(m_PlaybackNow) {
					Animate.m_PlaybackNow = 1;
					// Swap the start and end values
					int32_t tmp;
					tmp = Animate.m_StartValue;
					Animate.m_StartValue = Animate.m_EndValue;
					Animate.m_EndValue = tmp;
				}
			}
			Animate.SetTime(EGAnimate::SpeedToTime(AnimateSpeed, Animate.m_StartValue, Animate.m_EndValue));
			Animate.SetPlaybackTime(Animate.m_Time);
			EGAnimate::Create(&Animate);
		}
		else {
			EGAnimate::Delete(this, SetAnimateOffsetY);			// Delete the offset animation if not required
			m_Offset.m_Y = 0;
		}
	}
	// In roll inf. mode keep the Size but start offset animations
	else if(m_LongMode == EG_LABEL_LONG_SCROLL_CIRCULAR) {
		const EGAnimate *pAnimateTemplate = GetStyleAnimation(EG_PART_MAIN);
		uint16_t AnimateSpeed = GetStyleAnimationSpeed(EG_PART_MAIN);
		if(AnimateSpeed == 0) AnimateSpeed = EG_LABEL_DEF_SCROLL_SPEED;
		EGAnimate Animate;
		Animate.SetItem(this);
		Animate.SetRepeatCount(EG_ANIM_REPEAT_INFINITE);
		bool hor_anim = false;
		if(Size.m_X > TextRect.GetWidth()) {
#if EG_USE_BIDI
			int32_t start, end;
			EG_BaseDirection_e BaseDirection = EG_GetStyleBaseDirection(obj, EG_PART_MAIN);

			if(BaseDirection == EG_BASE_DIR_AUTO)
				BaseDirection = _lv_bidi_detect_base_dir(m_pText);

			if(BaseDirection == EG_BASE_DIR_RTL) {
				start = -Size.m_X - EG_FontGetGlyphWidth(pFont, ' ', ' ') * EG_LABEL_WAIT_CHAR_COUNT;
				end = 0;
			}
			else {
				start = 0;
				end = -Size.m_X - EG_FontGetGlyphWidth(pFont, ' ', ' ') * EG_LABEL_WAIT_CHAR_COUNT;
			}

			Animate.SetValues(start, end);
#else
			Animate.SetValues(0, -Size.m_X - EG_FontGetGlyphWidth(pFont, ' ', ' ') * EG_LABEL_WAIT_CHAR_COUNT);
#endif
			Animate.SetExcCB(SetAnimateOffsetX);
			Animate.SetTime(EGAnimate::SpeedToTime(AnimateSpeed, Animate.m_StartValue, Animate.m_EndValue));

			EGAnimate *pCurrentAnimate = EGAnimate::Get(this, SetAnimateOffsetX);
			int32_t ActiveTime = pCurrentAnimate ? pCurrentAnimate->m_ActiveTime : 0;
			if(pAnimateTemplate) {	// If Animate template animation exists, consider it's start delay and repeat delay
				Animate.m_ActiveTime= pAnimateTemplate->m_ActiveTime;
				Animate.m_RepeatDelay = pAnimateTemplate->m_RepeatDelay;
			}
			else if(ActiveTime < Animate.m_Time) {
				Animate.m_ActiveTime= ActiveTime; // To keep the old position when the label text is updated mid-scrolling
				Animate.m_EarlyApply = 0;
			}
			EGAnimate::Create(&Animate);
			hor_anim = true;
		}
		else {
			EGAnimate::Delete(this, SetAnimateOffsetX);		// Delete the offset animation if not required
			m_Offset.m_X = 0;
		}
		if(Size.m_Y > TextRect.GetHeight() && hor_anim == false) {
			Animate.SetValues(0, -Size.m_Y - (EG_FontGetLineHeight(pFont)));
			Animate.SetExcCB(SetAnimateOffsetY);
			Animate.SetTime(EGAnimate::SpeedToTime(AnimateSpeed, Animate.m_StartValue, Animate.m_EndValue));
			EGAnimate *pCurrentAnimate = EGAnimate::Get(this, SetAnimateOffsetY);
			int32_t ActiveTime = pCurrentAnimate ? pCurrentAnimate->m_ActiveTime : 0;

			// If Animate template animation exists, consider it's start delay and repeat delay
			if(pAnimateTemplate) {
				Animate.m_ActiveTime= pAnimateTemplate->m_ActiveTime;
				Animate.m_RepeatDelay = pAnimateTemplate->m_RepeatDelay;
			}
			else if(ActiveTime < Animate.m_Time) {
				Animate.m_ActiveTime= ActiveTime; // To keep the old position when the label text is updated mid-scrolling
				Animate.m_EarlyApply = 0;
			}
			EGAnimate::Create(&Animate);
		}
		else {
			EGAnimate::Delete(this, SetAnimateOffsetY);			// Delete the offset animation if not required
			m_Offset.m_Y = 0;
		}
	}
	else if(m_LongMode == EG_LABEL_LONG_DOT) {
		if(Size.m_Y <= TextRect.GetHeight()) { // No dots are required, the text is short enough
			m_DotEnd = EG_LABEL_DOT_END_INV;
		}
		else if(Size.m_Y <= EG_FontGetLineHeight(pFont)) { // No dots are required for one-line texts
			m_DotEnd = EG_LABEL_DOT_END_INV;
		}
		else if(EG_TextEncodedGetLength(m_pText) <= EG_LABEL_DOT_NUM) { // Don't turn to dots all the characters
			m_DotEnd = EG_LABEL_DOT_END_INV;
		}
		else {
			EGPoint Point;
			EG_Coord_t y_overed;
			Point.m_X = TextRect.GetWidth() -	(EG_FontGetGlyphWidth(pFont, '.', '.') + Kerning) *	EG_LABEL_DOT_NUM; // Shrink with dots
			Point.m_Y = TextRect.GetHeight();
			y_overed = Point.m_Y % (EG_FontGetLineHeight(pFont) + LineSpace); // Round down to the last line
			if(y_overed >= EG_FontGetLineHeight(pFont)) {
				Point.m_Y -= y_overed;
				Point.m_Y += EG_FontGetLineHeight(pFont);
			}
			else {
				Point.m_Y -= y_overed;
				Point.m_Y -= LineSpace;
			}
			uint32_t Index = GetCharacterAt(&Point);
			size_t txt_len = strlen(m_pText);			// Be sure there is space for the dots
			uint32_t ByteIndex = EG_TextEncodedGetIndex(m_pText, Index);
			while(ByteIndex + EG_LABEL_DOT_NUM > txt_len) {
				EG_TextDecodePrevious(m_pText, &ByteIndex);
				Index--;
			}
			uint32_t byte_id_ori = ByteIndex;			// Save letters under the dots and replace them with dots
			uint8_t Length = 0;
			for(uint32_t i = 0; i <= EG_LABEL_DOT_NUM; i++) {
				Length += EG_TextEncodedSize(&m_pText[ByteIndex]);
				EG_TextDecodeNext(m_pText, &ByteIndex);
				if(Length > EG_LABEL_DOT_NUM || ByteIndex > txt_len) break;
			}
			if(SetDotTemp(&m_pText[byte_id_ori], Length)) {
				for(uint32_t i = 0; i < EG_LABEL_DOT_NUM; i++) m_pText[byte_id_ori + i] = '.';
				m_pText[byte_id_ori + EG_LABEL_DOT_NUM] = '\0';
				m_DotEnd = Index + EG_LABEL_DOT_NUM;
			}
		}
	}
	else if(m_LongMode == EG_LABEL_LONG_CLIP) {
		// Do nothing
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::RevertDots(void)
{
	if(m_LongMode != EG_LABEL_LONG_DOT) return;
	if(m_DotEnd == EG_LABEL_DOT_END_INV) return;
	uint32_t letter_i = m_DotEnd - EG_LABEL_DOT_NUM;
	uint32_t byte_i = EG_TextEncodedGetIndex(m_pText, letter_i);
	uint8_t i = 0;  // Restore the characters
	char *dot_tmp = GetDotTemp();
	while(m_pText[byte_i + i] != '\0') {
		m_pText[byte_i + i] = dot_tmp[i];
		i++;
	}
	m_pText[byte_i + i] = dot_tmp[i];
	FreeDotTemp();
	m_DotEnd = EG_LABEL_DOT_END_INV;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGLabel::SetDotTemp(char *pData, uint32_t Length)
{
	FreeDotTemp(); // Deallocate any existing space
	if(Length > sizeof(char *)) {
		//Memory needs to be allocated. Allocates an additional byte for a NULL-terminator so it can be copied.
		m_Dot.pTemp = (char*)EG_AllocMem(Length + 1);
		if(m_Dot.pTemp == nullptr) {
			EG_LOG_ERROR("Failed to allocate memory for dot_tmp_ptr");
			return false;
		}
		EG_CopyMem(m_Dot.pTemp, pData, Length);
		m_Dot.pTemp[Length] = '\0';
		m_DotAllocated = true;
	}
	else {
		// Characters can be directly stored in object
		m_DotAllocated = false;
		EG_CopyMem(m_Dot.Temp, pData, Length);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

char* EGLabel::GetDotTemp(void)
{
	if(m_DotAllocated) return m_Dot.pTemp;
	else return m_Dot.Temp;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::FreeDotTemp(void)
{
	if(m_DotAllocated && m_Dot.pTemp) EG_FreeMem(m_Dot.pTemp);
	m_DotAllocated = false;
	m_Dot.pTemp = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetAnimateOffsetX(EGAnimate *pAnimate, int32_t X)
{
	((EGLabel*)pAnimate->m_pItem)->m_Offset.m_X = X;
	((EGLabel*)pAnimate->m_pItem)->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLabel::SetAnimateOffsetY(EGAnimate *pAnimate, int32_t Y)
{
	((EGLabel*)pAnimate->m_pItem)->m_Offset.m_Y = Y;
	((EGLabel*)pAnimate->m_pItem)->Invalidate();
}

#endif
