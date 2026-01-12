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

#include "core/EG_Object.h"
#include "widgets/EG_DropDown.h"

#if EG_USE_DROPDOWN != 0

#include "misc/EG_Assert.h"
#include "draw/EG_DrawContext.h"
#include "core/EG_Group.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Display.h"
#include "font/EG_SymbolDef.h"
#include "misc/EG_Animate.h"
#include "misc/EG_Math.h"
#include "misc/lv_txt_ap.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////

#define DROP_CLASS &c_DropDownClass
#define DROPLIST_CLASS &c_DropDownListClass
#define EG_DROPDOWN_PR_NONE 0xFFFF

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_DropDownClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGDropDown::EventCB,
	.WidthDef = EG_DPI_DEF,
	.HeightDef = EG_SIZE_CONTENT,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
  .pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGDropDown::EGDropDown(void) : EGObject(),
	m_pDropList(nullptr),
	m_pText(nullptr),
	m_pSymbol(EG_SYMBOL_DOWN),
	m_pItems(nullptr),
	m_ItemCount(0),
	m_SelectedIndex(0),
	m_FocusIndex(0),
	m_PressedIndex(EG_DROPDOWN_PR_NONE),
	m_Direction(EG_DIR_BOTTOM),
	m_StaticText(1),
	m_Highlight(1)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGDropDown::EGDropDown(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_DropDownClass*/) : EGObject(),
	m_pDropList(nullptr),
	m_pText(nullptr),
	m_pSymbol(EG_SYMBOL_DOWN),
	m_pItems(nullptr),
	m_ItemCount(0),
	m_SelectedIndex(0),
	m_FocusIndex(0),
	m_PressedIndex(EG_DROPDOWN_PR_NONE),
	m_Direction(EG_DIR_BOTTOM),
	m_StaticText(1),
	m_Highlight(1)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGDropDown::~EGDropDown(void)
{
	if(m_pDropList) {
		EGObject::Delete(m_pDropList);
		m_pDropList = nullptr;
	}
	if(!m_StaticText) {
		EG_FreeMem(m_pItems);
		m_pItems = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::Configure(void)
{
  EGObject::Configure();
	m_pDropList = nullptr;
	m_pItems = nullptr;
	m_pSymbol = EG_SYMBOL_DOWN;
	m_pText = nullptr;
	m_StaticText = 1;
	m_Highlight = 1;
	m_FocusIndex = 0;
	m_SelectedIndex = 0;
	m_PressedIndex = EG_DROPDOWN_PR_NONE;
	m_ItemCount = 0;
	m_Direction = EG_DIR_BOTTOM;
	AddFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
	SetItems("Item 1\nItem 2\nItem 3");
	m_pDropList = new EGDropDownList(GetScreen(), &c_DropDownListClass);
	m_pDropList->m_pDropDown = this;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetText(const char *pText)
{
  if(m_pDropList) {
    EGObject::Delete(m_pDropList);
    m_pDropList = nullptr;
  }
	if(m_pText == pText) return;
	m_pText = pText;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetItems(const char *pItems)
{
	EG_ASSERT_NULL(pItems);
	m_ItemCount = 0;	// Count the '\n'-s to determine the number of items
	for(uint32_t i = 0; pItems[i] != '\0'; i++) {
		if(pItems[i] == '\n') m_ItemCount++;
	}
	m_ItemCount++; // Last item has no `\n`
	m_SelectedIndex = 0;
	m_FocusIndex = 0;
// Allocate space for the new text
#if EG_USE_ARABIC_PERSIAN_CHARS == 0
	size_t Length = strlen(pItems) + 1;
#else
	size_t Length = _lv_txt_ap_calc_bytes_cnt(pItems) + 1;
#endif
	if(m_pItems != nullptr && m_StaticText == 0) {
		EG_FreeMem(m_pItems);
		m_pItems = nullptr;
	}
	m_pItems = (char*)EG_AllocMem(Length);
	EG_ASSERT_MALLOC(m_pItems);
	if(m_pItems == nullptr) return;
#if EG_USE_ARABIC_PERSIAN_CHARS == 0
	strcpy(m_pItems, pItems);
#else
	_lv_txt_ap_proc(pItems, m_pItems);
#endif
	// Now the text is dynamically allocated
	m_StaticText = 0;
	Invalidate();
	if(m_pDropList) m_pDropList->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetStaticItems(const char *pItems)
{
	EG_ASSERT_NULL(pItems);
	m_ItemCount = 0;
	for(uint32_t i = 0; pItems[i] != '\0'; i++) {	// Count the '\n'-s to determine the number of items
		if(pItems[i] == '\n') m_ItemCount++;
	}
	m_ItemCount++; // Last item has no `\n`
	m_SelectedIndex = 0;
	m_FocusIndex = 0;
	if((m_StaticText == 0) && (m_pItems != nullptr)) {
		EG_FreeMem(m_pItems);
		m_pItems = nullptr;
	}
	m_StaticText = 1;
	m_pItems = (char *)pItems;
	Invalidate();
	if(m_pDropList) m_pDropList->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::AddItems(const char *pItem, uint32_t Index)
{
	EG_ASSERT_NULL(pItem);
	if(m_StaticText != 0) {	// Convert static items to dynamic
		char *StaticItem = m_pItems;
		size_t Length = strlen(StaticItem) + 1;
		m_pItems = (char*)EG_AllocMem(Length);
		EG_ASSERT_MALLOC(m_pItems);
		if(m_pItems == nullptr) return;
		strcpy(m_pItems, StaticItem);
		m_StaticText = 0;
	}
	size_t OldLength = (m_pItems == nullptr) ? 0 : strlen(m_pItems);	// Allocate space for the new item
#if EG_USE_ARABIC_PERSIAN_CHARS == 0
	size_t InsertLength = strlen(pItem) + 1;
#else
	size_t InsertLength = _lv_txt_ap_calc_bytes_cnt(pItem) + 1;
#endif
	size_t NewLength = InsertLength + OldLength + 2; // +2 for terminating NULL and possible \n
	m_pItems = (char*)EG_ReallocMem(m_pItems, NewLength + 1);
	EG_ASSERT_MALLOC(m_pItems);
	if(m_pItems == nullptr) return;
	m_pItems[OldLength] = '\0';
	uint32_t InsertPosition = OldLength;	// Find the insert character position
	if(Index != EG_DROPDOWN_POS_LAST) {
		uint32_t ItemCount = 0;
		for(InsertPosition = 0; m_pItems[InsertPosition] != 0; InsertPosition++) {
			if(ItemCount == Index) break;
			if(m_pItems[InsertPosition] == '\n') ItemCount++;
		}
	}
	if((InsertPosition > 0) && (Index >= m_ItemCount))
		EG_TextInsert(m_pItems, EG_TextEncodedGetPosition(m_pItems, InsertPosition++), "\n");	// Add delimiter to existing items
	// Insert the new item, adding \n if necessary
	char *InsertBuffer = (char*)EG_GetBufferMem(InsertLength + 2); // + 2 for terminating NULL and possible \n
	EG_ASSERT_MALLOC(InsertBuffer);
	if(InsertBuffer == nullptr) return;
#if EG_USE_ARABIC_PERSIAN_CHARS == 0
	strcpy(InsertBuffer, pItem);
#else
	_lv_txt_ap_proc(pItem, InsertBuffer);
#endif
	if(Index < m_ItemCount) strcat(InsertBuffer, "\n");
	EG_TextInsert(m_pItems, EG_TextEncodedGetPosition(m_pItems, InsertPosition), InsertBuffer);
	EG_ReleaseBufferMem(InsertBuffer);
	m_ItemCount++;
	Invalidate();
	if(m_pDropList) m_pDropList->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::ClearItems(void)
{
	if(m_pItems == nullptr) return;
	if(m_StaticText == 0)	EG_FreeMem(m_pItems);
	m_pItems = nullptr;
	m_StaticText = 0;
	m_ItemCount = 0;
	Invalidate();
	if(m_pDropList) m_pDropList->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetSelectedIndex(uint16_t Index)
{
	if(m_SelectedIndex == Index) return;
	m_SelectedIndex = Index < m_ItemCount ? Index : m_ItemCount - 1;
	m_FocusIndex = m_SelectedIndex;
	if(m_pDropList) PositionToSelected();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetDirection(EG_DirType_e Direction)
{
	if(m_Direction== Direction) return;
	m_Direction = Direction;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetSymbol(const void *pSymbol)
{
	m_pSymbol = pSymbol;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::SetHighlight(bool Enable)
{
	m_Highlight = Enable;
	if(m_pDropList) m_pDropList->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::GetSelectedText(char *pBuffer, uint32_t Size)
{
uint32_t i, j, line = 0;
size_t Length;

	if(m_pItems) Length = strlen(m_pItems);
	else{
    pBuffer[0] = '\0';
		return;
	}
	for(i = 0; i < Length && line != m_FocusIndex; i++) {
		if(m_pItems[i] == '\n') line++;
	}
	for(j = 0; i < Length && m_pItems[i] != '\n'; j++, i++) {
		if(Size && j >= Size - 1) {
			EG_LOG_WARN("GetSelectedText: The buffer was too small");
			break;
		}
		pBuffer[j] = m_pItems[i];
	}
	pBuffer[j] = '\0';
}

///////////////////////////////////////////////////////////////////////////////////////

int32_t EGDropDown::GetItemIndex(const char *pItem)
{
uint32_t i = 0, Index = 0;
uint32_t ItemLength = strlen(pItem);
const char *pStart = m_pItems;

	while(pStart[i] != '\0') {
		for(i = 0; (pStart[i] != '\n') && (pStart[i] != '\0'); i++);
		if(ItemLength == i && memcmp(pStart, pItem, EG_MIN(ItemLength, i)) == 0) {
			return Index;
		}
		pStart = &pStart[i];
		if(pStart[0] == '\n') pStart++;
		i = 0;
		Index++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGDropDown::GetSymbol(void)
{
	return (char*)m_pSymbol;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGDropDown::GetHighlight(void)
{
	return m_Highlight;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_DirType_e EGDropDown::GetDirection(void)
{
	return m_Direction;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::Open(void)
{
  g_pItemObj = m_pDropList;
	AddState(EG_STATE_CHECKED);
	m_pDropList->SetParent(GetScreen());
	m_pDropList->MoveToIndex(-1);
	m_pDropList->ClearFlag(EG_OBJ_FLAG_HIDDEN);
	EGEvent::EventSend(this, EG_EVENT_READY, nullptr);	// To allow styling the list
	EGLabel *pLabel = GetLabel();
	pLabel->SetStaticText(m_pItems);
	m_pDropList->SetWidth(EG_SIZE_CONTENT);
	pLabel->UpdateLayout();
	if((m_pDropList->GetWidth() <= GetWidth()) &&	((m_Direction == EG_DIR_TOP) || (m_Direction == EG_DIR_BOTTOM))) {// Set smaller width to the width of the button
		m_pDropList->SetWidth(GetWidth());
	}
	EG_Coord_t LabelHeight = pLabel->GetHeight();
	EG_Coord_t BorderWidth = m_pDropList->GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t TopPad = m_pDropList->GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t BottomPad = m_pDropList->GetStylePadBottom(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t FitHeight = LabelHeight + TopPad + BottomPad;
	EG_Coord_t ListHeight = FitHeight;
	EG_DirType_e Direction = m_Direction;
  EG_Coord_t VerticalRes = EGDisplay::GetDefault()->GetVerticalRes();
	if(m_Direction == EG_DIR_BOTTOM) {	// No space on the BottomPad? See if TopPad is better.
		if(m_Rect.GetY2() + ListHeight > VerticalRes) {
			if(m_Rect.GetY1() > VerticalRes - m_Rect.GetY2()) {
				// There is more space on the TopPad, so make it drop up
				Direction = EG_DIR_TOP;
				ListHeight = m_Rect.GetY1() - 1;
			}
			else {
				ListHeight = VerticalRes - m_Rect.GetY2() - 1;
			}
		}
	}
	// No space on the TopPad? See if BottomPad is better.
	else if(m_Direction == EG_DIR_TOP) {
		if(m_Rect.GetY1() - ListHeight < 0) {
			if(m_Rect.GetY1() < VerticalRes - m_Rect.GetY2()) {
				// There is more space on the TopPad, so make it drop up
				Direction = EG_DIR_BOTTOM;
				ListHeight = VerticalRes - m_Rect.GetY2();
			}
			else {
				ListHeight = m_Rect.GetY1();
			}
		}
	}
	if(ListHeight > FitHeight) ListHeight = FitHeight;
	m_pDropList->SetHeight(ListHeight);
	PositionToSelected();
	switch(Direction){
	  case EG_DIR_LEFT:{
      m_pDropList->AlignTo(this, EG_ALIGN_OUT_LEFT_TOP, 0, 0);
      break;
    }
	  case EG_DIR_RIGHT:{
      m_pDropList->AlignTo(this, EG_ALIGN_OUT_RIGHT_TOP, 0, 0);
      break;
    }
	  case EG_DIR_TOP:{
      m_pDropList->AlignTo(this, EG_ALIGN_OUT_TOP_LEFT, 0, 0);
      break;
    }
    case EG_DIR_BOTTOM:{
      m_pDropList->AlignTo(this, EG_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
      break;
    }
    default: break;
  }
 	m_pDropList->UpdateLayout();
	if(m_Direction == EG_DIR_LEFT || m_Direction == EG_DIR_RIGHT) {
		EG_Coord_t y1 = m_pDropList->GetY();
		EG_Coord_t y2 = m_pDropList->GetY2();
		if(y2 >= EG_VERT_RES) {
			m_pDropList->SetY(y1 - (y2 - EG_VERT_RES) - 1);
		}
	}
	EG_TextAlignment_t Align = pLabel->CalculateTextAlignment(EG_PART_MAIN, m_pItems);
	switch(Align) {
		default:
		case EG_TEXT_ALIGN_LEFT:
			pLabel->Align(EG_ALIGN_TOP_LEFT, 0, 0);
			break;
		case EG_TEXT_ALIGN_RIGHT:
			pLabel->Align(EG_ALIGN_TOP_RIGHT, 0, 0);
			break;
		case EG_TEXT_ALIGN_CENTER:
			pLabel->Align(EG_ALIGN_TOP_MID, 0, 0);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::Close(void)
{
	ClearState(EG_STATE_CHECKED);
	m_PressedIndex = EG_DROPDOWN_PR_NONE;
	if(m_pDropList != nullptr) m_pDropList->AddFlag(EG_OBJ_FLAG_HIDDEN);
	EGEvent::EventSend(this, EG_EVENT_CANCEL, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGDropDown::IsOpen(void)
{
	return m_pDropList->HasFlagSet(EG_OBJ_FLAG_HIDDEN) ? false : true;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(DROP_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGDropDown *pDropDown = (EGDropDown*)pEvent->GetTarget();
  pDropDown->Event(pEvent); // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
	switch(Code){
    case EG_EVENT_FOCUSED:{
      EGGroup *pGroup = (EGGroup*)GetGroup();
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      if(InputType == EG_INDEV_TYPE_ENCODER) {		// Encoders need special handling
        if(IsEditing) Open();			// Open the list if IsEditing
        else{			                // Close the list if navigating
          m_SelectedIndex = m_FocusIndex;
          Close();
        }
      }
      break;
    }
    case EG_EVENT_DEFOCUSED:
    case EG_EVENT_LEAVE:{
      Close();
      break;
    }
    case EG_EVENT_RELEASED:{
      EG_Result_t Result = ButtonReleaseHandler();
      if(Result != EG_RES_OK) return;
      break;
    }
    case EG_EVENT_STYLE_CHANGED:{
      RefreshSelfSize();
      break;
    }
    case EG_EVENT_SIZE_CHANGED:{
      RefreshSelfSize();
      break;
    }
    case EG_EVENT_GET_SELF_SIZE:{
      EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
      const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
      pPoint->m_Y = EG_FontGetLineHeight(pFont);
      break;
    }
    case EG_EVENT_KEY:{
      char Key = *((char *)pEvent->GetParam());
      switch(Key){
        case EG_KEY_RIGHT:
        case EG_KEY_DOWN:{
          if(!IsOpen()) Open();
          else if(m_SelectedIndex + 1 < m_ItemCount) {
            m_SelectedIndex++;
            PositionToSelected();
          }
          break;
        }
        case EG_KEY_LEFT:
        case EG_KEY_UP:{
          if(!IsOpen()) Open();
          else if(m_SelectedIndex > 0) {
            m_SelectedIndex--;
            PositionToSelected();
          }
          break;
        }
        case EG_KEY_ESC:{
          m_SelectedIndex = m_FocusIndex;
          Close();
          break;
        }
        case EG_KEY_ENTER:{
          // Handle the ENTER key only if it was send by another object. Do no process
          // it if ENTER is sent by the dropdown because it's handled in EG_EVENT_RELEASED. 
          EGObject *indev_obj = EGInputDevice::GetActiveObj();
          if(indev_obj != this) {
            EG_Result_t Result  = ButtonReleaseHandler();
            if(Result != EG_RES_OK) return;
          }
          break;
        }
      }
      break;
    }
    case EG_EVENT_DRAW_MAIN:{
      DrawMain(pEvent);
      break;
    }
    default: break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::DrawMain(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t left = GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t right = GetStylePadRight(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t TopPad = GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	EGDrawLabel DrawSymbol;
	InititialseDrawLabel(EG_PART_INDICATOR, &DrawSymbol);
	const char *pItemText;	// If no text specified use the selected item
	if(m_pText)
		pItemText = m_pText;
	else {
		char *pTextBuffer = (char*)EG_GetBufferMem(128);
		GetSelectedText(pTextBuffer, 128);
		pItemText = pTextBuffer;
	}
	bool SymbolToLeft = false;
	if(m_Direction == EG_DIR_LEFT) SymbolToLeft = true;
	if(GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) SymbolToLeft = true;
	if(m_pSymbol) {
		EG_ImageSource_t SymbolType = EGDrawImage::GetType(m_pSymbol);
		EG_Coord_t SymbolWidth;
		EG_Coord_t SymbolHeight;
		if(SymbolType == EG_IMG_SRC_SYMBOL) {
			EGPoint TextSize;
			EG_GetTextSize(&TextSize, (char*)m_pSymbol, DrawSymbol.m_pFont, DrawSymbol.m_Kerning, DrawSymbol.m_LineSpace, EG_COORD_MAX, DrawSymbol.m_Flag);
			SymbolWidth = TextSize.m_X;
			SymbolHeight = TextSize.m_Y;
		}
		else {
			EG_ImageHeader_t header;
			EG_Result_t res = EGImageDecoder::GetInfo(m_pSymbol, &header);
			if(res == EG_RES_OK) {
				SymbolWidth = header.Width;
				SymbolHeight = header.Height;
			}
			else {
				SymbolWidth = -1;
				SymbolHeight = -1;
			}
		}
		EGRect SymbolRect;
		if(SymbolToLeft) {
			SymbolRect.SetX1(m_Rect.GetX1() + left);
			SymbolRect.SetX2(SymbolRect.GetX1() + SymbolWidth - 1);
		}
		else {
			SymbolRect.SetX1(m_Rect.GetX2() - right - SymbolWidth);
			SymbolRect.SetX2(SymbolRect.GetX1() + SymbolWidth - 1);
		}

		if(SymbolType == EG_IMG_SRC_SYMBOL) {
			SymbolRect.SetY1(m_Rect.GetY1() + TopPad);
			SymbolRect.SetY2(SymbolRect.GetY1() + SymbolHeight - 1);
			DrawSymbol.Draw(pContext, &SymbolRect, (char*)m_pSymbol, nullptr);
		}
		else {
			SymbolRect.SetY1(m_Rect.GetY1() + (GetHeight() - SymbolHeight) / 2);
			SymbolRect.SetY2(SymbolRect.GetY1() + SymbolHeight - 1);
			EGDrawImage DrawImage;
			InititialseDrawImage(EG_PART_INDICATOR, &DrawImage);
			DrawImage.m_Pivot.m_X = SymbolWidth / 2;
			DrawImage.m_Pivot.m_Y = SymbolHeight / 2;
			DrawImage.m_Angle = GetStyleTransformAngle(EG_PART_INDICATOR);
			DrawImage.Draw(pContext, &SymbolRect, m_pSymbol);
		}
	}
	EGDrawLabel DrawLabel;
	InititialseDrawLabel(EG_PART_MAIN, &DrawLabel);
	EGPoint TextSize;
	EG_GetTextSize(&TextSize, pItemText, DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace, EG_COORD_MAX, DrawLabel.m_Flag);
	EGRect TextRect;
	TextRect.SetY1(m_Rect.GetY1() + TopPad);
	TextRect.SetY2(TextRect.GetY1() + TextSize.m_Y);
	// Center Align the text if no symbol
	if(m_pSymbol == nullptr) {
		TextRect.SetX1(m_Rect.GetX1() + (GetWidth() - TextSize.m_X) / 2);
		TextRect.SetX2(TextRect.GetX1() + TextSize.m_X);
	}
	else {
		// Text to the right
		if(SymbolToLeft) {
			TextRect.SetX1(m_Rect.GetX2() - right - TextSize.m_X);
			TextRect.SetX2(TextRect.GetX1() + TextSize.m_X);
		}
		else {
			TextRect.SetX1(m_Rect.GetX1() + left);
			TextRect.SetX2(TextRect.GetX1() + TextSize.m_X);
		}
	}
	DrawLabel.Draw(pContext, &TextRect, pItemText, nullptr);
	if(m_pText == nullptr) {
		EG_ReleaseBufferMem((char *)pItemText);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::DrawBox(EGDrawContext *pContext, uint16_t Index, EGState_t State)
{
	if(Index == EG_DROPDOWN_PR_NONE) return;
	EGState_t OriginalState = m_pDropList->m_State;
	if(State != m_pDropList->m_State) {
		m_pDropList->m_State = State;
		m_pDropList->m_SkipTransition = 1;
	}
	const EG_Font_t *pFont = m_pDropList->GetStyleTextFont(EG_PART_SELECTED);	// Draw a rectangle under the selected item
	EG_Coord_t LineSpace = m_pDropList->GetStyleTextLineSpace(EG_PART_SELECTED);
	EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);
	EGObject *pLabel = GetLabel();	// Draw the selected
	EGRect LabelRect;
	LabelRect.SetY1(pLabel->m_Rect.GetY1());
	LabelRect.IncY1(Index * (FontHeight + LineSpace));
	LabelRect.DecY1(LineSpace / 2);
	LabelRect.SetY2(LabelRect.GetY1() + FontHeight + LineSpace - 1);
	LabelRect.SetX1(m_pDropList->m_Rect.GetX1());
	LabelRect.SetX2(m_pDropList->m_Rect.GetX2());
	EGDrawRect DrawRect;
	m_pDropList->InititialseDrawRect(EG_PART_SELECTED, &DrawRect);
	DrawRect.Draw(pContext, &LabelRect);
	m_pDropList->m_State = OriginalState;
	m_pDropList->m_SkipTransition = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::DrawBoxLabel(EGDrawContext *pContext, uint16_t Index, EGState_t State)
{
	if(Index == EG_DROPDOWN_PR_NONE) return;
	EGState_t OriginalState = m_pDropList->m_State;
	if(State != m_pDropList->m_State) {
		m_pDropList->m_State = State;
		m_pDropList->m_SkipTransition = 1;
	}
	EGDrawLabel DrawLabel;
	m_pDropList->InititialseDrawLabel(EG_PART_SELECTED, &DrawLabel);
	DrawLabel.m_LineSpace = m_pDropList->GetStyleTextLineSpace(EG_PART_SELECTED); // Line space should come from the list
	EGLabel *pLabel = GetLabel();
	if(pLabel == nullptr) return;
	EG_Coord_t FontHeight = EG_FontGetLineHeight(DrawLabel.m_pFont);
	EGRect SelectRect;
	SelectRect.SetY1(pLabel->m_Rect.GetY1());
	SelectRect.IncY1(Index * (FontHeight + DrawLabel.m_LineSpace));
	SelectRect.DecY1(DrawLabel.m_LineSpace / 2);
	SelectRect.SetY2(SelectRect.GetY1() + FontHeight + DrawLabel.m_LineSpace - 1);
	SelectRect.SetX1(m_pDropList->m_Rect.GetX1());
	SelectRect.SetX2(m_pDropList->m_Rect.GetX2());
	EGRect MaskRect;
	if(MaskRect.Intersect(pContext->m_pClipRect, &SelectRect)) {
		const EGRect *OriginalClipRect = pContext->m_pClipRect;
		pContext->m_pClipRect = &MaskRect;
		DrawLabel.Draw(pContext, &pLabel->m_Rect, pLabel->GetText(), nullptr);
		pContext->m_pClipRect = OriginalClipRect;
	}
	m_pDropList->m_State = OriginalState;
	m_pDropList->m_SkipTransition = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDropDown::ButtonReleaseHandler(void)
{
	EGInputDevice *pInput = EGInputDevice::GetActive();
//  if(g_pItemObj == obj) ESP_LOGI("[Object]", "Layout Invalid:%d, DoScroll:%d", obj->layout_inv, was_on_layout);
	if(pInput->GetScrollObj() == nullptr) {
		if(IsOpen()) {
			Close();
			if(m_FocusIndex != m_SelectedIndex) {
				m_FocusIndex = m_SelectedIndex;
				uint32_t Index = m_SelectedIndex; // Just to use uint32_t in event data
				EG_Result_t Result = EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, &Index);
				if(Result != EG_RES_OK) return Result;
				Invalidate();
			}
			EG_InDeviceType_e InputType = pInput->GetType();
			if(InputType == EG_INDEV_TYPE_ENCODER) {
				((EGGroup*)GetGroup())->SetEditing(false);
			}
		}
		else Open();
	}
	else {
		m_SelectedIndex = m_FocusIndex;
		Invalidate();
	}
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGDropDown::GetIndexOnPoint(EG_Coord_t Y)
{
	EGObject *pLabel = GetLabel();
	if(pLabel == nullptr) return 0;
	Y -= pLabel->m_Rect.GetY1();
	const EG_Font_t *pFont = pLabel->GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);
	EG_Coord_t LineSpace = pLabel->GetStyleTextLineSpace(EG_PART_MAIN);
	Y += LineSpace / 2;
	EG_Coord_t h = FontHeight + LineSpace;
	uint16_t Index = Y / h;
	if(Index >= m_ItemCount) Index = m_ItemCount - 1;
	return Index;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDown::PositionToSelected(void)
{
	EGObject *pLabel = GetLabel();
	if(pLabel == nullptr) return;
	if(pLabel->GetHeight() <= GetContentHeight()) return;
	const EG_Font_t *pFont = pLabel->GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);
	EG_Coord_t LineSpace = pLabel->GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t ItemHeight = FontHeight + LineSpace;
	EG_Coord_t ScrollPoint = m_SelectedIndex * ItemHeight;
	m_pDropList->ScrollToY(ScrollPoint, EG_ANIM_OFF);	// Scroll to the selected item
	m_pDropList->Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EGLabel* EGDropDown::GetLabel(void)
{
	if(m_pDropList == nullptr) return nullptr;
	return (EGLabel*)m_pDropList->GetChild(0);
}

///////////////////////////////////////////////////////////////////////////////////////

// DROPDOWN LIST //

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_DropDownListClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGDropDownList::EventCB,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGDropDownList::EGDropDownList(void) : EGObject(),
  m_pDropDown(nullptr),
  m_pLabel(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGDropDownList::EGDropDownList(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_DropDownListClass*/) : EGObject(),
  m_pDropDown(nullptr),
  m_pLabel(nullptr)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGDropDownList::~EGDropDownList(void)
{
  m_pDropDown->m_pDropList = nullptr;  // make sure there is no reference to this object
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDownList::Configure(void)
{
  EGObject::Configure();
	ClearFlag(EG_OBJ_FLAG_SCROLL_ON_FOCUS);
	ClearFlag(EG_OBJ_FLAG_CLICK_FOCUSABLE);
	AddFlag(EG_OBJ_FLAG_IGNORE_LAYOUT);
	AddFlag(EG_OBJ_FLAG_HIDDEN);
	m_pLabel = new EGLabel(this);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDownList::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(DROPLIST_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	((EGDropDownList*)pEvent->GetTarget())->Event(pEvent);;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDownList::Event(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
	switch(Code){
    case EG_EVENT_RELEASED: {
      if(EGInputDevice::GetActive()->GetScrollObj() == nullptr) {
        ReleaseHandler();
      }
      break;
    }
    case EG_EVENT_PRESSED: {
      PressHandler();
      break;
    }
    case EG_EVENT_SCROLL_BEGIN: {
      m_pDropDown->m_PressedIndex = EG_DROPDOWN_PR_NONE;
      Invalidate();
      break;
    }
    case EG_EVENT_DRAW_POST: {
      Draw(pEvent);
      pEvent->Pump(DROPLIST_CLASS);	// Call the ancestor'pSize event handler
      break;
    }
    default: break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDownList::Draw(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	EGRect ClipRect;	//  Clip area might be too large too to shadow but the selected item can be drawn on only the background
	if(ClipRect.Intersect(pContext->m_pClipRect, &m_Rect)) {
		const EGRect *pClipRect = pContext->m_pClipRect;
		pContext->m_pClipRect = &ClipRect;
		if(m_pDropDown->m_Highlight) {
			if(m_pDropDown->m_PressedIndex == m_pDropDown->m_SelectedIndex) {
				m_pDropDown->DrawBox(pContext, m_pDropDown->m_PressedIndex, EG_STATE_CHECKED | EG_STATE_PRESSED);
				m_pDropDown->DrawBoxLabel(pContext, m_pDropDown->m_PressedIndex, EG_STATE_CHECKED | EG_STATE_PRESSED);
			}
			else {
				m_pDropDown->DrawBox(pContext, m_pDropDown->m_PressedIndex, EG_STATE_PRESSED);
				m_pDropDown->DrawBoxLabel(pContext, m_pDropDown->m_PressedIndex, EG_STATE_PRESSED);
				m_pDropDown->DrawBox(pContext, m_pDropDown->m_SelectedIndex, EG_STATE_CHECKED);
				m_pDropDown->DrawBoxLabel(pContext, m_pDropDown->m_SelectedIndex, EG_STATE_CHECKED);
			}
		}
		else {
			m_pDropDown->DrawBox(pContext, m_pDropDown->m_PressedIndex, EG_STATE_PRESSED);
			m_pDropDown->DrawBoxLabel(pContext, m_pDropDown->m_PressedIndex, EG_STATE_PRESSED);
		}
		pContext->m_pClipRect = pClipRect;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGDropDownList::PressHandler(void)
{
	EGInputDevice *pInput = EGInputDevice::GetActive();
	if((pInput != nullptr) && ((pInput->GetType() == EG_INDEV_TYPE_POINTER) || (pInput->GetType() == EG_INDEV_TYPE_BUTTON))) {
		EGPoint pPoint;
		pInput->GetPoint(&pPoint);
		m_pDropDown->m_PressedIndex = m_pDropDown->GetIndexOnPoint(pPoint.m_Y);
		Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDropDownList::ReleaseHandler(void)
{
	EGInputDevice *pInput = EGInputDevice::GetActive();
	if(pInput->GetType() == EG_INDEV_TYPE_ENCODER) {	// Leave edit mode once a new item is selected
		m_pDropDown->m_FocusIndex = m_pDropDown->m_SelectedIndex;
    EGGroup *pGroup = (EGGroup*)GetGroup();
    bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
		if(IsEditing) pGroup->SetEditing(false);
	}
	// Search the clicked item (For KEYPAD and ENCODER the new value should be already set)
	if(pInput->GetType() == EG_INDEV_TYPE_POINTER || pInput->GetType() == EG_INDEV_TYPE_BUTTON) {
		EGPoint pPoint;
		pInput->GetPoint(&pPoint);
		m_pDropDown->m_SelectedIndex = m_pDropDown->GetIndexOnPoint(pPoint.m_Y);
		m_pDropDown->m_FocusIndex = m_pDropDown->m_SelectedIndex;
	}
	m_pDropDown->Close();
	if(m_pDropDown->m_pText == nullptr) m_pDropDown->Invalidate();	// Invalidate to refresh the text
	uint32_t Index = m_pDropDown->m_SelectedIndex; // Just to use uint32_t in event data
	EG_Result_t Result = EGEvent::EventSend(m_pDropDown, EG_EVENT_VALUE_CHANGED, &Index);
	if(Result != EG_RES_OK) return Result;
	return EG_RES_OK;
}


#endif
