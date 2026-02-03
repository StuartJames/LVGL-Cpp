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

#include "EG_Animate.h"

/*Data of anim_timeline_dsc*/
typedef struct {
	EGAnimate *pAnimation;
	uint32_t StartTime;
} EG_TimelineDiscriptor_t;


/////////////////////////////////////////////////////////////////////////////

class EGAnimationTimeline
{
public:
                              EGAnimationTimeline(void);
  void                        Add(uint32_t StartTime, EGAnimate *pAnimation);
  uint32_t                    Start(void);
  void                        Stop(void);
  void                        SetReverse(bool Reverse);
  void                        SetProgress(uint16_t Progress);
  uint32_t                    GetPlaytime(void);
  bool                        GetReverse(void);

  static EGAnimationTimeline* Configure(void);
  static void                 Delete(EGAnimationTimeline *pTimeLine);
  static void                 TimelineCB(EGAnimate *pAnimate, int32_t v);

private:
  EG_TimelineDiscriptor_t     *m_pTimelines;    // Dynamically allocated anim dsc array
  uint32_t                    m_TimelineCount;  // The length of anim dsc array
  bool                        m_Reverse;        // Reverse playback

};
