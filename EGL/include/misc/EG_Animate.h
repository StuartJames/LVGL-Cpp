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
#include "misc/EG_List.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "esp_log.h"

#define EG_ANIM_REPEAT_INFINITE      0xFFFF
#define EG_ANIM_PLAYTIME_INFINITE    0xFFFFFFFF

EG_EXPORT_CONST_INT(EG_ANIM_REPEAT_INFINITE);
EG_EXPORT_CONST_INT(EG_ANIM_PLAYTIME_INFINITE);

// Can be used to indicate if animations are enabled or disabled in a case
typedef enum {
    EG_ANIM_OFF,
    EG_ANIM_ON,
} EG_AnimateEnable_e;

class EGAnimate;

// Get the current value during an animation
typedef int32_t (*EG_AnimatePathCB_t)(EGAnimate *);

/** Generic prototype of "animator" functions.
 * First parameter is the variable to animate.
 * Second parameter is the value to set.*/
typedef void (*EG_AnimateExecCB_t)(EGAnimate *, int32_t);

/** Same as `EG_AnimateExecCB_t` but receives `EGAnimate *` as the first parameter.
 * It's more consistent but less convenient. Might be used by binding generator functions.*/
typedef void (*EG_AnimateCustomExecCB_t)(EGAnimate *, int32_t);

// Callback to call when the animation is ready
typedef void (*EG_AnimateReadyCB_t)(EGAnimate *);

// Callback to call when the animation really stars (considering `delay`)
typedef void (*EG_AnimateStartCB_t)(EGAnimate *);

// Callback used when the animation values are relative to get the current value
typedef int32_t (*EG_AnimateGetValueCB_t)(EGAnimate *);

// Callback used when the animation is deleted
typedef void (*EG_AnimateDeletedCB_t)(EGAnimate *);

class EGTimer;

/////////////////////////////////////////////////////////////////////////////////////////

class EGAnimate
{
public:
                          EGAnimate(void);
  virtual			            ~EGAnimate(void);
  uint16_t                RunningCount(void);
  EGTimer*                GetTimer(void);
  void                    SetItem(void *pItem);
  void                    SetExcCB(EG_AnimateExecCB_t AnimateCB);
  void                    SetTime(uint32_t Duration);
  void                    SetDelay(uint32_t Delay);
  void                    SetValues(int32_t Start, int32_t End);
  void                    SetCustomCB(EGAnimate *pAnimate, EG_AnimateCustomExecCB_t AnimateCB);
  void                    SetPathCB(EG_AnimatePathCB_t PathCB);
  void                    SetStartCB(EG_AnimateStartCB_t StartCB);
  void                    SetGetValueCB(EG_AnimateGetValueCB_t GetValueCB);
  void                    SetEndCB(EG_AnimateReadyCB_t ReadyCB);
  void                    SetDeletedCB(EG_AnimateDeletedCB_t DeletedCB);
  void                    SetPlaybackTime(uint32_t Time);
  void                    SetPlaybackDelay(uint32_t Delay);
  void                    SetRepeatCount(uint16_t Count);
  void                    SetRepeatDelay(uint32_t Delay);
  void                    SetEarlyApply(bool Enable);
  uint32_t                GetDelay(void);
  bool                    DeleteCustom(EGAnimate *pAnimate, EG_AnimateCustomExecCB_t exec_cb);
  EGAnimate*              GetCustom(EGAnimate *pAnimate, EG_AnimateCustomExecCB_t exec_cb);
  void                    Copy(EGAnimate *pAnimate);
  void                    operator = (const EGAnimate &rval);

#if EG_USE_USER_DATA
  void                    SetUserData(void *pParam);
  void*                   GetUserData(void);
#endif


  static EGAnimate*       Create(EGAnimate *pAnimate);
  static void             ReferenceNow(void);
  static void             InitialiseCore(void);
  static bool             Delete(void *pItem, EG_AnimateExecCB_t AnimateCB);
  static void             DeleteAll(void);
  static EGAnimate*       Get(void *pItem, EG_AnimateExecCB_t AnimateCB);
  static void             AnimateTimerCB(EGTimer *pParam);
  static void             EndHandler(EGAnimate *pAnimate);
  static void             MarkListChanged(void);
  static uint32_t         SpeedToTime(uint32_t Speed, int32_t Start, int32_t End);
  static int32_t          PathLinear(EGAnimate *pAnimate);
  static int32_t          PathEaseIn(EGAnimate *pAnimate);
  static int32_t          PathEaseOut(EGAnimate *pAnimate);
  static int32_t          PathEaseInOut(EGAnimate *pAnimate);
  static int32_t          PathOvershoot(EGAnimate *pAnimate);
  static int32_t          PathBounce(EGAnimate *pAnimate);
  static int32_t          PathStep(EGAnimate *pAnimate);
  static uint32_t         GetPlaytime(EGAnimate *pAnimate);

  void                   *m_pItem;           // Variable to animate
  EG_AnimateExecCB_t      m_AnimateCB;        // Function to execute to animate
  EG_AnimateStartCB_t     m_StartCB;          // Call it when the animation starts (considering `delay`)
  EG_AnimateReadyCB_t     m_EndCB;            // Call it when the animation is about to end
  EG_AnimateDeletedCB_t   m_DeletedCB;        // Call it when the animation is deleted
  EG_AnimateGetValueCB_t  m_GetValueCB;       // Get the current value in relative mode
  EG_AnimatePathCB_t      m_PathCB;           // Describe the path (curve) of animations
  void                    *m_pParam;          // Custom user data
  int32_t                 m_StartValue;       // Start value
  int32_t                 m_CurrentValue;     // Current value
  int32_t                 m_EndValue;         // End value
  int32_t                 m_ActiveTime;       // Current time in animation. Set to negative to make delay.
  int32_t                 m_Time;             // Animation time in ms
  uint32_t                m_PlaybackDelay;    // Wait before play back
  uint32_t                m_PlaybackTime;     // Duration of playback animation
  uint32_t                m_RepeatDelay;      // Wait before repeat
  uint16_t                m_RepeatCount;      // Repeat count for the animation
  uint8_t                 m_EarlyApply  : 1;  // 1: Apply start value immediately even is there is `delay`
  uint8_t                 m_PlaybackNow : 1;  // Play back is in progress
  uint8_t                 m_RunRound : 1;     // Indicates the animation has run in this round
  uint8_t                 m_StartCalled : 1;  // Indicates that the `start_cb` was already called

private:
  static bool             m_ListChanged;
  static uint32_t         m_LastTimerRun;
  static bool             m_AnimateRunRound;
  static EGTimer         *m_pAnimateTimer;
  static EGList           m_AnimateList;
};

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetItem(void *pItem)
{
  m_pItem = pItem;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetExcCB(EG_AnimateExecCB_t AnimateCB)
{
  m_AnimateCB = AnimateCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetTime(uint32_t Duration)
{
  m_Time = Duration;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetDelay(uint32_t Delay)
{
  m_ActiveTime = -(int32_t)(Delay);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetValues(int32_t Start, int32_t End)
{
  m_StartValue = Start;
  m_CurrentValue = Start;
  m_EndValue = End;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetCustomCB(EGAnimate *pAnimate, EG_AnimateCustomExecCB_t AnimateCB)
{
  m_pItem = pAnimate;
  m_AnimateCB = (EG_AnimateExecCB_t)AnimateCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetPathCB(EG_AnimatePathCB_t PathCB)
{
  m_PathCB = PathCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetStartCB(EG_AnimateStartCB_t StartCB)
{
  m_StartCB = StartCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetGetValueCB(EG_AnimateGetValueCB_t GetValueCB)
{
  m_GetValueCB = GetValueCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetEndCB(EG_AnimateReadyCB_t EndCB)
{
  m_EndCB = EndCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetDeletedCB(EG_AnimateDeletedCB_t DeletedCB)
{
  m_DeletedCB = DeletedCB;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetPlaybackTime(uint32_t Time)
{
  m_PlaybackTime = Time;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetPlaybackDelay( uint32_t Delay)
{
  m_RepeatDelay = Delay;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetRepeatCount(uint16_t Count)
{
  m_RepeatCount = Count;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetRepeatDelay(uint32_t Delay)
{
  m_RepeatDelay = Delay;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetEarlyApply(bool Enable)
{
  m_EarlyApply = Enable;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t EGAnimate::GetDelay(void)
{
  return -m_ActiveTime;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline bool EGAnimate::DeleteCustom(EGAnimate *pAnimate, EG_AnimateCustomExecCB_t exec_cb)
{
    return Delete(pAnimate ? pAnimate->m_pItem : NULL, (EG_AnimateExecCB_t)exec_cb);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline EGAnimate* EGAnimate::GetCustom(EGAnimate *pAnimate, EG_AnimateCustomExecCB_t exec_cb)
{
    return Get(pAnimate ? pAnimate->m_pItem : NULL, (EG_AnimateExecCB_t)exec_cb);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGAnimate::SetUserData(void *pParam)
{
  m_pParam = pParam;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void* EGAnimate::GetUserData(void)
{
  return m_pParam;
}
