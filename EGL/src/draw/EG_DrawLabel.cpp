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

#include "draw/EG_DrawLabel.h"
//#include "draw/EG_DrawContext.h"
#include "misc/EG_Math.h"
#include "hal/EG_HALDisplay.h"
#include "core/EG_Refresh.h"
#include "misc/lv_bidi.h"
#include "misc/EG_Assert.h"

//////////////////////////////////////////////////////////////////////////////////////

#define LABEL_RECOLOR_PAR_LENGTH 6
#define EG_LABEL_HINT_UPDATE_TH 1024 // Update the "Hint" if the label's y coordinates have changed more then this

//////////////////////////////////////////////////////////////////////////////////////

enum {
	CMD_STATE_WAIT,
	CMD_STATE_PAR,
	CMD_STATE_IN,
};
typedef uint8_t cmd_state_t;

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t HexToNumber(char hex);

//////////////////////////////////////////////////////////////////////////////////////

EGDrawLabel::EGDrawLabel(void) :
  m_pContext(nullptr),
	m_pFont(EG_FONT_DEFAULT),
	m_SelectStart(EG_DRAW_LABEL_NO_TXT_SEL),
	m_SelectEnd(EG_DRAW_LABEL_NO_TXT_SEL),
	m_Color(EG_ColorBlack()),
	m_SelectColor(EG_ColorBlack()),
	m_SelectBackColor(EG_MainPalette(EG_PALETTE_BLUE)),
  m_LineSpace(0),
  m_Kerning(0),
  m_OffsetX(0),
  m_OffsetY(0),
	m_OPA(EG_OPA_COVER),
  m_BidiDirection(EG_BASE_DIR_LTR),
  m_Align(EG_TEXT_ALIGN_AUTO),
  m_Flag(EG_TEXT_FLAG_NONE),
  m_Decoration(EG_TEXT_DECOR_NONE),
	m_BlendMode(EG_BLEND_MODE_NORMAL)
{
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGDrawLabel::Draw(const EGDrawContext  *pDrawContext, const EGRect *pRect, const char *pText, EG_DrawLabelHint_t *Hint)
{
	EG_LOG_WARN("Label Draw");
	if(m_OPA <= EG_OPA_MIN) return;
	if(m_pFont == nullptr) {
		EG_LOG_WARN("m_pFont == nullptr");
		return;
	}
	if(m_pContext->DrawCharacterProc == nullptr) {
		EG_LOG_WARN("There is no function to draw characters");
		return;
	}
  m_pContext = pDrawContext;
	const EG_Font_t *pFont = m_pFont;
	int32_t Width;
	// No need to waste processor time if string is empty
	if(pText == nullptr || pText[0] == '\0')	return;
	EGRect ClippedArea;
	if(!ClippedArea.Intersect(pRect, pDrawContext->m_pClipRect)) return;
	EG_TextAlignment_t Align = m_Align;
	EG_BaseDirection_e BaseDirection = m_BidiDirection;
	lv_bidi_calculate_align(&Align, &BaseDirection, pText);
	if((m_Flag & EG_TEXT_FLAG_EXPAND) == 0) {
		Width = pRect->GetWidth();		// Normally use the label width as width
	}
	else {
		EGPoint Point;		// If EXPAND is enabled then not limit the text's width to the object's width
		EG_GetTextSize(&Point, pText, m_pFont, m_Kerning, m_LineSpace, EG_COORD_MAX,	m_Flag);
		Width = Point.m_X;
	}
	int32_t FontHeight = EG_FontGetLineHeight(pFont);
	int32_t LineHeight = FontHeight + m_LineSpace;
	// Init variables for the first line
	int32_t LineWidth = 0;
	EGPoint Point;
	Point.m_X = pRect->GetX1();
	Point.m_Y = pRect->GetY1();
	int32_t OffsetX = 0;
	int32_t OffsetY = 0;
	OffsetX = m_OffsetX;
	OffsetY = m_OffsetY;
	Point.m_Y += OffsetY;
	uint32_t LineStart = 0;
	int32_t LastLineStart = -1;
	if(Hint && OffsetY == 0 && pRect->GetY1() < 0) {	// Check the Hint to use the cached info
		if(LV_ABS(Hint->CoordY - pRect->GetY1()) > EG_LABEL_HINT_UPDATE_TH - 2 * LineHeight) {
			Hint->LineStart = -1;		// If the label changed too much recalculate the Hint.
		}
		LastLineStart = Hint->LineStart;
	}
	if(Hint && LastLineStart >= 0) { // Use the Hint if it's valid
		LineStart = LastLineStart;
		Point.m_Y += Hint->Y;
	}
	uint32_t LineEnd = LineStart + EG_GetNextTextLine(&pText[LineStart], pFont, m_Kerning, Width, nullptr, m_Flag);
	while(Point.m_Y + FontHeight < pDrawContext->m_pClipRect->GetY1()) {	// Goto the first visible line
		LineStart = LineEnd;		// Go to next line
		LineEnd += EG_GetNextTextLine(&pText[LineStart], pFont, m_Kerning, Width, nullptr, m_Flag);
		Point.m_Y += LineHeight;
		if(Hint && Point.m_Y >= -EG_LABEL_HINT_UPDATE_TH && Hint->LineStart < 0) {		// Save at the threshold coordinate
			Hint->LineStart = LineStart;
			Hint->Y = Point.m_Y - pRect->GetY1();
			Hint->CoordY = pRect->GetY1();
		}
		if(pText[LineStart] == '\0') return;
	}
	if(Align == EG_TEXT_ALIGN_CENTER) {	// Align to middle
		LineWidth = EG_GetTextWidth(&pText[LineStart], LineEnd - LineStart, pFont, m_Kerning, m_Flag);
		Point.m_X += (pRect->GetWidth() - LineWidth) / 2;
	}
	else if(Align == EG_TEXT_ALIGN_RIGHT) {	// Align to the right
		LineWidth = EG_GetTextWidth(&pText[LineStart], LineEnd - LineStart, pFont, m_Kerning, m_Flag);
		Point.m_X += pRect->GetWidth() - LineWidth;
	}
	uint32_t SeletStart = m_SelectStart;
	uint32_t SeletEnd = m_SelectEnd;
	if(SeletStart > SeletEnd) {
		uint32_t tmp = SeletStart;
		SeletStart = SeletEnd;
		SeletEnd = tmp;
	}
	EGDrawLine DrawLine;
	if((m_Decoration & EG_TEXT_DECOR_UNDERLINE) || (m_Decoration & EG_TEXT_DECOR_STRIKETHROUGH)) {
		DrawLine.m_Color = m_Color;
		DrawLine.m_Width = pFont->UnderlineThickness ? pFont->UnderlineThickness : 1;
		DrawLine.m_OPA = m_OPA;
		DrawLine.m_BlendMode = m_BlendMode;
	}
	cmd_state_t cmd_state = CMD_STATE_WAIT;
	uint32_t par_start = 0;
	EG_Color_t recolor = EG_ColorBlack();
	EG_Color_t Color = EG_ColorBlack();
	EGDrawRect SelectRect;
	SelectRect.m_BackgroundColor = m_SelectBackColor;
	int32_t StartPosX = Point.m_X;
	while(pText[LineStart] != '\0') {	// Write out all lines
		Point.m_X += OffsetX;
		cmd_state = CMD_STATE_WAIT;		// Write all Chr of a line
		uint32_t i = 0;
#if EG_USE_BIDI
		char *bidi_txt = EG_GetBufferMem(LineEnd - LineStart + 1);
		_lv_bidi_process_paragraph(pText + LineStart, bidi_txt, LineEnd - LineStart, BaseDirection, nullptr, 0);
#else
		const char *bidi_txt = pText + LineStart;
#endif
		while(i < LineEnd - LineStart) {
			uint32_t logical_char_pos = 0;
			if(SeletStart != 0xFFFF && SeletEnd != 0xFFFF) {
#if EG_USE_BIDI
				logical_char_pos = EG_TextEncodedGetPosition(pText, LineStart);
				uint32_t t = EG_TextEncodedGetPosition(bidi_txt, i);
				logical_char_pos += _lv_bidi_get_logical_pos(bidi_txt, nullptr, LineEnd - LineStart, BaseDirection, t, nullptr);
#else
				logical_char_pos = EG_TextEncodedGetPosition(pText, LineStart + i);
#endif
			}
			uint32_t Chr;
			uint32_t NextChr;
			EG_TextDecode2(bidi_txt, &Chr, &NextChr, &i);
			if((m_Flag & EG_TEXT_FLAG_RECOLOR) != 0) {			// Handle the re-Color command
				if(Chr == (uint32_t)EG_TXT_COLOR_CMD[0]) {
					if(cmd_state == CMD_STATE_WAIT) { // Start char
						par_start = i;
						cmd_state = CMD_STATE_PAR;
						continue;
					}
					else if(cmd_state == CMD_STATE_PAR) { // Other start char in parameter escaped cmd. char
						cmd_state = CMD_STATE_WAIT;
					}
					else if(cmd_state == CMD_STATE_IN) { // Command end
						cmd_state = CMD_STATE_WAIT;
						continue;
					}
				}
				if(cmd_state == CMD_STATE_PAR) {	// Skip the Color parameter and wait the space after it
					if(Chr == ' ') {
						if(i - par_start == LABEL_RECOLOR_PAR_LENGTH + 1) {						// Get the parameter
							char buf[LABEL_RECOLOR_PAR_LENGTH + 1];
							EG_CopyMemSmall(buf, &bidi_txt[par_start], LABEL_RECOLOR_PAR_LENGTH);
							buf[LABEL_RECOLOR_PAR_LENGTH] = '\0';
							int r, g, b;
							r = (HexToNumber(buf[0]) << 4) + HexToNumber(buf[1]);
							g = (HexToNumber(buf[2]) << 4) + HexToNumber(buf[3]);
							b = (HexToNumber(buf[4]) << 4) + HexToNumber(buf[5]);
							recolor = EG_MixColor(r, g, b);
						}
						else {
							recolor.full = m_Color.full;
						}
						cmd_state = CMD_STATE_IN; // After the parameter the text is in the command
					}
					continue;
				}
			}
			Color = m_Color;
			if(cmd_state == CMD_STATE_IN) Color = recolor;
			int32_t CharWidth = EG_FontGetGlyphWidth(pFont, Chr, NextChr);
			if(SeletStart != 0xFFFF && SeletEnd != 0xFFFF) {
				if(logical_char_pos >= SeletStart && logical_char_pos < SeletEnd) {
					EGRect SelectArea(Point.m_X, Point.m_Y, Point.m_X + CharWidth + m_Kerning - 1, Point.m_Y + LineHeight - 1);
					SelectRect.Draw(pDrawContext, &SelectArea);
					Color = m_SelectColor;
				}
			}
			m_Color = Color;
			DrawChar(pDrawContext, &Point, Chr);
			if(CharWidth > 0) {
				Point.m_X += CharWidth + m_Kerning;
			}
		}
		if(m_Decoration & EG_TEXT_DECOR_STRIKETHROUGH) {
			EGPoint Point1, Point2;
			Point1.m_X = StartPosX;
			Point1.m_Y = Point.m_Y + (m_pFont->LineHeight / 2) + DrawLine.m_Width / 2;
			Point2.m_X = Point.m_X;
			Point2.m_Y = Point1.m_Y;
			DrawLine.m_Color = Color;
			DrawLine.Draw(pDrawContext, &Point1, &Point2);
		}
		if(m_Decoration & EG_TEXT_DECOR_UNDERLINE) {
			EGPoint Point1;
			EGPoint Point2;
			Point1.m_X = StartPosX;
			Point1.m_Y = Point.m_Y + m_pFont->LineHeight - m_pFont->BaseLine - pFont->UnderlinePosition;
			Point2.m_X = Point.m_X;
			Point2.m_Y = Point1.m_Y;
			DrawLine.m_Color = Color;
			DrawLine.Draw(pDrawContext, &Point1, &Point2);
		}
#if EG_USE_BIDI
		EG_ReleaseBufferMem(bidi_txt);
		bidi_txt = nullptr;
#endif
		LineStart = LineEnd;		// Go to next line
		LineEnd += EG_GetNextTextLine(&pText[LineStart], pFont, m_Kerning, Width, nullptr, m_Flag);
		Point.m_X = pRect->GetX1();
		if(Align == EG_TEXT_ALIGN_CENTER) {		// Align to middle
			LineWidth = EG_GetTextWidth(&pText[LineStart], LineEnd - LineStart, pFont, m_Kerning, m_Flag);
			Point.m_X += (pRect->GetWidth() - LineWidth) / 2;
		}
		else if(Align == EG_TEXT_ALIGN_RIGHT) {		// Align to the right
			LineWidth = EG_GetTextWidth(&pText[LineStart], LineEnd - LineStart, pFont, m_Kerning, m_Flag);
			Point.m_X += pRect->GetWidth() - LineWidth;
		}
		Point.m_Y += LineHeight;		// Go the next line position
		if(Point.m_Y > pDrawContext->m_pClipRect->GetY2()) return;
	}
	EG_ASSERT_MEM_INTEGRITY();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawLabel::DrawChar(const EGDrawContext  *pDrawContext, const EGPoint *pPos, uint32_t Char)
{
  m_pContext = pDrawContext;
	pDrawContext->DrawCharacterProc(this, pPos, Char);
}

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t HexToNumber(char hex)
{
uint8_t result = 0;

	if(hex >= '0' && hex <= '9') {
		result = hex - '0';
	}
	else {
		if(hex >= 'a') hex -= 'a' - 'A'; // Convert to upper case
		switch(hex) {
			case 'A':
				result = 10;
				break;
			case 'B':
				result = 11;
				break;
			case 'C':
				result = 12;
				break;
			case 'D':
				result = 13;
				break;
			case 'E':
				result = 14;
				break;
			case 'F':
				result = 15;
				break;
			default:
				result = 0;
				break;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////

void EGDrawLabel::Reset(void)
{
	m_OPA = EG_OPA_COVER;
	m_Color = EG_ColorBlack();
	m_pFont = EG_FONT_DEFAULT;
	m_SelectStart = EG_DRAW_LABEL_NO_TXT_SEL;
	m_SelectEnd = EG_DRAW_LABEL_NO_TXT_SEL;
	m_SelectColor = EG_ColorBlack();
	m_SelectBackColor = EG_MainPalette(EG_PALETTE_BLUE);
	m_BidiDirection = EG_BASE_DIR_LTR;
}

//////////////////////////////////////////////////////////////////////////////////

void EGDrawLabel::operator=(const EGDrawLabel &rval)
{
  m_pFont           = rval.m_pFont;
  m_SelectStart     = rval.m_SelectStart;
  m_SelectEnd       = rval.m_SelectEnd;
  m_Color           = rval.m_Color;
  m_SelectColor     = rval.m_SelectColor;
  m_SelectBackColor = rval.m_SelectBackColor;
  m_LineSpace       = rval.m_LineSpace;
  m_Kerning         = rval.m_Kerning;
  m_OffsetX         = rval.m_OffsetX;
  m_OffsetY         = rval.m_OffsetY;
  m_OPA             = rval.m_OPA;
  m_BidiDirection   = rval.m_BidiDirection;
  m_Align           = rval.m_Align;
  m_Flag            = rval.m_Flag;
  m_Decoration      = rval.m_Decoration;
  m_BlendMode       = rval.m_BlendMode;
}

