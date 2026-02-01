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

#include "widgets/EG_Slider.h"
#if EG_USE_SLIDER != 0

#include "misc/EG_Assert.h"
#include "core/EG_Group.h"
#include "core/EG_InputDevice.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Math.h"
#include "core/EG_Display.h"
#include "widgets/EG_Image.h"

///////////////////////////////////////////////////////////////////////////////////////

#define SLIDER_CLASS &c_SliderClass

#define EG_SLIDER_KNOB_COORD(IsRTL, area) (IsRTL ? area.GetX1() : area.GetX2())

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_SliderClass = {
  .pBaseClassType = &c_BarClass,
	.pEventCB = EGSlider::EventCB,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGSlider::EGSlider(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_SliderClass*/) : EGBar()
{
	m_pValueToSet = nullptr;
	m_IsDragging = 0U;
  m_LeftKnobRect.Set(0,0,0,0);
  m_RightKnobRect.Set(0,0,0,0);
// 	ESP_LOGI("[Slider]", "New: %p", (void*)pParent);
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSlider::Configure(void)
{
  EGBar::Configure();
	ClearFlag(EG_OBJ_FLAG_SCROLL_CHAIN);
	ClearFlag(EG_OBJ_FLAG_SCROLLABLE);
	SetExtClickArea(EG_DPX(8));
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSlider::IsDragged(void)
{
	return m_IsDragging ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSlider::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(SLIDER_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGSlider *pSlider = (EGSlider*)pEvent->GetTarget();
  pSlider->Event(pEvent);   // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSlider::Event(EGEvent *pEvent)
{
 	EG_SliderMode_e SliderMode = GetMode();
  switch(pEvent->GetCode()){	// Advanced hit testing: react only on dragging the knob(pSize)
    case EG_EVENT_HIT_TEST: {
      EG_HitTestState_t *info = (EG_HitTestState_t*)pEvent->GetParam();
      EG_Coord_t ExtClickArea = m_pAttributes ? m_pAttributes->ExtendedClickPadding : 0;
      EGRect HitRect(m_RightKnobRect);		// Ordinary pSlider: was the knob area hit?
      HitRect.Inflate(ExtClickArea, ExtClickArea);
      info->Result = HitRect.IsPointIn(info->pPoint, 0);
      if((info->Result == false) && (SliderMode == EG_SLIDER_MODE_RANGE)) {// There'pSize still a chance that there is a hit if there is another knob
        m_LeftKnobRect.Copy(&HitRect);
        HitRect.Inflate(ExtClickArea, ExtClickArea);
        info->Result = HitRect.IsPointIn(info->pPoint, 0);
      }
      break;
    }
    case EG_EVENT_PRESSED: {
      Invalidate();
      EGPoint Point;
      m_IsDragging = true;
      if(SliderMode == EG_SLIDER_MODE_NORMAL || SliderMode == EG_SLIDER_MODE_SYMMETRICAL) {
        m_pValueToSet = &m_CurrentValue;
      }
      else if(SliderMode == EG_SLIDER_MODE_RANGE) {
        EGInputDevice::GetActive()->GetPoint(&Point);
        EG_BaseDirection_e BaseDirection = GetStyleBaseDirection(EG_PART_MAIN);
        EG_Coord_t LeftDistance, RightDistance;
        if(GetWidth() >= GetHeight()) {
          if((BaseDirection != EG_BASE_DIR_RTL && Point.m_X > m_RightKnobRect.GetX2()) ||
            (BaseDirection == EG_BASE_DIR_RTL && Point.m_X < m_RightKnobRect.GetX1())) {
            m_pValueToSet = &m_CurrentValue;
          }
          else if((BaseDirection != EG_BASE_DIR_RTL && Point.m_X < m_LeftKnobRect.GetX1()) ||
                  (BaseDirection == EG_BASE_DIR_RTL && Point.m_X > m_LeftKnobRect.GetX2())) {
            m_pValueToSet = &m_StartValue;
          }
          else {
            // Calculate the distance from each knob
            LeftDistance = EG_ABS((m_LeftKnobRect.GetX1() + (m_LeftKnobRect.GetX2() - m_LeftKnobRect.GetX1()) / 2) - Point.m_X);
            RightDistance = EG_ABS((m_RightKnobRect.GetX1() + (m_RightKnobRect.GetX2() - m_RightKnobRect.GetX1()) / 2) - Point.m_X);
            if(RightDistance < LeftDistance) {				// Use whichever one is closer
              m_pValueToSet = &m_CurrentValue;
              m_LeftKnobFocus = 0;
            }
            else {
              m_pValueToSet = &m_StartValue;
              m_LeftKnobFocus = 1;
            }
          }
        }
        else {
          if(Point.m_Y < m_RightKnobRect.GetY1()) m_pValueToSet = &m_CurrentValue;
          else if(Point.m_Y > m_LeftKnobRect.GetY2()) m_pValueToSet = &m_StartValue;
          else {
            // Calculate the distance from each knob
            LeftDistance = EG_ABS((m_LeftKnobRect.GetY1() + (m_LeftKnobRect.GetY2() - m_LeftKnobRect.GetY1()) / 2) - Point.m_Y);
            RightDistance = EG_ABS((m_RightKnobRect.GetY1() + (m_RightKnobRect.GetY2() - m_RightKnobRect.GetY1()) / 2) - Point.m_Y);

            // Use whichever one is closer
            if(RightDistance < LeftDistance) {
              m_pValueToSet = &m_CurrentValue;
              m_LeftKnobFocus = 0;
            }
            else {
              m_pValueToSet = &m_StartValue;
              m_LeftKnobFocus = 1;
            }
          }
        }
      }
      break;
    }
    case EG_EVENT_PRESSING:{
      if(m_pValueToSet != nullptr) {
        EGInputDevice *pInput = EGInputDevice::GetActive();
        if(pInput->GetType() != EG_INDEV_TYPE_POINTER) return;
        EGPoint Point;
        pInput->GetPoint(&Point);
        int32_t Value = 0;
        const int32_t Range = m_MaximumValue - m_MinimumValue;
        if(IsHorizontal()) {
          const EG_Coord_t LeftPadding = GetStylePadLeft(EG_PART_MAIN);
          const EG_Coord_t RightPadding = GetStylePadRight(EG_PART_MAIN);
          const EG_Coord_t w = GetWidth();
          const EG_Coord_t indic_w = w - LeftPadding - RightPadding;
          if(GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) {
            Value = (m_Rect.GetX2() - RightPadding) - Point.m_X;				
          }
          else {
            Value = Point.m_X - (m_Rect.GetX1() + LeftPadding); 			
          }
          Value = (Value * Range + indic_w / 2) / indic_w;
          Value += m_MinimumValue;
        }
        else {
          const EG_Coord_t TopPadding = GetStylePadTop(EG_PART_MAIN);
          const EG_Coord_t BottomPadding = GetStylePadBottom(EG_PART_MAIN);
          const EG_Coord_t Height = GetHeight() - BottomPadding - TopPadding;

          // Make the point relative to the indicator
          Value = Point.m_Y - (m_Rect.GetY2() + BottomPadding);
          Value = (-Value * Range + Height / 2) / Height;
          Value += m_MinimumValue;
        }
        int32_t MaxValue = m_MaximumValue;
        int32_t MinValue = m_MinimumValue;
        // Figure out the min. and max. for this SliderMode
        if(m_pValueToSet == &m_StartValue) MaxValue = m_CurrentValue;
        else MinValue = m_StartValue;
        Value = EG_CLAMP(MinValue, Value, MaxValue);
        if(*m_pValueToSet != Value) {
          if(m_pValueToSet == &m_StartValue) SetStartValue(Value, EG_ANIM_ON);
          else SetValue(Value, EG_ANIM_ON);
          if(EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr) != EG_RES_OK) return;
        }
      }
      break;
    }
    case EG_EVENT_RELEASED:
    case EG_EVENT_PRESS_LOST: {
      m_IsDragging = false;
      m_pValueToSet = nullptr;
      Invalidate();
      EGGroup *pGroup = (EGGroup*)GetGroup();	// Leave edit SliderMode if released. (No need to wait for LONG_PRESS)
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      if(InputType == EG_INDEV_TYPE_ENCODER) {
        if(IsEditing) {
          if(GetMode() == EG_SLIDER_MODE_RANGE) {
            if(m_LeftKnobFocus == 0)
              m_LeftKnobFocus = 1;
            else {
              m_LeftKnobFocus = 0;
              pGroup->SetEditing(false);
            }
          }
          else {
            pGroup->SetEditing(false);
          }
        }
      
      }
      break;
    }
    case EG_EVENT_FOCUSED: {
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      if(InputType == EG_INDEV_TYPE_ENCODER || InputType == EG_INDEV_TYPE_KEYPAD) {
        m_LeftKnobFocus = 0;
      }
      break;
    }
    case EG_EVENT_SIZE_CHANGED: {
      RefreshExtDrawSize();
      break;
    }
    case EG_EVENT_REFR_EXT_DRAW_SIZE: {
      EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_KNOB);
      EG_Coord_t PadRight = GetStylePadRight(EG_PART_KNOB);
      EG_Coord_t PadTop = GetStylePadTop(EG_PART_KNOB);
      EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_KNOB);
      EG_Coord_t Zoom = GetStyleTransformZoom(EG_PART_KNOB);	// The smaller size is the knob diameter
      EG_Coord_t TransformWidth = GetStyleTransformWidth(EG_PART_KNOB);
      EG_Coord_t TransformHeight = GetStyleTransformHeight(EG_PART_KNOB);
      EG_Coord_t KnobSize = EG_MIN(GetWidth() + 2 * TransformWidth, GetHeight() + 2 * TransformHeight) >> 1;
      KnobSize = (KnobSize * Zoom) >> 8;
      KnobSize += EG_MAX(EG_MAX(PadLeft, PadRight), EG_MAX(PadBottom, PadTop));
      KnobSize += 2;                                      // For rounding error
      KnobSize += CalculateExtDrawSize(EG_PART_KNOB);
      EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();		// Indic. size is handled by bar
      *pSize = EG_MAX(*pSize, KnobSize);
      break;
    }
    case EG_EVENT_KEY: {
      char Key = *((char *)pEvent->GetParam());
      if(Key == EG_KEY_RIGHT || Key == EG_KEY_UP) {
        if(!m_LeftKnobFocus) SetValue(GetValue() + 1, EG_ANIM_ON);
        else SetStartValue(GetStartValue() + 1, EG_ANIM_ON);
      }
      else if(Key == EG_KEY_LEFT || Key == EG_KEY_DOWN) {
        if(!m_LeftKnobFocus) SetValue(GetValue() - 1, EG_ANIM_ON);
        else SetStartValue(GetStartValue() - 1, EG_ANIM_ON);
      }
      else break;
      if(EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr) != EG_RES_OK) return;
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      DrawKnob(pEvent);
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSlider::DrawKnob(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
 	EG_SliderMode_e SliderMode = GetMode();
	const bool IsRTL = EG_BASE_DIR_RTL == GetStyleBaseDirection(EG_PART_MAIN);
	const bool Horizontal = IsHorizontal();
	EGRect pKnobRect;
	EG_Coord_t KnobSize;
	bool IsSymmetrical = false;
	if(SliderMode == EG_BAR_MODE_SYMMETRICAL && m_MinimumValue < 0 && m_MaximumValue > 0) IsSymmetrical = true;
	if(Horizontal) {
		KnobSize = GetHeight();
		if(IsSymmetrical && m_CurrentValue < 0) pKnobRect.SetX1(m_IndicatorRect.GetX1());
		else pKnobRect.SetX1(EG_SLIDER_KNOB_COORD(IsRTL, m_IndicatorRect));
	}
	else {
		KnobSize = GetWidth();
		if(IsSymmetrical && m_CurrentValue < 0)	pKnobRect.SetY1(m_IndicatorRect.GetY2());
		else pKnobRect.SetY1(m_IndicatorRect.GetY1());
	}
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_KNOB, &DrawRect);
	PositionKnob(&pKnobRect, KnobSize, Horizontal);	  //  Update knob area with knob style 
	pKnobRect.Copy(&m_RightKnobRect);	      //  Update right knob area with calculated knob area 
	EGDrawDiscriptor PartDrawDiscriptor;
	InitDrawDescriptor(&PartDrawDiscriptor, pContext);
	PartDrawDiscriptor.m_Part = EG_PART_KNOB;
	PartDrawDiscriptor.m_pClass = m_pClass;
	PartDrawDiscriptor.m_Type = EG_SLIDER_DRAW_PART_KNOB;
	PartDrawDiscriptor.m_Index = 0;
	PartDrawDiscriptor.m_pRect = &m_RightKnobRect;
	PartDrawDiscriptor.m_pDrawRect = &DrawRect;
	if(GetMode() != EG_SLIDER_MODE_RANGE) {
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
		DrawRect.Draw(pContext, &m_RightKnobRect);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
	}
	else {
		EGDrawRect DrawRectTemp;		// Save the draw part_draw_dsc. because it can be modified in the event
    DrawRectTemp = DrawRect;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
		DrawRect.Draw(pContext, &m_RightKnobRect);		//  Draw the right knob 
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
		if(IsHorizontal()) pKnobRect.SetX1(EG_SLIDER_KNOB_COORD(!IsRTL, m_IndicatorRect));	// Calculate the second knob area	
		else pKnobRect.SetY1(m_IndicatorRect.GetY2());
		PositionKnob(&pKnobRect, KnobSize, IsHorizontal());
		pKnobRect.Copy(&m_LeftKnobRect);
		DrawRect = DrawRectTemp;
		PartDrawDiscriptor.m_Type = EG_SLIDER_DRAW_PART_KNOB_LEFT;
		PartDrawDiscriptor.m_pRect = &m_LeftKnobRect;
		PartDrawDiscriptor.m_pDrawRect = &DrawRect;
		PartDrawDiscriptor.m_Index = 1;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
		DrawRect.Draw(pContext, &m_LeftKnobRect);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSlider::PositionKnob(EGRect *pKnobRect, const EG_Coord_t KnobSize, const bool Horizontal)
{
	if(Horizontal) {
		pKnobRect->DecX1(KnobSize >> 1);
		pKnobRect->SetX2(pKnobRect->GetX1() + KnobSize - 1);
		pKnobRect->SetY1(m_Rect.GetY1());
		pKnobRect->SetY2(m_Rect.GetY2());
	}
	else {
		pKnobRect->DecY1(KnobSize >> 1);
		pKnobRect->SetY2(pKnobRect->GetY1() + KnobSize - 1);
		pKnobRect->SetX1(m_Rect.GetX1());
		pKnobRect->SetX2(m_Rect.GetX2());
	}
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_KNOB);
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_KNOB);
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_KNOB);
	EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_KNOB);
	EG_Coord_t TransformWidth = GetStyleTransformWidth(EG_PART_KNOB);
	EG_Coord_t TransformHeight = GetStyleTransformHeight(EG_PART_KNOB);
	pKnobRect->Inflate((PadLeft + TransformWidth), (PadRight + TransformWidth), (PadTop + TransformHeight), (PadBottom + TransformHeight));	// Apply the paddings on the knob area
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGSlider::IsHorizontal(void)
{
	return GetWidth() >= GetHeight();
}

#endif
