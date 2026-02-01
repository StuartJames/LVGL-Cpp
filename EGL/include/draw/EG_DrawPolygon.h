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

#include "EG_DrawRect.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawContext;

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawPolygon
{
public:
                    EGDrawPolygon() : m_pContext(nullptr){};
  void              Polygon(const EGDrawContext *pDrawContext, const EGDrawRect *pRect, const EGPoint Points[], uint16_t PointCount);
  void              Triangle(const EGDrawContext *pDrawContext, const EGDrawRect *pRect, const EGPoint Points[]);

  const EGDrawContext  *m_pContext;        
};
