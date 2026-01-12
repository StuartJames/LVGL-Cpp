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

#pragma once

#include "../EG_IntrnlConfig.h"
#include <stdint.h>

#include "EG_Types.h"


// Possible log level. For compatibility declare it independently from `EG_USE_LOG`

#define EG_LOG_LEVEL_TRACE 0 // A lot of logs to give detailed information
#define EG_LOG_LEVEL_INFO  1 // Log important events
#define EG_LOG_LEVEL_WARN  2 // Log if something unwanted happened but didn't caused problem
#define EG_LOG_LEVEL_ERROR 3 // Only critical issue, when the system may fail
#define EG_LOG_LEVEL_USER  4 // Custom logs from the user
#define EG_LOG_LEVEL_NONE  5 // Do not log anything
#define _EG_LOG_LEVEL_NUM  6 // Number of log levels

EG_EXPORT_CONST_INT(EG_LOG_LEVEL_TRACE);
EG_EXPORT_CONST_INT(EG_LOG_LEVEL_INFO);
EG_EXPORT_CONST_INT(EG_LOG_LEVEL_WARN);
EG_EXPORT_CONST_INT(EG_LOG_LEVEL_ERROR);
EG_EXPORT_CONST_INT(EG_LOG_LEVEL_USER);
EG_EXPORT_CONST_INT(EG_LOG_LEVEL_NONE);

typedef int8_t EG_LogLevel_t;

#if EG_USE_LOG

/////////////////////////////////////////////////////////////////////////////////

// Log print function. Receives a string buffer to print".
typedef void (*EG_LogPrintCB_t)(const char * buf);

/////////////////////////////////////////////////////////////////////////////////

/**
 * Register custom print/write function to call when a log is added.
 * It can format its "File path", "Line number" and "Description" as required
 * and send the formatted log message to a console or serial port.
 * @param           print_cb a function pointer to print a log
 */
void EG_RegisterLogPrintCB(EG_LogPrintCB_t print_cb);

/**
 * Print a log message via `printf` if enabled with `EG_LOG_PRINTF` in `EG_Config.h`
 * and/or a print callback if registered with `EG_RegisterLogPrintCB`
 * @param format    printf-like format string
 * @param ...       parameters for `format`
 */
void EG_Log(const char * format, ...) EG_FORMAT_ATTRIBUTE(1, 2);

/**
 * Add a log
 * @param level     the level of log. (From `EG_LogLevel_t` enum)
 * @param file      name of the file when the log added
 * @param line      line number in the source code where the log added
 * @param func      name of the function when the log added
 * @param format    printf-like format string
 * @param ...       parameters for `format`
 */
void EG_LOG_ADD(EG_LogLevel_t level, const char * file, int line,
                 const char * func, const char * format, ...) EG_FORMAT_ATTRIBUTE(5, 6);

/////////////////////////////////////////////////////////////////////////////////

#ifndef EG_LOG_TRACE
#  if EG_LOG_LEVEL <= EG_LOG_LEVEL_TRACE
#    define EG_LOG_TRACE(...) EG_LOG_ADD(EG_LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#  else
#    define EG_LOG_TRACE(...) do {}while(0)
#  endif
#endif

#ifndef EG_LOG_INFO
#  if EG_LOG_LEVEL <= EG_LOG_LEVEL_INFO
#    define EG_LOG_INFO(...) EG_LOG_ADD(EG_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#  else
#    define EG_LOG_INFO(...) do {}while(0)
#  endif
#endif

#ifndef EG_LOG_WARN
#  if EG_LOG_LEVEL <= EG_LOG_LEVEL_WARN
#    define EG_LOG_WARN(...) EG_LOG_ADD(EG_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#  else
#    define EG_LOG_WARN(...) do {}while(0)
#  endif
#endif

#ifndef EG_LOG_ERROR
#  if EG_LOG_LEVEL <= EG_LOG_LEVEL_ERROR
#    define EG_LOG_ERROR(...) EG_LOG_ADD(EG_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#  else
#    define EG_LOG_ERROR(...) do {}while(0)
#  endif
#endif

#ifndef EG_LOG_USER
#  if EG_LOG_LEVEL <= EG_LOG_LEVEL_USER
#    define EG_LOG_USER(...) EG_LOG_ADD(EG_LOG_LEVEL_USER, __FILE__, __LINE__, __func__, __VA_ARGS__)
#  else
#    define EG_LOG_USER(...) do {}while(0)
#  endif
#endif

#ifndef EG_LOG
#  if EG_LOG_LEVEL < EG_LOG_LEVEL_NONE
#    define EG_LOG(...) EG_Log(__VA_ARGS__)
#  else
#    define EG_LOG(...) do {} while(0)
#  endif
#endif

#else 

/*Do nothing if `EG_USE_LOG 0`*/
#define EG_LOG_ADD(level, file, line, ...)
#define EG_LOG_TRACE(...) do {}while(0)
#define EG_LOG_INFO(...) do {}while(0)
#define EG_LOG_WARN(...) do {}while(0)
#define EG_LOG_ERROR(...) do {}while(0)
#define EG_LOG_USER(...) do {}while(0)
#define EG_LOG(...) do {}while(0)

#endif 

