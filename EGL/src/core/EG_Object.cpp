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

#include "core/EG_Object.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Refresh.h"
#include "core/EG_Group.h"
#include "core/EG_Display.h"
#include "core/EG_Theme.h"
#include "misc/EG_Assert.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Animate.h"
#include "misc/EG_Timer.h"
#include "misc/EG_Async.h"
#include "misc/EG_FileSystem.h"
#include "misc/EG_Misc.h"
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"
#include "hal/EG_HAL.h"
#include "extra/EG_Extra.h"
#include <stdint.h>
#include <string.h>

EGObject *g_pTabObj;
EGObject *g_pItemObj;

////////////////////////////////////////////////////////////////////////////////

#define OBJ_CLASS &c_ObjectClass

#define EG_OBJ_DEF_WIDTH (EG_DPX(100))
#define EG_OBJ_DEF_HEIGHT (EG_DPX(50))
#define STYLE_TRANSITION_MAX 32

////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_ObjectClass = {
  .pBaseClassType = nullptr,
  .pEventCB = EGObject::EventCB,
  .WidthDef = EG_DPI_DEF,
  .HeightDef = EG_DPI_DEF,
  .IsEditable = EG_OBJ_CLASS_EDITABLE_FALSE,
  .GroupDef = EG_OBJ_CLASS_GROUP_DEF_FALSE,
#if EG_USE_USER_DATA
  .pExtData = nullptr
#endif
};

EGList    EGObject::m_TransitionsList;
EGList    EGObject::m_LayoutList;

////////////////////////////////////////////////////////////////////////////////

EGObject::EGObject(void) 
{
  m_pParent = nullptr;
  m_pClass = nullptr;
  m_pStyles = nullptr;
  m_pAttributes = nullptr;
  m_pUserData = nullptr;
  m_Flags = EG_OBJ_FLAG_HIDDEN;
  m_State = EG_STATE_DEFAULT;
  m_StyleCount = 0;
  m_LayoutInvalid = 0;
  m_ReadScrollAfterLayout = 0;
  m_ScreenLayoutInvalid = 0;
  m_SkipTransition = 0;
  m_HeightLayout = 0;
  m_WidthLayout = 0;
  m_IsBeingDeleted = 0;
}

////////////////////////////////////////////////////////////////////////////////

EGObject::EGObject(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_ObjectClass*/) 
{
  m_pParent = nullptr;
  m_pClass = nullptr;
  m_pStyles = nullptr;
  m_pAttributes = nullptr;
  m_pUserData = nullptr;
  m_Flags = EG_OBJ_FLAG_HIDDEN;
  m_State = EG_STATE_DEFAULT;
  m_StyleCount = 0;
  m_LayoutInvalid = 0;
  m_ReadScrollAfterLayout = 0;
  m_ScreenLayoutInvalid = 0;
  m_SkipTransition = 0;
  m_HeightLayout = 0;
  m_WidthLayout = 0;
  m_IsBeingDeleted = 0;
 	EG_LOG_INFO("[Object]", "New : Parent %p", (void*)pParent);
  Attach(this, pParent, pClassCnfg);  //  attach class and set parent
	Initialise();
}

////////////////////////////////////////////////////////////////////////////////

EGObject::~EGObject(void)
{
	EGEvent::MarkDeleted(this);
	EnableStyleRefresh(false);                        // No need to refresh the style because the object will be deleted
	RemoveAllStyles();	                              // Remove all style
	EnableStyleRefresh(true);
	EGAnimate::Delete(this, nullptr);	                // Remove the animations from this object
	EGGroup *Group = (EGGroup*)GetGroup();	          // Delete from the group
	if(Group) Group->RemoveObject(this);
	if(m_pAttributes) {
		if(m_pAttributes->ppChildren) {
			EG_FreeMem(m_pAttributes->ppChildren);
			m_pAttributes->ppChildren = nullptr;
		}
		if(m_pAttributes->pEventDescriptor) {
			EG_FreeMem(m_pAttributes->pEventDescriptor);
			m_pAttributes->pEventDescriptor = nullptr;
		}
    delete m_pAttributes->pScroll;
		EG_FreeMem(m_pAttributes);
		m_pAttributes = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::Configure(void) 
{
	if(m_pParent) {
    EG_LOG_INFO("[Object]", "Configure.");
		EG_Coord_t ScrollLeft = m_pParent->GetScrollLeft();
		EG_Coord_t ScrollTop = m_pParent->GetScrollTop();
		m_Rect.SetY1(m_pParent->m_Rect.GetY1() + m_pParent->GetStylePadTop(EG_PART_MAIN) - ScrollTop);
		m_Rect.SetY2(m_Rect.GetY1() - 1);
		m_Rect.SetX1(m_pParent->m_Rect.GetX1() + m_pParent->GetStylePadLeft(EG_PART_MAIN) - ScrollLeft);
		m_Rect.SetX2(m_Rect.GetX1() - 1);
	}
	m_Flags = EG_OBJ_FLAG_CLICKABLE;
	m_Flags |= EG_OBJ_FLAG_SNAPPABLE;
	if(m_pParent) m_Flags |= EG_OBJ_FLAG_PRESS_LOCK;
	if(m_pParent) m_Flags |= EG_OBJ_FLAG_SCROLL_CHAIN;
	m_Flags |= EG_OBJ_FLAG_CLICK_FOCUSABLE;
	m_Flags |= EG_OBJ_FLAG_SCROLLABLE;
	m_Flags |= EG_OBJ_FLAG_SCROLL_ELASTIC;
	m_Flags |= EG_OBJ_FLAG_SCROLL_MOMENTUM;
	m_Flags |= EG_OBJ_FLAG_SCROLL_WITH_ARROW;
	if(m_pParent) m_Flags |= EG_OBJ_FLAG_GESTURE_BUBBLE;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::AddFlag(EG_ObjectFlag_t Flag)
{
bool IsOnLayout = IsLayoutPositioned();

	/* We must invalidate the area occupied by the object before we hide it as calls to invalidate hidden objects are ignored */
	if(Flag & EG_OBJ_FLAG_HIDDEN) Invalidate();
	m_Flags |= Flag;
	if(Flag & EG_OBJ_FLAG_HIDDEN) {
		if(HasState(EG_STATE_FOCUSED)) {
			EGGroup *pGroup = (EGGroup*)GetGroup();
			if(pGroup != nullptr) {
				pGroup->FocusNext();
				EGObject *pNextObj = pGroup->GetFocused();
				if(pNextObj != nullptr) {
					pNextObj->Invalidate();
				}
			}
		}
	}
	if((IsOnLayout != IsLayoutPositioned()) || (Flag & (EG_OBJ_FLAG_LAYOUT_1 | EG_OBJ_FLAG_LAYOUT_2))) {
		GetParent()->MarkLayoutDirty();
		MarkLayoutDirty();
	}
	if(Flag & EG_OBJ_FLAG_SCROLLABLE) {
		EGRect HorizontalArea, VerticalArea;
		GetScrollbarArea(&HorizontalArea, &VerticalArea);
		InvalidateArea(&HorizontalArea);
		InvalidateArea(&VerticalArea);
	}
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::ClearFlag(EG_ObjectFlag_t Flag)
{
bool WasOnLayout = IsLayoutPositioned();

	if(Flag & EG_OBJ_FLAG_SCROLLABLE) {
		EGRect HorizontalArea, VerticalArea;
		GetScrollbarArea(&HorizontalArea, &VerticalArea);
		InvalidateArea(&HorizontalArea);
		InvalidateArea(&VerticalArea);
	}
	m_Flags &= (~Flag);
	if(Flag & EG_OBJ_FLAG_HIDDEN) {
		Invalidate();
		if(IsLayoutPositioned()) {
			GetParent()->MarkLayoutDirty();
			MarkLayoutDirty();
		}
	}
	if((WasOnLayout != IsLayoutPositioned()) || (Flag & (EG_OBJ_FLAG_LAYOUT_1 | EG_OBJ_FLAG_LAYOUT_2))) {
		GetParent()->MarkLayoutDirty();
	}
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::HasFlagSet(EG_ObjectFlag_t Flag) const
{
	return (m_Flags & Flag) == Flag ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::HasAnyFlagSet(EG_ObjectFlag_t Flags)
{
	return (m_Flags & Flags) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::AddState(EGState_t State)
{
	EGState_t NewState = m_State | State;
	if(m_State != NewState) UpdateState(NewState);
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::ClearState(EGState_t State)
{
	EGState_t NewState = m_State & (~State);
	if(m_State != NewState) UpdateState(NewState);
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::SetState(EGState_t State, bool Enable /*= true*/)
{
    if(Enable) AddState(State);
    else ClearState(State);
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::UpdateState(EGState_t NewState)
{
uint32_t i, j, t, TransitionIndex = 0;

	if(m_State == NewState) return;
	EGState_t PreviousState = m_State;
	m_State = NewState;
	EG_StyleStateCmp_e CompareResult = CompareState(PreviousState, NewState);
	if(CompareResult == _EG_STYLE_STATE_CMP_SAME) return;	      // If there is no difference in styles there is nothing else to do
	EG_TransitionDiscriptor_t *pTransitions = (EG_TransitionDiscriptor_t*)EG_AllocMem(sizeof(EG_TransitionDiscriptor_t) * STYLE_TRANSITION_MAX);
	EG_ZeroMem(pTransitions, sizeof(EG_TransitionDiscriptor_t) * STYLE_TRANSITION_MAX);
	for(i = 0; (i < m_StyleCount) && (TransitionIndex < STYLE_TRANSITION_MAX); i++) { // iterate the styles
		EG_ObjStyle_t *pObjStyle = &m_pStyles[i];
		EGState_t StyleState = GetSelectorState(pObjStyle->SelectFlags);
		EGPart_t StylePart = GetSelectorPart(pObjStyle->SelectFlags);
		if(StyleState & (~NewState)) continue; // Skip unrelated styles
		if(pObjStyle->IsTransition) continue;
		EG_StyleValue_t Value;
		if(pObjStyle->pStyle->GetProperty(EG_STYLE_TRANSITION, &Value) != EG_STYLE_RES_FOUND) continue;
		const EG_StyleTransitionDiscriptor_t *pStyleTransitionParams = (EG_StyleTransitionDiscriptor_t*)Value.pPtr;
		for(j = 0; (pStyleTransitionParams->pProperties[j] != 0) && (TransitionIndex < STYLE_TRANSITION_MAX); j++) {	 // iterate the transition properties
			for(t = 0; t < TransitionIndex; t++) {// Add the props to the set if not added yet or exists but with smaller weight
				EG_StyleFlags_t SelectFlags = pTransitions[t].SelectFlags;
				EGState_t TransState = GetSelectorState(SelectFlags);
				EGPart_t TransPart = GetSelectorPart(SelectFlags);
				if((pTransitions[t].Property == pStyleTransitionParams->pProperties[j]) && (TransPart == StylePart) && (TransState >= StyleState)) break;
			}
			if(t == TransitionIndex) {			// Add it if it doesn't exist
				pTransitions[t].Time = pStyleTransitionParams->Time;
				pTransitions[t].Delay = pStyleTransitionParams->Delay;
				pTransitions[t].PathCB = pStyleTransitionParams->PathCB;
				pTransitions[t].Property = pStyleTransitionParams->pProperties[j];
				pTransitions[t].m_pParam = pStyleTransitionParams->m_pParam;
				pTransitions[t].SelectFlags = pObjStyle->SelectFlags;
				TransitionIndex++;
			}
		}
	}
	for(i = 0; i < TransitionIndex; i++) {
		EGPart_t ActivePart = GetSelectorPart(pTransitions[i].SelectFlags);
		CreateTransition(ActivePart, PreviousState, NewState, &pTransitions[i]);
	}
	EG_FreeMem(pTransitions);
	if(CompareResult == _EG_STYLE_STATE_CMP_DIFF_REDRAW) Invalidate();
	else if(CompareResult == _EG_STYLE_STATE_CMP_DIFF_LAYOUT) RefreshStyle(EG_PART_ANY, EG_STYLE_PROP_ANY);
	else if(CompareResult == _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD){
		Invalidate();
		RefreshExtDrawSize();
	}
}

////////////////////////////////////////////////////////////////////////////////

EGState_t EGObject::GetState(void)
{
	return m_State;
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::HasState(EGState_t State)
{
	return m_State & State ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

EG_StyleStateCmp_e EGObject::CompareState(EGState_t PrevState, EGState_t NewState)
{
EG_StyleStateCmp_e Result = _EG_STYLE_STATE_CMP_SAME;
bool LayoutDifferent = false;	

	for(uint32_t i = 0; i < m_StyleCount; i++){	// Are there any new m_pStyles for the new State?
		if(m_pStyles[i].IsTransition) continue;   // skip
		EGState_t CurrentState = GetSelectorState(m_pStyles[i].SelectFlags);
		bool Valid1 = (CurrentState & (~PrevState)) ? false : true;		// The style is valid for a State but not the other
		bool Valid2 = (CurrentState & (~NewState)) ? false : true;
		if(Valid1 != Valid2) {
			EGStyle *pStyle = m_pStyles[i].pStyle;
			EG_StyleValue_t Value;
      // If there is Layout difference on the main part, return immediately. There is no more serious difference
			if(pStyle->GetProperty(EG_STYLE_PAD_TOP, &Value))	LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_PAD_BOTTOM, &Value))	LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_PAD_LEFT, &Value))	LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_PAD_RIGHT, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_PAD_COLUMN, &Value))	LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_PAD_ROW, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_LAYOUT, &Value))	LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_TRANSLATE_X, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_TRANSLATE_Y, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_WIDTH, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_HEIGHT, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_MIN_WIDTH, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_MAX_WIDTH, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_MIN_HEIGHT, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_MAX_HEIGHT, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_BORDER_WIDTH, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_TRANSFORM_ANGLE, &Value)) LayoutDifferent = true;
			else if(pStyle->GetProperty(EG_STYLE_TRANSFORM_ZOOM, &Value)) LayoutDifferent = true;
			if(LayoutDifferent) return _EG_STYLE_STATE_CMP_DIFF_LAYOUT;
			// Check for draw pad changes
			if(pStyle->GetProperty(EG_STYLE_TRANSFORM_WIDTH, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_TRANSFORM_HEIGHT, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_TRANSFORM_ANGLE, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_TRANSFORM_ZOOM, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_OUTLINE_OPA, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_OUTLINE_PAD, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_OUTLINE_WIDTH, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_SHADOW_WIDTH, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_SHADOW_OPA, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_SHADOW_OFS_X, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_SHADOW_OFS_Y, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_SHADOW_SPREAD, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(pStyle->GetProperty(EG_STYLE_LINE_WIDTH, &Value)) Result = _EG_STYLE_STATE_CMP_DIFF_DRAW_PAD;
			else if(Result == _EG_STYLE_STATE_CMP_SAME) Result = _EG_STYLE_STATE_CMP_DIFF_REDRAW;
		}
	}
	return Result;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::RefreshStyle(EG_StyleFlags_t SelectFlags, EGStyleProperty_e Property)
{
	if(!g_StyleRefreshEnable) return;
	Invalidate();
	EGPart_t Part = GetSelectorPart(SelectFlags);
	bool IsLayoutReference = PropertyHasFlag(Property, EG_STYLE_PROP_LAYOUT_REFRESH);
	bool IsExtDraw = PropertyHasFlag(Property, EG_STYLE_PROP_EXT_DRAW);
	bool IsInheritable = PropertyHasFlag(Property, EG_STYLE_PROP_INHERIT);
	bool IsLayerReference = PropertyHasFlag(Property, EG_STYLE_PROP_LAYER_REFR);
	if(IsLayoutReference) {
		if((Part == EG_PART_ANY) ||	(Part == EG_PART_MAIN) ||
			(GetStyleHeight(0) == EG_SIZE_CONTENT) ||	(GetStyleWidth(0) == EG_SIZE_CONTENT)) {
			EGEvent::EventSend(this, EG_EVENT_STYLE_CHANGED, nullptr);
			MarkLayoutDirty();
		}
	}
	if((Part == EG_PART_ANY || Part == EG_PART_MAIN) && (Property == EG_STYLE_PROP_ANY || IsLayoutReference)) {
		if(m_pParent) m_pParent->MarkLayoutDirty();
	}
	if((Part == EG_PART_ANY || Part == EG_PART_MAIN) && IsLayerReference) {	// Cache the layer type
		EG_LayerType_e LayerType = CalculateLayerType(this);
		if(m_pAttributes)	m_pAttributes->LayerType = LayerType;
		else{
      if(LayerType != EG_LAYER_TYPE_NONE) {
			  AllocateAttribute();
			  m_pAttributes->LayerType = LayerType;
      }
		}
	}
	if(Property == EG_STYLE_PROP_ANY || IsExtDraw) {
		RefreshExtDrawSize();
	}
	Invalidate();
	if(Property == EG_STYLE_PROP_ANY || (IsInheritable && (IsExtDraw || IsLayoutReference))) {
		if(Part != EG_PART_SCROLLBAR) RefreshChildrenStyle();
	}
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::RefreshChildrenStyle(void)
{
	uint32_t i;
	uint32_t ChildCount = GetChildCount();
	for(i = 0; i < ChildCount; i++) {
		EGObject *pChild = m_pAttributes->ppChildren[i];
		pChild->Invalidate();
		EGEvent::EventSend(pChild, EG_EVENT_STYLE_CHANGED, nullptr);
		pChild->Invalidate();
		pChild->RefreshChildrenStyle(); // Check Children too
	}
}

////////////////////////////////////////////////////////////////////////////////

void* EGObject::GetGroup(void)
{
	if(m_pAttributes) return m_pAttributes->pGroup;
	else return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::AllocateAttribute(void)
{
	if(m_pAttributes == nullptr) {
		m_pAttributes = (EGObjAttributes_t*)EG_AllocMem(sizeof(EGObjAttributes_t));
		EG_ASSERT_MALLOC(m_pAttributes);
		if(m_pAttributes == nullptr) return;
		EG_ZeroMem(m_pAttributes, sizeof(EGObjAttributes_t));
    m_pAttributes->pScroll = new EGPoint;
		m_pAttributes->ScrollDirection = EG_DIR_ALL;
		m_pAttributes->ScrollbarMode = EG_SCROLLBAR_MODE_AUTO;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::IsKindOf(const EGObject *pObj, const EG_ClassType_t *pClassType)
{
  if(pObj == nullptr) return false;
	return pObj->m_pClass == pClassType ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::HasClass(const EG_ClassType_t *pClassType)
{
  const EG_ClassType_t *pClass = OBJ_CLASS;
	while(pClass) {
		if(pClass == pClassType) return true;
		pClass = pClass->pBaseClassType;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::IsValid(void)
{
	EGDisplay *pDisplay = EGDisplay::GetNext(nullptr);
	while(pDisplay) {
		uint32_t i;
		for(i = 0; i < pDisplay->m_ScreenCount; i++) {
			if(pDisplay->m_pScreens[i] == this) return true;
			bool found = ValidChild(pDisplay->m_pScreens[i], this);
			if(found) return true;
		}
		pDisplay = EGDisplay::GetNext(pDisplay);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::SetLocalStyleProperty(EGStyleProperty_e Property, EG_StyleValue_t Value, EG_StyleFlags_t SelectFlags)
{
	EGStyle *pStyle = GetLocalStyle(SelectFlags);
	pStyle->SetProperty(Property, Value);
	RefreshStyle(SelectFlags, Property);
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::SetLocalStylePropertyMeta(EGStyleProperty_e Property, uint16_t Meta, EG_StyleFlags_t SelectFlags)
{
	EGStyle *pStyle = GetLocalStyle(SelectFlags);
	pStyle->SetPropertyMeta(Property, Meta);
	RefreshStyle(SelectFlags, Property);
}

////////////////////////////////////////////////////////////////////////////////

EG_StyleResult_t EGObject::GetLocalStylelProperty(EGStyleProperty_e Property, EG_StyleValue_t *pValue, EG_StyleFlags_t SelectFlags) const
{
	for(uint32_t i = 0; i < m_StyleCount; i++){
		if(m_pStyles[i].IsLocal && m_pStyles[i].SelectFlags == SelectFlags){
		  EG_StyleResult_t Res = m_pStyles[i].pStyle->GetProperty(Property, pValue);
      return Res;
		}
	}
	return EG_STYLE_RES_NOT_FOUND;
}

////////////////////////////////////////////////////////////////////////////////

EGStyle* EGObject::GetLocalStyle(EG_StyleFlags_t SelectFlags, bool AutoAdd /*= true*/)
{
uint32_t i;

	for(i = 0; i < m_StyleCount; i++) {
		if(m_pStyles[i].IsLocal && m_pStyles[i].SelectFlags == SelectFlags){
      return m_pStyles[i].pStyle;
    }
	}
  if(!AutoAdd) return nullptr;
	m_StyleCount++; // not found - add a new one
	m_pStyles = (EG_ObjStyle_t*)EG_ReallocMem(m_pStyles, m_StyleCount * sizeof(EG_ObjStyle_t));
	EG_ASSERT_MALLOC(m_pStyles);
	for(i = m_StyleCount - 1; i > 0; i--) {
	// Copy only normal m_pStyles (not local and transition). The new local style will be added as the last local style
		if(m_pStyles[i - 1].IsLocal || m_pStyles[i - 1].IsTransition) break;
		m_pStyles[i] = m_pStyles[i - 1];
	}
  EG_ZeroMem(&m_pStyles[i], sizeof(EG_ObjStyle_t));
	m_pStyles[i].pStyle = new EGStyle;
	m_pStyles[i].IsLocal = 1;
	m_pStyles[i].SelectFlags = SelectFlags;
//  ESP_LOGI("[Object]", "Get Local Obj:%p, Style - new Style:%p, Count:%d", (void*)this, (void*)m_pStyles[i].pStyle, m_StyleCount);
	return m_pStyles[i].pStyle;
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::RemoveStyleProperty(EGStyleProperty_e Property, EG_StyleFlags_t SelectFlags)
{
uint32_t i;

	// Find the style
	for(i = 0; i < m_StyleCount; i++) if(m_pStyles[i].IsLocal && m_pStyles[i].SelectFlags == SelectFlags) break;
	if(i == m_StyleCount) return false;	// The style is not found
	EG_Result_t Res = m_pStyles[i].pStyle->RemoveProperty(Property);
	if(Res == EG_RES_OK) RefreshStyle(SelectFlags, Property);
	return Res;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::Draw(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_COVER_CHECK:{
      EG_CoverCheckInfo_t *pInfo = (EG_CoverCheckInfo_t*)pEvent->GetParam();
      if(pInfo->Result == EG_COVER_RES_MASKED) break;
      if(GetStyleClipCorner(EG_PART_MAIN)) {
        pInfo->Result = EG_COVER_RES_MASKED;
        break;
      }
      // Most trivial test. Is the mask fully IN the object? If no it surely doesn't cover it
      EG_Coord_t Radius = GetStyleRadius(EG_PART_MAIN);
      EG_Coord_t Width = GetStyleTransformWidth(EG_PART_MAIN);
      EG_Coord_t Height = GetStyleTransformHeight(EG_PART_MAIN);
      EGRect Rect(m_Rect);
      Rect.Inflate(Width, Height);
      if(pInfo->pRect->IsInside(&Rect, Radius) == false) {
        pInfo->Result = EG_COVER_RES_NOT_COVER;
        break;
      }
      if(GetStyleBckgroundOPA(EG_PART_MAIN) < EG_OPA_MAX) {
        pInfo->Result = EG_COVER_RES_NOT_COVER;
        break;
      }
      if(GetStyleOPA(EG_PART_MAIN) < EG_OPA_MAX) {
        pInfo->Result = EG_COVER_RES_NOT_COVER;
        break;
      }
      pInfo->Result = EG_COVER_RES_COVER;
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      const EGDrawContext *pContext = pEvent->GetDrawContext();
      EGDrawRect DrawRect;
      if(GetStyleBorderPost(EG_PART_MAIN)) {		// If the border is drawn later disable loading its properties
        DrawRect.m_BorderPost = 1;
      }
      InititialseDrawRect(EG_PART_MAIN, &DrawRect);
      EG_Coord_t Width = GetStyleTransformWidth(EG_PART_MAIN);
      EG_Coord_t Height = GetStyleTransformHeight(EG_PART_MAIN);
      EGRect Rect(m_Rect);
      Rect.Inflate(Width, Height);
      EGDrawDiscriptor DrawDiscriptor;
      DrawDiscriptor.m_pContext = pContext;
      DrawDiscriptor.m_pClass = OBJ_CLASS;
      DrawDiscriptor.m_Type = EG_OBJ_DRAW_PART_RECTANGLE;
      DrawDiscriptor.m_pDrawRect = &DrawRect;
      DrawDiscriptor.m_pRect = &Rect;
      DrawDiscriptor.m_Part = EG_PART_MAIN;
      EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
#if EG_DRAW_COMPLEX
      // With clip corner enabled draw the bg img separately to make it clipped
      bool ClipCorner = (GetStyleClipCorner(EG_PART_MAIN) && DrawRect.m_Radius != 0) ? true : false;
      const void *pBackImageSource = DrawRect.m_pBackImageSource;
      if(ClipCorner) {
        DrawRect.m_pBackImageSource = nullptr;
      }
#endif
      DrawRect.Draw(pContext, &Rect);
#if EG_DRAW_COMPLEX
      if(ClipCorner) {
        MaskRadiusParam_t *pRadiusMask = (MaskRadiusParam_t*)EG_GetBufferMem(sizeof(MaskRadiusParam_t));
        DrawMaskSetRadius(pRadiusMask, &m_Rect, DrawRect.m_Radius, false);
        // Add the mask and use `obj+8` as custom id. Don't use `obj` directly because it might be used by the user
        DrawMaskAdd(pRadiusMask, this + 8);
        if(pBackImageSource) {
          DrawRect.m_BackgroundOPA = EG_OPA_TRANSP;
          DrawRect.m_BorderOPA = EG_OPA_TRANSP;
          DrawRect.m_OutlineOPA = EG_OPA_TRANSP;
          DrawRect.m_ShadowOPA = EG_OPA_TRANSP;
          DrawRect.m_pBackImageSource = pBackImageSource;
          DrawRect.Draw(pContext, &Rect);
        }
      }
#endif
      EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
      break;
    }
    case EG_EVENT_DRAW_POST:{
      EGDrawContext *pDrawContext = pEvent->GetDrawContext();
      DrawScrollbar(pDrawContext);
#if EG_DRAW_COMPLEX
      if(GetStyleClipCorner(EG_PART_MAIN)) {
        MaskRadiusParam_t *param = (MaskRadiusParam_t*)DrawMaskRemoveReferenced(this + 8);
        if(param) {
          DrawMaskFreeParam(param);
          EG_ReleaseBufferMem(param);
        }
      }
#endif
      if(GetStyleBorderPost(EG_PART_MAIN)) {	//If the border is drawn later disable loading other properties
        EGDrawRect DrawRect;
        DrawRect.m_BackgroundOPA = EG_OPA_TRANSP;
        DrawRect.m_BackImageOPA = EG_OPA_TRANSP;
        DrawRect.m_OutlineOPA = EG_OPA_TRANSP;
        DrawRect.m_ShadowOPA = EG_OPA_TRANSP;
        InititialseDrawRect(EG_PART_MAIN, &DrawRect);
        EG_Coord_t Width = GetStyleTransformWidth(EG_PART_MAIN);
        EG_Coord_t Height = GetStyleTransformHeight(EG_PART_MAIN);
        EGRect Rect(m_Rect);
        Rect.Inflate(Width, Height);
        EGDrawDiscriptor DrawDiscriptor;
        InitDrawDescriptor(&DrawDiscriptor, pDrawContext);
        DrawDiscriptor.m_pClass = OBJ_CLASS;
        DrawDiscriptor.m_Type = EG_OBJ_DRAW_PART_BORDER_POST;
        DrawDiscriptor.m_pDrawRect = &DrawRect;
        DrawDiscriptor.m_pRect = &Rect;
        DrawDiscriptor.m_Part = EG_PART_MAIN;
        EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
        DrawRect.Draw(pDrawContext, &Rect);
        EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
      }
      break;
    }
    default:{
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::DrawScrollbar(EGDrawContext *pDrawContext)
{
EGRect HorizontalArea, VerticalArea;
EGDrawRect DrawRect;

	GetScrollbarArea(&HorizontalArea, &VerticalArea);
	if(HorizontalArea.GetSize() <= 0 && VerticalArea.GetSize() <= 0) return;
	EG_Result_t sb_res = InitialiseScrollbarDrawDsc(&DrawRect);
	if(sb_res != EG_RES_OK) return;
	EGDrawDiscriptor DrawDiscriptor;
	InitDrawDescriptor(&DrawDiscriptor, pDrawContext);
	DrawDiscriptor.m_pClass = OBJ_CLASS;
	DrawDiscriptor.m_Type = EG_OBJ_DRAW_PART_SCROLLBAR;
	DrawDiscriptor.m_pDrawRect = &DrawRect;
	DrawDiscriptor.m_Part = EG_PART_SCROLLBAR;
	if(HorizontalArea.GetSize() > 0) {
		DrawDiscriptor.m_pRect = &HorizontalArea;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		DrawRect.Draw(pDrawContext, &HorizontalArea);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	}
	if(VerticalArea.GetSize() > 0) {
		DrawDiscriptor.m_pRect = &VerticalArea;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		DrawDiscriptor.m_pRect = &VerticalArea;
		DrawRect.Draw(pDrawContext, &VerticalArea);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	}
}

////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGObject::InitialiseScrollbarDrawDsc(EGDrawRect *pDiscriptor)
{
	pDiscriptor->m_BackgroundOPA = GetStyleBckgroundOPA(EG_PART_SCROLLBAR);
	if(pDiscriptor->m_BackgroundOPA > EG_OPA_MIN) {
		pDiscriptor->m_BackgroundColor = GetStyleBackColor(EG_PART_SCROLLBAR);
	}
	pDiscriptor->m_BorderOPA = GetStyleBorderOPA(EG_PART_SCROLLBAR);
	if(pDiscriptor->m_BorderOPA > EG_OPA_MIN) {
		pDiscriptor->m_BorderWidth = GetStyleBorderWidth(EG_PART_SCROLLBAR);
		if(pDiscriptor->m_BorderWidth > 0) {
			pDiscriptor->m_BorderColor = GetStyleBorderColor(EG_PART_SCROLLBAR);
		}
		else pDiscriptor->m_BorderOPA = EG_OPA_TRANSP;
	}
#if EG_DRAW_COMPLEX
	pDiscriptor->m_ShadowOPA = GetStyleShadowOPA(EG_PART_SCROLLBAR);
	if(pDiscriptor->m_ShadowOPA > EG_OPA_MIN) {
		pDiscriptor->m_ShadowWidth = GetStyleShadowWidth(EG_PART_SCROLLBAR);
		if(pDiscriptor->m_ShadowWidth > 0) {
			pDiscriptor->m_ShadowSpread = GetStyleShadowSpread(EG_PART_SCROLLBAR);
			pDiscriptor->m_ShadowColor = GetStyleShadowColor(EG_PART_SCROLLBAR);
		}
		else {
			pDiscriptor->m_ShadowOPA = EG_OPA_TRANSP;
		}
	}
	EG_OPA_t OPA = GetOPARecursive(this, EG_PART_SCROLLBAR);
	if(OPA < EG_OPA_MAX) {
		pDiscriptor->m_BackgroundOPA = (pDiscriptor->m_BackgroundOPA * OPA) >> 8;
		pDiscriptor->m_BorderOPA = (pDiscriptor->m_BackgroundOPA * OPA) >> 8;
		pDiscriptor->m_ShadowOPA = (pDiscriptor->m_BackgroundOPA * OPA) >> 8;
	}
	if(pDiscriptor->m_BackgroundOPA != EG_OPA_TRANSP || pDiscriptor->m_BorderOPA != EG_OPA_TRANSP || pDiscriptor->m_ShadowOPA != EG_OPA_TRANSP) {
		pDiscriptor->m_Radius = GetStyleRadius(EG_PART_SCROLLBAR);
		return EG_RES_OK;
	}
	else {
		return EG_RES_INVALID;
	}
#else
	if(pDiscriptor->bg_opa != EG_OPA_TRANSP || pDiscriptor->border_opa != EG_OPA_TRANSP)
		return EG_RES_OK;
	else
		return EG_RES_INVALID;
#endif
}

////////////////////////////////////////////////////////////////////////////////

bool EGObject::ValidChild(const EGObject *pParent, const EGObject *pFind)
{
uint32_t ChildCount = 0;	// Check all Children of `parent`

	if(pParent->m_pAttributes) ChildCount = pParent->m_pAttributes->ChildCount;
	for(uint32_t i = 0; i < ChildCount; i++) {
		EGObject *pChild = pParent->m_pAttributes->ppChildren[i];
		if(pChild == pFind) return true;
		if(ValidChild(pChild, pFind)) return true;		// Check the Children
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

void EGObject::RemoveStyle(EGStyle *pStyle, EG_StyleFlags_t SelectFlags)
{
bool Deleted = false;
uint32_t i = 0;

	EGState_t State = GetSelectorState(SelectFlags);
	EGPart_t Part = GetSelectorPart(SelectFlags);
	EGStyleProperty_e Property = EG_STYLE_PROP_ANY;
	if(pStyle && pStyle->m_PropertyCount == 0) Property = EG_STYLE_PROP_INV;
// 	EG_LOG_WARN("Obj:%p, i: %d, Count: %d, Styles: %p, Select:%d", (void*)this, i, m_StyleCount, m_pStyles[i], m_pStyles[i].SelectFlags);
	while(i < m_StyleCount) {
		EGState_t ActiveState = GetSelectorState(m_pStyles[i].SelectFlags);
		EGPart_t ActivePart = GetSelectorPart(m_pStyles[i].SelectFlags);
		if((State != EG_STATE_ANY && ActiveState != State) || (Part != EG_PART_ANY && ActivePart != Part) || (pStyle != nullptr && pStyle != m_pStyles[i].pStyle)) {
			i++;
			continue;
		}
		if(m_pStyles[i].IsTransition) {
			TransitionDelete(Part, EG_STYLE_PROP_ANY, nullptr);
		}
		if(m_pStyles[i].IsLocal || m_pStyles[i].IsTransition) {
			m_pStyles[i].pStyle->Reset();
		}
		m_StyleCount--;
		for(uint32_t j = i; j < (uint32_t)m_StyleCount; j++) m_pStyles[j] = m_pStyles[j + 1];		// Shift the m_pStyles after `i` by one
		m_pStyles = (EG_ObjStyle_t *)EG_ReallocMem(m_pStyles, m_StyleCount * sizeof(EG_ObjStyle_t));
		Deleted = true;
	}	// The style[i] is removed, so [i] points to the next style and therefore doesn't needs to be incremented
	if(Deleted && Property != EG_STYLE_PROP_INV) RefreshStyle(Part, Property);
}

////////////////////////////////////////////////////////////////////////////////

EGStyle* EGObject::GetTransitionStyle(EG_StyleFlags_t SelectFlags)
{
uint32_t i;

	for(i = 0; i < m_StyleCount; i++){
		if(m_pStyles[i].IsTransition && m_pStyles[i].SelectFlags == SelectFlags) break;
	}
	if(i != m_StyleCount) return m_pStyles[i].pStyle;	// Already have a transition style for it
	m_StyleCount++;
	m_pStyles = (EG_ObjStyle_t*)EG_ReallocMem(m_pStyles, m_StyleCount * sizeof(EG_ObjStyle_t));
	for(i = m_StyleCount - 1; i > 0; i--) m_pStyles[i] = m_pStyles[i - 1];
  EG_ZeroMem(&m_pStyles[0], sizeof(EG_ObjStyle_t));
	m_pStyles[0].pStyle = new EGStyle;
	m_pStyles[0].IsTransition = 1;
	m_pStyles[0].SelectFlags = SelectFlags;
	return m_pStyles[0].pStyle;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

void EGObject::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  EGObject *pObj = pEvent->GetTarget();
  pObj->Event(pEvent);
}
////////////////////////////////////////////////////////////////////////////////

void EGObject::Event(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_PRESSED:{
      AddState(EG_STATE_PRESSED);
      break;
    }
    case EG_EVENT_RELEASED:{
      ClearState(EG_STATE_PRESSED);
      EGInputDevice *pIndev = pEvent->GetInputDevice();
      if(pIndev->GetScrollObj() == nullptr && HasFlagSet(EG_OBJ_FLAG_CHECKABLE)) {  // Go the checked State if enabled
        if(!(GetState() & EG_STATE_CHECKED)) AddState(EG_STATE_CHECKED);
        else ClearState(EG_STATE_CHECKED);
        EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);
      }
      break;
    }
    case EG_EVENT_PRESS_LOST:{
      ClearState(EG_STATE_PRESSED);
      break;
    }
    case EG_EVENT_STYLE_CHANGED:{
      uint32_t ChildCount = GetChildCount();
      for(uint32_t i = 0; i < ChildCount; i++) {
        EGObject *pChild = m_pAttributes->ppChildren[i];
        pChild->MarkLayoutDirty();
      }
      break;
    }
    case EG_EVENT_KEY:{
      if(HasFlagSet(EG_OBJ_FLAG_CHECKABLE)) {
        char Key = *((char *)pEvent->GetParam());
        if(Key == EG_KEY_RIGHT || Key == EG_KEY_UP) AddState(EG_STATE_CHECKED);
        else if(Key == EG_KEY_LEFT || Key == EG_KEY_DOWN) ClearState(EG_STATE_CHECKED);
        if(Key != EG_KEY_ENTER) EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr); // With Enter EG_EVENT_RELEASED will send VALUE_CHANGE event
      }
      else if(HasFlagSet(EG_OBJ_FLAG_SCROLLABLE | EG_OBJ_FLAG_SCROLL_WITH_ARROW) && !IsEditable()) {
        EG_AnimateEnable_e AnimateEnable = EG_ANIM_OFF;     // scroll by keypad or encoder
        EG_Coord_t Left = GetScrollLeft();
        EG_Coord_t Right = GetScrollRight();
        switch(*((char *)pEvent->GetParam())){
          case EG_KEY_DOWN:{
            ScrollToY(GetScrollY() + GetHeight() / 4, AnimateEnable);
            break;
          }
          case EG_KEY_UP:{
            ScrollToY(GetScrollY() - GetHeight() / 4, AnimateEnable);
            break;
          }
          case EG_KEY_RIGHT:{     // If the object can't be scrolled horizontally then scroll it vertically
            if(!((GetScrollDirection() & EG_DIR_HOR) && (Left > 0 || Right > 0))) ScrollToY(GetScrollY() + GetHeight() / 4, AnimateEnable);
            else ScrollToX(GetScrollX() + GetWidth() / 4, AnimateEnable);
            break;
          }
          case EG_KEY_LEFT:{  // If the object can't be scrolled horizontally then scroll it vertically
            if(!((GetScrollDirection() & EG_DIR_HOR) && (Left > 0 || Right > 0))) ScrollToY(GetScrollY() - GetHeight() / 4, AnimateEnable);
            else ScrollToX(GetScrollX() - GetWidth() / 4, AnimateEnable);
            break;
          }
          default: break;
        }
      }
      break;
    }
    case EG_EVENT_FOCUSED:{
      if(HasFlagSet(EG_OBJ_FLAG_SCROLL_ON_FOCUS)) ScrollToViewRecursive(EG_ANIM_ON);
      EGGroup *pGroup = (EGGroup*)GetGroup();
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
      EGState_t State = EG_STATE_FOCUSED;
      // if the obj was focused manually it returns NULL so try to use the indev from the event
      EGInputDevice *pIndev = EGInputDevice::GetActive();
      if(pIndev == nullptr) pIndev = pEvent->GetInputDevice();
      EG_InDeviceType_e IndevType = pIndev->GetType();
      if(IndevType == EG_INDEV_TYPE_KEYPAD || IndevType == EG_INDEV_TYPE_ENCODER) State |= EG_STATE_FOCUS_KEY;
      if(IsEditing){
        State |= EG_STATE_EDITED;
        AddState(State);
      }
      else{
        AddState(State);
        ClearState(EG_STATE_EDITED);
      }
      break;
    }
    case EG_EVENT_SCROLL_BEGIN:{
      AddState(EG_STATE_SCROLLED);
      break;
    }
    case EG_EVENT_SCROLL_END:{
      ClearState(EG_STATE_SCROLLED);
      if(GetScrollbarMode() == EG_SCROLLBAR_MODE_ACTIVE) {
        EGRect HorizontalArea, VerticalArea;
        GetScrollbarArea(&HorizontalArea, &VerticalArea);
        InvalidateArea(&HorizontalArea);
        InvalidateArea(&VerticalArea);
      }
      break;
    }
    case EG_EVENT_DEFOCUSED:{
      ClearState(EG_STATE_FOCUSED | EG_STATE_EDITED | EG_STATE_FOCUS_KEY);
      break;
    }
    case EG_EVENT_SIZE_CHANGED:{
      EG_AlignType_e Align = GetStyleAlign(EG_PART_MAIN);
      uint32_t Layout = GetStyleLayout(EG_PART_MAIN);
      if(Layout || Align) MarkLayoutDirty();
      uint32_t ChildCount = GetChildCount();
      for(uint32_t i = 0; i < ChildCount; i++) {
        EGObject *pChild = m_pAttributes->ppChildren[i];
        pChild->MarkLayoutDirty();
      }
      break;
    }
    case EG_EVENT_CHILD_CHANGED:{
      EG_Coord_t Width = GetStyleWidth(EG_PART_MAIN);
      EG_Coord_t Height = GetStyleHeight(EG_PART_MAIN);
      EG_AlignType_e Align = GetStyleAlign(EG_PART_MAIN);
      uint32_t Layout = GetStyleLayout(EG_PART_MAIN);
      if(Layout || Align || Width == EG_SIZE_CONTENT || Height == EG_SIZE_CONTENT) {
        MarkLayoutDirty();
      }
      break;
    }
    case EG_EVENT_CHILD_DELETED:{
      m_ReadScrollAfterLayout = 1;
      MarkLayoutDirty();
      break;
    }
    case EG_EVENT_REFR_EXT_DRAW_SIZE:{
      EG_Coord_t Size = CalculateExtDrawSize(EG_PART_MAIN);
      pEvent->SetExtDrawSize(Size);
      break;
    }
    case EG_EVENT_DRAW_MAIN:
    case EG_EVENT_DRAW_POST:
    case EG_EVENT_COVER_CHECK:{
      Draw(pEvent);
      break;
    }
    default: break;  // keep compiler happy
  }
}



