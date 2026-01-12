/**
 * @file lv_bidi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stddef.h>
#include "misc/lv_bidi.h"
#include "misc/EG_Text.h"
#include "misc/EG_Memory.h"

#if EG_USE_BIDI

/*********************
 *      DEFINES
 *********************/
#define EG_BIDI_BRACKLET_DEPTH   4

// Highest bit of the 16-bit pos_conv value specifies whether this pos is RTL or not
#define GET_POS(x) ((x) & 0x7FFF)
#define IS_RTL_POS(x) (((x) & 0x8000) != 0)
#define SET_RTL_POS(x, is_rtl) (GET_POS(x) | ((is_rtl)? 0x8000: 0))

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint32_t bracklet_pos;
    EG_BaseDirection_e dir;
} bracket_stack_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static uint32_t lv_bidi_get_next_paragraph(const char * txt);
static EG_BaseDirection_e lv_bidi_get_letter_dir(uint32_t letter);
static bool lv_bidi_letter_is_weak(uint32_t letter);
static bool lv_bidi_letter_is_rtl(uint32_t letter);
static bool lv_bidi_letter_is_neutral(uint32_t letter);

static EG_BaseDirection_e get_next_run(const char * txt, EG_BaseDirection_e base_dir, uint32_t max_len, uint32_t * len,
                                  uint16_t  * pos_conv_len);
static void rtl_reverse(char * dest, const char * src, uint32_t len, uint16_t * pos_conv_out, uint16_t pos_conv_rd_base,
                        uint16_t pos_conv_len);
static uint32_t char_change_to_pair(uint32_t letter);
static EG_BaseDirection_e bracket_process(const char * txt, uint32_t next_pos, uint32_t len, uint32_t letter,
                                     EG_BaseDirection_e base_dir);
static void fill_pos_conv(uint16_t * out, uint16_t len, uint16_t index);
static uint32_t get_txt_len(const char * txt, uint32_t max_len);

/**********************
 *  STATIC VARIABLES
 **********************/
static const uint8_t bracket_left[] = {"<({["};
static const uint8_t bracket_right[] = {">)}]"};
static bracket_stack_t br_stack[EG_BIDI_BRACKLET_DEPTH];
static uint8_t br_stack_p;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Convert a text to get the characters in the correct visual order according to
 * Unicode Bidirectional Algorithm
 * @param str_in the text to process
 * @param str_out store the result here. Has the be `strlen(str_in)` length
 * @param base_dir `EG_BASE_DIR_LTR` or `EG_BASE_DIR_RTL`
 */
void _lv_bidi_process(const char * str_in, char * str_out, EG_BaseDirection_e base_dir)
{
    if(base_dir == EG_BASE_DIR_AUTO) base_dir = _lv_bidi_detect_base_dir(str_in);

    uint32_t par_start = 0;
    uint32_t par_len;

    while(str_in[par_start] == '\n' || str_in[par_start] == '\r') {
        str_out[par_start] = str_in[par_start];
        par_start ++;
    }

    while(str_in[par_start] != '\0') {
        par_len = lv_bidi_get_next_paragraph(&str_in[par_start]);
        _lv_bidi_process_paragraph(&str_in[par_start], &str_out[par_start], par_len, base_dir, NULL, 0);
        par_start += par_len;

        while(str_in[par_start] == '\n' || str_in[par_start] == '\r') {
            str_out[par_start] = str_in[par_start];
            par_start ++;
        }
    }

    str_out[par_start] = '\0';
}

/**
 * Auto-detect the direction of a text based on the first strong character
 * @param txt the text to process
 * @return `EG_BASE_DIR_LTR` or `EG_BASE_DIR_RTL`
 */
EG_BaseDirection_e _lv_bidi_detect_base_dir(const char * txt)
{
    uint32_t i = 0;
    uint32_t letter;
    while(txt[i] != '\0') {
        letter = EG_TextDecodeNext(txt, &i);

        EG_BaseDirection_e dir;
        dir = lv_bidi_get_letter_dir(letter);
        if(dir == EG_BASE_DIR_RTL || dir == EG_BASE_DIR_LTR) return dir;
    }

    /*If there were no strong char earlier return with the default base dir*/
    if(EG_BIDI_BASE_DIR_DEF == EG_BASE_DIR_AUTO) return EG_BASE_DIR_LTR;
    else return EG_BIDI_BASE_DIR_DEF;
}

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
                                  uint32_t visual_pos, bool * is_rtl)
{
    uint32_t pos_conv_len = get_txt_len(str_in, len);
    char * buf = EG_GetBufferMem(len + 1);
    if(buf == NULL) return (uint16_t) -1;

    uint16_t * pos_conv_buf = EG_GetBufferMem(pos_conv_len * sizeof(uint16_t));
    if(pos_conv_buf == NULL) {
        EG_ReleaseBufferMem(buf);
        return (uint16_t) -1;
    }

    if(bidi_txt) *bidi_txt = buf;

    _lv_bidi_process_paragraph(str_in, bidi_txt ? *bidi_txt : NULL, len, base_dir, pos_conv_buf, pos_conv_len);

    if(is_rtl) *is_rtl = IS_RTL_POS(pos_conv_buf[visual_pos]);

    if(bidi_txt == NULL) EG_ReleaseBufferMem(buf);
    uint16_t res = GET_POS(pos_conv_buf[visual_pos]);
    EG_ReleaseBufferMem(pos_conv_buf);
    return res;
}

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
                                 uint32_t logical_pos, bool * is_rtl)
{
    uint32_t pos_conv_len = get_txt_len(str_in, len);
    char * buf = EG_GetBufferMem(len + 1);
    if(buf == NULL) return (uint16_t) -1;

    uint16_t * pos_conv_buf = EG_GetBufferMem(pos_conv_len * sizeof(uint16_t));
    if(pos_conv_buf == NULL) {
        EG_ReleaseBufferMem(buf);
        return (uint16_t) -1;
    }

    if(bidi_txt) *bidi_txt = buf;

    _lv_bidi_process_paragraph(str_in, bidi_txt ? *bidi_txt : NULL, len, base_dir, pos_conv_buf, pos_conv_len);

    for(uint16_t i = 0; i < pos_conv_len; i++) {
        if(GET_POS(pos_conv_buf[i]) == logical_pos) {

            if(is_rtl) *is_rtl = IS_RTL_POS(pos_conv_buf[i]);
            EG_ReleaseBufferMem(pos_conv_buf);

            if(bidi_txt == NULL) EG_ReleaseBufferMem(buf);
            return i;
        }
    }
    EG_ReleaseBufferMem(pos_conv_buf);
    if(bidi_txt == NULL) EG_ReleaseBufferMem(buf);
    return (uint16_t) -1;
}

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
                                uint16_t * pos_conv_out, uint16_t pos_conv_len)
{
    uint32_t run_len = 0;
    EG_BaseDirection_e run_dir;
    uint32_t rd = 0;
    uint32_t wr;
    uint16_t pos_conv_run_len = 0;
    uint16_t pos_conv_rd = 0;
    uint16_t pos_conv_wr;

    if(base_dir == EG_BASE_DIR_AUTO) base_dir = _lv_bidi_detect_base_dir(str_in);
    if(base_dir == EG_BASE_DIR_RTL) {
        wr = len;
        pos_conv_wr = pos_conv_len;
    }
    else {
        wr = 0;
        pos_conv_wr = 0;
    }

    if(str_out) str_out[len] = '\0';

    EG_BaseDirection_e dir = base_dir;

    /*Empty the bracket stack*/
    br_stack_p = 0;

    /*Process neutral chars in the beginning*/
    while(rd < len) {
        uint32_t letter = EG_TextDecodeNext(str_in, &rd);
        pos_conv_rd++;
        dir = lv_bidi_get_letter_dir(letter);
        if(dir == LV_BASE_DIR_NEUTRAL)  dir = bracket_process(str_in, rd, len, letter, base_dir);
        if(dir != LV_BASE_DIR_NEUTRAL && dir != LV_BASE_DIR_WEAK) break;
    }

    if(rd && str_in[rd] != '\0') {
        EG_TextDecodePrevious(str_in, &rd);
        pos_conv_rd--;
    }

    if(rd) {
        if(base_dir == EG_BASE_DIR_LTR) {
            if(str_out) {
                EG_CopyMem(&str_out[wr], str_in, rd);
                wr += rd;
            }
            if(pos_conv_out) {
                fill_pos_conv(&pos_conv_out[pos_conv_wr], pos_conv_rd, 0);
                pos_conv_wr += pos_conv_rd;
            }
        }
        else {
            wr -= rd;
            pos_conv_wr -= pos_conv_rd;
            rtl_reverse(str_out ? &str_out[wr] : NULL, str_in, rd, pos_conv_out ? &pos_conv_out[pos_conv_wr] : NULL, 0,
                        pos_conv_rd);
        }
    }

    /*Get and process the runs*/

    while(rd < len && str_in[rd]) {
        run_dir = get_next_run(&str_in[rd], base_dir, len - rd, &run_len, &pos_conv_run_len);

        if(base_dir == EG_BASE_DIR_LTR) {
            if(run_dir == EG_BASE_DIR_LTR) {
                if(str_out) EG_CopyMem(&str_out[wr], &str_in[rd], run_len);
                if(pos_conv_out) fill_pos_conv(&pos_conv_out[pos_conv_wr], pos_conv_run_len, pos_conv_rd);
            }
            else rtl_reverse(str_out ? &str_out[wr] : NULL, &str_in[rd], run_len, pos_conv_out ? &pos_conv_out[pos_conv_wr] : NULL,
                                 pos_conv_rd, pos_conv_run_len);
            wr += run_len;
            pos_conv_wr += pos_conv_run_len;
        }
        else {
            wr -= run_len;
            pos_conv_wr -= pos_conv_run_len;
            if(run_dir == EG_BASE_DIR_LTR) {
                if(str_out) EG_CopyMem(&str_out[wr], &str_in[rd], run_len);
                if(pos_conv_out) fill_pos_conv(&pos_conv_out[pos_conv_wr], pos_conv_run_len, pos_conv_rd);
            }
            else rtl_reverse(str_out ? &str_out[wr] : NULL, &str_in[rd], run_len, pos_conv_out ? &pos_conv_out[pos_conv_wr] : NULL,
                                 pos_conv_rd, pos_conv_run_len);
        }

        rd += run_len;
        pos_conv_rd += pos_conv_run_len;
    }
}

void lv_bidi_calculate_align(EG_TextAlignment_t * align, EG_BaseDirection_e * base_dir, const char * txt)
{
    if(*base_dir == EG_BASE_DIR_AUTO) *base_dir = _lv_bidi_detect_base_dir(txt);

    if(*align == EG_TEXT_ALIGN_AUTO) {
        if(*base_dir == EG_BASE_DIR_RTL) *align = EG_TEXT_ALIGN_RIGHT;
        else *align = EG_TEXT_ALIGN_LEFT;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get the next paragraph from a text
 * @param txt the text to process
 * @return the length of the current paragraph in byte count
 */
static uint32_t lv_bidi_get_next_paragraph(const char * txt)
{
    uint32_t i = 0;

    EG_TextDecodeNext(txt, &i);

    while(txt[i] != '\0' && txt[i] != '\n' && txt[i] != '\r') {
        EG_TextDecodeNext(txt, &i);
    }

    return i;
}

/**
 * Get the direction of a character
 * @param letter a Unicode character
 * @return `EG_BASE_DIR_RTL/LTR/WEAK/NEUTRAL`
 */
static EG_BaseDirection_e lv_bidi_get_letter_dir(uint32_t letter)
{
    if(lv_bidi_letter_is_rtl(letter)) return EG_BASE_DIR_RTL;
    if(lv_bidi_letter_is_neutral(letter)) return LV_BASE_DIR_NEUTRAL;
    if(lv_bidi_letter_is_weak(letter)) return LV_BASE_DIR_WEAK;

    return EG_BASE_DIR_LTR;
}
/**
 * Tell whether a character is weak or not
 * @param letter a Unicode character
 * @return true/false
 */
static bool lv_bidi_letter_is_weak(uint32_t letter)
{
    uint32_t i = 0;
    static const char weaks[] = "0123456789";

    do {
        uint32_t x = EG_TextDecodeNext(weaks, &i);
        if(letter == x) {
            return true;
        }
    } while(weaks[i] != '\0');

    return false;
}
/**
 * Tell whether a character is RTL or not
 * @param letter a Unicode character
 * @return true/false
 */
static bool lv_bidi_letter_is_rtl(uint32_t letter)
{
    if(letter == 0x202E) return true;               /*Unicode of EG_BIDI_RLO*/

    /*Check for Persian and Arabic characters [https://en.wikipedia.org/wiki/Arabic_script_in_Unicode]*/
    if(letter >= 0x600 && letter <= 0x6FF) return true;
    if(letter >= 0xFB50 && letter <= 0xFDFF) return true;
    if(letter >= 0xFE70 && letter <= 0xFEFF) return true;

    /*Check for Hebrew characters [https://en.wikipedia.org/wiki/Unicode_and_HTML_for_the_Hebrew_alphabet]*/
    if(letter >= 0x590 && letter <= 0x5FF) return true;
    if(letter >= 0xFB1D && letter <= 0xFB4F) return true;

    return false;
}

/**
 * Tell whether a character is neutral or not
 * @param letter a Unicode character
 * @return true/false
 */
static bool lv_bidi_letter_is_neutral(uint32_t letter)
{
    uint16_t i;
    static const char neutrals[] = " \t\n\r.,:;'\"`!?%/\\-=()[]{}<>@#&$|";
    for(i = 0; neutrals[i] != '\0'; i++) {
        if(letter == (uint32_t)neutrals[i]) return true;
    }

    return false;
}

static uint32_t get_txt_len(const char * txt, uint32_t max_len)
{
    uint32_t len = 0;
    uint32_t i   = 0;

    while(i < max_len && txt[i] != '\0') {
        EG_TextDecodeNext(txt, &i);
        len++;
    }

    return len;
}

static void fill_pos_conv(uint16_t * out, uint16_t len, uint16_t index)
{
    uint16_t i;
    for(i = 0; i < len; i++) {
        out[i] = SET_RTL_POS(index, false);
        index++;
    }
}

static EG_BaseDirection_e get_next_run(const char * txt, EG_BaseDirection_e base_dir, uint32_t max_len, uint32_t * len,
                                  uint16_t  * pos_conv_len)
{
    uint32_t i = 0;
    uint32_t letter;

    uint16_t pos_conv_i = 0;

    letter = EG_TextDecodeNext(txt, NULL);
    EG_BaseDirection_e dir = lv_bidi_get_letter_dir(letter);
    if(dir == LV_BASE_DIR_NEUTRAL)  dir = bracket_process(txt, 0, max_len, letter, base_dir);

    /*Find the first strong char. Skip the neutrals*/
    while(dir == LV_BASE_DIR_NEUTRAL || dir == LV_BASE_DIR_WEAK) {
        letter = EG_TextDecodeNext(txt, &i);

        pos_conv_i++;
        dir = lv_bidi_get_letter_dir(letter);
        if(dir == LV_BASE_DIR_NEUTRAL)  dir = bracket_process(txt, i, max_len, letter, base_dir);

        if(dir == EG_BASE_DIR_LTR || dir == EG_BASE_DIR_RTL)  break;

        if(i >= max_len || txt[i] == '\0' || txt[i] == '\n' || txt[i] == '\r') {
            *len = i;
            *pos_conv_len = pos_conv_i;
            return base_dir;
        }
    }

    EG_BaseDirection_e run_dir = dir;

    uint32_t i_prev = i;
    uint32_t i_last_strong = i;
    uint16_t pos_conv_i_prev = pos_conv_i;
    uint16_t pos_conv_i_last_strong = pos_conv_i;

    /*Find the next char which has different direction*/
    EG_BaseDirection_e next_dir = base_dir;
    while(i_prev < max_len && txt[i] != '\0' && txt[i] != '\n' && txt[i] != '\r') {
        letter = EG_TextDecodeNext(txt, &i);
        pos_conv_i++;
        next_dir  = lv_bidi_get_letter_dir(letter);
        if(next_dir == LV_BASE_DIR_NEUTRAL)  next_dir = bracket_process(txt, i, max_len, letter, base_dir);

        if(next_dir == LV_BASE_DIR_WEAK) {
            if(run_dir == EG_BASE_DIR_RTL) {
                if(base_dir == EG_BASE_DIR_RTL) {
                    next_dir = EG_BASE_DIR_LTR;
                }
            }
        }

        /*New dir found?*/
        if((next_dir == EG_BASE_DIR_RTL || next_dir == EG_BASE_DIR_LTR) && next_dir != run_dir) {
            /*Include neutrals if `run_dir == base_dir`*/
            if(run_dir == base_dir) {
                *len = i_prev;
                *pos_conv_len = pos_conv_i_prev;
            }
            /*Exclude neutrals if `run_dir != base_dir`*/
            else {
                *len = i_last_strong;
                *pos_conv_len = pos_conv_i_last_strong;
            }

            return run_dir;
        }

        if(next_dir != LV_BASE_DIR_NEUTRAL) {
            i_last_strong = i;
            pos_conv_i_last_strong = pos_conv_i;
        }

        i_prev = i;
        pos_conv_i_prev = pos_conv_i;
    }

    /*Handle end of of string. Apply `base_dir` on trailing neutrals*/

    /*Include neutrals if `run_dir == base_dir`*/
    if(run_dir == base_dir) {
        *len = i_prev;
        *pos_conv_len = pos_conv_i_prev;
    }
    /*Exclude neutrals if `run_dir != base_dir`*/
    else {
        *len = i_last_strong;
        *pos_conv_len = pos_conv_i_last_strong;
    }

    return run_dir;
}

static void rtl_reverse(char * dest, const char * src, uint32_t len, uint16_t * pos_conv_out, uint16_t pos_conv_rd_base,
                        uint16_t pos_conv_len)
{
    uint32_t i = len;
    uint32_t wr = 0;
    uint16_t pos_conv_i = pos_conv_len;
    uint16_t pos_conv_wr = 0;

    while(i) {
        uint32_t letter = EG_TextDecodePrevious(src, &i);
        uint16_t pos_conv_letter = --pos_conv_i;

        /*Keep weak letters (numbers) as LTR*/
        if(lv_bidi_letter_is_weak(letter)) {
            uint32_t last_weak = i;
            uint32_t first_weak = i;
            uint16_t pos_conv_last_weak = pos_conv_i;
            uint16_t pos_conv_first_weak = pos_conv_i;
            while(i) {
                letter = EG_TextDecodePrevious(src, &i);
                pos_conv_letter = --pos_conv_i;

                /*No need to call `char_change_to_pair` because there not such chars here*/

                /*Finish on non-weak char*/
                /*but treat number and currency related chars as weak*/
                if(lv_bidi_letter_is_weak(letter) == false && letter != '.' && letter != ',' && letter != '$' && letter != '%') {
                    EG_TextDecodeNext(src, &i);   /*Rewind one letter*/
                    pos_conv_i++;
                    first_weak = i;
                    pos_conv_first_weak = pos_conv_i;
                    break;
                }
            }
            if(i == 0) {
                first_weak = 0;
                pos_conv_first_weak = 0;
            }

            if(dest) EG_CopyMem(&dest[wr], &src[first_weak], last_weak - first_weak + 1);
            if(pos_conv_out) fill_pos_conv(&pos_conv_out[pos_conv_wr], pos_conv_last_weak - pos_conv_first_weak + 1,
                                               pos_conv_rd_base + pos_conv_first_weak);
            wr += last_weak - first_weak + 1;
            pos_conv_wr += pos_conv_last_weak - pos_conv_first_weak + 1;
        }

        /*Simply store in reversed order*/
        else {
            uint32_t letter_size = EG_TextEncodedSize((const char *)&src[i]);
            /*Swap arithmetical symbols*/
            if(letter_size == 1) {
                uint32_t new_letter = letter = char_change_to_pair(letter);
                if(dest) dest[wr] = (uint8_t)new_letter;
                if(pos_conv_out) pos_conv_out[pos_conv_wr] = SET_RTL_POS(pos_conv_rd_base + pos_conv_letter, true);
                wr++;
                pos_conv_wr++;
            }
            /*Just store the letter*/
            else {
                if(dest) EG_CopyMem(&dest[wr], &src[i], letter_size);
                if(pos_conv_out) pos_conv_out[pos_conv_wr] = SET_RTL_POS(pos_conv_rd_base + pos_conv_i, true);
                wr += letter_size;
                pos_conv_wr++;
            }
        }
    }
}

static uint32_t char_change_to_pair(uint32_t letter)
{

    uint8_t i;
    for(i = 0; bracket_left[i] != '\0'; i++) {
        if(letter == bracket_left[i]) return bracket_right[i];
    }

    for(i = 0; bracket_right[i] != '\0'; i++) {
        if(letter == bracket_right[i]) return bracket_left[i];
    }

    return letter;
}

static EG_BaseDirection_e bracket_process(const char * txt, uint32_t next_pos, uint32_t len, uint32_t letter,
                                     EG_BaseDirection_e base_dir)
{
    EG_BaseDirection_e bracket_dir = LV_BASE_DIR_NEUTRAL;

    uint8_t i;
    /*Is the letter an opening bracket?*/
    for(i = 0; bracket_left[i] != '\0'; i++) {
        if(bracket_left[i] == letter) {
            /*If so find its matching closing bracket.
             *If a char with base dir. direction is found then the brackets will have `base_dir` direction*/
            uint32_t txt_i = next_pos;
            while(txt_i < len) {
                uint32_t letter_next = EG_TextDecodeNext(txt, &txt_i);
                if(letter_next == bracket_right[i]) {
                    /*Closing bracket found*/
                    break;
                }
                else {
                    /*Save the dir*/
                    EG_BaseDirection_e letter_dir = lv_bidi_get_letter_dir(letter_next);
                    if(letter_dir == base_dir) {
                        bracket_dir = base_dir;
                    }
                }
            }

            /*There were no matching closing bracket*/
            if(txt_i > len)  return LV_BASE_DIR_NEUTRAL;

            /*There where a strong char with base dir in the bracket so the dir is found.*/
            if(bracket_dir != LV_BASE_DIR_NEUTRAL && bracket_dir != LV_BASE_DIR_WEAK) break;

            /*If there were no matching strong chars in the brackets then check the previous chars*/
            txt_i = next_pos;
            if(txt_i) EG_TextDecodePrevious(txt, &txt_i);
            while(txt_i > 0) {
                uint32_t letter_next = EG_TextDecodePrevious(txt, &txt_i);
                EG_BaseDirection_e letter_dir = lv_bidi_get_letter_dir(letter_next);
                if(letter_dir == EG_BASE_DIR_LTR || letter_dir == EG_BASE_DIR_RTL) {
                    bracket_dir = letter_dir;
                    break;
                }
            }

            /*There where a previous strong char which can be used*/
            if(bracket_dir != LV_BASE_DIR_NEUTRAL) break;

            /*There were no strong chars before the bracket, so use the base dir.*/
            if(txt_i == 0) bracket_dir = base_dir;

            break;
        }
    }

    /*The letter was an opening bracket*/
    if(bracket_left[i] != '\0') {

        if(bracket_dir == LV_BASE_DIR_NEUTRAL || br_stack_p == EG_BIDI_BRACKLET_DEPTH) return LV_BASE_DIR_NEUTRAL;

        br_stack[br_stack_p].bracklet_pos = i;
        br_stack[br_stack_p].dir = bracket_dir;

        br_stack_p++;
        return bracket_dir;
    }
    else if(br_stack_p > 0) {
        /*Is the letter a closing bracket of the last opening?*/
        if(letter == bracket_right[br_stack[br_stack_p - 1].bracklet_pos]) {
            bracket_dir = br_stack[br_stack_p - 1].dir;
            br_stack_p--;
            return bracket_dir;
        }
    }

    return LV_BASE_DIR_NEUTRAL;
}

#endif /*EG_USE_BIDI*/
