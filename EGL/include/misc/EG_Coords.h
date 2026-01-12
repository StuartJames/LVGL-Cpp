
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

#include <stdbool.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////

#if EG_USE_LARGE_COORD
typedef int32_t EG_Coord_t;
#else
typedef int16_t EG_Coord_t;
#endif

#if EG_USE_LARGE_COORD
#define _EG_COORD_TYPE_SHIFT (29U)
#else
#define _EG_COORD_TYPE_SHIFT (13U)
#endif

#define _EG_COORD_TYPE_MASK (3 << _EG_COORD_TYPE_SHIFT)
#define _EG_COORD_TYPE(x) ((x)&_EG_COORD_TYPE_MASK)     /*Extract type specifiers*/
#define _EG_COORD_PLAIN(x) ((x) & ~_EG_COORD_TYPE_MASK) /*Remove type specifiers*/

#define _EG_COORD_TYPE_PX (0 << _EG_COORD_TYPE_SHIFT)
#define _EG_COORD_TYPE_SPEC (1 << _EG_COORD_TYPE_SHIFT)
#define _EG_COORD_TYPE_PX_NEG (3 << _EG_COORD_TYPE_SHIFT)

#define EG_COORD_IS_PX(x) (_EG_COORD_TYPE(x) == _EG_COORD_TYPE_PX ||        \
															 _EG_COORD_TYPE(x) == _EG_COORD_TYPE_PX_NEG ? \
														 true :                                         \
														 false)
#define EG_COORD_IS_SPEC(x) (_EG_COORD_TYPE(x) == _EG_COORD_TYPE_SPEC ? true : false)

#define EG_COORD_SET_SPEC(x) ((x) | _EG_COORD_TYPE_SPEC)

/*Special coordinates*/
#define _EG_PCT(x) (x < 0 ? EG_COORD_SET_SPEC(1000 - (x)) : EG_COORD_SET_SPEC(x))
#define EG_COORD_IS_PCT(x) ((EG_COORD_IS_SPEC(x) && _EG_COORD_PLAIN(x) <= 2000) ? true : false)
#define EG_COORD_GET_PCT(x) (_EG_COORD_PLAIN(x) > 1000 ? 1000 - _EG_COORD_PLAIN(x) : _EG_COORD_PLAIN(x))
#define EG_SIZE_CONTENT EG_COORD_SET_SPEC(2001)

EG_EXPORT_CONST_INT(EG_SIZE_CONTENT);

/*Max coordinate value*/
#define EG_COORD_MAX ((1 << _EG_COORD_TYPE_SHIFT) - 1)
#define EG_COORD_MIN (-EG_COORD_MAX)

EG_EXPORT_CONST_INT(EG_COORD_MAX);
EG_EXPORT_CONST_INT(EG_COORD_MIN);

#define _EG_TRANSFORM_TRIGO_SHIFT 10

