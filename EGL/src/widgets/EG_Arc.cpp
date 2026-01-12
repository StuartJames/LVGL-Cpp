/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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

#include "widgets/EG_Arc.h"

#if EG_USE_ARC != 0

#include "core/EG_Group.h"
#include "core/EG_InputDevice.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Math.h"
#include "draw/EG_DrawArc.h"

///////////////////////////////////////////////////////////////////////////////////////

#define ARC_CLASS &c_ArcClass

#define VALUE_UNSET INT16_MIN
#define CLICK_OUTSIDE_BG_ANGLES ((uint32_t)0x00U)
#define CLICK_INSIDE_BG_ANGLES ((uint32_t)0x01U)
#define CLICK_CLOSER_TO_MAX_END ((uint32_t)0x00U)
#define CLICK_CLOSER_TO_MIN_END ((uint32_t)0x01U)


///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_ArcClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGArc::EventCB,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGArc::EGArc(void) :
  EGObject(),
	m_Rotation(0),       
	m_IndicAngleStart(135), 
	m_IndicAngleEnd(270), 
	m_BackgroundAngleStart(135), 
	m_BackgroundAngleEnd(45), 
	m_Value(VALUE_UNSET), 
	m_MinimumValue(0), 
	m_MaximumValue(100), 
	m_Dragging(false), 
	m_Type(EG_ARC_MODE_NORMAL), 
	m_CloseToMinimum(1), 
	m_InOut(CLICK_OUTSIDE_BG_ANGLES),
	m_ChangeRate(720), 
	m_LastTick(EG_GetTick()), 
	m_LastAngle(m_IndicAngleEnd)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGArc::EGArc(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_LedClass*/) :
  EGObject(),
	m_Rotation(0),       
	m_IndicAngleStart(135), 
	m_IndicAngleEnd(270), 
	m_BackgroundAngleStart(135), 
	m_BackgroundAngleEnd(45), 
	m_Value(VALUE_UNSET), 
	m_MinimumValue(0), 
	m_MaximumValue(100), 
	m_Dragging(false), 
	m_Type(EG_ARC_MODE_NORMAL), 
	m_CloseToMinimum(1), 
	m_InOut(CLICK_OUTSIDE_BG_ANGLES),
	m_ChangeRate(720), 
	m_LastTick(EG_GetTick()), 
	m_LastAngle(m_IndicAngleEnd)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::Configure(void)
{
//  ESP_LOGI("[Arc   ]", "Configure.");
  EGObject::Configure();
	m_Rotation = 0;
	m_BackgroundAngleStart = 135;
	m_BackgroundAngleEnd = 45;
	m_IndicAngleStart = 135;
	m_IndicAngleEnd = 270;
	m_Type = EG_ARC_MODE_NORMAL;
	m_Value = VALUE_UNSET;
	m_CloseToMinimum = 1;
	m_MinimumValue = 0;
	m_MaximumValue = 100;
	m_Dragging = false;
	m_ChangeRate = 720;
	m_LastTick = EG_GetTick();
	m_LastAngle = m_IndicAngleEnd;
	m_InOut = CLICK_OUTSIDE_BG_ANGLES;
	AddFlag(EG_OBJ_FLAG_CLICKABLE);
	ClearFlag(EG_OBJ_FLAG_SCROLL_CHAIN | EG_OBJ_FLAG_SCROLLABLE);
	SetExtClickArea(EG_DPI_DEF / 10);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetStartAngle(uint16_t Start)
{
	Start = Start % 360;
	int16_t OldDelta = m_IndicAngleEnd - m_IndicAngleStart;
	int16_t NewDelta = m_IndicAngleEnd - Start;
	if(OldDelta < 0) OldDelta = 360 + OldDelta;
	if(NewDelta < 0) NewDelta = 360 + NewDelta;
	if(LV_ABS(NewDelta - OldDelta) > 180)	Invalidate();
	else if(NewDelta < OldDelta) InvalidateArcArea(m_IndicAngleStart, Start, EG_PART_INDICATOR);
	else if(OldDelta < NewDelta) InvalidateArcArea(Start, m_IndicAngleStart, EG_PART_INDICATOR);
	InvalidateKnobArea();
	m_IndicAngleStart = Start;
	InvalidateKnobArea();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetEndAngle(uint16_t End)
{
	End = End % 360;
	int16_t OldDelta = m_IndicAngleEnd - m_IndicAngleStart;
	int16_t NewDelta = End - m_IndicAngleStart;
	if(OldDelta < 0) OldDelta = 360 + OldDelta;
	if(NewDelta < 0) NewDelta = 360 + NewDelta;
	if(LV_ABS(NewDelta - OldDelta) > 180)	Invalidate();
	else if(NewDelta < OldDelta) InvalidateArcArea(End, m_IndicAngleEnd, EG_PART_INDICATOR);
	else if(OldDelta < NewDelta) InvalidateArcArea(m_IndicAngleEnd, End, EG_PART_INDICATOR);
	InvalidateKnobArea();
	m_IndicAngleEnd = End;
	InvalidateKnobArea();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetAngles(uint16_t Start, uint16_t End)
{
	SetEndAngle(End);
	SetStartAngle(Start);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetBackgroundStartAngle(uint16_t Start)
{
  Start = Start % 360;
	int16_t OldDelta = m_BackgroundAngleEnd - m_BackgroundAngleStart;
	int16_t NewDelta = m_BackgroundAngleEnd - Start;
	if(OldDelta < 0) OldDelta = 360 + OldDelta;
	if(NewDelta < 0) NewDelta = 360 + NewDelta;
	if(LV_ABS(NewDelta - OldDelta) > 180)	Invalidate();
	else if(NewDelta < OldDelta) InvalidateArcArea(m_BackgroundAngleStart, Start, EG_PART_MAIN);
	else if(OldDelta < NewDelta) InvalidateArcArea(Start, m_BackgroundAngleStart, EG_PART_MAIN);
	m_BackgroundAngleStart = Start;
	ValueUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetBackgroundEndAngle(uint16_t End)
{
	End = End % 360;
	int16_t OldDelta = m_BackgroundAngleEnd - m_BackgroundAngleStart;
	int16_t NewDelta = End - m_BackgroundAngleStart;
	if(OldDelta < 0) OldDelta = 360 + OldDelta;
	if(NewDelta < 0) NewDelta = 360 + NewDelta;
	if(LV_ABS(NewDelta - OldDelta) > 180) Invalidate();
	else if(NewDelta < OldDelta) InvalidateArcArea(End, m_BackgroundAngleEnd, EG_PART_MAIN);
	else if(OldDelta < NewDelta) InvalidateArcArea(m_BackgroundAngleEnd, End, EG_PART_MAIN);
	m_BackgroundAngleEnd = End;
	ValueUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetBackgroundAngles(uint16_t Start, uint16_t End)
{
	SetBackgroundEndAngle(End);
	SetBackgroundStartAngle(Start);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetRotation(uint16_t rotation)
{
	m_Rotation = rotation;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetMode(EG_ArcMode_e Type)
{
	int16_t Value = m_Value;
	m_Type = Type;
	m_Value = -1; // Force set_value handling
	int16_t BackgroundMidpoint, BackgroundEnd = m_BackgroundAngleEnd;
	if(m_BackgroundAngleEnd < m_BackgroundAngleStart) BackgroundEnd = m_BackgroundAngleEnd + 360;
	switch(m_Type) {
		case EG_ARC_MODE_SYMMETRICAL:
			BackgroundMidpoint = (m_BackgroundAngleStart + BackgroundEnd) / 2;
			SetStartAngle(BackgroundMidpoint);
			SetEndAngle(BackgroundMidpoint);
			break;
		case EG_ARC_MODE_REVERSE:
			SetEndAngle(m_BackgroundAngleEnd);
			break;
		default: // EG_ARC_TYPE_NORMAL
			SetStartAngle(m_BackgroundAngleStart);
	}

	SetValue(Value);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetValue(int16_t Value)
{
	if(m_Value == Value) return;
	int16_t NewValue;
	NewValue = Value > m_MaximumValue ? m_MaximumValue : Value;
	NewValue = NewValue < m_MinimumValue ? m_MinimumValue : NewValue;
	if(m_Value == NewValue) return;
	m_Value = NewValue;
	ValueUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetRange(int16_t Min, int16_t Max)
{
	if(m_MinimumValue == Min && m_MaximumValue == Max) return;
	m_MinimumValue = Min;
	m_MaximumValue = Max;
	if(m_Value < Min) m_Value = Min;
	if(m_Value > Max) m_Value = Max;
	ValueUpdate(); // value has changed relative to the new range
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::SetChangeRate(uint16_t Rate)
{
	m_ChangeRate = Rate;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGArc::GetAngleStart(void)
{
	return m_IndicAngleStart;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGArc::GetAngleEnd(void)
{
	return m_IndicAngleEnd;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGArc::GetBackgroundAngleStart(void)
{
	return m_BackgroundAngleStart;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGArc::GetBackgroundAngleEnd(void)
{
	return m_BackgroundAngleEnd;
}

///////////////////////////////////////////////////////////////////////////////////////

int16_t EGArc::GetValue(void)
{
	return m_Value;
}

///////////////////////////////////////////////////////////////////////////////////////

int16_t EGArc::GetMinValue(void)
{
	return m_MinimumValue;
}

///////////////////////////////////////////////////////////////////////////////////////

int16_t EGArc::GetMaxValue(void)
{
	return m_MaximumValue;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ArcMode_e EGArc::GetMode(void)
{
	return m_Type;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::AlignToAngle(EGObject *pObj, EG_Coord_t Offset)
{
EGPoint Center;
EG_Coord_t Radius;

	EG_ASSERT_NULL(pObj);
  UpdateLayout();
	GetCenter(&Center, &Radius);
	Radius -= GetStyleArcWidth(EG_PART_INDICATOR) / 2;
	Radius += Offset;
	uint16_t Angle = GetAngle();
	EG_Coord_t KnobX = (Radius * EG_TrigoSin(Angle + 90)) >> LV_TRIGO_SHIFT;
	EG_Coord_t KnobY = (Radius * EG_TrigoSin(Angle)) >> LV_TRIGO_SHIFT;
	pObj->AlignTo(this, EG_ALIGN_CENTER, KnobX, KnobY);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::RotateToAngle(EGObject *pObj, EG_Coord_t Offset)
{
EGPoint Center;
EG_Coord_t Radius;

	EG_ASSERT_NULL(pObj);
	UpdateLayout();
	GetCenter(&Center, &Radius);
	Radius -= GetStyleArcWidth(EG_PART_INDICATOR) / 2;
	Radius += Offset;
	pObj->AlignTo(this, EG_ALIGN_CENTER, 0, -Radius);
	UpdateLayout();
	uint16_t Angle = GetAngle();
	EG_Coord_t PivotX = pObj->m_Rect.GetX1() - Center.m_X;
	EG_Coord_t PivotY = pObj->m_Rect.GetY1() - Center.m_Y;
	pObj->SetStyleTransformPivotX(-PivotX, 0);
	pObj->SetStyleTransformPivotY(-PivotY, 0);
	pObj->SetStyleTransformAngle(Angle * 10 + 900, 0);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
 	EG_UNUSED(pClass);
  if(pEvent->Pump(ARC_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGArc *pArc = (EGArc*)pEvent->GetTarget();
  pArc->Event(pEvent); // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::Event(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_PRESSING:{
      EGInputDevice *pInputDevice = EGInputDevice::GetActive();
      if(pInputDevice == nullptr) return;
      EG_InDeviceType_e InputType = pInputDevice->GetType();		// Handle only pointers here
      if(InputType != EG_INDEV_TYPE_POINTER) return;
      EGPoint Point;
      pInputDevice->GetPoint(&Point);
      EGPoint Center;		// Make point relative to the pArc's Center
      EG_Coord_t Radius;
      GetCenter(&Center, &Radius);
      Point.m_X -= Center.m_X;
      Point.m_Y -= Center.m_Y;
      if(m_Dragging == false) {		// Enter dragging mode if pressed out of the knob
        EG_Coord_t IndicWidth = GetStyleArcWidth(EG_PART_INDICATOR);
        Radius -= IndicWidth;
        // Add some more sensitive area if there is no advanced git testing. (Advanced hit testing is more precise)
        if(HasFlagSet(EG_OBJ_FLAG_ADV_HITTEST)) Radius -= IndicWidth;
        else Radius -= EG_MAX(Radius / 4, IndicWidth);
        if(Radius < 1) Radius = 1;
        if(Point.m_X * Point.m_X + Point.m_Y * Point.m_Y > Radius * Radius) {
          m_Dragging = true;
          m_LastTick = EG_GetTick(); // Capture timestamp at dragging start
        }
      }
      if(m_Dragging == false) return;		// It must be in "dragging" mode to turn the pArc
      if(Point.m_X == 0 && Point.m_Y == 0) return;	// No angle can be determined if exactly the middle of the pArc is being pressed
      int16_t BackgroundEnd = m_BackgroundAngleEnd;		// Calculate the angle of the pressed point
      if(m_BackgroundAngleEnd < m_BackgroundAngleStart) {
        BackgroundEnd = m_BackgroundAngleEnd + 360;
      }
      int16_t Angle = EG_Atan2(Point.m_Y, Point.m_X);
      Angle -= m_Rotation;
      Angle -= m_BackgroundAngleStart; // Make the angle relative to the start angle
      Angle = Angle % 360;	//  If we click near the bg_angle_start the angle will be close to 360° instead of an small angle 
      const uint32_t Circumference = (uint32_t)((2U * Radius * 314U) / 100U); //  Equivalent to: 2r * 3.14, avoiding floats 
      const uint32_t ToleranceDeg = (360U * EG_DPX(50U)) / Circumference;
      const uint32_t PreviousCloseToMin = (uint32_t)m_CloseToMinimum;
      if(!IsAngleWithinBackgroundBounds((uint32_t)Angle, ToleranceDeg))	return;
      int16_t RangeDeg = BackgroundEnd - m_BackgroundAngleStart;
      int16_t LastRelativeAngle = m_LastAngle - m_BackgroundAngleStart;
      int16_t DeltaAngle = Angle - LastRelativeAngle;
      /*Do not allow big jumps (jumps bigger than 280°).
          *It's mainly to avoid jumping to the opposite end if the "dead" range between min. and max. is crossed.
          *Check which end was closer on the last valid press (m_CloseToMinimum) and prefer that end*/
      if(LV_ABS(DeltaAngle) > 280) {
        if(m_CloseToMinimum) Angle = 0;
        else Angle = RangeDeg;
      }
      //  Check if click was outside the background pArc start and end angles 
      else if(CLICK_OUTSIDE_BG_ANGLES == m_InOut) {
        if(m_CloseToMinimum) Angle = -RangeDeg;
        else Angle = RangeDeg;
      }	 //  Keep the angle value 
      /* Prevent big jumps when the click goes from start to end angle in the invisible
          * part of the background pArc without being released */
      if(((PreviousCloseToMin == CLICK_CLOSER_TO_MIN_END) && (m_CloseToMinimum == CLICK_CLOSER_TO_MAX_END)) &&
      ((CLICK_OUTSIDE_BG_ANGLES == m_InOut) && (LV_ABS(DeltaAngle) > 280))) Angle = 0;
      else if(((PreviousCloseToMin == CLICK_CLOSER_TO_MAX_END) && (m_CloseToMinimum == CLICK_CLOSER_TO_MIN_END)) &&
      (CLICK_OUTSIDE_BG_ANGLES == m_InOut)) Angle = RangeDeg;
      DeltaAngle = Angle - LastRelativeAngle;	// Calculate the slew rate limited angle based on change rate (degrees/sec)
      uint32_t DeltaTick = EG_TickElapse(m_LastTick);
      const uint16_t DeltaAngleMax = (m_ChangeRate * DeltaTick) / 1000;	//  DeltaAngleMax can never be signed. DeltaTick is always signed, same for ch_rate 
      if(DeltaAngle > DeltaAngleMax) DeltaAngle = DeltaAngleMax;
      else if(DeltaAngle < -DeltaAngleMax) DeltaAngle = -DeltaAngleMax;
      Angle = LastRelativeAngle + DeltaAngle; // Apply the limited angle change
      int32_t Rounding = ((BackgroundEnd - m_BackgroundAngleStart) * 8) / (m_MaximumValue - m_MinimumValue);	// Rounding for symmetry
      Rounding = (Rounding + 4) >> 4;
      Angle += Rounding;
      Angle += m_BackgroundAngleStart; // Make the angle absolute again
      int16_t OldValue = m_Value;		// Set the new value
      int16_t NewValue = EG_Map(Angle, m_BackgroundAngleStart, BackgroundEnd, m_MinimumValue, m_MaximumValue);
      if(NewValue != GetValue()) {
        m_LastTick = EG_GetTick();   // Cache timestamp for the next iteration
        SetValue(NewValue); // set_value caches the last_angle for the next iteration
        if(NewValue != OldValue) {
          if(EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr) != EG_RES_OK) return;
        }
      }
      // Don't let the elapsed time become too big while sitting on an end point
      if(NewValue == m_MinimumValue || NewValue == m_MaximumValue) {
        m_LastTick = EG_GetTick(); // Cache timestamp for the next iteration
      }
      break;
    }
    case EG_EVENT_RELEASED:
    case EG_EVENT_PRESS_LOST: {
      m_Dragging = false;
      EGGroup *pGroup = (EGGroup*)GetGroup();  // Leave edit mode if released. (No need to wait for LONG_PRESS)
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      if(InputType == EG_INDEV_TYPE_ENCODER) if(IsEditing) pGroup->SetEditing(false);
      break;
    }
    case EG_EVENT_KEY:{
      char Center = *((char *)pEvent->GetParam());
      int16_t OldValue = m_Value;
      if(Center == EG_KEY_RIGHT || Center == EG_KEY_UP) {
        SetValue(GetValue() + 1);
      }
      else if(Center == EG_KEY_LEFT || Center == EG_KEY_DOWN) {
        SetValue(GetValue() - 1);
      }
      if(OldValue != m_Value) {
        if(EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr) != EG_RES_OK) return;
      }
      break;
    }
    case EG_EVENT_HIT_TEST: {
      EG_HitTestState_t *pInfo = (EG_HitTestState_t*)pEvent->GetParam();
      EGPoint Point;
      EG_Coord_t Radius;
      GetCenter(&Point, &Radius);
      EG_Coord_t ExtendedClickArea = 0;
      if(m_pAttributes) ExtendedClickArea = m_pAttributes->ExtendedClickPadding;
      EG_Coord_t Width = GetStyleArcWidth(EG_PART_MAIN);
      Radius -= Width + ExtendedClickArea;
      EGRect Rect;
      // Invalid if clicked inside
      Rect.Set(Point.m_X - Radius, Point.m_Y - Radius, Point.m_X + Radius, Point.m_Y + Radius);
      if(Rect.IsPointIn(pInfo->pPoint, EG_RADIUS_CIRCLE)) {
        pInfo->Result = false;
        return;
      }
      Rect.Inflate(Width + ExtendedClickArea * 2, Width + ExtendedClickArea * 2);		// Valid if no clicked outside
      pInfo->Result = Rect.IsPointIn(pInfo->pPoint, EG_RADIUS_CIRCLE);
      break;
    }
    case EG_EVENT_REFR_EXT_DRAW_SIZE:{
      EG_Coord_t BackgroundLeft = GetStylePadLeft(EG_PART_MAIN);
      EG_Coord_t BackgroundRight = GetStylePadRight(EG_PART_MAIN);
      EG_Coord_t BackgroundTop = GetStylePadTop(EG_PART_MAIN);
      EG_Coord_t BackgroundBottom = GetStylePadBottom(EG_PART_MAIN);
      EG_Coord_t BackgroundPadding = EG_MAX4(BackgroundLeft, BackgroundRight, BackgroundTop, BackgroundBottom);
      EG_Coord_t KnobLeft = GetStylePadLeft(EG_PART_KNOB);
      EG_Coord_t KnobRight = GetStylePadRight(EG_PART_KNOB);
      EG_Coord_t KnobTop = GetStylePadTop(EG_PART_KNOB);
      EG_Coord_t KnobBottom = GetStylePadBottom(EG_PART_KNOB);
      EG_Coord_t KnobPadding = EG_MAX4(KnobLeft, KnobRight, KnobTop, KnobBottom) + 2;
      EG_Coord_t KnobExtraSize = KnobPadding - BackgroundPadding;
      KnobExtraSize += KnobGetExtraSize();
      EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();
      *pSize = EG_MAX(*pSize, KnobExtraSize);
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      Draw(pEvent);
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::Draw(EGEvent *pEvent)
{
	EGDrawContext *pDrawContext = pEvent->GetDrawContext();
	EGPoint Center;
	EG_Coord_t Radius;
	GetCenter(&Center, &Radius);
	EGDrawDiscriptor PartDrawDiscriptor;
	InitDrawDescriptor(&PartDrawDiscriptor, pDrawContext);
	if(Radius > 0) {
  	EGDrawArc DrawArc;	// Draw the background pArc
		InititialseDrawArc(EG_PART_MAIN, &DrawArc);
		PartDrawDiscriptor.m_Part = EG_PART_MAIN;
		PartDrawDiscriptor.m_pClass = ARC_CLASS;
		PartDrawDiscriptor.m_Type = EG_ARC_DRAW_PART_BACKGROUND;
		PartDrawDiscriptor.m_pPoint1 = &Center;
		PartDrawDiscriptor.m_Radius = Radius;
		PartDrawDiscriptor.m_pDrawArc = &DrawArc;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
		DrawArc.Draw(pDrawContext, &Center, PartDrawDiscriptor.m_Radius, m_BackgroundAngleStart + m_Rotation,	m_BackgroundAngleEnd + m_Rotation);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
	}
	// Make the indicator arc smaller or larger according to its greatest padding value
	EG_Coord_t IndicLeft = GetStylePadLeft(EG_PART_INDICATOR);
	EG_Coord_t IndicRight = GetStylePadRight(EG_PART_INDICATOR);
	EG_Coord_t IndicTop = GetStylePadTop(EG_PART_INDICATOR);
	EG_Coord_t IndicBottom = GetStylePadBottom(EG_PART_INDICATOR);
	EG_Coord_t IndicRadius = Radius - EG_MAX4(IndicLeft, IndicRight, IndicTop, IndicBottom);
	if(IndicRadius > 0) {
  	EGDrawArc DrawArc;	// Draw the indicator pArc
		InititialseDrawArc(EG_PART_INDICATOR, &DrawArc);
		PartDrawDiscriptor.m_Part = EG_PART_INDICATOR;
		PartDrawDiscriptor.m_pClass = ARC_CLASS;
		PartDrawDiscriptor.m_Type = EG_ARC_DRAW_PART_FOREGROUND;
		PartDrawDiscriptor.m_pPoint1 = &Center;
		PartDrawDiscriptor.m_Radius = IndicRadius;
		PartDrawDiscriptor.m_pDrawArc = &DrawArc;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
		if(DrawArc.m_Width > PartDrawDiscriptor.m_Radius) DrawArc.m_Width = PartDrawDiscriptor.m_Radius;
		DrawArc.Draw(pDrawContext, &Center, PartDrawDiscriptor.m_Radius, m_IndicAngleStart + m_Rotation,	m_IndicAngleEnd + m_Rotation);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
	}
	EGRect KnobRect;
	GetKnobArea(&Center, Radius, &KnobRect);
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_KNOB, &DrawRect);
	PartDrawDiscriptor.m_Part = EG_PART_KNOB;
	PartDrawDiscriptor.m_pClass = ARC_CLASS;
	PartDrawDiscriptor.m_Type = EG_ARC_DRAW_PART_KNOB;
	PartDrawDiscriptor.m_pRect = &KnobRect;
	PartDrawDiscriptor.m_pDrawRect = &DrawRect;
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
	DrawRect.Draw(pDrawContext, &KnobRect);
	EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::InvalidateArcArea(uint16_t StartAngle, uint16_t EndAngle, EGPart_t Part)
{
	if(IsVisible() == false) return;	// Skip this complicated invalidation if the pArc is not visible
	if(StartAngle == EndAngle) return;
	StartAngle += m_Rotation;
	EndAngle += m_Rotation;
	StartAngle = StartAngle % 360;
	EndAngle = EndAngle % 360;
	EG_Coord_t Radius;
	EGPoint Center;
	GetCenter(&Center, &Radius);
	EG_Coord_t Width = GetStyleArcWidth(Part);
	EG_Coord_t Rounded = GetStyleArcRounded(Part);
	EGRect Rect;
	EGDrawArc::GetArcRect(Center.m_X, Center.m_Y, Radius, StartAngle, EndAngle, Width, Rounded, &Rect);
	InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::InvalidateKnobArea(void)
{
EGRect Rect;
EGPoint Center;
EG_Coord_t Radius;

	GetCenter(&Center, &Radius);
	GetKnobArea(&Center, Radius, &Rect);
	EG_Coord_t KnobExtraSize = KnobGetExtraSize();
	if(KnobExtraSize > 0) {
		Rect.Inflate(KnobExtraSize, KnobExtraSize);
	}
	InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::GetCenter(EGPoint *pCenter, EG_Coord_t *pRadius)
{
	EG_Coord_t BackgroundLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t BackgroundRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t BackgroundTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t BackgroundBottom = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t Radius = (EG_MIN(GetWidth() - BackgroundLeft - BackgroundRight, GetHeight() - BackgroundTop - BackgroundBottom)) /	2;
	pCenter->m_X = m_Rect.GetX1() + Radius + BackgroundLeft;
	pCenter->m_Y = m_Rect.GetY1() + Radius + BackgroundTop;
	if(pRadius) *pRadius = Radius;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGArc::GetAngle(void)
{
	uint16_t Angle = m_Rotation;
	if(m_Type == EG_ARC_MODE_NORMAL) {
		Angle += m_IndicAngleEnd;
	}
	else if(m_Type == EG_ARC_MODE_REVERSE) {
		Angle += m_IndicAngleStart;
	}
	else if(m_Type == EG_ARC_MODE_SYMMETRICAL) {
		int16_t BackgroundEnd = m_BackgroundAngleEnd;
		if(m_BackgroundAngleEnd < m_BackgroundAngleStart) BackgroundEnd = m_BackgroundAngleEnd + 360;
		int16_t IndicEnd = m_IndicAngleEnd;
		if(m_IndicAngleEnd < m_IndicAngleStart) IndicEnd = m_IndicAngleEnd + 360;
		int32_t MidpointAngle = (int32_t)(m_BackgroundAngleStart + BackgroundEnd) / 2;
		if(m_IndicAngleStart < MidpointAngle) Angle += m_IndicAngleStart;
		else if(IndicEnd > MidpointAngle)	Angle += m_IndicAngleEnd;
		else Angle += MidpointAngle;
	}
	return Angle;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGArc::GetKnobArea(const EGPoint *Center, EG_Coord_t Radius, EGRect *KnobRect)
{
	EG_Coord_t IndicWidth = GetStyleArcWidth(EG_PART_INDICATOR);
	EG_Coord_t IndicHalfWidth = IndicWidth / 2;
	Radius -= IndicHalfWidth;
	EG_Coord_t Angle = GetAngle();
	EG_Coord_t KnobX = (Radius * EG_TrigoSin(Angle + 90)) >> LV_TRIGO_SHIFT;
	EG_Coord_t KnobY = (Radius * EG_TrigoSin(Angle)) >> LV_TRIGO_SHIFT;
	EG_Coord_t KnobLeft = GetStylePadLeft(EG_PART_KNOB);
	EG_Coord_t KnobRight = GetStylePadRight(EG_PART_KNOB);
	EG_Coord_t KnobTop = GetStylePadTop(EG_PART_KNOB);
	EG_Coord_t KnobBottom = GetStylePadBottom(EG_PART_KNOB);
	KnobRect->SetX1(Center->m_X + KnobX - KnobLeft - IndicHalfWidth);
	KnobRect->SetX2(Center->m_X + KnobX + KnobRight + IndicHalfWidth);
	KnobRect->SetY1(Center->m_Y + KnobY - KnobTop - IndicHalfWidth);
	KnobRect->SetY2(Center->m_Y + KnobY + KnobBottom + IndicHalfWidth);
}

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Used internally to update pArc angles after a value change
 * @param pArc pointer to an pArc object
 */
void EGArc::ValueUpdate(void)
{
	if(m_Value == VALUE_UNSET) return;	// If the value is still not set to any value do not update
	int16_t BackgroundMidpoint, range_midpoint, BackgroundEnd = m_BackgroundAngleEnd;
	if(m_BackgroundAngleEnd < m_BackgroundAngleStart) BackgroundEnd = m_BackgroundAngleEnd + 360;
	int16_t Angle;
	switch(m_Type) {
		case EG_ARC_MODE_SYMMETRICAL:
			BackgroundMidpoint = (m_BackgroundAngleStart + BackgroundEnd) / 2;
			range_midpoint = (int32_t)(m_MinimumValue + m_MaximumValue) / 2;
			if(m_Value < range_midpoint) {
				Angle = EG_Map(m_Value, m_MinimumValue, range_midpoint, m_BackgroundAngleStart, BackgroundMidpoint);
				SetStartAngle(Angle);
				SetEndAngle(BackgroundMidpoint);
			}
			else {
				Angle = EG_Map(m_Value, range_midpoint, m_MaximumValue, BackgroundMidpoint, BackgroundEnd);
				SetStartAngle(BackgroundMidpoint);
				SetEndAngle(Angle);
			}
			break;
		case EG_ARC_MODE_REVERSE:
			Angle = EG_Map(m_Value, m_MinimumValue, m_MaximumValue, BackgroundEnd, m_BackgroundAngleStart);
			SetAngles(Angle, m_BackgroundAngleEnd);
			break;
		case EG_ARC_MODE_NORMAL:
			Angle = EG_Map(m_Value, m_MinimumValue, m_MaximumValue, m_BackgroundAngleStart, BackgroundEnd);
			SetAngles(m_BackgroundAngleStart, Angle);

			break;
		default:
			EG_LOG_WARN("Invalid mode: %d", m_Type);
			return;
	}
	m_LastAngle = Angle; // Cache angle for slew rate limiting
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGArc::KnobGetExtraSize(void)
{
EG_Coord_t knob_shadow_size = 0, knob_outline_size = 0;

	knob_shadow_size += GetStyleShadowWidth(EG_PART_KNOB);
	knob_shadow_size += GetStyleShadowSpread(EG_PART_KNOB);
	knob_shadow_size += LV_ABS(GetStyleShadowOffsetX(EG_PART_KNOB));
	knob_shadow_size += LV_ABS(GetStyleShadowOffsetY(EG_PART_KNOB));
	knob_outline_size += GetStyleOutlineWidth(EG_PART_KNOB);
	knob_outline_size += GetStyleOutlinePadding(EG_PART_KNOB);
	return EG_MAX(knob_shadow_size, knob_outline_size);
}

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Check if angle is within pArc background bounds
 *
 * In order to avoid unexpected value update of the pArc value when the user clicks
 * outside of the pArc background we need to check if the angle (of the clicked point)
 * is within the bounds of the background.
 *
 * A tolerance (extra room) also should be taken into consideration.
 *
 * E.pGroup. Arc with start angle of 0° and end angle of 90°, the background is only visible in
 * that range, from 90° to 360° the background is invisible. Click in 150° should not update
 * the pArc value, click within the pArc angle range should.
 *
 * IMPORTANT NOTE: angle is always relative to bg_angle_start, pEvent.pGroup. if bg_angle_start is 30
 * and we click a bit to the left, angle is 10, not the expected 40.
 */
 bool EGArc::IsAngleWithinBackgroundBounds(const uint32_t Angle, const uint32_t ToleranceDeg)
{
uint32_t SmallerAngle = 0;
uint32_t BiggerAngle = 0;

	if(m_BackgroundAngleStart < m_BackgroundAngleEnd) {	//  Determine which background angle is smaller and bigger 
		BiggerAngle = m_BackgroundAngleEnd;
		SmallerAngle = m_BackgroundAngleStart;
	}
	else {
		BiggerAngle = (360 - m_BackgroundAngleStart) + m_BackgroundAngleEnd;
		SmallerAngle = 0;
	}
	if((SmallerAngle <= Angle) && (Angle <= BiggerAngle)) {	//  Angle is between both background angles 
		if(((BiggerAngle - SmallerAngle) / 2U) >= Angle) {
			m_CloseToMinimum = 1;
		}
		else {
			m_CloseToMinimum = 0;
		}
		m_InOut = CLICK_INSIDE_BG_ANGLES;
		return true;
	}
	//  Distance between background start and end angles is less than tolerance, consider the click inside the pArc 
	if(((SmallerAngle - ToleranceDeg) <= 0U) &&	(360U - (BiggerAngle + (SmallerAngle - ToleranceDeg)))) {
		m_CloseToMinimum = 1;
		m_InOut = CLICK_INSIDE_BG_ANGLES;
		return true;
	}
	/* Legends:
     * 0° = angle 0
     * 360° = angle 360
     * T: Tolerance
     * A: Angle
     * S: Arc background start angle
     * E: Arc background end angle
     */
	if((SmallerAngle >= ToleranceDeg)	&& ((Angle >= (360U - ToleranceDeg)) && (Angle <= 360U))) {	// (360° - T) --- A --- 360° 
    m_CloseToMinimum = 1;
		m_InOut = CLICK_OUTSIDE_BG_ANGLES;
		return true;
	}
	if((SmallerAngle < ToleranceDeg) && (((360U - (ToleranceDeg - SmallerAngle)) <= Angle)) && (Angle <= 360U)) {// (360° - (T - S)) --- A --- 360° 
		m_CloseToMinimum = 1;
		m_InOut = CLICK_OUTSIDE_BG_ANGLES;
		return true;
	}
	if((360U >= (BiggerAngle + ToleranceDeg)) && ((BiggerAngle <= (Angle + SmallerAngle)) && ((Angle + SmallerAngle) <= (BiggerAngle + ToleranceDeg)))) {// E --- A --- (E + T) 
		m_CloseToMinimum = 0;
		m_InOut = CLICK_OUTSIDE_BG_ANGLES;
		return true;
	}
	/* Background end angle + tolerance is bigger than 360° and bg_StartAngle + tolerance is not near 0° + ((bg_EndAngle + tolerance) - 360°)
     * Here we can assume background is not near 0° because of the first two initial checks */
	if((360U < (BiggerAngle + ToleranceDeg)) && (Angle <= 0U + ((BiggerAngle + ToleranceDeg) - 360U)) && (Angle > BiggerAngle)) {
		m_CloseToMinimum = 0;
		m_InOut = CLICK_OUTSIDE_BG_ANGLES;
		return true;
	}
	return false;
}

#endif
