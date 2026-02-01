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

#if EG_USE_MSGBOX

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_BTNMATRIX == 0
#error "lv_mbox: lv_btnm is required. Enable it in EG_Config.h (EG_USE_BTNMATRIX  1) "
#endif

#if EG_USE_LABEL == 0
#error "lv_mbox: lv_label is required. Enable it in EG_Config.h (EG_USE_LABEL  1) "
#endif

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_MsgBoxClass;
extern const EG_ClassType_t c_MsgBoxContentClass;
extern const EG_ClassType_t c_MsgBoxBackdropClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGMessageBox : public EGObject
{
public:
                    EGMessageBox(void);
                    EGMessageBox(EGObject *pParent, const char *pTitle, const char *pText, const char *pButtonTexts[],
                       bool HasCloseButton, const EG_ClassType_t *pClassCnfg = &c_MsgBoxClass);
  EGObject*         GetTitle(void){ return m_pTitle; };
  EGObject*         GetCloseButton(void){ return m_pCloseButton; };
  EGObject*         GetText(void){ return m_pText; };
  EGObject*         GetContent(void){ return m_pContent; };
  EGObject*         GetButtons(void){ return m_pButtons; };
  uint16_t          GetActiveButton(void){ return m_pButtons->GetSelected(); };
  const char*       GetActiveButtonText(void);
  void              Close(void);
  void              CloseAsync(void);

  static void       ClickEventCB(EGEvent *pEvent);

  EGLabel          *m_pTitle;
  EGButton         *m_pCloseButton;
  EGObject         *m_pContent;
  EGLabel          *m_pText;
  EGButtonMatrix   *m_pButtons;
};

#endif 