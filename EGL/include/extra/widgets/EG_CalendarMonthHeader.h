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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_CALENDAR_MONTH_HEADER

extern const EG_ClassType_t c_CalendarMonthHeaderClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGCalendarMonthHeader : public EGObject
{
public:
                            EGCalendarMonthHeader(void){};
                            EGCalendarMonthHeader(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_CalendarMonthHeaderClass);
  virtual void              Configure(void);

  static void               MonthEventCB(EGEvent *pEvent);
  static void               ValueChangedEventCB(EGEvent *pEvent);

private:
  static const char         *m_pMonthNamesDef[12];

};

#endif 