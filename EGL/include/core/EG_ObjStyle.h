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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../misc/lv_bidi.h"
#include "../misc/EG_Style.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGObject;

typedef enum : uint8_t {
	CACHE_ZERO = 0,
	CACHE_TRUE = 1,
	CACHE_UNSET = 2,
	CACHE_255 = 3,
	CACHE_NEED_CHECK = 4,
} Cache_e;

typedef enum : uint8_t {
  _EG_STYLE_STATE_CMP_SAME,           /*The style properties in the 2 states are identical*/
  _EG_STYLE_STATE_CMP_DIFF_REDRAW,    /*The differences can be shown with a simple redraw*/
  _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD,  /*The differences can be shown with a simple redraw*/
  _EG_STYLE_STATE_CMP_DIFF_LAYOUT,    /*The differences can be shown with a simple redraw*/
} EG_StyleStateCmp_e;

typedef uint32_t EG_StyleFlags_t;

typedef struct Transition_t{
  Transition_t(void) : pObj(nullptr), Property(EG_STYLE_PROP_INV), SelectFlags(0), StartValue(0), EndValue(0){}
	EGObject            *pObj;
	EGStyleProperty_e   Property;
	EG_StyleFlags_t     SelectFlags;
	EG_StyleValue_t     StartValue;
	EG_StyleValue_t     EndValue;
} Transition_t;

typedef struct EG_ObjStyle_t{
  EG_ObjStyle_t(void) : pStyle(nullptr), SelectFlags(0), IsLocal(0), IsTransition(0){}
  EGStyle *pStyle;
  EG_StyleFlags_t SelectFlags  : 24;
  uint32_t IsLocal      : 1;
  uint32_t IsTransition : 1;
} EG_ObjStyle_t;


typedef struct EG_TransitionDiscriptor_t{
  EG_TransitionDiscriptor_t(void) : Time(0), Delay(0), SelectFlags(0), Property(EG_STYLE_PROP_INV), PathCB(nullptr), m_pParam(nullptr){}
  uint16_t              Time;
  uint16_t              Delay;
  EG_StyleFlags_t       SelectFlags;
  EGStyleProperty_e     Property;
  EG_AnimatePathCB_t    PathCB;
  void                 *m_pParam;
} EG_TransitionDiscriptor_t;

