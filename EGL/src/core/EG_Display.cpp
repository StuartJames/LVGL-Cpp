/*
 *                LEGL 2025-2026 HydraSystems.
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
#include "misc/EG_Math.h"
#include "core/EG_Refresh.h"

/////////////////////////////////////////////////////////////////////////////

static inline EGObject* EGActiveScreen(void);
static inline EGObject* EGTopLayer(void);
static inline EGObject* EGSystemLayer(void);
static inline void EGLoadScreen(EGObject *pScreen);

EGList     EGDisplay::m_DisplayList;      
EGDisplay *EGDisplay::m_pDefaultDisplay = nullptr;

/////////////////////////////////////////////////////////////////////////////

EGDisplay::EGDisplay(void)
{
  m_SyncAreas.Initialise();
  m_pDriver = nullptr;
  m_pRefreshTimer = nullptr;
  m_pTheme = nullptr;
  m_pScreens = nullptr;
  m_pActiveScreen = nullptr;
  m_pPrevoiusScreen = nullptr;
  m_ScreenToLoad = nullptr;
  m_pTopLayer = nullptr;
  m_pSystemLayer = nullptr;
  m_ScreenCount = 0;
  m_DrawOverActive = 0;
  m_DeletePrevious = 0;
  m_RenderingInProgress = 0;
  m_BackgroundOPA = EG_OPA_COVER;
  m_BackgroundColor = EG_ColorWhite();
  m_BackgroundImage = nullptr;
  m_InvalidCount = 0;
  m_InvalidEnableCount = 0;
  m_LastActivityTime = 0;
};

/////////////////////////////////////////////////////////////////////////////

EGDisplay::~EGDisplay(void)
{
}

/////////////////////////////////////////////////////////////////////////////

EGDisplay* EGDisplay::GetDisplay(const EGObject *pScreen)
{
EGDisplay *pDisplay;

  POSITION Pos = m_DisplayList.GetHeadPosition();
	while(Pos != nullptr){
    pDisplay = (EGDisplay*)m_DisplayList.GetNext(Pos);
//    EG_LOG_WARN("display %p", (void*)pDisplay);
		for(uint32_t i = 0; i < pDisplay->m_ScreenCount; i++) {
//    	EG_LOG_WARN("Screen %d, %p", i, (void*)pDisplay->m_pScreens[i]);
			if(pDisplay->m_pScreens[i] == pScreen) return pDisplay;
		}
	}
	EG_LOG_WARN("No screen found");
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////

EGObject *EGDisplay::GetActiveScreen(EGDisplay *pDisplay)
{
  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("no display registered to get its active screen");
    return nullptr;
  }
	return pDisplay->m_pActiveScreen;
}

/////////////////////////////////////////////////////////////////////////////

EGObject *EGDisplay::GetPrevoiusScreen(EGDisplay *pDisplay)
{
  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("no display registered to get its previous screen");
    return nullptr;
  }
	return pDisplay->m_pPrevoiusScreen;
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::LoadScreen(EGObject *pScreen)
{
	LoadAnimation(pScreen, EG_SCR_LOAD_ANIM_NONE, 0, 0, false);
}

/////////////////////////////////////////////////////////////////////////////

EGObject *EGDisplay::GetTopLayer(EGDisplay *pDisplay)
{
  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("lv_layer_top: no display registered to get its top layer");
    return nullptr;
  }
	return pDisplay->m_pTopLayer;
}

/////////////////////////////////////////////////////////////////////////////

EGObject *EGDisplay::GetSystemLayer(EGDisplay *pDisplay)
{
  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("lv_layer_sys: no display registered to get its sys. layer");
    return nullptr;
  }
	return pDisplay->m_pSystemLayer;
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetTheme(EGDisplay *pDisplay, EGTheme *pTheme)
{
  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("no display registered");
    return;
  }
	pDisplay->m_pTheme = pTheme;
	if(pDisplay->m_ScreenCount == 3 &&
		pDisplay->m_pScreens[0]->GetChildCount() == 0 &&
		pDisplay->m_pScreens[1]->GetChildCount() == 0 &&
		pDisplay->m_pScreens[2]->GetChildCount() == 0) {
		EGTheme::ThemeApply((EGObject*)pDisplay->m_pScreens[0]);
	}
}
/////////////////////////////////////////////////////////////////////////////

EGTheme* EGDisplay::GetTheme(EGDisplay *pDisplay)
{
  if(!pDisplay) pDisplay = GetDefault();
	return pDisplay->m_pTheme;
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetBackgroundColor(EGDisplay *pDisplay, EG_Color_t Color)
{
EGRect Rect;

  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("no display registered");
    return;
  }
	pDisplay->m_BackgroundColor = Color;
	Rect.Set(0, 0, pDisplay->GetHorizontalRes() - 1, pDisplay->GetVerticalRes() - 1);
	InvalidateRect(pDisplay, &Rect);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetBackgroundImage(EGDisplay *pDisplay, const void *pImage)
{
EGRect Rect;

  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("no display registered");
    return;
  }
	pDisplay->m_BackgroundImage = pImage;
	Rect.Set(0, 0, pDisplay->GetHorizontalRes() - 1, pDisplay->GetVerticalRes() - 1);
	InvalidateRect(pDisplay, &Rect);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetBackgroundOpa(EGDisplay *pDisplay, EG_OPA_t opa)
{
EGRect Rect;

  if(!pDisplay) pDisplay = GetDefault();
  if(!pDisplay) {
    EG_LOG_WARN("no display registered");
    return;
  }
	pDisplay->m_BackgroundOPA = opa;
	Rect.Set(0, 0, pDisplay->GetHorizontalRes() - 1, pDisplay->GetVerticalRes() - 1);
	InvalidateRect(pDisplay, &Rect);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::LoadAnimation(EGObject *pScreen, EG_ScreenAnimateType_t AnimateType, uint32_t Time, uint32_t Delay, bool AutoDelete)
{
	EGDisplay *pDisplay = pScreen->GetDisplay();
	EGObject *pActiveScreen = EGDisplay::GetActiveScreen(pDisplay);
	if(pActiveScreen == pScreen || pDisplay->m_ScreenToLoad == pScreen) {
		return;
	}
	if(pDisplay->m_ScreenToLoad){	// If another screen load animation is in progress make target screen loaded immediately.
		EGAnimate::Delete(pDisplay->m_ScreenToLoad, nullptr);
		pDisplay->m_ScreenToLoad->SetPosition(0, 0);
		pDisplay->m_ScreenToLoad->RemoveStyleProperty(EG_STYLE_OPA, 0);
		if(pDisplay->m_DeletePrevious) EGObject::Delete(pActiveScreen);
		pActiveScreen = pDisplay->m_ScreenToLoad;
		LoadScreenPrivate(pDisplay->m_ScreenToLoad);
	}
	pDisplay->m_ScreenToLoad = pScreen;
	if(pDisplay->m_pPrevoiusScreen && pDisplay->m_DeletePrevious) {
		EGObject::Delete(pDisplay->m_pPrevoiusScreen);
		pDisplay->m_pPrevoiusScreen = nullptr;
	}
	pDisplay->m_DrawOverActive = IsOutAnimation(AnimateType);
	pDisplay->m_DeletePrevious = AutoDelete;
	EGAnimate::Delete(pScreen, nullptr);	// Be sure there is no other animation on the screens
	EGAnimate::Delete(EGActiveScreen(), nullptr);
	pScreen->SetPosition(0, 0);	// Be sure both screens are in a normal position
	EGActiveScreen()->SetPosition(0, 0);
	pScreen->RemoveStyleProperty(EG_STYLE_OPA, 0);
	EGActiveScreen()->RemoveStyleProperty(EG_STYLE_OPA, 0);
	if(Time == 0 && Delay == 0) {	// Shortcut for immediate load
		LoadScreenPrivate(pScreen);
		if(AutoDelete) EGObject::Delete(pActiveScreen);
		return;
	}
	EGAnimate NewAnimate;
	NewAnimate.SetItem(pScreen);
	NewAnimate.SetStartCB(LoadAnimationStart);
	NewAnimate.SetEndCB(AnimationEnd);
	NewAnimate.SetTime(Time);
	NewAnimate.SetDelay(Delay);
	EGAnimate OldAnimate;
	OldAnimate.SetItem(pDisplay->m_pActiveScreen);
	OldAnimate.SetTime(Time);
	OldAnimate.SetDelay(Delay);
	switch(AnimateType) {
		case EG_SCR_LOAD_ANIM_NONE:
			/*Create a dummy animation to apply the Delay*/
			NewAnimate.SetExcCB(SetAnimationX);
			NewAnimate.SetValues(0, 0);
			break;
		case EG_SCR_LOAD_ANIM_OVER_LEFT:
			NewAnimate.SetExcCB(SetAnimationX);
			NewAnimate.SetValues(pDisplay->GetHorizontalRes(), 0);
			break;
		case EG_SCR_LOAD_ANIM_OVER_RIGHT:
			NewAnimate.SetExcCB(SetAnimationX);
			NewAnimate.SetValues(-pDisplay->GetHorizontalRes(), 0);
			break;
		case EG_SCR_LOAD_ANIM_OVER_TOP:
			NewAnimate.SetExcCB(SetAnimationY);
			NewAnimate.SetValues(pDisplay->GetVerticalRes(), 0);
			break;
		case EG_SCR_LOAD_ANIM_OVER_BOTTOM:
			NewAnimate.SetExcCB(SetAnimationY);
			NewAnimate.SetValues(-pDisplay->GetVerticalRes(), 0);
			break;
		case EG_SCR_LOAD_ANIM_MOVE_LEFT:
			NewAnimate.SetExcCB(SetAnimationX);
			NewAnimate.SetValues(pDisplay->GetHorizontalRes(), 0);

			OldAnimate.SetExcCB(SetAnimationX);
			OldAnimate.SetValues(0, -pDisplay->GetHorizontalRes());
			break;
		case EG_SCR_LOAD_ANIM_MOVE_RIGHT:
			NewAnimate.SetExcCB(SetAnimationX);
			NewAnimate.SetValues(-pDisplay->GetHorizontalRes(), 0);

			OldAnimate.SetExcCB(SetAnimationX);
			OldAnimate.SetValues(0, pDisplay->GetHorizontalRes());
			break;
		case EG_SCR_LOAD_ANIM_MOVE_TOP:
			NewAnimate.SetExcCB(SetAnimationY);
			NewAnimate.SetValues(pDisplay->GetVerticalRes(), 0);

			OldAnimate.SetExcCB(SetAnimationY);
			OldAnimate.SetValues(0, -pDisplay->GetVerticalRes());
			break;
		case EG_SCR_LOAD_ANIM_MOVE_BOTTOM:
			NewAnimate.SetExcCB(SetAnimationY);
			NewAnimate.SetValues(-pDisplay->GetVerticalRes(), 0);

			OldAnimate.SetExcCB(SetAnimationY);
			OldAnimate.SetValues(0, pDisplay->GetVerticalRes());
			break;
		case EG_SCR_LOAD_ANIM_FADE_IN:
			NewAnimate.SetExcCB(OpaScaleAnimation);
			NewAnimate.SetValues(EG_OPA_TRANSP, EG_OPA_COVER);
			break;
		case EG_SCR_LOAD_ANIM_FADE_OUT:
			OldAnimate.SetExcCB(OpaScaleAnimation);
			OldAnimate.SetValues(EG_OPA_COVER, EG_OPA_TRANSP);
			break;
		case EG_SCR_LOAD_ANIM_OUT_LEFT:
			OldAnimate.SetExcCB(SetAnimationX);
			OldAnimate.SetValues(0, -pDisplay->GetHorizontalRes());
			break;
		case EG_SCR_LOAD_ANIM_OUT_RIGHT:
			OldAnimate.SetExcCB(SetAnimationX);
			OldAnimate.SetValues(0, pDisplay->GetHorizontalRes());
			break;
		case EG_SCR_LOAD_ANIM_OUT_TOP:
			OldAnimate.SetExcCB(SetAnimationY);
			OldAnimate.SetValues(0, -pDisplay->GetVerticalRes());
			break;
		case EG_SCR_LOAD_ANIM_OUT_BOTTOM:
			OldAnimate.SetExcCB(SetAnimationY);
			OldAnimate.SetValues(0, pDisplay->GetVerticalRes());
			break;
	}
	EGEvent::EventSend(pActiveScreen, EG_EVENT_SCREEN_UNLOAD_START, nullptr);
	EGAnimate::Create(&NewAnimate);
	EGAnimate::Create(&OldAnimate);
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGDisplay::GetInactiveTime(EGDisplay *pDisplay)
{
EGDisplay *pDisp;

  if(pDisplay) return EG_TickElapse(pDisplay->m_LastActivityTime);
	uint32_t Time = UINT32_MAX;
	pDisp = GetNext(nullptr);
	while(pDisp){
		uint32_t elaps = EG_TickElapse(pDisp->m_LastActivityTime);
		Time = EG_MIN(Time, elaps);
		pDisp = GetNext(pDisp);
	}
	return Time;
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::TriggerActivity(EGDisplay *pDisplay)
{
	pDisplay->m_LastActivityTime = EG_GetTick();
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::CleanDisplayCache(EGDisplay *pDisplay)
{
	if(pDisplay->m_pDriver->CleanDcacheCB != nullptr) pDisplay->m_pDriver->CleanDcacheCB(pDisplay->m_pDriver);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::EnableInvalidation(EGDisplay *pDisplay, bool Enable)
{
	pDisplay->m_InvalidEnableCount += Enable ? 1 : -1;
}

/////////////////////////////////////////////////////////////////////////////

EGTimer* EGDisplay::GetRefereshTimer(EGDisplay *pDisplay)
{
	return pDisplay->m_pRefreshTimer;
}

/////////////////////////////////////////////////////////////////////////////
// Static
/////////////////////////////////////////////////////////////////////////////

bool EGDisplay::IsInvalidationEnabled(EGDisplay *pDisplay)
{
	if(!pDisplay) pDisplay = GetDefault();
	if(!pDisplay) {
		EG_LOG_WARN("no display registered");
		return false;
	}
	return (pDisplay->m_InvalidEnableCount > 0);
}

/////////////////////////////////////////////////////////////////////////////
// Private
/////////////////////////////////////////////////////////////////////////////

void EGDisplay::LoadScreenPrivate(EGObject *pScreen)
{
	EGDisplay *pDisplay = pScreen->GetDisplay();
	if(!pDisplay) return;                           // Shouldn't happen, just to be sure
//	ESP_LOGI("[DISPLY]", "Screen loading: %p", (void*)pScreen);
	EGObject *pOldScreen = pDisplay->m_pActiveScreen;
	if(pDisplay->m_pActiveScreen) EGEvent::EventSend(pOldScreen, EG_EVENT_SCREEN_UNLOAD_START, nullptr);
	if(pDisplay->m_pActiveScreen) EGEvent::EventSend(pScreen, EG_EVENT_SCREEN_LOAD_START, nullptr);
	pDisplay->m_pActiveScreen = pScreen;
	pDisplay->m_ScreenToLoad = nullptr;
	if(pDisplay->m_pActiveScreen) EGEvent::EventSend(pScreen, EG_EVENT_SCREEN_LOADED, nullptr);
	if(pDisplay->m_pActiveScreen) EGEvent::EventSend(pOldScreen, EG_EVENT_SCREEN_UNLOADED, nullptr);
	pScreen->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::LoadAnimationStart(EGAnimate *pAnimate)
{
	EGDisplay *pDisplay = ((EGObject*)pAnimate->m_pItem)->GetDisplay();
	pDisplay->m_pPrevoiusScreen = EGActiveScreen();
	pDisplay->m_pActiveScreen = (EGObject*)pAnimate->m_pItem;
	EGEvent::EventSend(pDisplay->m_pActiveScreen, EG_EVENT_SCREEN_LOAD_START, nullptr);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::OpaScaleAnimation(void *pObj, int32_t Value)
{
	((EGObject*)pObj)->SetStyleOPA(Value, 0);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetAnimationX(void *pObj, int32_t Value)
{
	((EGObject*)pObj)->SetX(Value);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetAnimationY(void *pObj, int32_t Value)
{
	((EGObject*)pObj)->SetY(Value);
}

/////////////////////////////////////////////////////////////////////////////

void EGDisplay::AnimationEnd(EGAnimate *pAnimate)
{
	EGDisplay *pDisplay = ((EGObject*)pAnimate->m_pItem)->GetDisplay();
	EGEvent::EventSend(pDisplay->m_pActiveScreen, EG_EVENT_SCREEN_LOADED, nullptr);
	EGEvent::EventSend(pDisplay->m_pPrevoiusScreen, EG_EVENT_SCREEN_UNLOADED, nullptr);
	if(pDisplay->m_pPrevoiusScreen && pDisplay->m_DeletePrevious) EGObject::Delete(pDisplay->m_pPrevoiusScreen);
	pDisplay->m_pPrevoiusScreen = nullptr;
	pDisplay->m_DrawOverActive = false;
	pDisplay->m_ScreenToLoad = nullptr;
	((EGObject*)pAnimate->m_pItem)->RemoveStyleProperty(EG_STYLE_OPA, 0);
	pDisplay->m_pActiveScreen->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

bool EGDisplay::IsOutAnimation(EG_ScreenAnimateType_t AnimateType)
{
	return AnimateType == EG_SCR_LOAD_ANIM_FADE_OUT ||
		AnimateType == EG_SCR_LOAD_ANIM_OUT_LEFT ||
		AnimateType == EG_SCR_LOAD_ANIM_OUT_RIGHT ||
		AnimateType == EG_SCR_LOAD_ANIM_OUT_TOP ||
		AnimateType == EG_SCR_LOAD_ANIM_OUT_BOTTOM;
}

