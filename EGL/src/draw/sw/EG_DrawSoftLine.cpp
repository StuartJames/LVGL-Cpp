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

#include <stdbool.h>
#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"     // lv_draw_sw_blend
#include "misc/EG_Math.h"
#include "core/EG_Refresh.h"


//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::DrawLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
const EGDrawContext *pContext = pDrawLine->m_pContext;

//  ESP_LOGI("[Line  ]", "Draw Buffer: %p", (void*)pContext->m_pDrawBuffer);
	if(pPoint1->m_X == pPoint2->m_X && pPoint1->m_Y == pPoint2->m_Y) return;
	EGRect Clip;
	Clip.SetX1(EG_MIN(pPoint1->m_X, pPoint2->m_X) - pDrawLine->m_Width / 2);
	Clip.SetX2(EG_MAX(pPoint1->m_X, pPoint2->m_X) + pDrawLine->m_Width / 2);
	Clip.SetY1(EG_MIN(pPoint1->m_Y, pPoint2->m_Y) - pDrawLine->m_Width / 2);
	Clip.SetY2(EG_MAX(pPoint1->m_Y, pPoint2->m_Y) + pDrawLine->m_Width / 2);
	if(!Clip.Intersect(&Clip, pContext->m_pClipRect)) return;
	const EGRect *pClipRect = pContext->m_pClipRect;
	pContext->m_pClipRect = &Clip;
	if(pPoint1->m_Y == pPoint2->m_Y)	HorizontalLine(pDrawLine, pPoint1, pPoint2);
	else if(pPoint1->m_X == pPoint2->m_X)	VerticalLine(pDrawLine, pPoint1, pPoint2);
	else SkewLine(pDrawLine, pPoint1, pPoint2);
	if(pDrawLine->m_RoundEnd || pDrawLine->m_RoundStart) {
		EGDrawRect DrawRec;
		DrawRec.m_BackgroundColor = pDrawLine->m_Color;
		DrawRec.m_Radius = EG_RADIUS_CIRCLE;
		DrawRec.m_BackgroundOPA = pDrawLine->m_OPA;
		int32_t Radius = (pDrawLine->m_Width >> 1);
		int32_t RadiusCorrection = (pDrawLine->m_Width & 1) ? 0 : 1;
		EGRect CircleArea;
		if(pDrawLine->m_RoundStart) {
			CircleArea.SetX1(pPoint1->m_X - Radius);
			CircleArea.SetY1(pPoint1->m_Y - Radius);
			CircleArea.SetX2(pPoint1->m_X + Radius - RadiusCorrection);
			CircleArea.SetY2(pPoint1->m_Y + Radius - RadiusCorrection);
			DrawRec.Draw(pContext, &CircleArea);
		}
		if(pDrawLine->m_RoundEnd) {
			CircleArea.SetX1(pPoint2->m_X - Radius);
			CircleArea.SetY1(pPoint2->m_Y - Radius);
			CircleArea.SetX2(pPoint2->m_X + Radius - RadiusCorrection);
			CircleArea.SetY2(pPoint2->m_Y + Radius - RadiusCorrection);
			DrawRec.Draw(pContext, &CircleArea);
		}
	}
	pContext->m_pClipRect = pClipRect;
}
//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::HorizontalLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
int32_t Width = pDrawLine->m_Width - 1;
int32_t HalfWidthA = Width >> 1;
int32_t HalfWidthB = HalfWidthA + (Width & 0x1); // Compensate rounding error
const EGSoftContext *pContext = (EGSoftContext*)pDrawLine->m_pContext;

	EGRect BlendArea(EG_MIN(pPoint1->m_X, pPoint2->m_X), EG_MAX(pPoint1->m_X, pPoint2->m_X), pPoint1->m_Y - HalfWidthB, pPoint1->m_Y + HalfWidthA);
	if(!BlendArea.Intersect(&BlendArea, pContext->m_pClipRect)) return;
	bool DoDash = pDrawLine->m_DashGap && pDrawLine->m_DashWidth ? true : false;
	bool SimpleMode = true;
	if(HasAnyDrawMask(&BlendArea))	SimpleMode = false;
	else if(DoDash)	SimpleMode = false;
	EGSoftBlend BlendObj(pContext);
	BlendObj.m_pRect = &BlendArea;
	BlendObj.m_Color = pDrawLine->m_Color;
	BlendObj.m_OPA = pDrawLine->m_OPA;
	// If there is no mask then simply draw a rectangle
	if(SimpleMode) BlendObj.DoBlend();
#if EG_DRAW_COMPLEX
	else {	// If there other mask apply it
		int32_t BlendAreaWidth = BlendArea.GetWidth();
		EG_Coord_t Y2 = BlendArea.GetY2();
		BlendArea.SetY2(BlendArea.GetY1());
		EG_Coord_t DashStart = 0;
		if(DoDash) DashStart = (BlendArea.GetX1()) % (pDrawLine->m_DashGap + pDrawLine->m_DashWidth);
		EG_OPA_t *m_pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(BlendAreaWidth);
		BlendObj.m_pMaskBuffer = m_pMaskBuffer;
		BlendObj.m_pMaskRect = &BlendArea;
		for(int32_t Height = BlendArea.GetY1(); Height <= Y2; Height++) {
			EG_SetMemFF(m_pMaskBuffer, BlendAreaWidth);
			BlendObj.m_MaskResult = DrawMaskApply(m_pMaskBuffer, BlendArea.GetX1(), Height, BlendAreaWidth);
			if(DoDash) {
				if(BlendObj.m_MaskResult != EG_DRAW_MASK_RES_TRANSP) {
					EG_Coord_t DashCount = DashStart;
					for(EG_Coord_t i = 0; i < BlendAreaWidth; i++, DashCount++) {
						if(DashCount <= pDrawLine->m_DashWidth) {
							int16_t diff = pDrawLine->m_DashWidth - DashCount;
							i += diff;
							DashCount += diff;
						}
						else if(DashCount >= pDrawLine->m_DashGap + pDrawLine->m_DashWidth) DashCount = 0;
						else m_pMaskBuffer[i] = 0x00;
					}
					BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
				}
			}
			BlendObj.DoBlend();
			BlendArea.Move(0, 1);
		}
		EG_ReleaseBufferMem(m_pMaskBuffer);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::VerticalLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
int32_t Width = pDrawLine->m_Width - 1;
int32_t HalfWidthA = Width >> 1;
int32_t HalfWidthB = HalfWidthA + (Width & 0x1); // Compensate rounding error
const EGSoftContext *pContext = (EGSoftContext*)pDrawLine->m_pContext;

	EGRect BlendArea;
	BlendArea.SetX1(pPoint1->m_X - HalfWidthB);
	BlendArea.SetX2(pPoint1->m_X + HalfWidthA);
	BlendArea.SetY1(EG_MIN(pPoint1->m_Y, pPoint2->m_Y));
	BlendArea.SetY2(EG_MAX(pPoint1->m_Y, pPoint2->m_Y) - 1);
	if(!BlendArea.Intersect(&BlendArea, pContext->m_pClipRect)) return;
	bool DoDash = pDrawLine->m_DashGap && pDrawLine->m_DashWidth ? true : false;
	bool SimpleMode = true;
	if(HasAnyDrawMask(&BlendArea)) SimpleMode = false;
	else if(DoDash)	SimpleMode = false;
	EGSoftBlend BlendObj(pContext);
	BlendObj.m_pRect = &BlendArea;
	BlendObj.m_Color = pDrawLine->m_Color;
	BlendObj.m_OPA = pDrawLine->m_OPA;
	if(SimpleMode) BlendObj.DoBlend();	// If there is no mask then simply draw a rectangle
#if EG_DRAW_COMPLEX
	else {	// If there other mask apply it
		int32_t DrawWidth = BlendArea.GetWidth();
		EG_Coord_t Y2 = BlendArea.GetY2();
		BlendArea.SetY2(BlendArea.GetY1());
		EG_OPA_t *m_pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(DrawWidth);
		BlendObj.m_pMaskBuffer = m_pMaskBuffer;
		BlendObj.m_pMaskRect = &BlendArea;
		EG_Coord_t DashStart = 0;
		if(DoDash) DashStart = (BlendArea.GetY1()) % (pDrawLine->m_DashGap + pDrawLine->m_DashWidth);
		EG_Coord_t DashCount = DashStart;
		for(int32_t Height = BlendArea.GetY1(); Height <= Y2; Height++) {
			EG_SetMemFF(m_pMaskBuffer, DrawWidth);
			BlendObj.m_MaskResult = DrawMaskApply(m_pMaskBuffer, BlendArea.GetX1(), Height, DrawWidth);
			if(DoDash) {
				if(BlendObj.m_MaskResult != EG_DRAW_MASK_RES_TRANSP) {
					if(DashCount > pDrawLine->m_DashWidth) {
						BlendObj.m_MaskResult = EG_DRAW_MASK_RES_TRANSP;
					}
					if(DashCount >= pDrawLine->m_DashGap + pDrawLine->m_DashWidth) {
						DashCount = 0;
					}
				}
				DashCount++;
			}
			BlendObj.DoBlend();
			BlendArea.Move(0, 1);
		}
		EG_ReleaseBufferMem(m_pMaskBuffer);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::SkewLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2)
{
#if EG_DRAW_COMPLEX
static const uint8_t wcorr[] = {
  128, 128, 128, 129, 129, 130, 130, 131,
  132, 133, 134, 135, 137, 138, 140, 141,
  143, 145, 147, 149, 151, 153, 155, 158,
  160, 162, 165, 167, 170, 173, 175, 178,
  181,
};
EGPoint PointA, PointB;

	if(pPoint1->m_Y < pPoint2->m_Y) {
		PointA.m_Y = pPoint1->m_Y;
		PointB.m_Y = pPoint2->m_Y;
		PointA.m_X = pPoint1->m_X;
		PointB.m_X = pPoint2->m_X;
	}
	else {
		PointA.m_Y = pPoint2->m_Y;
		PointB.m_Y = pPoint1->m_Y;
		PointA.m_X = pPoint2->m_X;
		PointB.m_X = pPoint1->m_X;
	}
	int32_t xdiff = PointB.m_X - PointA.m_X;
	int32_t ydiff = PointB.m_Y - PointA.m_Y;
	bool flat = EG_ABS(xdiff) > EG_ABS(ydiff) ? true : false;
	int32_t Width = pDrawLine->m_Width;
	int32_t wcorr_i = 0;
	if(flat) wcorr_i = (EG_ABS(ydiff) << 5) / EG_ABS(xdiff);
	else wcorr_i = (EG_ABS(xdiff) << 5) / EG_ABS(ydiff);
	Width = (Width * wcorr[wcorr_i] + 63) >> 7; // + 63 for rounding
	int32_t HalfWidthA = Width >> 1;
	int32_t HalfWidthB = HalfWidthA + (Width & 0x1); // Compensate rounding error
	EGRect BlendArea;
	BlendArea.SetX1(EG_MIN(PointA.m_X, PointB.m_X) - Width);
	BlendArea.SetX2(EG_MAX(PointA.m_X, PointB.m_X) + Width);
	BlendArea.SetY1(EG_MIN(PointA.m_Y, PointB.m_Y) - Width);
	BlendArea.SetY2(EG_MAX(PointA.m_Y, PointB.m_Y) + Width);
	//Get the union of `coords` and `clip`. 'clip` is already truncated to the `draw_buf` size in 'lv_refr_area' function
	if(!BlendArea.Intersect(&BlendArea, pDrawLine->m_pContext->m_pClipRect)) return;
	MaskLineParam_t mask_left_param;
	MaskLineParam_t mask_right_param;
	MaskLineParam_t mask_top_param;
	MaskLineParam_t mask_bottom_param;
	if(flat) {
		if(xdiff > 0) {
			DrawMaskSetLinePoints(&mask_left_param, PointA.m_X, PointA.m_Y - HalfWidthA, PointB.m_X, PointB.m_Y - HalfWidthA,
																		EG_DRAW_MASK_LINE_SIDE_LEFT);
			DrawMaskSetLinePoints(&mask_right_param, PointA.m_X, PointA.m_Y + HalfWidthB, PointB.m_X, PointB.m_Y + HalfWidthB,
																		EG_DRAW_MASK_LINE_SIDE_RIGHT);
		}
		else {
			DrawMaskSetLinePoints(&mask_left_param, PointA.m_X, PointA.m_Y + HalfWidthB, PointB.m_X, PointB.m_Y + HalfWidthB,
																		EG_DRAW_MASK_LINE_SIDE_LEFT);
			DrawMaskSetLinePoints(&mask_right_param, PointA.m_X, PointA.m_Y - HalfWidthA, PointB.m_X, PointB.m_Y - HalfWidthA,
																		EG_DRAW_MASK_LINE_SIDE_RIGHT);
		}
	}
	else {
		DrawMaskSetLinePoints(&mask_left_param, PointA.m_X + HalfWidthB, PointA.m_Y, PointB.m_X + HalfWidthB, PointB.m_Y,
																	EG_DRAW_MASK_LINE_SIDE_LEFT);
		DrawMaskSetLinePoints(&mask_right_param, PointA.m_X - HalfWidthA, PointA.m_Y, PointB.m_X - HalfWidthA, PointB.m_Y,
																	EG_DRAW_MASK_LINE_SIDE_RIGHT);
	}
	int16_t mask_left_id = DrawMaskAdd(&mask_left_param, NULL);	// Use the normal vector for the endings
	int16_t mask_right_id = DrawMaskAdd(&mask_right_param, NULL);
	int16_t mask_top_id = EG_MASK_ID_INVALID;
	int16_t mask_bottom_id = EG_MASK_ID_INVALID;
	if(!pDrawLine->m_RawEnd) {
		DrawMaskSetLinePoints(&mask_top_param, PointA.m_X, PointA.m_Y, PointA.m_X - ydiff, PointA.m_Y + xdiff, EG_DRAW_MASK_LINE_SIDE_BOTTOM);
		DrawMaskSetLinePoints(&mask_bottom_param, PointB.m_X, PointB.m_Y, PointB.m_X - ydiff, PointB.m_Y + xdiff, EG_DRAW_MASK_LINE_SIDE_TOP);
		mask_top_id = DrawMaskAdd(&mask_top_param, NULL);
		mask_bottom_id = DrawMaskAdd(&mask_bottom_param, NULL);
	}
	/*The real draw area is around the line. It's easy to calculate with steep lines, but the area can be very wide with
   very flat lines. So deal with it only with steep lines.*/
	int32_t DrawWidth = BlendArea.GetWidth();
	uint32_t hor_res = (uint32_t)GetRefreshingDisplay()->GetHorizontalRes();	// Draw the background line by line
	size_t pMaskBufferSize = EG_MIN(BlendArea.GetSize(), hor_res);
	EG_OPA_t *m_pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(pMaskBufferSize);
	EG_Coord_t Y2 = BlendArea.GetY2();
	BlendArea.SetY2(BlendArea.GetY1());
	uint32_t pMask = 0;
	EG_SetMemFF(m_pMaskBuffer, pMaskBufferSize);
	EGSoftBlend BlendObj((EGSoftContext*)pDrawLine->m_pContext);
	BlendObj.m_pRect = &BlendArea;
	BlendObj.m_Color = pDrawLine->m_Color;
	BlendObj.m_OPA = pDrawLine->m_OPA;
	BlendObj.m_pMaskBuffer = m_pMaskBuffer;
	BlendObj.m_pMaskRect = &BlendArea;
	for(int32_t Height = BlendArea.GetY1(); Height <= Y2; Height++) {	// Fill the first row with 'color'
		BlendObj.m_MaskResult = DrawMaskApply(&m_pMaskBuffer[pMask], BlendArea.GetX1(), Height, DrawWidth);
		if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_TRANSP) {
			EG_ZeroMem(&m_pMaskBuffer[pMask], DrawWidth);
		}
		pMask += DrawWidth;
		if((uint32_t)pMask + DrawWidth < pMaskBufferSize) {
			BlendArea.SetY2(BlendArea.GetY2() + 1);
		}
		else {
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
			BlendObj.DoBlend();
			BlendArea.SetY1(BlendArea.GetY2() + 1);
			BlendArea.SetY2(BlendArea.GetY1());
			pMask = 0;
			EG_SetMemFF(m_pMaskBuffer, pMaskBufferSize);
		}
	}
	if(BlendArea.GetY1() != BlendArea.GetY2()) {	// Flush the last part
		BlendArea.SetY2(BlendArea.GetY2() - 1);
		BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
		BlendObj.DoBlend();
	}
	EG_ReleaseBufferMem(m_pMaskBuffer);
	DrawMaskFreeParam(&mask_left_param);
	DrawMaskFreeParam(&mask_right_param);
	if(mask_top_id != EG_MASK_ID_INVALID) DrawMaskFreeParam(&mask_top_param);
	if(mask_bottom_id != EG_MASK_ID_INVALID) DrawMaskFreeParam(&mask_bottom_param);
	DrawMaskRemove(mask_left_id);
	DrawMaskRemove(mask_right_id);
	DrawMaskRemove(mask_top_id);
	DrawMaskRemove(mask_bottom_id);
#else
	EG_UNUSED(pPoint1);
	EG_UNUSED(pPoint2);
	EG_UNUSED(draw_ctx);
	EG_UNUSED(dsc);
	EG_LOG_WARN("Can't draw skewed line with EG_DRAW_COMPLEX == 0");
#endif
}
