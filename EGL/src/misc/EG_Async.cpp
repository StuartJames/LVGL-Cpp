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

#include "misc/EG_Async.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Timer.h"

/////////////////////////////////////////////////////////////////////////////

EGAsyncFunc::EGAsyncFunc(EG_AsyncCB_t AsyncCB, void *pParam)
{
	m_AsyncCB = AsyncCB;
	m_pParam = pParam;
}

/////////////////////////////////////////////////////////////////////////////

void EGAsyncFunc::Create(EG_AsyncCB_t AsyncCB, void *pUserData)
{
	EGAsyncFunc *pAsyncObj = new EGAsyncFunc(AsyncCB, pUserData);
	EGTimer *pTimer = EGTimer::Create(TimerCB, 0, pAsyncObj);	// Create a new timer
	if(pTimer == nullptr){
    delete pAsyncObj;
    return;
  }
	pTimer->SetPepeatCount(1);
}

/////////////////////////////////////////////////////////////////////////////

EG_Result_t EGAsyncFunc::Cancel(EG_AsyncCB_t AsyncCB, void *pParam)
{
EG_Result_t Result = EG_RES_INVALID;

	EGTimer *pTimer = EGTimer::GetNext(nullptr);
	while(pTimer != nullptr) {		// Find the next timer node
		EGTimer *pTimerNext = EGTimer::GetNext(pTimer);
		if(pTimer->m_TimerCB == TimerCB) {		// Find async timer callback
			EGAsyncFunc *pAsyncObj = (EGAsyncFunc*)pTimer->m_pParam;
			if((pAsyncObj->m_AsyncCB == AsyncCB) && (pAsyncObj->m_pParam == pParam)) {			// Match user function callback and user data
				EGTimer::Delete(pTimer);
				delete pAsyncObj;
				Result = EG_RES_OK;
			}
		}
		pTimer = pTimerNext;
	}
	return Result;
}

/////////////////////////////////////////////////////////////////////////////

void EGAsyncFunc::TimerCB(EGTimer *pTimer)
{
	EGAsyncFunc *pAsyncObj = (EGAsyncFunc *)pTimer->m_pParam;
	pAsyncObj->m_AsyncCB(pAsyncObj->m_pParam);
	delete pAsyncObj;
}
