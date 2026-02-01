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

#include "core/EG_DrawDiscriptor.h"
#include "core/EG_Object.h"
#include "core/EG_Display.h"
#include "core/EG_InputDevice.h"

/////////////////////////////////////////////////////////////////////////////

#define Obj_CLASS &c_ObjectClass

/////////////////////////////////////////////////////////////////////////////

EGDrawDiscriptor::EGDrawDiscriptor(void)
{
  m_pContext = nullptr;        
  m_pClass = nullptr;             
  m_Type = 0;                
  m_pRect = nullptr;          
  m_pDrawRect = nullptr;    
  m_pDrawLabel = nullptr;  
  m_pDrawLine = nullptr;   
  m_pDrawImage = nullptr;  
  m_pDrawArc = nullptr;    
  m_pPoint1 = nullptr;           
  m_pPoint2 = nullptr;           
  m_pText = nullptr;             
  m_TextLength = 0;         
  m_Part = 0;               
  m_Index = 0;              
  m_Radius = 0;             
  m_Value = 0;              
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InititialseDrawRect(uint32_t Part, EGDrawRect *pDrawRect)
{
	EG_OPA_t OPA = GetOPARecursive(this, Part);
	if(Part != EG_PART_MAIN) {
		if(OPA <= EG_OPA_MIN) {
			pDrawRect->m_BackgroundOPA = EG_OPA_TRANSP;
			pDrawRect->m_BackImageOPA = EG_OPA_TRANSP;
			pDrawRect->m_BorderOPA = EG_OPA_TRANSP;
			pDrawRect->m_OutlineOPA = EG_OPA_TRANSP;
			pDrawRect->m_ShadowOPA = EG_OPA_TRANSP;
			return;
		}
	}
#if EG_DRAW_COMPLEX
	if(Part != EG_PART_MAIN) pDrawRect->m_BlendMode = GetStyleBlendMode(Part);
	pDrawRect->m_Radius = GetStyleRadius(Part);
	if(pDrawRect->m_BackgroundOPA != EG_OPA_TRANSP) {
		pDrawRect->m_BackgroundOPA = GetStyleBckgroundOPA(Part);
		if(pDrawRect->m_BackgroundOPA > EG_OPA_MIN) {
			pDrawRect->m_BackgroundColor = GetStyleBackColorFiltered(Part);
			const EG_GradDescriptor_t *pGrad = GetStyleBackGradient(Part);
			if(pGrad && pGrad->dir != EG_GRAD_DIR_NONE) {
				EG_CopyMem(&pDrawRect->m_BackgroundGrad, pGrad, sizeof(EG_GradDescriptor_t));
			}
			else {
				pDrawRect->m_BackgroundGrad.dir = GetStyleBackGradientDirection(Part);
				if(pDrawRect->m_BackgroundGrad.dir != EG_GRAD_DIR_NONE) {
					pDrawRect->m_BackgroundGrad.stops[0].color = GetStyleBackColorFiltered(Part);
					pDrawRect->m_BackgroundGrad.stops[1].color = GetStyleBackGradientColorFiltered(Part);
					pDrawRect->m_BackgroundGrad.stops[0].frac = GetStyleBackMainStop(Part);
					pDrawRect->m_BackgroundGrad.stops[1].frac = GetStyleBackGradientStop(Part);
				}
				pDrawRect->m_BackgroundGrad.dither = GetStyleBackDitherMode(Part);
			}
		}
	}

	pDrawRect->m_BorderWidth = GetStyleBorderWidth(Part);
	if(pDrawRect->m_BorderWidth) {
		if(pDrawRect->m_BorderOPA != EG_OPA_TRANSP) {
			pDrawRect->m_BorderOPA = GetStyleBorderOPA(Part);
			if(pDrawRect->m_BorderOPA > EG_OPA_MIN) {
				pDrawRect->m_BorderSide = GetStyleBorderSide(Part);
				pDrawRect->m_BorderColor = GetStyleBorderColorFiltered(Part);
			}
		}
	}

	pDrawRect->m_OutlineWidth = GetStyleOutlineWidth(Part);
	if(pDrawRect->m_OutlineWidth) {
		if(pDrawRect->m_OutlineOPA != EG_OPA_TRANSP) {
			pDrawRect->m_OutlineOPA = GetStyleOutlineOPA(Part);
			if(pDrawRect->m_OutlineOPA > EG_OPA_MIN) {
				pDrawRect->m_OutlinePadding = GetStyleOutlinePadding(Part);
				pDrawRect->m_OutlineColor = GetStyleOutlineColorFiltered(Part);
			}
		}
	}

	if(pDrawRect->m_BackImageOPA != EG_OPA_TRANSP) {
		pDrawRect->m_pBackImageSource = GetStyleBackImageSource(Part);
		if(pDrawRect->m_pBackImageSource != nullptr) {
			pDrawRect->m_BackImageOPA = GetStyleBackImageOPA(Part);
			if(pDrawRect->m_BackImageOPA > EG_OPA_MIN) {
				if(EGDrawImage::GetType(pDrawRect->m_pBackImageSource) == EG_IMG_SRC_SYMBOL) {
					pDrawRect->m_pBackImageSymbolFont = GetStyleTextFont(Part);
					pDrawRect->m_BackImageRecolor = GetStyleTextColorFiltered(Part);
				}
				else {
					pDrawRect->m_BackImageRecolor = GetStyleBackImageRecolorFiltered(Part);
					pDrawRect->m_BackImageRecolorOPA = GetStyleBackImageRecolorOPA(Part);
					pDrawRect->m_BackImageTiled = GetStyleBackImageTiled(Part);
				}
			}
		}
	}

	if(pDrawRect->m_ShadowOPA) {
		pDrawRect->m_ShadowWidth = GetStyleShadowWidth(Part);
		if(pDrawRect->m_ShadowWidth) {
			if(pDrawRect->m_ShadowOPA > EG_OPA_MIN) {
				pDrawRect->m_ShadowOPA = GetStyleShadowOPA(Part);
				if(pDrawRect->m_ShadowOPA > EG_OPA_MIN) {
					pDrawRect->m_ShadowOffsetX = GetStyleShadowOffsetX(Part);
					pDrawRect->m_ShadowOffsetY = GetStyleShadowOffsetY(Part);
					pDrawRect->m_ShadowSpread = GetStyleShadowSpread(Part);
					pDrawRect->m_ShadowOPA = GetStyleShadowOPA(Part);
				}
			}
		}
	}
#else 
	if(pDrawRect->m_BackgroundOPA != EG_OPA_TRANSP) {
		pDrawRect->m_BackgroundOPA = GetStyleBckgroundOPA(Part);
		if(pDrawRect->m_BackgroundOPA > EG_OPA_MIN) {
			pDrawRect->m_BackgroundColor = GetStyleBackColorFiltered(Part);
		}
	}

	pDrawRect->m_BorderWidth = GetStyleBorderWidth(Part);
	if(pDrawRect->m_BorderWidth) {
		if(pDrawRect->m_BorderOPA != EG_OPA_TRANSP) {
			pDrawRect->m_BorderOPA = GetStyleBorderOPA(Part);
			if(pDrawRect->m_BorderOPA > EG_OPA_MIN) {
				pDrawRect->m_BorderColor = GetStyleBorderColorFiltered(Part);
				pDrawRect->m_BorderSide = GetStyleBorderSide(Part);
			}
		}
	}

	pDrawRect->m_OutlineWidth = GetStyleOutlineWidth(Part);
	if(pDrawRect->m_OutlineWidth) {
		if(pDrawRect->m_OutlineOPA != EG_OPA_TRANSP) {
			pDrawRect->m_OutlineOPA = GetStyleOutlineOPA(Part);
			if(pDrawRect->m_OutlineOPA > EG_OPA_MIN) {
				pDrawRect->m_OutlinePadding = GetStyleOutlinePadding(Part);
				pDrawRect->m_OutlineColor = GetStyleOutlineColorFiltered(Part);
			}
		}
	}

	if(pDrawRect->m_BackImageOPA != EG_OPA_TRANSP) {
		pDrawRect->m_pBackImageSource = GetStyleBackImageSource(Part);
		if(pDrawRect->m_pBackImageSource) {
			pDrawRect->m_BackImageOPA = GetStyleBackImageOPA(Part);
			if(pDrawRect->m_BackImageOPA > EG_OPA_MIN) {
				if(lv_img_src_get_type(pDrawRect->m_pBackImageSource) == EG_IMG_SRC_SYMBOL) {
					pDrawRect->m_pBackImageSymbolFont = GetStyleTextFont(Part);
					pDrawRect->m_BackImageRecolor = GetStyleTextColorFiltered(Part);
				}
				else {
					pDrawRect->m_BackImageRecolor = GetStyleBackImageRecolorFiltered(Part);
					pDrawRect->m_BackImageRecolorOPA = GetStyleBackImageRecolorOPA(Part);
					pDrawRect->m_BackImageTiled = GetStyleBackImageTiled(Part);
				}
			}
		}
	}
#endif
	if(OPA < EG_OPA_MAX) {
		pDrawRect->m_BackgroundOPA = (OPA * pDrawRect->m_BackgroundOPA) >> 8;
		pDrawRect->m_BackImageOPA = (OPA * pDrawRect->m_BackImageOPA) >> 8;
		pDrawRect->m_BorderOPA = (OPA * pDrawRect->m_BorderOPA) >> 8;
		pDrawRect->m_OutlineOPA = (OPA * pDrawRect->m_OutlineOPA) >> 8;
		pDrawRect->m_ShadowOPA = (OPA * pDrawRect->m_ShadowOPA) >> 8;
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InititialseDrawLabel(uint32_t Part, EGDrawLabel *pDrawLabel)
{
	pDrawLabel->m_OPA = GetStyleTextOPA(Part);
	if(pDrawLabel->m_OPA <= EG_OPA_MIN) return;
	EG_OPA_t OPA = GetOPARecursive(this, Part);
	if(OPA <= EG_OPA_MIN) {
		pDrawLabel->m_OPA = EG_OPA_TRANSP;
		return;
	}
	if(OPA < EG_OPA_MAX) pDrawLabel->m_OPA = (OPA * pDrawLabel->m_OPA) >> 8;
	if(pDrawLabel->m_OPA <= EG_OPA_MIN) return;
	pDrawLabel->m_Color = GetStyleTextColorFiltered(Part);
	pDrawLabel->m_Kerning = GetStyleTextKerning(Part);
	pDrawLabel->m_LineSpace = GetStyleTextLineSpace(Part);
	pDrawLabel->m_Decoration = GetStyleTextDecoration(Part);
#if EG_DRAW_COMPLEX
	if(Part != EG_PART_MAIN) pDrawLabel->m_BlendMode = GetStyleBlendMode(Part);
#endif
	pDrawLabel->m_pFont = GetStyleTextFont(Part);
#if EG_USE_BIDI
	pDrawLabel->bidi_dir = GetStyleBaseDirection(obj, EG_PART_MAIN);
#endif
	pDrawLabel->m_Align = GetStyleTextAlign(Part);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InititialseDrawImage(uint32_t Part, EGDrawImage *pDrawImage)
{
	pDrawImage->m_OPA = GetStyleImageOPA(Part);
	if(pDrawImage->m_OPA <= EG_OPA_MIN) return;
	EG_OPA_t OPA = GetOPARecursive(this, Part);
	if(OPA <= EG_OPA_MIN) {
		pDrawImage->m_OPA = EG_OPA_TRANSP;
		return;
	}
	if(OPA < EG_OPA_MAX) pDrawImage->m_OPA = (OPA * pDrawImage->m_OPA) >> 8;
	if(pDrawImage->m_OPA <= EG_OPA_MIN) return;
	pDrawImage->m_Angle = 0;
	pDrawImage->m_Zoom = EG_SCALE_NONE;
	pDrawImage->m_Pivot.m_X = m_Rect.GetWidth() / 2;
	pDrawImage->m_Pivot.m_Y = m_Rect.GetHeight() / 2;
	pDrawImage->m_RecolorOPA = GetStyleImageRecolorOPA(Part);
	if(pDrawImage->m_RecolorOPA > 0) {
		pDrawImage->m_RecolorOPA = GetStyleImageRecolorOPA(Part);
	}
#if EG_DRAW_COMPLEX
	if(Part != EG_PART_MAIN) pDrawImage->m_BlendMode = GetStyleBlendMode(Part);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InititialseDrawLine(uint32_t Part, EGDrawLine *pDrawLine)
{
	pDrawLine->m_OPA = GetStyleLineOPA(Part);
	if(pDrawLine->m_OPA <= EG_OPA_MIN) return;
	EG_OPA_t OPA = GetOPARecursive(this, Part);
	if(OPA <= EG_OPA_MIN) {
		pDrawLine->m_OPA = EG_OPA_TRANSP;
		return;
	}
	if(OPA < EG_OPA_MAX) pDrawLine->m_OPA = (OPA * pDrawLine->m_OPA) >> 8;
	if(pDrawLine->m_OPA <= EG_OPA_MIN) return;
	pDrawLine->m_Width = GetStyleLineWidth(Part);
	if(pDrawLine->m_Width == 0) return;
	pDrawLine->m_Color = GetStyleLineColorFiltered(Part);
	pDrawLine->m_DashWidth = GetStyleLineDashWidth(Part);
	if(pDrawLine->m_DashWidth) {
		pDrawLine->m_DashGap = GetStyleLineDashGap(Part);
	}
	pDrawLine->m_RoundStart = GetStyleLineRounded(Part);
	pDrawLine->m_RoundEnd = pDrawLine->m_RoundStart;
#if EG_DRAW_COMPLEX
	if(Part != EG_PART_MAIN) pDrawLine->m_BlendMode = GetStyleBlendMode(Part);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InititialseDrawArc(uint32_t Part, EGDrawArc *pDrawArc)
{
	pDrawArc->m_Width = GetStyleArcWidth(Part);
	if(pDrawArc->m_Width == 0) return;
	pDrawArc->m_OPA = GetStyleArcOPA(Part);
	if(pDrawArc->m_OPA <= EG_OPA_MIN) return;
	EG_OPA_t OPA = GetOPARecursive(this, Part);
	if(OPA <= EG_OPA_MIN) {
		pDrawArc->m_OPA = EG_OPA_TRANSP;
		return;
	}
	if(OPA < EG_OPA_MAX) pDrawArc->m_OPA = (OPA * pDrawArc->m_OPA) >> 8;
	if(pDrawArc->m_OPA <= EG_OPA_MIN) return;
	pDrawArc->m_Color = GetStyleArcColorFiltered(Part);
	pDrawArc->m_pImageSource = GetStyleArcImageSource(Part);
	pDrawArc->m_Rounded = GetStyleArcRounded(Part);
#if EG_DRAW_COMPLEX
	if(Part != EG_PART_MAIN) pDrawArc->m_BlendMode = GetStyleBlendMode(Part);
#endif
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::CalculateExtDrawSize(uint32_t Part)
{
EG_Coord_t Size = 0;

	EG_Coord_t ShadowWidth = GetStyleShadowWidth(Part);
	if(ShadowWidth) {
		if(GetStyleShadowOPA(Part) > EG_OPA_MIN) {
			ShadowWidth = ShadowWidth / 2 + 1; // The blur adds only half width
			ShadowWidth += GetStyleShadowSpread(Part);
			ShadowWidth += EG_MAX(EG_ABS(GetStyleShadowOffsetX(Part)), EG_ABS(GetStyleShadowOffsetY(Part)));
			Size = EG_MAX(Size, ShadowWidth);
		}
	}
	EG_Coord_t OutlineWidth = GetStyleOutlineWidth(Part);
	if(OutlineWidth) {
		if(GetStyleOutlineOPA(Part) > EG_OPA_MIN) {
			Size = EG_MAX(Size, GetStyleOutlinePadding(Part) + OutlineWidth);
		}
	}
	EG_Coord_t WidthHeight = EG_MAX(GetStyleTransformWidth(Part), GetStyleTransformHeight(Part));
	if(WidthHeight > 0) Size += WidthHeight;
	return Size;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InitDrawDescriptor(EGDrawDiscriptor *pDescriptor, EGDrawContext *pDrawContext)
{
  pDescriptor->m_pContext = pDrawContext;        
  pDescriptor->m_pClass = nullptr;             
  pDescriptor->m_Type = 0;                
  pDescriptor->m_pRect = nullptr;          
  pDescriptor->m_pDrawRect = nullptr;    
  pDescriptor->m_pDrawLabel = nullptr;  
  pDescriptor->m_pDrawLine = nullptr;   
  pDescriptor->m_pDrawImage = nullptr;  
  pDescriptor->m_pDrawArc = nullptr;    
  pDescriptor->m_pPoint1 = nullptr;           
  pDescriptor->m_pPoint2 = nullptr;           
  pDescriptor->m_pText = nullptr;             
  pDescriptor->m_TextLength = 0;         
  pDescriptor->m_Part = 0;               
  pDescriptor->m_Index = 0;              
  pDescriptor->m_Radius = 0;             
  pDescriptor->m_Value = 0;              
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::DrawPartCheckType(EGDrawDiscriptor *pDescriptor, const EG_ClassType_t *pClass, uint32_t Type)
{
	if(pDescriptor->m_pClass == pClass && pDescriptor->m_Type == Type) return true;
	else return false;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::RefreshExtDrawSize(void)
{
	EG_Coord_t SizeOld = GetExtDrawSize();
	EG_Coord_t SizeNew = 0;
	EGEvent::EventSend(this, EG_EVENT_REFR_EXT_DRAW_SIZE, &SizeNew);
	if(SizeNew != SizeOld) Invalidate();
	if(m_pAttributes) m_pAttributes->ExtendedDrawSize = SizeNew;	// Store the result if the special attrs already allocated
	else if(SizeNew != 0) {	// Allocate spec. attrs. only if the result is not zero.
		AllocateAttribute();
		m_pAttributes->ExtendedDrawSize = SizeNew;
	}
	if(SizeNew != SizeOld) Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetExtDrawSize(void) const
{
	if(m_pAttributes != nullptr) return m_pAttributes->ExtendedDrawSize;
	else return 0;
}

/////////////////////////////////////////////////////////////////////////////

EG_LayerType_e EGObject::GetLayerType(void) const
{
// 	EG_LOG_TRACE("Obj:%p, Atributes:%p", (void*)this, (void*)m_pAttributes);
	if(m_pAttributes != nullptr) return (EG_LayerType_e)m_pAttributes->LayerType;
	else return EG_LAYER_TYPE_NONE;
}
