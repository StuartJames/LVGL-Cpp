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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

/*-------------------------------
 * Symbols from "normal" font
 *-----------------------------*/
#if !defined EG_SYMBOL_BULLET
#define EG_SYMBOL_BULLET          "\xE2\x80\xA2" /*20042, 0x2022*/
#endif

/*-------------------------------
 * Symbols from FontAwesome font
 *-----------------------------*/

/*In the font converter use this list as range:
      61441, 61448, 61451, 61452, 61453, 61457, 61459, 61461, 61465, 61468,
      61473, 61478, 61479, 61480, 61502, 61507, 61512, 61515, 61516, 61517,
      61521, 61522, 61523, 61524, 61543, 61544, 61550, 61552, 61553, 61556,
      61559, 61560, 61561, 61563, 61587, 61589, 61636, 61637, 61639, 61641,
      61664, 61671, 61674, 61683, 61724, 61732, 61787, 61931, 62016, 62017,
      62018, 62019, 62020, 62087, 62099, 62189, 62212, 62810, 63426, 63650
*/

/* These symbols can be prefined in the EG_Config.h file.
 * If they are not predefined, they will use the following values
 */

#if !defined EG_SYMBOL_AUDIO
#define EG_SYMBOL_AUDIO           "\xEF\x80\x81" /*61441, 0xF001*/
#endif

#if !defined EG_SYMBOL_VIDEO
#define EG_SYMBOL_VIDEO           "\xEF\x80\x88" /*61448, 0xF008*/
#endif

#if !defined EG_SYMBOL_LIST
#define EG_SYMBOL_LIST            "\xEF\x80\x8B" /*61451, 0xF00B*/
#endif

#if !defined EG_SYMBOL_OK
#define EG_SYMBOL_OK              "\xEF\x80\x8C" /*61452, 0xF00C*/
#endif

#if !defined EG_SYMBOL_CLOSE
#define EG_SYMBOL_CLOSE           "\xEF\x80\x8D" /*61453, 0xF00D*/
#endif

#if !defined EG_SYMBOL_POWER
#define EG_SYMBOL_POWER           "\xEF\x80\x91" /*61457, 0xF011*/
#endif

#if !defined EG_SYMBOL_SETTINGS
#define EG_SYMBOL_SETTINGS        "\xEF\x80\x93" /*61459, 0xF013*/
#endif

#if !defined EG_SYMBOL_HOME
#define EG_SYMBOL_HOME            "\xEF\x80\x95" /*61461, 0xF015*/
#endif

#if !defined EG_SYMBOL_DOWNLOAD
#define EG_SYMBOL_DOWNLOAD        "\xEF\x80\x99" /*61465, 0xF019*/
#endif

#if !defined EG_SYMBOL_DRIVE
#define EG_SYMBOL_DRIVE           "\xEF\x80\x9C" /*61468, 0xF01C*/
#endif

#if !defined EG_SYMBOL_REFRESH
#define EG_SYMBOL_REFRESH         "\xEF\x80\xA1" /*61473, 0xF021*/
#endif

#if !defined EG_SYMBOL_MUTE
#define EG_SYMBOL_MUTE            "\xEF\x80\xA6" /*61478, 0xF026*/
#endif

#if !defined EG_SYMBOL_VOLUME_MID
#define EG_SYMBOL_VOLUME_MID      "\xEF\x80\xA7" /*61479, 0xF027*/
#endif

#if !defined EG_SYMBOL_VOLUME_MAX
#define EG_SYMBOL_VOLUME_MAX      "\xEF\x80\xA8" /*61480, 0xF028*/
#endif

#if !defined EG_SYMBOL_IMAGE
#define EG_SYMBOL_IMAGE           "\xEF\x80\xBE" /*61502, 0xF03E*/
#endif

#if !defined EG_SYMBOL_TINT
#define EG_SYMBOL_TINT            "\xEF\x81\x83" /*61507, 0xF043*/
#endif

#if !defined EG_SYMBOL_PREV
#define EG_SYMBOL_PREV            "\xEF\x81\x88" /*61512, 0xF048*/
#endif

#if !defined EG_SYMBOL_PLAY
#define EG_SYMBOL_PLAY            "\xEF\x81\x8B" /*61515, 0xF04B*/
#endif

#if !defined EG_SYMBOL_PAUSE
#define EG_SYMBOL_PAUSE           "\xEF\x81\x8C" /*61516, 0xF04C*/
#endif

#if !defined EG_SYMBOL_STOP
#define EG_SYMBOL_STOP            "\xEF\x81\x8D" /*61517, 0xF04D*/
#endif

#if !defined EG_SYMBOL_NEXT
#define EG_SYMBOL_NEXT            "\xEF\x81\x91" /*61521, 0xF051*/
#endif

#if !defined EG_SYMBOL_EJECT
#define EG_SYMBOL_EJECT           "\xEF\x81\x92" /*61522, 0xF052*/
#endif

#if !defined EG_SYMBOL_LEFT
#define EG_SYMBOL_LEFT            "\xEF\x81\x93" /*61523, 0xF053*/
#endif

#if !defined EG_SYMBOL_RIGHT
#define EG_SYMBOL_RIGHT           "\xEF\x81\x94" /*61524, 0xF054*/
#endif

#if !defined EG_SYMBOL_PLUS
#define EG_SYMBOL_PLUS            "\xEF\x81\xA7" /*61543, 0xF067*/
#endif

#if !defined EG_SYMBOL_MINUS
#define EG_SYMBOL_MINUS           "\xEF\x81\xA8" /*61544, 0xF068*/
#endif

#if !defined EG_SYMBOL_EYE_OPEN
#define EG_SYMBOL_EYE_OPEN        "\xEF\x81\xAE" /*61550, 0xF06E*/
#endif

#if !defined EG_SYMBOL_EYE_CLOSE
#define EG_SYMBOL_EYE_CLOSE       "\xEF\x81\xB0" /*61552, 0xF070*/
#endif

#if !defined EG_SYMBOL_WARNING
#define EG_SYMBOL_WARNING         "\xEF\x81\xB1" /*61553, 0xF071*/
#endif

#if !defined EG_SYMBOL_SHUFFLE
#define EG_SYMBOL_SHUFFLE         "\xEF\x81\xB4" /*61556, 0xF074*/
#endif

#if !defined EG_SYMBOL_UP
#define EG_SYMBOL_UP              "\xEF\x81\xB7" /*61559, 0xF077*/
#endif

#if !defined EG_SYMBOL_DOWN
#define EG_SYMBOL_DOWN            "\xEF\x81\xB8" /*61560, 0xF078*/
#endif

#if !defined EG_SYMBOL_LOOP
#define EG_SYMBOL_LOOP            "\xEF\x81\xB9" /*61561, 0xF079*/
#endif

#if !defined EG_SYMBOL_DIRECTORY
#define EG_SYMBOL_DIRECTORY       "\xEF\x81\xBB" /*61563, 0xF07B*/
#endif

#if !defined EG_SYMBOL_UPLOAD
#define EG_SYMBOL_UPLOAD          "\xEF\x82\x93" /*61587, 0xF093*/
#endif

#if !defined EG_SYMBOL_CALL
#define EG_SYMBOL_CALL            "\xEF\x82\x95" /*61589, 0xF095*/
#endif

#if !defined EG_SYMBOL_CUT
#define EG_SYMBOL_CUT             "\xEF\x83\x84" /*61636, 0xF0C4*/
#endif

#if !defined EG_SYMBOL_COPY
#define EG_SYMBOL_COPY            "\xEF\x83\x85" /*61637, 0xF0C5*/
#endif

#if !defined EG_SYMBOL_SAVE
#define EG_SYMBOL_SAVE            "\xEF\x83\x87" /*61639, 0xF0C7*/
#endif

#if !defined EG_SYMBOL_BARS
#define EG_SYMBOL_BARS            "\xEF\x83\x89" /*61641, 0xF0C9*/
#endif

#if !defined EG_SYMBOL_ENVELOPE
#define EG_SYMBOL_ENVELOPE        "\xEF\x83\xA0" /*61664, 0xF0E0*/
#endif

#if !defined EG_SYMBOL_CHARGE
#define EG_SYMBOL_CHARGE          "\xEF\x83\xA7" /*61671, 0xF0E7*/
#endif

#if !defined EG_SYMBOL_PASTE
#define EG_SYMBOL_PASTE           "\xEF\x83\xAA" /*61674, 0xF0EA*/
#endif

#if !defined EG_SYMBOL_BELL
#define EG_SYMBOL_BELL            "\xEF\x83\xB3" /*61683, 0xF0F3*/
#endif

#if !defined EG_SYMBOL_KEYBOARD
#define EG_SYMBOL_KEYBOARD        "\xEF\x84\x9C" /*61724, 0xF11C*/
#endif

#if !defined EG_SYMBOL_GPS
#define EG_SYMBOL_GPS             "\xEF\x84\xA4" /*61732, 0xF124*/
#endif

#if !defined EG_SYMBOL_FILE
#define EG_SYMBOL_FILE            "\xEF\x85\x9B" /*61787, 0xF158*/
#endif

#if !defined EG_SYMBOL_WIFI
#define EG_SYMBOL_WIFI            "\xEF\x87\xAB" /*61931, 0xF1EB*/
#endif

#if !defined EG_SYMBOL_BATTERY_FULL
#define EG_SYMBOL_BATTERY_FULL    "\xEF\x89\x80" /*62016, 0xF240*/
#endif

#if !defined EG_SYMBOL_BATTERY_3
#define EG_SYMBOL_BATTERY_3       "\xEF\x89\x81" /*62017, 0xF241*/
#endif

#if !defined EG_SYMBOL_BATTERY_2
#define EG_SYMBOL_BATTERY_2       "\xEF\x89\x82" /*62018, 0xF242*/
#endif

#if !defined EG_SYMBOL_BATTERY_1
#define EG_SYMBOL_BATTERY_1       "\xEF\x89\x83" /*62019, 0xF243*/
#endif

#if !defined EG_SYMBOL_BATTERY_EMPTY
#define EG_SYMBOL_BATTERY_EMPTY   "\xEF\x89\x84" /*62020, 0xF244*/
#endif

#if !defined EG_SYMBOL_USB
#define EG_SYMBOL_USB             "\xEF\x8a\x87" /*62087, 0xF287*/
#endif

#if !defined EG_SYMBOL_BLUETOOTH
#define EG_SYMBOL_BLUETOOTH       "\xEF\x8a\x93" /*62099, 0xF293*/
#endif

#if !defined EG_SYMBOL_TRASH
#define EG_SYMBOL_TRASH           "\xEF\x8B\xAD" /*62189, 0xF2ED*/
#endif

#if !defined EG_SYMBOL_EDIT
#define EG_SYMBOL_EDIT            "\xEF\x8C\x84" /*62212, 0xF304*/
#endif

#if !defined EG_SYMBOL_BACKSPACE
#define EG_SYMBOL_BACKSPACE       "\xEF\x95\x9A" /*62810, 0xF55A*/
#endif

#if !defined EG_SYMBOL_SD_CARD
#define EG_SYMBOL_SD_CARD         "\xEF\x9F\x82" /*63426, 0xF7C2*/
#endif

#if !defined EG_SYMBOL_NEW_LINE
#define EG_SYMBOL_NEW_LINE        "\xEF\xA2\xA2" /*63650, 0xF8A2*/
#endif

#if !defined EG_SYMBOL_DUMMY
/** Invalid symbol at (U+F8FF). If written before a string then `lv_img` will show it as a label*/
#define EG_SYMBOL_DUMMY           "\xEF\xA3\xBF"
#endif

/*
 * The following list is generated using
 * cat src/font/EG_SymbolDef.h | sed -E -n 's/^#define\s+EG_(SYMBOL_\w+).*".*$/    _EG_STR_\1,/p'
 */
enum {
    _EG_STR_SYMBOL_BULLET,
    _EG_STR_SYMBOL_AUDIO,
    _EG_STR_SYMBOL_VIDEO,
    _EG_STR_SYMBOL_LIST,
    _EG_STR_SYMBOL_OK,
    _EG_STR_SYMBOL_CLOSE,
    _EG_STR_SYMBOL_POWER,
    _EG_STR_SYMBOL_SETTINGS,
    _EG_STR_SYMBOL_HOME,
    _EG_STR_SYMBOL_DOWNLOAD,
    _EG_STR_SYMBOL_DRIVE,
    _EG_STR_SYMBOL_REFRESH,
    _EG_STR_SYMBOL_MUTE,
    _EG_STR_SYMBOL_VOLUME_MID,
    _EG_STR_SYMBOL_VOLUME_MAX,
    _EG_STR_SYMBOL_IMAGE,
    _EG_STR_SYMBOL_TINT,
    _EG_STR_SYMBOL_PREV,
    _EG_STR_SYMBOL_PLAY,
    _EG_STR_SYMBOL_PAUSE,
    _EG_STR_SYMBOL_STOP,
    _EG_STR_SYMBOL_NEXT,
    _EG_STR_SYMBOL_EJECT,
    _EG_STR_SYMBOL_LEFT,
    _EG_STR_SYMBOL_RIGHT,
    _EG_STR_SYMBOL_PLUS,
    _EG_STR_SYMBOL_MINUS,
    _EG_STR_SYMBOL_EYE_OPEN,
    _EG_STR_SYMBOL_EYE_CLOSE,
    _EG_STR_SYMBOL_WARNING,
    _EG_STR_SYMBOL_SHUFFLE,
    _EG_STR_SYMBOL_UP,
    _EG_STR_SYMBOL_DOWN,
    _EG_STR_SYMBOL_LOOP,
    _EG_STR_SYMBOL_DIRECTORY,
    _EG_STR_SYMBOL_UPLOAD,
    _EG_STR_SYMBOL_CALL,
    _EG_STR_SYMBOL_CUT,
    _EG_STR_SYMBOL_COPY,
    _EG_STR_SYMBOL_SAVE,
    _EG_STR_SYMBOL_BARS,
    _EG_STR_SYMBOL_ENVELOPE,
    _EG_STR_SYMBOL_CHARGE,
    _EG_STR_SYMBOL_PASTE,
    _EG_STR_SYMBOL_BELL,
    _EG_STR_SYMBOL_KEYBOARD,
    _EG_STR_SYMBOL_GPS,
    _EG_STR_SYMBOL_FILE,
    _EG_STR_SYMBOL_WIFI,
    _EG_STR_SYMBOL_BATTERY_FULL,
    _EG_STR_SYMBOL_BATTERY_3,
    _EG_STR_SYMBOL_BATTERY_2,
    _EG_STR_SYMBOL_BATTERY_1,
    _EG_STR_SYMBOL_BATTERY_EMPTY,
    _EG_STR_SYMBOL_USB,
    _EG_STR_SYMBOL_BLUETOOTH,
    _EG_STR_SYMBOL_TRASH,
    _EG_STR_SYMBOL_EDIT,
    _EG_STR_SYMBOL_BACKSPACE,
    _EG_STR_SYMBOL_SD_CARD,
    _EG_STR_SYMBOL_NEW_LINE,
    _EG_STR_SYMBOL_DUMMY,
};

