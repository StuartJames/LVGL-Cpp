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

#include "EG_ImageDecoder.h"
#include "EG_ImageBuffer.h"
#include "EG_ImageCache.h"
#include "../misc/EG_Style.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDrawContext;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDrawImage
{
public:
                      EGDrawImage(void);
  void                Draw(const EGDrawContext *pContext, const EGRect *pRect, const void *pSource);
  void                DrawDecoded(const EGRect *pRect, const uint8_t *pMap, EG_ImageColorFormat_t ColorFormat);
  bool                IsChromaKeyed(EG_ImageColorFormat_t ColorFormat);

  static EG_ImageSource_t GetType(const void *pSource);
  static uint8_t      GetPixelSize(EG_ImageColorFormat_t ColorFormat);
  static bool         HasAlpha(EG_ImageColorFormat_t ColorFormat);

  const EGDrawContext *m_pContext;
  int16_t             m_Angle;
  uint16_t            m_Zoom;
  EGPoint             m_Pivot;
  EG_Color_t          m_Recolor;
  EG_OPA_t            m_RecolorOPA;
  EG_OPA_t            m_OPA;
  EG_BlendMode_e      m_BlendMode : 4;
  int32_t             m_FrameID;
  uint8_t             m_Antialias : 1;

private:
  EG_Result_t         PreDraw(const EGRect *pRect, const void *pSource);
  void                ShowError(const EGRect *pRect, const char *pMsg);
  void                DrawCleanup(ImageCacheEntry_t *pCache);

};







