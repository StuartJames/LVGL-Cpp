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
#include "sw/EG_DrawSoftGradient.h"

class EGDrawContext;

//////////////////////////////////////////////////////////////////////////////////////

#define EG_RADIUS_CIRCLE 0x7FFF // A very big radius to always draw as circle
EG_EXPORT_CONST_INT(EG_RADIUS_CIRCLE);

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawRect
{
public:
                    EGDrawRect(void);
  void              Draw(const EGDrawContext *pContext, const EGRect *pRect);
  void              DrawBackground(const EGDrawContext *pContext, const EGRect *pRect);
  void              InitialiseBuffer(void){};
  void              Reset(void);
  void              operator = (const EGDrawRect &rval);
           
  const EGDrawContext  *m_pContext;        
	EG_Coord_t            m_Radius;
	EG_BlendMode_e        m_BlendMode;
	EG_OPA_t              m_BackgroundOPA;	      // Background
	EG_Color_t            m_BackgroundColor;      // First element of a gradient is a color, so it maps well here
	EG_GradDescriptor_t   m_BackgroundGrad;
	const void           *m_pBackImageSource;	    // Background image
	const void           *m_pBackImageSymbolFont;
	EG_Color_t            m_BackImageRecolor;
	EG_OPA_t              m_BackImageOPA;
	EG_OPA_t              m_BackImageRecolorOPA;
	uint8_t               m_BackImageTiled;
	EG_Color_t            m_BorderColor;	        // Border
	EG_Coord_t            m_BorderWidth;
	EG_OPA_t              m_BorderOPA;
	uint8_t               m_BorderPost : 1;       // There is a border it will be drawn later.
	EG_BorderSide_t       m_BorderSide : 5;
	EG_Color_t            m_OutlineColor;	        // Outline
	EG_Coord_t            m_OutlineWidth;
	EG_Coord_t            m_OutlinePadding;
	EG_OPA_t              m_OutlineOPA;
	EG_Color_t            m_ShadowColor;	        // Shadow
	EG_Coord_t            m_ShadowWidth;
	EG_Coord_t            m_ShadowOffsetX;
	EG_Coord_t            m_ShadowOffsetY;
	EG_Coord_t            m_ShadowSpread;
	EG_OPA_t              m_ShadowOPA;
};
