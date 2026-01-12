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
 * SJ    2025/08/18   1.a.1    LV
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../misc/EG_Types.h"

/////////////////////////////////////////////////////////////////////////////////////

class EGEvent;

/////////////////////////////////////////////////////////////////////////////////////

typedef enum EG_ObjClassEditable_e : uint8_t{
	EG_OBJ_CLASS_EDITABLE_INHERIT, // Check the base class. Must have 0 value to let zero initialized class inherit*/
	EG_OBJ_CLASS_EDITABLE_TRUE,
	EG_OBJ_CLASS_EDITABLE_FALSE,
} EG_ObjClassEditable_e;


typedef enum EG_ObjClassGroupDef_e : uint8_t {
	EG_OBJ_CLASS_GROUP_DEF_INHERIT, // Check the base class. Must have 0 value to let zero initialized class inherit*/
	EG_OBJ_CLASS_GROUP_DEF_TRUE,
	EG_OBJ_CLASS_GROUP_DEF_FALSE,
} EG_ObjClassGroupDef_e;


typedef struct EG_ClassType_t {
  const EG_ClassType_t *pBaseClassType;
	void (*pEventCB)(const EG_ClassType_t *pClass, EGEvent *pEvent); // Widget type specific event function
	EG_Coord_t WidthDef;
	EG_Coord_t HeightDef;
	uint8_t IsEditable : 2;      
	uint8_t GroupDef : 2;        
#if EG_USE_USER_DATA
	void *pExtData;
#endif
} EG_ClassType_t;

typedef void (*EG_ObjClassEventCB_t)(EGEvent *pEvent);

///////////////////////////////////////////////////////////////////////////////////////////////////


