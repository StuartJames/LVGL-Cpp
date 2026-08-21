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

#pragma once

#include "EG_DrawBase.h"
#include "EG_ImageDecoder.h"
#include "EG_ImageBuffer.h"
#include "EG_ImageCache.h"
#include "../misc/EG_Style.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDeviceContext;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDrawImage : public EGDrawBase
{
public:
                      EGDrawImage(void);
  void                Initialise(void);
  void                Draw(EGDeviceContext *pDC, const EGRect *pRect, const void *pSource);
  void                DrawDecoded(const EGRect *pRect, const uint8_t *pMap, EG_ImageColorFormat_t ColorFormat);
  bool                IsChromaKeyed(EG_ImageColorFormat_t ColorFormat) const;

  static EG_ImageSource_e GetType(const void *pSource);
  static uint8_t      GetPixelSize(EG_ImageColorFormat_t ColorFormat);
  static bool         HasAlpha(EG_ImageColorFormat_t ColorFormat);

  int16_t             m_Angle;
  EGScale             m_Scale;
  EGPoint             m_Pivot;
  EG_Color_t          m_Recolor;
  EG_OPA_t            m_RecolorOPA;
  EG_OPA_t            m_OPA;
  int32_t             m_FrameID;
  uint8_t             m_AntiAlias : 1;
  EG_BlendMode_e      m_BlendMode : 4;

private:
  EG_Result_t         PreDraw(const EGRect *pRect, const void *pSource);
  void                ShowError(const EGRect *pRect, const char *pMsg);
  void                DrawCleanup(ImageCacheEntry_t *pCache);

};







