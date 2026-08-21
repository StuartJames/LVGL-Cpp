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

#include "../EG_IntrnlConfig.h"
#include "../core/EG_Object.h"
#include "EG_Image.h"

#if EG_USE_ANIMIMG != 0

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_IMG == 0       // Testing of dependencies
#error "EGAnimimg: EGImage is required. Enable it in EG_Config.h (EG_USE_IMG 1)"
#endif

///////////////////////////////////////////////////////////////////////////////////////

enum {
    EG_ANIM_IMG_PART_MAIN,
};
typedef uint8_t EG_AnimImgPart_t;

extern const EG_ClassType_t c_AnimateImageClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGAnimatedImage : public EGObject
{
public:
                          EGAnimatedImage(void){};
                          EGAnimatedImage(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_AnimateImageClass);
  virtual void            Configure(void);
  void                    SetSource(const void *pDescriptor[], uint8_t Count);
  void                    Start(void);
  void                    SetDuration(uint32_t Duration);
  void                    SetRepeatCount(uint16_t Count);

  static void             IndexChange(EGAnimate *pAnimation, int32_t Indexx);

  EGImage                 m_Image;
  EGAnimate               m_Animate;
  const void              **m_pDicriptors;        // picture sequence 
  int8_t                  m_PictureCount;


};

#endif 