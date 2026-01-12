/**
 * @file lv_bidi.h
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

#include <stdbool.h>
#include <stdint.h>
#include "EG_Text.h"

/*Special non printable strong characters.
 *They can be inserted to texts to affect the run's direction*/
#define EG_BIDI_LRO  "\xE2\x80\xAD" /*U+202D*/
#define EG_BIDI_RLO  "\xE2\x80\xAE" /*U+202E*/

typedef enum EG_BaseDirection_e : uint8_t{
    EG_BASE_DIR_LTR      = 0x00,
    EG_BASE_DIR_RTL      = 0x01,
    EG_BASE_DIR_AUTO     = 0x02,

    LV_BASE_DIR_NEUTRAL  = 0x20,
    LV_BASE_DIR_WEAK     = 0x21,
} EG_BaseDirection_e;

#if EG_USE_BIDI

/**
 * Convert a text to get the characters in the correct visual order according to
 * Unicode Bidirectional Algorithm
 * @param str_in the text to process
 * @param str_out store the result here. Has the be `strlen(str_in)` length
 * @param base_dir `EG_BASE_DIR_LTR` or `EG_BASE_DIR_RTL`
 */
void _lv_bidi_process(const char * str_in, char * str_out, EG_BaseDirection_e base_dir);

/**
 * Auto-detect the direction of a text based on the first strong character
 * @param txt the text to process
 * @return `EG_BASE_DIR_LTR` or `EG_BASE_DIR_RTL`
 */
EG_BaseDirection_e _lv_bidi_detect_base_dir(const char * txt);

/**
 * Get the logical position of a character in a line
 * @param str_in the input string. Can be only one line.
 * @param bidi_txt internally the text is bidi processed which buffer can be get here.
 * If not required anymore has to freed with `EG_FreeMem()`
 * Can be `NULL` is unused
 * @param len length of the line in character count
 * @param base_dir base direction of the text: `EG_BASE_DIR_LTR` or `EG_BASE_DIR_RTL`
 * @param visual_pos the visual character position which logical position should be get
 * @param is_rtl tell the char at `visual_pos` is RTL or LTR context
 * @return the logical character position
 */
uint16_t _lv_bidi_get_logical_pos(const char * str_in, char ** bidi_txt, uint32_t len, EG_BaseDirection_e base_dir,
                                  uint32_t visual_pos, bool * is_rtl);

/**
 * Get the visual position of a character in a line
 * @param str_in the input string. Can be only one line.
 * @param bidi_txt internally the text is bidi processed which buffer can be get here.
 * If not required anymore has to freed with `EG_FreeMem()`
 * Can be `NULL` is unused
 * @param len length of the line in character count
 * @param base_dir base direction of the text: `EG_BASE_DIR_LTR` or `EG_BASE_DIR_RTL`
 * @param logical_pos the logical character position which visual position should be get
 * @param is_rtl tell the char at `logical_pos` is RTL or LTR context
 * @return the visual character position
 */
uint16_t _lv_bidi_get_visual_pos(const char * str_in, char ** bidi_txt, uint16_t len, EG_BaseDirection_e base_dir,
                                 uint32_t logical_pos, bool * is_rtl);

/**
 * Bidi process a paragraph of text
 * @param str_in the string to process
 * @param str_out store the result here
 * @param len length of the text
 * @param base_dir base dir of the text
 * @param pos_conv_out an `uint16_t` array to store the related logical position of the character.
 * Can be `NULL` is unused
 * @param pos_conv_len length of `pos_conv_out` in element count
 */
void _lv_bidi_process_paragraph(const char * str_in, char * str_out, uint32_t len, EG_BaseDirection_e base_dir,
                                uint16_t * pos_conv_out, uint16_t pos_conv_len);

/**
 * Get the real text alignment from the a text alignment, base direction and a text.
 * @param align     EG_TEXT_ALIGN_..., write back the calculated align here (EG_TEXT_ALIGN_LEFT/RIGHT/CENTER)
 * @param base_dir  LV_BASE_DIR_..., write the calculated base dir here (EG_BASE_DIR_LTR/RTL)
 * @param txt       a text, used with EG_BASE_DIR_AUTO to determine the base direction
 */
void lv_bidi_calculate_align(EG_TextAlignment_t * align, EG_BaseDirection_e * base_dir, const char * txt);

/**********************
 *      MACROS
 **********************/

#else 
/**
 * For compatibility if EG_USE_BIDI = 0
 * Get the real text alignment from the a text alignment, base direction and a text.
 * @param align     For EG_TEXT_ALIGN_AUTO give EG_TEXT_ALIGN_LEFT else leave unchanged, write back the calculated align here
 * @param base_dir  Unused
 * @param txt       Unused
 */
static inline void lv_bidi_calculate_align(EG_TextAlignment_t * align, EG_BaseDirection_e * base_dir, const char * txt)
{
    EG_UNUSED(txt);
    EG_UNUSED(base_dir);
    if(*align == EG_TEXT_ALIGN_AUTO) * align = EG_TEXT_ALIGN_LEFT;
}
#endif
