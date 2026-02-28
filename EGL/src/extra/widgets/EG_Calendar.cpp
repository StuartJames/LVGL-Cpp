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

#include "extra/widgets/EG_Calendar.h"
#include "EGL.h"
#if EG_USE_CALENDAR

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_CALENDAR_CTRL_TODAY EG_BTNMATRIX_CTRL_CUSTOM_1
#define EG_CALENDAR_CTRL_HIGHLIGHT EG_BTNMATRIX_CTRL_CUSTOM_2

#define CALENDAR_CLASS &c_CalendarClass

constexpr int c_InitialYear = 2025;

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_CalendarClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = (EG_DPI_DEF * 3) / 2,
	.HeightDef = (EG_DPI_DEF * 3) / 2,
  .IsEditable = 0,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

const char *EGCalendar::m_pDayNamesDef[] = EG_CALENDAR_DEFAULT_DAY_NAMES;

///////////////////////////////////////////////////////////////////////////////////////

EGCalendar::EGCalendar(void) : EGObject(),
  m_pButtonMatrix(nullptr),
  m_pHeader(nullptr)
{
	m_Today.Year = c_InitialYear;
	m_Today.Month = 1;
	m_Today.Day = 1;
	m_VisableDate.Year = c_InitialYear;
	m_VisableDate.Month = 1;
	m_VisableDate.Day = 1;
	m_pHighlighted = nullptr;
	m_HighlightedCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

EGCalendar::EGCalendar(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_CalendarClass*/) : EGObject(),
  m_pButtonMatrix(nullptr),
  m_pHeader(nullptr)
{
	m_Today.Year = c_InitialYear;
	m_Today.Month = 1;
	m_Today.Day = 1;
	m_VisableDate.Year = c_InitialYear;
	m_VisableDate.Month = 1;
	m_VisableDate.Day = 1;
	m_pHighlighted = nullptr;
	m_HighlightedCount = 0;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGCalendar::~EGCalendar(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::Configure(void)
{
  EGObject::Configure();
	m_Today.Year = c_InitialYear;
	m_Today.Month = 1;
	m_Today.Day = 1;
	m_VisableDate.Year = c_InitialYear;
	m_VisableDate.Month = 1;
	m_VisableDate.Day = 1;
	m_pHighlighted = nullptr;
	m_HighlightedCount = 0;
	EG_ZeroMem(m_Numbers, sizeof(m_Numbers));
	uint8_t j = 0;
	for(uint8_t i = 0; i < 8 * 7; i++) {		// Every 8th string is "\n"
		if(i != 0 && (i + 1) % 8 == 0){
     m_pMap[i] = "\n";
    }
		else{
      if(i < 7){
        m_pMap[i] = m_pDayNamesDef[i];
      }
		  else {
  			m_Numbers[j][0] = 'x';
	  		m_pMap[i] = m_Numbers[j];
		  	j++;
      }
		}
	}
	m_pMap[8 * 7 - 1] = "";
  m_pButtonMatrix = new EGButtonMatrix(this);
	m_pButtonMatrix->SetMap(m_pMap);
	m_pButtonMatrix->SetControlAll(EG_BTNMATRIX_CTRL_CLICK_TRIG | EG_BTNMATRIX_CTRL_NO_REPEAT);
	EGEvent::AddEventCB(m_pButtonMatrix, DrawPartEventCB, EG_EVENT_DRAW_PART_BEGIN, nullptr);
	m_pButtonMatrix->SetWidth(EG_PCT(100));
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_COLUMN);
	EGFlexLayout::SetObjGrow(m_pButtonMatrix, 1);
	SetVisableDate(m_VisableDate.Year, m_VisableDate.Month);
	SetTodaysDate(m_Today.Year, m_Today.Month, m_Today.Day);
	m_pButtonMatrix->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
}

///////////////////////////////////////////////////////////////////////////////////////

EGObject* EGCalendar::CreateDropDownHeader(void)
{
  m_pHeader = new EGCalendarDropDownHeader(this);
  return m_pHeader;
}
 
///////////////////////////////////////////////////////////////////////////////////////

EGObject* EGCalendar::CreateMonthHeader(void)
{
  m_pHeader = new EGCalendarMonthHeader(this);
  return m_pHeader;
}
 
 ///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::SetDayNames(const char *pDayNames[])
{
	for(uint32_t i = 0; i < 7; i++) {
		m_pMap[i] = pDayNames[i];
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::SetTodaysDate(uint32_t Year, uint32_t Month, uint32_t Day)
{
	m_Today.Year = Year;
	m_Today.Month = Month;
	m_Today.Day = Day;
	UpdateHighlighted();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::SetHighlightedDates(EG_CalendarDate_t Highlighted[], uint16_t Count)
{
	EG_ASSERT_NULL(Highlighted);
	m_pHighlighted = Highlighted;
	m_HighlightedCount = Count;
	UpdateHighlighted();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::SetVisableDate(uint32_t Year, uint32_t Month)
{
uint32_t i;

	m_VisableDate.Year = Year;
	m_VisableDate.Month = Month;
	m_VisableDate.Day = 1;
	EG_CalendarDate_t Date;
	Date.Year = m_VisableDate.Year;
	Date.Month = m_VisableDate.Month;
	Date.Day = m_VisableDate.Day;
	// Remove the disabled state but revert it for Day names
	m_pButtonMatrix->ClearControlAll(EG_BTNMATRIX_CTRL_DISABLED);
	for(i = 0; i < 7; i++) {
		m_pButtonMatrix->SetControl(i, EG_BTNMATRIX_CTRL_DISABLED);
	}
	uint8_t MonthLength = GetMonthLength(Date.Year, Date.Month);
	uint8_t FirstDay = GetDayOfWeek(Date.Year, Date.Month, 1);
	uint8_t c;
	for(i = FirstDay, c = 1; i < MonthLength + FirstDay; i++, c++) {
		eg_snprintf(m_Numbers[i], sizeof(m_Numbers[0]), "%d", c);
	}
	uint8_t prev_mo_len = GetMonthLength(Date.Year, Date.Month - 1);
	for(i = 0, c = prev_mo_len - FirstDay + 1; i < FirstDay; i++, c++) {
		eg_snprintf(m_Numbers[i], sizeof(m_Numbers[0]), "%d", c);
		m_pButtonMatrix->SetControl(i + 7, EG_BTNMATRIX_CTRL_DISABLED);
	}
	for(i = FirstDay + MonthLength, c = 1; i < 6 * 7; i++, c++) {
		eg_snprintf(m_Numbers[i], sizeof(m_Numbers[0]), "%d", c);
		m_pButtonMatrix->SetControl(i + 7, EG_BTNMATRIX_CTRL_DISABLED);
	}
	UpdateHighlighted();
	if(m_pButtonMatrix->GetSelected() != EG_BTNMATRIX_BTN_NONE) {	// Reset the focused button if the days changes
		m_pButtonMatrix->SetSelected(FirstDay + 7);
	}
	Invalidate();
	//  The children of the calendar are probably headers. Notify them to let the headers updated to the new date
	uint32_t ChildCount = GetChildCount();	//  The children of the calendar are probably headers. Notify them to let the headers updated to the new date
	for(i = 0; i < ChildCount; i++) {
		EGObject *pChild = GetChild(i);
		if(pChild == m_pButtonMatrix) continue;
		EGEvent::EventSend(pChild, EG_EVENT_VALUE_CHANGED, this);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGCalendar::GetSelectedDate(EG_CalendarDate_t *pDate)
{
	uint16_t DayButton = m_pButtonMatrix->GetSelected();
	if(DayButton == EG_BTNMATRIX_BTN_NONE) {
		pDate->Year = 0;
		pDate->Month = 0;
		pDate->Day = 0;
		return EG_RES_INVALID;
	}
	const char *pText = m_pButtonMatrix->GetButtonText(DayButton);
//  ESP_LOGI("[Calndr]", "Day button:%d, %s", DayButton, pText);
	if(pText[1] == 0) pDate->Day = pText[0] - '0';
	else pDate->Day = (pText[0] - '0') * 10 + (pText[1] - '0');
	pDate->Year = m_VisableDate.Year;
	pDate->Month = m_VisableDate.Month;
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::DrawPartEventCB(EGEvent *pEvent)
{
	EGButtonMatrix *pButtonMtrx = (EGButtonMatrix*)pEvent->GetTarget();
	EGDrawDiscriptor *pDrawDiscriptor = (EGDrawDiscriptor*)pEvent->GetParam();
	if(pDrawDiscriptor->m_Part == EG_PART_ITEMS) {
		if(pDrawDiscriptor->m_Index < 7) {		// Day name styles
			pDrawDiscriptor->m_pDrawRect->m_BackgroundOPA = EG_OPA_TRANSP;
			pDrawDiscriptor->m_pDrawRect->m_BorderOPA = EG_OPA_TRANSP;
		}
		else if(pButtonMtrx->HasControl(pDrawDiscriptor->m_Index, EG_BTNMATRIX_CTRL_DISABLED)) {
			pDrawDiscriptor->m_pDrawRect->m_BackgroundOPA = EG_OPA_TRANSP;
			pDrawDiscriptor->m_pDrawRect->m_BorderOPA = EG_OPA_TRANSP;
			pDrawDiscriptor->m_pDrawLabel->m_Color = EG_MainPalette(EG_PALETTE_GREY);
		}
		if(pButtonMtrx->HasControl(pDrawDiscriptor->m_Index, EG_CALENDAR_CTRL_HIGHLIGHT)) {
			pDrawDiscriptor->m_pDrawRect->m_BackgroundOPA = EG_OPA_40;
			pDrawDiscriptor->m_pDrawRect->m_BackgroundColor = EGTheme::GetColorPrimary(pButtonMtrx);
			if(pButtonMtrx->GetSelected() == pDrawDiscriptor->m_Index) {
				pDrawDiscriptor->m_pDrawRect->m_BackgroundOPA = EG_OPA_70;
			}
		}
		if(pButtonMtrx->HasControl(pDrawDiscriptor->m_Index, EG_CALENDAR_CTRL_TODAY)) {
			pDrawDiscriptor->m_pDrawRect->m_BorderOPA = EG_OPA_COVER;
			pDrawDiscriptor->m_pDrawRect->m_BorderColor = EGTheme::GetColorPrimary(pButtonMtrx);
			pDrawDiscriptor->m_pDrawRect->m_BorderWidth += 1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

uint8_t EGCalendar::GetMonthLength(int32_t Year, int32_t Month)
{
	Month--;
	if(Month < 0) {
		Year--;             // Already in the previous Year (won't be less then -12 to skip a whole Year)
		Month = 12 + Month; // `Month` is negative, the result will be < 12
	}
	if(Month >= 12) {
		Year++;
		Month -= 12;
	}
	// Month == 1 is february
	return (Month == 1) ? (28 + IsLeapYear(Year)) : 31 - Month % 7 % 2;
}

///////////////////////////////////////////////////////////////////////////////////////

uint8_t EGCalendar::IsLeapYear(uint32_t Year)
{
	return (Year % 4) || ((Year % 100 == 0) && (Year % 400)) ? 0 : 1;
}

///////////////////////////////////////////////////////////////////////////////////////

uint8_t EGCalendar::GetDayOfWeek(uint32_t Year, uint32_t Month, uint32_t Day)
{
	uint32_t a = Month < 3 ? 1 : 0;
	uint32_t b = Year - a;
#if EG_CALENDAR_WEEK_STARTS_MONDAY
	uint32_t day_of_week = (Day + (31 * (Month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400) - 1) % 7;
#else
	uint32_t day_of_week = (Day + (31 * (Month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400)) % 7;
#endif
	return day_of_week;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCalendar::UpdateHighlighted(void)
{
uint16_t i;

	// Clear all selections
	m_pButtonMatrix->ClearControlAll(EG_CALENDAR_CTRL_TODAY | EG_CALENDAR_CTRL_HIGHLIGHT);
	uint8_t FirstDay = GetDayOfWeek(m_VisableDate.Year, m_VisableDate.Month, 1);
	if(m_pHighlighted) {
		for(i = 0; i < m_HighlightedCount; i++) {
			if(m_pHighlighted[i].Year == m_VisableDate.Year &&
				 m_pHighlighted[i].Month == m_VisableDate.Month) {
				m_pButtonMatrix->SetControl(m_pHighlighted[i].Day - 1 + FirstDay + 7, EG_CALENDAR_CTRL_HIGHLIGHT);
			}
		}
	}
	if(m_VisableDate.Year == m_Today.Year && m_VisableDate.Month == m_Today.Month) {
		m_pButtonMatrix->SetControl(m_Today.Day - 1 + FirstDay + 7, EG_CALENDAR_CTRL_TODAY);
	}
}


#endif 
