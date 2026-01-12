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

#include "EG_IntrnlConfig.h"

///////////////////////////////////////////////////////////////////////////////

#if EG_USE_FS_FATFS != '\0'
void EG_FSInitialise_FAT(void);
#endif

#if EG_USE_FS_LITTLEFS != '\0'
void EG_FSInitialise_LittleFS(void);
//EGFileDriver* lv_fs_littlefs_set_driver(char label, void * lfs_p);
#endif

#if EG_USE_FS_STDIO != '\0'
void lv_fs_stdio_init(void);
#endif

#if EG_USE_FS_POSIX != '\0'
void lv_fs_posix_init(void);
#endif

#if EG_USE_FS_WIN32 != '\0'
void lv_fs_win32_init(void);
#endif

