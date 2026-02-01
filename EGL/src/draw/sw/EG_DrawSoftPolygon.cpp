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
 


#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"     // lv_draw_sw_blend
#include "misc/EG_Math.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Point.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Color.h"
#include "draw/EG_DrawRect.h"

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGDrawRect *pRect, const EGPoint *pPoints, uint16_t PointCount)
{
#if EG_DRAW_COMPLEX
	if(PointCount < 3) return;
	if(pPoints == NULL) return;
	EGPoint *pPointArray = (EGPoint *)EG_GetBufferMem(PointCount * sizeof(EGPoint));	// Join adjacent pPoints if they are on the same coordinate
	if(pPointArray == NULL) return;
	uint16_t i;
	uint16_t TotalCount = 0;
	pPointArray[0] = pPoints[0];
	for(i = 0; i < PointCount - 1; i++) {
		if(pPoints[i].m_X != pPoints[i + 1].m_X || pPoints[i].m_Y != pPoints[i + 1].m_Y) {
			pPointArray[TotalCount] = pPoints[i];
			TotalCount++;
		}
	}
	if(pPoints[0].m_X != pPoints[PointCount - 1].m_X || pPoints[0].m_Y != pPoints[PointCount - 1].m_Y) {// The first and the last pPoints are also adjacent
		pPointArray[TotalCount] = pPoints[PointCount - 1];
		TotalCount++;
	}
	PointCount = TotalCount;
	if(PointCount < 3) {
		EG_ReleaseBufferMem(pPointArray);
		return;
	}
	EGRect PolyArea(EG_COORD_MAX, EG_COORD_MAX,  EG_COORD_MIN,  EG_COORD_MIN);
	for(i = 0; i < PointCount; i++) {
		PolyArea.SetX1(EG_MIN(PolyArea.GetX1(), pPointArray[i].m_X));
		PolyArea.SetY1(EG_MIN(PolyArea.GetY1(), pPointArray[i].m_Y));
		PolyArea.SetX2(EG_MAX(PolyArea.GetX2(), pPointArray[i].m_X));
		PolyArea.SetY2(EG_MAX(PolyArea.GetY2(), pPointArray[i].m_Y));
	}
	EGRect ClipRect;
	if(!ClipRect.Intersect(&PolyArea, pDrawPolygon->m_pContext->m_pClipRect)){
		EG_ReleaseBufferMem(pPointArray);
		return;
	}
	const EGRect *pClipRect = pDrawPolygon->m_pContext->m_pClipRect;  // save the original
	pDrawPolygon->m_pContext->m_pClipRect = &ClipRect;
	EG_Coord_t MinY = pPointArray[0].m_Y;	// Find the lowest point
	int16_t TotalY = 0;
	for(i = 1; i < PointCount; i++) {
		if(pPointArray[i].m_Y < MinY) {
			MinY = pPointArray[i].m_Y;
			TotalY = i;
		}
	}
	MaskLineParam_t *pMask = (MaskLineParam_t *)EG_GetBufferMem(sizeof(MaskLineParam_t) * PointCount);
	MaskLineParam_t *pMaskNext = pMask;
	int32_t PreveousLeft = TotalY;
	int32_t PreveousRight = TotalY;
	int32_t NextLeft;
	int32_t NextRight;
	uint32_t MaxCount = 0;
	NextLeft = TotalY - 1;	// Get the index of the left and right pPoints
	if(NextLeft < 0) NextLeft = PointCount + NextLeft;
	NextRight = TotalY + 1;
	if(NextRight > PointCount - 1) NextRight = 0;
	/* Check if the order of pPoints is inverted or not. The normal case is when the left point is on `TotalY - 1`
     * Explanation:
     *   if angle(p_left) < angle(p_right) -> inverted
     *   dy_left/dx_left < dy_right/dx_right
     *   dy_left * dx_right < dy_right * dx_left */
	EG_Coord_t dxl = pPointArray[NextLeft].m_X - pPointArray[TotalY].m_X;
	EG_Coord_t dxr = pPointArray[NextRight].m_X - pPointArray[TotalY].m_X;
	EG_Coord_t dyl = pPointArray[NextLeft].m_Y - pPointArray[TotalY].m_Y;
	EG_Coord_t dyr = pPointArray[NextRight].m_Y - pPointArray[TotalY].m_Y;
	bool inv = false;
	if(dyl * dxr < dyr * dxl) inv = true;
	do {
		if(!inv) {
			NextLeft = PreveousLeft - 1;
			if(NextLeft < 0) NextLeft = PointCount + NextLeft;
			NextRight = PreveousRight + 1;
			if(NextRight > PointCount - 1) NextRight = 0;
		}
		else {
			NextLeft = PreveousLeft + 1;
			if(NextLeft > PointCount - 1) NextLeft = 0;
			NextRight = PreveousRight - 1;
			if(NextRight < 0) NextRight = PointCount + NextRight;
		}
		if(pPointArray[NextLeft].m_Y >= pPointArray[PreveousLeft].m_Y) {
			if(pPointArray[NextLeft].m_Y != pPointArray[PreveousLeft].m_Y && pPointArray[NextLeft].m_X != pPointArray[PreveousLeft].m_X) {
				DrawMaskSetLinePoints(pMaskNext, pPointArray[PreveousLeft].m_X, pPointArray[PreveousLeft].m_Y,
																			pPointArray[NextLeft].m_X, pPointArray[NextLeft].m_Y,
																			EG_DRAW_MASK_LINE_SIDE_RIGHT);
				DrawMaskAdd(pMaskNext, pMask);
				pMaskNext++;
			}
			MaxCount++;
			PreveousLeft = NextLeft;
		}
		if(MaxCount == PointCount) break;
		if(pPointArray[NextRight].m_Y >= pPointArray[PreveousRight].m_Y) {
			if(pPointArray[NextRight].m_Y != pPointArray[PreveousRight].m_Y && pPointArray[NextRight].m_X != pPointArray[PreveousRight].m_X) {
				DrawMaskSetLinePoints(pMaskNext, pPointArray[PreveousRight].m_X, pPointArray[PreveousRight].m_Y,
																			pPointArray[NextRight].m_X, pPointArray[NextRight].m_Y,
																			EG_DRAW_MASK_LINE_SIDE_LEFT);
				DrawMaskAdd(pMaskNext, pMask);
				pMaskNext++;
			}
			MaxCount++;
			PreveousRight = NextRight;
		}
	} while(MaxCount < PointCount);
	DrawRect(pRect, &PolyArea);
	DrawMaskRemoveReferenced(pMask);
	EG_ReleaseBufferMem(pMask);
	EG_ReleaseBufferMem(pPointArray);
	pDrawPolygon->m_pContext->m_pClipRect = pClipRect;
#else
	EG_UNUSED(pPoints);
	EG_UNUSED(PointCount);
	EG_UNUSED(pDrawPolygon);
	EG_UNUSED(draw_dsc);
	EG_LOG_WARN("Can't draw polygon with EG_DRAW_COMPLEX == 0");
#endif // EG_DRAW_COMPLEX
}

