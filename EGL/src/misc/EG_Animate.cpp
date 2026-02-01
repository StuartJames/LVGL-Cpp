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

#include "misc/EG_Animate.h"

#include "hal/EG_HALTick.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Timer.h"
#include "misc/EG_Math.h"
#include "misc/EG_Memory.h"
#include "misc/lv_gc.h"

#define EG_ANIM_RESOLUTION 1024
#define EG_ANIM_RES_SHIFT 10

bool        EGAnimate::m_ListChanged = false;
bool        EGAnimate::m_AnimateRunRound = false;
uint32_t    EGAnimate::m_LastTimerRun = 0;
EGTimer    *EGAnimate::m_pAnimateTimer = nullptr;
EGList      EGAnimate::m_AnimateList;

#if EG_LOG_TRACE_ANIM
#define TRACE_ANIM(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define TRACE_ANIM(...)
#endif

/////////////////////////////////////////////////////////////////////////////////////////

EGAnimate::EGAnimate(void) : 
  m_pItem(nullptr),
  m_AnimateCB(nullptr),
  m_StartCB(nullptr),
  m_EndCB(nullptr),
  m_DeletedCB(nullptr),
  m_GetValueCB(nullptr),
	m_PathCB(PathLinear),
	m_StartValue(0),
  m_CurrentValue(0),
	m_EndValue(100),
  m_ActiveTime(0),      
	m_Time(500),
  m_PlaybackDelay(0),    
  m_PlaybackTime(0),   
  m_RepeatDelay(0),    
  m_RepeatCount(0),      
  m_EarlyApply(0),  
  m_PlaybackNow(0),  
  m_RunRound(0),     
  m_StartCalled(0)  
{
}

/////////////////////////////////////////////////////////////////////////////////////////

EGAnimate::~EGAnimate(void)
{
}

//////////////////////////////////////////////////////////////////////////////////

void EGAnimate::operator=(const EGAnimate &rval)
{
  m_pItem = rval.m_pItem;
  m_AnimateCB = rval.m_AnimateCB;
  m_StartCB = rval.m_StartCB;
  m_EndCB = rval.m_EndCB;
  m_DeletedCB = rval.m_DeletedCB;
  m_GetValueCB = rval.m_GetValueCB;
	m_PathCB = rval.m_PathCB;
	m_StartValue = rval.m_StartValue;
  m_CurrentValue = rval.m_CurrentValue;
	m_EndValue = rval.m_EndValue;
  m_ActiveTime = rval.m_ActiveTime;    
	m_Time = rval.m_Time;
  m_PlaybackDelay = rval.m_PlaybackDelay;   
  m_PlaybackTime = rval.m_PlaybackTime;   
  m_RepeatDelay = rval.m_RepeatDelay;    
  m_RepeatCount = rval.m_RepeatCount;      
  m_EarlyApply = rval.m_EarlyApply;  
  m_PlaybackNow = rval.m_PlaybackNow; 
  m_RunRound = rval.m_RunRound;    
  m_StartCalled = rval.m_StartCalled;  
}

//////////////////////////////////////////////////////////////////////////////////

void EGAnimate::Copy(EGAnimate *pAnimate)
{
  pAnimate->m_pItem = m_pItem;
  pAnimate->m_AnimateCB = m_AnimateCB;
  pAnimate->m_StartCB = m_StartCB;
  pAnimate->m_EndCB = m_EndCB;
  pAnimate->m_DeletedCB = m_DeletedCB;
  pAnimate->m_GetValueCB = m_GetValueCB;
	pAnimate->m_PathCB = m_PathCB;
	pAnimate->m_StartValue = m_StartValue;
  pAnimate->m_CurrentValue = m_CurrentValue;
	pAnimate->m_EndValue = m_EndValue;
  pAnimate->m_ActiveTime = m_ActiveTime;    
	pAnimate->m_Time = m_Time;
  pAnimate->m_PlaybackDelay = m_PlaybackDelay;   
  pAnimate->m_PlaybackTime = m_PlaybackTime;   
  pAnimate->m_RepeatDelay = m_RepeatDelay;    
  pAnimate->m_RepeatCount = m_RepeatCount;      
  pAnimate->m_EarlyApply = m_EarlyApply;  
  pAnimate->m_PlaybackNow = m_PlaybackNow; 
  pAnimate->m_RunRound = m_RunRound;    
  pAnimate->m_StartCalled = m_StartCalled;  
}

/////////////////////////////////////////////////////////////////////////////////////////

EGAnimate* EGAnimate::Create(EGAnimate *pAnimate)
{
	if(pAnimate->m_AnimateCB != nullptr) Delete(pAnimate->m_pItem, pAnimate->m_AnimateCB); // avoid duplicates. m_AnimateCB == NULL would delete all animations of m_pItem
	EGAnimate *pNewAnimate = new EGAnimate;
	if(pNewAnimate == nullptr) return nullptr;
  EG_LOG_WARN("Animate Count:%d", m_AnimateList.GetSize());
	if(m_AnimateList.IsEmpty()) m_LastTimerRun = EG_GetTick();  // If the list is empty the anim timer was suspended
  pAnimate->Copy(pNewAnimate);           // Copy the contents
	if(pAnimate->m_pItem == pAnimate) pNewAnimate->m_pItem = pNewAnimate;
  m_AnimateList.AddHead(pNewAnimate);	// Add the new animation to the animation linked list
	pNewAnimate->m_RunRound = m_AnimateRunRound;
	if(pNewAnimate->m_EarlyApply){	// Set the start value
		if(pNewAnimate->m_GetValueCB) {
			int32_t OffsetValue = pNewAnimate->m_GetValueCB(pNewAnimate);
			pNewAnimate->m_StartValue += OffsetValue;
			pNewAnimate->m_EndValue += OffsetValue;
		}
		if(pNewAnimate->m_AnimateCB && pNewAnimate->m_pItem) pNewAnimate->m_AnimateCB(pNewAnimate->m_pItem, pNewAnimate->m_StartValue);
	}
	MarkListChanged();	// It's important if it happens in a ready callback. (see `anim_timer`)
  return pNewAnimate;
}

/////////////////////////////////////////////////////////////////////////////////////////

EGAnimate* EGAnimate::Get(void *pVariable, EG_AnimateExecCB_t AnimateCB)
{
EGAnimate *pAnimate;

  POSITION Pos = m_AnimateList.GetHeadPosition();
  while(Pos != nullptr){
	  pAnimate = (EGAnimate*)m_AnimateList.GetNext(Pos);
		if(pAnimate->m_pItem == pVariable && (pAnimate->m_AnimateCB == AnimateCB || AnimateCB == nullptr)) {
			return pAnimate;
		}
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

EGTimer* EGAnimate::GetTimer(void)
{
	return m_pAnimateTimer;
}

/////////////////////////////////////////////////////////////////////////////////////////

uint16_t EGAnimate::RunningCount(void)
{
	return m_AnimateList.GetCount();
}

/////////////////////////////////////////////////////////////////////////////////////////

uint32_t EGAnimate::SpeedToTime(uint32_t Speed, int32_t Start, int32_t End)
{
uint32_t Duration = EG_ABS(Start - End);
uint32_t Time = (Duration * 1000) / Speed;

	if(Time == 0) Time++;
	return Time;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGAnimate::ReferenceNow(void)
{
	AnimateTimerCB(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathLinear(EGAnimate *pAnimate)
{
	// Calculate the current step
	int32_t Step = EG_Map(pAnimate->m_ActiveTime, 0, pAnimate->m_Time, 0, EG_ANIM_RESOLUTION);

  // Get the new value which will be proportional to `step` *and the `start` and `end` values
	int32_t Value = Step * (pAnimate->m_EndValue - pAnimate->m_StartValue);
	Value = Value >> EG_ANIM_RES_SHIFT;
	Value += pAnimate->m_StartValue;
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathEaseIn(EGAnimate *pAnimate)
{
	uint32_t Time = EG_Map(pAnimate->m_ActiveTime, 0, pAnimate->m_Time, 0, EG_BEZIER_VAL_MAX);
	int32_t Step = EG_Bezier3(Time, 0, 50, 100, EG_BEZIER_VAL_MAX);
	int32_t Value = Step * (pAnimate->m_EndValue - pAnimate->m_StartValue);
	Value = Value >> EG_BEZIER_VAL_SHIFT;
	Value += pAnimate->m_StartValue;
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathEaseOut(EGAnimate *pAnimate)
{
	uint32_t Time = EG_Map(pAnimate->m_ActiveTime, 0, pAnimate->m_Time, 0, EG_BEZIER_VAL_MAX);
	int32_t Step = EG_Bezier3(Time, 0, 900, 950, EG_BEZIER_VAL_MAX);
	int32_t Value = Step * (pAnimate->m_EndValue - pAnimate->m_StartValue);
	Value = Value >> EG_BEZIER_VAL_SHIFT;
	Value += pAnimate->m_StartValue;
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathEaseInOut(EGAnimate *pAnimate)
{
	uint32_t Time = EG_Map(pAnimate->m_ActiveTime, 0, pAnimate->m_Time, 0, EG_BEZIER_VAL_MAX);
	int32_t Step = EG_Bezier3(Time, 0, 50, 952, EG_BEZIER_VAL_MAX);
	int32_t Value = Step * (pAnimate->m_EndValue - pAnimate->m_StartValue);
	Value = Value >> EG_BEZIER_VAL_SHIFT;
	Value += pAnimate->m_StartValue;
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathOvershoot(EGAnimate *pAnimate)
{
	uint32_t Time = EG_Map(pAnimate->m_ActiveTime, 0, pAnimate->m_Time, 0, EG_BEZIER_VAL_MAX);
	int32_t Step = EG_Bezier3(Time, 0, 1000, 1300, EG_BEZIER_VAL_MAX);
	int32_t Value = Step * (pAnimate->m_EndValue - pAnimate->m_StartValue);
	Value = Value >> EG_BEZIER_VAL_SHIFT;
	Value += pAnimate->m_StartValue;
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathBounce(EGAnimate *pAnimate)
{
	int32_t Time = EG_Map(pAnimate->m_ActiveTime, 0, pAnimate->m_Time, 0, EG_BEZIER_VAL_MAX);
	int32_t Difference = (pAnimate->m_EndValue - pAnimate->m_StartValue);

	// 3 bounces has 5 parts: 3 down and 2 up. One part is Time / 5 long

	if(Time < 408) Time = (Time * 2500) >> EG_BEZIER_VAL_SHIFT; // Go down [0..1024] range
	else if(Time >= 408 && Time < 614) {
		Time -= 408;		// First bounce back
		Time = Time * 5; // to [0..1024] range
		Time = EG_BEZIER_VAL_MAX - Time;
		Difference = Difference / 20;
	}
	else if(Time >= 614 && Time < 819) {
		Time -= 614;		// Fall back
		Time = Time * 5; // to [0..1024] range
		Difference = Difference / 20;
	}
	else if(Time >= 819 && Time < 921) {
		Time -= 819;		// Second bounce back
		Time = Time * 10; // to [0..1024] range
		Time = EG_BEZIER_VAL_MAX - Time;
		Difference = Difference / 40;
	}
	else if(Time >= 921 && Time <= EG_BEZIER_VAL_MAX) {
		Time -= 921;		// Fall back
		Time = Time * 10; // to [0..1024] range
		Difference = Difference / 40;
	}
	if(Time > EG_BEZIER_VAL_MAX) Time = EG_BEZIER_VAL_MAX;
	if(Time < 0) Time = 0;
	int32_t Step = EG_Bezier3(Time, EG_BEZIER_VAL_MAX, 800, 500, 0);
	int32_t Value = Step * Difference;
	Value = Value >> EG_BEZIER_VAL_SHIFT;
	Value = pAnimate->m_EndValue - Value;
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t EGAnimate::PathStep(EGAnimate *pAnimate)
{
	if(pAnimate->m_ActiveTime >= pAnimate->m_Time) return pAnimate->m_EndValue;
	else return pAnimate->m_StartValue;
}

/////////////////////////////////////////////////////////////////////////////////////////

uint32_t EGAnimate::GetPlaytime(EGAnimate *pAnimate)
{
uint32_t playtime = EG_ANIM_PLAYTIME_INFINITE;

	if(pAnimate->m_RepeatCount == EG_ANIM_REPEAT_INFINITE) return playtime;
	playtime = pAnimate->m_Time - pAnimate->m_ActiveTime;
	if(pAnimate->m_PlaybackNow == 0) playtime += pAnimate->m_PlaybackDelay + pAnimate->m_PlaybackTime;
	if(pAnimate->m_RepeatCount <= 1) return playtime;
	playtime += (pAnimate->m_RepeatDelay + pAnimate->m_Time + pAnimate->m_PlaybackDelay + pAnimate->m_PlaybackTime) * (pAnimate->m_RepeatCount - 1);
	return playtime;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGAnimate::InitialiseCore(void)
{
//	m_AnimateList.RemoveAll();
	m_pAnimateTimer = EGTimer::Create(AnimateTimerCB, EG_DISP_DEF_REFR_PERIOD, nullptr);
	MarkListChanged();                  //Turn off the animation timer
	m_ListChanged = false;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EGAnimate::Delete(void *pVariable, EG_AnimateExecCB_t m_AnimateCB)
{
EGAnimate *pAnimate;
bool Deleted = false;
POSITION Pos;

  Pos = m_AnimateList.GetHeadPosition();
	while(Pos != nullptr){
	  pAnimate = (EGAnimate*)m_AnimateList.GetAt(Pos);
		if((pAnimate->m_pItem == pVariable || pVariable == nullptr) && (pAnimate->m_AnimateCB == m_AnimateCB || m_AnimateCB == nullptr)) {
			m_AnimateList.RemoveAt(Pos);    // Pos will be updated
			if(pAnimate->m_DeletedCB != nullptr) pAnimate->m_DeletedCB(pAnimate);
			delete pAnimate;
			MarkListChanged(); // Read by `AnimateTimerCB`. It need to know if a delete occurred in the linked list
			Deleted = true;
		}
    else m_AnimateList.GetNextPosition(Pos);
	}
	return Deleted;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGAnimate::DeleteAll(void)
{
  while(m_AnimateList.GetCount()) delete (EGAnimate*)m_AnimateList.RemoveHead(); // remove and delete head node
	MarkListChanged();
}

/////////////////////////////////////////////////////////////////////////////////////////
void EGAnimate::AnimateTimerCB(EGTimer *pTimer)
{
POSITION Pos;

	EG_UNUSED(pTimer);
	uint32_t Elapsed = EG_TickElapse(m_LastTimerRun);
	m_AnimateRunRound = m_AnimateRunRound ? false : true;	      // Toggle the run round
  Pos = m_AnimateList.GetHeadPosition();
	while(Pos != nullptr) {
    m_ListChanged = false;	// This can be set by `Delete()` typically in `ReadyHandler` which could change the list
  	EGAnimate *pAnimate = (EGAnimate*)m_AnimateList.GetNext(Pos); // Pos will point to next Animate
    if(pAnimate->m_RunRound != m_AnimateRunRound) {
      pAnimate->m_RunRound = m_AnimateRunRound; // The list readying might be reset so need to know which anim has run already
      int32_t NewTime = pAnimate->m_ActiveTime + Elapsed;    // The animation will run now for the first time. Call `m_StartCB`
      if(!pAnimate->m_StartCalled && (pAnimate->m_ActiveTime <= 0) && (NewTime >= 0)) {
        if((pAnimate->m_EarlyApply == 0) && (pAnimate->m_GetValueCB != nullptr)) {
          int32_t OffsetValue = pAnimate->m_GetValueCB(pAnimate);
          pAnimate->m_StartValue += OffsetValue;
          pAnimate->m_EndValue += OffsetValue;
        }
        if(pAnimate->m_StartCB) pAnimate->m_StartCB(pAnimate);
        pAnimate->m_StartCalled = 1;
      }
      pAnimate->m_ActiveTime += Elapsed;
      if(pAnimate->m_ActiveTime >= 0) {
        if(pAnimate->m_ActiveTime > pAnimate->m_Time) pAnimate->m_ActiveTime = pAnimate->m_Time;
        int32_t Value = pAnimate->m_PathCB(pAnimate);
        if(Value != pAnimate->m_CurrentValue) {
          pAnimate->m_CurrentValue = Value;
          if(pAnimate->m_AnimateCB) pAnimate->m_AnimateCB(pAnimate->m_pItem, Value);	// Apply the calculated value
        }
        if(pAnimate->m_ActiveTime >= pAnimate->m_Time) {// If the time has elapsed the animation has ended
					EndHandler(pAnimate);
				}
      }
    }
    if(m_ListChanged)	Pos = m_AnimateList.GetHeadPosition();	// If the linked list changed start again from the head
  }
	m_LastTimerRun = EG_GetTick();
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGAnimate::EndHandler(EGAnimate *pAnimate)
{
	// In the end of a forward anim decrement repeat cnt.
	if((pAnimate->m_PlaybackNow == 0) && (pAnimate->m_RepeatCount > 0) && (pAnimate->m_RepeatCount != EG_ANIM_REPEAT_INFINITE)) {
		pAnimate->m_RepeatCount--;
	}
	// Delete the animation if: no repeat left and no play back or no repeat, play back is enabled and play back is ready
	if(pAnimate->m_RepeatCount == 0 && (pAnimate->m_PlaybackTime == 0 || pAnimate->m_PlaybackTime == 1)) {
    POSITION Pos = m_AnimateList.Find(pAnimate);
	  if(Pos != nullptr) m_AnimateList.RemoveAt(Pos);                     // Delete the animation from the list.
		MarkListChanged();		                                              // Flag that the list has changed
		if(pAnimate->m_EndCB != nullptr) pAnimate->m_EndCB(pAnimate);		    // Call the function for the final time
		if(pAnimate->m_DeletedCB != nullptr) pAnimate->m_DeletedCB(pAnimate);
		delete pAnimate;
	}
	else {
		pAnimate->m_ActiveTime = -(int32_t)(pAnimate->m_RepeatDelay); // Restart the animation
		if(pAnimate->m_PlaybackTime != 0) {		// Swap the start and end values in play back mode
			// If now turning back use the 'playback_pause
			if(pAnimate->m_PlaybackNow == 0) pAnimate->m_ActiveTime = -(int32_t)(pAnimate->m_PlaybackDelay);
			pAnimate->m_PlaybackNow = pAnimate->m_PlaybackNow == 0 ? 1 : 0;			// Toggle the play back state
			int32_t Temp = pAnimate->m_StartValue;			// Swap the start and end values
			pAnimate->m_StartValue = pAnimate->m_EndValue;
			pAnimate->m_EndValue = Temp;
			Temp = pAnimate->m_Time;			              // Swap the time and playback_time
			pAnimate->m_Time = pAnimate->m_PlaybackTime;
			pAnimate->m_PlaybackTime = Temp;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGAnimate::MarkListChanged(void)
{
	m_ListChanged = true;
	if(m_AnimateList.GetCount() > 0)	m_pAnimateTimer->Resume();
  else m_pAnimateTimer->Pause();
}


