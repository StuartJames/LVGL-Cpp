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

#include "../EG_IntrnlConfig.h"

#if EG_USE_BTNMATRIX != 0

#include "../core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_BTNMATRIX_BTN_NONE 0xFFFF
EG_EXPORT_CONST_INT(EG_BTNMATRIX_BTN_NONE);

/** Type to store button control bits (disabled, hidden etc.)
 * The first 3 bits are used to store the width*/
enum {
  _EG_BTNMATRIX_WIDTH            = 0x000F, // Reserved to store the size units
  EG_BTNMATRIX_CTRL_HIDDEN       = 0x0010, // Button hidden
  EG_BTNMATRIX_CTRL_NO_REPEAT    = 0x0020, // Do not repeat press this button.
  EG_BTNMATRIX_CTRL_DISABLED     = 0x0040, // Disable this button.
  EG_BTNMATRIX_CTRL_CHECKABLE    = 0x0080, // The button can be toggled.
  EG_BTNMATRIX_CTRL_CHECKED      = 0x0100, // Button is currently toggled (e.g. checked).
  EG_BTNMATRIX_CTRL_CLICK_TRIG   = 0x0200, // 1: Send EG_EVENT_VALUE_CHANGE on CLICK, 0: Send EG_EVENT_VALUE_CHANGE on PRESS
  EG_BTNMATRIX_CTRL_POPOVER      = 0x0400, // Show a popover when pressing this key
  EG_BTNMATRIX_CTRL_RECOLOR      = 0x0800, // Enable text recoloring with `#color`
  _EG_BTNMATRIX_CTRL_RESERVED_1  = 0x1000, // Reserved for later use
  _EG_BTNMATRIX_CTRL_RESERVED_2  = 0x2000, // Reserved for later use
  EG_BTNMATRIX_CTRL_CUSTOM_1     = 0x4000, // Custom free to use flag
  EG_BTNMATRIX_CTRL_CUSTOM_2     = 0x8000, // Custom free to use flag
};

typedef uint16_t EG_ButMatrixCtrl_t;

typedef bool (*lv_btnmatrix_btn_draw_cb_t)(EGObject * btnm, uint32_t btn_id, const EGRect * draw_area,
                                           const EGRect * clip_area);

// Data of button matrix
typedef struct {
} lv_btnmatrix_t;

typedef enum {
    EG_BTNMATRIX_DRAW_PART_BTN,    // The rectangle and label of buttons
} lv_BtnMatrixDrawPartType_t;

extern const EG_ClassType_t c_ButtonMatrixClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGButtonMatrix : public EGObject
{
public:
                    EGButtonMatrix(void);
                    EGButtonMatrix(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ButtonMatrixClass);
  virtual           ~EGButtonMatrix(void);
  virtual void      Configure(void);
  void              SetMap(const char *pMap[]);
  void              SetControlMap(const EG_ButMatrixCtrl_t CtrlMap[]);
  void              SetSelected(uint16_t ButtonID);
  void              SetControl(uint16_t ButtonID, EG_ButMatrixCtrl_t ctrl);
  void              ClearControl(uint16_t ButtonID, EG_ButMatrixCtrl_t ctrl);
  void              SetControlAll(EG_ButMatrixCtrl_t ctrl);
  void              ClearControlAll(EG_ButMatrixCtrl_t ctrl);
  void              SetButtonWidth(uint16_t ButtonID, uint8_t width);
  void              SetOneChecked(bool en);
  const char**      GetMap(void){ return m_ppMap; };
  uint16_t          GetSelected(void){ return m_SelectID; };
  const char*       GetButtonText(uint16_t ButtonID);
  bool              HasControl(uint16_t ButtonID, EG_ButMatrixCtrl_t ctrl);
  bool              GetOneChecked(void){ return m_OneCheck; };
  void              Event(EGEvent *pEvent);

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
 
  const char        **m_ppMap;          // Pointer to the current map
  EGRect             *m_pButtonRects;   // Array of areas of buttons
  EG_ButMatrixCtrl_t *m_pControlBits;   // Array of control bytes
  uint16_t            m_ButtonCount;    // Number of button in 'map_p'(Handled by the library)
  uint16_t            m_RowCount;       // Number of rows in 'map_p'(Handled by the library)
  uint16_t            m_SelectID;       // Index of the active button (being pressed/released etc) or EG_BTNMATRIX_BTN_NONE
  uint8_t             m_OneCheck : 1;   // Single button toggled at once

private:
  void              Draw(EGEvent *pEvent);
  void              AllocateMap(const char **ppMap);
  uint8_t           GetButtonWidth(EG_ButMatrixCtrl_t ControlBits);
  bool              IsHidden(EG_ButMatrixCtrl_t ControlBits);
  bool              IsChecked(EG_ButMatrixCtrl_t ControlBits);
  bool              IsRepeatDisabled(EG_ButMatrixCtrl_t ControlBits);
  bool              IsInactive(EG_ButMatrixCtrl_t ControlBits);
  bool              IsClickTrig(EG_ButMatrixCtrl_t ControlBits);
  bool              IsPopover(EG_ButMatrixCtrl_t ControlBits);
  bool              IsCheckable(EG_ButMatrixCtrl_t ControlBits);
  bool              IsRecolor(EG_ButMatrixCtrl_t ControlBits);
  bool              GetChecked(EG_ButMatrixCtrl_t ControlBits);
  uint16_t          GetFromPoint(EGPoint *pPoint);
  void              InvalidateButton(uint16_t Index);
  void              MakeOneButtonChecked(uint16_t Index);
  bool              HasPopoversInTopRow(void);
};

#endif
