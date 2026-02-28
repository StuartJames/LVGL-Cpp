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

#include "extra/widgets/EG_TileView.h"
#include "core/EG_InputDevice.h"

#if EG_USE_TILEVIEW

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_TileViewClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

const EG_ClassType_t c_TileViewTitleClass = {
  .pBaseClassType = &c_ObjectClass,
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

EGTileViewTile::EGTileViewTile(EGObject *pParent) : EGObject()
{
  Attach(this, pParent, &c_TileViewTitleClass);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

EGTileView::EGTileView(void) : EGObject(), m_pActiveTile(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGTileView::EGTileView(EGObject *pParent) : EGObject(), m_pActiveTile(nullptr)
{
  Attach(this, pParent, &c_TileViewClass);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTileView::Configure(void)
{
  EGObject::Configure();
	SetSize(_EG_PCT(100), _EG_PCT(100));
	EGEvent::AddEventCB(this, EventCB, EG_EVENT_ALL, nullptr);
	AddFlag(EG_OBJ_FLAG_SCROLL_ONE);
	SetScrollSnapX(EG_SCROLL_SNAP_CENTER);
	SetScrollSnapY(EG_SCROLL_SNAP_CENTER);
}

///////////////////////////////////////////////////////////////////////////////////////

EGTileViewTile* EGTileView::AddTile(uint8_t ColumnIndex, uint8_t RowIndex, EG_DirType_e Direction)
{
	EGTileViewTile *pTile = new EGTileViewTile(this);
	pTile->SetSize(_EG_PCT(100), _EG_PCT(100));
	pTile->UpdateLayout(); // Be sure the size is correct
	pTile->SetPosition(ColumnIndex * GetContentWidth(), RowIndex * GetContentHeight());
	pTile->m_Direction = Direction;
	if(ColumnIndex == 0 && RowIndex == 0) {
		SetScrollDirection(Direction);
	}
	return pTile;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTileView::SetTile(EGTileViewTile *pTile, EG_AnimateEnable_e Enable)
{
	EG_Coord_t tx = pTile->GetX();
	EG_Coord_t ty = pTile->GetY();
	m_pActiveTile = pTile;
	SetScrollDirection(pTile->m_Direction);
	ScrollTo(tx, ty, Enable);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTileView::SetTileIndex(uint32_t ColumnIndex, uint32_t RowIndex, EG_AnimateEnable_e Enable)
{
	UpdateLayout();
	EG_Coord_t Width = GetContentWidth();
	EG_Coord_t Height = GetContentHeight();
	EG_Coord_t tx = ColumnIndex * Width;
	EG_Coord_t ty = RowIndex * Height;

	uint32_t i;
	for(i = 0; i < GetChildCount(); i++) {
		EGTileViewTile *pTile = (EGTileViewTile*)GetChild(i);
		EG_Coord_t x = pTile->GetX();
		EG_Coord_t y = pTile->GetY();
		if(x == tx && y == ty) {
			SetTile(pTile, Enable);
			return;
		}
	}
	EG_LOG_WARN("No tile found with at (%d,%d) index", (int)ColumnIndex, (int)RowIndex);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTileView::EventCB(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
	EGTileView *pTileView = (EGTileView*)pEvent->GetTarget();
	if(Code == EG_EVENT_SCROLL_END) {
		EGInputDevice *pInput = EGInputDevice::GetActive();
		if(pInput && pInput->m_Process.State == EG_INDEV_STATE_PRESSED) {
			return;
		}

		EG_Coord_t Width = pTileView->GetContentWidth();
		EG_Coord_t Height = pTileView->GetContentHeight();
		EGPoint scroll_end;
		pTileView->GetScrollEnd(&scroll_end);
		EG_Coord_t left = scroll_end.m_X;
		EG_Coord_t top = scroll_end.m_Y;
		EG_Coord_t tx = ((left + (Width / 2)) / Width) * Width;
		EG_Coord_t ty = ((top + (Height / 2)) / Height) * Height;
		EG_DirType_e Direction = EG_DIR_ALL;
		for(uint32_t i = 0; i < pTileView->GetChildCount(); i++) {
			EGTileViewTile *pTile = (EGTileViewTile*)pTileView->GetChild(i);
			EG_Coord_t x = pTile->GetX();
			EG_Coord_t y = pTile->GetY();
			if(x == tx && y == ty) {
				pTileView->m_pActiveTile = pTile;
				Direction = pTile->m_Direction;
				EGEvent::EventSend(pTileView, EG_EVENT_VALUE_CHANGED, NULL);
				break;
			}
		}
		pTileView->SetScrollDirection(Direction);
	}
}
#endif 
