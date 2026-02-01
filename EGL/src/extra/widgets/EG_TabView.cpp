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


#include "extra/widgets/EG_TabView.h"
#if EG_USE_TABVIEW

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

EG_DirType_e  EGTabView::m_TabPosCreate;
EG_Coord_t    EGTabView::m_TabSizeCreate;

///////////////////////////////////////////////////////////////////////////////////////

#define TABVIEW_CLASS &c_TabViewClass


const EG_ClassType_t c_TabViewClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGTabView::EventCB,
	.WidthDef = _EG_PCT(100),
	.HeightDef = _EG_PCT(100),
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = NULL,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGTabView::EGTabView(void) : EGObject(),
  m_ppMap(nullptr),
  m_TabCount(0),
  m_CurrentTab(0),
  m_TabPosition(EG_DIR_NONE)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGTabView::EGTabView(EGObject *pParent, EG_DirType_e TabPos, EG_Coord_t TabSize, 
  const EG_ClassType_t *pClassCnfg /*= &c_TabViewClass*/) : EGObject(),
  m_ppMap(nullptr),
  m_TabCount(0),
  m_CurrentTab(0),
  m_TabPosition(EG_DIR_NONE)
{
	m_TabPosCreate = TabPos;
	m_TabSizeCreate = TabSize;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGTabView::~EGTabView(void)
{
uint32_t i;

	if(m_TabPosition & EG_DIR_VER) {
		for(i = 0; i < m_TabCount; i++) {
			EG_FreeMem((void *)m_ppMap[i]);
			m_ppMap[i] = NULL;
		}
	}
	if(m_TabPosition & EG_DIR_HOR) {
		for(i = 0; i < m_TabCount; i++) {
			EG_FreeMem((void *)m_ppMap[i * 2]);
			m_ppMap[i * 2] = NULL;
		}
	}
	EG_FreeMem(m_ppMap);
	m_ppMap = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTabView::Configure(void)
{
  EGObject::Configure();
	m_TabPosition = m_TabPosCreate;
	switch(m_TabPosition) {
		case EG_DIR_TOP:
			EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_COLUMN);
			break;
		case EG_DIR_BOTTOM:
			EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_COLUMN_REVERSE);
			break;
		case EG_DIR_LEFT:
			EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_ROW);
			break;
		case EG_DIR_RIGHT:
			EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_ROW_REVERSE);
			break;
    default:
      break;
  }
	SetSize(_EG_PCT(100), _EG_PCT(100));
	EGButtonMatrix *pButtons = new EGButtonMatrix(this);
	EGObject *pContent = new EGObject(this);
	pButtons->SetOneChecked(true);
	m_ppMap = (const char**)EG_AllocMem(sizeof(const char *));
	m_ppMap[0] = "";
	pButtons->SetMap((const char **)m_ppMap);
	EGEvent::AddEventCB(pButtons, ButtonEventCB, EG_EVENT_VALUE_CHANGED, nullptr);
	pButtons->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
	EGEvent::AddEventCB(pContent, ContentEventCB, EG_EVENT_ALL, nullptr);
	pContent->SetScrollbarMode(EG_SCROLLBAR_MODE_OFF);
	switch(m_TabPosition) {
		case EG_DIR_TOP:
		case EG_DIR_BOTTOM:{
			pButtons->SetSize(_EG_PCT(100), m_TabSizeCreate);
			pContent->SetWidth(_EG_PCT(100));
			EGFlexLayout::SetObjGrow(pContent, 1);
			break;
    }
		case EG_DIR_LEFT:
		case EG_DIR_RIGHT:{
			pButtons->SetSize(m_TabSizeCreate, _EG_PCT(100));
			pContent->SetHeight(_EG_PCT(100));
			EGFlexLayout::SetObjGrow(pContent, 1);
			break;
    }
    default: break;
	}
	EGGroup *pGroup = EGGroup::GetDefault();
	if(pGroup) pGroup->AddObject(pButtons);
	if((m_TabPosition & EG_DIR_VER) != 0) {
		EGFlexLayout::SetObjFlow(pContent, EG_FLEX_FLOW_ROW);
		pContent->SetScrollSnapX(EG_SCROLL_SNAP_CENTER);
	}
	else {
		EGFlexLayout::SetObjFlow(pContent, EG_FLEX_FLOW_COLUMN);
		pContent->SetScrollSnapY(EG_SCROLL_SNAP_CENTER);
	}
	pContent->AddFlag(EG_OBJ_FLAG_SCROLL_ONE);
	pContent->ClearFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
}

///////////////////////////////////////////////////////////////////////////////////////

EGObject* EGTabView::AddTab(const char *pName)
{
	EGObject *pContent = GetContent();
	EGObject *pPage = new EGObject(pContent);
	pPage->SetSize(_EG_PCT(100), _EG_PCT(100));
	pPage->ClearFlag(EG_OBJ_FLAG_CLICK_FOCUSABLE);
	uint32_t TabIndex = pContent->GetChildCount();
	EGButtonMatrix *pButtons = GetTabButtons();
	const char **OldMap = (const char **)m_ppMap;
	const char **NewMap;
	if(m_TabPosition & EG_DIR_VER) {	// top or bottom dir*/
		NewMap = (const char**)EG_AllocMem((TabIndex + 1) * sizeof(const char *));
		EG_CopyMemSmall(NewMap, OldMap, sizeof(const char *) * (TabIndex - 1));
		NewMap[TabIndex - 1] = (char*)EG_AllocMem(strlen(pName) + 1);
		strcpy((char *)NewMap[TabIndex - 1], pName);
		NewMap[TabIndex] = "";
	}
	else {	// left or right dir*/
		NewMap = (const char**)EG_AllocMem((TabIndex * 2) * sizeof(const char *));
		EG_CopyMemSmall(NewMap, OldMap, sizeof(const char *) * (TabIndex - 1) * 2);
		if(m_TabCount == 0) {
			NewMap[0] = (char*)EG_AllocMem(strlen(pName) + 1);
			strcpy((char *)NewMap[0], pName);
			NewMap[1] = "";
		}
		else {
			NewMap[TabIndex * 2 - 3] = "\n";
			NewMap[TabIndex * 2 - 2] = (char*)EG_AllocMem(strlen(pName) + 1);
			NewMap[TabIndex * 2 - 1] = "";
			strcpy((char *)NewMap[(TabIndex * 2) - 2], pName);
		}
	}
	m_ppMap = NewMap;
	pButtons->SetMap((const char **)NewMap);
	EG_FreeMem(OldMap);
	pButtons->SetControlAll(EG_BTNMATRIX_CTRL_CHECKABLE | EG_BTNMATRIX_CTRL_CLICK_TRIG | EG_BTNMATRIX_CTRL_NO_REPEAT);
	m_TabCount++;
	if(m_TabCount == 1) SetActive(0, EG_ANIM_OFF);
	pButtons->SetControl(m_CurrentTab, EG_BTNMATRIX_CTRL_CHECKED);
	return pPage;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTabView::RenameTab(uint32_t Index, const char *pName)
{
	if(Index >= m_TabCount) return;
	if(m_TabPosition & EG_DIR_HOR) Index *= 2;
	EG_FreeMem((void *)m_ppMap[Index]);
	m_ppMap[Index] = (char*)EG_AllocMem(strlen(pName) + 1);
	strcpy((char *)m_ppMap[Index], pName);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTabView::SetActive(uint32_t Index, EG_AnimateEnable_e Enable)
{
	if(m_IsBeingDeleted) return;
	if(Index >= m_TabCount) Index = m_TabCount - 1;
	UpdateLayout();	// To be sure 'get content width' will return valid value
	EGObject *pContent = GetContent();
	if(pContent == nullptr) return;
	if((m_TabPosition & EG_DIR_VER) != 0) {
		EG_Coord_t Gap = pContent->GetStylePadColumn(EG_PART_MAIN);
		EG_Coord_t Width = pContent->GetContentWidth();
		if(GetStyleBaseDirection(EG_PART_MAIN) != EG_BASE_DIR_RTL) {
			pContent->ScrollToX(Index * (Gap + Width), Enable);
		}
		else {
			int32_t ReverseIndex = -(int32_t)Index;
			pContent->ScrollToX((Gap + Width) * ReverseIndex, Enable);
		}
	}
	else {
		EG_Coord_t Gap = pContent->GetStylePadRow(EG_PART_MAIN);
		EG_Coord_t Height = pContent->GetContentHeight();
		pContent->ScrollToY(Index * (Gap + Height), Enable);
	}

	EGButtonMatrix *pButtons = GetTabButtons();
	pButtons->SetControl(Index, EG_BTNMATRIX_CTRL_CHECKED);
	m_CurrentTab= Index;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTabView::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(TABVIEW_CLASS) != EG_RES_OK) return;   // Call the ancestor's event handler
	EGTabView *pTabView = (EGTabView*)pEvent->GetTarget();
	if(pEvent->GetCode() == EG_EVENT_SIZE_CHANGED) {
		pTabView->SetActive(pTabView->GetActive(), EG_ANIM_OFF);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTabView::ButtonEventCB(EGEvent *pEvent)
{
	EGButtonMatrix *pButtons = (EGButtonMatrix*)pEvent->GetTarget();
	EGTabView *pTabView = (EGTabView*)pButtons->GetParent();
	uint32_t Index = pButtons->GetSelected();
	pTabView->SetActive(Index, EG_ANIM_OFF);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTabView::ContentEventCB(EGEvent *pEvent)
{
	EGObject *pContent = (EGObject*)pEvent->GetTarget();
  EG_EventCode_e Code = pEvent->GetCode();
	EGTabView *pTabView = (EGTabView*)pContent->GetParent();
	if(Code == EG_EVENT_LAYOUT_CHANGED) {
		pTabView->SetActive(pTabView->GetActive(), EG_ANIM_OFF);
    return;
	}
	if(Code == EG_EVENT_SCROLL_END) {
    EGInputDevice *pInput = EGInputDevice::GetActive();
		if(pInput && pInput->m_Process.State == EG_INDEV_STATE_PRESSED) {
			return;
		}
		EGPoint Point;
		pContent->GetScrollEnd(&Point);
		EG_Coord_t Tab;
		if((pTabView->m_TabPosition & EG_DIR_VER) != 0) {
			EG_Coord_t Width = pContent->GetContentWidth();
			if(pTabView->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) Tab = -(Point.m_X - Width / 2) / Width;
			else Tab = (Point.m_X + Width / 2) / Width;
		}
		else {
			EG_Coord_t Height = pContent->GetContentHeight();
			Tab = (Point.m_Y + Height / 2) / Height;
		}
		if(Tab < 0) Tab = 0;
		bool NewTab = false;
		if(Tab != pTabView->GetActive()) NewTab = true;
		pTabView->SetActive(Tab, EG_ANIM_ON);
		if(NewTab) EGEvent::EventSend(pTabView, EG_EVENT_VALUE_CHANGED, NULL);
	}
}
#endif
