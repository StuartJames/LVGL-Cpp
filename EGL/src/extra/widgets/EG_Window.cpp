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
 *  Based on Rect design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.Rect.1    Original by LVGL Kft
 *
 */

#include "extra/widgets/EG_Window.h"

#if EG_USE_WIN

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_WindowClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = _EG_PCT(100),
	.HeightDef = _EG_PCT(100),
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGWindow::EGWindow(EGObject *pParent, EG_Coord_t HeaderHeight) : EGObject()
{
	create_header_height = HeaderHeight;
  Attach(this, pParent, &c_WindowClass);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGWindow::Configure(void)
{
  EGObject::Configure();
	EGObject *pParent = GetParent();
	SetSize(pParent->GetWidth(), pParent->GetHeight());
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_COLUMN);
	EGObject *pHeader = new EGObject(this);
	pHeader->SetSize(_EG_PCT(100), create_header_height);
	EGFlexLayout::SetObjFlow(pHeader, EG_FLEX_FLOW_ROW);
	EGFlexLayout::SetObjAlign(pHeader, EG_FLEX_ALIGN_START, EG_FLEX_ALIGN_CENTER, EG_FLEX_ALIGN_CENTER);
	EGObject *pContainer = new EGObject(this);
	EGFlexLayout::SetObjGrow(pContainer, 1);
	pContainer->SetWidth(_EG_PCT(100));
}

///////////////////////////////////////////////////////////////////////////////////////

EGLabel* EGWindow::AddTitle(const char *pText)
{
	EGObject *pHeader = GetHeader();
	EGLabel *pLabel = new EGLabel(pHeader);
	pLabel->SetLongMode(EG_LABEL_LONG_DOT);
	pLabel->SetText(pText);
	EGFlexLayout::SetObjGrow(pLabel, 1);
	return pLabel;
}

///////////////////////////////////////////////////////////////////////////////////////

EGButton* EGWindow::AddButton(const void *pIcon, EG_Coord_t Width)
{
	EGObject *pHeader = GetHeader();
	EGButton *pButton = new EGButton(pHeader);
	pButton->SetSize(Width, _EG_PCT(100));
	EGImage *pImage = new EGImage(pButton);
	pImage->SetSource(pIcon);
	pImage->Align(EG_ALIGN_CENTER, 0, 0);
	return pButton;
}

#endif
