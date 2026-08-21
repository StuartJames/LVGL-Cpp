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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "EGL.h"

#if EG_USE_MONKEY != 0

///////////////////////////////////////////////////////////////////////////////

typedef struct {
    EG_InDeviceType_e Type;    // Input device type
    struct {      // Monkey execution period
      uint32_t Min;
      uint32_t Max;
    } Period;
    struct {      // The range of input value
      int32_t Min;
      int32_t Max;
    } Input;
} EG_MonkeyConfig_t;

///////////////////////////////////////////////////////////////////////////////

class EGMonkey
{
public:
                      EGMonkey(const EG_MonkeyConfig_t *pConfig);
                      ~EGMonkey(void);

  EGInputDevice*      GetIndev(void){ return m_pIndev; };
  void                SetEnable(bool Flag);
  bool                GetEnable(void){ return m_pTimer->IsPaused(); };

  static void         InitialiseConfig(EG_MonkeyConfig_t *pConfig);

#if EG_USE_EXT_DATA
  void                SetExtData(void *pExtData){ m_pExtData = pExtData; };
  void*               GetExtData(void){ return m_pExtData; };
#endif

private:
  static int32_t      Random(int32_t Lowest, int32_t Highest);
  static void         ReadCB(EGInputDriver *pIndevDriver, EG_InputData_t *pData);
  static void         TimerCB(EGTimer *pTimer);

  EG_MonkeyConfig_t   m_Config;
	EGInputDriver       m_IndevDriver;
	EG_InputData_t      m_IndevData;
	EGInputDevice      *m_pIndev;
	EGTimer            *m_pTimer;

#if EG_USE_EXT_DATA
	void               *m_pExtData;
#endif

  static constexpr EG_Key_e      m_KeyMap[12] = {
    EG_KEY_UP,
    EG_KEY_DOWN,
    EG_KEY_RIGHT,
    EG_KEY_LEFT,
    EG_KEY_ESC,
    EG_KEY_DEL,
    EG_KEY_BACKSPACE,
    EG_KEY_ENTER,
    EG_KEY_NEXT,
    EG_KEY_PREV,
    EG_KEY_HOME,
    EG_KEY_END,
  };

};

#endif