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
 

#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"     // lv_draw_sw_blend
#include "misc/EG_Math.h"
#include "misc/lv_txt_ap.h"
#include "core/EG_Refresh.h"
#include "misc/EG_Assert.h"
#include "draw/sw/EG_DrawSoftDither.h"

//////////////////////////////////////////////////////////////////////////////////////

#define SHADOW_UPSCALE_SHIFT 6
#define SHADOW_ENHANCE 1
#define SPLIT_LIMIT 50

#if defined(EG_SHADOW_CACHE_SIZE) && EG_SHADOW_CACHE_SIZE > 0
static uint8_t sh_cache[EG_SHADOW_CACHE_SIZE * EG_SHADOW_CACHE_SIZE];
static int32_t sh_cache_size = -1;
static int32_t sh_cache_r = -1;
#endif

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawRect(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
#if EG_DRAW_COMPLEX
	DrawShadow(pDrawRect, pRect);
#endif

	DrawRectBackground(pDrawRect, pRect);
	DrawBackgroundImage(pDrawRect, pRect);
	DrawBorder(pDrawRect, pRect);
	DrawOutline(pDrawRect, pRect);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawBackground(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
#if EG_COLOR_SCREEN_TRANSP && EG_COLOR_DEPTH == 32
	EG_ZeroMem(pDrawRect->buf, EG_Rect_get_size(pDrawRect->buf_area) * sizeof(EG_Color_t));
#endif

	DrawRectBackground(pDrawRect, pRect);
	DrawBackgroundImage(pDrawRect, pRect);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawRectBackground(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
bool MaskAnyCenter;
const EGSoftContext *pContext = (EGSoftContext*)pDrawRect->m_pContext;

	if(pDrawRect->m_BackgroundOPA <= EG_OPA_MIN) return;
	EGRect BackgroundRect(pRect);
	// If the border fully covers make the bg area 1px smaller to avoid artifacts on the corners
	if((pDrawRect->m_BorderWidth > 1) && (pDrawRect->m_BorderOPA >= EG_OPA_MAX) && (pDrawRect->m_Radius != 0)) {
		BackgroundRect.Deflate(((pDrawRect->m_BorderSide & EG_BORDER_SIDE_LEFT) ? 1 : 0),
		                      ((pDrawRect->m_BorderSide & EG_BORDER_SIDE_TOP) ? 1 : 0),
		                      ((pDrawRect->m_BorderSide & EG_BORDER_SIDE_RIGHT) ? 1 : 0),
		                      ((pDrawRect->m_BorderSide & EG_BORDER_SIDE_BOTTOM) ? 1 : 0));
	}
	EGRect ClippedArea;
	if(!ClippedArea.Intersect(&BackgroundRect, pContext->m_pClipRect)) return;
	EG_GradDirection_e GradDir = pDrawRect->m_BackgroundGrad.dir;
	EG_Color_t BackgroundColor = (GradDir == EG_GRAD_DIR_NONE) ? pDrawRect->m_BackgroundColor : pDrawRect->m_BackgroundGrad.stops[0].color;
	if(BackgroundColor.full == pDrawRect->m_BackgroundGrad.stops[1].color.full) GradDir = EG_GRAD_DIR_NONE;
	bool MaskAny = HasAnyDrawMask(&BackgroundRect);
 	EGSoftBlend BlendObj(pContext);
	BlendObj.m_BlendMode = pDrawRect->m_BlendMode;
	BlendObj.m_Color = BackgroundColor;
 	if(!MaskAny && pDrawRect->m_Radius == 0 && (GradDir == EG_GRAD_DIR_NONE)) {	// Most Simple case: just a plain rectangle
		BlendObj.m_pRect = &BackgroundRect;
		BlendObj.m_OPA = pDrawRect->m_BackgroundOPA;
	  BlendObj.DoBlend();
  	return;
	}
#if EG_DRAW_COMPLEX == 0                // Complex case: there is gradient, mask, or m_Radius
	EG_LOG_WARN("Can't draw complex rectangle because EG_DRAW_COMPLEX = 0");
#else
	EG_OPA_t BackOPA = pDrawRect->m_BackgroundOPA >= (EG_OPA_t)EG_OPA_MAX ? (EG_OPA_t)EG_OPA_COVER : pDrawRect->m_BackgroundOPA;
	// Get the real m_Radius. Can't be larger than the half of the shortest side 
	EG_Coord_t BackWidth = BackgroundRect.GetWidth();
	EG_Coord_t BackHeight = BackgroundRect.GetHeight();
	int32_t ShortSide = EG_MIN(BackWidth, BackHeight);
	int32_t OutsideRadius = EG_MIN(pDrawRect->m_Radius, ShortSide >> 1);
	int32_t ClippedWidth = ClippedArea.GetWidth();	// Add a m_Radius mask if there is m_Radius
	int16_t MaskOutsideRadiusID = EG_MASK_ID_INVALID;
	EG_OPA_t *pMaskBuffer = NULL;
	MaskRadiusParam_t MaskRadiusOuterParam;
	if(OutsideRadius > 0 || MaskAny) {
		pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(ClippedWidth);
		DrawMaskSetRadius(&MaskRadiusOuterParam, &BackgroundRect, OutsideRadius, false);
		MaskOutsideRadiusID = DrawMaskAdd(&MaskRadiusOuterParam, NULL);
	}
	int32_t Height;
	EGRect BlendRect;
	BlendRect.SetX1(ClippedArea.GetX1());
	BlendRect.SetX2(ClippedArea.GetX2());
	BlendObj.m_pMaskBuffer = pMaskBuffer;
	BlendObj.m_pRect = &BlendRect;
	BlendObj.m_pMaskRect = &BlendRect;
	BlendObj.m_OPA = EG_OPA_COVER;
	EG_GradCacheItem_t *pGrad = EG_GetGradient(&pDrawRect->m_BackgroundGrad, BackWidth, BackHeight);	// Get gradient if appropriate
	if(pGrad && GradDir == EG_GRAD_DIR_HOR) {
		BlendObj.m_pSourceBuffer = pGrad->pMap + ClippedArea.GetX1() - BackgroundRect.GetX1();
	}
#if _DITHER_GRADIENT
	EG_DitherMode_t dither_mode = pDrawRect->m_BackgroundGrad.dither;
	EG_DitherFunc_t dither_func = &EG_DitherNone;
	EG_Coord_t grad_size = BackWidth;
	if(GradDir == EG_GRAD_DIR_VER && dither_mode != EG_DITHER_NONE) {
		//  When dithering, we are still using a map that's changing from line to line
		BlendObj.src_buf = pGrad->pMap;
	}

	if(pGrad && dither_mode == EG_DITHER_NONE) {
		pGrad->filled = 0; // Should we force refilling it each draw call ?
		if(GradDir == EG_GRAD_DIR_VER)
			grad_size = BackHeight;
	}
	else
#if EG_DITHER_ERROR_DIFFUSION
		if(dither_mode == EG_DITHER_ORDERED)
#endif
		switch(GradDir) {
			case EG_GRAD_DIR_HOR:
				dither_func = EG_DitherOrderedHorizontal;
				break;
			case EG_GRAD_DIR_VER:
				dither_func = EG_DitherOrderedVertical;
				break;
			default:
				dither_func = NULL;
		}

#if EG_DITHER_ERROR_DIFFUSION
	else if(dither_mode == EG_DITHER_ERR_DIFF)
		switch(GradDir) {
			case EG_GRAD_DIR_HOR:
				dither_func = EG_DitherErrorDiffHorizontal;
				break;
			case EG_GRAD_DIR_VER:
				dither_func = EG_DitherErrorDiffVertical;
				break;
			default:
				dither_func = NULL;
		}
#endif
#endif
	if(MaskAny) {	// There is another mask too. Draw line by line. 
		for(Height = ClippedArea.GetY1(); Height <= ClippedArea.GetY2(); Height++) {
			BlendRect.SetY1(Height);
			BlendRect.SetY2(Height);
			// Initialize the mask to BackOPA instead of 0xFF and blend with EG_OPA_COVER. It saves calculating the final BackOPA in lv_draw_sw_blend
			EG_SetMem(pMaskBuffer, BackOPA, ClippedWidth);
			BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClippedArea.GetX1(), Height, ClippedWidth);
			if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;

#if _DITHER_GRADIENT
			if(dither_func) dither_func(pGrad, BlendRect.GetX1(), Height - BackgroundRect.GetY1(), grad_size);
#endif
			if(GradDir == EG_GRAD_DIR_VER) BlendObj.m_Color = pGrad->pMap[Height - BackgroundRect.GetY1()];
			BlendObj.DoBlend();
		}
	}
  else{
    for(Height = 0; Height < OutsideRadius; Height++) {	// Draw the top of the rectangle line by line and mirror it to the bottom. 
      EG_Coord_t TopY = BackgroundRect.GetY1() + Height;
      EG_Coord_t BottomY = BackgroundRect.GetY2() - Height;
      if((TopY < ClippedArea.GetY1()) && (BottomY > ClippedArea.GetY2())) continue; // This line is clipped now
      // Initialize the mask to BackOPA instead of 0xFF and blend with EG_OPA_COVER. It saves calculating the final BackOPA in lv_draw_sw_blend
      EG_SetMem(pMaskBuffer, BackOPA, ClippedWidth);
      BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, BlendRect.GetX1(), TopY, ClippedWidth);
      if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
      if(TopY >= ClippedArea.GetY1()) {
        BlendRect.SetY1(TopY);
        BlendRect.SetY2(TopY);
  #if _DITHER_GRADIENT
        if(dither_func) dither_func(pGrad, BlendRect.GetX1(), TopY - BackgroundRect.GetY1(), grad_size);
  #endif
        if(GradDir == EG_GRAD_DIR_VER) BlendObj.m_Color = pGrad->pMap[TopY - BackgroundRect.GetY1()];
        BlendObj.DoBlend();
      }
      if(BottomY <= ClippedArea.GetY2()) {
        BlendRect.SetY1(BottomY);
        BlendRect.SetY2(BottomY);

  #if _DITHER_GRADIENT
        if(dither_func) dither_func(pGrad, BlendRect.GetX1(), BottomY - BackgroundRect.GetY1(), grad_size);
  #endif
        if(GradDir == EG_GRAD_DIR_VER) BlendObj.m_Color = pGrad->pMap[BottomY - BackgroundRect.GetY1()];
        BlendObj.DoBlend();
      }
    }
    // Draw the center of the rectangle. If no other masks and no gradient, the center is a Simple rectangle
    EGRect CanterArea(BackgroundRect.GetX1(), BackgroundRect.GetX2(), BackgroundRect.GetY1() + OutsideRadius, BackgroundRect.GetY2() - OutsideRadius);
    MaskAnyCenter = HasAnyDrawMask(&CanterArea);
    if(!MaskAnyCenter && GradDir == EG_GRAD_DIR_NONE) {
      BlendRect.SetY1(BackgroundRect.GetY1() + OutsideRadius);
      BlendRect.SetY2(BackgroundRect.GetY2() - OutsideRadius);
      BlendObj.m_OPA = BackOPA;
      BlendObj.m_pMaskBuffer = NULL;
      BlendObj.DoBlend();
    }
    else {	// With gradient and/or mask draw line by line
      BlendObj.m_OPA = BackOPA;
      BlendObj.m_MaskResult = EG_DRAW_MASK_RES_FULL_COVER;
      int32_t h_end = BackgroundRect.GetY2() - OutsideRadius;
      for(Height = BackgroundRect.GetY1() + OutsideRadius; Height <= h_end; Height++) {
        // If there is no other mask do not apply mask as in the center there is no m_Radius to mask
        if(MaskAnyCenter) {
          EG_SetMem(pMaskBuffer, BackOPA, ClippedWidth);
          BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClippedArea.GetX1(), Height, ClippedWidth);
        }
        BlendRect.SetY1(Height);
        BlendRect.SetY2(Height);

  #if _DITHER_GRADIENT
        if(dither_func) dither_func(pGrad, BlendRect.GetX1(), Height - BackgroundRect.GetY1(), grad_size);
  #endif
        if(GradDir == EG_GRAD_DIR_VER) BlendObj.m_Color = pGrad->pMap[Height - BackgroundRect.GetY1()];
        BlendObj.DoBlend();
      }
    }
  }
  if(pMaskBuffer) EG_ReleaseBufferMem(pMaskBuffer);
  if(MaskOutsideRadiusID != EG_MASK_ID_INVALID){
    DrawMaskRemove(MaskOutsideRadiusID);
    DrawMaskFreeParam(&MaskRadiusOuterParam);
  }
  if(pGrad){
    EG_GradientCleanup(pGrad);
  }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawBackgroundImage(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
const EGSoftContext *pContext = (EGSoftContext*)pDrawRect->m_pContext;

	if(pDrawRect->m_pBackImageSource == nullptr) return;
	if(pDrawRect->m_BackImageOPA <= EG_OPA_MIN) return;
	EGRect ClipRect;
	if(!ClipRect.Intersect(pRect, pContext->m_pClipRect)) return;
	const EGRect *OriginalClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &ClipRect;
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pDrawRect->m_pBackImageSource);
	if(SourceType == EG_IMG_SRC_SYMBOL) {
		EGPoint size;
		EG_GetTextSize(&size, (char*)pDrawRect->m_pBackImageSource, (EG_Font_t*)pDrawRect->m_pBackImageSymbolFont, 0, 0, EG_COORD_MAX, EG_TEXT_FLAG_NONE);
		EGRect LabelArea;
		LabelArea.SetX1(pRect->GetX1() + pRect->GetWidth() / 2 - size.m_X / 2);
		LabelArea.SetX2(LabelArea.GetX1() + size.m_X - 1);
		LabelArea.SetY1(pRect->GetY1() + pRect->GetHeight() / 2 - size.m_Y / 2);
		LabelArea.SetY2(LabelArea.GetY1() + size.m_Y - 1);
		EGDrawLabel Label;
		Label.m_pFont = (EG_Font_t*)pDrawRect->m_pBackImageSymbolFont;
		Label.m_Color = pDrawRect->m_BackImageRecolor;
		Label.m_OPA = pDrawRect->m_BackImageOPA;
		Label.Draw(pContext, &LabelArea, (char*)pDrawRect->m_pBackImageSource, NULL);
	}
	else {
		EG_ImageHeader_t header;
		EG_Result_t res = EGImageDecoder::GetInfo(pDrawRect->m_pBackImageSource, &header);
		if(res == EG_RES_OK) {
			EGDrawImage Image;
			Image.m_BlendMode = pDrawRect->m_BlendMode;
			Image.m_Recolor = pDrawRect->m_BackImageRecolor;
			Image.m_RecolorOPA = pDrawRect->m_BackImageRecolorOPA;
			Image.m_OPA = pDrawRect->m_BackImageOPA;
			EGRect ImageArea;
			if(pDrawRect->m_BackImageTiled == false) {			// Center align
				ImageArea.SetX1(pRect->GetX1() + pRect->GetWidth() / 2 - header.Width / 2);
				ImageArea.SetY1(pRect->GetY1() + pRect->GetHeight() / 2 - header.Height / 2);
				ImageArea.SetX2(ImageArea.GetX1() + header.Width - 1);
				ImageArea.SetY2(ImageArea.GetY1() + header.Height - 1);
				Image.Draw(pContext, &ImageArea, pDrawRect->m_pBackImageSource);
			}
			else {
				ImageArea.SetY1(pRect->GetY1());
				ImageArea.SetY2(ImageArea.GetY1() + header.Height - 1);
				for(; ImageArea.GetY1() <= pRect->GetY2(); ImageArea.IncY1(header.Height), ImageArea.IncY2(header.Height)) {
					ImageArea.SetX1(pRect->GetX1());
					ImageArea.SetX2(ImageArea.GetX1() + header.Width - 1);
					for(; ImageArea.GetX1() <= pRect->GetX2(); ImageArea.IncX1(header.Width), ImageArea.IncX2(header.Width)) {
						Image.Draw(pContext, &ImageArea, pDrawRect->m_pBackImageSource);
					}
				}
			}
		}
		else {
			EG_LOG_WARN("Couldn't read the background image");
		}
	}
	pContext->m_pClipRect = OriginalClipRect;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawBorder(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
	if(pDrawRect->m_BorderOPA <= EG_OPA_MIN) return;
	if(pDrawRect->m_BorderWidth == 0) return;
	if(pDrawRect->m_BorderSide == EG_BORDER_SIDE_NONE) return;
	if(pDrawRect->m_BorderPost) return;
	int32_t Width = pRect->GetWidth();
	int32_t Height = pRect->GetHeight();
	int32_t OutsideRadius = pDrawRect->m_Radius;
	int32_t ShortSide = EG_MIN(Width, Height);
	if(OutsideRadius > ShortSide >> 1) OutsideRadius = ShortSide >> 1;
	EGRect InnerArea(pRect);	// Get the inner area
	InnerArea.Deflate(((pDrawRect->m_BorderSide & EG_BORDER_SIDE_LEFT) ? pDrawRect->m_BorderWidth : -(pDrawRect->m_BorderWidth + OutsideRadius)),
	                  ((pDrawRect->m_BorderSide & EG_BORDER_SIDE_RIGHT) ? pDrawRect->m_BorderWidth : -(pDrawRect->m_BorderWidth + OutsideRadius)),
	                  ((pDrawRect->m_BorderSide & EG_BORDER_SIDE_TOP) ? pDrawRect->m_BorderWidth : -(pDrawRect->m_BorderWidth + OutsideRadius)),
	                  ((pDrawRect->m_BorderSide & EG_BORDER_SIDE_BOTTOM) ? pDrawRect->m_BorderWidth : -(pDrawRect->m_BorderWidth + OutsideRadius)));
	EG_Coord_t InsideRadius = OutsideRadius - pDrawRect->m_BorderWidth;
	if(InsideRadius < 0) InsideRadius = 0;
	DrawBorderGeneric(pDrawRect, pRect, &InnerArea, OutsideRadius, InsideRadius, pDrawRect->m_BorderColor, pDrawRect->m_BorderOPA, pDrawRect->m_BlendMode);
}

//////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX
void EG_ATTRIBUTE_FAST_MEM EGSoftContext::DrawShadow(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
const EGSoftContext *pContext = (EGSoftContext*)pDrawRect->m_pContext;

	if(pDrawRect->m_ShadowWidth == 0) return;	// Check whether the shadow is visible
	if(pDrawRect->m_ShadowOPA <= EG_OPA_MIN) return;

	if(pDrawRect->m_ShadowWidth == 1 && pDrawRect->m_ShadowSpread <= 0 &&
		 pDrawRect->m_ShadowOffsetX == 0 && pDrawRect->m_ShadowOffsetY == 0) {
		return;
	}
	EGRect CoreArea;	// Calculate the rectangle which is blurred to get the shadow in `ShadowArea`
	CoreArea.SetX1(pRect->GetX1() + pDrawRect->m_ShadowOffsetX - pDrawRect->m_ShadowSpread);
	CoreArea.SetX2(pRect->GetX2() + pDrawRect->m_ShadowOffsetX + pDrawRect->m_ShadowSpread);
	CoreArea.SetY1(pRect->GetY1() + pDrawRect->m_ShadowOffsetY - pDrawRect->m_ShadowSpread);
	CoreArea.SetY2(pRect->GetY2() + pDrawRect->m_ShadowOffsetY + pDrawRect->m_ShadowSpread);
	EGRect ShadowArea;	// Calculate the bounding box of the shadow
	ShadowArea.SetX1(CoreArea.GetX1() - pDrawRect->m_ShadowWidth / 2 - 1);
	ShadowArea.SetX2(CoreArea.GetX2() + pDrawRect->m_ShadowWidth / 2 + 1);
	ShadowArea.SetY1(CoreArea.GetY1() - pDrawRect->m_ShadowWidth / 2 - 1);
	ShadowArea.SetY2(CoreArea.GetY2() + pDrawRect->m_ShadowWidth / 2 + 1);
	EG_OPA_t BackOPA = pDrawRect->m_ShadowOPA;
	if(BackOPA > EG_OPA_MAX) BackOPA = EG_OPA_COVER;
	EGRect DrawArea;	// Get clipped draw area which is the real draw area. It is always the same or inside `ShadowArea`
	if(!DrawArea.Intersect(&ShadowArea, pContext->m_pClipRect)) return;
	EGRect BackArea(pRect);	// Consider 1 px smaller bg to be sure the edge will be covered by the shadow
	BackArea.Deflate(1, 1);
	int32_t BackRadius = pDrawRect->m_Radius;	// Get the clamped m_Radius
	EG_Coord_t ShortSide = EG_MIN(BackArea.GetWidth(), BackArea.GetHeight());
	if(BackRadius > ShortSide >> 1) BackRadius = ShortSide >> 1;
	int32_t ShadowRadius = pDrawRect->m_Radius;	// Get the clamped m_Radius
	ShortSide = EG_MIN(CoreArea.GetWidth(), CoreArea.GetHeight());
	if(ShadowRadius > ShortSide >> 1) ShadowRadius = ShortSide >> 1;
	int32_t CornerSize = pDrawRect->m_ShadowWidth + ShadowRadius;	// Get how many pixels are affected by the blur on the corners

	EG_OPA_t *pShadowBuffer;

#if EG_SHADOW_CACHE_SIZE
	if(sh_cache_size == CornerSize && sh_cache_r == ShadowRadius) {
		// Use the cache if available
		pShadowBuffer = EG_GetBufferMem(CornerSize * CornerSize);
		EG_CopyMem(pShadowBuffer, sh_cache, CornerSize * CornerSize);
	}
	else {
		// A larger buffer is required for calculation
		pShadowBuffer = EG_GetBufferMem(CornerSize * CornerSize * sizeof(uint16_t));
		shadow_draw_corner_buf(&CoreArea, (uint16_t *)pShadowBuffer, pDrawRect->m_ShadowWidth, ShadowRadius);

		// Cache the corner if it fits into the cache size
		if((uint32_t)CornerSize * CornerSize < sizeof(sh_cache)) {
			EG_CopyMem(sh_cache, pShadowBuffer, CornerSize * CornerSize);
			sh_cache_size = CornerSize;
			sh_cache_r = ShadowRadius;
		}
	}
#else
	pShadowBuffer = (EG_OPA_t*)EG_GetBufferMem(CornerSize * CornerSize * sizeof(uint16_t));
	ShadowCornerDrawBuffer(&CoreArea, (uint16_t *)pShadowBuffer, pDrawRect->m_ShadowWidth, ShadowRadius);
#endif
	bool MaskAny = HasAnyDrawMask(&ShadowArea);	// Skip a lot of masking if the background will cover the shadow that would be masked out
	bool Simple = true;
	if(MaskAny || pDrawRect->m_BackgroundOPA < EG_OPA_COVER || pDrawRect->m_BlendMode != EG_BLEND_MODE_NORMAL) Simple = false;

	// Create a m_Radius mask to clip remove shadow on the bg area

	MaskRadiusParam_t MaskRadiusOuterParam;
	int16_t MaskOutsideRadiusID = EG_MASK_ID_INVALID;
	if(!Simple) {
		DrawMaskSetRadius(&MaskRadiusOuterParam, &BackArea, BackRadius, true);
		MaskOutsideRadiusID = DrawMaskAdd(&MaskRadiusOuterParam, NULL);
	}
	EG_OPA_t *pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(ShadowArea.GetWidth());
	EGRect BlendRect;
	EG_OPA_t *ShadowBufferTmp;
	EG_Coord_t y;
	bool SimpleSub;
	EGSoftBlend BlendObj(pContext);
	BlendObj.m_pRect = &BlendRect;
	BlendObj.m_pMaskRect = &BlendRect;
	BlendObj.m_pMaskBuffer = pMaskBuffer;
	BlendObj.m_Color = pDrawRect->m_ShadowColor;
	BlendObj.m_OPA = pDrawRect->m_ShadowOPA;
	BlendObj.m_BlendMode = pDrawRect->m_BlendMode;
	EG_Coord_t HalfWidth = ShadowArea.GetX1() + ShadowArea.GetWidth() / 2;
	EG_Coord_t HalfHeight = ShadowArea.GetY1() + ShadowArea.GetHeight() / 2;
	// Draw the corners if they are on the current clip area and not fully covered by the bg
	// Top right corner
	BlendRect.SetX2(ShadowArea.GetX2());
	BlendRect.SetX1(ShadowArea.GetX2() - CornerSize + 1);
	BlendRect.SetY1(ShadowArea.GetY1());
	BlendRect.SetY2(ShadowArea.GetY1() + CornerSize - 1);
	BlendRect.SetX1(EG_MAX(BlendRect.GetX1(), HalfWidth));	// Do not overdraw the other top corners
	BlendRect.SetY2(EG_MIN(BlendRect.GetY2(), HalfHeight));
	EGRect ClipRectSub;
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (ClipRectSub.GetY1() - ShadowArea.GetY1()) * CornerSize;
		ShadowBufferTmp += ClipRectSub.GetX1() - (ShadowArea.GetX2() - CornerSize + 1);
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;		// Do not mask if out of the bg
		else SimpleSub = Simple;
		if(Width > 0) {
			BlendObj.m_pMaskBuffer = pMaskBuffer;
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED; // In Simple mode it won't be overwritten
			for(y = ClipRectSub.GetY1(); y <= ClipRectSub.GetY2(); y++) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_CopyMem(pMaskBuffer, ShadowBufferTmp, CornerSize);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
				else {
					BlendObj.m_pMaskBuffer = ShadowBufferTmp;
				}
				BlendObj.DoBlend();
				ShadowBufferTmp += CornerSize;
			}
		}
	}
	// Bottom right corner. Almost the same as top right just read the lines of `pShadowBuffer` from then end
	BlendRect.SetX2(ShadowArea.GetX2());
	BlendRect.SetX1(ShadowArea.GetX2() - CornerSize + 1);
	BlendRect.SetY1(ShadowArea.GetY2() - CornerSize + 1);
	BlendRect.SetY2(ShadowArea.GetY2());
	BlendRect.SetX1(EG_MAX(BlendRect.GetX1(), HalfWidth));	// Do not overdraw the other corners
	BlendRect.SetY1(EG_MAX(BlendRect.GetY1(), HalfHeight + 1));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (BlendRect.GetY2() - ClipRectSub.GetY2()) * CornerSize;
		ShadowBufferTmp += ClipRectSub.GetX1() - (ShadowArea.GetX2() - CornerSize + 1);
		// Do not mask if out of the bg
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;
		else SimpleSub = Simple;
		if(Width > 0) {
			BlendObj.m_pMaskBuffer = pMaskBuffer;
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED; // In Simple mode it won't be overwritten
			for(y = ClipRectSub.GetY2(); y >= ClipRectSub.GetY1(); y--) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_CopyMem(pMaskBuffer, ShadowBufferTmp, CornerSize);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
				else {
					BlendObj.m_pMaskBuffer = ShadowBufferTmp;
				}
				BlendObj.DoBlend();
				ShadowBufferTmp += CornerSize;
			}
		}
	}
	// Top side
	BlendRect.SetX1(ShadowArea.GetX1() + CornerSize);
	BlendRect.SetX2(ShadowArea.GetX2() - CornerSize);
	BlendRect.SetY1(ShadowArea.GetY1());
	BlendRect.SetY2(ShadowArea.GetY1() + CornerSize - 1);
	BlendRect.SetY2(EG_MIN(BlendRect.GetY2(), HalfHeight));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (ClipRectSub.GetY1() - BlendRect.GetY1()) * CornerSize;
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;	// Do not mask if out of the bg
		else SimpleSub = Simple;
		if(Width > 0) {
			if(!SimpleSub) BlendObj.m_pMaskBuffer = pMaskBuffer;
			else BlendObj.m_pMaskBuffer = NULL;
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			for(y = ClipRectSub.GetY1(); y <= ClipRectSub.GetY2(); y++) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_SetMem(pMaskBuffer, ShadowBufferTmp[0], Width);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
					BlendObj.DoBlend();
				}
				else {
					BlendObj.m_OPA = BackOPA == EG_OPA_COVER ? ShadowBufferTmp[0] : (ShadowBufferTmp[0] * pDrawRect->m_ShadowOPA) >> 8;
					BlendObj.DoBlend();
				}
				ShadowBufferTmp += CornerSize;
			}
		}
	}
	BlendObj.m_OPA = pDrawRect->m_ShadowOPA; // Restore
	// Bottom side
	BlendRect.SetX1(ShadowArea.GetX1() + CornerSize);
	BlendRect.SetX2(ShadowArea.GetX2() - CornerSize);
	BlendRect.SetY1(ShadowArea.GetY2() - CornerSize + 1);
	BlendRect.SetY2(ShadowArea.GetY2());
	BlendRect.SetY1(EG_MAX(BlendRect.GetY1(), HalfHeight + 1));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (BlendRect.GetY2() - ClipRectSub.GetY2()) * CornerSize;
		if(Width > 0) {
			// Do not mask if out of the bg
			if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;
			else SimpleSub = Simple;
			if(!SimpleSub) BlendObj.m_pMaskBuffer = pMaskBuffer;
			else BlendObj.m_pMaskBuffer = NULL;
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			for(y = ClipRectSub.GetY2(); y >= ClipRectSub.GetY1(); y--) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true; // Do not mask if out of the bg
				else SimpleSub = Simple;
				if(!SimpleSub) {
					EG_SetMem(pMaskBuffer, ShadowBufferTmp[0], Width);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
					BlendObj.DoBlend();
				}
				else {
					BlendObj.m_OPA = BackOPA == EG_OPA_COVER ? ShadowBufferTmp[0] : (ShadowBufferTmp[0] * pDrawRect->m_ShadowOPA) >> 8;
					BlendObj.DoBlend();
				}
				ShadowBufferTmp += CornerSize;
			}
		}
	}
	BlendObj.m_OPA = pDrawRect->m_ShadowOPA; // Restore
	// Right side
	BlendRect.SetX1(ShadowArea.GetX2() - CornerSize + 1);
	BlendRect.SetX2(ShadowArea.GetX2());
	BlendRect.SetY1(ShadowArea.GetY1() + CornerSize);
	BlendRect.SetY2(ShadowArea.GetY2() - CornerSize);
	BlendRect.SetY1(EG_MIN(BlendRect.GetY1(), HalfHeight + 1));	// Do not overdraw the other corners
	BlendRect.SetY2(EG_MAX(BlendRect.GetY2(), HalfHeight));
	BlendRect.SetX1(EG_MAX(BlendRect.GetX1(), HalfWidth));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (CornerSize - 1) * CornerSize;
		ShadowBufferTmp += ClipRectSub.GetX1() - (ShadowArea.GetX2() - CornerSize + 1);
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;	// Do not mask if out of the bg
		else SimpleSub = Simple;
		BlendObj.m_pMaskBuffer = SimpleSub ? ShadowBufferTmp : pMaskBuffer;
		if(Width > 0) {
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED; // In Simple mode it won't be overwritten
			for(y = ClipRectSub.GetY1(); y <= ClipRectSub.GetY2(); y++) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_CopyMem(pMaskBuffer, ShadowBufferTmp, Width);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
				BlendObj.DoBlend();
			}
		}
	}
	// Mirror the shadow corner buffer horizontally
	ShadowBufferTmp = pShadowBuffer;
	for(y = 0; y < CornerSize; y++) {
		int32_t x;
		EG_OPA_t *start = ShadowBufferTmp;
		EG_OPA_t *end = ShadowBufferTmp + CornerSize - 1;
		for(x = 0; x < CornerSize / 2; x++) {
			EG_OPA_t tmp = *start;
			*start = *end;
			*end = tmp;
			start++;
			end--;
		}
		ShadowBufferTmp += CornerSize;
	}
	// Left side
	BlendRect.SetX1(ShadowArea.GetX1());
	BlendRect.SetX2(ShadowArea.GetX1() + CornerSize - 1);
	BlendRect.SetY1(ShadowArea.GetY1() + CornerSize);
	BlendRect.SetY2(ShadowArea.GetY2() - CornerSize);
	// Do not overdraw the other corners
	BlendRect.SetY1(EG_MIN(BlendRect.GetY1(), HalfHeight + 1));
	BlendRect.SetY2(EG_MAX(BlendRect.GetY2(), HalfHeight));
	BlendRect.SetX2(EG_MIN(BlendRect.GetX2(), HalfWidth - 1));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (CornerSize - 1) * CornerSize;
		ShadowBufferTmp += ClipRectSub.GetX1() - BlendRect.GetX1();
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;		// Do not mask if out of the bg
		else SimpleSub = Simple;
		BlendObj.m_pMaskBuffer = SimpleSub ? ShadowBufferTmp : pMaskBuffer;
		if(Width > 0) {
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED; // In Simple mode it won't be overwritten
			for(y = ClipRectSub.GetY1(); y <= ClipRectSub.GetY2(); y++) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_CopyMem(pMaskBuffer, ShadowBufferTmp, Width);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
				BlendObj.DoBlend();
			}
		}
	}
	// Top left corner
	BlendRect.SetX1(ShadowArea.GetX1());
	BlendRect.SetX2(ShadowArea.GetX1() + CornerSize - 1);
	BlendRect.SetY1(ShadowArea.GetY1());
	BlendRect.SetY2(ShadowArea.GetY1() + CornerSize - 1);
	BlendRect.SetX2(EG_MIN(BlendRect.GetX2(), HalfWidth - 1));	// Do not overdraw the other corners
	BlendRect.SetY2(EG_MIN(BlendRect.GetY2(), HalfHeight));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (ClipRectSub.GetY1() - BlendRect.GetY1()) * CornerSize;
		ShadowBufferTmp += ClipRectSub.GetX1() - BlendRect.GetX1();
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;	// Do not mask if out of the bg
		else SimpleSub = Simple;
		BlendObj.m_pMaskBuffer = pMaskBuffer;
		if(Width > 0) {
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED; // In Simple mode it won't be overwritten
			for(y = ClipRectSub.GetY1(); y <= ClipRectSub.GetY2(); y++) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_CopyMem(pMaskBuffer, ShadowBufferTmp, CornerSize);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
				else BlendObj.m_pMaskBuffer = ShadowBufferTmp;
				BlendObj.DoBlend();
				ShadowBufferTmp += CornerSize;
			}
		}
	}
	// Bottom left corner. Almost the same as bottom right just read the lines of `pShadowBuffer` from then end
	BlendRect.SetX1(ShadowArea.GetX1());
	BlendRect.SetX2(ShadowArea.GetX1() + CornerSize - 1);
	BlendRect.SetY1(ShadowArea.GetY2() - CornerSize + 1);
	BlendRect.SetY2(ShadowArea.GetY2());
	// Do not overdraw the other corners
	BlendRect.SetY1(EG_MAX(BlendRect.GetY1(), HalfHeight + 1));
	BlendRect.SetX2(EG_MIN(BlendRect.GetX2(), HalfWidth - 1));
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		ShadowBufferTmp = pShadowBuffer;
		ShadowBufferTmp += (BlendRect.GetY2() - ClipRectSub.GetY2()) * CornerSize;
		ShadowBufferTmp += ClipRectSub.GetX1() - BlendRect.GetX1();
		if(Simple && ClipRectSub.IsOutside(&BackArea, BackRadius)) SimpleSub = true;	// Do not mask if out of the bg
		else SimpleSub = Simple;
		BlendObj.m_pMaskBuffer = pMaskBuffer;
		if(Width > 0) {
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED; // In Simple mode it won't be overwritten
			for(y = ClipRectSub.GetY2(); y >= ClipRectSub.GetY1(); y--) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				if(!SimpleSub) {
					EG_CopyMem(pMaskBuffer, ShadowBufferTmp, CornerSize);
					BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
					if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_FULL_COVER) BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
				else BlendObj.m_pMaskBuffer = ShadowBufferTmp;
				BlendObj.DoBlend();
				ShadowBufferTmp += CornerSize;
			}
		}
	}
	// Draw the center rectangle.
	BlendRect.SetX1(ShadowArea.GetX1() + CornerSize);
	BlendRect.SetX2(ShadowArea.GetX2() - CornerSize);
	BlendRect.SetY1(ShadowArea.GetY1() + CornerSize);
	BlendRect.SetY2(ShadowArea.GetY2() - CornerSize);
	BlendObj.m_pMaskBuffer = pMaskBuffer;
	if(ClipRectSub.Intersect(&BlendRect, pContext->m_pClipRect) && !ClipRectSub.IsInside(&BackArea, BackRadius)) {
		EG_Coord_t Width = ClipRectSub.GetWidth();
		if(Width > 0) {
			BlendRect.SetX1(ClipRectSub.GetX1());
			BlendRect.SetX2(ClipRectSub.GetX2());
			for(y = ClipRectSub.GetY1(); y <= ClipRectSub.GetY2(); y++) {
				BlendRect.SetY1(y);
				BlendRect.SetY2(y);
				EG_SetMemFF(pMaskBuffer, Width);
				BlendObj.m_MaskResult = DrawMaskApply(pMaskBuffer, ClipRectSub.GetX1(), y, Width);
				BlendObj.DoBlend();
			}
		}
	}
	if(!Simple) {
		DrawMaskFreeParam(&MaskRadiusOuterParam);
		DrawMaskRemove(MaskOutsideRadiusID);
	}
	EG_ReleaseBufferMem(pShadowBuffer);
	EG_ReleaseBufferMem(pMaskBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::ShadowCornerDrawBuffer(const EGRect *pRect, uint16_t *pShadowBuffer, EG_Coord_t ShadowWidth, EG_Coord_t Radius)
{
int32_t sw_ori = ShadowWidth;
int32_t size = sw_ori + Radius;

	EGRect ShadowArea(pRect);
	ShadowArea.SetX2(ShadowWidth / 2 + Radius - 1 - ((ShadowWidth & 1) ? 0 : 1));
	ShadowArea.SetY1(ShadowWidth / 2 + 1);
	ShadowArea.SetX1(ShadowArea.GetX2() - pRect->GetWidth());
	ShadowArea.SetY2(ShadowArea.GetY1() + pRect->GetHeight());
	MaskRadiusParam_t MaskParam;
	DrawMaskSetRadius(&MaskParam, &ShadowArea, Radius, false);
#if SHADOW_ENHANCE
	// Set half shadow width width because blur will be repeated
	if(sw_ori == 1)	ShadowWidth = 1;
	else ShadowWidth = sw_ori >> 1;
#endif
	EG_OPA_t *MaskLine = (EG_OPA_t*)EG_GetBufferMem(size);
	uint16_t *sh_ups_tmp_buf = (uint16_t *)pShadowBuffer;
	for(int32_t y = 0; y < size; y++) {
		EG_SetMemFF(MaskLine, size);
		DrawMaskRes_t m_MaskResult = MaskParam.dsc.DrawCB(MaskLine, 0, y, size, &MaskParam);
		if(m_MaskResult == EG_DRAW_MASK_RES_TRANSP) {
			EG_ZeroMem(sh_ups_tmp_buf, size * sizeof(sh_ups_tmp_buf[0]));
		}
		else {
			int32_t i;
			sh_ups_tmp_buf[0] = (MaskLine[0] << SHADOW_UPSCALE_SHIFT) / ShadowWidth;
			for(i = 1; i < size; i++) {
				if(MaskLine[i] == MaskLine[i - 1])
					sh_ups_tmp_buf[i] = sh_ups_tmp_buf[i - 1];
				else
					sh_ups_tmp_buf[i] = (MaskLine[i] << SHADOW_UPSCALE_SHIFT) / ShadowWidth;
			}
		}
		sh_ups_tmp_buf += size;
	}
	EG_ReleaseBufferMem(MaskLine);
	DrawMaskFreeParam(&MaskParam);
	if(ShadowWidth == 1) {
		int32_t i;
		EG_OPA_t *res_buf = (EG_OPA_t *)pShadowBuffer;
		for(i = 0; i < size * size; i++) {
			res_buf[i] = (pShadowBuffer[i] >> SHADOW_UPSCALE_SHIFT);
		}
		return;
	}
	BlurShadowCorner(size, ShadowWidth, pShadowBuffer);
#if SHADOW_ENHANCE == 0
	// The result is required in EG_OPA_t not uint16_t
	uint32_t x;
	EG_OPA_t *res_buf = (EG_OPA_t *)pShadowBuffer;
	for(x = 0; x < size * size; x++) {
		res_buf[x] = pShadowBuffer[x];
	}
#else
	ShadowWidth += sw_ori & 1;
	if(ShadowWidth > 1) {
		uint32_t i;
		uint32_t max_v_div = (EG_OPA_COVER << SHADOW_UPSCALE_SHIFT) / ShadowWidth;
		for(i = 0; i < (uint32_t)size * size; i++) {
			if(pShadowBuffer[i] == 0)
				continue;
			else if(pShadowBuffer[i] == EG_OPA_COVER)
				pShadowBuffer[i] = max_v_div;
			else
				pShadowBuffer[i] = (pShadowBuffer[i] << SHADOW_UPSCALE_SHIFT) / ShadowWidth;
		}
		BlurShadowCorner(size, ShadowWidth, pShadowBuffer);
	}
	int32_t x;
	EG_OPA_t *res_buf = (EG_OPA_t *)pShadowBuffer;
	for(x = 0; x < size * size; x++) {
		res_buf[x] = pShadowBuffer[x];
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::BlurShadowCorner(EG_Coord_t size, EG_Coord_t ShadowWidth, uint16_t *sh_ups_buf)
{
	int32_t s_left = ShadowWidth >> 1;
	int32_t s_right = (ShadowWidth >> 1);
	if((ShadowWidth & 1) == 0) s_left--;

	// Horizontal blur
	uint16_t *sh_ups_blur_buf = (uint16_t*)EG_GetBufferMem(size * sizeof(uint16_t));

	int32_t x;
	int32_t y;

	uint16_t *sh_ups_tmp_buf = sh_ups_buf;

	for(y = 0; y < size; y++) {
		int32_t v = sh_ups_tmp_buf[size - 1] * ShadowWidth;
		for(x = size - 1; x >= 0; x--) {
			sh_ups_blur_buf[x] = v;

			// Forget the right pixel
			uint32_t right_val = 0;
			if(x + s_right < size) right_val = sh_ups_tmp_buf[x + s_right];
			v -= right_val;

			// Add the left pixel
			uint32_t left_val;
			if(x - s_left - 1 < 0)
				left_val = sh_ups_tmp_buf[0];
			else
				left_val = sh_ups_tmp_buf[x - s_left - 1];
			v += left_val;
		}
		EG_CopyMem(sh_ups_tmp_buf, sh_ups_blur_buf, size * sizeof(uint16_t));
		sh_ups_tmp_buf += size;
	}

	// Vertical blur
	uint32_t i;
	uint32_t max_v = EG_OPA_COVER << SHADOW_UPSCALE_SHIFT;
	uint32_t max_v_div = max_v / ShadowWidth;
	for(i = 0; i < (uint32_t)size * size; i++) {
		if(sh_ups_buf[i] == 0)
			continue;
		else if(sh_ups_buf[i] == max_v)
			sh_ups_buf[i] = max_v_div;
		else
			sh_ups_buf[i] = sh_ups_buf[i] / ShadowWidth;
	}

	for(x = 0; x < size; x++) {
		sh_ups_tmp_buf = &sh_ups_buf[x];
		int32_t v = sh_ups_tmp_buf[0] * ShadowWidth;
		for(y = 0; y < size; y++, sh_ups_tmp_buf += size) {
			sh_ups_blur_buf[y] = v < 0 ? 0 : (v >> SHADOW_UPSCALE_SHIFT);

			// Forget the top pixel
			uint32_t top_val;
			if(y - s_right <= 0)
				top_val = sh_ups_tmp_buf[0];
			else
				top_val = sh_ups_buf[(y - s_right) * size + x];
			v -= top_val;

			// Add the bottom pixel
			uint32_t bottom_val;
			if(y + s_left + 1 < size)
				bottom_val = sh_ups_buf[(y + s_left + 1) * size + x];
			else
				bottom_val = sh_ups_buf[(size - 1) * size + x];
			v += bottom_val;
		}

		// Write back the result into `sh_ups_buf`
		sh_ups_tmp_buf = &sh_ups_buf[x];
		for(y = 0; y < size; y++, sh_ups_tmp_buf += size) {
			(*sh_ups_tmp_buf) = sh_ups_blur_buf[y];
		}
	}

	EG_ReleaseBufferMem(sh_ups_blur_buf);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawOutline(const EGDrawRect *pDrawRect, const EGRect *pRect)
{
	if(pDrawRect->m_OutlineOPA <= EG_OPA_MIN) return;
	if(pDrawRect->m_OutlineWidth == 0) return;
	EG_OPA_t OutlineOPA = pDrawRect->m_OutlineOPA;
	if(OutlineOPA > EG_OPA_MAX) OutlineOPA = EG_OPA_COVER;
	EGRect InnerArea(pRect);	// Get the inner m_Radius
	EG_Coord_t Padding = pDrawRect->m_OutlinePadding - 1;	// Bring the outline closer to make sure there is no color bleeding when Padding=0
	InnerArea.Inflate(Padding, Padding);
	EGRect OuterArea(InnerArea);
	OuterArea.Inflate(pDrawRect->m_OutlineWidth, pDrawRect->m_OutlineWidth);
	int32_t inner_w = InnerArea.GetWidth();
	int32_t inner_h = InnerArea.GetHeight();
	int32_t InsideRadius = pDrawRect->m_Radius;
	int32_t ShortSide = EG_MIN(inner_w, inner_h);
	if(InsideRadius > (ShortSide >> 1)) InsideRadius = ShortSide >> 1;
	EG_Coord_t OutsideRadius = InsideRadius + pDrawRect->m_OutlineWidth;
	DrawBorderGeneric(pDrawRect, &OuterArea, &InnerArea, OutsideRadius, InsideRadius, pDrawRect->m_OutlineColor, pDrawRect->m_OutlineOPA,	pDrawRect->m_BlendMode);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawBorderGeneric(const EGDrawRect *pDrawRect, const EGRect *pOuterArea, const EGRect *pInnerArea, EG_Coord_t OutsideRadius, EG_Coord_t InsideRadius, EG_Color_t Color, EG_OPA_t BackOPA, EG_BlendMode_e BlendMode)
{
const EGSoftContext *pContext = (EGSoftContext*)pDrawRect->m_pContext;

	BackOPA = BackOPA >= (EG_OPA_t)EG_OPA_COVER ? (EG_OPA_t)EG_OPA_COVER : BackOPA;
	bool MaskAny = HasAnyDrawMask(pOuterArea);
#if EG_DRAW_COMPLEX
	if(!MaskAny && OutsideRadius == 0 && InsideRadius == 0) {
		DrawBorderSimple(pDrawRect, pOuterArea, pInnerArea, Color, BackOPA);
		return;
	}
	EGRect DrawArea;	// Get clipped draw area which is the real draw area. It is always the same or inside `coords`
	if(!DrawArea.Intersect(pOuterArea, pContext->m_pClipRect)) return;
	int32_t DrawWidth = DrawArea.GetWidth();
	EGSoftBlend BlendObj(pContext);
	BlendObj.m_pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(DrawWidth);
	int16_t MaskOutsideRadiusID = EG_MASK_ID_INVALID;	// Create mask for the outer area
	MaskRadiusParam_t MaskRadiusOuterParam;
	if(OutsideRadius > 0) {
		DrawMaskSetRadius(&MaskRadiusOuterParam, pOuterArea, OutsideRadius, false);
		MaskOutsideRadiusID = DrawMaskAdd(&MaskRadiusOuterParam, NULL);
	}
	MaskRadiusParam_t MaskRadiusInnerParam;	// Create mask for the inner mask
	DrawMaskSetRadius(&MaskRadiusInnerParam, pInnerArea, InsideRadius, true);
	int16_t mask_rin_id = DrawMaskAdd(&MaskRadiusInnerParam, NULL);
	int32_t Height;
	EGRect BlendRect;
	BlendObj.m_pRect = &BlendRect;
	BlendObj.m_pMaskRect = &BlendRect;
	BlendObj.m_Color = Color;
	BlendObj.m_OPA = BackOPA;
	BlendObj.m_BlendMode = BlendMode;
	EGRect CoreArea;	// Calculate the x and y coordinates where the straight parts area
	CoreArea.SetX1(EG_MAX(pOuterArea->GetX1() + OutsideRadius, pInnerArea->GetX1()));
	CoreArea.SetX2(EG_MIN(pOuterArea->GetX2() - OutsideRadius, pInnerArea->GetX2()));
	CoreArea.SetY1(EG_MAX(pOuterArea->GetY1() + OutsideRadius, pInnerArea->GetY1()));
	CoreArea.SetY2(EG_MIN(pOuterArea->GetY2() - OutsideRadius, pInnerArea->GetY2()));
	EG_Coord_t core_w = CoreArea.GetWidth();
	bool top_side = (pOuterArea->GetY1() <= pInnerArea->GetY1()) ? true : false;
	bool bottom_side = (pOuterArea->GetY2() >= pInnerArea->GetY2()) ? true : false;
	if(MaskAny) {	// If there is other masks, need to draw line by line
		BlendRect.SetX1(DrawArea.GetX1());
		BlendRect.SetX2(DrawArea.GetX2());
		for(Height = DrawArea.GetY1(); Height <= DrawArea.GetY2(); Height++) {
			if(!top_side && Height < CoreArea.GetY1()) continue;
			if(!bottom_side && Height > CoreArea.GetY2()) break;
			BlendRect.SetY1(Height);
			BlendRect.SetY2(Height);
			EG_SetMemFF(BlendObj.m_pMaskBuffer, DrawWidth);
			BlendObj.m_MaskResult = DrawMaskApply(BlendObj.m_pMaskBuffer, DrawArea.GetX1(), Height, DrawWidth);
			BlendObj.DoBlend();
		}
		DrawMaskFreeParam(&MaskRadiusInnerParam);
		DrawMaskRemove(mask_rin_id);
		if(MaskOutsideRadiusID != EG_MASK_ID_INVALID) {
			DrawMaskFreeParam(&MaskRadiusOuterParam);
			DrawMaskRemove(MaskOutsideRadiusID);
		}
		EG_ReleaseBufferMem(BlendObj.m_pMaskBuffer);
		return;
	}
	// No masks
	bool LeftSide = pOuterArea->GetX1() <= pInnerArea->GetX1() ? true : false;
	bool RightSide = pOuterArea->GetX2() >= pInnerArea->GetX2() ? true : false;
	bool HorizontalSplit = true;
	if(LeftSide && RightSide && top_side && bottom_side && core_w < SPLIT_LIMIT) {
		HorizontalSplit = false;
	}
	BlendObj.m_MaskResult = EG_DRAW_MASK_RES_FULL_COVER;
	if(top_side && HorizontalSplit) {   	// Draw the straight lines first if they are long enough
		BlendRect.SetX1(CoreArea.GetX1());
		BlendRect.SetX2(CoreArea.GetX2());
		BlendRect.SetY1(pOuterArea->GetY1());
		BlendRect.SetY2(pInnerArea->GetY1() - 1);
		BlendObj.DoBlend();
	}
	if(bottom_side && HorizontalSplit) {
		BlendRect.SetX1(CoreArea.GetX1());
		BlendRect.SetX2(CoreArea.GetX2());
		BlendRect.SetY1(pInnerArea->GetY2() + 1);
		BlendRect.SetY2(pOuterArea->GetY2());
		BlendObj.DoBlend();
	}
	if(LeftSide) {
		BlendRect.SetX1(pOuterArea->GetX1());
		BlendRect.SetX2(pInnerArea->GetX1() - 1);
		BlendRect.SetY1(CoreArea.GetY1());
		BlendRect.SetY2(CoreArea.GetY2());
		BlendObj.DoBlend();
	}
	if(RightSide) {
		BlendRect.SetX1(pInnerArea->GetX2() + 1);
		BlendRect.SetX2(pOuterArea->GetX2());
		BlendRect.SetY1(CoreArea.GetY1());
		BlendRect.SetY2(CoreArea.GetY2());
		BlendObj.DoBlend();
	}
	// Draw the corners
	EG_Coord_t blend_w;
	if(!HorizontalSplit) {	// Left and right corner together if they are close to each other
		BlendRect.SetX1(DrawArea.GetX1());		// Calculate the top corner and mirror it to the bottom
		BlendRect.SetX2(DrawArea.GetX2());
		EG_Coord_t max_h = EG_MAX(OutsideRadius, pInnerArea->GetY1() - pOuterArea->GetY1());
		for(Height = 0; Height < max_h; Height++) {
			EG_Coord_t TopY = pOuterArea->GetY1() + Height;
			EG_Coord_t BottomY = pOuterArea->GetY2() - Height;
			if(TopY < DrawArea.GetY1() && BottomY > DrawArea.GetY2()) continue; // This line is clipped now
			EG_SetMemFF(BlendObj.m_pMaskBuffer, DrawWidth);
			BlendObj.m_MaskResult = DrawMaskApply(BlendObj.m_pMaskBuffer, BlendRect.GetX1(), TopY, DrawWidth);
			if(TopY >= DrawArea.GetY1()) {
				BlendRect.SetY1(TopY);
				BlendRect.SetY2(TopY);
				BlendObj.DoBlend();
			}
			if(BottomY <= DrawArea.GetY2()) {
				BlendRect.SetY1(BottomY);
				BlendRect.SetY2(BottomY);
				BlendObj.DoBlend();
			}
		}
	}
	else {
		// Left corners
		BlendRect.SetX1(DrawArea.GetX1());
		BlendRect.SetX2(EG_MIN(DrawArea.GetX2(), CoreArea.GetX1() - 1));
		blend_w = BlendRect.GetWidth();
		if(blend_w > 0) {
			if(LeftSide || top_side) {
				for(Height = DrawArea.GetY1(); Height < CoreArea.GetY1(); Height++) {
					BlendRect.SetY1(Height);
					BlendRect.SetY2(Height);
					EG_SetMemFF(BlendObj.m_pMaskBuffer, blend_w);
					BlendObj.m_MaskResult = DrawMaskApply(BlendObj.m_pMaskBuffer, BlendRect.GetX1(), Height, blend_w);
				  BlendObj.DoBlend();
				}
			}
			if(LeftSide || bottom_side) {
				for(Height = CoreArea.GetY2() + 1; Height <= DrawArea.GetY2(); Height++) {
					BlendRect.SetY1(Height);
					BlendRect.SetY2(Height);
					EG_SetMemFF(BlendObj.m_pMaskBuffer, blend_w);
					BlendObj.m_MaskResult = DrawMaskApply(BlendObj.m_pMaskBuffer, BlendRect.GetX1(), Height, blend_w);
				  BlendObj.DoBlend();
				}
			}
		}
		// Right corners
		BlendRect.SetX1(EG_MAX(DrawArea.GetX1(), CoreArea.GetX2() + 1));
		BlendRect.SetX2(DrawArea.GetX2());
		blend_w = BlendRect.GetWidth();
		if(blend_w > 0) {
			if(RightSide || top_side) {
				for(Height = DrawArea.GetY1(); Height < CoreArea.GetY1(); Height++) {
					BlendRect.SetY1(Height);
					BlendRect.SetY2(Height);
					EG_SetMemFF(BlendObj.m_pMaskBuffer, blend_w);
					BlendObj.m_MaskResult = DrawMaskApply(BlendObj.m_pMaskBuffer, BlendRect.GetX1(), Height, blend_w);
  				BlendObj.DoBlend();
				}
			}
			if(RightSide || bottom_side) {
				for(Height = CoreArea.GetY2() + 1; Height <= DrawArea.GetY2(); Height++) {
					BlendRect.SetY1(Height);
					BlendRect.SetY2(Height);
					EG_SetMemFF(BlendObj.m_pMaskBuffer, blend_w);
					BlendObj.m_MaskResult = DrawMaskApply(BlendObj.m_pMaskBuffer, BlendRect.GetX1(), Height, blend_w);
				  BlendObj.DoBlend();
				}
			}
		}
	}
	DrawMaskFreeParam(&MaskRadiusInnerParam);
	DrawMaskRemove(mask_rin_id);
	DrawMaskFreeParam(&MaskRadiusOuterParam);
	DrawMaskRemove(MaskOutsideRadiusID);
	EG_ReleaseBufferMem(BlendObj.m_pMaskBuffer);
#else 
	EG_UNUSED(BlendMode);
	EG_UNUSED(OutsideRadius);
	EG_UNUSED(InsideRadius);
	if(!MaskAny) {
		DrawBorderSimple(draw_ctx, pOuterArea, pInnerArea, Color, BackOPA);
		return;
	}
#endif 
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawBorderSimple(const EGDrawRect *pDrawRect, const EGRect *pOuterArea, const EGRect *pInnerArea, EG_Color_t Color, EG_OPA_t BackOPA)
{
	EGRect BlendRect;
	EGSoftBlend BlendObj((EGSoftContext*)pDrawRect->m_pContext);
	BlendObj.m_pRect = &BlendRect;
	BlendObj.m_Color = Color;
	BlendObj.m_OPA = BackOPA;
	bool top_side = pOuterArea->GetY1() <= pInnerArea->GetY1() ? true : false;
	bool bottom_side = pOuterArea->GetY2() >= pInnerArea->GetY2() ? true : false;
	bool LeftSide = pOuterArea->GetX1() <= pInnerArea->GetX1() ? true : false;
	bool RightSide = pOuterArea->GetX2() >= pInnerArea->GetX2() ? true : false;
	// Top
	BlendRect.SetX1(pOuterArea->GetX1());
	BlendRect.SetX2(pOuterArea->GetX2());
	BlendRect.SetY1(pOuterArea->GetY1());
	BlendRect.SetY2(pInnerArea->GetY1() - 1);
	if(top_side) BlendObj.DoBlend();
	// Bottom
	BlendRect.SetY1(pInnerArea->GetY2() + 1);
	BlendRect.SetY2(pOuterArea->GetY2());
	if(bottom_side) BlendObj.DoBlend();
	// Left
	BlendRect.SetX1(pOuterArea->GetX1());
	BlendRect.SetX2(pInnerArea->GetX1() - 1);
	BlendRect.SetY1((top_side) ? pInnerArea->GetY1() : pOuterArea->GetY1());
	BlendRect.SetY2((bottom_side) ? pInnerArea->GetY2() : pOuterArea->GetY2());
	if(LeftSide) BlendObj.DoBlend();
	// Right
	BlendRect.SetX1(pInnerArea->GetX2() + 1);
	BlendRect.SetX2(pOuterArea->GetX2());
	if(RightSide)	BlendObj.DoBlend();

}
