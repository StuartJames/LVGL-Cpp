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

#include "EGL.h"

#if EG_USE_GIF

#include "gifdec.h"

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t EG_GifClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGImageGIF : public EGImage
{
public:
                    EGImageGIF(void){};
                    EGImageGIF(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &EG_GifClass);
                    ~EGImageGIF(void);
  virtual void      Configure(void);
  void              SetSource(const void *pSource);
  void              Restart(void);

  static void       NextFrameTaskCB(EGTimer *pTimer);

  gd_GIF            *m_pGif;
  EGTimer           *m_pTimer;
  EGImageBuffer      m_ImageBuffer;
  uint32_t           m_LastCall;

private:

};

#endif

