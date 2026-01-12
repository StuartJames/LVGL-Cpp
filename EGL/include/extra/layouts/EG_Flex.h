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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once


#include "core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////

#if EG_USE_FLEX

#define EG_OBJ_FLAG_FLEX_IN_NEW_TRACK   EG_OBJ_FLAG_LAYOUT_1
EG_EXPORT_CONST_INT(EG_OBJ_FLAG_FLEX_IN_NEW_TRACK);

#define _EG_FLEX_COLUMN        (1 << 0)
#define _EG_FLEX_WRAP       (1 << 2)
#define _EG_FLEX_REVERSE    (1 << 3)

///////////////////////////////////////////////////////////////////////////////

class EGObject; // Can't include EG_Object.h because it includes this header file

typedef enum : uint8_t {
    EG_FLEX_ALIGN_START,
    EG_FLEX_ALIGN_END,
    EG_FLEX_ALIGN_CENTER,
    EG_FLEX_ALIGN_SPACE_EVENLY,
    EG_FLEX_ALIGN_SPACE_AROUND,
    EG_FLEX_ALIGN_SPACE_BETWEEN,
} EG_FlexAlign_e;

typedef enum : uint16_t {
    EG_FLEX_FLOW_ROW                 = 0x00,
    EG_FLEX_FLOW_COLUMN              = _EG_FLEX_COLUMN,
    EG_FLEX_FLOW_ROW_WRAP            = EG_FLEX_FLOW_ROW | _EG_FLEX_WRAP,
    EG_FLEX_FLOW_ROW_REVERSE         = EG_FLEX_FLOW_ROW | _EG_FLEX_REVERSE,
    EG_FLEX_FLOW_ROW_WRAP_REVERSE    = EG_FLEX_FLOW_ROW | _EG_FLEX_WRAP | _EG_FLEX_REVERSE,
    EG_FLEX_FLOW_COLUMN_WRAP         = EG_FLEX_FLOW_COLUMN | _EG_FLEX_WRAP,
    EG_FLEX_FLOW_COLUMN_REVERSE      = EG_FLEX_FLOW_COLUMN | _EG_FLEX_REVERSE,
    EG_FLEX_FLOW_COLUMN_WRAP_REVERSE = EG_FLEX_FLOW_COLUMN | _EG_FLEX_WRAP | _EG_FLEX_REVERSE,
} EG_FlexFlow_e;

typedef struct {
	EGObject *pItem;
	EG_Coord_t min_size;
	EG_Coord_t max_size;
	EG_Coord_t final_size;
	uint32_t grow_value;
	uint32_t clamped : 1;
} GrowProps_t;

typedef struct {
	EG_Coord_t track_cross_size;
	EG_Coord_t track_main_size;     /*For all items*/
	EG_Coord_t track_fix_main_size; /*For non grow items*/
	uint32_t item_cnt;
	GrowProps_t *grow_dsc;
	uint32_t grow_item_cnt;
	uint32_t grow_dsc_calc : 1;
} TrackProps_t;

///////////////////////////////////////////////////////////////////////////////

class EGFlexLayout
{
public:
                          EGFlexLayout(void);
  void                    StyleSetFlow(EGStyle *pStyle, EG_FlexFlow_e value);
  void                    StyleSetMainPlace(EGStyle *pStyle, EG_FlexAlign_e value);
  void                    StyleSetCrossPlace(EGStyle *pStyle, EG_FlexAlign_e value);
  void                    StyleSetTrackPlace(EGStyle *pStyle, EG_FlexAlign_e value);
  void                    StyleSetGrow(EGStyle *pStyle, uint8_t value);

  static void             Initialise(void);
  static void             SetObjFlow(EGObject *pObject, EG_FlexFlow_e Flow);
  static void             SetObjAlign(EGObject *pObject, EG_FlexAlign_e main_place, EG_FlexAlign_e cross_place,
                                      EG_FlexAlign_e track_cross_place);
  static void             SetObjGrow(EGObject *pObject, uint8_t grow);
  static void             SetObjStyleFlow(EGObject *pObject, EG_FlexFlow_e value, EG_StyleFlags_t selector);
  static void             SetObjStyleMainPlace(EGObject *pObject, EG_FlexAlign_e value, EG_StyleFlags_t selector);
  static void             SetObjStyleCrossPlace(EGObject *pObject, EG_FlexAlign_e value, EG_StyleFlags_t selector);
  static void             SetObjStyleTrackPlace(EGObject *pObject, EG_FlexAlign_e value, EG_StyleFlags_t selector);
  static void             SetObjStyleGrow(EGObject *pObject, uint8_t value, EG_StyleFlags_t selector);
  static EG_FlexFlow_e    GetObjStyleFlow(const EGObject *pObject, uint32_t part);
  static EG_FlexAlign_e   GetObjStyleMainPlace(const EGObject *pObject, uint32_t part);
  static EG_FlexAlign_e   GetObjStyleCrossPlace(const EGObject *pObject, uint32_t part);
  static EG_FlexAlign_e   GetObjStyleTrackPlace(const EGObject *pObject, uint32_t part);
  static uint8_t          GetObjStyleGrow(const EGObject *pObject, uint32_t part);
        
  static void             UpdateCB(EGObject *pObject, void *pUserData);

  static EGStyleProperty_e  m_StyleFlow;
  static EGStyleProperty_e  m_StyleMainPlace;
  static EGStyleProperty_e  m_StyleCrossPlace;
  static EGStyleProperty_e  m_StyleTrackPlace;
  static EGStyleProperty_e  m_StyleGrow;
  static uint32_t           m_LayoutReference;

private:
  int32_t       FindTrackEnd(EGObject *pObject, int32_t item_start_id, EG_Coord_t max_main_size,
                  EG_Coord_t item_gap, TrackProps_t *t);
  void          RepositionChildren(EGObject *pObject, int32_t item_first_id, int32_t item_last_id, EG_Coord_t abs_x,
                  EG_Coord_t abs_y, EG_Coord_t max_main_size, EG_Coord_t item_gap, TrackProps_t *t);
  void          PlaceContent(EG_FlexAlign_e place, EG_Coord_t max_size, EG_Coord_t content_size, EG_Coord_t item_cnt,
                  EG_Coord_t *start_pos, EG_Coord_t *gap);
  EGObject*     GetNextItem(EGObject *cont, bool rev, int32_t *item_id);

	EG_FlexAlign_e            m_MainPlace;
	EG_FlexAlign_e            m_CrossPlace;
	EG_FlexAlign_e            m_TrackPlace;
	uint8_t                   m_Row : 1;
	uint8_t                   m_Wrap : 1;
	uint8_t                   m_Reverse : 1;
};

///////////////////////////////////////////////////////////////////////////////

inline EG_FlexFlow_e EGFlexLayout::GetObjStyleFlow(const EGObject *pObject, uint32_t part)
{
    EG_StyleValue_t Value = pObject->GetProperty(part, m_StyleFlow);
    return (EG_FlexFlow_e)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline EG_FlexAlign_e EGFlexLayout::GetObjStyleMainPlace(const EGObject *pObject, uint32_t part)
{
    EG_StyleValue_t Value = pObject->GetProperty(part, m_StyleMainPlace);
    return (EG_FlexAlign_e)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline EG_FlexAlign_e EGFlexLayout::GetObjStyleCrossPlace(const EGObject *pObject, uint32_t part)
{
    EG_StyleValue_t Value = pObject->GetProperty(part, m_StyleCrossPlace);
    return (EG_FlexAlign_e)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline EG_FlexAlign_e EGFlexLayout::GetObjStyleTrackPlace(const EGObject *pObject, uint32_t part)
{
    EG_StyleValue_t Value = pObject->GetProperty(part, m_StyleTrackPlace);
    return (EG_FlexAlign_e)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGFlexLayout::GetObjStyleGrow(const EGObject *pObject, uint32_t part)
{
    EG_StyleValue_t Value = pObject->GetProperty(part, m_StyleGrow);
    return (uint8_t)Value.Number;
}

#endif