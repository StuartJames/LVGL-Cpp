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

#include "extra/others/EG_Monkey.h"

#if EG_USE_MONKEY != 0

///////////////////////////////////////////////////////////////////////////////

#define MONKEY_PERIOD_RANGE_MIN_DEF 100
#define MONKEY_PERIOD_RANGE_MAX_DEF 1000

///////////////////////////////////////////////////////////////////////////////

void EGMonkey::InitialiseConfig(EG_MonkeyConfig_t *pConfig)
{
	EG_ZeroMem(pConfig, sizeof(EG_MonkeyConfig_t));
	pConfig->Type = EG_INDEV_TYPE_POINTER;
	pConfig->Period.Min = MONKEY_PERIOD_RANGE_MIN_DEF;
	pConfig->Period.Max = MONKEY_PERIOD_RANGE_MAX_DEF;
}

///////////////////////////////////////////////////////////////////////////////

EGMonkey::EGMonkey(const EG_MonkeyConfig_t *pConfig) :
  m_pIndev(nullptr),
  m_pTimer(nullptr),
  m_pExtData(nullptr)
{
	m_Config = *pConfig;
	EGInputDriver *pDriver = &m_IndevDriver;
	pDriver->m_Type = pConfig->Type;
	pDriver->ReadCB = ReadCB;
	pDriver->m_pExtData = this;
	m_pTimer = EGTimer::Create(TimerCB, m_Config.Period.Min, this, true);
	m_pIndev = EGInputDevice::RegisterDriver(pDriver);
}

///////////////////////////////////////////////////////////////////////////////

EGMonkey::~EGMonkey(void)
{
  if(m_pTimer) EGTimer::Delete(m_pTimer);
	if(m_pIndev) delete m_pIndev;
}

///////////////////////////////////////////////////////////////////////////////

void EGMonkey::SetEnable(bool Flag)
{
	Flag ? m_pTimer->Resume() : m_pTimer->Pause();
}

///////////////////////////////////////////////////////////////////////////////

void EGMonkey::ReadCB(EGInputDriver *pIndevDriver, EG_InputData_t *pData)
{
	EGMonkey *pMonkey = (EGMonkey*)pIndevDriver->m_pExtData;
	pData->ButtonID = pMonkey->m_IndevData.ButtonID;
	pData->Point = pMonkey->m_IndevData.Point;
	pData->EncoderSteps = pMonkey->m_IndevData.EncoderSteps;
	pData->State = pMonkey->m_IndevData.State;
}

///////////////////////////////////////////////////////////////////////////////

int32_t EGMonkey::Random(int32_t Lowest, int32_t Highest)
{
	if(Lowest >= Highest) return Lowest;
	int32_t Diff = Highest - Lowest;
	return (int32_t)EG_Rand(0, Diff) + Lowest;
}

///////////////////////////////////////////////////////////////////////////////

void EGMonkey::TimerCB(EGTimer *pTimer)
{
	EGMonkey *pMonkey = (EGMonkey*)pTimer->m_pParam;
	EG_InputData_t *pData = &pMonkey->m_IndevData;
	switch(pMonkey->m_IndevDriver.m_Type) {
		case EG_INDEV_TYPE_POINTER:
			pData->Point.m_X = (int32_t)EGMonkey::Random(0, EG_DISP_HORZ_RES - 1);
			pData->Point.m_Y = (int32_t)EGMonkey::Random(0, EG_DISP_VERT_RES - 1);
			break;
		case EG_INDEV_TYPE_ENCODER:
			pData->EncoderSteps = (int16_t)EGMonkey::Random(pMonkey->m_Config.Input.Min, pMonkey->m_Config.Input.Max);
			break;
		case EG_INDEV_TYPE_BUTTON:
			pData->ButtonID = (uint32_t)EGMonkey::Random(pMonkey->m_Config.Input.Min, pMonkey->m_Config.Input.Max);
			break;
		case EG_INDEV_TYPE_KEYPAD: {
			int32_t index = EGMonkey::Random(0, sizeof(m_KeyMap) / sizeof(m_KeyMap[0]) - 1);
			pData->Key = (uint32_t)m_KeyMap[index];
			break;
		}
		default:
			break;
	}
	pData->State = EGMonkey::Random(0, 100) < 50 ? EG_INDEV_STATE_RELEASED : EG_INDEV_STATE_PRESSED;
	pMonkey->m_pTimer->SetPeriod(EGMonkey::Random(pMonkey->m_Config.Period.Min, pMonkey->m_Config.Period.Max));
}

#endif
