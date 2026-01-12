/**
 * @file lv_fsdrv.h
 *
 */

#pragma once

#include "EG_IntrnlConfig.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

#if EG_USE_FS_FATFS != '\0'
void lv_fs_fatfs_init(void);
#endif

#if EG_USE_FS_LITTLEFS != '\0'
void lv_fs_littlefs_init(void);
EG_FS_Driver_t * lv_fs_littlefs_set_driver(char label, void * lfs_p);
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

