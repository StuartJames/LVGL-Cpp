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

#include "widgets/EG_ButtonMatrix.h"

#if EG_USE_CALENDAR

///////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint16_t Year;
    int8_t Month;  //  1..12
    int8_t Day;    //  1..31
} EG_CalendarDate_t;

extern const EG_ClassType_t c_CalendarClass;
class EGCalendarDropDownHeader;

///////////////////////////////////////////////////////////////////////////////////////

class EGCalendar : public EGObject
{
public:

                            EGCalendar(void);
                            EGCalendar(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_CalendarClass);
  virtual                   ~EGCalendar(void);
  void                      Configure(void);
  void                      SetTodaysDate(uint32_t Year, uint32_t Month, uint32_t Day);
  void                      SetVisableDate(uint32_t Year, uint32_t Month);
  void                      SetHighlightedDates(EG_CalendarDate_t Highlighted[], uint16_t Count);
  void                      SetDayNames(const char *pDayNames[]);
  const EG_CalendarDate_t   GetTodaysDate(void){ return m_Today; };
  const EG_CalendarDate_t*  GetVisableDate(void){ return &m_VisableDate; };
  EG_CalendarDate_t*        GetHighlightedDates(void){ return m_pHighlighted; };
  uint16_t                  GetHighlightedCount(void){ return m_HighlightedCount; };
  EG_Result_t               GetSelectedDate(EG_CalendarDate_t *pDate);
  EGObject*                 CreateDropDownHeader(void);
  EGObject*                 CreateMonthHeader(void);

  static void               DrawPartEventCB(EGEvent *pEvent);

  EG_CalendarDate_t         m_Today;        // Date of today
  EG_CalendarDate_t         m_VisableDate;  // Currently visible Month (Day is ignored)
  EG_CalendarDate_t         *m_pHighlighted; // Apply different style on these days (pointer to an array defined by the user)
  uint16_t                  m_HighlightedCount; // Number of elements in `highlighted_days`
  const char                *m_pMap[8 * 7];
  char                      m_Numbers[7 * 6][4];

private:
  uint8_t                   GetDayOfWeek(uint32_t Year, uint32_t Month, uint32_t Day);
  uint8_t                   GetMonthLength(int32_t Year, int32_t Month);
  uint8_t                   IsLeapYear(uint32_t Year);
  void                      UpdateHighlighted(void);

  EGButtonMatrix            *m_pButtonMatrix;
  EGObject                  *m_pHeader;
  static const char         *m_pDayNamesDef[7];

};

#endif  