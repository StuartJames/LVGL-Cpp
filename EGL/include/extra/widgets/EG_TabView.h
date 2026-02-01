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

#pragma once

#include "EGL.h"

#if EG_USE_TABVIEW

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_TabViewClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGTabView : public EGObject
{
public:
                    EGTabView(void);
                    EGTabView(EGObject *pParent, EG_DirType_e TabPos, EG_Coord_t TabSize, 
                      const EG_ClassType_t *pClassCnfg = &c_TabViewClass);
                    ~EGTabView(void);
  virtual void      Configure(void);

  EGObject*         AddTab(const char *pName);
  void              RenameTab(uint32_t Index, const char *pName);
  EGObject*         GetContent(void){ return GetChild(1); };
  EGButtonMatrix*   GetTabButtons(void){ return (EGButtonMatrix*)GetChild(0); };
  void              SetActive(uint32_t Index, EG_AnimateEnable_e Enable);
  uint16_t          GetActive(void){ return m_CurrentTab; };

  static void       EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);
  static void       ButtonEventCB(EGEvent *pEvent);
  static void       ContentEventCB(EGEvent *pEvent);

  const char      **m_ppMap;
  uint16_t          m_TabCount;
  uint16_t          m_CurrentTab;
  EG_DirType_e      m_TabPosition;

private:
  static EG_DirType_e m_TabPosCreate;
  static EG_Coord_t   m_TabSizeCreate;


};


#endif 
