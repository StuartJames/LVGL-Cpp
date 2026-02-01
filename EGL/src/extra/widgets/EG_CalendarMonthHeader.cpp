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
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "extra/widgets/EG_CalendarMonthHeader.h"
#if EG_USE_CALENDAR_MONTH_HEADER

#include "extra/widgets/EG_Calendar.h"
#include "widgets/EG_Button.h"
#include "widgets/EG_Label.h"
#include "extra/layouts/EG_Flex.h"

///////////////////////////////////////////////////////////////////////////////////////

#define CALENDAR_MONTH_HEADER_CLASS &c_CalendarMonthHeaderClass

const EG_ClassType_t c_CalendarMonthHeaderClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = _EG_PCT(100),
	.HeightDef = EG_DPI_DEF / 3,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

const char *EGCalendarMonthHeader::m_pMonthNamesDef[12] = EG_CALENDAR_DEFAULT_MONTH_NAMES;

///////////////////////////////////////////////////////////////////////////////////////

EGCalendarMonthHeader::EGCalendarMonthHeader(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_CalendarHeaderArrowClass*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarMonthHeader::Configure(void)
{
  EGObject::Configure();
	MoveToIndex(0);
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_ROW);
	EGFlexLayout::SetObjAlign(this, EG_FLEX_ALIGN_START, EG_FLEX_ALIGN_CENTER, EG_FLEX_ALIGN_START);
	EGButton *pMonthPrev = new EGButton(this);
	pMonthPrev->SetStyleBackImageSource(EG_SYMBOL_LEFT, 0);
	pMonthPrev->SetHeight(EG_PCT(100));
	pMonthPrev->UpdateLayout();
	EG_Coord_t btn_size = pMonthPrev->GetHeight();
	pMonthPrev->SetWidth(btn_size);
	EGEvent::AddEventCB(pMonthPrev, MonthEventCB, EG_EVENT_CLICKED, nullptr);
	pMonthPrev->ClearFlag(EG_OBJ_FLAG_CLICK_FOCUSABLE);
	EGLabel *pLabel = new EGLabel(this);
	pLabel->SetLongMode(EG_LABEL_LONG_SCROLL_CIRCULAR);
	pLabel->SetStyleTextAlign(EG_TEXT_ALIGN_CENTER, 0);
	EGFlexLayout::SetObjGrow(pLabel, 1);
	EGButton *pMonthNext = new EGButton(this);
	pMonthNext->SetStyleBackImageSource(EG_SYMBOL_RIGHT, 0);
	pMonthNext->SetSize(btn_size, btn_size);
	EGEvent::AddEventCB(pMonthNext, MonthEventCB, EG_EVENT_CLICKED, nullptr);
	pMonthNext->ClearFlag(EG_OBJ_FLAG_CLICK_FOCUSABLE);
	EGEvent::AddEventCB(this, ValueChangedEventCB, EG_EVENT_VALUE_CHANGED, nullptr);
	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);	/*Refresh the drop downs*/
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarMonthHeader::MonthEventCB(EGEvent *pEvent)
{
	EGCalendarMonthHeader *pButton = (EGCalendarMonthHeader*)pEvent->GetTarget();
	EGObject *pHeader = pButton->GetParent();
	EGCalendar *pCalendar = (EGCalendar*)pHeader->GetParent();
	const EG_CalendarDate_t *pDate;
	pDate = pCalendar->GetVisableDate();
	EG_CalendarDate_t Date = *pDate;
	if(pHeader->GetChild(0) == pButton) {	// The last child is the right button
		if(Date.Month == 1) {
			Date.Month = 12;
			Date.Year--;
		}
		else Date.Month--;
	}
	else {
		if(Date.Month == 12) {
			Date.Month = 1;
			Date.Year++;
		}
		else Date.Month++;
	}
	pCalendar->SetVisableDate(Date.Year, Date.Month);
	EGLabel *pLabel = (EGLabel*)pHeader->GetChild(1);
	pLabel->SetFormatText("%d %s", Date.Year, m_pMonthNamesDef[Date.Month - 1]);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarMonthHeader::ValueChangedEventCB(EGEvent *pEvent)
{
	EGObject *pHeader = pEvent->GetTarget();
	EGCalendar *pCalendar = (EGCalendar*)pHeader->GetParent();
	const EG_CalendarDate_t *pDate = pCalendar->GetVisableDate();
	EGLabel *pLabel = (EGLabel*)pHeader->GetChild(1);
	pLabel->SetFormatText("%d %s", pDate->Year, m_pMonthNamesDef[pDate->Month - 1]);
}

#endif
