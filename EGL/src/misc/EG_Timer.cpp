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

#include "misc/EG_Timer.h"
#include "hal/EG_HALTick.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Memory.h"
#include "misc/EG_List.h"
#include "misc/EG_Misc.h"

/////////////////////////////////////////////////////////////////////////////

#define IDLE_MEAS_PERIOD  500 // [ms]
#define DEF_PERIOD        500


#if EG_LOG_TRACE_TIMER
#define TIMER_TRACE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define TIMER_TRACE(...)
#endif

/////////////////////////////////////////////////////////////////////////////

EGList EGTimer::m_TimerList;
uint8_t EGTimer::m_IdleLast = 0;
bool EGTimer::m_TimerRun    = false;
bool EGTimer::m_Deleted     = false;
bool EGTimer::m_Created     = false;

/////////////////////////////////////////////////////////////////////////////

EGTimer::EGTimer(void) :
  m_Period(0),
  m_LastRun(0),
  m_TimerCB(nullptr),
  m_pParam(nullptr),
  m_RepeatCount(0),
  m_Paused(0),
  m_Run(0)
{
}

/////////////////////////////////////////////////////////////////////////////

void EGTimer::InitialiseCore(void)
{
 	m_TimerList.RemoveAll();
 	m_TimerRun = true;	// Initially enable the lv_timer handling
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EG_ATTRIBUTE_TIMER_HANDLER EGTimer::TimerHandler(void)
{
static bool IsRunning = false;  // Avoid concurrent running of the timer handler
static uint32_t IdlePeriodStart = 0;
static uint32_t BusyTime = 0;
static uint32_t RunCount = 0;
POSITION Pos;

//  TIMER_TRACE("begin");
	if(IsRunning) {
		TIMER_TRACE("Already running, concurrent calls are not allow, returning");
		return 1;
	}
	IsRunning = true;
	if(m_TimerRun == false) {
		IsRunning = false; // Release mutex
		return 1;
	}
	uint32_t HandlerStart = EG_GetTick();
	if(HandlerStart == 0) {
		RunCount++;
		if(RunCount > 100) {
			RunCount = 0;
			EG_LOG_WARN("It seems EG_IncrementTick() is not called.");
		}
	}
	EGTimer *pTimer = nullptr;  // Run all timers from the list
//	TIMER_TRACE("Processing Timer Exec's");
	do {
		m_Deleted = false;
		m_Created = false;
		Pos = m_TimerList.GetHeadPosition();
		while(Pos != nullptr) {
			pTimer = (EGTimer *)m_TimerList.GetNext(Pos); // The timer might be deleted if it runs only once ('repeat_count = 1') 	
      if(pTimer->m_Run){
        if(TimerExec(pTimer)){
          if(m_Created || m_Deleted) {		// If a timer was created or deleted then this or the next item might be corrupted
            TIMER_TRACE("Start from the first timer again because a timer was created or deleted");
            break;
          }
        }
      }
		}
	}
  while(Pos != nullptr);
	uint32_t NextLoopDelay = EG_NO_TIMER_READY;
//	TIMER_TRACE("Checking Timer Delays");
	Pos = m_TimerList.GetHeadPosition();
	while(Pos != nullptr) {
		pTimer = (EGTimer *)m_TimerList.GetNext(Pos);  
		if(!pTimer->m_Paused) {
			uint32_t TimerLoopDelay = pTimer->TimeRemaining();
			if(TimerLoopDelay < NextLoopDelay) NextLoopDelay = TimerLoopDelay;
		}
	}
	BusyTime += EG_TickElapse(HandlerStart);
	uint32_t IdlePeriod = EG_TickElapse(IdlePeriodStart);
	if(IdlePeriod >= IDLE_MEAS_PERIOD) {
		m_IdleLast = (BusyTime * 100) / IdlePeriod;   // Calculate the busy percentage
		m_IdleLast = m_IdleLast > 100 ? 0 : 100 - m_IdleLast;  // But we need idle time
		BusyTime = 0;
		IdlePeriodStart = EG_GetTick();
	}
	IsRunning = false;  // Release the mutex
//	TIMER_TRACE("finished (%d ms until the next timer call)", NextLoopDelay);
	return NextLoopDelay;
}

/////////////////////////////////////////////////////////////////////////////

EGTimer* EGTimer::CreateBasic(void)
{
	return Create(nullptr, DEF_PERIOD, nullptr);
}

/////////////////////////////////////////////////////////////////////////////

EGTimer* EGTimer::Create(EG_TimerCB_t TimerCB, uint32_t Period, void *pParam, bool AutoRun /*= true*/)
{
	EGTimer *pTimer = new EGTimer;
	if(pTimer == nullptr) return nullptr;
	pTimer->m_Period = Period;
	pTimer->m_pParam = pParam;
	pTimer->m_TimerCB = TimerCB;
	pTimer->m_RepeatCount = -1;
	pTimer->m_Paused = false;
  pTimer->m_Run = AutoRun;
	pTimer->m_LastRun = EG_GetTick();
	m_TimerList.AddHead(pTimer);
	m_Created = true;
	return pTimer;
}

/////////////////////////////////////////////////////////////////////////////

void EGTimer::SetExcCB(EG_TimerCB_t TimerCB)
{
	m_TimerCB = TimerCB;
}

/////////////////////////////////////////////////////////////////////////////

void EGTimer::Delete(EGTimer *pTimer)
{
	POSITION Pos = m_TimerList.Find(pTimer);
	TIMER_TRACE("Delete timer at %d", Pos);
  if(Pos == nullptr) return;
  pTimer->m_Run = false;
  m_TimerList.RemoveAt(Pos);
	m_Deleted = true;
	delete pTimer;
}

/////////////////////////////////////////////////////////////////////////////

EGTimer* EGTimer::GetNext(EGTimer *pTimer)
{ 
	if(pTimer == nullptr)	return (EGTimer*)m_TimerList.GetHead(nullptr);
	else return (EGTimer*)m_TimerList.GetPrev(pTimer);
}

/////////////////////////////////////////////////////////////////////////////

bool EGTimer::TimerExec(EGTimer *pTimer)
{
	if(pTimer->m_Paused) return false;
	bool Executed = false;
	if(pTimer->TimeRemaining() == 0) {
		int32_t OriginalRepeatCount = pTimer->m_RepeatCount;		// Decrement the repeat count before executing the timer_cb.
		if(pTimer->m_RepeatCount > 0) pTimer->m_RepeatCount--;  // When the count is zero the timer can be deleted in the next round
		pTimer->m_LastRun = EG_GetTick();
//		TIMER_TRACE("calling timer callback: %p", *((void **)&pTimer->m_TimerCB));
		if(pTimer->m_TimerCB && OriginalRepeatCount != 0) pTimer->m_TimerCB(pTimer);
//		TIMER_TRACE("timer callback %p finished", *((void **)&pTimer->m_TimerCB));
		EG_ASSERT_MEM_INTEGRITY();
		Executed = true;
	}
	if(m_Deleted == false) {     // The timer might be deleted by itself as well
		if(pTimer->m_RepeatCount == 0) { // The repeat count is over, delete the timer
			TIMER_TRACE("deleting timer with %p callback because the repeat count is over", *((void **)&pTimer->m_TimerCB));
			Delete(pTimer);
		}
	}
	return Executed;
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGTimer::TimeRemaining(void)
{
	uint32_t Elapse = EG_TickElapse(m_LastRun);	// Check if at least 'period' time elapsed
	if(Elapse >= m_Period) return 0;
	return m_Period - Elapse;
}
