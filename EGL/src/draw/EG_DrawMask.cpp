/*
 *        Copyright (pCircle) 2025-2026 HydraSystems..
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
#if EG_DRAW_COMPLEX
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"
#include "misc/EG_Assert.h"
#include "misc/lv_gc.h"

//namespace EG_DrawMask
//{

/////////////////////////////////////////////////////////////////////////////////

#define CIRCLE_CACHE_LIFE_MAX 1000
#define CIRCLE_CACHE_AGING(life, r) life = EG_MIN(life + (r < 16 ? 1 : (r >> 4)), 1000)

static DrawMaskRes_t   DrawMaskLine(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskLineParam_t *pParam);
static DrawMaskRes_t   DrawMaskRadius(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskRadiusParam_t *pParam);
static DrawMaskRes_t   DrawMaskAngle(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskAngleParam_t *pParam);
static DrawMaskRes_t   DrawMaskFade(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskFadeParam_t *pParam);
static DrawMaskRes_t   DrawMaskMap(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskMapParam_t *pParam);
static DrawMaskRes_t   DrawMaskPolygon(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskPolygonParam_t *pParam);
static DrawMaskRes_t   DrawMaskFlat(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskLineParam_t *pParam);
static DrawMaskRes_t   DrawMaskSteep(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskLineParam_t *pParam);
static void            CircleInitialise(EGPoint *pCircle, EG_Coord_t *pTemp, EG_Coord_t radius);
static bool            CircleContinue(EGPoint *pCircle);
static void            CircleNext(EGPoint *pCircle, EG_Coord_t *pTemp);
static void            CircleCalc_aa4(MaskRadiusCircleDiscrpt_t *pCircle, EG_Coord_t radius);
static EG_OPA_t *      GetNextLine(MaskRadiusCircleDiscrpt_t *pCircle, EG_Coord_t y, EG_Coord_t *len, EG_Coord_t *x_start);
static EG_OPA_t        MaskMix(EG_OPA_t mask_act, EG_OPA_t mask_new);

  /////////////////////////////////////////////////////////////////////////////////

int16_t DrawMaskAdd(void *pParam, void *pID)
{
uint8_t Index;

  for(Index = 0; Index < _EG_MASK_MAX_NUM; Index++) {	// Look for a free entry
		if(EG_GC_ROOT(EG_DrawMaskArray[Index]).pParam == nullptr) break;
	}
	if(Index >= _EG_MASK_MAX_NUM) {
		EG_LOG_WARN("lv_mask_add: no free slots availabe for mask");
		return EG_MASK_ID_INVALID;
	}
	EG_GC_ROOT(EG_DrawMaskArray[Index]).pParam = pParam;
	EG_GC_ROOT(EG_DrawMaskArray[Index]).pReference = pID;
	return Index;
}

/////////////////////////////////////////////////////////////////////////////////

DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskApply(EG_OPA_t *pMaskBuffer, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length)
{
bool changed = false;
MaskCommonDiscrpt_t *pDiscripter;

	EG_DrawMaskList_t *pMask = EG_GC_ROOT(EG_DrawMaskArray);
	while(pMask->pParam) {
		pDiscripter = (MaskCommonDiscrpt_t *)pMask->pParam;
		DrawMaskRes_t res = EG_DRAW_MASK_RES_FULL_COVER;
		res = pDiscripter->DrawCB(pMaskBuffer, AbsX, AbsY, Length, (void *)pMask->pParam);
		if(res == EG_DRAW_MASK_RES_TRANSP) return EG_DRAW_MASK_RES_TRANSP;
		else if(res == EG_DRAW_MASK_RES_CHANGED) changed = true;
		pMask++;
	}
	return changed ? EG_DRAW_MASK_RES_CHANGED : EG_DRAW_MASK_RES_FULL_COVER;
}

/////////////////////////////////////////////////////////////////////////////////

DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskApplyIDs(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, const int16_t *pIndexes, int16_t Count)
{
bool changed = false;
MaskCommonDiscrpt_t *pDiscripter;

	for(int i = 0; i < Count; i++) {
		int16_t Index = pIndexes[i];
		if(Index == EG_MASK_ID_INVALID) continue;
		pDiscripter = (MaskCommonDiscrpt_t *)EG_GC_ROOT(EG_DrawMaskArray[Index]).pParam;
		if(!pDiscripter) continue;
		DrawMaskRes_t Res = EG_DRAW_MASK_RES_FULL_COVER;
		Res = pDiscripter->DrawCB(pMaskArray, AbsX, AbsY, Length, pDiscripter);
		if(Res == EG_DRAW_MASK_RES_TRANSP) return EG_DRAW_MASK_RES_TRANSP;
		else if(Res == EG_DRAW_MASK_RES_CHANGED) changed = true;
	}
	return changed ? EG_DRAW_MASK_RES_CHANGED : EG_DRAW_MASK_RES_FULL_COVER;
}

/////////////////////////////////////////////////////////////////////////////////

void* DrawMaskRemove(int16_t Index)
{
	MaskCommonDiscrpt_t *pDiscripter = nullptr;

	if((Index >= 0) && (Index < _EG_MASK_MAX_NUM)) {
		pDiscripter = (MaskCommonDiscrpt_t *)EG_GC_ROOT(EG_DrawMaskArray[Index]).pParam;
		EG_GC_ROOT(EG_DrawMaskArray[Index]).pParam = nullptr;
		EG_GC_ROOT(EG_DrawMaskArray[Index]).pReference = nullptr;
	}
	return pDiscripter;
}

/////////////////////////////////////////////////////////////////////////////////

void* DrawMaskRemoveReferenced(void *pReference)
{
	MaskCommonDiscrpt_t *pDiscripter = nullptr;
	for(uint8_t i = 0; i < _EG_MASK_MAX_NUM; i++) {
		if(EG_GC_ROOT(EG_DrawMaskArray[i]).pReference == pReference) {
			pDiscripter = (MaskCommonDiscrpt_t *)EG_GC_ROOT(EG_DrawMaskArray[i]).pParam;
			DrawMaskRemove(i);
		}
	}
	return pDiscripter;
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskFreeParam(void *pParam)
{
	MaskCommonDiscrpt_t *pDiscripter = (MaskCommonDiscrpt_t *)pParam;
	if(pDiscripter->Type == EG_DRAW_MASK_TYPE_RADIUS) {
		MaskRadiusParam_t *pRadius = (MaskRadiusParam_t *)pParam;
		if(pRadius->pCircle) {
			if(pRadius->pCircle->Life < 0) {
				EG_FreeMem(pRadius->pCircle->pCircleOPA);
				EG_FreeMem(pRadius->pCircle);
			}
			else {
				pRadius->pCircle->UsedCount--;
			}
		}
	}
	else if(pDiscripter->Type == EG_DRAW_MASK_TYPE_POLYGON) {
		MaskPolygonParam_t *pPoly = (MaskPolygonParam_t *)pParam;
		EG_FreeMem(pPoly->cfg.pPoints);
	}
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskCleanup(void)
{
	for(uint8_t i = 0; i < EG_CIRCLE_CACHE_SIZE; i++) {
		if(EG_GC_ROOT(_lv_circle_cache[i]).pBuffer) {
			EG_FreeMem(EG_GC_ROOT(_lv_circle_cache[i]).pBuffer);
		}
		EG_ZeroMem(&EG_GC_ROOT(_lv_circle_cache[i]), sizeof(EG_GC_ROOT(_lv_circle_cache[i])));
	}
}

/////////////////////////////////////////////////////////////////////////////////

uint8_t EG_ATTRIBUTE_FAST_MEM DrawMaskGetCount(void)
{
uint8_t Count = 0;

	for(uint8_t i = 0; i < _EG_MASK_MAX_NUM; i++) if(EG_GC_ROOT(EG_DrawMaskArray[i]).pParam) Count++;
	return Count;
}

/////////////////////////////////////////////////////////////////////////////////

bool HasAnyDrawMask(const EGRect *pRect)
{
	if(pRect == nullptr) return EG_GC_ROOT(EG_DrawMaskArray[0]).pParam ? true : false;
	for(uint8_t i = 0; i < _EG_MASK_MAX_NUM; i++) {
		MaskCommonDiscrpt_t *pParam = (MaskCommonDiscrpt_t *)EG_GC_ROOT(EG_DrawMaskArray[i]).pParam;
		if(pParam == nullptr) continue;
		if(pParam->Type == EG_DRAW_MASK_TYPE_RADIUS) {
			MaskRadiusParam_t *pParam = (MaskRadiusParam_t *)EG_GC_ROOT(EG_DrawMaskArray[i]).pParam;
			if(pParam->cfg.Outer) {
				if(!pRect->IsOutside(&pParam->cfg.Area, pParam->cfg.Radius)) return true;
			}
			else if(!pRect->IsInside(&pParam->cfg.Area, pParam->cfg.Radius)) return true;
		}
		else return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetLinePoints(MaskLineParam_t *pParam, EG_Coord_t Point1X, EG_Coord_t Point1Y, EG_Coord_t Point2X,
																	 EG_Coord_t Point2Y, MaskLineSide_t Side)
{
//	EG_ZeroMem(pParam, sizeof(MaskLineParam_t));
	if(Point1Y == Point2Y && Side == EG_DRAW_MASK_LINE_SIDE_BOTTOM) {
		Point1Y--;
		Point2Y--;
	}
	if(Point1Y > Point2Y) {
		EG_Coord_t Temp;
		Temp = Point2X;
		Point2X = Point1X;
		Point1X = Temp;
		Temp = Point2Y;
		Point2Y = Point1Y;
		Point1Y = Temp;
	}
	pParam->cfg.Point1.m_X = Point1X;
	pParam->cfg.Point1.m_Y = Point1Y;
	pParam->cfg.Point2.m_X = Point2X;
	pParam->cfg.Point2.m_Y = Point2Y;
	pParam->cfg.Side = Side;
	pParam->Origin.m_X = Point1X;
	pParam->Origin.m_Y = Point1Y;
	pParam->Flat = (EG_ABS(Point2X - Point1X) > EG_ABS(Point2Y - Point1Y)) ? 1 : 0;
	pParam->SteepYX = 0;
	pParam->SteepXY = 0;
	pParam->dsc.DrawCB = (DrawMaskCB)DrawMaskLine;
	pParam->dsc.Type = EG_DRAW_MASK_TYPE_LINE;
	int32_t dx = Point2X - Point1X;
	int32_t dy = Point2Y - Point1Y;
	if(pParam->Flat) {		// Normalize the steep. Delta x should be relative to delta x = 1024
		int32_t m;
		if(dx) {
			m = (1L << 20) / dx; // m is multiplier to normalize y (upscaled by 1024)
			pParam->SteepYX = (m * dy) >> 10;
		}
		if(dy) {
			m = (1L << 20) / dy; // m is multiplier to normalize x (upscaled by 1024)
			pParam->SteepXY = (m * dx) >> 10;
		}
		pParam->Steep = pParam->SteepYX;
	}
	else {		// Normalize the steep. Delta y should be relative to delta x = 1024
		int32_t m;
		if(dy) {
			m = (1L << 20) / dy; // m is multiplier to normalize x (upscaled by 1024)
			pParam->SteepXY = (m * dx) >> 10;
		}
		if(dx) {
			m = (1L << 20) / dx; // m is multiplier to normalize x (upscaled by 1024)
			pParam->SteepYX = (m * dy) >> 10;
		}
		pParam->Steep = pParam->SteepXY;
	}
	if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_LEFT) pParam->Invert = 0;
	else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_RIGHT) pParam->Invert = 1;
	else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_TOP) {
		if(pParam->Steep > 0) pParam->Invert = 1;
		else pParam->Invert = 0;
	}
	else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_BOTTOM) {
		if(pParam->Steep > 0) pParam->Invert = 0;
		else pParam->Invert = 1;
	}
	pParam->SteepPixel = pParam->Steep >> 2;
	if(pParam->Steep < 0) pParam->SteepPixel = -pParam->SteepPixel;
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetLineAngle(MaskLineParam_t *pParam, EG_Coord_t p1x, EG_Coord_t py, int16_t angle,	MaskLineSide_t side)
{
	/* Find an optimal degree.
     *lv_mask_line_points_init will swap the pPoints to keep the smaller y in Point1
     *Theoretically a line with `angle` or `angle+180` is the same only the pPoints are swapped
     *Find the degree which keeps the origo in place*/
	if(angle > 180) angle -= 180; // > 180 will swap the origo
	int32_t Point2X;
	int32_t Point2Y;
	Point2X = (EG_TrigoSin(angle + 90) >> 5) + p1x;
	Point2Y = (EG_TrigoSin(angle) >> 5) + py;
	DrawMaskSetLinePoints(pParam, p1x, py, Point2X, Point2Y, side);
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetAngle(MaskAngleParam_t *pParam, EG_Coord_t vertex_x, EG_Coord_t vertex_y,
														 EG_Coord_t StartAngle, EG_Coord_t EndAngle)
{
	MaskLineSide_t start_side;
	MaskLineSide_t end_side;
	// Constrain the input angles
	if(StartAngle < 0)	StartAngle = 0;
	else if(StartAngle > 359) StartAngle = 359;
	if(EndAngle < 0)	EndAngle = 0;
	else if(EndAngle > 359) EndAngle = 359;
	if(EndAngle < StartAngle) {
		pParam->DeltaDeg = 360 - StartAngle + EndAngle;
	}
	else {
		pParam->DeltaDeg = EG_ABS(EndAngle - StartAngle);
	}
	pParam->cfg.StartAngle = StartAngle;
	pParam->cfg.EndAngle = EndAngle;
	pParam->cfg.Vertex.m_X = vertex_x;
	pParam->cfg.Vertex.m_Y = vertex_y;
	pParam->dsc.DrawCB = (DrawMaskCB)DrawMaskAngle;
	pParam->dsc.Type = EG_DRAW_MASK_TYPE_ANGLE;
	EG_ASSERT_MSG(StartAngle >= 0 && StartAngle <= 360, "Unexpected start angle");
	if(StartAngle >= 0 && StartAngle < 180) {
		start_side = EG_DRAW_MASK_LINE_SIDE_LEFT;
	}
	else start_side = EG_DRAW_MASK_LINE_SIDE_RIGHT; // silence compiler
	EG_ASSERT_MSG(EndAngle >= 0 && StartAngle <= 360, "Unexpected end angle");
	if(EndAngle >= 0 && EndAngle < 180) {
		end_side = EG_DRAW_MASK_LINE_SIDE_RIGHT;
	}
	else if(EndAngle >= 180 && EndAngle < 360) {
		end_side = EG_DRAW_MASK_LINE_SIDE_LEFT;
	}
	else end_side = EG_DRAW_MASK_LINE_SIDE_RIGHT; // silence compiler
	DrawMaskSetLineAngle(&pParam->StartLine, vertex_x, vertex_y, StartAngle, start_side);
	DrawMaskSetLineAngle(&pParam->EndLine, vertex_x, vertex_y, EndAngle, end_side);
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetRadius(MaskRadiusParam_t *pParam, const EGRect *pRect, EG_Coord_t Radius, bool Invert)
{
uint32_t i;

	EG_Coord_t Width = pRect->GetWidth();
	EG_Coord_t Height = pRect->GetHeight();
	int32_t ShortSide = EG_MIN(Width, Height);
	if(Radius > ShortSide >> 1) Radius = ShortSide >> 1;
	if(Radius < 0) Radius = 0;
	pRect->Copy(&pParam->cfg.Area);
	pParam->cfg.Radius = Radius;
	pParam->cfg.Outer = Invert ? 1 : 0;
	pParam->dsc.DrawCB = (DrawMaskCB)DrawMaskRadius;
	pParam->dsc.Type = EG_DRAW_MASK_TYPE_RADIUS;
	if(Radius == 0) {
		pParam->pCircle = nullptr;
		return;
	}
	for(i = 0; i < EG_CIRCLE_CACHE_SIZE; i++) {	// Try to reuse a pCircle cache entry
		if(EG_GC_ROOT(_lv_circle_cache[i]).Radius == Radius) {
			EG_GC_ROOT(_lv_circle_cache[i]).UsedCount++;
			CIRCLE_CACHE_AGING(EG_GC_ROOT(_lv_circle_cache[i]).Life, Radius);
			pParam->pCircle = &EG_GC_ROOT(_lv_circle_cache[i]);
			return;
		}
	}
	MaskRadiusCircleDiscrpt_t *pEntry = nullptr;	// If not found find a free entry with lowest life
	for(i = 0; i < EG_CIRCLE_CACHE_SIZE; i++) {
		if(EG_GC_ROOT(_lv_circle_cache[i]).UsedCount == 0) {
			if(!pEntry) pEntry = &EG_GC_ROOT(_lv_circle_cache[i]);
			else if(EG_GC_ROOT(_lv_circle_cache[i]).Life < pEntry->Life)	pEntry = &EG_GC_ROOT(_lv_circle_cache[i]);
		}
	}
	if(!pEntry) {
		pEntry = (MaskRadiusCircleDiscrpt_t *)EG_AllocMem(sizeof(MaskRadiusCircleDiscrpt_t));
		EG_ASSERT_MALLOC(pEntry);
		EG_ZeroMem(pEntry, sizeof(MaskRadiusCircleDiscrpt_t));
		pEntry->Life = -1;
	}
	else {
		pEntry->UsedCount++;
		pEntry->Life = 0;
		CIRCLE_CACHE_AGING(pEntry->Life, Radius);
	}
	pParam->pCircle = pEntry;
	CircleCalc_aa4(pParam->pCircle, Radius);
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetFade(MaskFadeParam_t *pParam, const EGRect *pRect, EG_OPA_t TopOPA,
														EG_Coord_t TopY,
														EG_OPA_t BottomOPA, EG_Coord_t BottomY)
{
	pRect->Copy(&pParam->cfg.Area);
	pParam->cfg.TopOPA = TopOPA;
	pParam->cfg.BottomOPA = BottomOPA;
	pParam->cfg.TopY = TopY;
	pParam->cfg.BottomY = BottomY;
	pParam->dsc.DrawCB = (DrawMaskCB)DrawMaskFade;
	pParam->dsc.Type = EG_DRAW_MASK_TYPE_FADE;
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetMap(MaskMapParam_t *pParam, const EGRect *pRect, const EG_OPA_t *pMap)
{
	pRect->Copy(&pParam->cfg.Area);
	pParam->cfg.pMap = pMap;
	pParam->dsc.DrawCB = (DrawMaskCB)DrawMaskMap;
	pParam->dsc.Type = EG_DRAW_MASK_TYPE_MAP;
}

/////////////////////////////////////////////////////////////////////////////////

void DrawMaskSetPolygon(MaskPolygonParam_t *pParam, const EGPoint *pPoints, uint16_t PointCount)
{
	// Join adjacent pPoints if they are on the same coordinate
	EGPoint *pArray = (EGPoint *)EG_AllocMem(PointCount * sizeof(EGPoint));
	if(pArray == nullptr) return;
	uint16_t pcnt = 0;
	pArray[0] = pPoints[0];
	for(uint16_t i = 0; i < PointCount - 1; i++) {
		if(pPoints[i].m_X != pPoints[i + 1].m_X || pPoints[i].m_Y != pPoints[i + 1].m_Y) {
			pArray[pcnt] = pPoints[i];
			pcnt++;
		}
	}
	// The first and the last pPoints are also adjacent
	if(pPoints[0].m_X != pPoints[PointCount - 1].m_X || pPoints[0].m_Y != pPoints[PointCount - 1].m_Y) {
		pArray[pcnt] = pPoints[PointCount - 1];
		pcnt++;
	}
	pParam->cfg.pPoints = pArray;
	pParam->cfg.PointCount = pcnt;
	pParam->dsc.DrawCB = (DrawMaskCB)DrawMaskPolygon;
	pParam->dsc.Type = EG_DRAW_MASK_TYPE_POLYGON;
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskLine(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskLineParam_t *pParam)
{
	AbsY -= pParam->Origin.m_Y;	// Make to pPoints relative to the vertex
	AbsX -= pParam->Origin.m_X;
	if(pParam->Steep == 0) {	// Handle special cases
		// Horizontal
		if(pParam->Flat) {
			// Non sense: Can't be on the right/left of a horizontal line
			if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_LEFT || pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_RIGHT) return EG_DRAW_MASK_RES_FULL_COVER;
			else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_TOP && AbsY + 1 < 0) return EG_DRAW_MASK_RES_FULL_COVER;
			else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_BOTTOM && AbsY > 0)	return EG_DRAW_MASK_RES_FULL_COVER;
			else return EG_DRAW_MASK_RES_TRANSP;
		}
		// Vertical
		else {
			// Non sense: Can't be on the top/bottom of a vertical line
			if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_TOP || pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_BOTTOM)	return EG_DRAW_MASK_RES_FULL_COVER;
			else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_RIGHT && AbsX > 0) return EG_DRAW_MASK_RES_FULL_COVER;
			else if(pParam->cfg.Side == EG_DRAW_MASK_LINE_SIDE_LEFT) {
				if(AbsX + Length < 0)	return EG_DRAW_MASK_RES_FULL_COVER;
				else {
					int32_t k = -AbsX;
					if(k < 0) return EG_DRAW_MASK_RES_TRANSP;
					if(k >= 0 && k < Length) EG_ZeroMem(&pMaskArray[k], Length - k);
					return EG_DRAW_MASK_RES_CHANGED;
				}
			}
			else {
				if(AbsX + Length < 0)	return EG_DRAW_MASK_RES_TRANSP;
				else {
					int32_t k = -AbsX;
					if(k < 0) k = 0;
					if(k >= Length) return EG_DRAW_MASK_RES_TRANSP;
					else if(k >= 0 && k < Length) EG_ZeroMem(&pMaskArray[0], k);
					return EG_DRAW_MASK_RES_CHANGED;
				}
			}
		}
	}
	DrawMaskRes_t res;
	if(pParam->Flat) {
		res = DrawMaskFlat(pMaskArray, AbsX, AbsY, Length, pParam);
	}
	else {
		res = DrawMaskSteep(pMaskArray, AbsX, AbsY, Length, pParam);
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskFlat(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskLineParam_t *pParam)
{
	int32_t y_at_x;
	y_at_x = (int32_t)((int32_t)pParam->SteepYX * AbsX) >> 10;

	if(pParam->SteepYX > 0) {
		if(y_at_x > AbsY) {
			if(pParam->Invert) {
				return EG_DRAW_MASK_RES_FULL_COVER;
			}
			else {
				return EG_DRAW_MASK_RES_TRANSP;
			}
		}
	}
	else {
		if(y_at_x < AbsY) {
			if(pParam->Invert) {
				return EG_DRAW_MASK_RES_FULL_COVER;
			}
			else {
				return EG_DRAW_MASK_RES_TRANSP;
			}
		}
	}
	// At the end of the mask if the limit line is smaller than the mask's y. Then the mask is in the "good" area
	y_at_x = (int32_t)((int32_t)pParam->SteepYX * (AbsX + Length)) >> 10;
	if(pParam->SteepYX > 0) {
		if(y_at_x < AbsY) {
			if(pParam->Invert) {
				return EG_DRAW_MASK_RES_TRANSP;
			}
			else {
				return EG_DRAW_MASK_RES_FULL_COVER;
			}
		}
	}
	else {
		if(y_at_x > AbsY) {
			if(pParam->Invert) {
				return EG_DRAW_MASK_RES_TRANSP;
			}
			else {
				return EG_DRAW_MASK_RES_FULL_COVER;
			}
		}
	}
	int32_t xe;
	if(pParam->SteepYX > 0)	xe = ((AbsY * 256) * pParam->SteepXY) >> 10;
	else xe = (((AbsY + 1) * 256) * pParam->SteepXY) >> 10;
	int32_t xei = xe >> 8;
	int32_t xef = xe & 0xFF;
	int32_t px_h;
	if(xef == 0) px_h = 255;
	else px_h = 255 - (((255 - xef) * pParam->SteepPixel) >> 8);
	int32_t k = xei - AbsX;
	EG_OPA_t MaskNem;
	if(xef) {
		if(k >= 0 && k < Length) {
			MaskNem = 255 - (((255 - xef) * (255 - px_h)) >> 9);
			if(pParam->Invert) MaskNem = 255 - MaskNem;
			pMaskArray[k] = MaskMix(pMaskArray[k], MaskNem);
		}
		k++;
	}
	while(px_h > pParam->SteepPixel) {
		if(k >= 0 && k < Length) {
			MaskNem = px_h - (pParam->SteepPixel >> 1);
			if(pParam->Invert) MaskNem = 255 - MaskNem;
			pMaskArray[k] = MaskMix(pMaskArray[k], MaskNem);
		}
		px_h -= pParam->SteepPixel;
		k++;
		if(k >= Length) break;
	}
	if(k < Length && k >= 0) {
		int32_t x_inters = (px_h * pParam->SteepXY) >> 10;
		MaskNem = (x_inters * px_h) >> 9;
		if(pParam->SteepYX < 0) MaskNem = 255 - MaskNem;
		if(pParam->Invert) MaskNem = 255 - MaskNem;
		pMaskArray[k] = MaskMix(pMaskArray[k], MaskNem);
	}

	if(pParam->Invert) {
		k = xei - AbsX;
		if(k > Length) {
			return EG_DRAW_MASK_RES_TRANSP;
		}
		if(k >= 0) {
			EG_ZeroMem(&pMaskArray[0], k);
		}
	}
	else {
		k++;
		if(k < 0) {
			return EG_DRAW_MASK_RES_TRANSP;
		}
		if(k <= Length) {
			EG_ZeroMem(&pMaskArray[k], Length - k);
		}
	}

	return EG_DRAW_MASK_RES_CHANGED;
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskSteep(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY,
																																EG_Coord_t Length,
																																MaskLineParam_t *pParam)
{
	int32_t k;
	int32_t x_at_y;
	// At the beginning of the mask if the limit line is greater than the mask's y. Then the mask is in the "wrong" area
	x_at_y = (int32_t)((int32_t)pParam->SteepXY * AbsY) >> 10;
	if(pParam->SteepXY > 0) x_at_y++;
	if(x_at_y < AbsX) {
		if(pParam->Invert) return EG_DRAW_MASK_RES_FULL_COVER;
		else return EG_DRAW_MASK_RES_TRANSP;
	}
	// At the end of the mask if the limit line is smaller than the mask's y. Then the mask is in the "good" area
	x_at_y = (int32_t)((int32_t)pParam->SteepXY * (AbsY)) >> 10;
	if(x_at_y > AbsX + Length) {
		if(pParam->Invert) return EG_DRAW_MASK_RES_TRANSP;
		else return EG_DRAW_MASK_RES_FULL_COVER;
	}
	// X start
	int32_t xs = ((AbsY * 256) * pParam->SteepXY) >> 10;
	int32_t xsi = xs >> 8;
	int32_t xsf = xs & 0xFF;
	// X end
	int32_t xe = (((AbsY + 1) * 256) * pParam->SteepXY) >> 10;
	int32_t xei = xe >> 8;
	int32_t xef = xe & 0xFF;
	EG_OPA_t m;
	k = xsi - AbsX;
	if(xsi != xei && (pParam->SteepXY < 0 && xsf == 0)) {
		xsf = 0xFF;
		xsi = xei;
		k--;
	}
	if(xsi == xei) {
		if(k >= 0 && k < Length) {
			m = (xsf + xef) >> 1;
			if(pParam->Invert) m = 255 - m;
			pMaskArray[k] = MaskMix(pMaskArray[k], m);
		}
		k++;
		if(pParam->Invert) {
			k = xsi - AbsX;
			if(k >= Length) {
				return EG_DRAW_MASK_RES_TRANSP;
			}
			if(k >= 0) EG_ZeroMem(&pMaskArray[0], k);
		}
		else {
			if(k > Length) k = Length;
			if(k == 0)
				return EG_DRAW_MASK_RES_TRANSP;
			else if(k > 0)
				EG_ZeroMem(&pMaskArray[k], Length - k);
		}
	}
	else {
		int32_t y_inters;
		if(pParam->SteepXY < 0) {
			y_inters = (xsf * (-pParam->SteepYX)) >> 10;
			if(k >= 0 && k < Length) {
				m = (y_inters * xsf) >> 9;
				if(pParam->Invert) m = 255 - m;
				pMaskArray[k] = MaskMix(pMaskArray[k], m);
			}
			k--;
			int32_t x_inters = ((255 - y_inters) * (-pParam->SteepXY)) >> 10;
			if(k >= 0 && k < Length) {
				m = 255 - (((255 - y_inters) * x_inters) >> 9);
				if(pParam->Invert) m = 255 - m;
				pMaskArray[k] = MaskMix(pMaskArray[k], m);
			}
			k += 2;
			if(pParam->Invert) {
				k = xsi - AbsX - 1;
				if(k > Length)	k = Length;
				else if(k > 0) EG_ZeroMem(&pMaskArray[0], k);
			}
			else {
				if(k > Length) return EG_DRAW_MASK_RES_FULL_COVER;
				if(k >= 0) EG_ZeroMem(&pMaskArray[k], Length - k);
			}
		}
		else {
			y_inters = ((255 - xsf) * pParam->SteepYX) >> 10;
			if(k >= 0 && k < Length) {
				m = 255 - ((y_inters * (255 - xsf)) >> 9);
				if(pParam->Invert) m = 255 - m;
				pMaskArray[k] = MaskMix(pMaskArray[k], m);
			}
			k++;
			int32_t x_inters = ((255 - y_inters) * pParam->SteepXY) >> 10;
			if(k >= 0 && k < Length) {
				m = ((255 - y_inters) * x_inters) >> 9;
				if(pParam->Invert) m = 255 - m;
				pMaskArray[k] = MaskMix(pMaskArray[k], m);
			}
			k++;
			if(pParam->Invert) {
				k = xsi - AbsX;
				if(k > Length) return EG_DRAW_MASK_RES_TRANSP;
				if(k >= 0) EG_ZeroMem(&pMaskArray[0], k);
			}
			else {
				if(k > Length) k = Length;
				if(k == 0)
					return EG_DRAW_MASK_RES_TRANSP;
				else if(k > 0)
					EG_ZeroMem(&pMaskArray[k], Length - k);
			}
		}
	}
	return EG_DRAW_MASK_RES_CHANGED;
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskAngle(EG_OPA_t *pMaskArray, EG_Coord_t AbsX,
																																	 EG_Coord_t AbsY, EG_Coord_t Length,
																																	 MaskAngleParam_t *pParam)
{
	int32_t rel_y = AbsY - pParam->cfg.Vertex.m_Y;
	int32_t rel_x = AbsX - pParam->cfg.Vertex.m_X;
	if(pParam->cfg.StartAngle < 180 && pParam->cfg.EndAngle < 180 &&
		 pParam->cfg.StartAngle != 0 && pParam->cfg.EndAngle != 0 &&
		 pParam->cfg.StartAngle > pParam->cfg.EndAngle) {
		if(AbsY < pParam->cfg.Vertex.m_Y) {
			return EG_DRAW_MASK_RES_FULL_COVER;
		}
		// Start angle mask can work only from the end of end angle mask
		int32_t end_angle_first = (rel_y * pParam->EndLine.SteepXY) >> 10;
		int32_t start_angle_last = ((rel_y + 1) * pParam->StartLine.SteepXY) >> 10;
		// Do not let the line end cross the vertex else it will affect the opposite part
		if(pParam->cfg.StartAngle > 270 && pParam->cfg.StartAngle <= 359 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.StartAngle > 0 && pParam->cfg.StartAngle <= 90 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.StartAngle > 90 && pParam->cfg.StartAngle < 270 && start_angle_last > 0) start_angle_last = 0;
		if(pParam->cfg.EndAngle > 270 && pParam->cfg.EndAngle <= 359 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.EndAngle > 0 && pParam->cfg.EndAngle <= 90 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.EndAngle > 90 && pParam->cfg.EndAngle < 270 && start_angle_last > 0) start_angle_last = 0;
		int32_t dist = (end_angle_first - start_angle_last) >> 1;
		DrawMaskRes_t res1 = EG_DRAW_MASK_RES_FULL_COVER;
		DrawMaskRes_t res2 = EG_DRAW_MASK_RES_FULL_COVER;
		int32_t tmp = start_angle_last + dist - rel_x;
		if(tmp > Length) tmp = Length;
		if(tmp > 0) {
			res1 = DrawMaskLine(&pMaskArray[0], AbsX, AbsY, tmp, &pParam->StartLine);
			if(res1 == EG_DRAW_MASK_RES_TRANSP) {
				EG_ZeroMem(&pMaskArray[0], tmp);
			}
		}
		if(tmp > Length) tmp = Length;
		if(tmp < 0) tmp = 0;
		res2 = DrawMaskLine(&pMaskArray[tmp], AbsX + tmp, AbsY, Length - tmp, &pParam->EndLine);
		if(res2 == EG_DRAW_MASK_RES_TRANSP) {
			EG_ZeroMem(&pMaskArray[tmp], Length - tmp);
		}
		if(res1 == res2) return res1;
		else return EG_DRAW_MASK_RES_CHANGED;
	}
	else if(pParam->cfg.StartAngle > 180 && pParam->cfg.EndAngle > 180 && pParam->cfg.StartAngle > pParam->cfg.EndAngle) {
		if(AbsY > pParam->cfg.Vertex.m_Y) {
			return EG_DRAW_MASK_RES_FULL_COVER;
		}
		// Start angle mask can work only from the end of end angle mask
		int32_t end_angle_first = (rel_y * pParam->EndLine.SteepXY) >> 10;
		int32_t start_angle_last = ((rel_y + 1) * pParam->StartLine.SteepXY) >> 10;
		// Do not let the line end cross the vertex else it will affect the opposite part
		if(pParam->cfg.StartAngle > 270 && pParam->cfg.StartAngle <= 359 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.StartAngle > 0 && pParam->cfg.StartAngle <= 90 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.StartAngle > 90 && pParam->cfg.StartAngle < 270 && start_angle_last > 0) start_angle_last = 0;
		if(pParam->cfg.EndAngle > 270 && pParam->cfg.EndAngle <= 359 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.EndAngle > 0 && pParam->cfg.EndAngle <= 90 && start_angle_last < 0)	start_angle_last = 0;
		else if(pParam->cfg.EndAngle > 90 && pParam->cfg.EndAngle < 270 && start_angle_last > 0) start_angle_last = 0;
		int32_t dist = (end_angle_first - start_angle_last) >> 1;
		DrawMaskRes_t res1 = EG_DRAW_MASK_RES_FULL_COVER;
		DrawMaskRes_t res2 = EG_DRAW_MASK_RES_FULL_COVER;
		int32_t tmp = start_angle_last + dist - rel_x;
		if(tmp > Length) tmp = Length;
		if(tmp > 0) {
			res1 = DrawMaskLine(&pMaskArray[0], AbsX, AbsY, tmp, (MaskLineParam_t *)&pParam->EndLine);
			if(res1 == EG_DRAW_MASK_RES_TRANSP) {
				EG_ZeroMem(&pMaskArray[0], tmp);
			}
		}
		if(tmp > Length) tmp = Length;
		if(tmp < 0) tmp = 0;
		res2 = DrawMaskLine(&pMaskArray[tmp], AbsX + tmp, AbsY, Length - tmp, (MaskLineParam_t *)&pParam->StartLine);
		if(res2 == EG_DRAW_MASK_RES_TRANSP) {
			EG_ZeroMem(&pMaskArray[tmp], Length - tmp);
		}
		if(res1 == res2) return res1;
		else return EG_DRAW_MASK_RES_CHANGED;
	}
	else {
		DrawMaskRes_t res1 = EG_DRAW_MASK_RES_FULL_COVER;
		DrawMaskRes_t res2 = EG_DRAW_MASK_RES_FULL_COVER;
		if(pParam->cfg.StartAngle == 180) {
			if(AbsY < pParam->cfg.Vertex.m_Y) res1 = EG_DRAW_MASK_RES_FULL_COVER;
			else res1 = EG_DRAW_MASK_RES_UNKNOWN;
		}
		else if(pParam->cfg.StartAngle == 0) {
			if(AbsY < pParam->cfg.Vertex.m_Y) res1 = EG_DRAW_MASK_RES_UNKNOWN;
			else res1 = EG_DRAW_MASK_RES_FULL_COVER;
		}
		else if((pParam->cfg.StartAngle < 180 && AbsY < pParam->cfg.Vertex.m_Y) ||	(pParam->cfg.StartAngle > 180 && AbsY >= pParam->cfg.Vertex.m_Y)) {
			res1 = EG_DRAW_MASK_RES_UNKNOWN;
		}
		else {
			res1 = DrawMaskLine(pMaskArray, AbsX, AbsY, Length, &pParam->StartLine);
		}
		if(pParam->cfg.EndAngle == 180) {
			if(AbsY < pParam->cfg.Vertex.m_Y) res2 = EG_DRAW_MASK_RES_UNKNOWN;
			else res2 = EG_DRAW_MASK_RES_FULL_COVER;
		}
		else if(pParam->cfg.EndAngle == 0) {
			if(AbsY < pParam->cfg.Vertex.m_Y) res2 = EG_DRAW_MASK_RES_FULL_COVER;
			else res2 = EG_DRAW_MASK_RES_UNKNOWN;
		}
		else if((pParam->cfg.EndAngle < 180 && AbsY < pParam->cfg.Vertex.m_Y) ||	(pParam->cfg.EndAngle > 180 && AbsY >= pParam->cfg.Vertex.m_Y)) {
			res2 = EG_DRAW_MASK_RES_UNKNOWN;
		}
		else {
			res2 = DrawMaskLine(pMaskArray, AbsX, AbsY, Length, &pParam->EndLine);
		}
		if(res1 == EG_DRAW_MASK_RES_TRANSP || res2 == EG_DRAW_MASK_RES_TRANSP) return EG_DRAW_MASK_RES_TRANSP;
		else if(res1 == EG_DRAW_MASK_RES_UNKNOWN && res2 == EG_DRAW_MASK_RES_UNKNOWN)	return EG_DRAW_MASK_RES_TRANSP;
		else if(res1 == EG_DRAW_MASK_RES_FULL_COVER && res2 == EG_DRAW_MASK_RES_FULL_COVER)	return EG_DRAW_MASK_RES_FULL_COVER;
		else return EG_DRAW_MASK_RES_CHANGED;
	}
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskRadius(EG_OPA_t *pMaskArray, EG_Coord_t AbsX,	EG_Coord_t AbsY, EG_Coord_t Length,	MaskRadiusParam_t *pParam)
{
	bool Outer = pParam->cfg.Outer;
	int32_t Radius = pParam->cfg.Radius;
	EGRect Area(pParam->cfg.Area);
	if(Outer == false) {
		if((AbsY < Area.GetY1() || AbsY > Area.GetY2())) {
			return EG_DRAW_MASK_RES_TRANSP;
		}
	}
	else {
		if(AbsY < Area.GetY1() || AbsY > Area.GetY2()) {
			return EG_DRAW_MASK_RES_FULL_COVER;
		}
	}

	if((AbsX >= Area.GetX1() + Radius && AbsX + Length <= Area.GetX2() - Radius) ||
		 (AbsY >= Area.GetY1() + Radius && AbsY <= Area.GetY2() - Radius)) {
		if(Outer == false) {
			// Remove the edges
			int32_t last = Area.GetX1() - AbsX;
			if(last > Length) return EG_DRAW_MASK_RES_TRANSP;
			if(last >= 0) {
				EG_ZeroMem(&pMaskArray[0], last);
			}

			int32_t first = Area.GetX2() - AbsX + 1;
			if(first <= 0)
				return EG_DRAW_MASK_RES_TRANSP;
			else if(first < Length) {
				EG_ZeroMem(&pMaskArray[first], Length - first);
			}
			if(last == 0 && first == Length)
				return EG_DRAW_MASK_RES_FULL_COVER;
			else
				return EG_DRAW_MASK_RES_CHANGED;
		}
		else {
			int32_t first = Area.GetX1() - AbsX;
			if(first < 0) first = 0;
			if(first <= Length) {
				int32_t last = Area.GetX2() - AbsX - first + 1;
				if(first + last > Length) last = Length - first;
				if(last >= 0) {
					EG_ZeroMem(&pMaskArray[first], last);
				}
			}
		}
		return EG_DRAW_MASK_RES_CHANGED;
	}
	//    printf("exec: x:%d.. %d, y:%d: r:%d, %s\n", AbsX, AbsX + Length - 1, AbsY, pParam->cfg.Radius, pParam->cfg.Outer ? "Invert" : "norm");

	//    if( AbsX == 276 && AbsX + Length - 1 == 479 && AbsY == 63 && pParam->cfg.Radius == 5 && pParam->cfg.Outer == 1) {
	//        char x = 0;
	//    }
	//exec: x:276.. 479, y:63: r:5, Invert)

	int32_t k = Area.GetX1() - AbsX; // First relevant coordinate on the of the mask
	int32_t Width = Area.GetWidth();
	int32_t Height = Area.GetHeight();
	AbsX -= Area.GetX1();
	AbsY -= Area.GetY1();

	EG_Coord_t aa_len;
	EG_Coord_t x_start;
	EG_Coord_t cir_y;
	if(AbsY < Radius) cir_y = Radius - AbsY - 1;
	else cir_y = AbsY - (Height - Radius);
	EG_OPA_t *aa_opa = GetNextLine(pParam->pCircle, cir_y, &aa_len, &x_start);
	EG_Coord_t cir_x_right = k + Width - Radius + x_start;
	EG_Coord_t cir_x_left = k + Radius - x_start - 1;
	EG_Coord_t i;
	if(Outer == false) {
		for(i = 0; i < aa_len; i++) {
			EG_OPA_t opa = aa_opa[aa_len - i - 1];
			if(cir_x_right + i >= 0 && cir_x_right + i < Length) {
				pMaskArray[cir_x_right + i] = MaskMix(opa, pMaskArray[cir_x_right + i]);
			}
			if(cir_x_left - i >= 0 && cir_x_left - i < Length) {
				pMaskArray[cir_x_left - i] = MaskMix(opa, pMaskArray[cir_x_left - i]);
			}
		}
		// Clean the right side
		cir_x_right = EG_CLAMP(0, cir_x_right + i, Length);
		EG_ZeroMem(&pMaskArray[cir_x_right], Length - cir_x_right);
		// Clean the left side
		cir_x_left = EG_CLAMP(0, cir_x_left - aa_len + 1, Length);
		EG_ZeroMem(&pMaskArray[0], cir_x_left);
	}
	else {
		for(i = 0; i < aa_len; i++) {
			EG_OPA_t opa = 255 - (aa_opa[aa_len - 1 - i]);
			if(cir_x_right + i >= 0 && cir_x_right + i < Length) {
				pMaskArray[cir_x_right + i] = MaskMix(opa, pMaskArray[cir_x_right + i]);
			}
			if(cir_x_left - i >= 0 && cir_x_left - i < Length) {
				pMaskArray[cir_x_left - i] = MaskMix(opa, pMaskArray[cir_x_left - i]);
			}
		}
		EG_Coord_t clr_start = EG_CLAMP(0, cir_x_left + 1, Length);
		EG_Coord_t clr_len = EG_CLAMP(0, cir_x_right - clr_start, Length - clr_start);
		EG_ZeroMem(&pMaskArray[clr_start], clr_len);
	}

	return EG_DRAW_MASK_RES_CHANGED;
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskFade(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskFadeParam_t *pParam)
{
	if(AbsY < pParam->cfg.Area.GetY1()) return EG_DRAW_MASK_RES_FULL_COVER;
	if(AbsY > pParam->cfg.Area.GetY2()) return EG_DRAW_MASK_RES_FULL_COVER;
	if(AbsX + Length < pParam->cfg.Area.GetX1()) return EG_DRAW_MASK_RES_FULL_COVER;
	if(AbsX > pParam->cfg.Area.GetX2()) return EG_DRAW_MASK_RES_FULL_COVER;

	if(AbsX + Length > pParam->cfg.Area.GetX2()) Length -= AbsX + Length - pParam->cfg.Area.GetX2() - 1;

	if(AbsX < pParam->cfg.Area.GetX1()) {
		int32_t x_ofs = 0;
		x_ofs = pParam->cfg.Area.GetX1() - AbsX;
		Length -= x_ofs;
		pMaskArray += x_ofs;
	}
	int32_t i;
	if(AbsY <= pParam->cfg.TopY) {
		for(i = 0; i < Length; i++) {
			pMaskArray[i] = MaskMix(pMaskArray[i], pParam->cfg.TopOPA);
		}
		return EG_DRAW_MASK_RES_CHANGED;
	}
	else if(AbsY >= pParam->cfg.BottomY) {
		for(i = 0; i < Length; i++) {
			pMaskArray[i] = MaskMix(pMaskArray[i], pParam->cfg.BottomOPA);
		}
		return EG_DRAW_MASK_RES_CHANGED;
	}
	else {
		// Calculate the opa proportionally
		int16_t opa_diff = pParam->cfg.BottomOPA - pParam->cfg.TopOPA;
		int32_t y_diff = pParam->cfg.BottomY - pParam->cfg.TopY + 1;
		EG_OPA_t opa_act = (int32_t)((int32_t)(AbsY - pParam->cfg.TopY) * opa_diff) / y_diff;
		opa_act += pParam->cfg.TopOPA;
		for(i = 0; i < Length; i++) {
			pMaskArray[i] = MaskMix(pMaskArray[i], opa_act);
		}
		return EG_DRAW_MASK_RES_CHANGED;
	}
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskMap(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskMapParam_t *pParam)
{
	// Handle out of the mask cases
	if(AbsY < pParam->cfg.Area.GetY1()) return EG_DRAW_MASK_RES_FULL_COVER;
	if(AbsY > pParam->cfg.Area.GetY2()) return EG_DRAW_MASK_RES_FULL_COVER;
	if(AbsX + Length < pParam->cfg.Area.GetX1()) return EG_DRAW_MASK_RES_FULL_COVER;
	if(AbsX > pParam->cfg.Area.GetX2()) return EG_DRAW_MASK_RES_FULL_COVER;
	// Got to the current row in the map
	const EG_OPA_t *map_tmp = pParam->cfg.pMap;
	map_tmp += (AbsY - pParam->cfg.Area.GetY1()) * pParam->cfg.Area.GetWidth();
	if(AbsX + Length > pParam->cfg.Area.GetX2()) Length -= AbsX + Length - pParam->cfg.Area.GetX2() - 1;

	if(AbsX < pParam->cfg.Area.GetX1()) {
		int32_t x_ofs = 0;
		x_ofs = pParam->cfg.Area.GetX1() - AbsX;
		Length -= x_ofs;
		pMaskArray += x_ofs;
	}
	else {
		map_tmp += (AbsX - pParam->cfg.Area.GetX1());
	}

	int32_t i;
	for(i = 0; i < Length; i++) {
		pMaskArray[i] = MaskMix(pMaskArray[i], map_tmp[i]);
	}

	return EG_DRAW_MASK_RES_CHANGED;
}

/////////////////////////////////////////////////////////////////////////////////

static DrawMaskRes_t EG_ATTRIBUTE_FAST_MEM DrawMaskPolygon(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, MaskPolygonParam_t *pParam)
{
	uint16_t i;
	struct {
		EGPoint Point1;
		EGPoint Point2;
	} lines[2], tmp;
	uint16_t line_cnt = 0;
	EG_ZeroMem(&lines, sizeof(lines));
	int psign_prev = 0;
	for(i = 0; i < pParam->cfg.PointCount; i++) {
		EGPoint Point1 = pParam->cfg.pPoints[i];
		EGPoint Point2 = pParam->cfg.pPoints[i + 1 < pParam->cfg.PointCount ? i + 1 : 0];
		int pdiff = Point1.m_Y - Point2.m_Y, psign = pdiff / EG_ABS(pdiff);
		if(pdiff > 0) {
			if(AbsY > Point1.m_Y || AbsY < Point2.m_Y) continue;
			lines[line_cnt].Point1 = Point2;
			lines[line_cnt].Point2 = Point1;
		}
		else {
			if(AbsY < Point1.m_Y || AbsY > Point2.m_Y) continue;
			lines[line_cnt].Point1 = Point1;
			lines[line_cnt].Point2 = Point2;
		}
		if(psign_prev && psign_prev == psign) continue;
		psign_prev = psign;
		line_cnt++;
		if(line_cnt == 2) break;
	}
	if(line_cnt != 2) return EG_DRAW_MASK_RES_TRANSP;
	if(lines[0].Point1.m_X > lines[1].Point1.m_X || lines[0].Point2.m_X > lines[1].Point2.m_X) {
		tmp = lines[0];
		lines[0] = lines[1];
		lines[1] = tmp;
	}
	MaskLineParam_t LineParam;
	DrawMaskSetLinePoints(&LineParam, lines[0].Point1.m_X, lines[0].Point1.m_Y, lines[0].Point2.m_X, lines[0].Point2.m_Y, EG_DRAW_MASK_LINE_SIDE_RIGHT);
	if(LineParam.Steep == 0 && LineParam.Flat) {
		EG_Coord_t x1 = EG_MIN(lines[0].Point1.m_X, lines[0].Point2.m_X);
		EG_Coord_t x2 = EG_MAX(lines[0].Point1.m_X, lines[0].Point2.m_X);
		for(i = 0; i < Length; i++) {
			pMaskArray[i] = MaskMix(pMaskArray[i], (AbsX + i >= x1 && AbsX + i <= x2) * 0xFF);
		}
		DrawMaskFreeParam(&LineParam);
		return EG_DRAW_MASK_RES_CHANGED;
	}
	DrawMaskRes_t res1 = DrawMaskLine(pMaskArray, AbsX, AbsY, Length, &LineParam);
	DrawMaskFreeParam(&LineParam);
	if(res1 == EG_DRAW_MASK_RES_TRANSP) {
		return EG_DRAW_MASK_RES_TRANSP;
	}
	DrawMaskSetLinePoints(&LineParam, lines[1].Point1.m_X, lines[1].Point1.m_Y, lines[1].Point2.m_X, lines[1].Point2.m_Y, EG_DRAW_MASK_LINE_SIDE_LEFT);
	if(LineParam.Steep == 0 && LineParam.Flat) {
		EG_Coord_t x1 = EG_MIN(lines[1].Point1.m_X, lines[1].Point2.m_X);
		EG_Coord_t x2 = EG_MAX(lines[1].Point1.m_X, lines[1].Point2.m_X);
		for(i = 0; i < Length; i++) {
			pMaskArray[i] = MaskMix(pMaskArray[i], (AbsX + i >= x1 && AbsX + i <= x2) * 0xFF);
		}
		DrawMaskFreeParam(&LineParam);
		return EG_DRAW_MASK_RES_CHANGED;
	}
	DrawMaskRes_t res2 = DrawMaskLine(pMaskArray, AbsX, AbsY, Length, &LineParam);
	DrawMaskFreeParam(&LineParam);
	if(res2 == EG_DRAW_MASK_RES_TRANSP) {
		return EG_DRAW_MASK_RES_TRANSP;
	}
	if(res1 == EG_DRAW_MASK_RES_CHANGED || res2 == EG_DRAW_MASK_RES_CHANGED) return EG_DRAW_MASK_RES_CHANGED;
	return res1;
}

/////////////////////////////////////////////////////////////////////////////////

static void CircleInitialise(EGPoint *pCircle, EG_Coord_t *pTemp, EG_Coord_t Radius)
{
	pCircle->m_X = Radius;
	pCircle->m_Y = 0;
	*pTemp = 1 - Radius;
}

/////////////////////////////////////////////////////////////////////////////////

static bool CircleContinue(EGPoint *pCircle)
{
	return pCircle->m_Y <= pCircle->m_X ? true : false;
}

/////////////////////////////////////////////////////////////////////////////////

static void CircleNext(EGPoint *pCircle, EG_Coord_t *pTemp)
{
	if(*pTemp <= 0) {
		(*pTemp) += 2 * pCircle->m_Y + 3; // Change in decision criterion for y -> y+1
	}
	else {
		(*pTemp) += 2 * (pCircle->m_Y - pCircle->m_X) + 5; // Change for y -> y+1, x -> x-1
		pCircle->m_X--;
	}
	pCircle->m_Y++;
}

/////////////////////////////////////////////////////////////////////////////////

static void CircleCalc_aa4(MaskRadiusCircleDiscrpt_t *pCircle, EG_Coord_t Radius)
{
	if(Radius == 0) return;
	pCircle->Radius = Radius;

	// Allocate buffers
	if(pCircle->pBuffer) EG_FreeMem(pCircle->pBuffer);

	pCircle->pBuffer = (uint8_t *)EG_AllocMem(Radius * 6 + 6); // Use uint16_t for pStartOPAonY and pStartXonY
	EG_ASSERT_MALLOC(pCircle->pBuffer);
	pCircle->pCircleOPA = pCircle->pBuffer;
	pCircle->pStartOPAonY = (uint16_t *)(pCircle->pBuffer + 2 * Radius + 2);
	pCircle->pStartXonY = (uint16_t *)(pCircle->pBuffer + 4 * Radius + 4);

	// Special case, handle manually
	if(Radius == 1) {
		pCircle->pCircleOPA[0] = 180;
		pCircle->pStartOPAonY[0] = 0;
		pCircle->pStartOPAonY[1] = 1;
		pCircle->pStartXonY[0] = 0;
		return;
	}

	EG_Coord_t *cir_x = (EG_Coord_t *)EG_GetBufferMem((Radius + 1) * 2 * 2 * sizeof(EG_Coord_t));
	EG_Coord_t *cir_y = &cir_x[(Radius + 1) * 2];

	uint32_t y_8th_cnt = 0;
	EGPoint cp;
	EG_Coord_t tmp;
	CircleInitialise(&cp, &tmp, Radius * 4); // Upscale by 4
	int32_t i;
	uint32_t x_int[4];
	uint32_t x_fract[4];
	EG_Coord_t cir_size = 0;
	x_int[0] = cp.m_X >> 2;
	x_fract[0] = 0;
	// Calculate an 1/8 pCircle
	while(CircleContinue(&cp)) {
		// Calculate 4 point of the pCircle 
		for(i = 0; i < 4; i++) {
			CircleNext(&cp, &tmp);
			if(CircleContinue(&cp) == false) break;
			x_int[i] = cp.m_X >> 2;
			x_fract[i] = cp.m_X & 0x3;
		}
		if(i != 4) break;
		// All lines on the same x when downscaled
		if(x_int[0] == x_int[3]) {
			cir_x[cir_size] = x_int[0];
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = x_fract[0] + x_fract[1] + x_fract[2] + x_fract[3];
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
		}
		// Second line on new x when downscaled
		else if(x_int[0] != x_int[1]) {
			cir_x[cir_size] = x_int[0];
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = x_fract[0];
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
			cir_x[cir_size] = x_int[0] - 1;
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = 1 * 4 + x_fract[1] + x_fract[2] + x_fract[3];
			;
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
		}
		// Third line on new x when downscaled
		else if(x_int[0] != x_int[2]) {
			cir_x[cir_size] = x_int[0];
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = x_fract[0] + x_fract[1];
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
			cir_x[cir_size] = x_int[0] - 1;
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = 2 * 4 + x_fract[2] + x_fract[3];
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
		}
		// Forth line on new x when downscaled
		else {
			cir_x[cir_size] = x_int[0];
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = x_fract[0] + x_fract[1] + x_fract[2];
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
			cir_x[cir_size] = x_int[0] - 1;
			cir_y[cir_size] = y_8th_cnt;
			pCircle->pCircleOPA[cir_size] = 3 * 4 + x_fract[3];
			pCircle->pCircleOPA[cir_size] *= 16;
			cir_size++;
		}
		y_8th_cnt++;
	}
	// The point on the 1/8 pCircle is special, calculate it manually
	int32_t mid = Radius * 723;
	int32_t mid_int = mid >> 10;
	if(cir_x[cir_size - 1] != mid_int || cir_y[cir_size - 1] != mid_int) {
		int32_t tmp_val = mid - (mid_int << 10);
		if(tmp_val <= 512) {
			tmp_val = tmp_val * tmp_val * 2;
			tmp_val = tmp_val >> (10 + 6);
		}
		else {
			tmp_val = 1024 - tmp_val;
			tmp_val = tmp_val * tmp_val * 2;
			tmp_val = tmp_val >> (10 + 6);
			tmp_val = 15 - tmp_val;
		}
		cir_x[cir_size] = mid_int;
		cir_y[cir_size] = mid_int;
		pCircle->pCircleOPA[cir_size] = tmp_val;
		pCircle->pCircleOPA[cir_size] *= 16;
		cir_size++;
	}
	// Build the second octet by mirroring the first
	for(i = cir_size - 2; i >= 0; i--, cir_size++) {
		cir_x[cir_size] = cir_y[i];
		cir_y[cir_size] = cir_x[i];
		pCircle->pCircleOPA[cir_size] = pCircle->pCircleOPA[i];
	}
	EG_Coord_t y = 0;
	i = 0;
	pCircle->pStartOPAonY[0] = 0;
	while(i < cir_size) {
		pCircle->pStartOPAonY[y] = i;
		pCircle->pStartXonY[y] = cir_x[i];
		for(; cir_y[i] == y && i < (int32_t)cir_size; i++) {
			pCircle->pStartXonY[y] = EG_MIN(pCircle->pStartXonY[y], cir_x[i]);
		}
		y++;
	}
	EG_ReleaseBufferMem(cir_x);
}

/////////////////////////////////////////////////////////////////////////////////

static EG_OPA_t* GetNextLine(MaskRadiusCircleDiscrpt_t *pCircle, EG_Coord_t PosY, EG_Coord_t *pLength, EG_Coord_t *pStartX)
{
	*pLength = pCircle->pStartOPAonY[PosY + 1] - pCircle->pStartOPAonY[PosY];
	*pStartX = pCircle->pStartXonY[PosY];
	return &pCircle->pCircleOPA[pCircle->pStartOPAonY[PosY]];
}

/////////////////////////////////////////////////////////////////////////////////

static inline EG_OPA_t EG_ATTRIBUTE_FAST_MEM MaskMix(EG_OPA_t Active, EG_OPA_t New)
{
	if(New >= EG_OPA_MAX) return Active;
	if(New <= EG_OPA_MIN) return 0;
	return EG_UDIV255(Active * New);  // >> 8);
}

#endif
