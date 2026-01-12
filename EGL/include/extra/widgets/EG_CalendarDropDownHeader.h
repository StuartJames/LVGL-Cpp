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

#if EG_USE_CALENDAR_DROPDOWN_HEADER

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_CalendarDropDownHeaderClass;

class EGCalendar;
class EGDropDown;

///////////////////////////////////////////////////////////////////////////////////////

class EGCalendarDropDownHeader : public EGObject
{
public:
//                      EGCalendarDropDownHeader(void){};
                      EGCalendarDropDownHeader(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_CalendarDropDownHeaderClass);
  virtual             ~EGCalendarDropDownHeader(void);
  virtual void        Configure(void);

  void                GenerateYearList(uint16_t Start, uint16_t End);
  void                SetYearList(EGCalendar *pParent, const char *pYearsList);
  
  static void         YearEventCB(EGEvent *pEvent);
  static void         MonthEventCB(EGEvent *pEvent);
  static void         ChangedEventCB(EGEvent *pEvent);

private:
  EGDropDown         *m_pYear;
  EGDropDown         *m_pMonth;
};
#endif 