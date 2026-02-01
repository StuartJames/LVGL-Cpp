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

#pragma once

#include "../EG_IntrnlConfig.h"
#include "../hal/EG_HALTick.h"

#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////

class EGList;
class EGTimer;

#ifndef EG_ATTRIBUTE_TIMER_HANDLER
#define EG_ATTRIBUTE_TIMER_HANDLER
#endif

#define EG_NO_TIMER_READY 0xFFFFFFFF

typedef void (*EG_TimerCB_t)(EGTimer *pTimer);

/////////////////////////////////////////////////////////////////////////////

class EGTimer
{
public:
                  EGTimer(void);
	void            Start(void);
	void            Pause(void);
	void            Resume(void);
	void            SetExcCB(EG_TimerCB_t TimerCB);
	void            SetPeriod(uint32_t Period);
	void            MakeReady(void);
	void            SetPepeatCount(int32_t repeat_count);
	void            Reset(void);

	static void     InitialiseCore(void);
	static EGTimer* CreateBasic(void);
	static EGTimer* Create(EG_TimerCB_t TimerCB, uint32_t Period, void *pParam, bool AutoRun = true);
	static void     Delete(EGTimer *pTimer);
  static EGTimer* GetNext(EGTimer *pTimer);
	static void     EnableAll(bool Flag);
	static uint8_t  GetIdle(void);


	static uint32_t TimerHandler(void);
	static uint32_t EG_ATTRIBUTE_TIMER_HANDLER RunInPeriod(uint32_t ms);


	uint32_t        m_Period;       // How often the timer should run
	uint32_t        m_LastRun;      // Last time the timer ran
	EG_TimerCB_t    m_TimerCB;      // Timer function
	void           *m_pParam;       // Call back function parameter
	int32_t         m_RepeatCount;  // 1: One time;  -1 : infinity;  n>0: residual times
	bool            m_Paused;
  bool            m_Run;

private:
	uint32_t        TimeRemaining(void);

	static bool     TimerExec(EGTimer *pTimer);

  static EGList   m_TimerList;
  static bool     m_TimerRun;
  static uint8_t  m_IdleLast;
  static bool     m_Deleted;
  static bool     m_Created;

};

/////////////////////////////////////////////////////////////////////////////

inline uint32_t EG_ATTRIBUTE_TIMER_HANDLER EGTimer::RunInPeriod(uint32_t ms)
{
	static uint32_t LastTick = 0;

	uint32_t CurrentTick = EG_GetTick();
	if((CurrentTick - LastTick) >= (ms)) {
		LastTick = CurrentTick;
		return TimerHandler();
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::Start(void)
{
	m_Run = true;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::Pause(void)
{
	m_Paused = true;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::Resume(void)
{
	m_Paused = false;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::SetPeriod(uint32_t Period)
{
	m_Period = Period;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::MakeReady(void)
{
	m_LastRun = EG_GetTick() - m_Period - 1;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::SetPepeatCount( int32_t RepeatCount)
{
	m_RepeatCount = RepeatCount;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::Reset(void)
{
	m_LastRun = EG_GetTick();
}

/////////////////////////////////////////////////////////////////////////////

inline uint8_t EGTimer::GetIdle(void)
{
	return m_IdleLast;
}

/////////////////////////////////////////////////////////////////////////////

inline void EGTimer::EnableAll(bool Flag)
{
	m_TimerRun = Flag;
}


