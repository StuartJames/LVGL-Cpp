
/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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

#include "extra/widgets/EG_CalendarDropDownHeader.h"
#if EG_USE_CALENDAR_DROPDOWN_HEADER

#include "extra/widgets/EG_Calendar.h"
#include "widgets/EG_DropDown.h"
#include "extra/layouts/EG_Flex.h"

///////////////////////////////////////////////////////////////////////////////////////

#define CALENDAR_HEADER_CLASS &c_CalendarDropDownHeaderClass

const EG_ClassType_t c_CalendarDropDownHeaderClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = _EG_PCT(100),
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const char *c_pMonthList = "Jan\nFeb\nMar\nApr\nMay\nJun\nJul\nAug\nSep\nOct\nNov\nDec";

///////////////////////////////////////////////////////////////////////////////////////

EGCalendarDropDownHeader::EGCalendarDropDownHeader(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_CalendarDropDownHeaderClass*/) : EGObject(),
  m_pYear(nullptr),
  m_pMonth(nullptr)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGCalendarDropDownHeader::~EGCalendarDropDownHeader(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarDropDownHeader::Configure(void)
{
  EGObject::Configure();
	EGCalendar *pCalendar = (EGCalendar*)GetParent();
	MoveToIndex(0);     // Make sure it's drawn first
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_ROW);
	m_pYear = new EGDropDown(this);
  GenerateYearList(1950, 2025);
	EGEvent::AddEventCB(m_pYear, YearEventCB, EG_EVENT_VALUE_CHANGED, pCalendar);
	EGFlexLayout::SetObjGrow(m_pYear, 1);
	m_pMonth = new EGDropDown(this);
	m_pMonth->SetStaticItems(c_pMonthList);
	EGEvent::AddEventCB(m_pMonth, MonthEventCB, EG_EVENT_VALUE_CHANGED, pCalendar);
	EGFlexLayout::SetObjGrow(m_pMonth, 1);
	EGEvent::AddEventCB(this, ChangedEventCB, EG_EVENT_VALUE_CHANGED, nullptr);
	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr);	//Refresh the drop downs
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarDropDownHeader::GenerateYearList(uint16_t StartYear, uint16_t EndYear)
{
char DateStr[8];

  if((StartYear == 0) || (StartYear > EndYear)) return;
	m_pYear->ClearItems();
  for(int Year = StartYear; Year <= EndYear; ++Year){
		eg_snprintf(DateStr, sizeof(DateStr), "%04d", Year);
    m_pYear->AddItems(DateStr, 0);
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarDropDownHeader::SetYearList(EGCalendar *pCalendar, const char *pYearsList)
{
	m_pYear->ClearItems();
	m_pYear->SetItems(pYearsList);
	pCalendar->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

/*void EGCalendarDropDownHeader::SetYearList(EGCalendar *pCalendar, const char *pYearsList)
{
bool ChildFound = false;
uint32_t Index = 0;
EGObject *pChild = nullptr;

	uint32_t ChildCount = pCalendar->GetChildCount();
	for(Index = 0; Index < ChildCount; Index++) {	// Search for the header dropdown 
		pChild = pCalendar->GetChild(Index);
		if(EGObject::IsKindOf(pChild, &c_CalendarDropDownHeaderClass)) {
			ChildFound = true;
			break;
		}
	}
	if(!ChildFound) return;
	ChildFound = false;
	ChildCount = pChild->GetChildCount();
	for(Index = 0; Index < ChildCount; Index++) {	// Search for the year dropdown 
		pChild = pChild->GetChild(Index);
		if(EGObject::IsKindOf(pChild, &c_DropDownClass)) {
			ChildFound = true;
			break;
		}
	}
	if(!ChildFound) return;
	((EGDropDown*)pChild)->ClearItems();
	((EGDropDown*)pChild)->SetItems(pYearsList);
	pCalendar->Invalidate();
}*/

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarDropDownHeader::MonthEventCB(EGEvent *pEvent)
{
	EGDropDown *pDropDown = (EGDropDown*)pEvent->GetTarget();
	EGCalendar *pCalendar = (EGCalendar*)pEvent->GetExtParam();
	uint16_t Selected = pDropDown->GetSelectedIndex();
	const EG_CalendarDate_t *pDate = pCalendar->GetVisableDate();
	EG_CalendarDate_t NewDate = *pDate;
	NewDate.Month = Selected + 1;
	pCalendar->SetVisableDate(NewDate.Year, NewDate.Month);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarDropDownHeader::YearEventCB(EGEvent *pEvent)
{
	EGDropDown *pDropDown = (EGDropDown*)pEvent->GetTarget();
	EGCalendar *pCalendar = (EGCalendar*)pEvent->GetExtParam();
	uint16_t Selected = pDropDown->GetSelectedIndex();
	const EG_CalendarDate_t *pDate = pCalendar->GetVisableDate();
	// Get the first year on the options list NOTE: Assumes the first 4 digits in the option list are numbers 
	EG_CalendarDate_t NewDate = *pDate;
	const char *pYear = pDropDown->GetItems();
	const uint32_t Year = (pYear[0] - '0') * 1000 + (pYear[1] - '0') * 100 + (pYear[2] - '0') * 10 + (pYear[3] - '0');
	NewDate.Year = Year - Selected;
	pCalendar->SetVisableDate(NewDate.Year, NewDate.Month);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendarDropDownHeader::ChangedEventCB(EGEvent *pEvent)
{
	EGObject *pHeader = (EGObject*)pEvent->GetTarget();
	EGCalendar *pCalendar = (EGCalendar*)pHeader->GetParent();
	const EG_CalendarDate_t *pDate = pCalendar->GetVisableDate();
	EGDropDown *pYearDD = (EGDropDown*)pHeader->GetChild(0);
	// Get the first year on the options list NOTE: Assumes the first 4 digits in the option list are numbers 
	const char *pYear = pYearDD->GetItems();
	const uint32_t Year = (pYear[0] - '0') * 1000 + (pYear[1] - '0') * 100 + (pYear[2] - '0') * 10 + (pYear[3] - '0');
	pYearDD->SetSelectedIndex(Year - pDate->Year);
	EGDropDown *pMonthDD = (EGDropDown*)pHeader->GetChild(1);
	pMonthDD->SetSelectedIndex(pDate->Month - 1);
}

#endif 
