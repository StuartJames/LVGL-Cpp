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

#include "extra/widgets/EG_Meter.h"
#if EG_USE_METER != 0

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

#define METER_CLASS &c_MeterClass

const EG_ClassType_t c_MeterClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGMeter::EventCB,
  .WidthDef = 0,
  .HeightDef = 0,
  .IsEditable = 0,  
  .GroupDef = 0, 
#if EG_USE_USER_DATA
  .pExtData = NULL,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGMeter::EGMeter(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_MeterClass*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGMeter::~EGMeter(void)
{
	m_ScaleList.RemoveAll();
	m_IndicatorList.RemoveAll();
}


///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::Configure(void)
{
  EGObject::Configure();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_MeterScale_t* EGMeter::AddScale(void)
{
  EG_MeterScale_t *pScale = (EG_MeterScale_t*)EG_AllocMem(sizeof(EG_MeterScale_t));
	EG_ASSERT_MALLOC(pScale);
	m_ScaleList.AddHead(pScale);
	EG_ZeroMem(pScale, sizeof(EG_MeterScale_t));
	pScale->AngleRange = 270;
	pScale->Rotation = 90 + (360 - pScale->AngleRange) / 2;
	pScale->Minimum = 0;
	pScale->Maximum = 100;
	pScale->TickCount = 6;
	pScale->TickLength = 8;
	pScale->TickWidth = 2;
	pScale->LabelGap = 2;
	return pScale;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::SetScaleTicks(EG_MeterScale_t *pScale, uint16_t Count, uint16_t Width, uint16_t Length, EG_Color_t Color)
{
	pScale->TickCount = Count;
	pScale->TickWidth = Width;
	pScale->TickLength = Length;
	pScale->TickColor = Color;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::SetScaleMajorTicks(EG_MeterScale_t *pScale, uint16_t Nth, uint16_t Width, uint16_t Length, EG_Color_t Color, int16_t LabelGap)
{
	pScale->TickMajorNth = Nth;
	pScale->TickMajorWidth = Width;
	pScale->TickMajorLength = Length;
	pScale->TickMajorColor = Color;
	pScale->LabelGap = LabelGap;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::SetScaleRange(EG_MeterScale_t *pScale, int32_t Minimum, int32_t Maximum, uint32_t AngleRange, uint32_t Rotation)
{
	pScale->Minimum = Minimum;
	pScale->Maximum = Maximum;
	pScale->AngleRange = AngleRange;
	pScale->Rotation = Rotation;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Indicator_t* EGMeter::AddNeedleLine(EG_MeterScale_t *pScale, uint16_t Width, EG_Color_t Color, int16_t RadiusMod)
{
  EG_Indicator_t *pIndicator = (EG_Indicator_t*)EG_AllocMem(sizeof(EG_Indicator_t));
	EG_ASSERT_MALLOC(pIndicator);
	m_IndicatorList.AddHead(pIndicator);
	EG_ZeroMem(pIndicator, sizeof(EG_Indicator_t));
	pIndicator->pScale = pScale;
	pIndicator->OPA = EG_OPA_COVER;
	pIndicator->Type = EG_METER_INDICATOR_TYPE_NEEDLE_LINE;
	pIndicator->TypeData.NeedleLine.Width = Width;
	pIndicator->TypeData.NeedleLine.Width = Width;
	pIndicator->TypeData.NeedleLine.RadiusMod = RadiusMod;
	Invalidate();
	return pIndicator;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Indicator_t* EGMeter::AddNeedleImage(EG_MeterScale_t *pScale, const void *pSource, EG_Coord_t PivotX, EG_Coord_t PivotY)
{
  EG_Indicator_t *pIndicator = (EG_Indicator_t*)EG_AllocMem(sizeof(EG_Indicator_t));
	EG_ASSERT_MALLOC(pIndicator);
	m_IndicatorList.AddHead(pIndicator);
	EG_ZeroMem(pIndicator, sizeof(EG_Indicator_t));
	pIndicator->pScale = pScale;
	pIndicator->OPA = EG_OPA_COVER;
	pIndicator->Type = EG_METER_INDICATOR_TYPE_NEEDLE_IMG;
	pIndicator->TypeData.NeedleImage.pSource = pSource;
	pIndicator->TypeData.NeedleImage.Pivot.m_X = PivotX;
	pIndicator->TypeData.NeedleImage.Pivot.m_Y = PivotY;
	Invalidate();
	return pIndicator;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Indicator_t* EGMeter::AddArc(EG_MeterScale_t *pScale, uint16_t Width, EG_Color_t Color, int16_t RadiusMod)
{
  EG_Indicator_t *pIndicator = (EG_Indicator_t*)EG_AllocMem(sizeof(EG_Indicator_t));
	EG_ASSERT_MALLOC(pIndicator);
	m_IndicatorList.AddHead(pIndicator);
	EG_ZeroMem(pIndicator, sizeof(EG_Indicator_t));
	pIndicator->pScale = pScale;
	pIndicator->OPA = EG_OPA_COVER;
	pIndicator->Type = EG_METER_INDICATOR_TYPE_ARC;
	pIndicator->TypeData.Arc.Width = Width;
	pIndicator->TypeData.Arc.Color = Color;
	pIndicator->TypeData.Arc.RadiusMod = RadiusMod;
	Invalidate();
	return pIndicator;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Indicator_t* EGMeter::AddScaleLines(EG_MeterScale_t *pScale, EG_Color_t StartColor, EG_Color_t EndColor, bool Local, int16_t WidthMod)
{
  EG_Indicator_t *pIndicator = (EG_Indicator_t*)EG_AllocMem(sizeof(EG_Indicator_t));
	EG_ASSERT_MALLOC(pIndicator);
	m_IndicatorList.AddHead(pIndicator);
	EG_ZeroMem(pIndicator, sizeof(EG_Indicator_t));
	pIndicator->pScale = pScale;
	pIndicator->OPA = EG_OPA_COVER;
	pIndicator->Type = EG_METER_INDICATOR_TYPE_SCALE_LINES;
	pIndicator->TypeData.ScaleLines.StartColor = StartColor;
	pIndicator->TypeData.ScaleLines.EndColor = EndColor;
	pIndicator->TypeData.ScaleLines.LocalGrad = Local;
	pIndicator->TypeData.ScaleLines.WidthMod = WidthMod;
	Invalidate();
	return pIndicator;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::SetIndicatorValue(EG_Indicator_t *pIndicator, int32_t Value)
{
	int32_t OldStart = pIndicator->StartValue;
	int32_t OldEnd = pIndicator->EndValue;
	pIndicator->StartValue = Value;
	pIndicator->EndValue = Value;
	if(pIndicator->Type == EG_METER_INDICATOR_TYPE_ARC) {
		InvArc(pIndicator, OldStart, Value);
		InvArc(pIndicator, OldEnd, Value);
	}
	else if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_IMG || pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_LINE) {
		InvLine(pIndicator, OldStart);
		InvLine(pIndicator, OldEnd);
		InvLine(pIndicator, Value);
	}
	else Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::SetIndicatorStartValue(EG_Indicator_t *pIndicator, int32_t Value)
{
	int32_t OldValue = pIndicator->StartValue;
	pIndicator->StartValue = Value;
	if(pIndicator->Type == EG_METER_INDICATOR_TYPE_ARC) {
		InvArc(pIndicator, OldValue, Value);
	}
	else if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_IMG || pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_LINE) {
		InvLine(pIndicator, OldValue);
		InvLine(pIndicator, Value);
	}
	else Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::SetIndicatorEndValue(EG_Indicator_t *pIndicator, int32_t Value)
{
	int32_t OldValue = pIndicator->EndValue;
	pIndicator->EndValue = Value;
	if(pIndicator->Type == EG_METER_INDICATOR_TYPE_ARC) {
		InvArc(pIndicator, OldValue, Value);
	}
	else if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_IMG || pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_LINE) {
		InvLine(pIndicator, OldValue);
		InvLine(pIndicator, Value);
	}
	else Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(METER_CLASS) != EG_RES_OK) return;// Call the ancestor's event handler
	((EGMeter*)pEvent->GetTarget())->Event(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_DRAW_MAIN) {
		EGDrawContext *pContext = pEvent->GetDrawContext();
		EGRect ScaleArea;
		GetContentArea(&ScaleArea);
		DrawArcs(pContext, &ScaleArea);
		DrawTicksLabels(pContext, &ScaleArea);
		DrawNeedles(pContext, &ScaleArea);
		EG_Coord_t EdgeRadius = ScaleArea.GetWidth() / 2;
		EGPoint ScaleCenter;
		ScaleCenter.m_X = ScaleArea.GetX1() + EdgeRadius;
		ScaleCenter.m_Y = ScaleArea.GetY1() + EdgeRadius;
		EGDrawRect DrawRect;
		InititialseDrawRect(EG_PART_INDICATOR, &DrawRect);
		EG_Coord_t Width = GetStyleWidth(EG_PART_INDICATOR) / 2;
		EG_Coord_t Height = GetStyleHeight(EG_PART_INDICATOR) / 2;
		EGRect Rect;
		Rect.SetX1(ScaleCenter.m_X - Width);
		Rect.SetY1(ScaleCenter.m_Y - Height);
		Rect.SetX2(ScaleCenter.m_X + Width);
		Rect.SetY2(ScaleCenter.m_Y + Height);
		DrawRect.Draw(pContext, &Rect);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::DrawArcs(EGDrawContext *pContext, const EGRect *pScaleArea)
{
	EGDrawArc DrawArc;
	DrawArc.m_Rounded = GetStyleArcRounded(EG_PART_ITEMS);
	EG_Coord_t OutsideRadius = pScaleArea->GetWidth() / 2;
	EGPoint ScaleCenter;
	ScaleCenter.m_X = pScaleArea->GetX1() + OutsideRadius;
	ScaleCenter.m_Y = pScaleArea->GetY1() + OutsideRadius;
	EG_OPA_t MainOPA = EGObject::GetOPARecursive(this, EG_PART_MAIN);
	EG_Indicator_t *pIndicator;
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_pDrawArc = &DrawArc;
	DrawDiscriptor.m_Part = EG_PART_INDICATOR;
	DrawDiscriptor.m_pClass = METER_CLASS;
	DrawDiscriptor.m_Type = EG_METER_DRAW_PART_ARC;
  POSITION Pos;
	for(pIndicator = (EG_Indicator_t*)m_IndicatorList.GetTail(Pos); pIndicator != NULL; pIndicator = (EG_Indicator_t*)m_IndicatorList.GetPrev(Pos)){
		if(pIndicator->Type != EG_METER_INDICATOR_TYPE_ARC) continue;
		DrawArc.m_Color = pIndicator->TypeData.Arc.Color;
		DrawArc.m_Width = pIndicator->TypeData.Arc.Width;
		DrawArc.m_OPA = pIndicator->OPA > EG_OPA_MAX ? MainOPA : (MainOPA * pIndicator->OPA) >> 8;
		EG_MeterScale_t *pScale = pIndicator->pScale;
		int32_t StartAngle = EG_Map(pIndicator->StartValue, pScale->Minimum, pScale->Maximum, pScale->Rotation,
																 pScale->Rotation + pScale->AngleRange);
		int32_t EndAngle = EG_Map(pIndicator->EndValue, pScale->Minimum, pScale->Maximum, pScale->Rotation,
															 pScale->Rotation + pScale->AngleRange);
		DrawArc.m_StartAngle = StartAngle;
		DrawArc.m_EndAngle = EndAngle;
		DrawDiscriptor.m_Radius = OutsideRadius + pIndicator->TypeData.Arc.RadiusMod;
		DrawDiscriptor.m_pSubPart = pIndicator;
		DrawDiscriptor.m_pPoint1 = &ScaleCenter;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		DrawArc.Draw(pContext, &ScaleCenter, DrawDiscriptor.m_Radius, StartAngle, EndAngle);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::DrawTicksLabels(EGDrawContext *pContext, const EGRect *pScaleArea)
{
	EGPoint CenterPoint;
	EG_Coord_t EdgeRadius = EG_MIN(pScaleArea->GetWidth() / 2, pScaleArea->GetHeight() / 2);
	CenterPoint.m_X = pScaleArea->GetX1() + EdgeRadius;
	CenterPoint.m_Y = pScaleArea->GetY1() + EdgeRadius;
	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_TICKS, &DrawLine);
	DrawLine.m_RawEnd = 1;
	EGDrawLabel label_dsc;
	InititialseDrawLabel(EG_PART_TICKS, &label_dsc);
	EG_MeterScale_t *pScale;
	MaskRadiusParam_t InnerMinorMask;
	MaskRadiusParam_t InnerMajorMask;
	MaskRadiusParam_t OuterMask;
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_pClass = m_pClass;
	DrawDiscriptor.m_Part = EG_PART_TICKS;
	DrawDiscriptor.m_Type = EG_METER_DRAW_PART_TICK;
	DrawDiscriptor.m_pDrawLine = &DrawLine;
  POSITION Pos;
	for(pScale = (EG_MeterScale_t*)m_ScaleList.GetTail(Pos); pScale != NULL; pScale = (EG_MeterScale_t*)m_ScaleList.GetPrev(Pos)){
		DrawDiscriptor.m_pSubPart = pScale;
		EG_Coord_t OutsideRadius = EdgeRadius;
		EG_Coord_t r_in_minor = OutsideRadius - pScale->TickLength;
		EG_Coord_t r_in_major = OutsideRadius - pScale->TickMajorLength;
		EGRect InnerMinorRect;
		InnerMinorRect.SetX1(CenterPoint.m_X - r_in_minor);
		InnerMinorRect.SetY1(CenterPoint.m_Y - r_in_minor);
		InnerMinorRect.SetX2(CenterPoint.m_X + r_in_minor);
		InnerMinorRect.SetY2(CenterPoint.m_Y + r_in_minor);
		DrawMaskSetRadius(&InnerMinorMask, &InnerMinorRect, EG_RADIUS_CIRCLE, true);
		EGRect InnerMajorRect;
		InnerMajorRect.SetX1(CenterPoint.m_X - r_in_major);
		InnerMajorRect.SetY1(CenterPoint.m_Y - r_in_major);
		InnerMajorRect.SetX2(CenterPoint.m_X + r_in_major - 1);
		InnerMajorRect.SetY2(CenterPoint.m_Y + r_in_major - 1);
		DrawMaskSetRadius(&InnerMajorMask, &InnerMajorRect, EG_RADIUS_CIRCLE, true);
		EGRect OuterRect;
		OuterRect.SetX1(CenterPoint.m_X - OutsideRadius);
		OuterRect.SetY1(CenterPoint.m_Y - OutsideRadius);
		OuterRect.SetX2(CenterPoint.m_X + OutsideRadius - 1);
		OuterRect.SetY2(CenterPoint.m_Y + OutsideRadius - 1);
		DrawMaskSetRadius(&OuterMask, &OuterRect, EG_RADIUS_CIRCLE, false);
		int16_t OuterMaskID = DrawMaskAdd(&OuterMask, NULL);
		int16_t InnerActMaskID = EG_MASK_ID_INVALID; // Will be added later
		uint32_t MinorCount = pScale->TickMajorNth ? pScale->TickMajorNth - 1 : 0xFFFF;
		uint16_t i;
		for(i = 0; i < pScale->TickCount; i++) {
			MinorCount++;
			bool DoMajor = false;
			if(MinorCount == pScale->TickMajorNth) {
				MinorCount = 0;
				DoMajor = true;
			}
			int32_t LineValue = EG_Map(i, 0, pScale->TickCount - 1, pScale->Minimum, pScale->Maximum);
			DrawDiscriptor.m_Value = LineValue;
			EG_Color_t line_color = DoMajor ? pScale->TickMajorColor : pScale->TickColor;
			EG_Color_t line_color_ori = line_color;
			EG_Coord_t line_width_ori = DoMajor ? pScale->TickMajorWidth : pScale->TickWidth;
			EG_Coord_t line_width = line_width_ori;
			EG_Indicator_t *pIndicator;
      POSITION Pos2;
		  for(pIndicator = (EG_Indicator_t*)m_IndicatorList.GetTail(Pos2); pIndicator != NULL; pIndicator = (EG_Indicator_t*)m_IndicatorList.GetPrev(Pos2)){
				if(pIndicator->Type != EG_METER_INDICATOR_TYPE_SCALE_LINES) continue;
				if(LineValue >= pIndicator->StartValue && LineValue <= pIndicator->EndValue) {
					line_width += pIndicator->TypeData.ScaleLines.WidthMod;
					if(pIndicator->TypeData.ScaleLines.StartColor.full == pIndicator->TypeData.ScaleLines.EndColor.full) {
						line_color = pIndicator->TypeData.ScaleLines.StartColor;
					}
					else {
						EG_OPA_t ratio;
						if(pIndicator->TypeData.ScaleLines.LocalGrad) {
							ratio = EG_Map(LineValue, pIndicator->StartValue, pIndicator->EndValue, EG_OPA_TRANSP, EG_OPA_COVER);
						}
						else {
							ratio = EG_Map(LineValue, pScale->Minimum, pScale->Maximum, EG_OPA_TRANSP, EG_OPA_COVER);
						}
						line_color = EG_ColorMix(pIndicator->TypeData.ScaleLines.EndColor, pIndicator->TypeData.ScaleLines.StartColor, ratio);
					}
				}
			}
			int32_t UpscaleAngle = ((i * pScale->AngleRange) * 10) / (pScale->TickCount - 1) + +pScale->Rotation * 10;
			DrawLine.m_Color = line_color;
			DrawLine.m_Width = line_width;
			// Draw a little bit longer lines to be sure the mask will clip them correctly and to get a better precision
			EGPoint OuterPoint;
			OuterPoint.m_X = CenterPoint.m_X + OutsideRadius + EG_MAX(EG_DPI_DEF, OutsideRadius);
			OuterPoint.m_Y = CenterPoint.m_Y;
			OuterPoint.PointTransform(UpscaleAngle, 256, 256, &CenterPoint);
			DrawDiscriptor.m_pPoint1 = &CenterPoint;
			DrawDiscriptor.m_pPoint2 = &OuterPoint;
			DrawDiscriptor.m_Index = i;
			DrawDiscriptor.m_pDrawLabel = &label_dsc;
			if(DoMajor) {			// Draw the text
				DrawMaskRemove(OuterMaskID);
				uint32_t r_text = r_in_major - pScale->LabelGap;
				EGPoint Point;
				Point.m_X = CenterPoint.m_X + r_text;
				Point.m_Y = CenterPoint.m_Y;
				Point.PointTransform(UpscaleAngle, 256, 256, &CenterPoint);
				EGDrawLabel DrawLabel;
				EG_CopyMem(&DrawLabel, &label_dsc, sizeof(DrawLabel));

				DrawDiscriptor.m_pDrawLabel = &DrawLabel;
				char Buffer[16];
				eg_snprintf(Buffer, sizeof(Buffer), "%" EG_PRId32, LineValue);
				DrawDiscriptor.m_pText = Buffer;
  			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
				EGPoint label_size;
				EG_GetTextSize(&label_size, DrawDiscriptor.m_pText, DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace,	EG_COORD_MAX, EG_TEXT_FLAG_NONE);
				EGRect LabelRect;
				LabelRect.SetX1(Point.m_X - label_size.m_X / 2);
				LabelRect.SetY1(Point.m_Y - label_size.m_Y / 2);
				LabelRect.SetX2(LabelRect.GetX1() + label_size.m_X);
				LabelRect.SetY2(LabelRect.GetY1() + label_size.m_Y);
				DrawDiscriptor.m_pDrawLabel->Draw(pContext, &LabelRect, DrawDiscriptor.m_pText, NULL);
				OuterMaskID = DrawMaskAdd(&OuterMask, NULL);
			}
			else {
				DrawDiscriptor.m_pDrawLabel = NULL;
				DrawDiscriptor.m_pText = NULL;
				EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			}
			InnerActMaskID = DrawMaskAdd(DoMajor ? &InnerMajorMask : &InnerMinorMask, NULL);
			DrawLine.Draw(pContext, &OuterPoint, &CenterPoint);
			DrawMaskRemove(InnerActMaskID);
			EGEvent::EventSend(this, EG_EVENT_DRAW_MAIN_END, &DrawDiscriptor);
			DrawLine.m_Color = line_color_ori;
			DrawLine.m_Width = line_width_ori;
		}
		DrawMaskFreeParam(&InnerMinorMask);
		DrawMaskFreeParam(&InnerMajorMask);
		DrawMaskFreeParam(&OuterMask);
		DrawMaskRemove(OuterMaskID);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::DrawNeedles(EGDrawContext *pContext, const EGRect *pScaleArea)
{
	EG_Coord_t EdgeRadius = pScaleArea->GetWidth() / 2;
	EGPoint ScaleCenter;
	ScaleCenter.m_X = pScaleArea->GetX1() + EdgeRadius;
	ScaleCenter.m_Y = pScaleArea->GetY1() + EdgeRadius;
	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_ITEMS, &DrawLine);
	EGDrawImage DrawImage;
	InititialseDrawImage(EG_PART_ITEMS, &DrawImage);
	EG_OPA_t MainOPA = EGObject::GetOPARecursive(this, EG_PART_MAIN);
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pContext);
	DrawDiscriptor.m_pClass = m_pClass;
	DrawDiscriptor.m_pPoint1 = &ScaleCenter;
	DrawDiscriptor.m_Part = EG_PART_INDICATOR;
	EG_Indicator_t *pIndicator;
  POSITION Pos;
	for(pIndicator = (EG_Indicator_t*)m_IndicatorList.GetTail(Pos); pIndicator != NULL; pIndicator = (EG_Indicator_t*)m_IndicatorList.GetPrev(Pos)){
		EG_MeterScale_t *pScale = pIndicator->pScale;
		DrawDiscriptor.m_pSubPart = pIndicator;
		if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_LINE) {
			int32_t Angle = EG_Map(pIndicator->EndValue, pScale->Minimum, pScale->Maximum, pScale->Rotation, pScale->Rotation + pScale->AngleRange);
			EG_Coord_t OutsideRadius = EdgeRadius + pScale->RadiusMod + pIndicator->TypeData.NeedleLine.RadiusMod;
			EGPoint EndPoint;
			EndPoint.m_Y = (EG_TrigoSin(Angle) * (OutsideRadius)) / EG_TRIGO_SIN_MAX + ScaleCenter.m_Y;
			EndPoint.m_X = (lv_trigo_cos(Angle) * (OutsideRadius)) / EG_TRIGO_SIN_MAX + ScaleCenter.m_X;
			DrawLine.m_Color = pIndicator->TypeData.NeedleLine.Color;
			DrawLine.m_Width = pIndicator->TypeData.NeedleLine.Width;
			DrawLine.m_OPA = pIndicator->OPA > EG_OPA_MAX ? MainOPA : (MainOPA * pIndicator->OPA) >> 8;
			DrawDiscriptor.m_Type = EG_METER_DRAW_PART_NEEDLE_LINE;
			DrawDiscriptor.m_pDrawLine = &DrawLine;
			DrawDiscriptor.m_pPoint2 = &EndPoint;
			DrawDiscriptor.m_pPoint1 = &ScaleCenter;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			DrawLine.Draw(pContext, DrawDiscriptor.m_pPoint1, &EndPoint);
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		}
		else if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_IMG) {
			if(pIndicator->TypeData.NeedleImage.pSource == NULL) continue;
			int32_t Angle = EG_Map(pIndicator->EndValue, pScale->Minimum, pScale->Maximum, pScale->Rotation, pScale->Rotation + pScale->AngleRange);
			EG_ImageHeader_t info;
			EGImageDecoder::GetInfo(pIndicator->TypeData.NeedleImage.pSource, &info);
			EGRect Rect;
			Rect.SetX1(ScaleCenter.m_X - pIndicator->TypeData.NeedleImage.Pivot.m_X);
			Rect.SetY1(ScaleCenter.m_Y - pIndicator->TypeData.NeedleImage.Pivot.m_Y);
			Rect.SetX2(Rect.GetX1() + info.Width - 1);
			Rect.SetY2(Rect.GetY1() + info.Height - 1);
			DrawImage.m_OPA = pIndicator->OPA > EG_OPA_MAX ? MainOPA : (MainOPA * pIndicator->OPA) >> 8;
			DrawImage.m_Pivot.m_X = pIndicator->TypeData.NeedleImage.Pivot.m_X;
			DrawImage.m_Pivot.m_Y = pIndicator->TypeData.NeedleImage.Pivot.m_Y;
			Angle = Angle * 10;
			if(Angle > 3600) Angle -= 3600;
			DrawImage.m_Angle = Angle;
			DrawDiscriptor.m_Type = EG_METER_DRAW_PART_NEEDLE_IMG;
			DrawDiscriptor.m_pDrawImage = &DrawImage;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
			DrawImage.Draw(pContext, &Rect, pIndicator->TypeData.NeedleImage.pSource);
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMeter::InvArc(EG_Indicator_t *pIndicator, int32_t OldValue, int32_t NewValue)
{
	bool Rounded = GetStyleArcRounded(EG_PART_ITEMS);
	EGRect ScaleArea;
	GetContentArea(&ScaleArea);
	EG_Coord_t OutsideRadius = ScaleArea.GetWidth() / 2;
	EGPoint ScaleCenter;
	ScaleCenter.m_X = ScaleArea.GetX1() + OutsideRadius;
	ScaleCenter.m_Y = ScaleArea.GetY1() + OutsideRadius;
	OutsideRadius += pIndicator->TypeData.Arc.RadiusMod;
	EG_MeterScale_t *pScale = pIndicator->pScale;
	int32_t start_angle = EG_Map(OldValue, pScale->Minimum, pScale->Maximum, pScale->Rotation, pScale->AngleRange + pScale->Rotation);
	int32_t end_angle = EG_Map(NewValue, pScale->Minimum, pScale->Maximum, pScale->Rotation, pScale->AngleRange + pScale->Rotation);
	EGRect Rect;
	EGDrawArc::GetArcRect(ScaleCenter.m_X, ScaleCenter.m_Y, OutsideRadius, EG_MIN(start_angle, end_angle), EG_MAX(start_angle, end_angle),
											 pIndicator->TypeData.Arc.Width, Rounded, &Rect);
	InvalidateArea(&Rect);
}

void EGMeter::InvLine(EG_Indicator_t *pIndicator, int32_t Value)
{
	EGRect ScaleArea;
	GetContentArea(&ScaleArea);
	EG_Coord_t OutsideRadius = ScaleArea.GetWidth() / 2;
	EGPoint ScaleCenter;
	ScaleCenter.m_X = ScaleArea.GetX1() + OutsideRadius;
	ScaleCenter.m_Y = ScaleArea.GetY1() + OutsideRadius;
	EG_MeterScale_t *pScale = pIndicator->pScale;
	if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_LINE) {
		int32_t Angle = EG_Map(Value, pScale->Minimum, pScale->Maximum, pScale->Rotation, pScale->Rotation + pScale->AngleRange);
		OutsideRadius += pScale->RadiusMod + pIndicator->TypeData.NeedleLine.RadiusMod;
		EGPoint EndPoint;
		EndPoint.m_Y = (EG_TrigoSin(Angle) * (OutsideRadius)) / EG_TRIGO_SIN_MAX + ScaleCenter.m_Y;
		EndPoint.m_X = (lv_trigo_cos(Angle) * (OutsideRadius)) / EG_TRIGO_SIN_MAX + ScaleCenter.m_X;
		EGRect Rect;
		Rect.SetX1(EG_MIN(ScaleCenter.m_X, EndPoint.m_X) - pIndicator->TypeData.NeedleLine.Width - 2);
		Rect.SetY1(EG_MIN(ScaleCenter.m_Y, EndPoint.m_Y) - pIndicator->TypeData.NeedleLine.Width - 2);
		Rect.SetX2(EG_MAX(ScaleCenter.m_X, EndPoint.m_X) + pIndicator->TypeData.NeedleLine.Width + 2);
		Rect.SetY2(EG_MAX(ScaleCenter.m_Y, EndPoint.m_Y) + pIndicator->TypeData.NeedleLine.Width + 2);
		InvalidateArea(&Rect);
	}
	else if(pIndicator->Type == EG_METER_INDICATOR_TYPE_NEEDLE_IMG) {
		int32_t Angle = EG_Map(Value, pScale->Minimum, pScale->Maximum, pScale->Rotation, pScale->Rotation + pScale->AngleRange);
		EG_ImageHeader_t info;
		EGImageDecoder::GetInfo(pIndicator->TypeData.NeedleImage.pSource, &info);
		Angle = Angle * 10;
		if(Angle > 3600) Angle -= 3600;
		ScaleCenter.m_X -= pIndicator->TypeData.NeedleImage.Pivot.m_X;
		ScaleCenter.m_Y -= pIndicator->TypeData.NeedleImage.Pivot.m_Y;
		EGRect Rect2;
		EGImageBuffer::GetTransformedRect(&Rect2, info.Width, info.Height, Angle, EG_SCALE_NONE, EG_SCALE_NONE, &pIndicator->TypeData.NeedleImage.Pivot);
		Rect2.IncX1(ScaleCenter.m_X - 2);
		Rect2.IncY1(ScaleCenter.m_Y - 2);
		Rect2.IncX2(ScaleCenter.m_X + 2);
		Rect2.IncY2(ScaleCenter.m_Y + 2);
		InvalidateArea(&Rect2);
	}
}

#endif
