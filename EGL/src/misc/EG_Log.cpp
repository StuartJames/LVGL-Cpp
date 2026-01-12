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

#include "misc/EG_Log.h"

#if EG_USE_LOG

#include <stdarg.h>
#include <string.h>
#include "misc/lv_printf.h"
#include "hal/EG_HALTick.h"

#if EG_LOG_PRINTF
#include <stdio.h>
#endif

/////////////////////////////////////////////////////////////////////////////////

static EG_LogPrintCB_t custom_print_cb;

/////////////////////////////////////////////////////////////////////////////////

/**
 * Register custom print/write function to call when a log is added.
 * It can format its "File path", "Line number" and "Description" as required
 * and send the formatted log message to a console or serial port.
 * @param print_cb a function pointer to print a log
 */
void EG_RegisterLogPrintCB(EG_LogPrintCB_t print_cb)
{
	custom_print_cb = print_cb;
}

/////////////////////////////////////////////////////////////////////////////////

/**
 * Add a log
 * @param level the level of log. (From `EG_LogLevel_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void EG_LOG_ADD(EG_LogLevel_t level, const char *file, int line, const char *func, const char *format, ...)
{
	if(level >= _EG_LOG_LEVEL_NUM) return; /*Invalid level*/

	static uint32_t last_log_time = 0;

	if(level >= EG_LOG_LEVEL) {
		va_list args;
		va_start(args, format);

		/*Use only the file name not the path*/
		size_t p;
		for(p = strlen(file); p > 0; p--) {
			if(file[p] == '/' || file[p] == '\\') {
				p++; /*Skip the slash*/
				break;
			}
		}

		uint32_t t = EG_GetTick();
		static const char *lvl_prefix[] = {"Trace", "Info", "Warn", "Error", "User"};

#if EG_LOG_PRINTF
		printf("[%s]\t(%" EG_PRId32 ".%03" EG_PRId32 ", +%" EG_PRId32 ")\t %s: ",
					 lvl_prefix[level], t / 1000, t % 1000, t - last_log_time, func);
		vprintf(format, args);
		printf(" \t(in %s line #%d)\n", &file[p], line);
#else
		if(custom_print_cb) {
			char buf[512];
#if EG_SPRINTF_CUSTOM
			char msg[256];
			eg_vsnprintf(msg, sizeof(msg), format, args);
			eg_snprintf(buf, sizeof(buf), "[%s]\t(%" EG_PRId32 ".%03" EG_PRId32 ", +%" EG_PRId32 ")\t %s: %s \t(in %s line #%d)\n",
									lvl_prefix[level], t / 1000, t % 1000, t - last_log_time, func, msg, &file[p], line);
#else
			eg_vaformat_t vaf = {format, &args};
			eg_snprintf(buf, sizeof(buf), "[%s]\t(%" EG_PRId32 ".%03" EG_PRId32 ", +%" EG_PRId32 ")\t %s: %pV \t(in %s line #%d)\n",
									lvl_prefix[level], t / 1000, t % 1000, t - last_log_time, func, (void *)&vaf, &file[p], line);
#endif
			custom_print_cb(buf);
		}
#endif

		last_log_time = t;
		va_end(args);
	}
}

/////////////////////////////////////////////////////////////////////////////////

void EG_Log(const char *format, ...)
{
	if(EG_LOG_LEVEL >= EG_LOG_LEVEL_NONE) return; /* disable log */

	va_list args;
	va_start(args, format);

#if EG_LOG_PRINTF
	vprintf(format, args);
#else
	if(custom_print_cb) {
		char buf[512];
#if EG_SPRINTF_CUSTOM
		eg_vsnprintf(buf, sizeof(buf), format, args);
#else
		eg_vaformat_t vaf = {format, &args};
		eg_snprintf(buf, sizeof(buf), "%pV", (void *)&vaf);
#endif
		custom_print_cb(buf);
	}
#endif

	va_end(args);
}


#endif
