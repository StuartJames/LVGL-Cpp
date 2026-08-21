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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */
 


#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"
#include "misc/EG_Math.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Point.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Color.h"
#include "draw/EG_DrawRect.h"

#if 0
//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGPoint *pVertices, uint16_t VerticesCount)
{
#if EG_DRAW_COMPLEX
uint16_t i;
uint16_t TotalCount = 0;

	if((VerticesCount < 3)  || (pVertices == nullptr)) return;
	EGPoint *pArray = new EGPoint[VerticesCount];	// Join adjacent vertices if they are on the same coordinate
	if(pArray == nullptr) return;
	pArray[0] = pVertices[0];
	for(i = 0; i < VerticesCount - 1; i++) {
		if(pVertices[i].m_X != pVertices[i + 1].m_X || pVertices[i].m_Y != pVertices[i + 1].m_Y) {
			pArray[TotalCount] = pVertices[i];
			TotalCount++;
		}
	}
	if(pVertices[0].m_X != pVertices[VerticesCount - 1].m_X || pVertices[0].m_Y != pVertices[VerticesCount - 1].m_Y) {// The first and the last pPoints are also adjacent
		pArray[TotalCount] = pVertices[VerticesCount - 1];
		TotalCount++;
	}
	VerticesCount = TotalCount;
	if(VerticesCount < 3) {
		EG_ReleaseBufferMem(pArray);
		return;
	}
	EGRect PolyRect(EG_COORD_MAX, EG_COORD_MAX,  EG_COORD_MIN,  EG_COORD_MIN);
	for(i = 0; i < VerticesCount; i++) {																	// define the bounding rectangle
		PolyRect.SetX1(EG_MIN(PolyRect.GetX1(), pArray[i].m_X));
		PolyRect.SetY1(EG_MIN(PolyRect.GetY1(), pArray[i].m_Y));
		PolyRect.SetX2(EG_MAX(PolyRect.GetX2(), pArray[i].m_X));
		PolyRect.SetY2(EG_MAX(PolyRect.GetY2(), pArray[i].m_Y));
	}
	EGRect ClipRect;																									// check that the bounding rectangle is within the clip area
	if(!ClipRect.Intersect(&PolyRect, pDrawPolygon->m_pContext->m_pClipRect)){
		EG_ReleaseBufferMem(pArray);
		return;
	}
	const EGRect *pClipRect = pDrawPolygon->m_pContext->m_pClipRect;  // save the original
	pDrawPolygon->m_pContext->m_pClipRect = &ClipRect;								// set the clip area as the visible part of the bounding box
	int32_t MinY = pArray[0].m_Y;															// Find the lowest point
	int16_t IndexMinY = 0;
	for(i = 1; i < VerticesCount; i++) {
		if(pArray[i].m_Y < MinY) {
			MinY = pArray[i].m_Y;
			IndexMinY = i;
		}
	}
	MaskLineParam_t *pMask = (MaskLineParam_t *)EG_GetBufferMem(sizeof(MaskLineParam_t) * VerticesCount);
	MaskLineParam_t *pMaskNext = pMask;
	int32_t PrevCCW = IndexMinY;
	int32_t PrevCW = IndexMinY;
	uint32_t MaxCount = 0;
	int32_t NextCCW = IndexMinY - 1;	// Get the index of the left and right pPoints
	int32_t NextCW = IndexMinY + 1;
	if(NextCCW < 0) NextCCW = VerticesCount + NextCCW;
	if(NextCW > VerticesCount - 1) NextCW = 0;
	/* Check if the order of pPoints is inverted or not. The normal case is when the left point is on `IndexMinY - 1`
     * Explanation:
     *   if angle(p_left) < angle(p_right) -> inverted
     *   dy_left/dx_left < dy_right/dx_right
     *   dy_left * dx_right < dy_right * dx_left */
	int32_t DiffLeftX = pArray[NextCCW].m_X - pArray[IndexMinY].m_X;
	int32_t DiffRightX = pArray[NextCW].m_X - pArray[IndexMinY].m_X;
	int32_t DiffLeftY = pArray[NextCCW].m_Y - pArray[IndexMinY].m_Y;
	int32_t DiffRightY = pArray[NextCW].m_Y - pArray[IndexMinY].m_Y;
	bool Invert = false;
	if(DiffLeftY * DiffRightX < DiffRightY * DiffLeftX) Invert = true;
	do {
		if(!Invert) {
			NextCCW = PrevCCW - 1;
			if(NextCCW < 0) NextCCW = VerticesCount + NextCCW;
			NextCW = PrevCW + 1;
			if(NextCW > VerticesCount - 1) NextCW = 0;
		}
		else {
			NextCCW = PrevCCW + 1;
			if(NextCCW > VerticesCount - 1) NextCCW = 0;
			NextCW = PrevCW - 1;
			if(NextCW < 0) NextCW = VerticesCount + NextCW;
		}
		if(pArray[NextCCW].m_Y >= pArray[PrevCCW].m_Y) {
			if(pArray[NextCCW].m_Y != pArray[PrevCCW].m_Y && pArray[NextCCW].m_X != pArray[PrevCCW].m_X) {
				DrawMaskSetLinePoints(pMaskNext, pArray[PrevCCW].m_X, pArray[PrevCCW].m_Y,
															pArray[NextCCW].m_X, pArray[NextCCW].m_Y,	EG_DRAW_MASK_LINE_SIDE_RIGHT);
				DrawMaskAdd(pMaskNext, pMask);
				pMaskNext++;
			}
			PrevCCW = NextCCW;
			if(++MaxCount == VerticesCount) break;			// that's it, no more
		}
		if(pArray[NextCW].m_Y >= pArray[PrevCW].m_Y) {
			if(pArray[NextCW].m_Y != pArray[PrevCW].m_Y && pArray[NextCW].m_X != pArray[PrevCW].m_X) {
				DrawMaskSetLinePoints(pMaskNext, pArray[PrevCW].m_X, pArray[PrevCW].m_Y,
															pArray[NextCW].m_X, pArray[NextCW].m_Y,	EG_DRAW_MASK_LINE_SIDE_LEFT);
				DrawMaskAdd(pMaskNext, pMask);
				pMaskNext++;
			}
			MaxCount++;
			PrevCW = NextCW;
		}
	}
	while(MaxCount < VerticesCount);
	EGDrawRect DrawRect;									// Background rectangle that will mask tothe shape of the polygon
	DrawRect.m_BlendMode = pDrawPolygon->m_BlendMode;
	DrawRect.m_BackgroundOPA = pDrawPolygon->m_FillOPA;
	DrawRect.m_BackgroundColor = pDrawPolygon->m_FillColor;
	DrawRect.Draw(pDrawPolygon->m_pContext, &PolyRect);
	DrawMaskRemoveReferenced(pMask);
	EGDrawLine DrawLine;									// Now draw the border arround the poygon
	DrawLine.m_BlendMode = pDrawPolygon->m_BlendMode;
	DrawLine.m_Color = pDrawPolygon->m_Color;
	DrawLine.m_Width = pDrawPolygon->m_Width;
	DrawLine.m_OPA = pDrawPolygon->m_OPA;
	for(int16_t i = 1; i < VerticesCount; ++i) DrawLine.Draw(pDrawPolygon->m_pContext, &pArray[i - 1], &pArray[i]);
	DrawLine.Draw(pDrawPolygon->m_pContext, &pArray[VerticesCount - 1], &pArray[0]);
	EG_ReleaseBufferMem(pMask);
	delete[] pArray;
	pDrawPolygon->m_pContext->m_pClipRect = pClipRect; // restore
#else
	EG_UNUSED(pPoints);
	EG_UNUSED(PointCount);
	EG_UNUSED(pDrawPolygon);
	EG_UNUSED(draw_dsc);
	EG_LOG_WARN("Can't draw polygon with EG_DRAW_COMPLEX == 0");
#endif // EG_DRAW_COMPLEX
}

#else
//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGPoint *pVertices, uint16_t VerticesCount)
{
#if EG_DRAW_COMPLEX
uint16_t i;
uint16_t TotalCount = 0;

	if((VerticesCount < 3)  || (pVertices == nullptr)) return;
	EGPoint *pArray = new EGPoint[VerticesCount];	// Join adjacent vertices if they are on the same coordinate
	if(pArray == nullptr) return;
	pArray[0] = pVertices[0];
	for(i = 0; i < VerticesCount - 1; i++) {
		if(pVertices[i].m_X != pVertices[i + 1].m_X || pVertices[i].m_Y != pVertices[i + 1].m_Y) {
			pArray[TotalCount] = pVertices[i];
			TotalCount++;
		}
	}
	if(pVertices[0].m_X != pVertices[VerticesCount - 1].m_X || pVertices[0].m_Y != pVertices[VerticesCount - 1].m_Y) {// The first and the last pPoints are also adjacent
		pArray[TotalCount] = pVertices[VerticesCount - 1];
		TotalCount++;
	}
	VerticesCount = TotalCount;
	if(VerticesCount < 3) {
		delete[] pArray;
		return;
	}
	EGRect PolyRect(EG_COORD_MAX, EG_COORD_MAX,  EG_COORD_MIN,  EG_COORD_MIN);
	for(i = 0; i < VerticesCount; i++) {																	// define the bounding rectangle
		PolyRect.SetX1(EG_MIN(PolyRect.GetX1(), pArray[i].m_X));
		PolyRect.SetY1(EG_MIN(PolyRect.GetY1(), pArray[i].m_Y));
		PolyRect.SetX2(EG_MAX(PolyRect.GetX2(), pArray[i].m_X));
		PolyRect.SetY2(EG_MAX(PolyRect.GetY2(), pArray[i].m_Y));
	}
	EGRect ClipRect;																									// check that the bounding rectangle is within the clip area
	if(!ClipRect.Intersect(&PolyRect, pDrawPolygon->m_pContext->m_pClipRect)){
		delete[] pArray;
		return;
	}
	const EGRect *pClipRect = pDrawPolygon->m_pContext->m_pClipRect;  // save the original
	pDrawPolygon->m_pContext->m_pClipRect = &ClipRect;								// set the clip area as the visible part of the bounding box
	int32_t MinY = pArray[0].m_Y;															// Find the lowest point
	int16_t IndexMinY = 0;
	for(i = 1; i < VerticesCount; i++) {
		if(pArray[i].m_Y < MinY) {
			MinY = pArray[i].m_Y;
			IndexMinY = i;
		}
	}
	MaskLineParam_t *pMask = (MaskLineParam_t *)EG_GetBufferMem(sizeof(MaskLineParam_t) * VerticesCount);
	MaskLineParam_t *pMaskNext = pMask;
	uint32_t MaxCount = 0;
	int32_t PrevCCW = IndexMinY;			// Set up Mask analysis clockwise and anti-clockwise indices
	int32_t NextCCW = PrevCCW - 1;	// Get the index of the leading vertices
	int32_t PrevCW = IndexMinY;
	int32_t NextCW = PrevCW + 1;
	if(NextCCW < 0) NextCCW = VerticesCount + NextCCW;
	if(NextCW > VerticesCount - 1) NextCW = 0;
	/* Check if the order of vertices is inverted or not. The normal case is when the left vertices is on `IndexMinY - 1`
     * Explanation:
     *   if angle(p_left) < angle(p_right) -> inverted
     *   dy_left/dx_left < dy_right/dx_right
     *   dy_left * dx_right < dy_right * dx_left */
	EGPoint DiffLeft = pArray[IndexMinY].Difference(&pArray[NextCCW]);
	EGPoint DiffRight = pArray[IndexMinY].Difference(&pArray[NextCW]);
	bool Invert = false;
	if(DiffLeft.m_Y * DiffRight.m_X < DiffRight.m_Y * DiffLeft.m_X) Invert = true;
	do {
		if(!Invert) {
			NextCCW = PrevCCW - 1;
			if(NextCCW < 0) NextCCW = VerticesCount + NextCCW;
			NextCW = PrevCW + 1;
			if(NextCW > VerticesCount - 1) NextCW = 0;
		}
		else {
			NextCCW = PrevCCW + 1;
			if(NextCCW > VerticesCount - 1) NextCCW = 0;
			NextCW = PrevCW - 1;
			if(NextCW < 0) NextCW = VerticesCount + NextCW;
		}
		if(pArray[NextCCW].m_Y >= pArray[PrevCCW].m_Y) {
			if(pArray[NextCCW].m_Y != pArray[PrevCCW].m_Y && pArray[NextCCW].m_X != pArray[PrevCCW].m_X) {
				DrawMaskSetLinePoints(pMaskNext, pArray[PrevCCW], pArray[NextCCW],	EG_DRAW_MASK_LINE_SIDE_RIGHT);
				DrawMaskAdd(pMaskNext, pMask);
				pMaskNext++;
			}
			PrevCCW = NextCCW;
			if(++MaxCount == VerticesCount) break;			// that's it, no more
		}
		if(pArray[NextCW].m_Y >= pArray[PrevCW].m_Y) {
			if(pArray[NextCW].m_Y != pArray[PrevCW].m_Y && pArray[NextCW].m_X != pArray[PrevCW].m_X) {
				DrawMaskSetLinePoints(pMaskNext, pArray[PrevCW], pArray[NextCW],	EG_DRAW_MASK_LINE_SIDE_LEFT);
				DrawMaskAdd(pMaskNext, pMask);
				pMaskNext++;
			}
			MaxCount++;
			PrevCW = NextCW;
		}
	}
	while(MaxCount < VerticesCount);
	EGDrawRect DrawRect;									// Background rectangle that will mask tothe shape of the polygon
	DrawRect.m_BlendMode = pDrawPolygon->m_BlendMode;
	DrawRect.m_BackgroundOPA = pDrawPolygon->m_FillOPA;
	DrawRect.m_BackgroundColor = pDrawPolygon->m_FillColor;
	DrawRect.Draw(pDrawPolygon->m_pContext, &PolyRect);
	DrawMaskRemoveReferenced(pMask);
	EG_ReleaseBufferMem(pMask);
	EGDrawLine DrawLine;									// Now draw the border arround the poygon
	DrawLine.m_BlendMode = pDrawPolygon->m_BlendMode;
	DrawLine.m_Color = pDrawPolygon->m_Color;
	DrawLine.m_Width = pDrawPolygon->m_Width;
	DrawLine.m_OPA = pDrawPolygon->m_OPA;
	for(int16_t i = 1; i < VerticesCount; ++i) DrawLine.Draw(pDrawPolygon->m_pContext, &pArray[i - 1], &pArray[i]);
	DrawLine.Draw(pDrawPolygon->m_pContext, &pArray[VerticesCount - 1], &pArray[0]);
	delete[] pArray;
	pDrawPolygon->m_pContext->m_pClipRect = pClipRect; // Restore clip area
#else
	EG_UNUSED(pVertices);
	EG_UNUSED(VerticesCount);
	EG_UNUSED(pDrawPolygon);
	EG_LOG_WARN("Can't draw polygon with EG_DRAW_COMPLEX == 0");
#endif
}

#endif

