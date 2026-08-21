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

#include "EGL.h"

#if EG_USE_QRCODE

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_QRCodeClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGQRCode : public EGCanvas
{
public:
                    EGQRCode(){};
                    EGQRCode(EGObject *pParent, int32_t Size, EG_Color_t DarkColor, EG_Color_t LightColor, 
                        const EG_ClassType_t *pClassCnfg = &c_QRCodeClass);
                    ~EGQRCode();
  virtual void      Configure(void);

  EG_Result_t       Update(const void *pData, uint32_t Length);

private:
  int32_t           m_Size;
  EG_Color_t        m_DarkColor;
  EG_Color_t        m_LightColor;
};

#endif

