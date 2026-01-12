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

#include "widgets/EG_Switch.h"

#if EG_USE_SWITCH != 0

#include "misc/EG_Assert.h"
#include "misc/EG_Math.h"
#include "misc/EG_Animate.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Display.h"
#include "widgets/EG_Image.h"

///////////////////////////////////////////////////////////////////////////////////////

#define SWITCH_CLASS &c_SwitchClass

// * Switch animation start value. (Not the real value of the switch just indicates process animation)
#define EG_SWITCH_ANIM_STATE_START 0

// * Switch animation end value.  (Not the real value of the switch just indicates process animation)
#define EG_SWITCH_ANIM_STATE_END 256

// * Mark no animation is in progress
#define EG_SWITCH_ANIM_STATE_INVALID -1

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_SwitchClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGSwitch::EventCB,
	.WidthDef = (4 * EG_DPI_DEF) / 10,
	.HeightDef = (4 * EG_DPI_DEF) / 17,
  .IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGSwitch::EGSwitch(void) : EGObject(),
	m_AnimateState(EG_SWITCH_ANIM_STATE_INVALID)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGSwitch::EGSwitch(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_LedClass*/) : EGObject(),
	m_AnimateState(EG_SWITCH_ANIM_STATE_INVALID)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGSwitch::~EGSwitch(void)
{
	EGAnimate::Delete(this, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSwitch::Configure(void)
{
  EGObject::Configure();
	m_AnimateState = EG_SWITCH_ANIM_STATE_INVALID;
	ClearFlag(EG_OBJ_FLAG_SCROLLABLE);
	AddFlag(EG_OBJ_FLAG_CHECKABLE);
	AddFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSwitch::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(SWITCH_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
  EG_EventCode_e Code = pEvent->GetCode();
	EGSwitch *pSwitch = (EGSwitch*)pEvent->GetTarget();
	if(Code == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		EG_Coord_t KnobLeft = pSwitch->GetStylePadLeft(EG_PART_KNOB);
		EG_Coord_t KnobRight = pSwitch->GetStylePadRight(EG_PART_KNOB);
		EG_Coord_t KnobTop = pSwitch->GetStylePadTop(EG_PART_KNOB);
		EG_Coord_t KnobBottom = pSwitch->GetStylePadBottom(EG_PART_KNOB);
		EG_Coord_t KnobSize = EG_MAX4(KnobLeft, KnobRight, KnobBottom, KnobTop);		// The smaller size is the knob diameter
		KnobSize += _EG_SWITCH_KNOB_EXT_AREA_CORRECTION;
		KnobSize += pSwitch->CalculateExtDrawSize(EG_PART_KNOB);
		EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();
		*pSize = EG_MAX(*pSize, KnobSize);
		*pSize = EG_MAX(*pSize, pSwitch->CalculateExtDrawSize(EG_PART_INDICATOR));
	}
	else if(Code == EG_EVENT_VALUE_CHANGED) {
		pSwitch->TriggerAnimate();
		pSwitch->Invalidate();
	}
	else if(Code == EG_EVENT_DRAW_MAIN) {
		pSwitch->DrawMain(pEvent);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSwitch::DrawMain(EGEvent *pEvent)
{
  EGDrawContext *pContext = pEvent->GetDrawContext();
	EG_Coord_t BackLeft = GetStylePadLeft(EG_PART_MAIN);	// Calculate the indicator area
	EG_Coord_t BackRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t BackTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t BackBottom = GetStylePadBottom(EG_PART_MAIN);
	EGRect IndicatorRect(m_Rect);	    // Draw the indicator. Respect the background'pSize padding
	IndicatorRect.Deflate(BackLeft, BackRight, BackTop, BackBottom);
	EGDrawRect DrawIndicator;
	InititialseDrawRect(EG_PART_INDICATOR, &DrawIndicator);
	DrawIndicator.Draw(pContext, &IndicatorRect);
	EG_Coord_t AnimateX = 0;	// Draw the knob
	EG_Coord_t KnobSize = GetHeight();
	EG_Coord_t AnimateLength = m_Rect.GetWidth() - KnobSize;
	if(m_AnimateState != EG_SWITCH_ANIM_STATE_INVALID) {
		AnimateX = (AnimateLength * m_AnimateState) / EG_SWITCH_ANIM_STATE_END;		//  Use the animation's coordinate 
	}
	else {
		bool Checked = m_State & EG_STATE_CHECKED;		//  Use EG_STATE_CHECKED to decide the coordinate 
		AnimateX = Checked ? AnimateLength : 0;
	}
	if(EG_BASE_DIR_RTL == GetStyleBaseDirection(EG_PART_MAIN)) {
		AnimateX = AnimateLength - AnimateX;
	}
	EGRect KnobRect;
	KnobRect.SetX1(m_Rect.GetX1() + AnimateX);
	KnobRect.SetX2(KnobRect.GetX1() + KnobSize);
	KnobRect.SetY1(m_Rect.GetY1());
	KnobRect.SetY2(m_Rect.GetY2());
	EG_Coord_t KnobLeft = GetStylePadLeft(EG_PART_KNOB);
	EG_Coord_t KnobRight = GetStylePadRight(EG_PART_KNOB);
	EG_Coord_t KnobTop = GetStylePadTop(EG_PART_KNOB);
	EG_Coord_t KnobBottom = GetStylePadBottom(EG_PART_KNOB);
	KnobRect.Inflate(KnobLeft, KnobRight, KnobTop, KnobBottom);	// Apply the paddings on the knob area
	EGDrawRect DrawKnob;
	InititialseDrawRect(EG_PART_KNOB, &DrawKnob);
	DrawKnob.Draw(pContext, &KnobRect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSwitch::AnimateExecCB(void *pObject, int32_t Value)
{
	EGSwitch *pSwitch = (EGSwitch*)pObject;
	pSwitch->m_AnimateState = Value;
	((EGSwitch*)pObject)->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSwitch::AnimateEndCB(EGAnimate *pAnimate)
{
	EGSwitch *pSwitch = (EGSwitch*)pAnimate->m_pItem;
	pSwitch->m_AnimateState = EG_SWITCH_ANIM_STATE_INVALID;
	pSwitch->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSwitch::TriggerAnimate(void)
{
	uint32_t AnimationTime = GetStyleAnimationTime(EG_PART_MAIN);
	if(AnimationTime > 0) {
		bool Checked = m_State & EG_STATE_CHECKED;
		int32_t AnimateStart;
		int32_t AnimateEnd;
		if(m_AnimateState == EG_SWITCH_ANIM_STATE_INVALID) {		// No animation in progress -> simply set the values
			AnimateStart = Checked ? EG_SWITCH_ANIM_STATE_START : EG_SWITCH_ANIM_STATE_END;
			AnimateEnd = Checked ? EG_SWITCH_ANIM_STATE_END : EG_SWITCH_ANIM_STATE_START;
		}
		else {		// Animation in progress. Start from the animation end value
			AnimateStart = m_AnimateState;
			AnimateEnd = Checked ? EG_SWITCH_ANIM_STATE_END : EG_SWITCH_ANIM_STATE_START;
		}
		// Calculate actual animation duration
		uint32_t AnimateDuration = (AnimationTime * LV_ABS(AnimateStart - AnimateEnd)) / EG_SWITCH_ANIM_STATE_END;
		EGAnimate::Delete(this, nullptr);		// Stop the previous animation if it exists
		EGAnimate Animate;
		Animate.SetItem(this);
		Animate.SetExcCB(AnimateExecCB);
		Animate.SetValues(AnimateStart, AnimateEnd);
		Animate.SetEndCB(AnimateEndCB);
		Animate.SetTime(AnimateDuration);
		EGAnimate::Create(&Animate);
	}
}

#endif
