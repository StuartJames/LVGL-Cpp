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


/*This typedef exists purely to keep -Wpedantic happy when the file is empty.*/
/*It can be removed.*/
typedef int _keep_pedantic_happy;

#pragma once

#include "core/EG_Object.h"

#if EG_USE_GRIDNAV

///////////////////////////////////////////////////////////////////////////////

typedef enum {
    EG_GRIDNAV_CTRL_NONE          = 0x00,
    /* If there is no next/previous object in a direction,
     * the focus goes to the object in the next/previous row (on left/right keys)
     * or first/last row (on up/down keys) */
    EG_GRIDNAV_CTRL_ROLLOVER      = 0x01,
    /* If an arrow is pressed and the focused object can be scrolled in that direction
     * then it will be scrolled instead of going to the next/previous object.
     * If there is no more room for scrolling the next/previous object will be focused normally */
    EG_GRIDNAV_CTRL_SCROLL_FIRST  = 0x02,

} EG_GridNavControl_e;


typedef enum EG_FindMode_e : uint8_t{
	FIND_LEFT,
	FIND_RIGHT,
	FIND_TOP,
	FIND_BOTTOM,
	FIND_NEXT_ROW_FIRST_ITEM,
	FIND_PREV_ROW_LAST_ITEM,
	FIND_FIRST_ROW,
	FIND_LAST_ROW,
} EG_FindMode_e;

///////////////////////////////////////////////////////////////////////////////

class EGGridNav
{
public:
                          EGGridNav(void){};
  void                    SetFocused(EGObject *pCont, EGObject *pToFocus, EG_AnimateEnable_e Animate);

  static void             Install(EGObject *pObj, EG_GridNavControl_e m_Control);
  static void             Remove(EGObject * pObj);

private:
  EGObject*               FindChild(EGObject *pObj, EGObject *pStartChild, EG_FindMode_e Mode);
  EGObject*               FindFirstFocusable(EGObject *pObj);
  EGObject*               FindLastFocusable(EGObject *pObj);
  bool                    IsFocusable(EGObject *pObj);
  EG_Coord_t              GetCenterX(EGObject *pObj);
  EG_Coord_t              GetCenterY(EGObject *pObj);

  static void             EventCB(EGEvent *pEvent);

  EG_GridNavControl_e     m_Control;
	EGObject                *m_pFocused;

};


#endif 