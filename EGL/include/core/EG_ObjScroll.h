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
 */
 

#pragma once

#include <stddef.h>
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////

class EGObject;

// Scrollbar modes: shows when should the scrollbars be visible
typedef enum EG_ScrollbarMode_e : uint8_t {
  EG_SCROLLBAR_MODE_OFF,      // Never show scrollbars
  EG_SCROLLBAR_MODE_ON,       // Always show scrollbars
  EG_SCROLLBAR_MODE_ACTIVE,   // Show scroll bars when object is being scrolled
  EG_SCROLLBAR_MODE_AUTO,     // Show scroll bars when the content is large enough to be scrolled
} EG_ScrollbarMode_e;

// Scroll span align options. Tells where to align the snappable children when scroll stops.
typedef enum EG_ScrollSnap_e : uint8_t{
  EG_SCROLL_SNAP_NONE,    // Do not align, leave where it is
  EG_SCROLL_SNAP_START,   // Align to the left/top
  EG_SCROLL_SNAP_END,     // Align to the right/bottom
  EG_SCROLL_SNAP_CENTER   // Align to the center
} EG_ScrollSnap_e;



