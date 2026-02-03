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


#include "core/EG_InputDevice.h"

//////////////////////////////////////////////////////////////////////////////

#define ELASTIC_SLOWNESS_FACTOR 4 // Scrolling on elastic parts are slower by this factor

//////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ScrollHandler(EG_ProcessedInput_t *pProcess)
{
	if(pProcess->Pointer.Vector.m_X == 0 && pProcess->Pointer.Vector.m_Y == 0) {
		return;
	}
	EGObject *pScrollObj = pProcess->Pointer.pScrollObj;
	if(pScrollObj == nullptr){	// If there is no scroll object yet try to find one
		pScrollObj = FindScrollObj(pProcess);
		if(pScrollObj == nullptr) return;
		InitScrollLimits(pProcess);
		EGEvent::EventSend(pScrollObj, EG_EVENT_SCROLL_BEGIN, nullptr);
		if(pProcess->ResetQuery) return;
	}
	int16_t Angle = 0;	// Set new position or scroll if the vector is not zero
	int16_t Zoom = 256;
	EGObject *pParent = pScrollObj;
	while(pParent) {
		Angle += pParent->GetStyleTransformAngle(0);
		Zoom *= (pParent->GetStyleTransformZoom(0) / 256);
		pParent = pParent->GetParent();
	}
	if(Angle != 0 || Zoom != EG_SCALE_NONE) {
		Angle = -Angle;
		Zoom = (256 * 256) / Zoom;
		EGPoint Pivot = {0, 0};
		pProcess->Pointer.Vector.PointTransform(Angle, Zoom, &Pivot);
	}
	EG_Coord_t DifferenceX = 0;
	EG_Coord_t DifferenceY = 0;
	if(pProcess->Pointer.ScrollDirection == EG_DIR_HOR) {
		EG_Coord_t sr = pScrollObj->GetScrollRight();
		EG_Coord_t sl = pScrollObj->GetScrollLeft();
		DifferenceX = ElasticDifference(pScrollObj, pProcess->Pointer.Vector.m_X, sl, sr, EG_DIR_HOR);
	}
	else {
		EG_Coord_t st = pScrollObj->GetScrollTop();
		EG_Coord_t sb = pScrollObj->GetScrollBottom();
		DifferenceY = ElasticDifference(pScrollObj, pProcess->Pointer.Vector.m_Y, st, sb, EG_DIR_VER);
	}
	EG_DirType_e scroll_dir = pScrollObj->GetScrollDirection();
	if((scroll_dir & EG_DIR_LEFT) == 0 && DifferenceX > 0) DifferenceX = 0;
	if((scroll_dir & EG_DIR_RIGHT) == 0 && DifferenceX < 0) DifferenceX = 0;
	if((scroll_dir & EG_DIR_TOP) == 0 && DifferenceY > 0) DifferenceY = 0;
	if((scroll_dir & EG_DIR_BOTTOM) == 0 && DifferenceY < 0) DifferenceY = 0;
	ScrollLimitDifference(pProcess, &DifferenceX, &DifferenceY);	// Respect the scroll limit area
	pScrollObj->ScrollByRaw(DifferenceX, DifferenceY);
	if(pProcess->ResetQuery) return;
	pProcess->Pointer.ScrollSum.m_X += DifferenceX;
	pProcess->Pointer.ScrollSum.m_Y += DifferenceY;
}

//////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ScrollThrowHandler(EG_ProcessedInput_t *pProcess)
{
	EGObject *pScrollObj = pProcess->Pointer.pScrollObj;
	if(pScrollObj == nullptr) return;
	if(pProcess->Pointer.ScrollDirection == EG_DIR_NONE) return;
	EGInputDevice *pActiveInput = EGInputDevice::GetActive();
	EG_Coord_t ScrollThrow = pActiveInput->m_pDriver->m_ScrollThrow;
	if(pScrollObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_MOMENTUM) == false) {
		pProcess->Pointer.ScrollThrowVector.m_Y = 0;
		pProcess->Pointer.ScrollThrowVector.m_X = 0;
	}
	EG_ScrollSnap_e align_x = pScrollObj->GetScrollSnapX();
	EG_ScrollSnap_e align_y = pScrollObj->GetScrollSnapY();
	if(pProcess->Pointer.ScrollDirection == EG_DIR_VER) {
		pProcess->Pointer.ScrollThrowVector.m_X = 0;
		if(align_y == EG_SCROLL_SNAP_NONE) {		// If no snapping "throw"
			pProcess->Pointer.ScrollThrowVector.m_Y *= (100 - ScrollThrow) / 100;
			EG_Coord_t sb = pScrollObj->GetScrollBottom();
			EG_Coord_t st = pScrollObj->GetScrollTop();
			pProcess->Pointer.ScrollThrowVector.m_Y = ElasticDifference(pScrollObj, pProcess->Pointer.ScrollThrowVector.m_Y, st, sb, EG_DIR_VER);
			pScrollObj->ScrollBy(0, pProcess->Pointer.ScrollThrowVector.m_Y, EG_ANIM_OFF);
		}
		else {		// With snapping find the nearest Snap point and scroll there
			EG_Coord_t DifferenceY = ScrollThrowPredictY(pProcess);
			pProcess->Pointer.ScrollThrowVector.m_Y = 0;
			ScrollLimitDifference(pProcess, nullptr, &DifferenceY);
			EG_Coord_t y = FindSnapPointY(pScrollObj, EG_COORD_MIN, EG_COORD_MAX, DifferenceY);
			pScrollObj->ScrollBy(0, DifferenceY + y, EG_ANIM_ON);
		}
	}
	else if(pProcess->Pointer.ScrollDirection == EG_DIR_HOR) {
		pProcess->Pointer.ScrollThrowVector.m_Y = 0;
		// If no snapping "throw"
		if(align_x == EG_SCROLL_SNAP_NONE) {
			pProcess->Pointer.ScrollThrowVector.m_X *= (100 - ScrollThrow) / 100;
			EG_Coord_t sl = pScrollObj->GetScrollLeft();
			EG_Coord_t sr = pScrollObj->GetScrollRight();
			pProcess->Pointer.ScrollThrowVector.m_X = ElasticDifference(pScrollObj, pProcess->Pointer.ScrollThrowVector.m_X, sl, sr, EG_DIR_HOR);
			pScrollObj->ScrollBy(pProcess->Pointer.ScrollThrowVector.m_X, 0, EG_ANIM_OFF);
		}
		// With snapping find the nearest Snap point and scroll there
		else {
			EG_Coord_t DifferenceX = ScrollThrowPredictX(pProcess);
			pProcess->Pointer.ScrollThrowVector.m_X = 0;
			ScrollLimitDifference(pProcess, &DifferenceX, nullptr);
			EG_Coord_t x = FindSnapPointX(pScrollObj, EG_COORD_MIN, EG_COORD_MAX, DifferenceX);
			pScrollObj->ScrollBy(x + DifferenceX, 0, EG_ANIM_ON);
		}
	}
	if((pProcess->Pointer.ScrollThrowVector.m_X == 0) && (pProcess->Pointer.ScrollThrowVector.m_Y == 0)){	// Check if the scroll has finished
		if(align_y == EG_SCROLL_SNAP_NONE) {		// Revert if scrolled in If vertically scrollable and not controlled by Snap
			EG_Coord_t st = pScrollObj->GetScrollTop();
			EG_Coord_t sb = pScrollObj->GetScrollBottom();
			if(st > 0 || sb > 0) {
				if(st < 0) {
					pScrollObj->ScrollBy(0, st, EG_ANIM_ON);
				}
				else if(sb < 0) {
					pScrollObj->ScrollBy(0, -sb, EG_ANIM_ON);
				}
			}
		}
		if(align_x == EG_SCROLL_SNAP_NONE) {	// If horizontally scrollable and not controlled by Snap
			EG_Coord_t sl = pScrollObj->GetScrollLeft();
			EG_Coord_t sr = pScrollObj->GetScrollRight();
			if(sl > 0 || sr > 0) {
				if(sl < 0) {
					pScrollObj->ScrollBy(sl, 0, EG_ANIM_ON);
				}
				else if(sr < 0) {
					pScrollObj->ScrollBy(-sr, 0, EG_ANIM_ON);
				}
			}
		}
		EGEvent::EventSend(pScrollObj, EG_EVENT_SCROLL_END, pActiveInput);
		if(pProcess->ResetQuery) return;
		pProcess->Pointer.ScrollDirection = EG_DIR_NONE;
		pProcess->Pointer.pScrollObj = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGInputDevice::ScrollThrowPredict(EG_DirType_e Direction)
{
EG_Coord_t Value;
	EG_Coord_t Sum = 0;

	switch(Direction) {
		case EG_DIR_VER:
			Value = m_Process.Pointer.ScrollThrowVectorOri.m_Y;
			break;
		case EG_DIR_HOR:
			Value = m_Process.Pointer.ScrollThrowVectorOri.m_X;
			break;
		default:
			return 0;
	}
	EG_Coord_t ScrollThrow = m_pDriver->m_ScrollThrow;
	while(Value) {
		Sum += Value;
		Value = Value * (100 - ScrollThrow) / 100;
	}
	return Sum;
}

//////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ScrollGetSnapDist(EGObject *pObj, EGPoint *pPoint)
{
	pPoint->m_X = FindSnapPointX(pObj, pObj->m_Rect.GetX1(), pObj->m_Rect.GetX2(), 0);
	pPoint->m_Y = FindSnapPointY(pObj, pObj->m_Rect.GetY1(), pObj->m_Rect.GetY2(), 0);
}

//////////////////////////////////////////////////////////////////////////////

EGObject* EGInputDevice::FindScrollObj(EG_ProcessedInput_t *pProcess)
{
EGObject *pObj = nullptr;
EG_DirType_e Direction = EG_DIR_NONE;
bool HorizontalEnable = false, VerticalEnable = false;

	EGInputDevice *pActiveInput = GetActive();
	EG_Coord_t ScrollLimit = pActiveInput->m_pDriver->m_ScrollLimit;
	/* Go until find a scrollable object in the current direction
     *More precisely:
     * 1. Check the pressed object and all of its ancestors and try to find an object which is scrollable
     * 2. Scrollable means it has some content out of its area
     * 3. If an object can be scrolled into the current direction then use it ("real match"")
     * 4. If can be scrolled on the current axis (hor/ver) save it as candidate (at least show an elastic scroll effect)
     * 5. Use the last candidate. Always the "deepest" pParent or the object from point 3*/
	EGObject *pActiveObj = pProcess->Pointer.pActiveObj;
	pProcess->Pointer.ScrollSum.m_X += pProcess->Pointer.Vector.m_X;
	pProcess->Pointer.ScrollSum.m_Y += pProcess->Pointer.Vector.m_Y;

	while(pActiveObj) {
		// Get the transformed ScrollSum with this object
		int16_t Angle = 0;
		int32_t Zoom = 256;
		EGPoint Pivot = {0, 0};
		EGObject *pParent = pActiveObj;
		while(pParent) {
			Angle += pParent->GetStyleTransformAngle(0);
			int32_t zoom_act = pParent->GetStyleTransformZoom(0);
			Zoom = (Zoom * zoom_act) >> 8;
			pParent = pParent->GetParent();
		}
		EGPoint ScrollSum = pProcess->Pointer.ScrollSum;
		if(Angle != 0 || Zoom != EG_SCALE_NONE) {
			Angle = -Angle;
			Zoom = (256 * 256) / Zoom;
			ScrollSum.PointTransform(Angle, Zoom, &Pivot);
		}
		if(EG_ABS(ScrollSum.m_X) > EG_ABS(ScrollSum.m_Y)) HorizontalEnable = true; // Decide if it's a horizontal or vertical scroll
		else VerticalEnable = true;
		if(pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLLABLE) == false) {
			// If this object don't want to chain the scroll to the pParent stop searching
			if(pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_CHAIN_HOR) == false && HorizontalEnable) break;
			if(pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_CHAIN_VER) == false && VerticalEnable) break;
			pActiveObj = pActiveObj->GetParent();
			continue;
		}
		// Consider both up-down or left/right scrollable according to the current direction
		bool UpEnable = VerticalEnable;
		bool DownEnable = VerticalEnable;
		bool LeftEnable = HorizontalEnable;
		bool RightEnable = HorizontalEnable;
		// The object might have disabled some directions.
		EG_DirType_e scroll_dir = pActiveObj->GetScrollDirection();
		if((scroll_dir & EG_DIR_LEFT) == 0) LeftEnable = false;
		if((scroll_dir & EG_DIR_RIGHT) == 0) RightEnable = false;
		if((scroll_dir & EG_DIR_TOP) == 0) UpEnable = false;
		if((scroll_dir & EG_DIR_BOTTOM) == 0) DownEnable = false;
		// The object is scrollable to a direction if its content overflow in that direction.
		EG_Coord_t st = pActiveObj->GetScrollTop();
		EG_Coord_t sb = pActiveObj->GetScrollBottom();
		EG_Coord_t sl = pActiveObj->GetScrollLeft();
		EG_Coord_t sr = pActiveObj->GetScrollRight();
		/* If this object is scrollable into the current scroll direction then save it as a candidate.
         *It's important only to be scrollable on the current axis (hor/ver) because if the scroll
         *is propagated to this object it can show at least elastic scroll effect.
         *But if not hor/ver scrollable do not scroll it at all (so it's not a good candidate)*/
		if((st > 0 || sb > 0) && ((UpEnable && ScrollSum.m_Y >= ScrollLimit) || (DownEnable && ScrollSum.m_Y <= -ScrollLimit))) {
			pObj = pActiveObj;
			Direction = EG_DIR_VER;
		}

		if((sl > 0 || sr > 0) &&
			 ((LeftEnable && ScrollSum.m_X >= ScrollLimit) ||
				(RightEnable && ScrollSum.m_X <= -ScrollLimit))) {
			pObj = pActiveObj;
			Direction = EG_DIR_HOR;
		}
		if(st <= 0) UpEnable = false;
		if(sb <= 0) DownEnable = false;
		if(sl <= 0) LeftEnable = false;
		if(sr <= 0) RightEnable = false;
		// If the object really can be scrolled into the current direction then use it.
		if((LeftEnable && ScrollSum.m_X >= ScrollLimit) ||
			 (RightEnable && ScrollSum.m_X <= -ScrollLimit) ||
			 (UpEnable && ScrollSum.m_Y >= ScrollLimit) ||
			 (DownEnable && ScrollSum.m_Y <= -ScrollLimit)) {
			pProcess->Pointer.ScrollDirection = HorizontalEnable ? EG_DIR_HOR : EG_DIR_VER;
			break;
		}
		// If this object don't want to chain the scroll to the pParent stop searching
		if(pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_CHAIN_HOR) == false && HorizontalEnable) break;
		if(pActiveObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_CHAIN_VER) == false && VerticalEnable) break;
		pActiveObj = pActiveObj->GetParent();		// Try the pParent
	}
	if(pObj) {	// Use the last candidate
		pProcess->Pointer.ScrollDirection = Direction;
		pProcess->Pointer.pScrollObj = pObj;
		pProcess->Pointer.ScrollSum.m_X = 0;
		pProcess->Pointer.ScrollSum.m_Y = 0;
	}
	return pObj;
}

//////////////////////////////////////////////////////////////////////////////

void EGInputDevice::InitScrollLimits(EG_ProcessedInput_t *pProcess)
{
	EGObject *pObj = pProcess->Pointer.pScrollObj;
	// If there no STOP allow scrolling anywhere
	if(pObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_ONE) == false) {
		pProcess->Pointer.ScrollArea.Set(EG_COORD_MIN, EG_COORD_MIN, EG_COORD_MAX, EG_COORD_MAX);
	}
	// With STOP limit the scrolling to the perv and next Snap point
	else {
		switch(pObj->GetScrollSnapY()) {
			case EG_SCROLL_SNAP_START:{
				pProcess->Pointer.ScrollArea.SetY1(FindSnapPointY(pObj, pObj->m_Rect.GetY1() + 1, EG_COORD_MAX, 0));
				pProcess->Pointer.ScrollArea.SetY2(FindSnapPointY(pObj, EG_COORD_MIN, pObj->m_Rect.GetY1() - 1, 0));
				break;
      }
			case EG_SCROLL_SNAP_END:{
				pProcess->Pointer.ScrollArea.SetY1(FindSnapPointY(pObj, pObj->m_Rect.GetY2(), EG_COORD_MAX, 0));
				pProcess->Pointer.ScrollArea.SetY2(FindSnapPointY(pObj, EG_COORD_MIN, pObj->m_Rect.GetY2(), 0));
				break;
      }
			case EG_SCROLL_SNAP_CENTER: {
				EG_Coord_t Middle = pObj->m_Rect.GetY1() + pObj->m_Rect.GetHeight() / 2;
				pProcess->Pointer.ScrollArea.SetY1(FindSnapPointY(pObj, Middle + 1, EG_COORD_MAX, 0));
				pProcess->Pointer.ScrollArea.SetY2(FindSnapPointY(pObj, EG_COORD_MIN, Middle - 1, 0));
				break;
			}
			default:{
				pProcess->Pointer.ScrollArea.SetY1(EG_COORD_MIN);
				pProcess->Pointer.ScrollArea.SetY2(EG_COORD_MAX);
				break;
      }
		}
		switch(pObj->GetScrollSnapX()){
			case EG_SCROLL_SNAP_START:{
				pProcess->Pointer.ScrollArea.SetX1(FindSnapPointX(pObj, pObj->m_Rect.GetX1(), EG_COORD_MAX, 0));
				pProcess->Pointer.ScrollArea.SetX2(FindSnapPointX(pObj, EG_COORD_MIN, pObj->m_Rect.GetX1(), 0));
				break;
      }
			case EG_SCROLL_SNAP_END:{
				pProcess->Pointer.ScrollArea.SetX1(FindSnapPointX(pObj, pObj->m_Rect.GetX2(), EG_COORD_MAX, 0));
				pProcess->Pointer.ScrollArea.SetX2(FindSnapPointX(pObj, EG_COORD_MIN, pObj->m_Rect.GetX2(), 0));
				break;
      }
			case EG_SCROLL_SNAP_CENTER: {
				EG_Coord_t Middle = pObj->m_Rect.GetX1() + pObj->m_Rect.GetWidth() / 2;
				pProcess->Pointer.ScrollArea.SetX1(FindSnapPointX(pObj, Middle + 1, EG_COORD_MAX, 0));
				pProcess->Pointer.ScrollArea.SetX2(FindSnapPointX(pObj, EG_COORD_MIN, Middle - 1, 0));
				break;
			}
			default:{
				pProcess->Pointer.ScrollArea.SetX1(EG_COORD_MIN);
				pProcess->Pointer.ScrollArea.SetX2(EG_COORD_MAX);
				break;
      }
		}
	}
	// Allow scrolling on the edges. It will be reverted to the edge due to snapping anyway
	if(pProcess->Pointer.ScrollArea.GetX1() == 0) pProcess->Pointer.ScrollArea.SetX1(EG_COORD_MIN);
	if(pProcess->Pointer.ScrollArea.GetX2() == 0) pProcess->Pointer.ScrollArea.SetX2(EG_COORD_MAX);
	if(pProcess->Pointer.ScrollArea.GetY1() == 0) pProcess->Pointer.ScrollArea.SetY1(EG_COORD_MIN);
	if(pProcess->Pointer.ScrollArea.GetY2() == 0) pProcess->Pointer.ScrollArea.SetY2(EG_COORD_MAX);
}

//////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGInputDevice::FindSnapPointX(EGObject *pObj, EG_Coord_t Min, EG_Coord_t Max, EG_Coord_t Offset)
{
	EG_ScrollSnap_e Align = pObj->GetScrollSnapX();
	if(Align == EG_SCROLL_SNAP_NONE) return 0;
	EG_Coord_t Distance = EG_COORD_MAX;
	EG_Coord_t PadLeft = pObj->GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PadRight = pObj->GetStylePadRight(EG_PART_MAIN);
	uint32_t ChildCount = pObj->GetChildCount();
	for(uint32_t i = 0; i < ChildCount; i++) {
		EGObject *pChild = pObj->m_pAttributes->ppChildren[i];
		if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
		if(pChild->HasFlagSet(EG_OBJ_FLAG_SNAPPABLE)) {
			EG_Coord_t ChildX = 0;
			EG_Coord_t ParentX = 0;
			switch(Align) {
				case EG_SCROLL_SNAP_START:
					ChildX = pChild->m_Rect.GetX1();
					ParentX = pObj->m_Rect.GetX1() + PadLeft;
					break;
				case EG_SCROLL_SNAP_END:
					ChildX = pChild->m_Rect.GetX2();
					ParentX = pObj->m_Rect.GetX2() - PadRight;
					break;
				case EG_SCROLL_SNAP_CENTER:
					ChildX = pChild->m_Rect.GetX1() + pChild->m_Rect.GetWidth() / 2;
					ParentX = pObj->m_Rect.GetX1() + PadLeft + (pObj->m_Rect.GetWidth() - PadLeft - PadRight) / 2;
					break;
				default:
					continue;
			}
			ChildX += Offset;
			if(ChildX >= Min && ChildX <= Max) {
				EG_Coord_t x = ChildX - ParentX;
				if(EG_ABS(x) < EG_ABS(Distance)) Distance = x;
			}
		}
	}
	return Distance == EG_COORD_MAX ? 0 : -Distance;
}

//////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGInputDevice::FindSnapPointY(EGObject *pObj, EG_Coord_t Min, EG_Coord_t Max, EG_Coord_t Offset)
{
	EG_ScrollSnap_e Align = pObj->GetScrollSnapY();
	if(Align == EG_SCROLL_SNAP_NONE) return 0;
	EG_Coord_t Distance = EG_COORD_MAX;
	EG_Coord_t PadTop = pObj->GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PadBottom = pObj->GetStylePadBottom(EG_PART_MAIN);
	uint32_t ChildCount = pObj->GetChildCount();
	for(uint32_t i = 0; i < ChildCount; i++) {
		EGObject *pChild = pObj->m_pAttributes->ppChildren[i];
		if(pChild->HasAnyFlagSet(EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
		if(pChild->HasFlagSet(EG_OBJ_FLAG_SNAPPABLE)) {
			EG_Coord_t ChildY = 0;
			EG_Coord_t ParentY = 0;
			switch(Align) {
				case EG_SCROLL_SNAP_START:
					ChildY = pChild->m_Rect.GetY1();
					ParentY = pObj->m_Rect.GetY1() + PadTop;
					break;
				case EG_SCROLL_SNAP_END:
					ChildY = pChild->m_Rect.GetY2();
					ParentY = pObj->m_Rect.GetY2() - PadBottom;
					break;
				case EG_SCROLL_SNAP_CENTER:
					ChildY = pChild->m_Rect.GetY1() + pChild->m_Rect.GetHeight() / 2;
					ParentY = pObj->m_Rect.GetY1() + PadTop + (pObj->m_Rect.GetHeight() - PadTop - PadBottom) / 2;
					break;
				default:
					continue;
			}
			ChildY += Offset;
			if(ChildY >= Min && ChildY <= Max) {
				EG_Coord_t y = ChildY - ParentY;
				if(EG_ABS(y) < EG_ABS(Distance)) Distance = y;
			}
		}
	}
	return Distance == EG_COORD_MAX ? 0 : -Distance;
}

//////////////////////////////////////////////////////////////////////////////

void EGInputDevice::ScrollLimitDifference(EG_ProcessedInput_t *pProcess, EG_Coord_t *DifferenceX, EG_Coord_t *DifferenceY)
{
	if(DifferenceY) {
		if(pProcess->Pointer.ScrollSum.m_Y + *DifferenceY < pProcess->Pointer.ScrollArea.GetY1()) {
			*DifferenceY = pProcess->Pointer.ScrollArea.GetY1() - pProcess->Pointer.ScrollSum.m_Y;
		}
		if(pProcess->Pointer.ScrollSum.m_Y + *DifferenceY > pProcess->Pointer.ScrollArea.GetY2()) {
			*DifferenceY = pProcess->Pointer.ScrollArea.GetY2() - pProcess->Pointer.ScrollSum.m_Y;
		}
	}
	if(DifferenceX) {
		if(pProcess->Pointer.ScrollSum.m_X + *DifferenceX < pProcess->Pointer.ScrollArea.GetX1()) {
			*DifferenceX = pProcess->Pointer.ScrollArea.GetX1() - pProcess->Pointer.ScrollSum.m_X;
		}
		if(pProcess->Pointer.ScrollSum.m_X + *DifferenceX > pProcess->Pointer.ScrollArea.GetX2()) {
			*DifferenceX = pProcess->Pointer.ScrollArea.GetX2() - pProcess->Pointer.ScrollSum.m_X;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGInputDevice::ScrollThrowPredictY(EG_ProcessedInput_t *pProcess)
{
EG_Coord_t Move = 0;

	EG_Coord_t PosY = pProcess->Pointer.ScrollThrowVector.m_Y;
	EGInputDevice *pActiveInput = GetActive();
	EG_Coord_t ScrollThrow = pActiveInput->m_pDriver->m_ScrollThrow;
	while(PosY) {
		Move += PosY;
		PosY = PosY * (100 - ScrollThrow) / 100;
	}
	return Move;
}

//////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGInputDevice::ScrollThrowPredictX(EG_ProcessedInput_t *pProcess)
{
EG_Coord_t Move = 0;

	EG_Coord_t PosX = pProcess->Pointer.ScrollThrowVector.m_X;
	EGInputDevice *pActiveInput = GetActive();
	EG_Coord_t ScrollThrow = pActiveInput->m_pDriver->m_ScrollThrow;
	while(PosX) {
		Move += PosX;
		PosX = PosX * (100 - ScrollThrow) / 100;
	}
	return Move;
}

//////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGInputDevice::ElasticDifference(EGObject *pScrollObj, EG_Coord_t Difference, EG_Coord_t ScrollStart, EG_Coord_t ScrollEnd, EG_DirType_e Direction)
{
	if(pScrollObj->HasFlagSet(EG_OBJ_FLAG_SCROLL_ELASTIC)) {
		// If there is snapping in the current direction don't use the elastic factor 
		EG_ScrollSnap_e Snap;
		Snap = (Direction == EG_DIR_HOR) ? pScrollObj->GetScrollSnapX() : pScrollObj->GetScrollSnapY();
		EGObject *pActiveObj = GetActive()->GetActiveObj();
		EG_Coord_t SnapPoint = 0;
		EG_Coord_t ActiveObjPoint = 0;
		if(Direction == EG_DIR_HOR) {
			EG_Coord_t PadLeft = pScrollObj->GetStylePadLeft(EG_PART_MAIN);
			EG_Coord_t PadRight = pScrollObj->GetStylePadRight(EG_PART_MAIN);

			switch((uint8_t)Snap){
				case EG_SCROLL_SNAP_CENTER:
					SnapPoint = PadLeft + (pScrollObj->m_Rect.GetWidth() - PadLeft - PadRight) / 2 + pScrollObj->m_Rect.GetX1();
					ActiveObjPoint = pActiveObj->m_Rect.GetWidth() / 2 + pActiveObj->m_Rect.GetX1();
					break;
				case EG_SCROLL_SNAP_START:
					SnapPoint = pScrollObj->m_Rect.GetX1() + PadLeft;
					ActiveObjPoint = pActiveObj->m_Rect.GetX1();
					break;
				case EG_SCROLL_SNAP_END:
					SnapPoint = pScrollObj->m_Rect.GetX2() - PadRight;
					ActiveObjPoint = pActiveObj->m_Rect.GetX2();
					break;
			}
		}
		else {
			EG_Coord_t PadTop = pScrollObj->GetStylePadTop(EG_PART_MAIN);
			EG_Coord_t PadBottom = pScrollObj->GetStylePadBottom(EG_PART_MAIN);
			switch((uint8_t)Snap){
				case EG_SCROLL_SNAP_CENTER:
					SnapPoint = PadTop + (pScrollObj->m_Rect.GetHeight() - PadTop - PadBottom) / 2 + pScrollObj->m_Rect.GetY1();
					ActiveObjPoint = pActiveObj->m_Rect.GetHeight() / 2 + pActiveObj->m_Rect.GetY1();
					break;
				case EG_SCROLL_SNAP_START:
					SnapPoint = pScrollObj->m_Rect.GetY1() + PadTop;
					ActiveObjPoint = pActiveObj->m_Rect.GetY1();
					break;
				case EG_SCROLL_SNAP_END:
					SnapPoint = pScrollObj->m_Rect.GetY2() - PadBottom;
					ActiveObjPoint = pActiveObj->m_Rect.GetY2();
					break;
			}
		}
		if(ScrollEnd < 0) {
			if(Snap != EG_SCROLL_SNAP_NONE && ActiveObjPoint > SnapPoint) return Difference;
			if(Difference < 0) Difference -= ELASTIC_SLOWNESS_FACTOR / 2;			// Rounding
			if(Difference > 0) Difference += ELASTIC_SLOWNESS_FACTOR / 2;
			return Difference / ELASTIC_SLOWNESS_FACTOR;
		}
		else if(ScrollStart < 0) {
			if(Snap != EG_SCROLL_SNAP_NONE && ActiveObjPoint < SnapPoint) return Difference;
			if(Difference < 0) Difference -= ELASTIC_SLOWNESS_FACTOR / 2;			// Rounding
			if(Difference > 0) Difference += ELASTIC_SLOWNESS_FACTOR / 2;
			return Difference / ELASTIC_SLOWNESS_FACTOR;
		}
	}
	else {
		if(ScrollEnd + Difference < 0) Difference = -ScrollEnd;		// Scroll back to the boundary if required
		if(ScrollStart - Difference < 0) Difference = ScrollStart;
	}
	return Difference;
}
