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

#include "extra/widgets/EG_Menu.h"

#if EG_USE_MENU

#define MENU_CLASS &c_MenuClass

#include "core/EG_Object.h"
#include "extra/layouts/EG_Flex.h"
#include "widgets/EG_Label.h"
#include "widgets/EG_Button.h"
#include "widgets/EG_Image.h"

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_MenuClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = (EG_DPI_DEF * 3) / 2,
	.HeightDef = EG_DPI_DEF * 2,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_MenuPageClass = {
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

const EG_ClassType_t c_MenuContainerClass = {
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

const EG_ClassType_t c_MenuSectionClass = {
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

const EG_ClassType_t c_MenuSeparatorClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_MenuSidebarContainerClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_MenuMainContainerClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_MenuMainHeaderContainerClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_MenuSidebarHeaderContainerClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGMenuPage::EGMenuPage(EGObject *pParent, char *pTitle) : EGObject()
{
  Attach(this, pParent, &c_MenuPageClass);
	Initialise();
	if(pTitle) {
		m_pTitle = (char*)EG_AllocMem(strlen(pTitle) + 1);
		EG_ASSERT_MALLOC(m_pTitle);
		if(m_pTitle == nullptr) return;
		strcpy(m_pTitle, pTitle);
	}
	else {
		m_pTitle = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EGMenuPage::~EGMenuPage(void)
{
	if(m_pTitle != NULL) {
		EG_FreeMem(m_pTitle);
		m_pTitle = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenuPage::Configure(void)
{
  EGObject::Configure();
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_COLUMN);
	EGFlexLayout::SetObjAlign(this, EG_FLEX_ALIGN_START, EG_FLEX_ALIGN_CENTER, EG_FLEX_ALIGN_CENTER);
	AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
}

///////////////////////////////////////////////////////////////////////////////////////

EGMenu::EGMenu(void) : EGObject(),
  m_CurrentDepth(0),
  m_PreviousDepth(0),
  m_SidebarActive(0),
  m_ModeHeader(EG_MENU_HEADER_TOP_FIXED),
  m_RootBackButtonMode(EG_MENU_ROOT_BACK_BTN_DISABLED)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGMenu::EGMenu(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_MenuClass*/) : EGObject(),
  m_CurrentDepth(0),
  m_PreviousDepth(0),
  m_SidebarActive(0),
  m_ModeHeader(EG_MENU_HEADER_TOP_FIXED),
  m_RootBackButtonMode(EG_MENU_ROOT_BACK_BTN_DISABLED)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGMenu::~EGMenu(void)
{
	m_History.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////////////


void EGMenu::Configure(void)
{
	SetLayout(EGFlexLayout::m_Reference);
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_ROW);
	m_ModeHeader = EG_MENU_HEADER_TOP_FIXED;
	m_RootBackButtonMode = EG_MENU_ROOT_BACK_BTN_DISABLED;
	m_CurrentDepth = 0;
	m_PreviousDepth = 0;
	m_SidebarActive = false;
	m_pStorage = new EGObject(this);
	m_pStorage->AddFlag(EG_OBJ_FLAG_HIDDEN);
	m_pSidebar = NULL;
	m_pSidebarHeader = NULL;
	m_pSidebarHeaderButton = NULL;
	m_pSidebarHeaderTitle = NULL;
	m_pSidebarPage = NULL;

	EGObject *pContainer = new EGObject(this, &c_MenuMainContainerClass);
	pContainer->SetHeight(_EG_PCT(100));
	EGFlexLayout::SetObjGrow(pContainer, 1);
	EGFlexLayout::SetObjFlow(pContainer, EG_FLEX_FLOW_COLUMN);
	pContainer->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
	pContainer->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	m_pMain = pContainer;

	EGObject *pHeader = new EGObject(pContainer, &c_MenuMainHeaderContainerClass);
	pHeader->SetSize(_EG_PCT(100), EG_SIZE_CONTENT);
	EGFlexLayout::SetObjFlow(pHeader, EG_FLEX_FLOW_ROW);
	EGFlexLayout::SetObjAlign(pHeader, EG_FLEX_ALIGN_START, EG_FLEX_ALIGN_CENTER, EG_FLEX_ALIGN_CENTER);
	pHeader->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	pHeader->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
	m_pMainHeader = pHeader;

	//  Create the default simple back btn and title 
	EGButton *pButton = new EGButton(pHeader);
	EGEvent::AddEventCB(pButton, EGMenu::BackEventCB, EG_EVENT_CLICKED, this);
	pButton->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
	EGFlexLayout::SetObjFlow(pButton, EG_FLEX_FLOW_ROW);
	m_pMainHeaderButton = pButton;

	EGImage *pImage = new EGImage(m_pMainHeaderButton);
	pImage->SetSource(EG_SYMBOL_LEFT);

	EGLabel *pLabel = new EGLabel(pHeader);
	pLabel->AddFlag(EG_OBJ_FLAG_HIDDEN);
	m_pMainHeaderTitle = pLabel;

	m_pMainPage = NULL;
	m_pSelectedTab = NULL;

	EGEvent::AddEventCB(this, EGMenu::ValueChangedEventCB, EG_EVENT_VALUE_CHANGED, this);

	EG_TRACE_OBJ_CREATE("finished");
}

///////////////////////////////////////////////////////////////////////////////////////

EGObject* EGMenu::CreateContainer(EGObject *pParent)
{
	EGObject *pContainer = new EGObject(pParent, &c_MenuContainerClass);
	EGFlexLayout::SetObjFlow(pContainer, EG_FLEX_FLOW_ROW);
	EGFlexLayout::SetObjAlign(pContainer, EG_FLEX_ALIGN_START, EG_FLEX_ALIGN_CENTER, EG_FLEX_ALIGN_CENTER);
	pContainer->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	return pContainer;
}

///////////////////////////////////////////////////////////////////////////////////////

EGObject* EGMenu::CreateSection(EGObject *pParent)
{
	EGObject *pSection = new EGObject(pParent, &c_MenuSectionClass);
	EGFlexLayout::SetObjFlow(pSection, EG_FLEX_FLOW_COLUMN);
	pSection->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	return pSection;
}

///////////////////////////////////////////////////////////////////////////////////////

EGObject* EGMenu::CreateSeparator(EGObject *pParent)
{
	EGObject *pSeperator = new EGObject(pParent, &c_MenuSectionClass);
	return pSeperator;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::Refresh(void)
{
EGMenuPage *pPage = NULL;

	EGMenuPage *pActivePage = (EGMenuPage*)m_History.GetHead();	//  The current menu 
	if(pActivePage != NULL) {
		pPage = pActivePage;
		m_History.RemoveHead();		//  Delete the current item from the history 
		delete pActivePage;
		m_CurrentDepth--;
	}
	SetPage(pPage);	//  Set it 
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::SetPage(EGMenuPage *pPage)
{
	if(m_pMainPage == pPage) return;	//  Guard against setting the same page again 
	if(m_pMainPage != NULL) {	//  Hide previous page 
		m_pMainPage->SetParent(m_pStorage);
	}
	if(pPage != NULL) {
		m_History.AddHead(pPage);
		m_CurrentDepth++;
		pPage->SetParent(m_pMain);		//  Place page in main 
	}
	else {
		m_History.RemoveAll();		//  Empty page, clear history 
	}
	m_pMainPage = pPage;
	if(m_pSelectedTab != NULL) {	//  If there is a selected tab, update checked state 
		if(m_pSidebarPage != NULL) {
			m_pSelectedTab->AddState(EG_STATE_CHECKED);
		}
		else {
			m_pSelectedTab->ClearState(EG_STATE_CHECKED);
		}
	}
	if(m_pSidebarPage != NULL) {	//  Back btn management 
		if(m_SidebarActive) {		//  With sidebar enabled 
			if(m_RootBackButtonMode == EG_MENU_ROOT_BACK_BTN_ENABLED) {
				m_pSidebarHeaderButton->ClearFlag(EG_OBJ_FLAG_HIDDEN);	//  Root back btn is always shown if enabled
				m_pSidebarHeaderButton->AddFlag(EG_OBJ_FLAG_CLICKABLE);
			}
			else {
				m_pSidebarHeaderButton->AddFlag(EG_OBJ_FLAG_HIDDEN);
				m_pSidebarHeaderButton->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
			}
		}
		if(m_CurrentDepth >= 2) {
			m_pMainHeaderButton->ClearFlag(EG_OBJ_FLAG_HIDDEN);
			m_pMainHeaderButton->AddFlag(EG_OBJ_FLAG_CLICKABLE);
		}
		else {
			m_pMainHeaderButton->AddFlag(EG_OBJ_FLAG_HIDDEN);
			m_pMainHeaderButton->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
		}
	}
	else {		//  With sidebar disabled 
		if(m_CurrentDepth >= 2 || m_RootBackButtonMode == EG_MENU_ROOT_BACK_BTN_ENABLED) {
			m_pMainHeaderButton->ClearFlag(EG_OBJ_FLAG_HIDDEN);
			m_pMainHeaderButton->AddFlag(EG_OBJ_FLAG_CLICKABLE);
		}
		else {
			m_pMainHeaderButton->AddFlag(EG_OBJ_FLAG_HIDDEN);
			m_pMainHeaderButton->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
		}
	}
	EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, NULL);
	RefreshMainHeaderMode();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::SetSidebarPage(EGMenuPage *pPage)
{
	//  Sidebar management
	if(pPage != NULL) {		//  Sidebar should be enabled 
		if(!m_SidebarActive) {			//  Create sidebar 
      EGObject *pSidebarContainer = new EGObject(this, &c_MenuSidebarContainerClass);
			pSidebarContainer->MoveToIndex(1);
			pSidebarContainer->SetSize(_EG_PCT(30), _EG_PCT(100));
			EGFlexLayout::SetObjFlow(pSidebarContainer, EG_FLEX_FLOW_COLUMN);
			pSidebarContainer->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
			pSidebarContainer->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
			m_pSidebar = pSidebarContainer;

			EGObject *pHeader = new EGObject(pSidebarContainer, &c_MenuSidebarHeaderContainerClass);
			pHeader->SetSize(_EG_PCT(100), EG_SIZE_CONTENT);
			EGFlexLayout::SetObjFlow(pHeader, EG_FLEX_FLOW_ROW);
			EGFlexLayout::SetObjAlign(pHeader, EG_FLEX_ALIGN_START, EG_FLEX_ALIGN_CENTER, EG_FLEX_ALIGN_CENTER);
			pHeader->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
			pHeader->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
			m_pSidebarHeader = pHeader;

			EGButton *pButton = new EGButton(m_pSidebarHeader);
			EGEvent::AddEventCB(pButton, EGMenu::BackEventCB, EG_EVENT_CLICKED, this);
			pButton->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE);
			EGFlexLayout::SetObjFlow(pButton, EG_FLEX_FLOW_ROW);
			m_pSidebarHeaderButton = pButton;

			EGImage *pImage = new EGImage(m_pSidebarHeaderButton);
			pImage->SetSource(EG_SYMBOL_LEFT);

			EGLabel *pLabel = new EGLabel(m_pSidebarHeader);
			pLabel->AddFlag(EG_OBJ_FLAG_HIDDEN);
			m_pSidebarHeaderTitle = pLabel;

			m_SidebarActive = true;
		}
		pPage->SetParent(m_pSidebar);
		RefreshSidebarHeaderMode();
	}
	else {
		//  Sidebar should be disabled 
		if(m_SidebarActive) {
			m_pSidebarPage->SetParent(m_pStorage);
			EGObject::Delete(m_pSidebar);
			m_SidebarActive = false;
		}
	}
	m_pSidebarPage = pPage;
	Refresh();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::SetModeHeader(EG_MenuHeaderMode_e HeaderMode)
{
	if(m_ModeHeader != HeaderMode) {
		m_ModeHeader = HeaderMode;
		RefreshMainHeaderMode();
		if(m_SidebarActive) RefreshSidebarHeaderMode();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::SetModeRootButton(EG_MenuRootButtonMode_e ButtonMode)
{
	if(m_RootBackButtonMode != ButtonMode) {
		m_RootBackButtonMode = ButtonMode;
		Refresh();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::SetLoadPageEvent(EGObject *pContainer, EGMenuPage *pPage)
{
	pContainer->AddFlag(EG_OBJ_FLAG_CLICKABLE);
	pContainer->ClearFlag(EG_OBJ_FLAG_SCROLLABLE);
	pContainer->AddFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
	//  Remove old event 
	if(EGEvent::RemoveEventCB(pContainer, LoadPageEventCB)) {
		EGEvent::EventSend(pContainer, EG_EVENT_DELETE, NULL);
		EGEvent::RemoveEventCB(pContainer, DeleteEventCB);
	}
	EG_PageEventData_t *pEventData = (EG_PageEventData_t*)EG_AllocMem(sizeof(EG_PageEventData_t));
	pEventData->pMenu = this;
	pEventData->pPage = pPage;
	EGEvent::AddEventCB(pContainer, LoadPageEventCB, EG_EVENT_CLICKED, pEventData);
	EGEvent::AddEventCB(pContainer, DeleteEventCB, EG_EVENT_DELETE, pEventData);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGMenu::BackButtonIsRoot(EGMenu *pMenu, EGButton *pButton)
{
	if(pButton == pMenu->m_pSidebarHeaderButton) return true;
	if((pButton == pMenu->m_pMainHeaderButton) && (pMenu->m_PreviousDepth <= 1)) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::ClearHistory(void)
{
  m_History.RemoveAll();
	m_CurrentDepth = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::RefreshSidebarHeaderMode(void)
{
	if(m_pSidebarHeader == NULL || m_pSidebarPage == NULL) return;
	switch(m_ModeHeader) {
		case EG_MENU_HEADER_TOP_FIXED:
			//  Content should fill the remaining space 
			m_pSidebarHeader->MoveToIndex(0);
			EGFlexLayout::SetObjGrow(m_pSidebarPage, 1);
			break;
		case EG_MENU_HEADER_TOP_UNFIXED:
			m_pSidebarHeader->MoveToIndex(0);
			EGFlexLayout::SetObjGrow(m_pSidebarPage, 0);
			break;
		case EG_MENU_HEADER_BOTTOM_FIXED:
			m_pSidebarHeader->MoveToIndex(1);
			EGFlexLayout::SetObjGrow(m_pSidebarPage, 1);
			break;
	}
	m_pSidebarHeader->RefreshSize();
	m_pSidebarPage->RefreshSize();
	if(m_pSidebarHeader->GetContentHeight() == 0) m_pSidebarHeader->AddFlag(EG_OBJ_FLAG_HIDDEN);
	else m_pSidebarHeader->ClearFlag(EG_OBJ_FLAG_HIDDEN);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::RefreshMainHeaderMode(void)
{
	if(m_pMainHeader == NULL || m_pMainPage == NULL) return;
	switch(m_ModeHeader) {
		case EG_MENU_HEADER_TOP_FIXED:
			//  Content should fill the remaining space 
			m_pMainHeader->MoveToIndex(0);
			EGFlexLayout::SetObjGrow(m_pMainPage, 1);
			break;
		case EG_MENU_HEADER_TOP_UNFIXED:
			m_pMainHeader->MoveToIndex(0);
			EGFlexLayout::SetObjGrow(m_pMainPage, 0);
			break;
		case EG_MENU_HEADER_BOTTOM_FIXED:
			m_pMainHeader->MoveToIndex(1);
			EGFlexLayout::SetObjGrow(m_pMainPage, 1);
			break;
	}
	m_pMainHeader->RefreshSize();
	m_pMainPage->RefreshSize();
	m_pMainHeader->UpdateLayout();
	if(m_pMainHeader->GetContentHeight() == 0) m_pMainHeader->AddFlag(EG_OBJ_FLAG_HIDDEN);
	else m_pMainHeader->ClearFlag(EG_OBJ_FLAG_HIDDEN);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::LoadPageEventCB(EGEvent *pEvent)
{
	EGMenu *pContainer = (EGMenu*)pEvent->GetTarget();
	EG_PageEventData_t *pEventData = (EG_PageEventData_t*)pEvent->GetExtParam();
	EGMenu *pMenu = (EGMenu*)(pEventData->pMenu);
	EGMenuPage *pPage = pEventData->pPage;
	if(pMenu->m_pSidebarPage != NULL) {
		//  Check if clicked obj is in the sidebar 
		bool IsSidebar = false;
		EGObject *pParent = pContainer;
		while(pParent) {
			if(pParent == (EGObject *)pMenu) break;
			if(pParent == pMenu->m_pSidebar) {
				IsSidebar = true;
				break;
			}
			pParent = pParent->GetParent();
		}
		if(IsSidebar) {			//  Clear checked state of previous obj 
			if(pMenu->m_pSelectedTab != pContainer && pMenu->m_pSelectedTab != NULL) {
				pMenu->m_pSelectedTab->ClearState(EG_STATE_CHECKED);
			}
			pMenu->m_History.RemoveAll();
			pMenu->m_pSelectedTab = pContainer;
		}
	}
	pMenu->SetPage(pPage);
	if(EGGroup::GetDefault() != NULL && pMenu->m_pSidebarPage == NULL) {
		EGGroup::GetDefault()->FocusNext();		//  Sidebar is not supported for now
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::DeleteEventCB(EGEvent *pEvent)
{
	EG_PageEventData_t *pEventData = (EG_PageEventData_t*)pEvent->GetExtParam();
	EG_FreeMem(pEventData);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::BackEventCB(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_CLICKED) {
		EGButton *pButton = (EGButton*)pEvent->GetTarget();
		EGMenu *pMenu = (EGMenu*)pEvent->GetExtParam();
		if(!(pButton == pMenu->m_pMainHeaderButton || pButton == pMenu->m_pSidebarHeaderButton)) return;
		pMenu->m_PreviousDepth = pMenu->m_CurrentDepth; //  Save the previous value for user event handler 
		if(BackButtonIsRoot(pMenu, pButton)) return;
    POSITION Pos = 0;
  	EGMenuPage *pActivePage = (EGMenuPage*)pMenu->m_History.GetHead(Pos);		//  The current menu 
		EGMenuPage *pPreviousPage = (EGMenuPage*)pMenu->m_History.GetNext(Pos);		//  The previous menu 
		if(pPreviousPage != NULL) {
			//  Previous menu exists. Delete the current item from the history 
      pMenu->m_History.RemoveHead();
			delete pActivePage;
			pMenu->m_CurrentDepth--;
			//  Create the previous menu. Remove it from the history because `lv_menu_set_page` will add it again 
      pMenu->m_History.RemoveHead();
			pMenu->m_CurrentDepth--;
			pMenu->SetPage(pPreviousPage);
			EG_FreeMem(pPreviousPage);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMenu::ValueChangedEventCB(EGEvent *pEvent)
{
	EGMenu *pMenu = (EGMenu*)pEvent->GetExtParam();
	EGMenuPage *pMainPage = (EGMenuPage*)pMenu->GetCurrentMainPage();
	if(pMainPage != NULL && pMenu->m_pMainHeaderTitle != NULL) {
		if(pMainPage->m_pTitle != NULL) {
			pMenu->m_pMainHeaderTitle->SetText(pMainPage->m_pTitle);
			pMenu->m_pMainHeaderTitle->ClearFlag(EG_OBJ_FLAG_HIDDEN);
		}
		else {
			pMenu->m_pMainHeaderTitle->AddFlag(EG_OBJ_FLAG_HIDDEN);
		}
	}
	EGMenuPage *pSidebarPage = (EGMenuPage*)pMenu->GetCurrentSidebarPage();
	if(pSidebarPage != NULL && pMenu->m_pSidebarHeaderTitle != NULL) {
		if(pSidebarPage->m_pTitle != NULL) {
			pMenu->m_pSidebarHeaderTitle->SetText(pSidebarPage->m_pTitle);
			pMenu->m_pSidebarHeaderTitle->ClearFlag(EG_OBJ_FLAG_HIDDEN);
		}
		else {
			pMenu->m_pSidebarHeaderTitle->AddFlag(EG_OBJ_FLAG_HIDDEN);
		}
	}
}
#endif
