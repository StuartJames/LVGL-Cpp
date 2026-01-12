/**
 * @file EG_Conf.h
 * Configuration file for v8.3.11
 */

/*
 * Copy this file as `EG_Conf.h`
 * 1. simply next to the `EGL` folder
 * 2. or any other places and
 *    - define `EG_CONF_INCLUDE_SIMPLE`
 *    - add the path as include path
 */

/* clang-format off */
#if 0 /*Set it to "1" to enable content*/

#ifndef EG_CONF_H
#define EG_CONF_H

#include <stdint.h>

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
    #define EG_MEM_SIZE (48U * 1024U)          /*[bytes]*/

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
 *It removes the need to manually update the tick with `EG_IncrementTick()`)*/
#define EG_TICK_CUSTOM 0
#if EG_TICK_CUSTOM
    #define EG_TICK_CUSTOM_INCLUDE "Arduino.h"         /*Header for the system time function*/
    #define EG_TICK_CUSTOM_SYS_TIME_EXPR (millis())    /*Expression evaluating to current system time in ms*/
    /*If using lvgl as ESP32 component*/
    // #define EG_TICK_CUSTOM_INCLUDE "esp_timer.h"
    // #define EG_TICK_CUSTOM_SYS_TIME_EXPR ((esp_timer_get_time() / 1000LL))
#endif   /*EG_TICK_CUSTOM*/

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
 *This adds (sizeof(EG_Color_t) + 1) bytes per additional stop*/
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

/*Use Arm's 2D acceleration library Arm-2D */
#define EG_USE_GPU_ARM2D 0

/*Use STM32's DMA2D (aka Chrom Art) GPU*/
#define EG_USE_GPU_STM32_DMA2D 0
#if EG_USE_GPU_STM32_DMA2D
    /*Must be defined to include path of CMSIS header of target processor
    e.g. "stm32f7xx.h" or "stm32f4xx.h"*/
    #define EG_GPU_DMA2D_CMSIS_INCLUDE
#endif

/*Enable RA6M3 G2D GPU*/
#define EG_USE_GPU_RA6M3_G2D 0
#if EG_USE_GPU_RA6M3_G2D
    /*include path of target processor
    e.g. "hal_data.h"*/
    #define EG_GPU_RA6M3_G2D_INCLUDE "hal_data.h"
#endif

/*Use SWM341's DMA2D GPU*/
#define EG_USE_GPU_SWM341_DMA2D 0
#if EG_USE_GPU_SWM341_DMA2D
    #define EG_GPU_SWM341_DMA2D_INCLUDE "SWM341.h"
#endif

/*Use NXP's PXP GPU iMX RTxxx platforms*/
#define EG_USE_GPU_NXP_PXP 0
#if EG_USE_GPU_NXP_PXP
    /*1: Add default bare metal and FreeRTOS interrupt handling routines for PXP (lv_gpu_nxp_pxp_osa.c)
    *   and call lv_gpu_nxp_pxp_init() automatically during EG_Init(). Note that symbol SDK_OS_FREE_RTOS
    *   has to be defined in order to use FreeRTOS OSA, otherwise bare-metal implementation is selected.
    *0: lv_gpu_nxp_pxp_init() has to be called manually before EG_Init()
    */
    #define EG_USE_GPU_NXP_PXP_AUTO_INIT 0
#endif

/*Use NXP's VG-Lite GPU iMX RTxxx platforms*/
#define EG_USE_GPU_NXP_VG_LITE 0

/*Use SDL renderer API*/
#define EG_USE_GPU_SDL 0
#if EG_USE_GPU_SDL
    #define EG_GPU_SDL_INCLUDE_PATH <SDL2/SDL.h>
    /*Texture cache size, 8MB by default*/
    #define EG_GPU_SDL_LRU_SIZE (1024 * 1024 * 8)
    /*Custom blend mode for mask drawing, disable if you need to link with older SDL2 lib*/
    #define EG_GPU_SDL_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
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
    *0: User need to register a callback with `EG_RegisterLogPrintCB()`*/
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
#define EG_USE_ASSERT_MEM_INTEGRITY 0   /*Check the integrity of `EG_Memory` after critical operations. (Slow)*/
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
    #define eg_snprintf  snprintf
    #define eg_vsnprintf vsnprintf
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

/*Define a custom attribute to `EG_IncrementTick` function*/
#define EG_ATTRIBUTE_TICK_INC

/*Define a custom attribute to `EG_TimerHandler` function*/
#define EG_ATTRIBUTE_TIMER_HANDLER

/*Define a custom attribute to `EG_DispFlushReady` function*/
#define EG_ATTRIBUTE_FLUSH_READY

/*Required alignment size for buffers*/
#define EG_ATTRIBUTE_MEM_ALIGN_SIZE 1

/*Will be added where memories needs to be aligned (with -Os data might not be aligned to boundary by default).
 * E.g. __attribute__((aligned(4)))*/
#define EG_ATTRIBUTE_MEM_ALIGN

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
#define EG_FONT_MONTSERRAT_12 0
#define EG_FONT_MONTSERRAT_14 1
#define EG_FONT_MONTSERRAT_16 0
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
#define EG_FONT_DEFAULT &EG_FontMontserrat14

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

#define EG_USE_DROPDOWN   1   /*Requires: EGLabel*/

#define EG_USE_IMG        1   /*Requires: EGLabel*/

#define EG_USE_LABEL      1
#if EG_USE_LABEL
    #define EG_LABEL_TEXT_SELECTION 1 /*Enable selecting text of the label*/
    #define EG_LABEL_LONG_TXT_HINT 1  /*Store some extra info in labels to speed up drawing of very long texts*/
#endif

#define EG_USE_LINE       1

#define EG_USE_ROLLER     1   /*Requires: EGLabel*/
#if EG_USE_ROLLER
    #define EG_ROLLER_INF_PAGES 7 /*Number of extra "pages" when the roller is infinite*/
#endif

#define EG_USE_SLIDER     1   /*Requires: EGBar*/

#define EG_USE_SWITCH     1

#define EG_USE_TEXTAREA   1   /*Requires: EGLabel*/
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
    #define EG_USE_CALENDAR_MONTH_HEADER 1
    #define EG_USE_CALENDAR_DROPDOWN_HEADER 1
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

/*PNG decoder library*/
#define EG_USE_PNG 0

/*BMP decoder library*/
#define EG_USE_BMP 0

/* JPG + split JPG decoder library.
 * Split JPG is a custom format optimized for embedded systems. */
#define EG_USE_SJPG 0

/*GIF decoder library*/
#define EG_USE_GIF 0

/*QR code library*/
#define EG_USE_QRCODE 0

/*FreeType library*/
#define EG_USE_FREETYPE 0
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
#define EG_USE_TINY_TTF 0
#if EG_USE_TINY_TTF
    /*Load TTF data from files*/
    #define EG_TINY_TTF_FILE_SUPPORT 0
#endif

/*Rlottie library*/
#define EG_USE_RLOTTIE 0

/*FFmpeg library for image decoding and playing videos
 *Supports all major image formats so do not enable other image decoder with it*/
#define EG_USE_FFMPEG 0
#if EG_USE_FFMPEG
    /*Dump input information to stderr*/
    #define EG_FFMPEG_DUMP_FORMAT 0
#endif

/*-----------
 * Others
 *----------*/

/*1: Enable API to take snapshot for object*/
#define EG_USE_SNAPSHOT 0

/*1: Enable Monkey test*/
#define EG_USE_MONKEY 0

/*1: Enable grid navigation*/
#define EG_USE_GRIDNAV 0

/*1: Enable EGObject fragment*/
#define EG_USE_FRAGMENT 0

/*1: Support using images as font in label or span widgets */
#define EG_USE_IMGFONT 0

/*1: Enable a published subscriber based messaging system */
#define EG_USE_MSG 0

/*1: Enable Pinyin input method*/
/*Requires: EGKeyboard*/
#define EG_USE_IME_PINYIN 0
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
#define EG_USE_DEMO_WIDGETS 0
#if EG_USE_DEMO_WIDGETS
#define EG_DEMO_WIDGETS_SLIDESHOW 0
#endif

/*Demonstrate the usage of encoder and keyboard*/
#define EG_USE_DEMO_KEYPAD_AND_ENCODER 0

/*Benchmark your system*/
#define EG_USE_DEMO_BENCHMARK 0
#if EG_USE_DEMO_BENCHMARK
/*Use RGB565A8 images with 16 bit color depth instead of ARGB8565*/
#define EG_DEMO_BENCHMARK_RGB565A8 0
#endif

/*Stress test for LVGL*/
#define EG_USE_DEMO_STRESS 0

/*Music player demo*/
#define EG_USE_DEMO_MUSIC 0
#if EG_USE_DEMO_MUSIC
    #define EG_DEMO_MUSIC_SQUARE    0
    #define EG_DEMO_MUSIC_LANDSCAPE 0
    #define EG_DEMO_MUSIC_ROUND     0
    #define EG_DEMO_MUSIC_LARGE     0
    #define EG_DEMO_MUSIC_AUTO_PLAY 0
#endif

/*--END OF EG_CONF_H--*/

#endif /*EG_CONF_H*/

#endif /*End of "Content enable"*/
