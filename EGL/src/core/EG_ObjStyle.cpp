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

#include "core/EG_Object.h"
#include "core/EG_ObjStyle.h"
#include "core/EG_Display.h"
#include "misc/EG_Misc.h"
//#include "core/EG_Object.h"

#define OBJ_CLASS &c_ObjectClass

bool EGObject::g_StyleRefreshEnable = true;

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::AddStyle(EGStyle *pStyle, EG_StyleFlags_t SelectFlags)
{
uint32_t i, j;

//	EG_LOG_WARN("Obj: %p, Style: %p, Count: %p", (void*)this, (void*)pStyle, m_StyleCount);
	TransitionDelete(SelectFlags, EG_STYLE_PROP_ANY, nullptr);
	for(i = 0; i < m_StyleCount; i++) {	        // Find a position after the transition and local m_pStyles
		if(m_pStyles[i].IsTransition) continue;
		if(m_pStyles[i].IsLocal) continue;
		break;
	}	// Now `i` is at the first normal style. Insert the new style before this
	m_StyleCount++;	// Allocate space for the new style and shift the rest of the style to the end
	m_pStyles = (EG_ObjStyle_t*)EG_ReallocMem(m_pStyles, m_StyleCount * sizeof(EG_ObjStyle_t));
	for(j = m_StyleCount - 1; j > i; j--)	m_pStyles[j] = m_pStyles[j - 1];
  EG_ZeroMem(&m_pStyles[i], sizeof(EG_ObjStyle_t));
	m_pStyles[i].pStyle = pStyle;
	m_pStyles[i].SelectFlags = SelectFlags;
	m_pStyles[i].IsTransition = 0;
	m_pStyles[i].IsLocal = 0;
	RefreshStyle(SelectFlags, EG_STYLE_PROP_ANY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::ReportStyleChange(EGStyle *pStyle)
{
	if(!g_StyleRefreshEnable) return;
	EGDisplay *pDisplay = EGDisplay::GetNext(nullptr);
	while(pDisplay) {
		for(uint32_t i = 0; i < pDisplay->m_ScreenCount; i++) {
			ReportStyleChangeCore(pStyle, pDisplay->m_pScreens[i]);
		}
		pDisplay = EGDisplay::GetNext(pDisplay);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::EnableStyleRefresh(bool Enable)
{
	g_StyleRefreshEnable = Enable;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::FadeIn(uint32_t Time, uint32_t Delay)
{
	EGAnimate Animation;
	Animation.SetItem(this);
	Animation.SetValues(0, EG_OPA_COVER);
	Animation.SetExcCB(FadeAnimationCB);
	Animation.SetEndCB(FadeInAnimationEnd);
	Animation.SetTime(Time);
	Animation.SetDelay(Delay);
	EGAnimate::Create(&Animation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::FadeOut(uint32_t Time, uint32_t Delay)
{
	EGAnimate Animation;
	Animation.SetItem(this);
	Animation.SetValues(GetStyleOPA(0), EG_OPA_TRANSP);
	Animation.SetExcCB(FadeAnimationCB);
	Animation.SetTime(Time);
	Animation.SetDelay(Delay);
	EGAnimate::Create(&Animation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_TextAlignment_t EGObject::CalculateTextAlignment(EGPart_t Part, const char *pText)
{
	EG_TextAlignment_t Align = GetStyleTextAlign(Part);
	EG_BaseDirection_e BaseDirection = GetStyleBaseDirection(Part);
	lv_bidi_calculate_align(&Align, &BaseDirection, pText);
	return Align;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::ReportStyleChangeCore(EGStyle *pStyle, EGObject *pObj)
{
uint32_t i;

	for(i = 0; i < pObj->m_StyleCount; i++) {
		if((pStyle == nullptr) || (pObj->m_pStyles[i].pStyle == pStyle)) {
			pObj->RefreshStyle(EG_PART_ANY, EG_STYLE_PROP_ANY);
			break;
		}
	}
	uint32_t ChildCount = pObj->GetChildCount();
	for(i = 0; i < ChildCount; i++) {
		ReportStyleChangeCore(pStyle, pObj->m_pAttributes->ppChildren[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_StyleValue_t EGObject::ApplyColorFilter(uint32_t Part, EG_StyleValue_t Value)
{
	const EG_ColorFilterProps_t *pFilter = GetStyleColorFilterDiscriptor(Part);
  if(pFilter) ESP_LOGI("[ObjStl]", "ApplyFilterCB %d - %p.", Part, pFilter);
	if((pFilter != nullptr) && (pFilter->FilterCB != nullptr)){
		EG_OPA_t FilterOPA = GetStyleColorFilterOPA(Part);
		if(FilterOPA != 0) Value.Color = pFilter->FilterCB(pFilter, Value.Color, FilterOPA);
	}
	return Value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_StyleValue_t EGObject::GetProperty(EGPart_t Part, EGStyleProperty_e Property) const
{
EG_StyleValue_t ActiveValue;
EG_StyleResult_t Found = EG_STYLE_RES_NOT_FOUND;
const EGObject *pObj = this;

  ActiveValue.Number = 0;
	bool Inheritable = PropertyHasFlag(Property, EG_STYLE_PROP_INHERIT);
	while(pObj) {
		Found = EGObject::GetPropertyCore(pObj, Part, Property, &ActiveValue);
    if(Found == EG_STYLE_RES_FOUND) break;
    if(!Inheritable) break;
		if(Found != EG_STYLE_RES_INHERIT && Part != EG_PART_MAIN) {	// If not found, check the `MAIN` style first
			Part = EG_PART_MAIN;
			continue;
		}
		pObj = pObj->GetParent();		// Check the parent too.
	}
 	if(Found != EG_STYLE_RES_FOUND) {
		if(Part == EG_PART_MAIN && (Property == EG_STYLE_WIDTH || Property == EG_STYLE_HEIGHT)) {
			const EG_ClassType_t *pClass = pObj->m_pClass;
			while(pClass) {
				if(Property == EG_STYLE_WIDTH) {
					if(pClass->WidthDef != 0) break;
				}
				else {
					if(pClass->HeightDef != 0) break;
				}
				pClass = pClass->pBaseClassType;
			}
			if(pClass) ActiveValue.Number = (Property == EG_STYLE_WIDTH) ? pClass->WidthDef : pClass->HeightDef;
			else ActiveValue.Number = 0;
		}
		else return GetDefaultProperty(Property);
	}
	return ActiveValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_StyleResult_t EGObject::GetPropertyCore(const EGObject *pObj, EGPart_t Part, EGStyleProperty_e Property, EG_StyleValue_t *pValue)
{
uint32_t i;
EG_StyleValue_t Value;
int32_t Weight = -1;
EG_StyleResult_t Found;

	uint8_t Group = (1 << GetPropertyGroup(Property));
	EGState_t State = pObj->m_State;
	EGState_t InvertedState = ~State;
	bool SkipTransition = pObj->m_SkipTransition;
	for(i = 0; i < pObj->m_StyleCount; i++) {
    EG_ObjStyle_t *pStyle = &pObj->m_pStyles[i];
		if(pStyle->IsTransition == false) break;
		if(SkipTransition) continue;
		EGPart_t ActivePart = GetSelectorPart(pStyle->SelectFlags);
		if(ActivePart != Part) continue;
		if((pStyle->pStyle->m_HasGroup & Group) == 0) continue;
		Found = pStyle->pStyle->GetProperty(Property, &Value);
		if(Found == EG_STYLE_RES_FOUND) {
			*pValue = Value;
			return EG_STYLE_RES_FOUND;
		}
		if(Found == EG_STYLE_RES_INHERIT) return EG_STYLE_RES_INHERIT;
	}
	for(; i < pObj->m_StyleCount; i++) {
    EG_ObjStyle_t *pStyle = &pObj->m_pStyles[i];
	  if((pStyle->pStyle->m_HasGroup & Group) == 0) continue;
		EGPart_t ActivePart = GetSelectorPart(pStyle->SelectFlags);
		EGState_t ActiveState = GetSelectorState(pStyle->SelectFlags);
		if(ActivePart != Part) continue;
		// Verify the style doesn't specifies other States other than the requested.
    // E.g. For HOVER+PRESS object State, only HOVER style is OK, but HOVER+FOCUS style is not
		if((ActiveState & InvertedState)) continue;
		if(ActiveState <= Weight) continue;		// Check only better candidates
		Found = pStyle->pStyle->GetProperty(Property, &Value);
		if(Found == EG_STYLE_RES_FOUND) {
			if(ActiveState == State) {
				*pValue = Value;
				return EG_STYLE_RES_FOUND;
			}
			if(Weight < ActiveState) {
				Weight = ActiveState;
				*pValue = Value;
			}
		}
		else if(Found == EG_STYLE_RES_INHERIT) return EG_STYLE_RES_INHERIT;
	}
	if(Weight >= 0) {
		*pValue = Value;
		return EG_STYLE_RES_FOUND;
	}
	return EG_STYLE_RES_NOT_FOUND;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_OPA_t EGObject::GetOPARecursive(EGObject *pObj, EGPart_t Part)
{
	EG_OPA_t StyleOPA = pObj->GetStyleOPA(Part);
	if(StyleOPA <= EG_OPA_MIN) return EG_OPA_TRANSP;
	EG_OPA_t ResultOPA = EG_OPA_COVER;
	if(StyleOPA < EG_OPA_MAX) {
		ResultOPA = ((uint32_t)ResultOPA * StyleOPA) >> 8; 
	}
	if(Part != EG_PART_MAIN) Part = EG_PART_MAIN;
	else pObj = pObj->GetParent();
	while(pObj) {
		StyleOPA = pObj->GetStyleOPA(Part);
		if(StyleOPA <= EG_OPA_MIN) return EG_OPA_TRANSP;
		if(StyleOPA < EG_OPA_MAX) ResultOPA = ((uint32_t)ResultOPA * StyleOPA) >> 8;
		pObj = pObj->GetParent();
	}
	if(ResultOPA <= EG_OPA_MIN) return EG_OPA_TRANSP;
	if(ResultOPA >= EG_OPA_MAX) return EG_OPA_COVER;
	return ResultOPA;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

EG_LayerType_e EGObject::CalculateLayerType(EGObject *pObj)
{
	if(pObj->GetStyleTransformAngle(0) != 0) return EG_LAYER_TYPE_TRANSFORM;
	if(pObj->GetStyleTransformZoom(0) != 256) return EG_LAYER_TYPE_TRANSFORM;
	if(pObj->GetStyleOPALayered(0) != EG_OPA_COVER) return EG_LAYER_TYPE_SIMPLE;
#if EG_DRAW_COMPLEX
	if(pObj->GetStyleBlendMode(0) != EG_BLEND_MODE_NORMAL) return EG_LAYER_TYPE_SIMPLE;
#endif
	return EG_LAYER_TYPE_NONE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::CreateTransition(EGPart_t Part, EGState_t PreviousState, EGState_t NewState, const EG_TransitionDiscriptor_t *pTransitionDiscriptor)
{
	m_SkipTransition = 1;
	m_State = PreviousState; 
	EG_StyleValue_t Value1 = GetProperty(Part, pTransitionDiscriptor->Property);
	m_State = NewState;
	EG_StyleValue_t Value2 = GetProperty(Part, pTransitionDiscriptor->Property);
	m_SkipTransition = 0;
	if((Value1.pPtr == Value2.pPtr) && (Value1.Number == Value2.Number) && Value1.Color.full == Value2.Color.full) return;
	m_State = PreviousState;
	Value1 = GetProperty(Part, pTransitionDiscriptor->Property);
	m_State = NewState;
	EGStyle *pStyle = GetTransitionStyle(Part);
	pStyle->SetProperty(pTransitionDiscriptor->Property, Value1); // Be sure `pStyle` has a valid value
	if(pTransitionDiscriptor->Property == EG_STYLE_RADIUS) {
		if(Value1.Number == EG_RADIUS_CIRCLE || Value2.Number == EG_RADIUS_CIRCLE) {
			EG_Coord_t HalfWidth = GetWidth() / 2;
			EG_Coord_t HalfHeight = GetHeight() / 2;
			if(Value1.Number == EG_RADIUS_CIRCLE) Value1.Number = EG_MIN(HalfWidth + 1, HalfHeight + 1);
			if(Value2.Number == EG_RADIUS_CIRCLE) Value2.Number = EG_MIN(HalfWidth + 1, HalfHeight + 1);
		}
	}
  Transition_t *pTransition = (Transition_t*)EG_AllocMem(sizeof(Transition_t));
  EG_ASSERT_MALLOC(pTransition);
	if(pTransition == nullptr) return;
  m_TransitionsList.AddHead(pTransition);
	pTransition->StartValue = Value1;
	pTransition->EndValue = Value2;
	pTransition->pObj = this;
	pTransition->Property = pTransitionDiscriptor->Property;
	pTransition->SelectFlags = Part;
	EGAnimate Animation;
	Animation.SetItem(pTransition);
	Animation.SetExcCB(TransitionAnimationCB);
	Animation.SetStartCB(TransitionAnimationStartCB);
	Animation.SetEndCB(TransitionAnimationEndCB);
	Animation.SetValues(0x00, 0xFF);
	Animation.SetTime(pTransitionDiscriptor->Time);
	Animation.SetDelay(pTransitionDiscriptor->Delay);
	Animation.SetPathCB(pTransitionDiscriptor->PathCB);
	Animation.SetEarlyApply(false);
	Animation.m_pParam = pTransitionDiscriptor->m_pParam;
  EGAnimate::Create(&Animation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool EGObject::TransitionDelete(EGPart_t Part, EGStyleProperty_e Property, Transition_t *pTransitionLimit)
{
Transition_t *pTransition;
bool Removed = false;

  POSITION Pos = m_TransitionsList.GetTailPosition();   // reverse iteration
	while(Pos != nullptr) {
    POSITION DelPos = Pos;
  	pTransition = (Transition_t*)m_TransitionsList.GetPrev(Pos);
		if(pTransition == pTransitionLimit) break;          // We're at the new entry so stop
		if((pTransition->pObj == this) && ((Part == pTransition->SelectFlags) || (Part == EG_PART_ANY)) && ((Property == pTransition->Property) || (Property == EG_STYLE_PROP_ANY))) {
			for(uint32_t i = 0; i < m_StyleCount; i++) {	// Remove any transitioned properties from the style to allow changing it as a normal styles
				if(m_pStyles[i].IsTransition && (Part == EG_PART_ANY || m_pStyles[i].SelectFlags == Part)) {
					m_pStyles[i].pStyle->RemoveProperty(pTransition->Property);
				}
			}
			EGAnimate::Delete(pTransition, nullptr);			// Free the animation object 
			m_TransitionsList.RemoveAt(DelPos);           // remove from the list
			EG_FreeMem(pTransition);
      Pos = m_TransitionsList.GetTailPosition();    // start again
			Removed = true;
		}
	}
	return Removed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::TransitionAnimationStartCB(EGAnimate *pAnimation)
{
	Transition_t *pTransition = (Transition_t*)pAnimation->m_pItem;
  EGObject *pObj = pTransition->pObj;
	EGPart_t Part = GetSelectorPart(pTransition->SelectFlags);
	pTransition->StartValue = pObj->GetProperty(Part, pTransition->Property);
	EGStyleProperty_e Property = pTransition->Property;
	pTransition->Property = EG_STYLE_PROP_INV;// make Property an invalid value so that `transition delete` won't remove it
	pObj->TransitionDelete(Part, Property, pTransition);	// Avoid duplicates, delete the related transitions 
	pTransition->Property = Property;
	EGStyle *pStyle = pObj->GetTransitionStyle(pTransition->SelectFlags);
	pStyle->SetProperty(pTransition->Property, pTransition->StartValue); // Be sure `pStyle` has a valid value
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::TransitionAnimationCB(EGAnimate *pAnimation, int32_t Value)
{
Transition_t *pTransition = (Transition_t*)pAnimation->m_pItem;
EGObject *pObj = pTransition->pObj;
EG_StyleValue_t FinalValue, OldValue;
bool Refresh = true;
	
	for(uint32_t i = 0; i < pObj->m_StyleCount; i++) {
		if((!pObj->m_pStyles[i].IsTransition) || (pObj->m_pStyles[i].SelectFlags != pTransition->SelectFlags)) continue;
		switch(pTransition->Property) {
			case EG_STYLE_BORDER_SIDE:
			case EG_STYLE_BORDER_POST:
			case EG_STYLE_BLEND_MODE:{
				if(Value < 255) FinalValue.Number = pTransition->StartValue.Number;
				else FinalValue.Number = pTransition->EndValue.Number;
				break;
      }
			case EG_STYLE_TRANSITION:
			case EG_STYLE_TEXT_FONT:{
				if(Value < 255) FinalValue.pPtr = pTransition->StartValue.pPtr;
				else FinalValue.pPtr = pTransition->EndValue.pPtr;
				break;
      }
			case EG_STYLE_COLOR_FILTER_DSC:{
				if(pTransition->StartValue.pPtr == nullptr) FinalValue.pPtr = pTransition->EndValue.pPtr;
				else if(pTransition->EndValue.pPtr == nullptr) FinalValue.pPtr = pTransition->StartValue.pPtr;
				else if(Value < 128) FinalValue.pPtr = pTransition->StartValue.pPtr;
				else FinalValue.pPtr = pTransition->EndValue.pPtr;
				break;
      }
			case EG_STYLE_BG_COLOR:
			case EG_STYLE_BG_GRAD_COLOR:
			case EG_STYLE_BORDER_COLOR:
			case EG_STYLE_TEXT_COLOR:
			case EG_STYLE_SHADOW_COLOR:
			case EG_STYLE_OUTLINE_COLOR:
			case EG_STYLE_IMG_RECOLOR:{
				if(Value <= 0) FinalValue.Color = pTransition->StartValue.Color;
				else if(Value >= 255)	FinalValue.Color = pTransition->EndValue.Color;
				else FinalValue.Color = EG_ColorMix(pTransition->EndValue.Color, pTransition->StartValue.Color, Value);
				break;
      }
			default:{
				if(Value == 0) FinalValue.Number = pTransition->StartValue.Number;
				else if(Value == 255)	FinalValue.Number = pTransition->EndValue.Number;
				else FinalValue.Number = pTransition->StartValue.Number + ((int32_t)((int32_t)(pTransition->EndValue.Number - pTransition->StartValue.Number) * Value) >> 8);
				break;
      }
		}
		if(pObj->m_pStyles[i].pStyle->GetProperty(pTransition->Property, &OldValue)) {
			if(FinalValue.pPtr == OldValue.pPtr && FinalValue.Color.full == OldValue.Color.full &&
				 FinalValue.Number == OldValue.Number) {
				Refresh = false;
			}
		}
		pObj->m_pStyles[i].pStyle->GetProperty(pTransition->Property, &FinalValue);
		if(Refresh) pObj->RefreshStyle(pTransition->SelectFlags, pTransition->Property);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::TransitionAnimationEndCB(EGAnimate *pAnimation)
{
Transition_t *pTransition = (Transition_t*)pAnimation->m_pItem;
EGObject *pObj = pTransition->pObj;
EGStyleProperty_e Property = pTransition->Property;
bool Running = false;

  POSITION Pos = m_TransitionsList.GetHeadPosition();   // Remove the transitioned property from transitons list...
  while(Pos != nullptr){	                              // if there is no more transitions for this property
    Transition_t *pTran = (Transition_t*)m_TransitionsList.GetNext(Pos);
		if((pTran != pTransition) && (pTran->pObj == pObj) && (pTran->SelectFlags == pTransition->SelectFlags) && (pTran->Property == Property)) {
			Running = true;
			break;
		}
	}
	if(!Running) {
		for(uint32_t i = 0; i < pObj->m_StyleCount; i++) {
			if(pObj->m_pStyles[i].IsTransition && (pObj->m_pStyles[i].SelectFlags == pTransition->SelectFlags)) {
        POSITION Pos = m_TransitionsList.Find(pTransition);
        if(Pos != nullptr) m_TransitionsList.RemoveAt(Pos);
				EG_FreeMem(pTransition);
				EGStyle *pStyle = pObj->m_pStyles[i].pStyle;
				pStyle->RemoveProperty(Property);
				if(pStyle->IsEmpty()) {
					pObj->RemoveStyle(pStyle, pObj->m_pStyles[i].SelectFlags);
				}
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::FadeAnimationCB(EGAnimate *pAnimation, int32_t Value)
{
	((EGObject*)pAnimation->m_pItem)->SetStyleOPA(Value, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EGObject::FadeInAnimationEnd(EGAnimate *pAnimation)
{
  ((EGObject*)pAnimation->m_pItem)->RemoveStyleProperty(EG_STYLE_OPA, 0);
}

