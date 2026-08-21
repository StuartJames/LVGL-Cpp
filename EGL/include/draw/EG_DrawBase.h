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
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include "../misc/EG_Color.h"
#include "../misc/EG_Types.h"
#include "../misc/EG_Style.h"
#include "../core/EG_ObjClass.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGDeviceContext;
class EGObject;
class EGLayerContext;

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawBase
{
public:
                        EGDrawBase(void);
//  virtual void          Draw(const EGDeviceContext  *pDC) = 0;

  EGDeviceContext      *m_pContext;
  EGLayerContext       *m_pLayer;         // The target layer
  EGObject             *m_pObj;           // The widget for which draw descriptor was created
  uint32_t              m_Part;           // The widget part for which draw descriptor was created
  uint32_t              m_ID1;            // A widget type specific ID (e.g. table row index). See the docs of the given widget.
  uint32_t              m_ID2;            // A widget type specific ID (e.g. table column index). See the docs of the given widget.
  int16_t               m_ShadowOffsetX;  // Shadow offset in X
  int16_t               m_ShadowOffsetY;  // Shadow offset in Y
  EG_Color_t            m_ShadowColor;
  EG_OPA_t              m_ShadowOPA;
  int32_t               m_ShadowBlurRadius: 20;
  EG_BlurQuality_e      m_ShadowQuality : 8;
	int32_t               m_ShadowWidth;
	int32_t               m_ShadowSpread;
  void                 *m_pExtData;
};