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

#include "extra/widgets/EG_ListCtrl.h"
#include "core/EG_Display.h"
#include "widgets/EG_Label.h"
#include "widgets/EG_Image.h"
#include "widgets/EG_Button.h"

#if EG_USE_LIST

///////////////////////////////////////////////////////////////////////////////////////

#define LISTCTRL_CLASS &c_ListClass

const EG_ClassType_t c_ListClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = (EG_DPI_DEF * 3) / 2,
	.HeightDef = EG_DPI_DEF * 2,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_ListButtonClass = {
  .pBaseClassType = &c_ButtonClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_ListTextClass = {
  .pBaseClassType = &c_LabelClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGListCtrl::EGListCtrl(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= LISTCTRL_CLASS*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_COLUMN);
}

///////////////////////////////////////////////////////////////////////////////////////

EGLabel* EGListCtrl::AddText(const char *pText)
{
  EGLabel *pLabel = new EGLabel(this, &c_ListTextClass);        
	pLabel->SetText(pText);
	pLabel->SetLongMode(EG_LABEL_LONG_SCROLL_CIRCULAR);
	pLabel->SetWidth(_EG_PCT(100));
	return pLabel;
}

///////////////////////////////////////////////////////////////////////////////////////

EGButton* EGListCtrl::AddButton(const void *pIcon, const char *pText)
{
	EGButton *pButton = new EGButton(this, &c_ListButtonClass);
	pButton->SetSize(_EG_PCT(100), EG_SIZE_CONTENT);
	EGFlexLayout::SetObjFlow(pButton, EG_FLEX_FLOW_ROW);
#if EG_USE_IMG == 1
	if(pIcon) {
		EGImage *pImage = new EGImage(pButton);
		pImage->SetSource(pIcon);
	}
#endif
	if(pText) {
    EGLabel *pLabel = new EGLabel(this);        
		pLabel->SetText(pText);
	  pLabel->SetLongMode(EG_LABEL_LONG_SCROLL_CIRCULAR);
    EGFlexLayout::SetObjGrow(pLabel, 1);
	}
	return pButton;
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGListCtrl::GetButtonText(EGObject *pButton)
{
	for(uint32_t i = 0; i < pButton->GetChildCount(); i++) {
		EGObject *pChild = pButton->GetChild(i);
		if(EGObject::IsKindOf(pChild, &c_LabelClass)) {
			return ((EGLabel*)pChild)->GetText();
		}
	}
	return "";
}

#endif
