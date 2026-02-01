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

#include "draw/EG_DrawContext.h"
#include "draw/EG_DrawRect.h"
#include "misc/EG_Assert.h"

//////////////////////////////////////////////////////////////////////////////////////

EGDrawRect::EGDrawRect(void) : 
  m_pContext(nullptr),
  m_Radius(0),
  m_BlendMode(EG_BLEND_MODE_NORMAL),
	m_BackgroundOPA(EG_OPA_COVER),
	m_BackgroundColor(EG_ColorWhite()),
  m_pBackImageSource(nullptr),
	m_pBackImageSymbolFont(EG_FONT_DEFAULT),
	m_BackImageRecolor(EG_ColorBlack()),
	m_BackImageOPA(EG_OPA_COVER),
	m_BackImageRecolorOPA(EG_OPA_COVER),
  m_BackImageTiled(0),
	m_BorderColor(EG_ColorBlack()),
  m_BorderWidth(0),
	m_BorderOPA(EG_OPA_COVER),
  m_BorderPost(0),
  m_BorderSide(EG_BORDER_SIDE_FULL),
  m_OutlineColor(EG_ColorBlack()),
  m_OutlineWidth(0),
  m_OutlinePadding(0),
	m_OutlineOPA(EG_OPA_COVER),
	m_ShadowColor(EG_ColorBlack()),
  m_ShadowWidth(0),
  m_ShadowOffsetX(0),
  m_ShadowOffsetY(0),
	m_ShadowOPA(EG_OPA_COVER)
{
	m_BackgroundGrad.stops[0].color = EG_ColorWhite();
	m_BackgroundGrad.stops[1].color = EG_ColorBlack();
	m_BackgroundGrad.stops[1].frac = 0xFF;
	m_BackgroundGrad.stops_count = 2;
  m_BackgroundGrad.dir = EG_GRAD_DIR_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawRect::Draw(const EGDrawContext *pContext, const EGRect *pRect)
{
	if(pRect->GetHeight() < 1 || pRect->GetWidth() < 1) return;
  m_pContext = pContext;
	pContext->DrawRectProc(this, pRect);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDrawRect::DrawBackground(const EGDrawContext *pContext, const EGRect *pRect)
{
	if(pRect->GetHeight() < 1 || pRect->GetWidth() < 1) return;
  m_pContext = pContext;
	pContext->DrawBackgroundProc(this, pRect);
}

//////////////////////////////////////////////////////////////////////////////////

void EGDrawRect::Reset(void)
{
  m_Radius = 0;
  m_BlendMode = EG_BLEND_MODE_NORMAL;
	m_BackgroundColor = EG_ColorWhite();
	m_BackgroundGrad.stops[0].color = EG_ColorWhite();
	m_BackgroundGrad.stops[1].color = EG_ColorBlack();
	m_BackgroundGrad.stops[1].frac = 0xFF;
	m_BackgroundGrad.stops_count = 2;
	m_BorderColor = EG_ColorBlack();
	m_ShadowColor = EG_ColorBlack();
  m_pBackImageSource = nullptr;
	m_pBackImageSymbolFont = EG_FONT_DEFAULT;
	m_BackgroundOPA = EG_OPA_COVER;
	m_BackImageOPA = EG_OPA_COVER;
	m_OutlineOPA = EG_OPA_COVER;
	m_BorderOPA = EG_OPA_COVER;
	m_ShadowOPA = EG_OPA_COVER;
	m_BorderSide = EG_BORDER_SIDE_FULL;
}

//////////////////////////////////////////////////////////////////////////////////

void EGDrawRect::operator=(const EGDrawRect &rval)
{
  m_pContext              = rval.m_pContext;        
	m_Radius                = rval.m_Radius;
	m_BlendMode             = rval.m_BlendMode;
	m_BackgroundOPA         = rval.m_BackgroundOPA;	   
	m_BackgroundColor       = rval.m_BackgroundColor;     
	m_pBackImageSource      = rval.m_pBackImageSource;	   
	m_pBackImageSymbolFont  = rval.m_pBackImageSymbolFont;	
  m_BackImageRecolor      = rval.m_BackImageRecolor;
	m_BackImageOPA          = rval.m_BackImageOPA;
	m_BackImageRecolorOPA   = rval.m_BackImageRecolorOPA;
	m_BackImageTiled        = rval.m_BackImageTiled;
	m_BorderColor           = rval.m_BorderColor;	       
	m_BorderWidth           = rval.m_BorderWidth;
	m_BorderOPA             = rval.m_BorderOPA;
	m_BorderPost            = rval.m_BorderPost;      
	m_BorderSide            = rval.m_BorderSide;
	m_OutlineColor          = rval.m_OutlineColor;	       
	m_OutlineWidth          = rval.m_OutlineWidth;
	m_OutlinePadding        = rval.m_OutlinePadding;
	m_OutlineOPA            = rval.m_OutlineOPA;
	m_ShadowColor           = rval.m_ShadowColor;	       
	m_ShadowWidth           = rval.m_ShadowWidth;
	m_ShadowOffsetX         = rval.m_ShadowOffsetX;
	m_ShadowOffsetY         = rval.m_ShadowOffsetY;
	m_ShadowSpread          = rval.m_ShadowSpread;
	m_ShadowOPA             = rval.m_ShadowOPA;
  EG_CopyMem(&m_BackgroundGrad, &rval.m_BackgroundGrad, sizeof(EG_GradDescriptor_t));
}


