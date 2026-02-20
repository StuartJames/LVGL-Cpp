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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "core/EG_Object.h"
#include "core/EG_ObjPosition.h"
#include "core/EG_Display.h"
#include "core/EG_Refresh.h"
#include "misc/EG_Misc.h"

/////////////////////////////////////////////////////////////////////////////

uint32_t EGObject::m_LayoutCount = 0;

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetPosition(EG_Coord_t X, EG_Coord_t Y)
{
	SetX(X);
	SetY(Y);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetX(EG_Coord_t X)
{
EG_Result_t Result;
EG_StyleValue_t Value;

	Result = GetLocalStylelProperty(EG_STYLE_X, &Value, 0);
	if((Result == EG_RES_OK && Value.Number != X) || Result == EG_RES_INVALID) {
		SetStyleX(X, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetY(EG_Coord_t Y)
{
EG_Result_t Result;
EG_StyleValue_t Value;

	Result = GetLocalStylelProperty(EG_STYLE_Y, &Value, 0);
	if((Result == EG_RES_OK && Value.Number != Y) || Result == EG_RES_INVALID) {
		SetStyleY(Y, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::RefreshSize(void)
{
bool WidthIsContent = false, HeightIsContent = false;   // Content controls
bool WidthIsPct = false, HeightIsPct = false;           // Percentage controls
EG_Coord_t Height, Width;
EGRect OriginalRect, ParentRect;

	if(m_WidthLayout && m_HeightLayout) return false;	// If the width or height is set by a layout do not modify them
	EGObject *pParent = GetParent();
	if(pParent == nullptr) return false;
	EG_Coord_t ScrollLeft = GetScrollLeft();
	if(m_WidthLayout)  Width = GetWidth();
	else {
		Width = GetStyleWidth(EG_PART_MAIN);
		WidthIsContent = (Width == EG_SIZE_CONTENT) ? true : false;
		WidthIsPct = EG_COORD_IS_PCT(Width) ? true : false;
		EG_Coord_t ParentWidth = pParent->GetContentWidth();
		if(WidthIsContent) Width = CalcContentWidth();
		else if(WidthIsPct) {
			// If pParent has content size and the child has pct size a circular dependency will occur. To solve it keep child size at zero 
			if((pParent->m_WidthLayout == 0) && (pParent->GetStyleWidth(0) == EG_SIZE_CONTENT)) {
				EG_Coord_t BorderWidth = GetStyleBorderWidth(0);
				Width = GetStylePadLeft(0) + BorderWidth;
				Width += GetStylePadRight(0) + BorderWidth;
			}
			else Width = (EG_COORD_GET_PCT(Width) * ParentWidth) / 100;
		}
		EG_Coord_t MinWidth = GetStyleMinWidth(EG_PART_MAIN);
		EG_Coord_t MaxWidth = GetStyleMaxWidth(EG_PART_MAIN);
		Width = ClampWidth(Width, MinWidth, MaxWidth, ParentWidth);
	}
	EG_Coord_t ScrollTop = GetScrollTop();
	if(m_HeightLayout){
		Height = GetHeight();
	}
	else {
		Height = GetStyleHeight(EG_PART_MAIN);
		HeightIsContent = (Height == EG_SIZE_CONTENT) ? true : false;
		HeightIsPct = EG_COORD_IS_PCT(Height) ? true : false;
		EG_Coord_t ParentHeight = pParent->GetContentHeight();
		if(HeightIsContent) Height = CalcContentHeight();
		else if(HeightIsPct) {
			// If pParent has content size and the child has pct size a circular dependency will occur. To solve it keep child size at zero 
			if((pParent->m_HeightLayout == 0) && (pParent->GetStyleHeight(0) == EG_SIZE_CONTENT)) {
				EG_Coord_t BorderWidth = GetStyleBorderWidth(0);
				Height = GetStylePadTop(0) + BorderWidth;
				Height += GetStylePadBottom(0) + BorderWidth;
			}
			else {
				Height = (EG_COORD_GET_PCT(Height) * ParentHeight) / 100;
			}
		}
		EG_Coord_t MinHeight = GetStyleMinHeight(EG_PART_MAIN);
		EG_Coord_t MaxHeight = GetStyleMaxHeight(EG_PART_MAIN);
		Height = ClampHeight(Height, MinHeight, MaxHeight, ParentHeight);
	}
	if(WidthIsContent || HeightIsContent) {	//calc_auto_size set the scroll x/y to 0 so revert the original value
		ScrollTo(ScrollLeft, ScrollTop, EG_ANIM_OFF);
	}
	// Do nothing if the size is not changed. It is very important else recursive resizing can occur without size change
	if((GetWidth() == Width) && (GetHeight() == Height)) return false;
	Invalidate();	// Invalidate the original area
	m_Rect.Copy(&OriginalRect);	// Save the original coordinates
	pParent->GetContentArea(&ParentRect);	// Check if the object is inside the pParent or not
	// If the object is already out of the parent area and its position is changes invalidate the scroll bars
	bool Invalidate1 = OriginalRect.IsInside(&ParentRect, 0);
	if(!Invalidate1) pParent->ScrollbarInvalidate();
	// Set the length and height. Be sure the content is not scrolled in an invalid position on the new size
	m_Rect.SetY2(m_Rect.GetY1() + Height - 1);
	if(GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) m_Rect.SetX1(m_Rect.GetX2() - Width + 1);
	else m_Rect.SetX2(m_Rect.GetX1() + Width - 1);
	EGEvent::EventSend(this, EG_EVENT_SIZE_CHANGED, &OriginalRect);	// Call the ancestor's event handler to the object with its new coordinates
	EGEvent::EventSend(pParent, EG_EVENT_CHILD_CHANGED, this);	// Call the ancestor's event handler to the pParent too
	Invalidate();	// Invalidate the new area
	m_ReadScrollAfterLayout = 1;
	// If the object was outside the pParent, invalidate the new scrollbar area too. If it wasn't but out now, also invalidate the scrollbars
	bool Invalidate2 = m_Rect.IsInside(&ParentRect, 0);
	if(Invalidate1 || (!Invalidate1 && Invalidate2)) pParent->ScrollbarInvalidate();
	RefreshExtDrawSize();
	return true;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetSize(EG_Coord_t Width, EG_Coord_t Height)
{
	SetWidth(Width);
	SetHeight(Height);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetWidth(EG_Coord_t Width)
{
EG_Result_t Result;
EG_StyleValue_t Value;

	Result = GetLocalStylelProperty(EG_STYLE_WIDTH, &Value, 0);
	if(((Result == EG_RES_OK) && (Value.Number != Width)) || (Result == EG_RES_INVALID)) {
		SetStyleWidth(Width, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetHeight(EG_Coord_t Height)
{
EG_Result_t Result;
EG_StyleValue_t Value;

	Result = GetLocalStylelProperty(EG_STYLE_HEIGHT, &Value, 0);
	if(((Result == EG_RES_OK) && (Value.Number != Height)) || (Result == EG_RES_INVALID)) {
		SetStyleHeight(Height, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetContentWidth(EG_Coord_t Width)
{
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PsdRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	SetWidth(Width + PadLeft + PsdRight + 2 * BorderWidth);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetContentHeight(EG_Coord_t Height)
{
	EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	SetHeight(Height + PadTop + PadBottom + 2 * BorderWidth);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetLayout(uint32_t Layout)
{
	SetStyleLayout(Layout, 0);
	MarkLayoutDirty();
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::IsLayoutPositioned(void)
{
	if(HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_FLOATING)) return false;
	EGObject *pParent = GetParent();
	if(pParent == nullptr) return false;
	uint32_t Layout = pParent->GetStyleLayout(EG_PART_MAIN);
	if(Layout) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::MarkLayoutDirty(void)
{
	m_LayoutInvalid = 1;
	EGObject *pScreen = GetScreen();	// Mark the screen as dirty too to mark that there is something to do on this screen
	pScreen->m_ScreenLayoutInvalid = 1;
	EGDisplay *pDisplay = pScreen->GetDisplay();	// Make the display refreshing
	if(pDisplay->m_pRefreshTimer) pDisplay->m_pRefreshTimer->Resume();
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::UpdateLayout(void)
{
static bool Mutex = false;

	if(Mutex) {
		EG_LOG_TRACE("Warning: Layout update already in progress");
		return;
	}
	Mutex = true;
	EGObject *pScreen = GetScreen();
	while(pScreen->m_ScreenLayoutInvalid) {	
		EG_LOG_INFO("Layout update begin");
		pScreen->m_ScreenLayoutInvalid = 0;
		pScreen->LayoutUpdateCore();
		EG_LOG_TRACE("Layout update end");
	}
	Mutex = false;
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EGObject::LayoutRegister(EG_LayoutUpdateCB_t LayoutCB, void *pUserData)
{
	m_LayoutCount++;
	EG_LayoutDiscriptor *pNewLayout = (EG_LayoutDiscriptor *)EG_AllocMem(sizeof(EG_LayoutDiscriptor));
	EG_ASSERT_MALLOC(pNewLayout);
	pNewLayout->UpdateCB = LayoutCB;
	pNewLayout->pUserData = pUserData;
	return (uint32_t)m_LayoutList.AddHead(pNewLayout);     
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetAlign(EG_AlignType_e Align)
{
	SetStyleAlign(Align, 0);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::Align(EG_AlignType_e Align, EG_Coord_t OffsetX, EG_Coord_t OfsetY)
{
	SetStyleAlign(Align, 0);
	SetPosition(OffsetX, OfsetY);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::AlignTo(EGObject *pBase, EG_AlignType_e Align, EG_Coord_t OffsetX, EG_Coord_t OffsetY)
{
EG_Coord_t X = 0;
EG_Coord_t Y = 0;

	UpdateLayout();
	if(pBase == nullptr) pBase = GetParent();
	EGObject *pParent = GetParent();
	EG_Coord_t ParentBorder = pParent->GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t ParentLeft = pParent->GetStylePadLeft(EG_PART_MAIN) + ParentBorder;
	EG_Coord_t ParentTop = pParent->GetStylePadTop(EG_PART_MAIN) + ParentBorder;
	EG_Coord_t BaseBorder = pBase->GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t BaseLeft = pBase->GetStylePadLeft(EG_PART_MAIN) + BaseBorder;
	EG_Coord_t BaseTop = pBase->GetStylePadTop(EG_PART_MAIN) + BaseBorder;
	if(Align == EG_ALIGN_DEFAULT) {
		if(pBase->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL)	Align = EG_ALIGN_TOP_RIGHT;
		else Align = EG_ALIGN_TOP_LEFT;
	}
	switch(Align) {
    case EG_ALIGN_DEFAULT:
		case EG_ALIGN_CENTER:{
			X = pBase->GetContentWidth() / 2 - GetWidth() / 2 + BaseLeft;
			Y = pBase->GetContentWidth() / 2 - GetContentHeight() / 2 + BaseTop;
			break;
    }
		case EG_ALIGN_TOP_LEFT:{
			X = BaseLeft;
			Y = BaseTop;
			break;
    }
		case EG_ALIGN_TOP_MID:{
			X = pBase->GetContentWidth() / 2 - GetWidth() / 2 + BaseLeft;
			Y = BaseTop;
			break;
    }
		case EG_ALIGN_TOP_RIGHT:{
			X = pBase->GetContentWidth() - GetWidth() + BaseLeft;
			Y = BaseTop;
			break;
    }
		case EG_ALIGN_BOTTOM_LEFT:{
			X = BaseLeft;
			Y = pBase->GetContentHeight() - GetHeight() + BaseTop;
			break;
    }
		case EG_ALIGN_BOTTOM_MID:{
			X = pBase->GetContentWidth() / 2 - GetWidth() / 2 + BaseLeft;
			Y = pBase->GetContentHeight() - GetHeight() + BaseTop;
			break;
    }
		case EG_ALIGN_BOTTOM_RIGHT:{
			X = pBase->GetContentWidth() - GetWidth() + BaseLeft;
			Y = pBase->GetContentHeight() - GetHeight() + BaseTop;
			break;
    }
		case EG_ALIGN_LEFT_MID:{
			X = BaseLeft;
			Y = pBase->GetContentHeight() / 2 - GetHeight() / 2 + BaseTop;
			break;
    }
		case EG_ALIGN_RIGHT_MID:{
			X = pBase->GetContentWidth() - GetWidth() + BaseLeft;
			Y = pBase->GetContentHeight() / 2 - GetHeight() / 2 + BaseTop;
			break;
    }
		case EG_ALIGN_OUT_TOP_LEFT:{
			X = 0;
			Y = -GetHeight();
			break;
    }
		case EG_ALIGN_OUT_TOP_MID:{
			X = ((EGObject*)pBase)->GetWidth() / 2 - GetWidth() / 2;
			Y = -GetHeight();
			break;
    }
		case EG_ALIGN_OUT_TOP_RIGHT:{
			X = ((EGObject*)pBase)->GetWidth() - GetWidth();
			Y = -GetHeight();
			break;
    }
		case EG_ALIGN_OUT_BOTTOM_LEFT:{
			X = 0;
			Y = ((EGObject*)pBase)->GetHeight();
			break;
    }
		case EG_ALIGN_OUT_BOTTOM_MID:{
			X = ((EGObject*)pBase)->GetWidth() / 2 - GetWidth() / 2;
			Y = ((EGObject*)pBase)->GetHeight();
			break;
    }
		case EG_ALIGN_OUT_BOTTOM_RIGHT:{
			X = ((EGObject*)pBase)->GetWidth() - GetWidth();
			Y = GetHeight();
			break;
    }
		case EG_ALIGN_OUT_LEFT_TOP:{
			X = -GetWidth();
			Y = 0;
			break;
    }
		case EG_ALIGN_OUT_LEFT_MID:{
			X = -GetWidth();
			Y = ((EGObject*)pBase)->GetHeight() / 2 - GetHeight() / 2;
			break;
    }
		case EG_ALIGN_OUT_LEFT_BOTTOM:{
			X = -GetWidth();
			Y = ((EGObject*)pBase)->GetHeight() - GetHeight();
			break;
    }
		case EG_ALIGN_OUT_RIGHT_TOP:{
			X = ((EGObject*)pBase)->GetWidth();
			Y = 0;
			break;
    }
		case EG_ALIGN_OUT_RIGHT_MID:{
			X = ((EGObject*)pBase)->GetWidth();
			Y = ((EGObject*)pBase)->GetHeight() / 2 - GetHeight() / 2;
			break;
    }
		case EG_ALIGN_OUT_RIGHT_BOTTOM:{
			X = ((EGObject*)pBase)->GetWidth();
			Y = ((EGObject*)pBase)->GetHeight() - GetHeight();
			break;
    }
	}
	if(pParent->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) {
	  X += OffsetX + pBase->m_Rect.GetX1() - pParent->m_Rect.GetX1() + pParent->GetScrollRight() - ParentLeft;
	}
	else X += OffsetX + pBase->m_Rect.GetX1() - pParent->m_Rect.GetX1() + pParent->GetScrollLeft() - ParentLeft;
	Y += OffsetY + pBase->m_Rect.GetY1() - pParent->m_Rect.GetY1() + pParent->GetScrollTop() - ParentTop;
	SetStyleAlign(EG_ALIGN_TOP_LEFT, 0);
	SetPosition(X, Y);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetX(void)
{
EG_Coord_t Coord;
EGObject *pParent = GetParent();

	if(pParent) {
		Coord = m_Rect.GetX1() - pParent->m_Rect.GetX1();
		Coord += pParent->GetScrollX();
		Coord -= pParent->GetStylePadLeft(EG_PART_MAIN);
		Coord -= pParent->GetStyleBorderWidth(EG_PART_MAIN);
	}
	else Coord = m_Rect.GetX1();
	return Coord;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetX2(void)
{
	return GetX() + GetWidth();
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetY(void)
{
EG_Coord_t Coord;
EGObject *pParent = GetParent();

	if(pParent) {
		Coord = m_Rect.GetY1() - pParent->m_Rect.GetY1();
		Coord += pParent->GetScrollY();
		Coord -= pParent->GetStylePadTop(EG_PART_MAIN);
		Coord -= pParent->GetStyleBorderWidth(EG_PART_MAIN);
	}
	else Coord = m_Rect.GetY1();
	return Coord;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetY2(void)
{
	return GetY() + GetHeight();
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetAlignedX(void)
{
	return GetStyleX(EG_PART_MAIN);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetAlignedY(void)
{
	return GetStyleY(EG_PART_MAIN);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetWidth(void)
{
	return m_Rect.GetWidth();
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetHeight(void)
{
	return m_Rect.GetHeight();
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetContentWidth()
{
	EG_Coord_t Left = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t Right = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t Width = (m_Rect.GetWidth() - Left - Right - BorderWidth * 2);
  return Width;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetContentHeight()
{
	EG_Coord_t Top = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t Bottom = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	return m_Rect.GetHeight() - Top - Bottom - BorderWidth * 2;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::GetContentArea(EGRect *pRect)
{
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	m_Rect.Copy(pRect);
	pRect->Deflate(BorderWidth, BorderWidth);
	pRect->Deflate(GetStylePadLeft(EG_PART_MAIN), GetStylePadRight(EG_PART_MAIN), GetStylePadTop(EG_PART_MAIN), GetStylePadBottom(EG_PART_MAIN));
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetSelfWidth(void)
{
	EGPoint Point = {0, EG_COORD_MIN};
	EGEvent::EventSend((EGObject *)this, EG_EVENT_GET_SELF_SIZE, &Point);
	return Point.m_X;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetSelfHeight(void)
{
	EGPoint Point = {EG_COORD_MIN, 0};
	EGEvent::EventSend((EGObject *)this, EG_EVENT_GET_SELF_SIZE, &Point);
	return Point.m_Y;
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::RefreshSelfSize(void)
{
	EG_Coord_t StyleWidth = GetStyleWidth(EG_PART_MAIN);
	EG_Coord_t StyleHeight = GetStyleHeight(EG_PART_MAIN);
	if(StyleWidth != EG_SIZE_CONTENT && StyleHeight != EG_SIZE_CONTENT) return false;
	MarkLayoutDirty();
	return true;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::RefreshPosition(void)
{
	if(IsLayoutPositioned()) return;
	EGObject *pParent = GetParent();
	EG_Coord_t X = GetStyleX(EG_PART_MAIN);
	EG_Coord_t Y = GetStyleY(EG_PART_MAIN);
	if(pParent == nullptr) {
		MoveTo(X, Y);
		return;
	}
	EG_Coord_t ParentWidth = pParent->GetContentWidth();	// Handle percentage value
	EG_Coord_t ParentHeight = pParent->GetContentHeight();
	if(EG_COORD_IS_PCT(X)) X = (ParentWidth * EG_COORD_GET_PCT(X)) / 100;
	if(EG_COORD_IS_PCT(Y)) Y = (ParentHeight * EG_COORD_GET_PCT(Y)) / 100;
	EG_Coord_t TranslateX = GetStyleTranslateX(EG_PART_MAIN);	// Handle percentage value of translate
	EG_Coord_t TranslateY = GetStyleTranslateY(EG_PART_MAIN);
	EG_Coord_t Width = GetWidth();
	EG_Coord_t Height = GetHeight();
	if(EG_COORD_IS_PCT(TranslateX)) TranslateX = (Width * EG_COORD_GET_PCT(TranslateX)) / 100;
	if(EG_COORD_IS_PCT(TranslateY)) TranslateY = (Height * EG_COORD_GET_PCT(TranslateY)) / 100;
	X += TranslateX;	// Use the translation
	Y += TranslateY;
	EG_AlignType_e Align = GetStyleAlign(EG_PART_MAIN);
	if(Align == EG_ALIGN_DEFAULT) {
		if(pParent->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL)	Align = EG_ALIGN_TOP_RIGHT;
		else Align = EG_ALIGN_TOP_LEFT;
	}
  switch(Align) {
    case EG_ALIGN_TOP_MID:{
      X += ParentWidth / 2 - Width / 2;
      break;
    }
    case EG_ALIGN_TOP_RIGHT:{
      X += ParentWidth - Width;
      break;
    }
    case EG_ALIGN_LEFT_MID:{
      Y += ParentHeight / 2 - Height / 2;
      break;
    }
    case EG_ALIGN_BOTTOM_LEFT:{
      Y += ParentHeight - Height;
      break;
    }
    case EG_ALIGN_BOTTOM_MID:{
      X += ParentWidth / 2 - Width / 2;
      Y += ParentHeight - Height;
      break;
    }
    case EG_ALIGN_BOTTOM_RIGHT:{
      X += ParentWidth - Width;
      Y += ParentHeight - Height;
      break;
    }
    case EG_ALIGN_RIGHT_MID:{
      X += ParentWidth - Width;
      Y += ParentHeight / 2 - Height / 2;
      break;
    }
    case EG_ALIGN_CENTER:{
      X += ParentWidth / 2 - Width / 2;
      Y += ParentHeight / 2 - Height / 2;
      break;
    }
    case EG_ALIGN_TOP_LEFT:
    default:{
      break;
    }
  }
  MoveTo(X, Y);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::MoveTo(EG_Coord_t X, EG_Coord_t Y)
{
EGObject *pParent = GetParent();
bool Invalidate1 = false;

	if(pParent) {
		EG_Coord_t LeftPadding = pParent->GetStylePadLeft(EG_PART_MAIN);
		EG_Coord_t TopPadding = pParent->GetStylePadTop(EG_PART_MAIN);
		if(HasFlagSet(EG_OBJ_FLAG_FLOATING)) {
			X += LeftPadding + pParent->m_Rect.GetX1();
			Y += TopPadding + pParent->m_Rect.GetY1();
		}
		else {
			X += LeftPadding + pParent->m_Rect.GetX1() - pParent->GetScrollX();
			Y += TopPadding + pParent->m_Rect.GetY1() - pParent->GetScrollY();
		}
		EG_Coord_t BorderWidth = pParent->GetStyleBorderWidth(EG_PART_MAIN);
		X += BorderWidth;
		Y += BorderWidth;
	}
	EGPoint Adjustment;
	Adjustment.m_X = X - m_Rect.GetX1();	// Calculate and set the movement
	Adjustment.m_Y = Y - m_Rect.GetY1();
	// Do nothing if the position is not changed. It is very important else recursive positioning can occur without
	if((Adjustment.m_X == 0) && (Adjustment.m_Y == 0)) return;
	Invalidate();	// Invalidate the original area
	EGRect OriginalArea;
	m_Rect.Copy(&OriginalArea);	// Save the original coordinates
	EGRect ParentArea;	// Check if the object inside the pParent or not
	if(pParent) {
		pParent->GetContentArea(&ParentArea);
		// If the object is already out of the pParent and its position is changes invalidate scrollbars
		Invalidate1 = OriginalArea.IsInside(&ParentArea, 0);
		if(!Invalidate1) pParent->ScrollbarInvalidate();
	}
	m_Rect.Move(Adjustment.m_X, Adjustment.m_Y);
	MoveChildrenBy(Adjustment.m_X, Adjustment.m_Y, false);
	if(pParent) EGEvent::EventSend(pParent, EG_EVENT_CHILD_CHANGED, this);	// Call the ancestor's event handler to the pParent too
	Invalidate();	//Invalidate the new area

	// If the object was outside the pParent, invalidate the new scrollbar area too. If it wasn't but out now, also invalidate the scrollbars
	if(pParent){
		bool Invalidate2 = m_Rect.IsInside(&ParentArea, 0);
  	if(Invalidate1 || (!Invalidate1 && Invalidate2)) pParent->ScrollbarInvalidate();
	}
}


/////////////////////////////////////////////////////////////////////////////

void EGObject::MoveChildrenBy(EG_Coord_t AdjustmentX, EG_Coord_t AdjustmentY, bool IgnoreFloating)
{
uint32_t i;

  uint32_t ChildCount = GetChildCount();
	for(i = 0; i < ChildCount; i++) {
		EGObject *pChild = m_pAttributes->ppChildren[i];
		if(IgnoreFloating && pChild->HasFlagSet( EG_OBJ_FLAG_FLOATING)) continue;
		pChild->m_Rect.Move(AdjustmentX, AdjustmentY);
		pChild->MoveChildrenBy(AdjustmentX, AdjustmentY, false);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::TransformPoint(EGPoint *pPoint, bool Recursive, bool Invert)
{
bool DoTransform = false;
EG_LayerType_e LayerType;
EGObject *pParent;

  if((LayerType = GetLayerType()) == EG_LAYER_TYPE_TRANSFORM) DoTransform = true;
  if(Invert) {
    if(Recursive) if((pParent = GetParent()) != nullptr) pParent->TransformPoint(pPoint, Recursive, Invert);
    if(DoTransform) TransformCore(pPoint, Invert);
  }
  else {
    if(DoTransform) TransformCore(pPoint, Invert);
    if(Recursive) if((pParent = GetParent()) != nullptr) pParent->TransformPoint(pPoint, Recursive, Invert);
  }
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::GetTransformedArea(EGRect *pRect, bool Recursive, bool Invert)
{
EGPoint Point[4] = {
  {pRect->GetX1(), pRect->GetY1()},
  {pRect->GetX1(), pRect->GetY2()},
  {pRect->GetX2(), pRect->GetY1()},
  {pRect->GetX2(), pRect->GetY2()},
};

	TransformPoint(&Point[0], Recursive, Invert);
	TransformPoint(&Point[1], Recursive, Invert);
	TransformPoint(&Point[2], Recursive, Invert);
	TransformPoint(&Point[3], Recursive, Invert);
	pRect->SetX1(EG_MIN4(Point[0].m_X, Point[1].m_X, Point[2].m_X, Point[3].m_X));
	pRect->SetX2(EG_MAX4(Point[0].m_X, Point[1].m_X, Point[2].m_X, Point[3].m_X));
	pRect->SetY1(EG_MIN4(Point[0].m_Y, Point[1].m_Y, Point[2].m_Y, Point[3].m_Y));
	pRect->SetY2(EG_MAX4(Point[0].m_Y, Point[1].m_Y, Point[2].m_Y, Point[3].m_Y));
	pRect->Inflate(5, 5);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::InvalidateArea(const EGRect *pRect)
{
	EGDisplay *pDisplay = GetDisplay();
	if(!EGDisplay::IsInvalidationEnabled(pDisplay)) return;
	EGRect RectTemp(pRect);
	if(!AreaIsVisible(&RectTemp)) return;
	InvalidateRect(pDisplay, &RectTemp);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::Invalidate(void)
{
	EG_Coord_t ExtenSize = GetExtDrawSize();
  EGRect Area(m_Rect);
  Area.Inflate(ExtenSize, ExtenSize);
	InvalidateArea(&Area);
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::AreaIsVisible(EGRect *pRect)
{
	if(HasFlagSet(EG_OBJ_FLAG_HIDDEN)) return false;
	// Invalidate the object only if it belongs to the current or previous or one of the layers
	EGObject *pScreen = GetScreen();
	EGDisplay *pDisplay = pScreen->GetDisplay();
	if((pScreen != EGDisplay::GetActiveScreen(pDisplay)) && (pScreen != EGDisplay::GetPrevoiusScreen(pDisplay)) &&
		 (pScreen != EGDisplay::GetTopLayer(pDisplay)) && (pScreen != EGDisplay::GetSystemLayer(pDisplay))) return false;
	if(!HasAnyFlagSet(EG_OBJ_FLAG_OVERFLOW_VISIBLE)){	// Truncate the area to the object
		EG_Coord_t ExtenSize = GetExtDrawSize();
		EGRect Area(m_Rect);
    Area.Inflate(ExtenSize, ExtenSize);
		if(!pRect->Intersect(pRect, &Area)) return false;		// return if the area is not on the object
	}
	GetTransformedArea(pRect, true, false);
	EGObject *pParent = GetParent();	      // Truncate recursively to the parents
	while(pParent != nullptr) {	// If the pParent is hidden then the child is hidden and won't be drawn
		if(pParent->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN)) return false;
		if(!pParent->HasAnyFlagSet(EG_OBJ_FLAG_OVERFLOW_VISIBLE)){ // Truncate to the pParent and if no common parts break
			EGRect ParentRect = pParent->m_Rect;
			((EGObject*)pParent)->GetTransformedArea(&ParentRect, true, false);
			if(!pRect->Intersect(pRect, &ParentRect)) return false;
		}
		pParent = pParent->GetParent();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::IsVisible(void)
{
	EG_Coord_t ExtenSize = GetExtDrawSize();
	EGRect Area(m_Rect);
  Area.Inflate(ExtenSize, ExtenSize);
	return AreaIsVisible(&Area);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetExtClickArea(EG_Coord_t Size)
{
	AllocateAttribute();
	m_pAttributes->ExtendedClickPadding = Size;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::GetClickArea(EGRect *pRect)
{
	m_Rect.Copy(pRect);
	if(m_pAttributes) pRect->Inflate(m_pAttributes->ExtendedClickPadding, m_pAttributes->ExtendedClickPadding);
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::HitTest(const EGPoint *pPoint)
{
EGRect Rect;

	if(!HasFlagSet(EG_OBJ_FLAG_CLICKABLE)) return false;
	if(HasState(EG_STATE_DISABLED)) return false;
	GetClickArea(&Rect);
	if(Rect.IsPointIn(pPoint, 0) == false) return false;
	if(HasFlagSet(EG_OBJ_FLAG_ADV_HITTEST)) {
		EG_HitTestState_t HitInfo;
		HitInfo.pPoint = pPoint;
		HitInfo.Result = true;
		EGEvent::EventSend(this, EG_EVENT_HIT_TEST, &HitInfo);
		return HitInfo.Result;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::ClampWidth(EG_Coord_t width, EG_Coord_t min_width, EG_Coord_t max_width, EG_Coord_t ref_width)
{
	if(EG_COORD_IS_PCT(min_width)) min_width = (ref_width * EG_COORD_GET_PCT(min_width)) / 100;
	if(EG_COORD_IS_PCT(max_width)) max_width = (ref_width * EG_COORD_GET_PCT(max_width)) / 100;
	return EG_CLAMP(min_width, width, max_width);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::ClampHeight(EG_Coord_t height, EG_Coord_t min_height, EG_Coord_t max_height, EG_Coord_t ref_height)
{
	if(EG_COORD_IS_PCT(min_height)) min_height = (ref_height * EG_COORD_GET_PCT(min_height)) / 100;
	if(EG_COORD_IS_PCT(max_height)) max_height = (ref_height * EG_COORD_GET_PCT(max_height)) / 100;
	return EG_CLAMP(min_height, height, max_height);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::CalcContentWidth(void)
{
uint32_t i;

	ScrollToX(0, EG_ANIM_OFF);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t RightPadding = GetStylePadRight(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t LeftPadding = GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t Width = GetSelfWidth() + LeftPadding + RightPadding;
	EG_Coord_t ChildResolution = EG_COORD_MIN;
	uint32_t ChildCount = GetChildCount();
	// With RTL find the left most coordinate
	if(GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) {
		for(i = 0; i < ChildCount; i++) {
			EGObject *pChild = m_pAttributes->ppChildren[i];
			if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
			if(!((EGObject*)pChild)->IsLayoutPositioned()) {
				EG_AlignType_e Align = pChild->GetStyleAlign(0);
				switch(Align){
					case EG_ALIGN_DEFAULT:
					case EG_ALIGN_TOP_RIGHT:
					case EG_ALIGN_BOTTOM_RIGHT:
					case EG_ALIGN_RIGHT_MID:
						/*Normal right aligns. Other are ignored due to possible circular dependencies*/
						ChildResolution = EG_MAX(ChildResolution, m_Rect.GetX2() - pChild->m_Rect.GetX1() + 1);
						break;
					default:
						// Consider other cases only if x=0 and use the width of the object. With x!=0 circular dependency could occur. 
						if(pChild->GetStyleX(0) == 0) {
							ChildResolution = EG_MAX(ChildResolution, pChild->GetWidth() + RightPadding);
						}
				}
			}
			else {
				ChildResolution = EG_MAX(ChildResolution, m_Rect.GetX2() - pChild->m_Rect.GetX1() + 1);
			}
		}
		if(ChildResolution != EG_COORD_MIN) {
			ChildResolution += LeftPadding;
		}
	}
	else {	// Else find the right most coordinate
		for(i = 0; i < ChildCount; i++) {
			EGObject *pChild = m_pAttributes->ppChildren[i];
			if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
			if(!((EGObject*)pChild)->IsLayoutPositioned()) {
				EG_AlignType_e Align = GetStyleAlign(0);
				switch(Align) {
					case EG_ALIGN_DEFAULT:
					case EG_ALIGN_TOP_LEFT:
					case EG_ALIGN_BOTTOM_LEFT:
					case EG_ALIGN_LEFT_MID:{
						// Normal left aligns.
						ChildResolution = EG_MAX(ChildResolution, pChild->m_Rect.GetX2() - m_Rect.GetX1() + 1);
						break;
          }
					default:{
						// Consider other cases only if x=0 and use the width of the object. With x!=0 circular dependency could occur. 
						if(pChild->GetStyleY(0) == 0) {
							ChildResolution = EG_MAX(ChildResolution, pChild->m_Rect.GetWidth() + LeftPadding);
						}
            break;
          }
				}
			}
			else {
				ChildResolution = EG_MAX(ChildResolution, pChild->m_Rect.GetX2() - m_Rect.GetX1() + 1);
			}
		}
		if(ChildResolution != EG_COORD_MIN) {
			ChildResolution += RightPadding;
		}
	}
	if(ChildResolution == EG_COORD_MIN)	return Width;
  return EG_MAX(ChildResolution, Width);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::CalcContentHeight(void)
{
	ScrollToY(0, EG_ANIM_OFF);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t TopPadding = GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t BottomPadding = GetStylePadBottom(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t Height = GetSelfHeight() + TopPadding + BottomPadding;
	EG_Coord_t ChildResolution = EG_COORD_MIN;
	uint32_t ChildCount = GetChildCount();
	for(uint32_t i = 0; i < ChildCount; i++) {
		EGObject *pChild = m_pAttributes->ppChildren[i];
		if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
		if(!(pChild)->IsLayoutPositioned()) {
			EG_AlignType_e Align = pChild->GetStyleAlign(0);
			switch(Align) {
				case EG_ALIGN_DEFAULT:
				case EG_ALIGN_TOP_RIGHT:
				case EG_ALIGN_TOP_MID:
				case EG_ALIGN_TOP_LEFT:		// Normal top aligns. 
					ChildResolution = EG_MAX(ChildResolution, pChild->m_Rect.GetY2() - m_Rect.GetY1() + 1);
					break;
				default:
					// Consider other cases only if y=0 and use the height of the object. With y!=0 circular dependency could occur. 
					if(pChild->GetStyleY(0) == 0) {
						ChildResolution = EG_MAX(ChildResolution, pChild->m_Rect.GetHeight() + TopPadding);
					}
					break;
			}
		}
		else {
			ChildResolution = EG_MAX(ChildResolution, pChild->m_Rect.GetY2() - m_Rect.GetY1() + 1);
		}
	}
	if(ChildResolution != EG_COORD_MIN) {
		ChildResolution += BottomPadding;
		return EG_MAX(ChildResolution, Height);
	}
	return Height;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::LayoutUpdateCore(void)
{
	uint32_t ChildCount = GetChildCount();
	for(uint32_t i = 0; i < ChildCount; i++) {  // Refresh children layout first
		EGObject *pChild = m_pAttributes->ppChildren[i];
		((EGObject*)pChild)->LayoutUpdateCore();
	}
	if(m_LayoutInvalid) {
		m_LayoutInvalid = 0;
 		RefreshSize();
		RefreshPosition();
		if(ChildCount > 0) {
			uint32_t LayoutReference = GetStyleLayout(EG_PART_MAIN);
      if(LayoutReference > 0){
			  EG_LayoutDiscriptor *pLayout  = (EG_LayoutDiscriptor*)m_LayoutList.GetAt((POSITION)LayoutReference);
        void *pUserData = pLayout->pUserData;     // get the layout to update
  		  pLayout->UpdateCB(this, pUserData);       // call the update function
      }
		}
	}
	if(m_ReadScrollAfterLayout){
		m_ReadScrollAfterLayout = 0;
		ReadjustScroll(EG_ANIM_OFF);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::TransformCore(EGPoint *pPoint, bool Invert)
{
	int16_t Angle = GetStyleTransformAngle(0);
	int16_t Zoom = GetStyleTransformZoom( 0);
	if((Angle == 0) && (Zoom == EG_SCALE_NONE)) return;
	EGPoint Pivot(GetStyleTransformPivotX(0), GetStyleTransformPivotY(0));
	if(EG_COORD_IS_PCT(Pivot.m_X)) {
		Pivot.m_X = (EG_COORD_GET_PCT(Pivot.m_X) * m_Rect.GetWidth()) / 100;
	}
	if(EG_COORD_IS_PCT(Pivot.m_Y)) {
		Pivot.m_Y = (EG_COORD_GET_PCT(Pivot.m_Y) * m_Rect.GetHeight()) / 100;
	}
	Pivot.m_X = m_Rect.GetX1() + Pivot.m_X;
	Pivot.m_Y = m_Rect.GetY1() + Pivot.m_Y;
	if(Invert){
		Angle = -Angle;
		Zoom = (256 * 256) / Zoom;
	}
	pPoint->PointTransform(Angle, EGScale(Zoom), &Pivot);
}


