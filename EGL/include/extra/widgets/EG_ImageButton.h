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

#if EG_USE_IMGBTN != 0

///////////////////////////////////////////////////////////////////////////////////////

typedef enum {
  EG_IMGBTN_STATE_RELEASED,
  EG_IMGBTN_STATE_PRESSED,
  EG_IMGBTN_STATE_DISABLED,
  EG_IMGBTN_STATE_CHECKED_RELEASED,
  EG_IMGBTN_STATE_CHECKED_PRESSED,
  EG_IMGBTN_STATE_CHECKED_DISABLED,
  _EG_IMGBTN_STATE_NUM,
} EG_ImageButtonState_e;

extern const EG_ClassType_t c_ImageButtonClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGImageButton : public EGObject
{
public:
                          EGImageButton(void);
                          EGImageButton(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ImageButtonClass);
  virtual void            Configure(void);
  void                    SetSource(EG_ImageButtonState_e State, const void *pSourceLeft, const void *pSourceMid,
                          const void *pSourceRight);
  void                    SetState(EG_ImageButtonState_e State);
  const void*             GetSourceLeft(EG_ImageButtonState_e State);
  const void*             GetSourceMiddle(EG_ImageButtonState_e State);
  const void*             GetSourceRight(EG_ImageButtonState_e State);
  void                    Event(EGEvent *pEvent);

  static void             EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  const void              *m_pImageSourceMid[_EG_IMGBTN_STATE_NUM];   // Store center images to each state
  const void              *m_pImsgeSourceLeft[_EG_IMGBTN_STATE_NUM];  // Store left side images to each state
  const void              *m_pImageSourceRight[_EG_IMGBTN_STATE_NUM]; // Store right side images to each state
  EG_ImageColorFormat_t    m_ColorFormat; // Color format of the currently active image

private:
  void                    DrawMain(EGEvent *pEvent);
  void                    RefreshImage(void);
  EG_ImageButtonState_e   SuggestState(EG_ImageButtonState_e State);
  EG_ImageButtonState_e   GetState(void);
};

#endif 