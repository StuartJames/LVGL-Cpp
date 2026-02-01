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
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"
#include "misc/EG_Memory.h"
#include "draw/EG_DrawContext.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#define SPLIT_RADIUS_LIMIT 10    /*With radius greater than this the arc will drawn in quarters. A quarter is drawn only if there is arc in it*/
#define SPLIT_ANGLE_GAP_LIMIT 60 /*With small gaps in the arc don't bother with splitting because there is nothing to skip.*/

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawArc(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,	uint16_t StartAngle, uint16_t EndAngle)
{
#if EG_DRAW_COMPLEX
  const EGDrawContext *pContext = pDrawArc->m_pContext;
	if(pDrawArc->m_OPA <= EG_OPA_MIN) return;
	if(pDrawArc->m_Width == 0) return;
	if(StartAngle == EndAngle) return;
	EG_Coord_t Width = pDrawArc->m_Width;
	if(Width > Radius) Width = Radius;
	EGDrawRect DrawRect;
	DrawRect.m_BlendMode = pDrawArc->m_BlendMode;
	if(pDrawArc->m_pImageSource) {
		DrawRect.m_BackgroundOPA = EG_OPA_TRANSP;
		DrawRect.m_pBackImageSource = pDrawArc->m_pImageSource;
		DrawRect.m_BackImageOPA = pDrawArc->m_OPA;
	}
	else {
		DrawRect.m_BackgroundOPA = pDrawArc->m_OPA;
		DrawRect.m_BackgroundColor = pDrawArc->m_Color;
	}
	EGRect OutsideArea;
	OutsideArea.SetX1(pCenter->m_X - Radius);
	OutsideArea.SetY1(pCenter->m_Y - Radius);
	OutsideArea.SetX2(pCenter->m_X + Radius - 1); // -1 because the center already belongs to the left/bottom part
	OutsideArea.SetY2(pCenter->m_Y + Radius - 1);
	EGRect InsideArea;
	OutsideArea.Copy(&InsideArea);
	InsideArea.Deflate(pDrawArc->m_Width, pDrawArc->m_Width); // Defaltion
	int16_t MaskInsideID = EG_MASK_ID_INVALID;	    // Create inner the mask
	MaskRadiusParam_t MaskInsideParam;
	bool MaskParamIsValid = false;
	if((InsideArea.GetWidth() > 0) && (InsideArea.GetHeight() > 0)) {
		DrawMaskSetRadius(&MaskInsideParam, &InsideArea, EG_RADIUS_CIRCLE, true);
		MaskParamIsValid = true;
		MaskInsideID = DrawMaskAdd(&MaskInsideParam, nullptr);
	}
	MaskRadiusParam_t MaskOutsideParam;
	DrawMaskSetRadius(&MaskOutsideParam, &OutsideArea, EG_RADIUS_CIRCLE, false);
	int16_t MaskOutsideID = DrawMaskAdd(&MaskOutsideParam, nullptr);
	if(StartAngle + 360 == EndAngle || StartAngle == EndAngle + 360) {	// Draw a full ring
		DrawRect.m_Radius = EG_RADIUS_CIRCLE;
		DrawRect.Draw(pContext, &OutsideArea);
		DrawMaskRemove(MaskOutsideID);
		if(MaskInsideID != EG_MASK_ID_INVALID) DrawMaskRemove(MaskInsideID);
		DrawMaskFreeParam(&MaskOutsideParam);
		if(MaskParamIsValid) DrawMaskFreeParam(&MaskInsideParam);
		return;
	}
	StartAngle %= 360;
	EndAngle %= 360;
	MaskAngleParam_t MaskAngleParam;
	DrawMaskSetAngle(&MaskAngleParam, pCenter->m_X, pCenter->m_Y, StartAngle, EndAngle);
	int16_t MaskAngleID = DrawMaskAdd(&MaskAngleParam, nullptr);
	int32_t AngleGap;
	if(EndAngle > StartAngle) AngleGap = 360 - (EndAngle - StartAngle);
	else AngleGap = StartAngle - EndAngle;
	const EGRect *OriginalClipRect = pContext->m_pClipRect;
	if(AngleGap > SPLIT_ANGLE_GAP_LIMIT && Radius > SPLIT_RADIUS_LIMIT) {
		/*Handle each Quadrant individually and skip which is empty*/
		QuadrantDiscriptor_t QuadrantConfig;
		QuadrantConfig.pCenter = pCenter;
		QuadrantConfig.Radius = Radius;
		QuadrantConfig.StartAngle = StartAngle;
		QuadrantConfig.EndAngle = EndAngle;
		QuadrantConfig.StartQuarter = (StartAngle / 90) & 0x3;
		QuadrantConfig.EndQuarter = (EndAngle / 90) & 0x3;
		QuadrantConfig.Width = Width;
		QuadrantConfig.pDrawRec = &DrawRect;
		QuadrantConfig.pDrawRect = &OutsideArea;
	  QuadrantConfig.pDrawArc = pDrawArc;
    QuadrantConfig.pContext = pContext;
		DrawQuadrant0(&QuadrantConfig);
		DrawQuadrant1(&QuadrantConfig);
		DrawQuadrant2(&QuadrantConfig);
		DrawQuadrant3(&QuadrantConfig);
	}
	else DrawRect.Draw(pContext, &OutsideArea);
	DrawMaskFreeParam(&MaskAngleParam);
	DrawMaskFreeParam(&MaskOutsideParam);
	if(MaskParamIsValid) DrawMaskFreeParam(&MaskInsideParam);
	DrawMaskRemove(MaskAngleID);
	DrawMaskRemove(MaskOutsideID);
	if(MaskInsideID != EG_MASK_ID_INVALID) DrawMaskRemove(MaskInsideID);
	if(pDrawArc->m_Rounded) {
		MaskRadiusParam_t mask_end_param;
		EGRect RoundArea;
    GetRoundedArea(StartAngle, Radius, Width, &RoundArea);
		RoundArea.Move(pCenter->m_X, pCenter->m_Y);
		EGRect ClipRect;
		if(ClipRect.Intersect(OriginalClipRect, &RoundArea)) {
			DrawMaskSetRadius(&mask_end_param, &RoundArea, EG_RADIUS_CIRCLE, false);
			int16_t mask_end_id = DrawMaskAdd(&mask_end_param, nullptr);
			pContext->m_pClipRect = &ClipRect;
			DrawRect.Draw(pContext, &OutsideArea);
			DrawMaskRemove(mask_end_id);
			DrawMaskFreeParam(&mask_end_param);
		}
		GetRoundedArea(EndAngle, Radius, Width, &RoundArea);
		RoundArea.Move(pCenter->m_X, pCenter->m_Y);
		if(ClipRect.Intersect(OriginalClipRect, &RoundArea)) {
			DrawMaskSetRadius(&mask_end_param, &RoundArea, EG_RADIUS_CIRCLE, false);
			int16_t mask_end_id = DrawMaskAdd(&mask_end_param, nullptr);
			pContext->m_pClipRect = &ClipRect;
			DrawRect.Draw(pContext, &OutsideArea);
			DrawMaskRemove(mask_end_id);
			DrawMaskFreeParam(&mask_end_param);
		}
		pContext->m_pClipRect = OriginalClipRect;
	}
#else
	EG_LOG_WARN("Can't draw arc with EG_DRAW_COMPLEX == 0");
	EG_UNUSED(pCenter);
	EG_UNUSED(Radius);
	EG_UNUSED(StartAngle);
	EG_UNUSED(EndAngle);
	EG_UNUSED(draw_ctx);
	EG_UNUSED(dsc);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX
void EGSoftContext::DrawQuadrant0(QuadrantDiscriptor_t *pQuadrant)
{
	const EGRect *OriginalClipRect = pQuadrant->pDrawArc->m_pContext->m_pClipRect;
	EGRect QuadrantArea;
	if(pQuadrant->StartQuarter == 0 && pQuadrant->EndQuarter == 0 && pQuadrant->StartAngle < pQuadrant->EndAngle) {
		/*Small arc here*/
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * pQuadrant->Radius) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	else if(pQuadrant->StartQuarter == 0 || pQuadrant->EndQuarter == 0) {
		/*Start and/or end arcs here*/
		if(pQuadrant->StartQuarter == 0) {
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X);
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + pQuadrant->Radius);
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
			if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
  			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
		if(pQuadrant->EndQuarter == 0) {
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X + pQuadrant->Radius);
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y);
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * pQuadrant->Radius) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
			if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
  			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
	}
	else if((pQuadrant->StartQuarter == pQuadrant->EndQuarter && pQuadrant->StartQuarter != 0 && pQuadrant->EndAngle < pQuadrant->StartAngle) ||
					(pQuadrant->StartQuarter == 2 && pQuadrant->EndQuarter == 1) ||
					(pQuadrant->StartQuarter == 3 && pQuadrant->EndQuarter == 2) ||
					(pQuadrant->StartQuarter == 3 && pQuadrant->EndQuarter == 1)) {
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X);		/*Arc crosses here*/
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y);
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X + pQuadrant->Radius);
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + pQuadrant->Radius);
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	pQuadrant->pContext->m_pClipRect = OriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawQuadrant1(QuadrantDiscriptor_t *pQuadrant)
{
	const EGRect *OriginalClipRect = pQuadrant->pContext->m_pClipRect;
	EGRect QuadrantArea;
	if(pQuadrant->StartQuarter == 1 && pQuadrant->EndQuarter == 1 && pQuadrant->StartAngle < pQuadrant->EndAngle) {	/*Small arc here*/
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	else if(pQuadrant->StartQuarter == 1 || pQuadrant->EndQuarter == 1) {
		/*Start and/or end arcs here*/
		if(pQuadrant->StartQuarter == 1) {
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X - pQuadrant->Radius);
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y);
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		  if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			  pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
		if(pQuadrant->EndQuarter == 1) {
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X - 1);
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + pQuadrant->Radius);
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		  if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			  pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
	}
	else if((pQuadrant->StartQuarter == pQuadrant->EndQuarter && pQuadrant->StartQuarter != 1 && pQuadrant->EndAngle < pQuadrant->StartAngle) ||
					(pQuadrant->StartQuarter == 0 && pQuadrant->EndQuarter == 2) ||
					(pQuadrant->StartQuarter == 0 && pQuadrant->EndQuarter == 3) ||
					(pQuadrant->StartQuarter == 3 && pQuadrant->EndQuarter == 2)) {
		/*Arc crosses here*/
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X - pQuadrant->Radius);
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y);
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X - 1);
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + pQuadrant->Radius);
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	pQuadrant->pContext->m_pClipRect = OriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawQuadrant2(QuadrantDiscriptor_t *pQuadrant)
{
	const EGRect *OriginalClipRect = pQuadrant->pContext->m_pClipRect;
	EGRect QuadrantArea;

	if(pQuadrant->StartQuarter == 2 && pQuadrant->EndQuarter == 2 && pQuadrant->StartAngle < pQuadrant->EndAngle) {		/*Small arc here*/
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * pQuadrant->Radius) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	else if(pQuadrant->StartQuarter == 2 || pQuadrant->EndQuarter == 2) {		/*Start and/or end arcs here*/
		if(pQuadrant->StartQuarter == 2) {
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X - 1);
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y - pQuadrant->Radius);
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
	  	if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
   			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
		if(pQuadrant->EndQuarter == 2) {
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X - pQuadrant->Radius);
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y - 1);
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		  if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			  pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
	}
	else if((pQuadrant->StartQuarter == pQuadrant->EndQuarter && pQuadrant->StartQuarter != 2 && pQuadrant->EndAngle < pQuadrant->StartAngle) ||
					(pQuadrant->StartQuarter == 0 && pQuadrant->EndQuarter == 3) ||
					(pQuadrant->StartQuarter == 1 && pQuadrant->EndQuarter == 3) ||
					(pQuadrant->StartQuarter == 1 && pQuadrant->EndQuarter == 0)) {
		/*Arc crosses here*/
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X - pQuadrant->Radius);
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y - pQuadrant->Radius);
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X - 1);
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y - 1);
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	pQuadrant->pContext->m_pClipRect = OriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawQuadrant3(QuadrantDiscriptor_t *pQuadrant)
{
	const EGRect *OriginalClipRect = pQuadrant->pContext->m_pClipRect;
	EGRect QuadrantArea;

	if(pQuadrant->StartQuarter == 3 && pQuadrant->EndQuarter == 3 && pQuadrant->StartAngle < pQuadrant->EndAngle) {	// Small arc here
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	else if(pQuadrant->StartQuarter == 3 || pQuadrant->EndQuarter == 3) {
		if(pQuadrant->StartQuarter == 3) {		// Start and/or end arcs here
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X + pQuadrant->Radius);
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y - 1);
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->StartAngle + 90) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->StartAngle) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
			if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			  pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
		if(pQuadrant->EndQuarter == 3) {
			QuadrantArea.SetX1(pQuadrant->pCenter->m_X);
			QuadrantArea.SetY1(pQuadrant->pCenter->m_Y - pQuadrant->Radius);
			QuadrantArea.SetX2(pQuadrant->pCenter->m_X + ((EG_TrigoSin(pQuadrant->EndAngle + 90) * (pQuadrant->Radius)) >> EG_TRIGO_SHIFT));
			QuadrantArea.SetY2(pQuadrant->pCenter->m_Y + ((EG_TrigoSin(pQuadrant->EndAngle) * (pQuadrant->Radius - pQuadrant->Width)) >> EG_TRIGO_SHIFT));
			if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
				pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			  pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
			}
		}
	}
	else if((pQuadrant->StartQuarter == pQuadrant->EndQuarter && pQuadrant->StartQuarter != 3 && pQuadrant->EndAngle < pQuadrant->StartAngle) ||
					(pQuadrant->StartQuarter == 2 && pQuadrant->EndQuarter == 0) ||
					(pQuadrant->StartQuarter == 1 && pQuadrant->EndQuarter == 0) ||
					(pQuadrant->StartQuarter == 2 && pQuadrant->EndQuarter == 1)) {		// Arc crosses here
		QuadrantArea.SetX1(pQuadrant->pCenter->m_X);
		QuadrantArea.SetY1(pQuadrant->pCenter->m_Y - pQuadrant->Radius);
		QuadrantArea.SetX2(pQuadrant->pCenter->m_X + pQuadrant->Radius);
		QuadrantArea.SetY2(pQuadrant->pCenter->m_Y - 1);
		if(QuadrantArea.Intersect(&QuadrantArea, OriginalClipRect)){
			pQuadrant->pContext->m_pClipRect = &QuadrantArea;
 			pQuadrant->pDrawRec->Draw(pQuadrant->pContext, pQuadrant->pDrawRect);
		}
	}
	pQuadrant->pContext->m_pClipRect = OriginalClipRect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::GetRoundedArea(int16_t Angle, EG_Coord_t Radius, uint8_t Thickness, EGRect *pRect)
{
const uint8_t ps = 8;
const uint8_t pa = 127;
int32_t CircleX;
int32_t CircleY;

	int32_t ThicknessHalf = Thickness / 2;
	uint8_t thick_corr = (Thickness & 0x01) ? 0 : 1;
	CircleX = ((Radius - ThicknessHalf) * EG_TrigoSin(90 - Angle)) >> (EG_TRIGO_SHIFT - ps);
	CircleY = ((Radius - ThicknessHalf) * EG_TrigoSin(Angle)) >> (EG_TRIGO_SHIFT - ps);

	/*Actually the pCenter of the pixel need to be calculated so apply 1/2 px offset*/
	if(CircleX > 0) {
		CircleX = (CircleX - pa) >> ps;
		pRect->SetX1(CircleX - ThicknessHalf + thick_corr);
		pRect->SetX2(CircleX + ThicknessHalf);
	}
	else {
		CircleX = (CircleX + pa) >> ps;
		pRect->SetX1(CircleX - ThicknessHalf);
		pRect->SetX2(CircleX + ThicknessHalf - thick_corr);
	}

	if(CircleY > 0) {
		CircleY = (CircleY - pa) >> ps;
		pRect->SetY1(CircleY - ThicknessHalf + thick_corr);
		pRect->SetY2(CircleY + ThicknessHalf);
	}
	else {
		CircleY = (CircleY + pa) >> ps;
		pRect->SetY1(CircleY - ThicknessHalf);
		pRect->SetY2(CircleY + ThicknessHalf - thick_corr);
	}
}

#endif
