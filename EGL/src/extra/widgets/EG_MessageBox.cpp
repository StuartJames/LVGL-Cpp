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

#include "extra/widgets/EG_MessageBox.h"
#if EG_USE_MSGBOX

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

#define LV_MSGBOX_FLAG_AUTO_PARENT EG_OBJ_FLAG_WIDGET_1 // Mark that the pParent was automatically created
#define MSGBOX_CLASS &c_MsgBoxClass

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_MsgBoxClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = EG_DPI_DEF * 2,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_MsgBoxContentClass = {
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

const EG_ClassType_t c_MsgBoxBackdropClass = {
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

EGMessageBox::EGMessageBox(void) : EGObject(),
  m_pTitle(nullptr),
  m_pCloseButton(nullptr),
  m_pContent(nullptr),
  m_pText(nullptr),
  m_pButtons(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGMessageBox::EGMessageBox(EGObject *pParent, const char *pTitle, const char *pText, const char *pButtonTexts[],
                       bool HasCloseButton, const EG_ClassType_t *pClassCnfg /*= &c_MsgBoxClass*/) : EGObject(),
  m_pTitle(nullptr),
  m_pCloseButton(nullptr),
  m_pContent(nullptr),
  m_pText(nullptr),
  m_pButtons(nullptr)
{
bool AutoParent = false;

	if(pParent == nullptr) {
		AutoParent = true;
		pParent = new EGObject(EGTopLayer(), &c_MsgBoxBackdropClass);
		pParent->ClearFlag(EG_OBJ_FLAG_IGNORE_LAYOUT);
		pParent->SetSize(_EG_PCT(100), _EG_PCT(100));
	}
	if(AutoParent) AddFlag(LV_MSGBOX_FLAG_AUTO_PARENT);
	bool HasTitle = pTitle && strlen(pTitle) > 0;
	// When a close button is required, we need the empty pLabel as spacer to push the button to the right
	if(HasCloseButton || HasTitle) {
		m_pTitle = new EGLabel(this);
		m_pTitle->SetText(HasTitle ? pTitle : "");
		m_pTitle->SetLongMode(EG_LABEL_LONG_SCROLL_CIRCULAR);
		if(HasCloseButton)	EGFlexLayout::SetObjGrow(m_pTitle, 1);
		else m_pTitle->SetWidth(_EG_PCT(100));
	}
	if(HasCloseButton) {
		m_pCloseButton = new EGButton(this);
		m_pCloseButton->SetExtClickArea(EG_DPX(10));
		EGEvent::AddEventCB(m_pCloseButton, EGMessageBox::ClickEventCB, EG_EVENT_CLICKED, nullptr);
		EGLabel *pLabel = new EGLabel(m_pCloseButton);
		pLabel->SetText(EG_SYMBOL_CLOSE);
		const EG_Font_t *pFont = m_pCloseButton->GetStyleTextFont(EG_PART_MAIN);
		EG_Coord_t CloseSize = EG_FontGetLineHeight(pFont) + EG_DPX(10);
		m_pCloseButton->SetSize(CloseSize, CloseSize);
		pLabel->Align(EG_ALIGN_CENTER, 0, 0);
	}
	m_pContent = new EGObject(this, &c_MsgBoxContentClass);
	if(m_pContent == NULL) return;
	bool HasText = pText && strlen(pText) > 0;
	if(HasText) {
		m_pText = new EGLabel(m_pContent);
		m_pText->SetText(pText);
		m_pText->SetLongMode(EG_LABEL_LONG_WRAP);
		m_pText->SetWidth(EG_PCT(100));
	}
	if(pButtonTexts) {
		m_pButtons = new EGButtonMatrix(this);
		m_pButtons->SetMap(pButtonTexts);
		m_pButtons->SetControlAll(EG_BTNMATRIX_CTRL_CLICK_TRIG | EG_BTNMATRIX_CTRL_NO_REPEAT);
		uint32_t ButtonCount = 0;
		while(pButtonTexts[ButtonCount] && pButtonTexts[ButtonCount][0] != '\0') ButtonCount++;
		const EG_Font_t *font = m_pButtons->GetStyleTextFont(EG_PART_ITEMS);
		EG_Coord_t btn_h = EG_FontGetLineHeight(font) + EG_DPI_DEF / 10;
		m_pButtons->SetSize(ButtonCount * (2 * EG_DPI_DEF / 3), btn_h);
		m_pButtons->SetStyleMaxWidth(EG_PCT(100), 0);
		m_pButtons->AddFlag(EG_OBJ_FLAG_EVENT_BUBBLE); // To see the event directly on the message box
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMessageBox::Configure(void)
{
  EGObject::Configure();
	EGFlexLayout::SetObjFlow(this, EG_FLEX_FLOW_ROW_WRAP);
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGMessageBox::GetActiveButtonText(void)
{
	return m_pButtons->GetButtonText(m_pButtons->GetSelected());
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMessageBox::Close(void)
{
	if(HasFlagSet(LV_MSGBOX_FLAG_AUTO_PARENT))	EGObject::Delete(GetParent());
	else EGObject::Delete(this);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMessageBox::CloseAsync(void)
{
	if(HasFlagSet(LV_MSGBOX_FLAG_AUTO_PARENT))	DeleteAsync(GetParent());
	else DeleteAsync(this);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGMessageBox::ClickEventCB(EGEvent *pEvent)
{
	EGButton *pButton = (EGButton*)pEvent->GetTarget();
	EGMessageBox *pMsgBox = (EGMessageBox*)pButton->GetParent();
	pMsgBox->Close();
}

#endif 
