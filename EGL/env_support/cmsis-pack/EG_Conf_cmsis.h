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

#if 1 // Set it to "1" to enable content

#ifndef EG_CONF_H
#define EG_CONF_H

#include <stdint.h>

#if defined(_RTE_)
    #include "RTE_Components.h"
#endif

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define EG_COLOR_DEPTH 16

/*Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI)*/
#define EG_COLOR_16_SWAP 0

/*Enable features to draw on transparent background.
 *It's required if opa, and transform_* style properties are used.
 *Can be also used if the UI is above another layer, e.g. an OSD menu or video player.*/
#define EG_COLOR_SCREEN_TRANSP 0

/* Adjust color mix functions rounding. GPUs might calculate color mix (blending) differently.
 * 0: round down, 64: round up from x.75, 128: round up from half, 192: round up from x.25, 254: round up */
#define EG_COLOR_MIX_ROUND_OFS 0

/*Images pixels with this color will not be drawn if they are chroma keyed)*/
#define EG_COLOR_CHROMA_KEY EG_ColorHex(0x00ff00)         /*pure green*/

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `EG_AllocMem()` and `EG_FreeMem()`*/
#define EG_MEM_CUSTOM 0
#if EG_MEM_CUSTOM == 0
    /*Size of the memory available for `EG_AllocMem()` in bytes (>= 2kB)*/
    #define EG_MEM_SIZE (64U * 1024U)          /*[bytes]*/

    /*Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too.*/
    #define EG_MEM_ADR 0     /*0: unused*/
    /*Instead of an address give a memory allocator that will be called to get a memory pool for LVGL. E.g. my_malloc*/
    #if EG_MEM_ADR == 0
        #undef EG_MEM_POOL_INCLUDE
        #undef EG_MEM_POOL_ALLOC
    #endif

#else       /*EG_MEM_CUSTOM*/
    #define EG_MEM_CUSTOM_INCLUDE <stdlib.h>   /*Header for the dynamic memory function*/
    #define EG_MEM_CUSTOM_ALLOC   malloc
    #define EG_MEM_CUSTOM_FREE    free
    #define EG_MEM_CUSTOM_REALLOC realloc
#endif     /*EG_MEM_CUSTOM*/

/*Number of the intermediate memory buffer used during rendering and other internal processing mechanisms.
 *You will see an error log message if there wasn't enough buffers. */
#define EG_MEM_BUF_MAX_NUM 16

/*Use the standard `memcpy` and `memset` instead of LVGL's own functions. (Might or might not be faster).*/
#define EG_MEMCPY_MEMSET_STD 0

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh period. LVG will redraw changed areas with this period time*/
#define EG_DISP_DEF_REFR_PERIOD 30      /*[ms]*/

/*Input device read period in milliseconds*/
#define EG_INDEV_DEF_READ_PERIOD 30     /*[ms]*/


/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `lv_tick_inc()`)*/
#ifdef __PERF_COUNTER__
    #define EG_TICK_CUSTOM 1
    #if EG_TICK_CUSTOM
        extern uint32_t SystemCoreClock;
        #define EG_TICK_CUSTOM_INCLUDE          "perf_counter.h"
        #define EG_TICK_CUSTOM_SYS_TIME_EXPR    get_system_ms()
    #endif   /*EG_TICK_CUSTOM*/
#else
    #define EG_TICK_CUSTOM 0
    #if EG_TICK_CUSTOM
        #define EG_TICK_CUSTOM_INCLUDE "Arduino.h"         /*Header for the system time function*/
        #define EG_TICK_CUSTOM_SYS_TIME_EXPR (millis())    /*Expression evaluating to current system time in ms*/
    /*If using lvgl as ESP32 component*/
    // #define EG_TICK_CUSTOM_INCLUDE "esp_timer.h"
    // #define EG_TICK_CUSTOM_SYS_TIME_EXPR ((esp_timer_get_time() / 1000LL))
    #endif   /*EG_TICK_CUSTOM*/
#endif       /*__PERF_COUNTER__*/


/*Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 *(Not so important, you can adjust it to modify default sizes and spaces)*/
#define EG_DPI_DEF 130     /*[px/inch]*/

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/

/*-------------
 * Drawing
 *-----------*/

/*Enable complex draw engine.
 *Required to draw shadow, gradient, rounded corners, circles, arc, skew lines, image transformations or any masks*/
#define EG_DRAW_COMPLEX 1
#if EG_DRAW_COMPLEX != 0

    /*Allow buffering some shadow calculation.
    *EG_SHADOW_CACHE_SIZE is the max. shadow size to buffer, where shadow size is `shadow_width + radius`
    *Caching has EG_SHADOW_CACHE_SIZE^2 RAM cost*/
    #define EG_SHADOW_CACHE_SIZE 0

    /* Set number of maximally cached circle data.
    * The circumference of 1/4 circle are saved for anti-aliasing
    * radius * 4 bytes are used per circle (the most often used radiuses are saved)
    * 0: to disable caching */
    #define EG_CIRCLE_CACHE_SIZE 4
#endif /*EG_DRAW_COMPLEX*/

/**
 * "Simple layers" are used when a widget has `style_opa < 255` to buffer the widget into a layer
 * and blend it as an image with the given opacity.
 * Note that `bg_opa`, `text_opa` etc don't require buffering into layer)
 * The widget can be buffered in smaller chunks to avoid using large buffers.
 *
 * - EG_LAYER_SIMPLE_BUF_SIZE: [bytes] the optimal target buffer size. LVGL will try to allocate it
 * - EG_LAYER_SIMPLE_FALLBACK_BUF_SIZE: [bytes]  used if `EG_LAYER_SIMPLE_BUF_SIZE` couldn't be allocated.
 *
 * Both buffer sizes are in bytes.
 * "Transformed layers" (where transform_angle/zoom properties are used) use larger buffers
 * and can't be drawn in chunks. So these settings affects only widgets with opacity.
 */
#define EG_LAYER_SIMPLE_BUF_SIZE          (24 * 1024)
#define EG_LAYER_SIMPLE_FALLBACK_BUF_SIZE (3 * 1024)

/*Default image cache size. Image caching keeps the images opened.
 *If only the built-in image formats are used there is no real advantage of caching. (I.e. if no new image decoder is added)
 *With complex image decoders (e.g. PNG or JPG) caching can save the continuous open/decode of images.
 *However the opened images might consume additional RAM.
 *0: to disable caching*/
#define EG_IMG_CACHE_DEF_SIZE 0

/*Number of stops allowed per gradient. Increase this to allow more stops.
 *This adds (sizeof(lv_color_t) + 1) bytes per additional stop*/
#define EG_GRADIENT_MAX_STOPS 2

/*Default gradient buffer size.
 *When LVGL calculates the gradient "maps" it can save them into a cache to avoid calculating them again.
 *EG_GRAD_CACHE_DEF_SIZE sets the size of this cache in bytes.
 *If the cache is too small the map will be allocated only while it's required for the drawing.
 *0 mean no caching.*/
#define EG_GRAD_CACHE_DEF_SIZE 0

/*Allow dithering the gradients (to achieve visual smooth color gradients on limited color depth display)
 *EG_DITHER_GRADIENT implies allocating one or two more lines of the object's rendering surface
 *The increase in memory consumption is (32 bits * object width) plus 24 bits * object width if using error diffusion */
#define EG_DITHER_GRADIENT 0
#if EG_DITHER_GRADIENT
    /*Add support for error diffusion dithering.
     *Error diffusion dithering gets a much better visual result, but implies more CPU consumption and memory when drawing.
     *The increase in memory consumption is (24 bits * object's width)*/
    #define EG_DITHER_ERROR_DIFFUSION 0
#endif

/*Maximum buffer size to allocate for rotation.
 *Only used if software rotation is enabled in the display driver.*/
#define EG_DISP_ROT_MAX_BUF (10*1024)

/*-------------
 * GPU
 *-----------*/


/*Use STM32's DMA2D (aka Chrom Art) GPU*/
#if EG_USE_GPU_STM32_DMA2D
    /*Must be defined to include path of CMSIS header of target processor
    e.g. "stm32f7xx.h" or "stm32f4xx.h"*/
    #define EG_GPU_DMA2D_CMSIS_INCLUDE
#endif

/*Enable RA6M3 G2D GPU*/
#if EG_USE_GPU_RA6M3_G2D
    /*include path of target processor
    e.g. "hal_data.h"*/
    #define EG_GPU_RA6M3_G2D_INCLUDE "hal_data.h"
#endif

/*Use SWM341's DMA2D GPU*/
#if EG_USE_GPU_SWM341_DMA2D
    #define EG_GPU_SWM341_DMA2D_INCLUDE "SWM341.h"
#endif

/*Use NXP's PXP GPU iMX RTxxx platforms*/
#if EG_USE_GPU_NXP_PXP
    /*1: Add default bare metal and FreeRTOS interrupt handling routines for PXP (lv_gpu_nxp_pxp_osa.c)
    *   and call lv_gpu_nxp_pxp_init() automatically during lv_init(). Note that symbol SDK_OS_FREE_RTOS
    *   has to be defined in order to use FreeRTOS OSA, otherwise bare-metal implementation is selected.
    *0: lv_gpu_nxp_pxp_init() has to be called manually before lv_init()
    */
    #define EG_USE_GPU_NXP_PXP_AUTO_INIT 0
#endif

/*-------------
 * Logging
 *-----------*/

/*Enable the log module*/
#define EG_USE_LOG 0
#if EG_USE_LOG

    /*How important log should be added:
    *EG_LOG_LEVEL_TRACE       A lot of logs to give detailed information
    *EG_LOG_LEVEL_INFO        Log important events
    *EG_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
    *EG_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
    *EG_LOG_LEVEL_USER        Only logs added by the user
    *EG_LOG_LEVEL_NONE        Do not log anything*/
    #define EG_LOG_LEVEL EG_LOG_LEVEL_WARN

    /*1: Print the log with 'printf';
    *0: User need to register a callback with `lv_log_register_print_cb()`*/
    #define EG_LOG_PRINTF 0

    /*Enable/disable EG_LOG_TRACE in modules that produces a huge number of logs*/
    #define EG_LOG_TRACE_MEM        1
    #define EG_LOG_TRACE_TIMER      1
    #define EG_LOG_TRACE_INDEV      1
    #define EG_LOG_TRACE_DISP_REFR  1
    #define EG_LOG_TRACE_EVENT      1
    #define EG_LOG_TRACE_OBJ_CREATE 1
    #define EG_LOG_TRACE_LAYOUT     1
    #define EG_LOG_TRACE_ANIM       1

#endif  /*EG_USE_LOG*/

/*-------------
 * Asserts
 *-----------*/

/*Enable asserts if an operation is failed or an invalid data is found.
 *If EG_USE_LOG is enabled an error message will be printed on failure*/
#define EG_USE_ASSERT_NULL          1   /*Check if the parameter is NULL. (Very fast, recommended)*/
#define EG_USE_ASSERT_MALLOC        1   /*Checks is the memory is successfully allocated or no. (Very fast, recommended)*/
#define EG_USE_ASSERT_STYLE         0   /*Check if the styles are properly initialized. (Very fast, recommended)*/
#define EG_USE_ASSERT_MEM_INTEGRITY 0   /*Check the integrity of `lv_mem` after critical operations. (Slow)*/
#define EG_USE_ASSERT_OBJ           0   /*Check the object's type and existence (e.g. not deleted). (Slow)*/

/*Add a custom handler when assert happens e.g. to restart the MCU*/
#define EG_ASSERT_HANDLER_INCLUDE <stdint.h>
#define EG_ASSERT_HANDLER while(1);   /*Halt by default*/

/*-------------
 * Others
 *-----------*/

/*1: Show CPU usage and FPS count*/
#define EG_USE_PERF_MONITOR 0
#if EG_USE_PERF_MONITOR
    #define EG_USE_PERF_MONITOR_POS EG_ALIGN_BOTTOM_RIGHT
#endif

/*1: Show the used memory and the memory fragmentation
 * Requires EG_MEM_CUSTOM = 0*/
#define EG_USE_MEM_MONITOR 0
#if EG_USE_MEM_MONITOR
    #define EG_USE_MEM_MONITOR_POS EG_ALIGN_BOTTOM_LEFT
#endif

/*1: Draw random colored rectangles over the redrawn areas*/
#define EG_USE_REFR_DEBUG 0

/*Change the built in (v)snprintf functions*/
#define EG_SPRINTF_CUSTOM 0
#if EG_SPRINTF_CUSTOM
    #define EG_SPRINTF_INCLUDE <stdio.h>
    #define lv_snprintf  snprintf
    #define lv_vsnprintf vsnprintf
#else   /*EG_SPRINTF_CUSTOM*/
    #define EG_SPRINTF_USE_FLOAT 0
#endif  /*EG_SPRINTF_CUSTOM*/

#define EG_USE_USER_DATA 1

/*Garbage Collector settings
 *Used if lvgl is bound to higher level language and the memory is managed by that language*/
#define EG_ENABLE_GC 0
#if EG_ENABLE_GC != 0
    #define EG_GC_INCLUDE "gc.h"                           /*Include Garbage Collector related things*/
#endif /*EG_ENABLE_GC*/

/*=====================
 *  COMPILER SETTINGS
 *====================*/

/*For big endian systems set to 1*/
#define EG_BIG_ENDIAN_SYSTEM 0

/*Define a custom attribute to `lv_tick_inc` function*/
#define EG_ATTRIBUTE_TICK_INC

/*Define a custom attribute to `lv_timer_handler` function*/
#define EG_ATTRIBUTE_TIMER_HANDLER

/*Define a custom attribute to `lv_disp_flush_ready` function*/
#define EG_ATTRIBUTE_FLUSH_READY

/*Required alignment size for buffers*/
#define EG_ATTRIBUTE_MEM_ALIGN_SIZE 4

/*Will be added where memories needs to be aligned (with -Os data might not be aligned to boundary by default).
 * E.g. __attribute__((aligned(4)))*/
#define EG_ATTRIBUTE_MEM_ALIGN __attribute__((aligned(4)))

/*Attribute to mark large constant arrays for example font's bitmaps*/
#define EG_ATTRIBUTE_LARGE_CONST

/*Compiler prefix for a big array declaration in RAM*/
#define EG_ATTRIBUTE_LARGE_RAM_ARRAY

/*Place performance critical functions into a faster memory (e.g RAM)*/
#define EG_ATTRIBUTE_FAST_MEM

/*Prefix variables that are used in GPU accelerated operations, often these need to be placed in RAM sections that are DMA accessible*/
#define EG_ATTRIBUTE_DMA

/*Export integer constant to binding. This macro is used with constants in the form of EG_<CONST> that
 *should also appear on LVGL binding API such as Micropython.*/
#define EG_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*The default value just prevents GCC warning*/

/*Extend the default -32k..32k coordinate range to -4M..4M by using int32_t for coordinates instead of int16_t*/
#define EG_USE_LARGE_COORD 0

/*==================
 *   FONT USAGE
 *===================*/

/*Montserrat fonts with ASCII range and some symbols using bpp = 4
 *https://fonts.google.com/specimen/Montserrat*/
#define EG_FONT_MONTSERRAT_8  0
#define EG_FONT_MONTSERRAT_10 0
#define EG_FONT_MONTSERRAT_12 1
#define EG_FONT_MONTSERRAT_14 1
#define EG_FONT_MONTSERRAT_16 1
#define EG_FONT_MONTSERRAT_18 0
#define EG_FONT_MONTSERRAT_20 0
#define EG_FONT_MONTSERRAT_22 0
#define EG_FONT_MONTSERRAT_24 0
#define EG_FONT_MONTSERRAT_26 0
#define EG_FONT_MONTSERRAT_28 0
#define EG_FONT_MONTSERRAT_30 0
#define EG_FONT_MONTSERRAT_32 0
#define EG_FONT_MONTSERRAT_34 0
#define EG_FONT_MONTSERRAT_36 0
#define EG_FONT_MONTSERRAT_38 0
#define EG_FONT_MONTSERRAT_40 0
#define EG_FONT_MONTSERRAT_42 0
#define EG_FONT_MONTSERRAT_44 0
#define EG_FONT_MONTSERRAT_46 0
#define EG_FONT_MONTSERRAT_48 0

/*Demonstrate special features*/
#define EG_FONT_MONTSERRAT_12_SUBPX      0
#define EG_FONT_MONTSERRAT_28_COMPRESSED 0  /*bpp = 3*/
#define EG_FONT_DEJAVU_16_PERSIAN_HEBREW 0  /*Hebrew, Arabic, Persian letters and all their forms*/
#define EG_FONT_SIMSUN_16_CJK            0  /*1000 most common CJK radicals*/

/*Pixel perfect monospace fonts*/
#define EG_FONT_UNSCII_8  0
#define EG_FONT_UNSCII_16 0

/*Optionally declare custom fonts here.
 *You can use these fonts as default font too and they will be available globally.
 *E.g. #define EG_FONT_CUSTOM_DECLARE   EG_FONT_DECLARE(my_font_1) EG_FONT_DECLARE(my_font_2)*/
#define EG_FONT_CUSTOM_DECLARE

/*Always set a default font*/
#define EG_FONT_DEFAULT &lv_font_montserrat_14

/*Enable handling large font and/or fonts with a lot of characters.
 *The limit depends on the font size, font face and bpp.
 *Compiler error will be triggered if a font needs it.*/
#define EG_FONT_FMT_TXT_LARGE 0

/*Enables/disables support for compressed fonts.*/
#define EG_USE_FONT_COMPRESSED 0

/*Enable subpixel rendering*/
#define EG_USE_FONT_SUBPX 0
#if EG_USE_FONT_SUBPX
    /*Set the pixel order of the display. Physical order of RGB channels. Doesn't matter with "normal" fonts.*/
    #define EG_FONT_SUBPX_BGR 0  /*0: RGB; 1:BGR order*/
#endif

/*Enable drawing placeholders when glyph dsc is not found*/
#define EG_USE_FONT_PLACEHOLDER 1

/*=================
 *  TEXT SETTINGS
 *=================*/

/**
 * Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - EG_TXT_ENC_UTF8
 * - EG_TXT_ENC_ASCII
 */
#define EG_TXT_ENC EG_TXT_ENC_UTF8

/*Can break (wrap) texts on these chars*/
#define EG_TXT_BREAK_CHARS " ,.;:-_"

/*If a word is at least this long, will break wherever "prettiest"
 *To disable, set to a value <= 0*/
#define EG_TXT_LINE_BREAK_LONG_LEN 0

/*Minimum number of characters in a long word to put on a line before a break.
 *Depends on EG_TXT_LINE_BREAK_LONG_LEN.*/
#define EG_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/*Minimum number of characters in a long word to put on a line after a break.
 *Depends on EG_TXT_LINE_BREAK_LONG_LEN.*/
#define EG_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*The control character to use for signalling text recoloring.*/
#define EG_TXT_COLOR_CMD "#"

/*Support bidirectional texts. Allows mixing Left-to-Right and Right-to-Left texts.
 *The direction will be processed according to the Unicode Bidirectional Algorithm:
 *https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
#define EG_USE_BIDI 0
#if EG_USE_BIDI
    /*Set the default direction. Supported values:
    *`EG_BASE_DIR_LTR` Left-to-Right
    *`EG_BASE_DIR_RTL` Right-to-Left
    *`EG_BASE_DIR_AUTO` detect texts base direction*/
    #define EG_BIDI_BASE_DIR_DEF EG_BASE_DIR_AUTO
#endif

/*Enable Arabic/Persian processing
 *In these languages characters should be replaced with an other form based on their position in the text*/
#define EG_USE_ARABIC_PERSIAN_CHARS 0

/*==================
 *  WIDGET USAGE
 *================*/

/*Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html*/

#define EG_USE_ARC        1

#define EG_USE_BAR        1

#define EG_USE_BTN        1

#define EG_USE_BTNMATRIX  1

#define EG_USE_CANVAS     1

#define EG_USE_CHECKBOX   1

#define EG_USE_DROPDOWN   1   /*Requires: lv_label*/

#define EG_USE_IMG        1   /*Requires: lv_label*/

#define EG_USE_LABEL      1
#if EG_USE_LABEL
    #define EG_LABEL_TEXT_SELECTION 1 /*Enable selecting text of the label*/
    #define EG_LABEL_LONG_TXT_HINT 1  /*Store some extra info in labels to speed up drawing of very long texts*/
#endif

#define EG_USE_LINE       1

#define EG_USE_ROLLER     1   /*Requires: lv_label*/
#if EG_USE_ROLLER
    #define EG_ROLLER_INF_PAGES 7 /*Number of extra "pages" when the roller is infinite*/
#endif

#define EG_USE_SLIDER     1   /*Requires: lv_bar*/

#define EG_USE_SWITCH     1

#define EG_USE_TEXTAREA   1   /*Requires: lv_label*/
#if EG_USE_TEXTAREA != 0
    #define EG_TEXTAREA_DEF_PWD_SHOW_TIME 1500    /*ms*/
#endif

#define EG_USE_TABLE      1

/*==================
 * EXTRA COMPONENTS
 *==================*/

/*-----------
 * Widgets
 *----------*/
#define EG_USE_ANIMIMG    1

#define EG_USE_CALENDAR   1
#if EG_USE_CALENDAR
    #define EG_CALENDAR_WEEK_STARTS_MONDAY 0
    #if EG_CALENDAR_WEEK_STARTS_MONDAY
        #define EG_CALENDAR_DEFAULT_DAY_NAMES {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}
    #else
        #define EG_CALENDAR_DEFAULT_DAY_NAMES {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"}
    #endif

    #define EG_CALENDAR_DEFAULT_MONTH_NAMES {"January", "February", "March",  "April", "May",  "June", "July", "August", "September", "October", "November", "December"}
    #define EG_USE_CALENDAR_HEADER_ARROW 1
    #define EG_USE_CALENDAR_HEADER_DROPDOWN 1
#endif  /*EG_USE_CALENDAR*/

#define EG_USE_CHART      1

#define EG_USE_COLORWHEEL 1

#define EG_USE_IMGBTN     1

#define EG_USE_KEYBOARD   1

#define EG_USE_LED        1

#define EG_USE_LIST       1

#define EG_USE_MENU       1

#define EG_USE_METER      1

#define EG_USE_MSGBOX     1

#define EG_USE_SPAN       1
#if EG_USE_SPAN
    /*A line text can contain maximum num of span descriptor */
    #define EG_SPAN_SNIPPET_STACK_SIZE 64
#endif

#define EG_USE_SPINBOX    1

#define EG_USE_SPINNER    1

#define EG_USE_TABVIEW    1

#define EG_USE_TILEVIEW   1

#define EG_USE_WIN        1

/*-----------
 * Themes
 *----------*/

#ifdef RTE_GRAPHICS_EGL_USE_EXTRA_THEMES
    /*A simple, impressive and very complete theme*/
    #define EG_USE_THEME_DEFAULT 1
    #if EG_USE_THEME_DEFAULT

        /*0: Light mode; 1: Dark mode*/
        #define EG_THEME_DEFAULT_DARK 0

        /*1: Enable grow on press*/
        #define EG_THEME_DEFAULT_GROW 1

        /*Default transition time in [ms]*/
        #define EG_THEME_DEFAULT_TRANSITION_TIME 80
    #endif /*EG_USE_THEME_DEFAULT*/

    /*A very simple theme that is a good starting point for a custom theme*/
    #define EG_USE_THEME_BASIC 1

    /*A theme designed for monochrome displays*/
    #define EG_USE_THEME_MONO 1
#else
    #define EG_USE_THEME_DEFAULT    0
    #define EG_USE_THEME_BASIC      0
    #define EG_USE_THEME_MONO       0
#endif
/*-----------
 * Layouts
 *----------*/

/*A layout similar to Flexbox in CSS.*/
#define EG_USE_FLEX 1

/*A layout similar to Grid in CSS.*/
#define EG_USE_GRID 1

/*---------------------
 * 3rd party libraries
 *--------------------*/

/*File system interfaces for common APIs */

/*API for fopen, fread, etc*/
#define EG_USE_FS_STDIO 0
#if EG_USE_FS_STDIO
    #define EG_FS_STDIO_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define EG_FS_STDIO_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define EG_FS_STDIO_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for open, read, etc*/
#define EG_USE_FS_POSIX 0
#if EG_USE_FS_POSIX
    #define EG_FS_POSIX_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define EG_FS_POSIX_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define EG_FS_POSIX_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for CreateFile, ReadFile, etc*/
#define EG_USE_FS_WIN32 0
#if EG_USE_FS_WIN32
    #define EG_FS_WIN32_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define EG_FS_WIN32_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define EG_FS_WIN32_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for FATFS (needs to be added separately). Uses f_open, f_read, etc*/
#define EG_USE_FS_FATFS 0
#if EG_USE_FS_FATFS
    #define EG_FS_FATFS_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define EG_FS_FATFS_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for LittleFS (library needs to be added separately). Uses lfs_file_open, lfs_file_read, etc*/
#define EG_USE_FS_LITTLEFS 0
#if EG_USE_FS_LITTLEFS
    #define EG_FS_LITTLEFS_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define EG_FS_LITTLEFS_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif



/*FreeType library*/
#if EG_USE_FREETYPE
    /*Memory used by FreeType to cache characters [bytes] (-1: no caching)*/
    #define EG_FREETYPE_CACHE_SIZE (16 * 1024)
    #if EG_FREETYPE_CACHE_SIZE >= 0
        /* 1: bitmap cache use the sbit cache, 0:bitmap cache use the image cache. */
        /* sbit cache:it is much more memory efficient for small bitmaps(font size < 256) */
        /* if font size >= 256, must be configured as image cache */
        #define EG_FREETYPE_SBIT_CACHE 0
        /* Maximum number of opened FT_Face/FT_Size objects managed by this cache instance. */
        /* (0:use system defaults) */
        #define EG_FREETYPE_CACHE_FT_FACES 0
        #define EG_FREETYPE_CACHE_FT_SIZES 0
    #endif
#endif

/*Tiny TTF library*/
#if EG_USE_TINY_TTF
    /*Load TTF data from files*/
    #define EG_TINY_TTF_FILE_SUPPORT 0
#endif


/*FFmpeg library for image decoding and playing videos
 *Supports all major image formats so do not enable other image decoder with it*/
#if EG_USE_FFMPEG
    /*Dump input information to stderr*/
    #define EG_FFMPEG_DUMP_FORMAT 0
#endif

/*-----------
 * Others
 *----------*/

/*1: Enable Pinyin input method*/
/*Requires: lv_keyboard*/
#if EG_USE_IME_PINYIN
    /*1: Use default thesaurus*/
    /*If you do not use the default thesaurus, be sure to use `lv_ime_pinyin` after setting the thesauruss*/
    #define EG_IME_PINYIN_USE_DEFAULT_DICT 1
    /*Set the maximum number of candidate panels that can be displayed*/
    /*This needs to be adjusted according to the size of the screen*/
    #define EG_IME_PINYIN_CAND_TEXT_NUM 6

    /*Use 9 key input(k9)*/
    #define EG_IME_PINYIN_USE_K9_MODE      1
    #if EG_IME_PINYIN_USE_K9_MODE == 1
        #define EG_IME_PINYIN_K9_CAND_TEXT_NUM 3
    #endif // EG_IME_PINYIN_USE_K9_MODE
#endif

/*==================
* EXAMPLES
*==================*/

/*Enable the examples to be built with the library*/
#define EG_BUILD_EXAMPLES 1

/*===================
 * DEMO USAGE
 ====================*/

/*Show some widget. It might be required to increase `EG_MEM_SIZE` */
#if EG_USE_DEMO_WIDGETS
    #define EG_DEMO_WIDGETS_SLIDESHOW 0
#endif

/*Benchmark your system*/
#if EG_USE_DEMO_BENCHMARK
/*Use RGB565A8 images with 16 bit color depth instead of ARGB8565*/
    #define EG_DEMO_BENCHMARK_RGB565A8 1
#endif

/*--END OF EG_CONF_H--*/

#endif /*EG_CONF_H*/

#endif /*End of "Content enable"*/
