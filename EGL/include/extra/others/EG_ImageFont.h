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

#include "EGL.h"

/////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_IMGFONT

#define EG_IMAGEFONT_PATH_MAX_LEN 64

/////////////////////////////////////////////////////////////////////////////////////

class EGImageFont
{
public:
                          EGImageFont(void);
                          ~EGImageFont(void);
  EG_Font_t*              Create(uint16_t Height);
  virtual bool            GetPath(const EG_Font_t *pFont, void *pSource, uint16_t Len, uint32_t Unicode, uint32_t UnicodeNext) = 0;
  static  bool            GetGlyphDiscriptor(const EG_Font_t *pFont, EG_FontGlyphProps_t *pDiscriptor,	uint32_t Unicode, uint32_t UnicodeNext);
  static const uint8_t*   GetGlyphBitmap(const EG_Font_t *pFont, uint32_t Unicode);


  EG_Font_t              *m_pFont;
	char                    m_Path[EG_IMAGEFONT_PATH_MAX_LEN];
};

#endif 