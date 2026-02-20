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

#include "misc/EG_Misc.h"

/////////////////////////////////////////////////////////////////////////////

#if(!defined(EG_ENABLE_GC)) || EG_ENABLE_GC == 0
    EG_ROOTS
#endif

/////////////////////////////////////////////////////////////////////////////

void EG_GC_ClearRoots(void)
{
#define EG_CLEAR_ROOT(root_type, root_name) EG_ZeroMem(&EG_GC_ROOT(root_name), sizeof(EG_GC_ROOT(root_name)));
    EG_ITERATE_ROOTS(EG_CLEAR_ROOT)
}
