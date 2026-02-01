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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "EGL.h"


void EG_InitialiseExtra(void)
{

#if EG_USE_FLEX
  EGFlexLayout::Initialise();
#endif

#if EG_USE_GRID
  EGGridLayout::Initialise();
#endif


#if EG_USE_MSG
	EGMessageExec::Initialise();
#endif

#if EG_USE_FS_FATFS != '\0'
	EG_FSInitialise_FAT();
#endif

#if EG_USE_FS_LITTLEFS != '\0'
	EG_FSInitialise_LittleFS();
#endif

#if EG_USE_FS_STDIO != '\0'
	EG_FSInitialise_Stdio();
#endif

#if EG_USE_FS_POSIX != '\0'
	EG_FSInitialise_Posix();
#endif

#if EG_USE_FS_WIN32 != '\0'
	EG_FSInitialise_Win32();
#endif

#if EG_USE_FFMPEG
	EGImageDecoder::Register(&DecoderMPEG);
#endif

#if EG_USE_PNG
  EGImageDecoder::Register(&DecoderPNG);
#endif

#if EG_USE_SJPG
  EGImageDecoder::Register(&DecoderSJPG);
#endif

#if EG_USE_BMP
  EGImageDecoder::Register(&DecoderBMP);
#endif

#if EG_USE_FREETYPE
/*Init freetype library*/
#if EG_FREETYPE_CACHE_SIZE >= 0
	lv_freetype_init(EG_FREETYPE_CACHE_FT_FACES, EG_FREETYPE_CACHE_FT_SIZES, EG_FREETYPE_CACHE_SIZE);
#else
	lv_freetype_init(0, 0, 0);
#endif
#endif
}
