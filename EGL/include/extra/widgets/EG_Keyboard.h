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

#include "widgets/EG_ButtonMatrix.h"

#if EG_USE_KEYBOARD

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_BTNMATRIX == 0
#error "lv_kb: lv_btnm is required. Enable it in EG_Config.h (EG_USE_BTNMATRIX  1) "
#endif

#if EG_USE_TEXTAREA == 0
#error "lv_kb: lv_ta is required. Enable it in EG_Config.h (EG_USE_TEXTAREA  1) "
#endif

#define EG_KEYBOARD_CTRL_BTN_FLAGS (EG_BTNMATRIX_CTRL_NO_REPEAT | EG_BTNMATRIX_CTRL_CLICK_TRIG | EG_BTNMATRIX_CTRL_CHECKED)

///////////////////////////////////////////////////////////////////////////////////////

class EGEdit;

typedef enum {
    EG_KEYBOARD_MODE_TEXT_LOWER,
    EG_KEYBOARD_MODE_TEXT_UPPER,
    EG_KEYBOARD_MODE_SPECIAL,
    EG_KEYBOARD_MODE_NUMBER,
    EG_KEYBOARD_MODE_USER_1,
    EG_KEYBOARD_MODE_USER_2,
    EG_KEYBOARD_MODE_USER_3,
    EG_KEYBOARD_MODE_USER_4,
} EG_KeyboardMode_e;

extern const EG_ClassType_t c_KeyboardClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGKeyboard : public EGButtonMatrix
{
public:
                      EGKeyboard(void);
                      EGKeyboard(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_KeyboardClass);
  virtual void        Configure(void);
  void                SetEditCtrl(EGEdit *pEditCtrl);
  void                SetMode(EG_KeyboardMode_e Mode);
  void                SetPopoverModes(bool Enable);
  void                SetMaps(EG_KeyboardMode_e Mode, const char *pMap[], const EG_ButMatrixCtrl_t CtrlMap[]);
  EGEdit*             GetEditCtrl(void){ return m_pEditCtrl; };
  EG_KeyboardMode_e   GetMode(void){ return m_Mode; };
  bool                GetPopoverMode(void){ return m_Popovers; };
  void                Event(EGEvent *pEvent);

  static void         EventCB(EGEvent *pEvent);

  EGEdit              *m_pEditCtrl;          // Pointer to the assigned text area
  EG_KeyboardMode_e   m_Mode;               // Key map type
  uint8_t             m_Popovers : 1;       // Show button titles in popovers on press

private:
  void                UpdateMap(void);
  void                UpdateCtrlMap(void);


};

///////////////////////////////////////////////////////////////////////////////////////


#endif 