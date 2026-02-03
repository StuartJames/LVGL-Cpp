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

#include "misc/EG_Timeline.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Assert.h"

/////////////////////////////////////////////////////////////////////////////

EGAnimationTimeline::EGAnimationTimeline(void) :
  m_pTimelines(nullptr),
  m_TimelineCount(0),
  m_Reverse(0)
{
}
/////////////////////////////////////////////////////////////////////////////

EGAnimationTimeline *EGAnimationTimeline::Configure(void)
{
	EGAnimationTimeline *pTimeline = new EGAnimationTimeline;
	return pTimeline;
}

/////////////////////////////////////////////////////////////////////////////

void EGAnimationTimeline::Delete(EGAnimationTimeline *pTimeLine)
{
	EG_ASSERT_NULL(pTimeLine);
	pTimeLine->Stop();
	EG_FreeMem(pTimeLine->m_pTimelines);
	delete pTimeLine;
}

/////////////////////////////////////////////////////////////////////////////

void EGAnimationTimeline::Add(uint32_t StartTime, EGAnimate *pAnimation)
{
	m_TimelineCount++;
	m_pTimelines = (EG_TimelineDiscriptor_t *)EG_ReallocMem(m_pTimelines, m_TimelineCount * sizeof(EG_TimelineDiscriptor_t));
	EG_ASSERT_MALLOC(m_pTimelines);
	m_pTimelines[m_TimelineCount - 1].pAnimation = pAnimation;
	m_pTimelines[m_TimelineCount - 1].StartTime = StartTime;
	if(pAnimation->m_pItem == NULL && pAnimation->m_AnimateCB == NULL) {	//Add default var and virtual exec_cb, used to delete animation.
		m_pTimelines[m_TimelineCount - 1].pAnimation->m_pItem = this;
		m_pTimelines[m_TimelineCount - 1].pAnimation->m_AnimateCB = TimelineCB;
	}
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGAnimationTimeline::Start(void)
{
	const uint32_t playtime = GetPlaytime();
	for(uint32_t i = 0; i < m_TimelineCount; i++) {
		EGAnimate *pAnimation = m_pTimelines[i].pAnimation;
		uint32_t StartTime = m_pTimelines[i].StartTime;
		if(m_Reverse) {
			int32_t temp = pAnimation->m_StartValue;
			pAnimation->m_StartValue = pAnimation->m_EndValue;
			pAnimation->m_EndValue = temp;
			pAnimation->SetDelay(playtime - (StartTime + pAnimation->m_Time));
		}
		else {
			pAnimation->SetDelay(StartTime);
		}
		EGAnimate::Create(pAnimation);
	}
	return playtime;
}

/////////////////////////////////////////////////////////////////////////////

void EGAnimationTimeline::Stop(void)
{
	for(uint32_t i = 0; i < m_TimelineCount; i++) {
		EGAnimate *pAnimation = m_pTimelines[i].pAnimation;
		EGAnimate::Delete(pAnimation->m_pItem, pAnimation->m_AnimateCB);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGAnimationTimeline::SetReverse(bool Reverse)
{
	m_Reverse = Reverse;
}

/////////////////////////////////////////////////////////////////////////////

void EGAnimationTimeline::SetProgress(uint16_t Progress)
{
	const uint32_t ActiveTime = Progress * GetPlaytime() / 0xFFFF;
	for(uint32_t i = 0; i < m_TimelineCount; i++) {
		EGAnimate *pAnimation = m_pTimelines[i].pAnimation;
		if(pAnimation->m_AnimateCB == NULL) continue;
		uint32_t StartTime = m_pTimelines[i].StartTime;
		int32_t Value = 0;
		if(ActiveTime < StartTime) Value = pAnimation->m_StartValue;
		else if(ActiveTime < (StartTime + pAnimation->m_Time)) {
			pAnimation->m_ActiveTime = ActiveTime - StartTime;
			Value = pAnimation->m_PathCB(pAnimation);
		}
		else Value = pAnimation->m_EndValue;
		pAnimation->m_AnimateCB(pAnimation, Value);
	}
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGAnimationTimeline::GetPlaytime(void)
{
	uint32_t Playtime = 0;
	for(uint32_t i = 0; i < m_TimelineCount; i++) {
		uint32_t End = EGAnimate::GetPlaytime(m_pTimelines[i].pAnimation);
		if(End == EG_ANIM_PLAYTIME_INFINITE) return End;
		End += m_pTimelines[i].StartTime;
		if(End > Playtime) Playtime = End;
	}
	return Playtime;
}

/////////////////////////////////////////////////////////////////////////////

bool EGAnimationTimeline::GetReverse(void)
{
	return m_Reverse;
}

/////////////////////////////////////////////////////////////////////////////

void EGAnimationTimeline::TimelineCB(EGAnimate *pAnimate, int32_t Var)
{
	EG_UNUSED(pAnimate);
	EG_UNUSED(Var);
}
