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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */
 
#include "core/EG_Object.h"
#include "core/EG_ObjScroll.h"
#include "core/EG_InputDevice.h"
#include "core/EG_Display.h"

/////////////////////////////////////////////////////////////////////////////

#define OBJ_CLASS &c_ObjectClass
#define SCROLL_ANIM_TIME_MIN 200 //ms
#define SCROLL_ANIM_TIME_MAX 400 //ms
#define SCROLLBAR_MIN_SIZE (EG_DPX(10))

/////////////////////////////////////////////////////////////////////////////


void EGObject::SetScrollbarMode(EG_ScrollbarMode_e Mode)
{
	AllocateAttribute();
	if(m_pAttributes->ScrollbarMode == Mode) return;
	m_pAttributes->ScrollbarMode = Mode;
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetScrollDirection(EG_DirType_e Direction)
{
	AllocateAttribute();
	if(Direction != m_pAttributes->ScrollDirection) {
		m_pAttributes->ScrollDirection = Direction;
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetScrollSnapX(EG_ScrollSnap_e Align)
{
	AllocateAttribute();
	m_pAttributes->ScrollSnapX = Align;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::SetScrollSnapY(EG_ScrollSnap_e Align)
{
	AllocateAttribute();
	m_pAttributes->ScrollSnapY = Align;
}

/////////////////////////////////////////////////////////////////////////////

EG_ScrollbarMode_e EGObject::GetScrollbarMode(void)
{
	if(m_pAttributes)	return m_pAttributes->ScrollbarMode;
	else return EG_SCROLLBAR_MODE_AUTO;
}

/////////////////////////////////////////////////////////////////////////////

EG_DirType_e EGObject::GetScrollDirection(void)
{
	if(m_pAttributes)	return m_pAttributes->ScrollDirection;
	else return EG_DIR_ALL;
}

/////////////////////////////////////////////////////////////////////////////

EG_ScrollSnap_e EGObject::GetScrollSnapX(void)
{
	if(m_pAttributes)	return m_pAttributes->ScrollSnapX;
	else return EG_SCROLL_SNAP_NONE;
}

/////////////////////////////////////////////////////////////////////////////

EG_ScrollSnap_e EGObject::GetScrollSnapY(void)
{
	if(m_pAttributes) return m_pAttributes->ScrollSnapY;
	else return EG_SCROLL_SNAP_NONE;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetScrollX(void)
{
	if(m_pAttributes == nullptr) return 0;
	return -m_pAttributes->pScroll->m_X;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetScrollY(void)
{
	if(m_pAttributes == nullptr) return 0;
	return -m_pAttributes->pScroll->m_Y;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetScrollTop(void)
{
	if(m_pAttributes == nullptr) return 0;
	return -m_pAttributes->pScroll->m_Y;
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetScrollBottom(void)
{
EG_Coord_t ChildRes = EG_COORD_MIN;
uint32_t i;

	uint32_t ChildCount = GetChildCount();
	for(i = 0; i < ChildCount; i++) {
		EGObject *pChild = m_pAttributes->ppChildren[i];
		if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
		ChildRes = EG_MAX(ChildRes, pChild->m_Rect.GetY2());
	}
	EG_Coord_t TopPad = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t BottomPad = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	if(ChildRes != EG_COORD_MIN){
		ChildRes -= (m_Rect.GetY2() - BottomPad - BorderWidth);
	}
	EG_Coord_t self_h = GetSelfHeight();
	self_h = self_h - (GetHeight() - TopPad - BottomPad - 2 * BorderWidth);
	self_h -= GetScrollY();
	return EG_MAX(ChildRes, self_h);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetScrollLeft(void)
{
EG_Coord_t ChildRes = 0;

	// Normally can't scroll the object out on the left. So simply use the current scroll position as "left size"
	if(GetStyleBaseDirection(EG_PART_MAIN) != EG_BASE_DIR_RTL) {
		if(m_pAttributes == nullptr) return 0;
		return -m_pAttributes->pScroll->m_X;
	}
	// With RTL base direction scrolling the left is normal so find the left most coordinate
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t x1 = EG_COORD_MAX;
	uint32_t ChildCount = GetChildCount();
	for(uint32_t i = 0; i < ChildCount; i++) {
		EGObject *pChild = m_pAttributes->ppChildren[i];
		if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
		x1 = EG_MIN(x1, pChild->m_Rect.GetX1());
	}
	if(x1 != EG_COORD_MAX) {
		ChildRes = x1;
		ChildRes = (m_Rect.GetX1() + PadLeft + BorderWidth) - ChildRes;
	}
	else ChildRes = EG_COORD_MIN;
	EG_Coord_t self_w = GetSelfWidth();
	self_w = self_w - (GetWidth() - PadRight - PadLeft - 2 * BorderWidth);
	self_w += GetScrollX();
	return EG_MAX(ChildRes, self_w);
}

/////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGObject::GetScrollRight(void)
{
	// With RTL base Direction can't scroll to the object out on the right. So simply use the current scroll position as "right size"
	if(GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) {
		if(m_pAttributes == nullptr) return 0;
		return m_pAttributes->pScroll->m_X;
	}
	//With other base direction (LTR) scrolling to the right is normal so find the right most coordinate
	EG_Coord_t ChildRes = EG_COORD_MIN;
	uint32_t i;
	uint32_t ChildCount = GetChildCount();
	for(i = 0; i < ChildCount; i++) {
		EGObject *pChild = m_pAttributes->ppChildren[i];
		if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
		ChildRes = EG_MAX(ChildRes, pChild->m_Rect.GetX2());
	}
	EG_Coord_t PadRight = GetStylePadRight(EG_PART_MAIN);
	EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	if(ChildRes != EG_COORD_MIN) ChildRes -= (m_Rect.GetX2() - PadRight - BorderWidth);
	EG_Coord_t SelfWidth = GetSelfWidth();
	SelfWidth = SelfWidth - (GetWidth() - PadRight - PadLeft - 2 * BorderWidth);
	SelfWidth -= GetScrollX();
	return EG_MAX(ChildRes, SelfWidth);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::GetScrollEnd(EGPoint *pEnd)
{
EGAnimate *pAnimate;

	pAnimate = EGAnimate::Get(this, ScrollAnimatedX);
	pEnd->m_X = pAnimate ? -pAnimate->m_EndValue : GetScrollX();
	pAnimate = EGAnimate::Get(this, ScrollAnimatedY);
	pEnd->m_Y = pAnimate ? -pAnimate->m_EndValue : GetScrollY();
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollByBounded(EG_Coord_t SizeX, EG_Coord_t SizeY, EG_AnimateEnable_e AnimateEnable)
{
	if(SizeX == 0 && SizeY == 0) return;
	UpdateLayout();	                                  // We need to know the final sizes for bound check
	EG_Coord_t CurrentX = -GetScrollX();	// Don't let scroll more then naturally possible by the size of the content
	EG_Coord_t BoundedX = CurrentX + SizeX;

	if(GetStyleBaseDirection(EG_PART_MAIN) != EG_BASE_DIR_RTL) {
		if(BoundedX > 0) BoundedX = 0;
		if(BoundedX < 0) {
			EG_Coord_t scroll_max = GetScrollLeft() + GetScrollRight();
			if(scroll_max < 0) scroll_max = 0;
			if(BoundedX < -scroll_max) BoundedX = -scroll_max;
		}
	}
	else {
		if(BoundedX < 0) BoundedX = 0;
		if(BoundedX > 0) {
			EG_Coord_t scroll_max = GetScrollLeft() + GetScrollRight();
			if(scroll_max < 0) scroll_max = 0;
			if(BoundedX > scroll_max) BoundedX = scroll_max;
		}
	}
	EG_Coord_t CurrentY = -GetScrollY();	// Don't let scroll more then naturally possible by the size of the content
	EG_Coord_t BoundedY = CurrentY + SizeY;

	if(BoundedY > 0) BoundedY = 0;
	if(BoundedY < 0) {
		EG_Coord_t ScrollMax = GetScrollTop() + GetScrollBottom();
		if(ScrollMax < 0) ScrollMax = 0;
		if(BoundedY < -ScrollMax) BoundedY = -ScrollMax;
	}
	SizeX = BoundedX - CurrentX;
	SizeY = BoundedY - CurrentY;
	if(SizeX || SizeY) ScrollBy(SizeX, SizeY, AnimateEnable);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollBy(EG_Coord_t SizeX, EG_Coord_t SizeY, EG_AnimateEnable_e AnimateEnable)
{
	if(SizeX == 0 && SizeY == 0) return;
	if(AnimateEnable == EG_ANIM_ON){
		EGDisplay *pDisplay = GetDisplay();
		EGAnimate Animate;
		Animate.SetItem(this);
		Animate.SetEndCB(ScrollAnimatedEndCB);
		if(SizeX) {
			uint32_t Time = EGAnimate::SpeedToTime((pDisplay->GetHorizontalRes() * 2) >> 2, 0, SizeX);
			if(Time < SCROLL_ANIM_TIME_MIN) Time = SCROLL_ANIM_TIME_MIN;
			if(Time > SCROLL_ANIM_TIME_MAX) Time = SCROLL_ANIM_TIME_MAX;
			Animate.SetTime(Time);
			EG_Coord_t ScrollX = GetScrollX();
			Animate.SetValues(-ScrollX, -ScrollX + SizeX);
			Animate.SetExcCB(ScrollAnimatedX);
			Animate.SetPathCB(EGAnimate::PathEaseOut);
			if(EGEvent::EventSend(this, EG_EVENT_SCROLL_BEGIN, &Animate) != EG_RES_OK) return;
			EGAnimate::Create(&Animate);
		}
		if(SizeY) {
			uint32_t Time = EGAnimate::SpeedToTime((pDisplay->GetVerticalRes() * 2) >> 2, 0, SizeY);
			if(Time < SCROLL_ANIM_TIME_MIN) Time = SCROLL_ANIM_TIME_MIN;
			if(Time > SCROLL_ANIM_TIME_MAX) Time = SCROLL_ANIM_TIME_MAX;
			Animate.SetTime(Time);
			EG_Coord_t ScrollX = GetScrollY();
			Animate.SetValues(-ScrollX, -ScrollX + SizeY);
			Animate.SetExcCB(ScrollAnimatedY);
			Animate.SetPathCB(EGAnimate::PathEaseOut);
  		if(EGEvent::EventSend(this, EG_EVENT_SCROLL_BEGIN, &Animate) != EG_RES_OK) return;
			EGAnimate::Create(&Animate);
		}
	}
	else {
		EGAnimate::Delete(this, ScrollAnimatedY);		// Remove pending animations
		EGAnimate::Delete(this, ScrollAnimatedX);
		if(EGEvent::EventSend(this, EG_EVENT_SCROLL_BEGIN, nullptr) != EG_RES_OK) return;
		if(ScrollByRaw(SizeX, SizeY) != EG_RES_OK) return;
		EGEvent::EventSend(this, EG_EVENT_SCROLL_END, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollTo(EG_Coord_t PosX, EG_Coord_t PosY, EG_AnimateEnable_e AnimateEnable)
{
	ScrollToX(PosX, AnimateEnable);
	ScrollToY(PosY, AnimateEnable);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollToX(EG_Coord_t PosX, EG_AnimateEnable_e AnimateEnable)
{
	EGAnimate::Delete(this, ScrollAnimatedX);
	EG_Coord_t ScrollX = GetScrollX();
	EG_Coord_t Diff = -PosX + ScrollX;
	ScrollByBounded(Diff, 0, AnimateEnable);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollToY(EG_Coord_t PosY, EG_AnimateEnable_e AnimateEnable)
{
	EGAnimate::Delete(this, ScrollAnimatedY);
	EG_Coord_t ScrollY = GetScrollY();
	EG_Coord_t Diff = -PosY + ScrollY;
	ScrollByBounded(0, Diff, AnimateEnable);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollToView(EG_AnimateEnable_e AnimateEnable)
{
	UpdateLayout();	// Be sure the screens layout is correct
	EGPoint Point = {0, 0};
	ScrollAreaIntoView(&m_Rect, this, &Point, AnimateEnable);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollToViewRecursive(EG_AnimateEnable_e AnimateEnable)
{
	UpdateLayout();	// Be sure the screens layout is correct
	EGPoint Point;
	EGObject *pChild = this;
	EGObject *pParent = GetParent();
	while(pParent){
		ScrollAreaIntoView(&m_Rect, pChild, &Point, AnimateEnable);
		pChild = pParent;
		pParent = pChild->GetParent();
	}
}

/////////////////////////////////////////////////////////////////////////////

EG_Result_t EGObject::ScrollByRaw(EG_Coord_t PosX, EG_Coord_t PosY)
{
	if(PosX == 0 && PosY == 0) return EG_RES_OK;
	AllocateAttribute();
	m_pAttributes->pScroll->m_X += PosX;
	m_pAttributes->pScroll->m_Y += PosY;
	MoveChildrenBy(PosX, PosY, true);
	EG_Result_t Res = EGEvent::EventSend(this, EG_EVENT_SCROLL, nullptr);
	if(Res != EG_RES_OK) return Res;
	Invalidate();
	return EG_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////

bool EGObject::IsScrolling(void)
{
	EGInputDevice *pIndev = EGInputDevice::GetNext(nullptr);
	while(pIndev) {
		if(pIndev->GetScrollObj() == this) return true;
		pIndev = EGInputDevice::GetNext(pIndev);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::UpdateSnap(EG_AnimateEnable_e AnimateEnable)
{
EGPoint Point;

	UpdateLayout();
	EGInputDevice::ScrollGetSnapDist(this, &Point);
	ScrollBy(Point.m_X, Point.m_Y, AnimateEnable);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::GetScrollbarArea(EGRect *pHorizontalArea, EGRect *pVerticalArea)
{
bool DrawVirtical = false, DrawHorizontal = false;
EG_Coord_t Remaining;

	pHorizontalArea->Set(0, 0, -1, -1);
	pVerticalArea->Set(0, 0, -1, -1);
	if(HasFlagSet(EG_OBJ_FLAG_SCROLLABLE) == false) return;
	EG_ScrollbarMode_e ScrollMode = GetScrollbarMode();
	if(ScrollMode == EG_SCROLLBAR_MODE_OFF) return;
	EGInputDevice *pIndev = EGInputDevice::GetNext(nullptr);
	if(ScrollMode == EG_SCROLLBAR_MODE_ACTIVE) {
		while(pIndev) {
			if(pIndev->GetScrollObj() == this) break;
			pIndev = EGInputDevice::GetNext(pIndev);
		}
		if(pIndev == nullptr) return;
	}
	EG_Coord_t ScrollTop = GetScrollTop();
	EG_Coord_t ScrollBottom = GetScrollBottom();
	EG_Coord_t Scrollleft = GetScrollLeft();
	EG_Coord_t ScrollRight = GetScrollRight();
	EG_DirType_e Direction = GetScrollDirection();
	if((Direction & EG_DIR_VER) &&
		 ((ScrollMode == EG_SCROLLBAR_MODE_ON) ||
			(ScrollMode == EG_SCROLLBAR_MODE_AUTO && (ScrollTop > 0 || ScrollBottom > 0)) ||
			(ScrollMode == EG_SCROLLBAR_MODE_ACTIVE && pIndev->GetScrollDirection() == EG_DIR_VER))) {
		DrawVirtical = true;
	}
	if((Direction & EG_DIR_HOR) &&
		 ((ScrollMode == EG_SCROLLBAR_MODE_ON) ||
			(ScrollMode == EG_SCROLLBAR_MODE_AUTO && (Scrollleft > 0 || ScrollRight > 0)) ||
			(ScrollMode == EG_SCROLLBAR_MODE_ACTIVE && pIndev->GetScrollDirection() == EG_DIR_HOR))) {
		DrawHorizontal = true;
	}
	if(!DrawHorizontal && !DrawVirtical) return;
	bool RightToleft = GetStyleBaseDirection(EG_PART_SCROLLBAR) == EG_BASE_DIR_RTL ? true : false;
	EG_Coord_t TopSpace = GetStylePadTop(EG_PART_SCROLLBAR);
	EG_Coord_t BottomSpace = GetStylePadBottom(EG_PART_SCROLLBAR);
	EG_Coord_t LeftSpace = GetStylePadLeft(EG_PART_SCROLLBAR);
	EG_Coord_t RightSpace = GetStylePadRight(EG_PART_SCROLLBAR);
	EG_Coord_t Thickness = GetStyleWidth(EG_PART_SCROLLBAR);
	EG_Coord_t Height = GetHeight();
	EG_Coord_t Width = GetWidth();
	EG_Coord_t SpaceReqVertical = DrawVirtical ? Thickness : 0;	// Space required for the vertical and horizontal scrollbars
	EG_Coord_t SpaceReqHorizontal = DrawHorizontal ? Thickness : 0;
	if(GetStyleBckgroundOPA(EG_PART_SCROLLBAR) < EG_OPA_MIN && GetStyleBorderOPA(EG_PART_SCROLLBAR) < EG_OPA_MIN) return;
	EG_Coord_t ContentHeight = Height + ScrollTop + ScrollBottom;	// Draw vertical scrollbar if the mode is ON or can be scrolled in this direction
	if(DrawVirtical && ContentHeight){
		pVerticalArea->SetY1(m_Rect.GetY1());
		pVerticalArea->SetY2(m_Rect.GetY2());
		if(RightToleft){
			pVerticalArea->SetX1(m_Rect.GetX1() + LeftSpace);
			pVerticalArea->SetX2(pVerticalArea->GetX1() + Thickness - 1);
		}
		else{
			pVerticalArea->SetX2(m_Rect.GetX2() - RightSpace);
			pVerticalArea->SetX1(pVerticalArea->GetX2() - Thickness + 1);
		}
		EG_Coord_t ScrollbarHeight = ((Height - TopSpace - BottomSpace - SpaceReqHorizontal) * Height) / ContentHeight;
		ScrollbarHeight = EG_MAX(ScrollbarHeight, SCROLLBAR_MIN_SIZE);
		Remaining = (Height - TopSpace - BottomSpace - SpaceReqHorizontal) -	ScrollbarHeight; // Remaining size from the scrollbar track that is not the scrollbar itself
		EG_Coord_t ScrollHeight = ContentHeight - Height; // The size of the content which can be really scrolled
		if(ScrollHeight <= 0) {
			pVerticalArea->SetY1(m_Rect.GetY1() + TopSpace);
			pVerticalArea->SetY2(m_Rect.GetY2() - BottomSpace - SpaceReqHorizontal - 1);
		}
		else {
			EG_Coord_t ScrollbarY = (Remaining * ScrollBottom) / ScrollHeight;
			ScrollbarY = Remaining - ScrollbarY;
			pVerticalArea->SetY1(m_Rect.GetY1() + ScrollbarY + TopSpace);
			pVerticalArea->SetY2(pVerticalArea->GetY1() + ScrollbarHeight - 1);
			if(pVerticalArea->GetY1() < m_Rect.GetY1() + TopSpace) {
				pVerticalArea->SetY1(m_Rect.GetY1() + TopSpace);
				if(pVerticalArea->GetY1() + SCROLLBAR_MIN_SIZE > pVerticalArea->GetY2()) {
					pVerticalArea->SetY2(pVerticalArea->GetY1() + SCROLLBAR_MIN_SIZE);
				}
			}
			if(pVerticalArea->GetY2() > m_Rect.GetY2() - SpaceReqHorizontal - BottomSpace) {
				pVerticalArea->SetY2(m_Rect.GetY2() - SpaceReqHorizontal - BottomSpace);
				if(pVerticalArea->GetY2() - SCROLLBAR_MIN_SIZE < pVerticalArea->GetY1()) {
					pVerticalArea->SetY1(pVerticalArea->GetY2() - SCROLLBAR_MIN_SIZE);
				}
			}
		}
	}
	//Draw horizontal scrollbar if the mode is ON or can be scrolled in this direction
	EG_Coord_t ContentWidth = Width + Scrollleft + ScrollRight;
	if(DrawHorizontal && ContentWidth) {
		pHorizontalArea->SetY2(m_Rect.GetY2() - BottomSpace);
		pHorizontalArea->SetY1(pHorizontalArea->GetY2() - Thickness + 1);
		pHorizontalArea->SetX1(m_Rect.GetX1());
		pHorizontalArea->SetX2(m_Rect.GetX2());

		EG_Coord_t ScrollbarWidth = ((Width - LeftSpace - RightSpace - SpaceReqVertical) * Width) / ContentWidth;
		ScrollbarWidth = EG_MAX(ScrollbarWidth, SCROLLBAR_MIN_SIZE);
		Remaining = (Width - LeftSpace - RightSpace - SpaceReqVertical) -	ScrollbarWidth; // Remaining size from the scrollbar track that is not the scrollbar itself
		EG_Coord_t ScrollWidth = ContentWidth - Width; //The size of the content which can be really scrolled
		if(ScrollWidth <= 0) {
			if(RightToleft) {
				pHorizontalArea->SetX1(m_Rect.GetX1() + LeftSpace + SpaceReqVertical - 1);
				pHorizontalArea->SetX2(m_Rect.GetX2() - RightSpace);
			}
			else {
				pHorizontalArea->SetX1(m_Rect.GetX1() + LeftSpace);
				pHorizontalArea->SetX2(m_Rect.GetX2() - RightSpace - SpaceReqVertical - 1);
			}
		}
		else {
			EG_Coord_t ScrollbarX = (Remaining * ScrollRight) / ScrollWidth;
			ScrollbarX = Remaining - ScrollbarX;
			if(RightToleft) {
				pHorizontalArea->SetX1(m_Rect.GetX1() + ScrollbarX + LeftSpace + SpaceReqVertical);
				pHorizontalArea->SetX2(pHorizontalArea->GetX1() + ScrollbarWidth - 1);
				if(pHorizontalArea->GetX1() < m_Rect.GetX1() + LeftSpace + SpaceReqVertical) {
					pHorizontalArea->SetX1(m_Rect.GetX1() + LeftSpace + SpaceReqVertical);
					if(pHorizontalArea->GetX1() + SCROLLBAR_MIN_SIZE > pHorizontalArea->GetX2()) {
						pHorizontalArea->SetX2(pHorizontalArea->GetX1() + SCROLLBAR_MIN_SIZE);
					}
				}
				if(pHorizontalArea->GetX2() > m_Rect.GetX2() - RightSpace) {
					pHorizontalArea->SetX2(m_Rect.GetX2() - RightSpace);
					if(pHorizontalArea->GetX2() - SCROLLBAR_MIN_SIZE < pHorizontalArea->GetX1()) {
						pHorizontalArea->SetX1(pHorizontalArea->GetX2() - SCROLLBAR_MIN_SIZE);
					}
				}
			}
			else {
				pHorizontalArea->SetX1(m_Rect.GetX1() + ScrollbarX + LeftSpace);
				pHorizontalArea->SetX2(pHorizontalArea->GetX1() + ScrollbarWidth - 1);
				if(pHorizontalArea->GetX1() < m_Rect.GetX1() + LeftSpace) {
					pHorizontalArea->SetX1(m_Rect.GetX1() + LeftSpace);
					if(pHorizontalArea->GetX1() + SCROLLBAR_MIN_SIZE > pHorizontalArea->GetX2()) {
						pHorizontalArea->SetX2(pHorizontalArea->GetX1() + SCROLLBAR_MIN_SIZE);
					}
				}
				if(pHorizontalArea->GetX2() > m_Rect.GetX2() - SpaceReqVertical - RightSpace) {
					pHorizontalArea->SetX2(m_Rect.GetX2() - SpaceReqVertical - RightSpace);
					if(pHorizontalArea->GetX2() - SCROLLBAR_MIN_SIZE < pHorizontalArea->GetX1()) {
						pHorizontalArea->SetX1(pHorizontalArea->GetX2() - SCROLLBAR_MIN_SIZE);
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollbarInvalidate(void)
{
EGRect HorizontalArea, VerticalArea;

	GetScrollbarArea(&HorizontalArea, &VerticalArea);
	if(HorizontalArea.GetSize() <= 0 && VerticalArea.GetSize() <= 0) return;
	if(HorizontalArea.GetSize() > 0) InvalidateArea(&HorizontalArea);
	if(VerticalArea.GetSize() > 0) InvalidateArea(&VerticalArea);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ReadjustScroll(EG_AnimateEnable_e AnimateEnable)
{
	if(GetScrollSnapY() == EG_SCROLL_SNAP_NONE) {
		EG_Coord_t ScrollTop = GetScrollTop();
		EG_Coord_t ScrollBottom = GetScrollBottom();
		if(ScrollBottom < 0 && ScrollTop > 0) {
			ScrollBottom = EG_MIN(ScrollTop, -ScrollBottom);
			ScrollBy(0, ScrollBottom, AnimateEnable);
		}
	}

	if(GetScrollSnapX() == EG_SCROLL_SNAP_NONE) {
		EG_Coord_t Scrollleft = GetScrollLeft();
		EG_Coord_t ScrollRight = GetScrollRight();
		if(GetStyleBaseDirection(EG_PART_MAIN) != EG_BASE_DIR_RTL) {
			if(ScrollRight < 0 && Scrollleft > 0) {		// Be sure the left side is not remains scrolled in
				ScrollRight = EG_MIN(Scrollleft, -ScrollRight);
				ScrollBy(ScrollRight, 0, AnimateEnable);
			}
		}
		else {
			if(Scrollleft < 0 && ScrollRight > 0) {		// Be sure the right side is not remains scrolled in
				ScrollRight = EG_MIN(ScrollRight, -Scrollleft);
				ScrollBy(Scrollleft, 0, AnimateEnable);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollAnimatedX(void *pObj, int32_t v)
{
	((EGObject*)pObj)->ScrollByRaw(v + ((EGObject*)pObj)->GetScrollX(), 0);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollAnimatedY(void *pObj, int32_t v)
{
	((EGObject*)pObj)->ScrollByRaw(0, v + ((EGObject*)pObj)->GetScrollY());
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollAnimatedEndCB(EGAnimate *pAnimate)
{
	EGEvent::EventSend((EGObject*)pAnimate->m_pItem, EG_EVENT_SCROLL_END, nullptr);
}

/////////////////////////////////////////////////////////////////////////////

void EGObject::ScrollAreaIntoView(const EGRect *pRect, EGObject *pChild, EGPoint *pScrollValue, EG_AnimateEnable_e AnimateEnable)
{
EG_Coord_t ScrollY = 0, ScrollX = 0;
const EGRect *pRectTmp;
EG_Coord_t SnapGoal = 0;

	EGObject *pParent = pChild->GetParent();
	if(!pParent->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE)) return;
	EG_DirType_e ScrollDirection = pParent->GetScrollDirection();
	EG_ScrollSnap_e SnapY = pParent->GetScrollSnapY();
	if(SnapY != EG_SCROLL_SNAP_NONE) pRectTmp = &pChild->m_Rect;
	else pRectTmp = pRect;
	EG_Coord_t BorderWidth = pParent->GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t PadTop = pParent->GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t PadBottom = pParent->GetStylePadBottom(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t TopDifference = pParent->m_Rect.GetY1() + PadTop - pRectTmp->GetY1() - pScrollValue->m_Y;
	EG_Coord_t BottomDifference = -(pParent->m_Rect.GetY2() - PadBottom - pRectTmp->GetY2() - pScrollValue->m_Y);
	EG_Coord_t ParentHeight = pParent->GetHeight() - PadTop - PadBottom;
	if((TopDifference >= 0) && (BottomDifference >= 0))	ScrollY = 0;
	else if(TopDifference > 0){
		ScrollY = TopDifference;
		EG_Coord_t ScrollTop = pParent->GetScrollTop();		// Do not let scrolling in
		if(ScrollTop - ScrollY < 0) ScrollY = 0;
	}
	else if(BottomDifference > 0){
		ScrollY = -BottomDifference;
		EG_Coord_t ScrollBottom = pParent->GetScrollBottom();		// Do not let scrolling in
		if(ScrollBottom + ScrollY < 0) ScrollY = 0;
	}
	switch((uint8_t)SnapY) {
		case EG_SCROLL_SNAP_START:
			SnapGoal = pParent->m_Rect.GetY1() + PadTop;
			ScrollY += SnapGoal - pRectTmp->GetY1() + ScrollY;
			break;
		case EG_SCROLL_SNAP_END:
			SnapGoal = pParent->m_Rect.GetY2() - PadBottom;
			ScrollY += SnapGoal - pRectTmp->GetY2() + ScrollY;
			break;
		case EG_SCROLL_SNAP_CENTER:
			SnapGoal = pParent->m_Rect.GetY1() + PadTop + ParentHeight / 2;
			ScrollY += SnapGoal - pRectTmp->GetHeight() / 2 + pRectTmp->GetY1() + ScrollY;
			break;
	}
	EG_ScrollSnap_e SnapX = pParent->GetScrollSnapX();
	if(SnapX != EG_SCROLL_SNAP_NONE)	pRectTmp = &pChild->m_Rect;
	else pRectTmp = pRect;
	EG_Coord_t PadLeft = pParent->GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t PadRight = pParent->GetStylePadRight(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t LeftDifference = pParent->m_Rect.GetX1() + PadLeft - pRectTmp->GetX1() - pScrollValue->m_X;
	EG_Coord_t RightDifference = -(pParent->m_Rect.GetX2() - PadRight - pRectTmp->GetX2() - pScrollValue->m_X);
	if((LeftDifference >= 0 && RightDifference >= 0)) ScrollX = 0;
	else if(LeftDifference > 0) {
		ScrollX = LeftDifference;
		EG_Coord_t Scrollleft = pParent->GetScrollLeft();		// Do not let scrolling in
		if(Scrollleft - ScrollX < 0) ScrollX = 0;
	}
	else if(RightDifference > 0) {
		ScrollX = -RightDifference;
		EG_Coord_t ScrollRight = pParent->GetScrollRight();		// Do not let scrolling in
		if(ScrollRight + ScrollX < 0) ScrollX = 0;
	}
	EG_Coord_t parent_w = pParent->GetWidth() - PadLeft - PadRight;
	switch((uint8_t)SnapX) {
		case EG_SCROLL_SNAP_START:
			SnapGoal = pParent->m_Rect.GetX1() + PadLeft;
			ScrollX += SnapGoal - pRectTmp->GetX1() + ScrollX;
			break;
		case EG_SCROLL_SNAP_END:
			SnapGoal = pParent->m_Rect.GetX2() - PadRight;
			ScrollX += SnapGoal - pRectTmp->GetX2() + ScrollX;
			break;
		case EG_SCROLL_SNAP_CENTER:
			SnapGoal = pParent->m_Rect.GetX1() + PadLeft + parent_w / 2;
			ScrollX += SnapGoal - pRectTmp->GetWidth() / 2 + pRectTmp->GetX1() + ScrollX;
			break;
	}
	bool DeleteY = EGAnimate::Delete(pParent, ScrollAnimatedY);	// Remove any pending scroll animations.
	bool DeleteX = EGAnimate::Delete(pParent, ScrollAnimatedX);
	if(DeleteY || DeleteX) if(EGEvent::EventSend(pParent, EG_EVENT_SCROLL_END, nullptr) != EG_RES_OK) return;
	if(((ScrollDirection & EG_DIR_LEFT) == 0) && (ScrollX < 0)) ScrollX = 0;
	if(((ScrollDirection & EG_DIR_RIGHT) == 0) && (ScrollX > 0)) ScrollX = 0;
	if(((ScrollDirection & EG_DIR_TOP) == 0) && (ScrollY < 0)) ScrollY = 0;
	if(((ScrollDirection & EG_DIR_BOTTOM) == 0) && (ScrollY > 0)) ScrollY = 0;
	pScrollValue->m_X += (AnimateEnable == EG_ANIM_OFF) ? 0 : ScrollX;
	pScrollValue->m_Y += (AnimateEnable == EG_ANIM_OFF) ? 0 : ScrollY;
	pParent->ScrollBy(ScrollX, ScrollY, AnimateEnable);
}
