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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

#include <stdbool.h>
#include <stdarg.h>
#include "EG_Rect.h"
#include "../font/EG_Font.h"
#include "lv_printf.h"
#include "EG_Types.h"

/////////////////////////////////////////////////////////////////////////////

#ifndef EG_TXT_COLOR_CMD
#define EG_TXT_COLOR_CMD "#"
#endif

#define EG_TXT_ENC_UTF8 1
#define EG_TXT_ENC_ASCII 2

/////////////////////////////////////////////////////////////////////////////

// Options for text rendering.
enum {
    EG_TEXT_FLAG_NONE    = 0x00,
    EG_TEXT_FLAG_RECOLOR = 0x01, // Enable parsing of recolor command
    EG_TEXT_FLAG_EXPAND  = 0x02, // Ignore max-width to avoid automatic word wrapping
    EG_TEXT_FLAG_FIT     = 0x04, // Max-width is already equal to the longest line. (Used to skip some calculation)
};
typedef uint8_t EG_TextFlag_t;

// State machine for text renderer.
enum {
    EG_TEXT_CMD_STATE_WAIT, // Waiting for command
    EG_TEXT_CMD_STATE_PAR,  // Processing the parameter
    EG_TEXT_CMD_STATE_IN,   // Processing the command
};
typedef uint8_t EG_TextCommandState_t;

// Label align policy
enum {
    EG_TEXT_ALIGN_AUTO, // Align text auto
    EG_TEXT_ALIGN_LEFT, // Align text to left
    EG_TEXT_ALIGN_CENTER, // Align text to center
    EG_TEXT_ALIGN_RIGHT, // Align text to right
};
typedef uint8_t EG_TextAlignment_t;

/////////////////////////////////////////////////////////////////////////////

/**
 * Get size of a text
 * @param size_res pointer to a 'point_t' variable to store the result
 * @param text pointer to a text
 * @param font pointer to font of the text
 * @param letter_space letter space of the text
 * @param line_space line space of the text
 * @param flags settings for the text from ::EG_TextFlag_t
 * @param max_width max width of the text (break the lines to fit this size). Set COORD_MAX to avoid
 * line breaks
 */
void EG_GetTextSize(EGPoint * size_res, const char * text, const EG_Font_t * font, EG_Coord_t letter_space,
                     EG_Coord_t line_space, EG_Coord_t max_width, EG_TextFlag_t flag);

/**
 * Get the next line of text. Check line length and break chars too.
 * @param txt a '\0' terminated string
 * @param font pointer to a font
 * @param letter_space letter space
 * @param max_width max width of the text (break the lines to fit this size). Set COORD_MAX to avoid
 * line breaks
 * @param used_width When used_width != NULL, save the width of this line if
 * flag == EG_TEXT_FLAG_NONE, otherwise save -1.
 * @param flags settings for the text from 'txt_flag_type' enum
 * @return the index of the first char of the new line (in byte index not letter index. With UTF-8
 * they are different)
 */
uint32_t EG_GetNextTextLine(const char * txt, const EG_Font_t * font, EG_Coord_t letter_space,
                               EG_Coord_t max_width, EG_Coord_t * used_width, EG_TextFlag_t flag);

/**
 * Give the length of a text with a given font
 * @param txt a '\0' terminate string
 * @param length length of 'txt' in byte count and not characters (Á is 1 character but 2 bytes in
 * UTF-8)
 * @param font pointer to a font
 * @param letter_space letter space
 * @param flags settings for the text from 'txt_flag_t' enum
 * @return length of a char_num long text
 */
EG_Coord_t EG_GetTextWidth(const char * txt, uint32_t length, const EG_Font_t * font, EG_Coord_t letter_space,
                            EG_TextFlag_t flag);

/**
 * Check next character in a string and decide if the character is part of the command or not
 * @param state pointer to a txt_cmd_state_t variable which stores the current state of command
 * processing
 * @param c the current character
 * @return true: the character is part of a command and should not be written,
 *         false: the character should be written
 */
bool EG_TextIsCommand(EG_TextCommandState_t *pState, uint32_t Char);

/**
 * Insert a string into another
 * @param txt_buf the original text (must be big enough for the result text and NULL terminated)
 * @param pos position to insert (0: before the original text, 1: after the first char etc.)
 * @param ins_txt text to insert, must be '\0' terminated
 */
void EG_TextInsert(char *pBuffer, uint32_t Pos, const char *pText);

/**
 * Delete a part of a string
 * @param txt string to modify, must be '\0' terminated and should point to a heap or stack frame, not read-only memory.
 * @param pos position where to start the deleting (0: before the first char, 1: after the first
 * char etc.)
 * @param len number of characters to delete
 */
void EG_TextCut(char *pBuffer, uint32_t Pos, uint32_t Length);

/**
 * return a new formatted text. Memory will be allocated to store the text.
 * @param fmt `printf`-like format
 * @return pointer to the allocated text string.
 */
char* EG_TextFormat(const char *pFmt, va_list ap) EG_FORMAT_ATTRIBUTE(1, 0);

/**
 * Decode two encoded character from a string.
 * @param pBuffer pointer to '\0' terminated string
 * @param pChar1 the first decoded Unicode character or 0 on invalid data code
 * @param pChar2 the second decoded Unicode character or 0 on invalid data code
 * @param pOffset start index in 'txt' where to start.
 *                After the call it will point to the next encoded char in 'txt'.
 *                NULL to use txt[0] as index
 */
void EG_TextDecode2(const char *pBuffer, uint32_t *pChar1, uint32_t *pChar2, uint32_t *pOffset);

/////////////////////////////////////////////////////////////////////////////

/**
 * Test if char is break char or not (a text can broken here or not)
 * @param Char
 * @return false: 'Char' is not break char
 */
static inline bool EG_TextIsBreak(uint32_t Char)
{
  // each chinese character can be break 
  if(Char >= 0x4E00 && Char <= 0x9FA5) return true;
  // Compare the letter to TXT_BREAK_CHARS
  for(uint8_t i = 0; EG_TXT_BREAK_CHARS[i] != '\0'; i++) {
    if(Char == (uint32_t)EG_TXT_BREAK_CHARS[i]) {
      return true; // If match then it is break char
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////

/***************************************************************
 *  GLOBAL FUNCTION POINTERS FOR CHARACTER ENCODING INTERFACE
 ***************************************************************/

/**
 * Give the size of an encoded character
 * @param str pointer to a character in a string
 * @return length of the encoded character (1,2,3 ...). O in invalid
 */
extern uint8_t (*EG_TextEncodedSize)(const char *);

/**
 * Convert a Unicode letter to encoded
 * @param letter_uni a Unicode letter
 * @return Encoded character in Little Endian to be compatible with C chars (e.g. 'Á', 'Ü')
 */
extern uint32_t (*EG_TextUnicodeToEncoded)(uint32_t);

/**
 * Convert a wide character, e.g. 'Á' little endian to be compatible with the encoded format.
 * @param c a wide character
 * @return `c` in the encoded format
 */
extern uint32_t (*EG_TextEncodedToWide)(uint32_t c);

/**
 * Decode the next encoded character from a string.
 * @param txt pointer to '\0' terminated string
 * @param i start index in 'txt' where to start.
 *                After the call it will point to the next encoded char in 'txt'.
 *                NULL to use txt[0] as index
 * @return the decoded Unicode character or 0 on invalid data code
 */
extern uint32_t (*EG_TextDecodeNext)(const char *, uint32_t *);

/**
 * Get the previous encoded character form a string.
 * @param txt pointer to '\0' terminated string
 * @param i_start index in 'txt' where to start. After the call it will point to the previous
 * encoded char in 'txt'.
 * @return the decoded Unicode character or 0 on invalid data
 */
extern uint32_t (*EG_TextDecodePrevious)(const char *, uint32_t *);

/**
 * Convert a letter index (in the encoded text) to byte index.
 * E.g. in UTF-8 "AÁRT" index of 'R' is 2 but start at byte 3 because 'Á' is 2 bytes long
 * @param txt a '\0' terminated UTF-8 string
 * @param enc_id letter index
 * @return byte index of the 'enc_id'th letter
 */
extern uint32_t (*EG_TextEncodedGetIndex)(const char *, uint32_t);

/**
 * Convert a byte index (in an encoded text) to character index.
 * E.g. in UTF-8 "AÁRT" index of 'R' is 2 but start at byte 3 because 'Á' is 2 bytes long
 * @param txt a '\0' terminated UTF-8 string
 * @param byte_id byte index
 * @return character index of the letter at 'byte_id'th position
 */
extern uint32_t (*EG_TextEncodedGetPosition)(const char *, uint32_t);

/**
 * Get the number of characters (and NOT bytes) in a string.
 * E.g. in UTF-8 "ÁBC" is 3 characters (but 4 bytes)
 * @param txt a '\0' terminated char string
 * @return number of characters
 */
extern uint32_t (*EG_TextEncodedGetLength)(const char *);

