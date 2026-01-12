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
#include "../misc/EG_Color.h"
#include "../misc/EG_Point.h"
#include "../misc/EG_Rect.h"
#include "../misc/EG_Style.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawContext;

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawLine
{
public:
                    EGDrawLine(void);
  void              Draw(const EGDrawContext  *pDrawContext, const EGPoint *pPoint1, const EGPoint *pPoint2);

  const EGDrawContext  *m_pContext;        
  EG_Color_t        m_Color;
  EG_Coord_t        m_Width;
  EG_Coord_t        m_DashWidth;
  EG_Coord_t        m_DashGap;
  EG_OPA_t          m_OPA;
  EG_BlendMode_e    m_BlendMode  : 3;
  uint8_t           m_RoundStart : 1;
  uint8_t           m_RoundEnd   : 1;
  uint8_t           m_RawEnd     : 1;    /*Do not bother with perpendicular line ending if it's not visible for any reason*/
};
