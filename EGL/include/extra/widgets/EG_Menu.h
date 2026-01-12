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

#if EG_USE_MENU

///////////////////////////////////////////////////////////////////////////////////////

class EGMenuPage;
class EGButton;
class EGLabel;

typedef enum {
    EG_MENU_HEADER_TOP_FIXED, //  Header is positioned at the top 
    EG_MENU_HEADER_TOP_UNFIXED, //  Header is positioned at the top and can be scrolled out of view
    EG_MENU_HEADER_BOTTOM_FIXED //  Header is positioned at the bottom 
} EG_MenuHeaderMode_e;

typedef enum {
    EG_MENU_ROOT_BACK_BTN_DISABLED,
    EG_MENU_ROOT_BACK_BTN_ENABLED
} EG_MenuRootButtonMode_e;

typedef struct EG_PageEventData_t {
    EGObject    *pMenu;
    EGMenuPage  *pPage;
} EG_PageEventData_t;

extern const EG_ClassType_t c_MenuClass;
extern const EG_ClassType_t c_MenuPageClass;
extern const EG_ClassType_t c_MenuContainerClass;
extern const EG_ClassType_t c_MenuSectionClass;
extern const EG_ClassType_t c_MenuSeparatorClass;
extern const EG_ClassType_t c_MenuSidebarContainerClass;
extern const EG_ClassType_t c_MenuMainContainerClass;
extern const EG_ClassType_t c_MenuSidebarHeaderContainerClass;
extern const EG_ClassType_t c_MenuMainHeaderContainerClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGMenuPage : public EGObject
{
public:
                      EGMenuPage(void) : EGObject(), m_pTitle(nullptr){};
                      EGMenuPage(EGObject *pParent, char *pTitle);
  virtual             ~EGMenuPage(void);
  virtual void        Configure(void);

  char                *m_pTitle;
};

///////////////////////////////////////////////////////////////////////////////////////

class EGMenu : public EGObject
{
public:
                      EGMenu(void);
                      EGMenu(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_MenuClass);
  virtual             ~EGMenu(void);
  void                Configure(void);
  EGObject*           CreateContainer(EGObject *pParent);
  EGObject*           CreateSection(EGObject *pParent);
  EGObject*           CreateSeparator(EGObject *pParent);
  void                SetPage(EGMenuPage *pPage);
  void                SetSidebarPage(EGMenuPage *pPage);
  void                SetModeHeader(EG_MenuHeaderMode_e mode_header);
  void                SetModeRootButton(EG_MenuRootButtonMode_e mode_root_back_btn);
  void                SetLoadPageEvent(EGObject *pContainer, EGMenuPage *pPage);
  inline EGMenuPage*  GetCurrentMainPage(void){ return m_pMainPage; };
  inline EGMenuPage*  GetCurrentSidebarPage(void){ return m_pSidebarPage; };
  inline EGObject*    GetMainHeader(void){ return m_pMainHeader; };
  inline EGButton*    GetMainHeaderButton(void){ return m_pMainHeaderButton; };
  inline EGObject*    GetSidebarHeader(void){ return m_pSidebarHeader; };
  inline EGButton*    GetSidebarHeaderButton(void){ return m_pSidebarHeaderButton; };

  static bool         BackButtonIsRoot(EGMenu *pMenu, EGButton *pButton);

  static void         LoadPageEventCB(EGEvent *pEvent);
  static void         DeleteEventCB(EGEvent *pEvent);
  static void         BackEventCB(EGEvent *pEvent);
  static void         ValueChangedEventCB(EGEvent *pEvent);

  EGObject                 *m_pStorage; //  a pointer to obj that is the parent of all pages not displayed 
  EGObject                 *m_pMain;
  EGMenuPage               *m_pMainPage;
  EGObject                 *m_pMainHeader;
  EGButton                 *m_pMainHeaderButton; //  a pointer to obj that on click triggers back btn event handler, can be same as 'main_header' 
  EGLabel                  *m_pMainHeaderTitle;
  EGObject                 *m_pSidebar;
  EGMenuPage               *m_pSidebarPage;
  EGObject                 *m_pSidebarHeader;
  EGButton                 *m_pSidebarHeaderButton; //  a pointer to obj that on click triggers back btn event handler, can be same as 'sidebar_header' 
  EGLabel                  *m_pSidebarHeaderTitle;
  EGObject                 *m_pSelectedTab;
  EGList                    m_History;
  uint8_t                   m_CurrentDepth;
  uint8_t                   m_PreviousDepth;
  uint8_t                   m_SidebarActive : 1;
  EG_MenuHeaderMode_e       m_ModeHeader : 2;
  EG_MenuRootButtonMode_e   m_RootBackButtonMode : 1;

private:
  void                Refresh(void);
  void                RefreshSidebarHeaderMode(void);
  void                RefreshMainHeaderMode(void);
  void                ClearHistory(void);

};

///////////////////////////////////////////////////////////////////////////////////////

#endif 