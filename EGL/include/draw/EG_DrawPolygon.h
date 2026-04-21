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
                    EGDrawPolygon();
  void              Draw(const EGDrawContext *pDrawContext, const EGPoint *pVertices, uint16_t VerticesCount);

  const EGDrawContext  *m_pContext;
  EG_BlendMode_e        m_BlendMode;
	EG_OPA_t              m_FillOPA;	      // Background
	EG_Color_t            m_FillColor;      // First element of a gradient is a color, so it maps well here
	EG_Color_t            m_Color;	        // Border
	EG_Coord_t            m_Width;
	EG_OPA_t              m_OPA;
};
