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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */


#include <stdarg.h>
#include "misc/EG_Text.h"
#include "misc/lv_txt_ap.h"
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Assert.h"

/////////////////////////////////////////////////////////////////////////////

#define NO_BREAK_FOUND UINT32_MAX

#if EG_TXT_ENC == EG_TXT_ENC_UTF8
static uint8_t lv_txt_utf8_size(const char *str);
static uint32_t lv_txt_unicode_to_utf8(uint32_t letter_uni);
static uint32_t lv_txt_utf8_conv_wc(uint32_t c);
static uint32_t lv_txt_utf8_next(const char *txt, uint32_t *i);
static uint32_t lv_txt_utf8_prev(const char *txt, uint32_t *i_start);
static uint32_t lv_txt_utf8_get_byte_id(const char *txt, uint32_t utf8_id);
static uint32_t lv_txt_utf8_get_char_id(const char *txt, uint32_t byte_id);
static uint32_t lv_txt_utf8_get_length(const char *txt);
#elif EG_TXT_ENC == EG_TXT_ENC_ASCII
static uint8_t lv_txt_iso8859_1_size(const char *str);
static uint32_t lv_txt_unicode_to_iso8859_1(uint32_t letter_uni);
static uint32_t lv_txt_iso8859_1_conv_wc(uint32_t c);
static uint32_t lv_txt_iso8859_1_next(const char *txt, uint32_t *i);
static uint32_t lv_txt_iso8859_1_prev(const char *txt, uint32_t *i_start);
static uint32_t lv_txt_iso8859_1_get_byte_id(const char *txt, uint32_t utf8_id);
static uint32_t lv_txt_iso8859_1_get_char_id(const char *txt, uint32_t byte_id);
static uint32_t lv_txt_iso8859_1_get_length(const char *txt);
#endif

#if EG_TXT_ENC == EG_TXT_ENC_UTF8
uint8_t (*EG_TextEncodedSize)(const char *) = lv_txt_utf8_size;
uint32_t (*EG_TextUnicodeToEncoded)(uint32_t) = lv_txt_unicode_to_utf8;
uint32_t (*EG_TextEncodedToWide)(uint32_t) = lv_txt_utf8_conv_wc;
uint32_t (*EG_TextDecodeNext)(const char *, uint32_t *) = lv_txt_utf8_next;
uint32_t (*EG_TextDecodePrevious)(const char *, uint32_t *) = lv_txt_utf8_prev;
uint32_t (*EG_TextEncodedGetIndex)(const char *, uint32_t) = lv_txt_utf8_get_byte_id;
uint32_t (*EG_TextEncodedGetPosition)(const char *, uint32_t) = lv_txt_utf8_get_char_id;
uint32_t (*EG_TextEncodedGetLength)(const char *) = lv_txt_utf8_get_length;
#elif EG_TXT_ENC == EG_TXT_ENC_ASCII
uint8_t (*EG_TextEncodedSize)(const char *) = lv_txt_iso8859_1_size;
uint32_t (*EG_TextUnicodeToEncoded)(uint32_t) = lv_txt_unicode_to_iso8859_1;
uint32_t (*EG_TextEncodedToWide)(uint32_t) = lv_txt_iso8859_1_conv_wc;
uint32_t (*EG_TextDecodeNext)(const char *, uint32_t *) = lv_txt_iso8859_1_next;
uint32_t (*EG_TextDecodePrevious)(const char *, uint32_t *) = lv_txt_iso8859_1_prev;
uint32_t (*EG_TextEncodedGetIndex)(const char *, uint32_t) = lv_txt_iso8859_1_get_byte_id;
uint32_t (*EG_TextEncodedGetPosition)(const char *, uint32_t) = lv_txt_iso8859_1_get_char_id;
uint32_t (*EG_TextEncodedGetLength)(const char *) = lv_txt_iso8859_1_get_length;

#endif

#define LV_IS_ASCII(value) ((value & 0x80U) == 0x00U)
#define LV_IS_2BYTES_UTF8_CODE(value) ((value & 0xE0U) == 0xC0U)
#define LV_IS_3BYTES_UTF8_CODE(value) ((value & 0xF0U) == 0xE0U)
#define LV_IS_4BYTES_UTF8_CODE(value) ((value & 0xF8U) == 0xF0U)
#define LV_IS_INVALID_UTF8_CODE(value) ((value & 0xC0U) != 0x80U)

/////////////////////////////////////////////////////////////////////////////

void EG_GetTextSize(EGPoint *pSize, const char *pText, const EG_Font_t *pFont, EG_Coord_t Kerning,
										 EG_Coord_t LineSpace, EG_Coord_t MaxWidth, EG_TextFlag_t Flag)
{
	if((pText == nullptr) || (pFont == nullptr)) return;
	pSize->m_X = 0;
	pSize->m_Y = 0;
	if(Flag & EG_TEXT_FLAG_EXPAND) MaxWidth = EG_COORD_MAX;
	uint32_t LineStart = 0;
	uint32_t NewLineStart = 0;
	uint16_t LineHeight = EG_FontGetLineHeight(pFont);
	while(pText[LineStart] != '\0') {	// Calc. the height and longest line
		NewLineStart += EG_GetNextTextLine(&pText[LineStart], pFont, Kerning, MaxWidth, NULL, Flag);
		if((unsigned long)pSize->m_Y + (unsigned long)LineHeight + (unsigned long)LineSpace > EG_MAX_OF(EG_Coord_t)) {
			EG_LOG_WARN("EG_GetTextSize: integer overflow while calculating pText height");
			return;
		}
		else {
			pSize->m_Y += LineHeight;
			pSize->m_Y += LineSpace;
		}
		// Calculate the longest line
		EG_Coord_t act_line_length = EG_GetTextWidth(&pText[LineStart], NewLineStart - LineStart, pFont, Kerning,	Flag);
		pSize->m_X = EG_MAX(act_line_length, pSize->m_X);
		LineStart = NewLineStart;
	}
	// Make the text one line taller if the last character is '\n' or '\r'
	if((LineStart != 0) && (pText[LineStart - 1] == '\n' || pText[LineStart - 1] == '\r')) {
		pSize->m_Y += LineHeight + LineSpace;
	}
	// Correction with the last line space or set the height manually if the text is empty
	if(pSize->m_Y == 0)	pSize->m_Y = LineHeight;
	else pSize->m_Y -= LineSpace;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Get the next word of text. A word is delimited by break characters.
 *
 * If the word cannot fit in the MaxWidth space, obey EG_TXT_LINE_BREAK_LONG_* rules.
 *
 * If the next word cannot fit anything, return 0.
 *
 * If the first character is a break character, returns the next index.
 *
 * Example calls from lv_txt_get_next_line() assuming sufficient MaxWidth and
 * txt = "Test text\n"
 *        0123456789
 *
 * Calls would be as follows:
 *     1. Return i=4, pointing at breakchar ' ', for the string "Test"
 *     2. Return i=5, since i=4 was a breakchar.
 *     3. Return i=9, pointing at breakchar '\n'
 *     4. Parenting lv_txt_get_next_line() would detect subsequent '\0'
 *
 * TODO: Returned word_w_ptr may overestimate the returned word's width when
 * MaxWidth is reached. In current usage, this has no impact.
 *
 * @param txt a '\0' terminated string
 * @param pFont pointer to a font
 * @param Kerning letter space
 * @param MaxWidth max width of the text (break the lines to fit this size). Set COORD_MAX to avoid line breaks
 * @param flags settings for the text from 'txt_flag_type' enum
 * @param[out] word_w_ptr width (in pixels) of the parsed word. May be NULL.
 * @param cmd_state pointer to a txt_cmd_state_t variable which stores the current state of command processing
 * @param force Force return the fraction of the word that can fit in the provided space.
 * @return the index of the first char of the next word (in byte index not letter index. With UTF-8 they are different)
 */
static uint32_t lv_txt_get_next_word(const char *txt, const EG_Font_t *pFont,
																		 EG_Coord_t Kerning, EG_Coord_t MaxWidth,
																		 EG_TextFlag_t Flag, uint32_t *word_w_ptr, EG_TextCommandState_t *cmd_state, bool force)
{
	if(txt == NULL || txt[0] == '\0') return 0;
	if(pFont == NULL) return 0;

	if(Flag & EG_TEXT_FLAG_EXPAND) MaxWidth = EG_COORD_MAX;

	uint32_t i = 0, i_next = 0, i_next_next = 0; /*Iterating index into txt*/
	uint32_t letter = 0;                         /*Letter at i*/
	uint32_t letter_next = 0;                    /*Letter at i_next*/
	EG_Coord_t letter_w;
	EG_Coord_t cur_w = 0;                  /*Pixel Width of transversed string*/
	uint32_t word_len = 0;                 /*Number of characters in the transversed word*/
	uint32_t break_index = NO_BREAK_FOUND; /*only used for "long" words*/
	uint32_t break_letter_count = 0;       /*Number of characters up to the long word break point*/

	letter = EG_TextDecodeNext(txt, &i_next);
	i_next_next = i_next;

	/*Obtain the full word, regardless if it fits or not in MaxWidth*/
	while(txt[i] != '\0') {
		letter_next = EG_TextDecodeNext(txt, &i_next_next);
		word_len++;

		/*Handle the recolor command*/
		if((Flag & EG_TEXT_FLAG_RECOLOR) != 0) {
			if(EG_TextIsCommand(cmd_state, letter) != false) {
				i = i_next;
				i_next = i_next_next;
				letter = letter_next;
				continue; /*Skip the letter if it is part of a command*/
			}
		}

		letter_w = EG_FontGetGlyphWidth(pFont, letter, letter_next);
		cur_w += letter_w;

		if(letter_w > 0) {
			cur_w += Kerning;
		}

		/*Test if this character fits within MaxWidth*/
		if(break_index == NO_BREAK_FOUND && (cur_w - Kerning) > MaxWidth) {
			break_index = i;
			break_letter_count = word_len - 1;
			/*break_index is now pointing at the character that doesn't fit*/
		}

		/*Check for new line chars and breakchars*/
		if(letter == '\n' || letter == '\r' || EG_TextIsBreak(letter)) {
			/*Update the output width on the first character if it fits.
             *Must do this here in case first letter is a break character.*/
			if(i == 0 && break_index == NO_BREAK_FOUND && word_w_ptr != NULL) *word_w_ptr = cur_w;
			word_len--;
			break;
		}

		/*Update the output width*/
		if(word_w_ptr != NULL && break_index == NO_BREAK_FOUND) *word_w_ptr = cur_w;

		i = i_next;
		i_next = i_next_next;
		letter = letter_next;
	}

	/*Entire Word fits in the provided space*/
	if(break_index == NO_BREAK_FOUND) {
		if(word_len == 0 || (letter == '\r' && letter_next == '\n')) i = i_next;
		return i;
	}

#if EG_TXT_LINE_BREAK_LONG_LEN > 0
	/*Word doesn't fit in provided space, but isn't "long"*/
	if(word_len < EG_TXT_LINE_BREAK_LONG_LEN) {
		if(force) return break_index;
		if(word_w_ptr != NULL) *word_w_ptr = 0; /*Return no word*/
		return 0;
	}

	/*Word is "long," but insufficient amounts can fit in provided space*/
	if(break_letter_count < EG_TXT_LINE_BREAK_LONG_PRE_MIN_LEN) {
		if(force) return break_index;
		if(word_w_ptr != NULL) *word_w_ptr = 0;
		return 0;
	}

	/*Word is a "long", but letters may need to be better distributed*/
	{
		i = break_index;
		int32_t n_move = EG_TXT_LINE_BREAK_LONG_POST_MIN_LEN - (word_len - break_letter_count);
		/*Move pointer "i" backwards*/
		for(; n_move > 0; n_move--) {
			EG_TextDecodePrevious(txt, &i);
			// TODO: it would be appropriate to update the returned word width here
			// However, in current usage, this doesn't impact anything.
		}
	}
	return i;
#else
	if(force) return break_index;
	if(word_w_ptr != NULL) *word_w_ptr = 0; /*Return no word*/
	(void)break_letter_count;
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EG_GetNextTextLine(const char *txt, const EG_Font_t *pFont,
															 EG_Coord_t Kerning, EG_Coord_t MaxWidth,
															 EG_Coord_t *used_width, EG_TextFlag_t Flag)
{
	if(used_width) *used_width = 0;

	if(txt == NULL) return 0;
	if(txt[0] == '\0') return 0;
	if(pFont == NULL) return 0;

	EG_Coord_t line_w = 0;

	/*If MaxWidth doesn't mater simply find the new line character
     *without thinking about word wrapping*/
	if((Flag & EG_TEXT_FLAG_EXPAND) || (Flag & EG_TEXT_FLAG_FIT)) {
		uint32_t i;
		for(i = 0; txt[i] != '\n' && txt[i] != '\r' && txt[i] != '\0'; i++) {
			/*Just find the new line chars or string ends by incrementing `i`*/
		}
		if(txt[i] != '\0') i++; /*To go beyond `\n`*/
		if(used_width) *used_width = -1;
		return i;
	}

	if(Flag & EG_TEXT_FLAG_EXPAND) MaxWidth = EG_COORD_MAX;
	EG_TextCommandState_t cmd_state = EG_TEXT_CMD_STATE_WAIT;
	uint32_t i = 0; /*Iterating index into txt*/

	while(txt[i] != '\0' && MaxWidth > 0) {
		uint32_t word_w = 0;
		uint32_t advance = lv_txt_get_next_word(&txt[i], pFont, Kerning, MaxWidth, Flag, &word_w, &cmd_state, i == 0);
		MaxWidth -= word_w;
		line_w += word_w;

		if(advance == 0) {
			break;
		}

		i += advance;

		if(txt[0] == '\n' || txt[0] == '\r') break;

		if(txt[i] == '\n' || txt[i] == '\r') {
			i++; /*Include the following newline in the current line*/
			break;
		}
	}

	/*Always step at least one to avoid infinite loops*/
	if(i == 0) {
		uint32_t letter = EG_TextDecodeNext(txt, &i);
		if(used_width != NULL) {
			line_w = EG_FontGetGlyphWidth(pFont, letter, '\0');
		}
	}

	if(used_width != NULL) {
		*used_width = line_w;
	}

	return i;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EG_GetTextWidth(const char *pText, uint32_t Length, const EG_Font_t *pFont, EG_Coord_t Kerning,
														EG_TextFlag_t Flag)
{
uint32_t i = 0;
EG_Coord_t Width = 0;
EG_TextCommandState_t cmd_state = EG_TEXT_CMD_STATE_WAIT;

	if((pText == nullptr) || (pFont == nullptr) || (pText[0] == '\0')) return 0;
	if(Length != 0) {
		while(i < Length) {
			uint32_t Char;
			uint32_t NextChar;
			EG_TextDecode2(pText, &Char, &NextChar, &i);
			if((Flag & EG_TEXT_FLAG_RECOLOR) != 0) {
				if(EG_TextIsCommand(&cmd_state, Char) != false) continue;
			}
			EG_Coord_t char_width = EG_FontGetGlyphWidth(pFont, Char, NextChar);
			if(char_width > 0) {
				Width += char_width;
				Width += Kerning;
			}
		}
		if(Width > 0) {
			Width -= Kerning; // Trim the last character space. Important if the text is center aligned
		}
	}
	return Width;
}

/////////////////////////////////////////////////////////////////////////////

bool EG_TextIsCommand(EG_TextCommandState_t *state, uint32_t c)
{
	bool ret = false;

	if(c == (uint32_t)EG_TXT_COLOR_CMD[0]) {
		if(*state == EG_TEXT_CMD_STATE_WAIT) { /*Start char*/
			*state = EG_TEXT_CMD_STATE_PAR;
			ret = true;
		}
		/*Other start char in parameter is escaped cmd. char*/
		else if(*state == EG_TEXT_CMD_STATE_PAR) {
			*state = EG_TEXT_CMD_STATE_WAIT;
		}
		/*Command end*/
		else if(*state == EG_TEXT_CMD_STATE_IN) {
			*state = EG_TEXT_CMD_STATE_WAIT;
			ret = true;
		}
	}

	/*Skip the color parameter and wait the space after it*/
	if(*state == EG_TEXT_CMD_STATE_PAR) {
		if(c == ' ') {
			*state = EG_TEXT_CMD_STATE_IN; /*After the parameter the text is in the command*/
		}
		ret = true;
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////

void EG_TextInsert(char *txt_buf, uint32_t pos, const char *ins_txt)
{
	if(txt_buf == NULL || ins_txt == NULL) return;

	size_t old_len = strlen(txt_buf);
	size_t ins_len = strlen(ins_txt);
	if(ins_len == 0) return;

	size_t new_len = ins_len + old_len;
	pos = EG_TextEncodedGetIndex(txt_buf, pos); /*Convert to byte index instead of letter index*/

	/*Copy the second part into the end to make place to text to insert*/
	size_t i;
	for(i = new_len; i >= pos + ins_len; i--) {
		txt_buf[i] = txt_buf[i - ins_len];
	}

	/*Copy the text into the new space*/
	EG_CopyMemSmall(txt_buf + pos, ins_txt, ins_len);
}

void EG_TextCut(char *txt, uint32_t pos, uint32_t len)
{
	if(txt == NULL) return;

	size_t old_len = strlen(txt);

	pos = EG_TextEncodedGetIndex(txt, pos); /*Convert to byte index instead of letter index*/
	len = EG_TextEncodedGetIndex(&txt[pos], len);

	/*Copy the second part into the end to make place to text to insert*/
	uint32_t i;
	for(i = pos; i <= old_len - len; i++) {
		txt[i] = txt[i + len];
	}
}

/////////////////////////////////////////////////////////////////////////////

char *EG_TextFormat(const char *fmt, va_list ap)
{
	/*Allocate space for the new text by using trick from C99 standard section 7.19.6.12*/
	va_list ap_copy;
	va_copy(ap_copy, ap);
	uint32_t len = eg_vsnprintf(NULL, 0, fmt, ap_copy);
	va_end(ap_copy);

	char *text = 0;
#if EG_USE_ARABIC_PERSIAN_CHARS
	/*Put together the text according to the format string*/
	char *raw_txt = EG_GetBufferMem(len + 1);
	EG_ASSERT_MALLOC(raw_txt);
	if(raw_txt == NULL) {
		return NULL;
	}

	eg_vsnprintf(raw_txt, len + 1, fmt, ap);

	/*Get the size of the Arabic text and process it*/
	size_t len_ap = _lv_txt_ap_calc_bytes_cnt(raw_txt);
	text = EG_AllocMem(len_ap + 1);
	EG_ASSERT_MALLOC(text);
	if(text == NULL) {
		return NULL;
	}
	_lv_txt_ap_proc(raw_txt, text);

	EG_ReleaseBufferMem(raw_txt);
#else
	text = (char*)EG_AllocMem(len + 1);
	EG_ASSERT_MALLOC(text);
	if(text == NULL) {
		return NULL;
	}
	text[len] = 0; /*Ensure NULL termination*/

	eg_vsnprintf(text, len + 1, fmt, ap);
#endif

	return text;
}

/////////////////////////////////////////////////////////////////////////////

void EG_TextDecode2(const char *txt, uint32_t *letter, uint32_t *letter_next, uint32_t *ofs)
{
	*letter = EG_TextDecodeNext(txt, ofs);
	*letter_next = *letter != '\0' ? EG_TextDecodeNext(&txt[*ofs], NULL) : 0;
}

#if EG_TXT_ENC == EG_TXT_ENC_UTF8

/////////////////////////////////////////////////////////////////////////////

/**
 * Give the size of an UTF-8 coded character
 * @param str pointer to a character in a string
 * @return length of the UTF-8 character (1,2,3 or 4), 0 on invalid code.
 */
static uint8_t lv_txt_utf8_size(const char *str)
{
	if(LV_IS_ASCII(str[0]))	return 1;
	else if(LV_IS_2BYTES_UTF8_CODE(str[0]))	return 2;
	else if(LV_IS_3BYTES_UTF8_CODE(str[0]))	return 3;
	else if(LV_IS_4BYTES_UTF8_CODE(str[0]))	return 4;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a Unicode letter to UTF-8.
 * @param letter_uni a Unicode letter
 * @return UTF-8 coded character in Little Endian to be compatible with C chars (e.g. 'Á', 'Ű')
 */
static uint32_t lv_txt_unicode_to_utf8(uint32_t letter_uni)
{
	if(letter_uni < 128) return letter_uni;
	uint8_t bytes[4];

	if(letter_uni < 0x0800) {
		bytes[0] = ((letter_uni >> 6) & 0x1F) | 0xC0;
		bytes[1] = ((letter_uni >> 0) & 0x3F) | 0x80;
		bytes[2] = 0;
		bytes[3] = 0;
	}
	else if(letter_uni < 0x010000) {
		bytes[0] = ((letter_uni >> 12) & 0x0F) | 0xE0;
		bytes[1] = ((letter_uni >> 6) & 0x3F) | 0x80;
		bytes[2] = ((letter_uni >> 0) & 0x3F) | 0x80;
		bytes[3] = 0;
	}
	else if(letter_uni < 0x110000) {
		bytes[0] = ((letter_uni >> 18) & 0x07) | 0xF0;
		bytes[1] = ((letter_uni >> 12) & 0x3F) | 0x80;
		bytes[2] = ((letter_uni >> 6) & 0x3F) | 0x80;
		bytes[3] = ((letter_uni >> 0) & 0x3F) | 0x80;
	}
	else {
		return 0;
	}
	uint32_t *res_p = (uint32_t *)bytes;
	return *res_p;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a wide character, e.g. 'Á' little endian to be UTF-8 compatible
 * @param c a wide character or a  Little endian number
 * @return `c` in big endian
 */
static uint32_t lv_txt_utf8_conv_wc(uint32_t c)
{
#if EG_BIG_ENDIAN_SYSTEM == 0
	/*Swap the bytes (UTF-8 is big endian, but the MCUs are little endian)*/
	if((c & 0x80) != 0) {
		uint32_t swapped;
		uint8_t c8[4];
		EG_CopyMemSmall(c8, &c, 4);
		swapped = (c8[0] << 24) + (c8[1] << 16) + (c8[2] << 8) + (c8[3]);
		uint8_t i;
		for(i = 0; i < 4; i++) {
			if((swapped & 0xFF) == 0)
				swapped = (swapped >> 8); /*Ignore leading zeros (they were in the end originally)*/
		}
		c = swapped;
	}
#endif
	return c;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Decode an UTF-8 character from a string.
 * @param txt pointer to '\0' terminated string
 * @param i start byte index in 'txt' where to start.
 *          After call it will point to the next UTF-8 char in 'txt'.
 *          NULL to use txt[0] as index
 * @return the decoded Unicode character or 0 on invalid UTF-8 code
 */
static uint32_t lv_txt_utf8_next(const char *txt, uint32_t *i)
{
	/**
     * Unicode to UTF-8
     * 00000000 00000000 00000000 0xxxxxxx -> 0xxxxxxx
     * 00000000 00000000 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
     * 00000000 00000000 zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx
     * 00000000 000wwwzz zzzzyyyy yyxxxxxx -> 11110www 10zzzzzz 10yyyyyy 10xxxxxx
     */

	uint32_t result = 0;

	/*Dummy 'i' pointer is required*/
	uint32_t i_tmp = 0;
	if(i == NULL) i = &i_tmp;

	/*Normal ASCII*/
	if(LV_IS_ASCII(txt[*i])) {
		result = txt[*i];
		(*i)++;
	}
	/*Real UTF-8 decode*/
	else {
		/*2 bytes UTF-8 code*/
		if(LV_IS_2BYTES_UTF8_CODE(txt[*i])) {
			result = (uint32_t)(txt[*i] & 0x1F) << 6;
			(*i)++;
			if(LV_IS_INVALID_UTF8_CODE(txt[*i])) return 0;
			result += (txt[*i] & 0x3F);
			(*i)++;
		}
		/*3 bytes UTF-8 code*/
		else if(LV_IS_3BYTES_UTF8_CODE(txt[*i])) {
			result = (uint32_t)(txt[*i] & 0x0F) << 12;
			(*i)++;

			if(LV_IS_INVALID_UTF8_CODE(txt[*i])) return 0;
			result += (uint32_t)(txt[*i] & 0x3F) << 6;
			(*i)++;

			if(LV_IS_INVALID_UTF8_CODE(txt[*i])) return 0;
			result += (txt[*i] & 0x3F);
			(*i)++;
		}
		/*4 bytes UTF-8 code*/
		else if(LV_IS_4BYTES_UTF8_CODE(txt[*i])) {
			result = (uint32_t)(txt[*i] & 0x07) << 18;
			(*i)++;

			if(LV_IS_INVALID_UTF8_CODE(txt[*i])) return 0;
			result += (uint32_t)(txt[*i] & 0x3F) << 12;
			(*i)++;

			if(LV_IS_INVALID_UTF8_CODE(txt[*i])) return 0;
			result += (uint32_t)(txt[*i] & 0x3F) << 6;
			(*i)++;

			if(LV_IS_INVALID_UTF8_CODE(txt[*i])) return 0;
			result += txt[*i] & 0x3F;
			(*i)++;
		}
		else {
			(*i)++; /*Not UTF-8 char. Go the next.*/
		}
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Get previous UTF-8 character form a string.
 * @param txt pointer to '\0' terminated string
 * @param i start byte index in 'txt' where to start. After the call it will point to the previous
 * UTF-8 char in 'txt'.
 * @return the decoded Unicode character or 0 on invalid UTF-8 code
 */
static uint32_t lv_txt_utf8_prev(const char *txt, uint32_t *i)
{
	uint8_t c_size;
	uint8_t cnt = 0;

	/*Try to find a !0 long UTF-8 char by stepping one character back*/
	(*i)--;
	do {
		if(cnt >= 4) return 0; /*No UTF-8 char found before the initial*/

		c_size = EG_TextEncodedSize(&txt[*i]);
		if(c_size == 0) {
			if(*i != 0)
				(*i)--;
			else
				return 0;
		}
		cnt++;
	} while(c_size == 0);

	uint32_t i_tmp = *i;
	uint32_t letter = EG_TextDecodeNext(txt, &i_tmp); /*Character found, get it*/

	return letter;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a character index (in an UTF-8 text) to byte index.
 * E.g. in "AÁRT" index of 'R' is 2th char but start at byte 3 because 'Á' is 2 bytes long
 * @param txt a '\0' terminated UTF-8 string
 * @param utf8_id character index
 * @return byte index of the 'utf8_id'th letter
 */
static uint32_t lv_txt_utf8_get_byte_id(const char *txt, uint32_t utf8_id)
{
	uint32_t i;
	uint32_t byte_cnt = 0;
	for(i = 0; i < utf8_id && txt[byte_cnt] != '\0'; i++) {
		uint8_t c_size = EG_TextEncodedSize(&txt[byte_cnt]);
		/* If the char was invalid tell it's 1 byte long*/
		byte_cnt += c_size ? c_size : 1;
	}

	return byte_cnt;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a byte index (in an UTF-8 text) to character index.
 * E.g. in "AÁRT" index of 'R' is 2th char but start at byte 3 because 'Á' is 2 bytes long
 * @param txt a '\0' terminated UTF-8 string
 * @param byte_id byte index
 * @return character index of the letter at 'byte_id'th position
 */
static uint32_t lv_txt_utf8_get_char_id(const char *txt, uint32_t byte_id)
{
	uint32_t i = 0;
	uint32_t char_cnt = 0;

	while(i < byte_id) {
		EG_TextDecodeNext(txt, &i); /*'i' points to the next letter so use the prev. value*/
		char_cnt++;
	}

	return char_cnt;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Get the number of characters (and NOT bytes) in a string. Decode it with UTF-8 if enabled.
 * E.g.: "ÁBC" is 3 characters (but 4 bytes)
 * @param txt a '\0' terminated char string
 * @return number of characters
 */
static uint32_t lv_txt_utf8_get_length(const char *txt)
{
	uint32_t len = 0;
	uint32_t i = 0;

	while(txt[i] != '\0') {
		EG_TextDecodeNext(txt, &i);
		len++;
	}

	return len;
}

#elif EG_TXT_ENC == EG_TXT_ENC_ASCII

/////////////////////////////////////////////////////////////////////////////

/*******************************
 *  ASCII ENCODER/DECODER
 ******************************/

/**
 * Give the size of an ISO8859-1 coded character
 * @param str pointer to a character in a string
 * @return length of the UTF-8 character (1,2,3 or 4). O on invalid code
 */
static uint8_t lv_txt_iso8859_1_size(const char *str)
{
	EG_UNUSED(str); /*Unused*/
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a Unicode letter to ISO8859-1.
 * @param letter_uni a Unicode letter
 * @return ISO8859-1 coded character in Little Endian to be compatible with C chars (e.g. 'Á', 'Ű')
 */
static uint32_t lv_txt_unicode_to_iso8859_1(uint32_t letter_uni)
{
	if(letter_uni < 256)
		return letter_uni;
	else
		return ' ';
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert wide characters to ASCII, however wide characters in ASCII range (e.g. 'A') are ASCII compatible by default.
 * So this function does nothing just returns with `c`.
 * @param c a character, e.g. 'A'
 * @return same as `c`
 */
static uint32_t lv_txt_iso8859_1_conv_wc(uint32_t c)
{
	return c;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Decode an ISO8859-1 character from a string.
 * @param txt pointer to '\0' terminated string
 * @param i start byte index in 'txt' where to start.
 *          After call it will point to the next UTF-8 char in 'txt'.
 *          NULL to use txt[0] as index
 * @return the decoded Unicode character or 0 on invalid UTF-8 code
 */
static uint32_t lv_txt_iso8859_1_next(const char *txt, uint32_t *i)
{
	if(i == NULL) return txt[0]; /*Get the next char*/

	uint8_t letter = txt[*i];
	(*i)++;
	return letter;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Get previous ISO8859-1 character form a string.
 * @param txt pointer to '\0' terminated string
 * @param i start byte index in 'txt' where to start. After the call it will point to the previous UTF-8 char in 'txt'.
 * @return the decoded Unicode character or 0 on invalid UTF-8 code
 */
static uint32_t lv_txt_iso8859_1_prev(const char *txt, uint32_t *i)
{
	if(i == NULL) return *(txt - 1); /*Get the prev. char*/

	(*i)--;
	uint8_t letter = txt[*i];

	return letter;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a character index (in an ISO8859-1 text) to byte index.
 * E.g. in "AÁRT" index of 'R' is 2th char but start at byte 3 because 'Á' is 2 bytes long
 * @param txt a '\0' terminated UTF-8 string
 * @param utf8_id character index
 * @return byte index of the 'utf8_id'th letter
 */
static uint32_t lv_txt_iso8859_1_get_byte_id(const char *txt, uint32_t utf8_id)
{
	EG_UNUSED(txt); /*Unused*/
	return utf8_id; /*In Non encoded no difference*/
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Convert a byte index (in an ISO8859-1 text) to character index.
 * E.g. in "AÁRT" index of 'R' is 2th char but start at byte 3 because 'Á' is 2 bytes long
 * @param txt a '\0' terminated UTF-8 string
 * @param byte_id byte index
 * @return character index of the letter at 'byte_id'th position
 */
static uint32_t lv_txt_iso8859_1_get_char_id(const char *txt, uint32_t byte_id)
{
	EG_UNUSED(txt); /*Unused*/
	return byte_id; /*In Non encoded no difference*/
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Get the number of characters (and NOT bytes) in a string. Decode it with UTF-8 if enabled.
 * E.g.: "ÁBC" is 3 characters (but 4 bytes)
 * @param txt a '\0' terminated char string
 * @return number of characters
 */
static uint32_t lv_txt_iso8859_1_get_length(const char *txt)
{
	return strlen(txt);
}

/////////////////////////////////////////////////////////////////////////////

#else

#error "Invalid character encoding. See `EG_TXT_ENC` in `EG_Config.h`"

#endif
