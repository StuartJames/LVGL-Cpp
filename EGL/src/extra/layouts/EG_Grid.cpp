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

#include "extra/layouts/EG_Layouts.h"

///////////////////////////////////////////////////////////////////////////////

#if EG_USE_GRID

// Some helper defines
#define IS_FR(x) (x >= EG_COORD_MAX - 100)
#define IS_CONTENT(x) (x == EG_COORD_MAX - 101)
#define GET_FR(x) (x - (EG_COORD_MAX - 100))

EGStyleProperty_e   EGGridLayout::STYLE_COLUMN_ARRAY_PROPS;
EGStyleProperty_e   EGGridLayout::STYLE_COLUMN_ALIGN;
EGStyleProperty_e   EGGridLayout::STYLE_ROW_ARRAY_PROPS;
EGStyleProperty_e   EGGridLayout::STYLE_ROW_ALIGN;
EGStyleProperty_e   EGGridLayout::STYLE_CELL_COLUMN_POSITION;
EGStyleProperty_e   EGGridLayout::STYLE_CELL_COLUMN_SPAN;
EGStyleProperty_e   EGGridLayout::STYLE_CELL_ALIGN_X;
EGStyleProperty_e   EGGridLayout::STYLE_CELL_ROW_POSITION;
EGStyleProperty_e   EGGridLayout::STYLE_CELL_ROW_SPAN;
EGStyleProperty_e   EGGridLayout::STYLE_CELL_ALIGN_Y;
uint32_t            EGGridLayout::m_LayoutReference;

///////////////////////////////////////////////////////////////////////////////

EGGridLayout::EGGridLayout(void) :
  m_pColumnPositions(nullptr),
	m_pRowPositions(nullptr),
	m_pColumnWidths(nullptr),
	m_pRowHeights(nullptr),
	m_ColumCount(0),
	m_RowCount(0),
	m_GridWidth(0),
	m_GridHeight(0)
{
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::Initialise(void)
{
	STYLE_COLUMN_ARRAY_PROPS = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_ROW_ARRAY_PROPS = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_COLUMN_ALIGN = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_ROW_ALIGN = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_CELL_ROW_SPAN = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_CELL_ROW_POSITION = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_CELL_COLUMN_SPAN = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_CELL_COLUMN_POSITION = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_CELL_ALIGN_X = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	STYLE_CELL_ALIGN_Y = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	m_LayoutReference = EGObject::LayoutRegister(EGGridLayout::UpdateCB, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::Clear(void)
{
  m_pColumnPositions = nullptr;
	m_pRowPositions = nullptr;
	m_pColumnWidths = nullptr;
	m_pRowHeights = nullptr;
	m_ColumCount = 0;
	m_RowCount = 0;
	m_GridWidth = 0;
	m_GridHeight = 0;
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjGridParams(EGObject *pObj, const EG_Coord_t ColumnProps[], const EG_Coord_t RowProps[])
{
	SetObjStyleColumnParams(pObj, ColumnProps, 0);
	SetObjStyleRowParams(pObj, RowProps, 0);
	pObj->SetStyleLayout(m_LayoutReference, 0);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjAlign(EGObject *pObj, EG_GridAlign_e ColumnAlign, EG_GridAlign_e RowAlign)
{
	SetObjStyleColumnAlign(pObj, ColumnAlign, 0);
	SetObjStyleRowAlign(pObj, RowAlign, 0);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjCell(EGObject *pObj, EG_GridAlign_e AlignX, uint8_t ColumnPosition, uint8_t ColumnSpan,
													                    EG_GridAlign_e AlignY, uint8_t RowPosition, uint8_t RowSpan)

{
	SetObjStyleCellColumnPosition(pObj, ColumnPosition, 0);
	SetObjStyleCellRowPosition(pObj, RowPosition, 0);
	SetObjStyleCellAlignX(pObj, AlignX, 0);
	SetObjStyleCellColumnSpan(pObj, ColumnSpan, 0);
	SetObjStyleCellRowSpan(pObj, RowSpan, 0);
	SetObjStyleCellAlignY(pObj, AlignY, 0);
	pObj->GetParent()->MarkLayoutDirty();
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleRowParams(EGStyle *pStyle, const EG_Coord_t Value[])
{
EG_StyleValue_t v = {
	.pPtr = (const void *)Value
};

	pStyle->SetProperty(STYLE_ROW_ARRAY_PROPS, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleColumnParams(EGStyle *pStyle, const EG_Coord_t Value[])
{
EG_StyleValue_t v = {
	.pPtr = (const void *)Value
};

	pStyle->SetProperty(STYLE_COLUMN_ARRAY_PROPS, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleRowAlign(EGStyle *pStyle, EG_GridAlign_e Value)
{
EG_StyleValue_t v = {
	.Number = (EG_GridAlign_e)Value
};

	pStyle->SetProperty(STYLE_ROW_ALIGN, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleColumnAlign(EGStyle *pStyle, EG_GridAlign_e Value)
{
EG_StyleValue_t v = {
	.Number = (EG_GridAlign_e)Value
};

	pStyle->SetProperty(STYLE_COLUMN_ALIGN, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleCellColumnPosition(EGStyle *pStyle, uint8_t Value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pStyle->SetProperty(STYLE_CELL_COLUMN_POSITION, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleCellColumnSpan(EGStyle *pStyle, uint8_t Value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pStyle->SetProperty(STYLE_CELL_COLUMN_SPAN, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleCellRowPosition(EGStyle *pStyle, uint8_t Value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pStyle->SetProperty(STYLE_CELL_ROW_POSITION, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleCellRowSpan(EGStyle *pStyle, uint8_t Value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pStyle->SetProperty(STYLE_CELL_ROW_SPAN, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleCellAlignX(EGStyle *pStyle, uint8_t Value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pStyle->SetProperty(STYLE_CELL_ALIGN_X, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetStyleCellAlignY(EGStyle *pStyle, uint8_t Value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pStyle->SetProperty(STYLE_CELL_ALIGN_Y, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleRowParams(EGObject *pObj, const EG_Coord_t Value[], EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.pPtr = (const void *)Value
};

	pObj->SetLocalStyleProperty(STYLE_ROW_ARRAY_PROPS, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleColumnParams(EGObject *pObj, const EG_Coord_t Value[], EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.pPtr = (const void *)Value
};

	pObj->SetLocalStyleProperty(STYLE_COLUMN_ARRAY_PROPS, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleRowAlign(EGObject *pObj, EG_GridAlign_e Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_ROW_ALIGN, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleColumnAlign(EGObject *pObj, EG_GridAlign_e Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_COLUMN_ALIGN, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleCellColumnPosition(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_CELL_COLUMN_POSITION, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleCellColumnSpan(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_CELL_COLUMN_SPAN, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleCellRowPosition(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_CELL_ROW_POSITION, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleCellRowSpan(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_CELL_ROW_SPAN, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleCellAlignX(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_CELL_ALIGN_X, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::SetObjStyleCellAlignY(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags)
{
EG_StyleValue_t v = {
	.Number = (int32_t)Value
};

	pObj->SetLocalStyleProperty(STYLE_CELL_ALIGN_Y, v, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::UpdateCB(EGObject *pObj, void *pUserData)
{
EGGridLayout Grid;

	EG_LOG_INFO("update %p content", (void *)pObj);
//	const EG_Coord_t *pColumnParams = Grid.GetColumnParams(pObj);
//	const EG_Coord_t *pRowParams = Grid.GetRowParams(pObj);
//	if(pColumnParams == nullptr || pRowParams == nullptr) return;
	Grid.Calculate(pObj);
	EGPoint GridPosition;
	// Calculate the grids absolute x and y coordinates. It will be used as helper during item
  // repositioning to avoid calculating this value for every child
	EG_Coord_t BorderWidth = pObj->GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t LeftPadding = pObj->GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
	EG_Coord_t TopPadding = pObj->GetStylePadTop(EG_PART_MAIN) + BorderWidth;
	GridPosition.m_X = LeftPadding + pObj->m_Rect.GetX1() - pObj->GetScrollX();
	GridPosition.m_Y = TopPadding + pObj->m_Rect.GetY1() - pObj->GetScrollY();
	for(uint32_t i = 0; i < pObj->m_pAttributes->ChildCount; i++) {
		EGObject *pItem = pObj->m_pAttributes->ppChildren[i];
		Grid.RepositionItem(pItem, &GridPosition);
	}
	Grid.FreeBuffers();
	EG_Coord_t WidthSet = pObj->GetStyleWidth(EG_PART_MAIN);
	EG_Coord_t HeightSet = pObj->GetStyleHeight(EG_PART_MAIN);
	if(WidthSet == EG_SIZE_CONTENT || HeightSet == EG_SIZE_CONTENT) {
		pObj->RefreshSize();
	}
	EGEvent::EventSend(pObj, EG_EVENT_LAYOUT_CHANGED, nullptr);
	EG_TRACE_LAYOUT("finished");
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::Calculate(EGObject *pObj)
{
	if(pObj->GetChild(0) == nullptr){
    Clear();
    return;
  }
	CalculateRows(pObj);
	CalculateColumns(pObj);
	EG_Coord_t ColumnGap = pObj->GetStylePadColumn(EG_PART_MAIN);
	EG_Coord_t RowGap = pObj->GetStylePadRow(EG_PART_MAIN);
	bool Reverse = pObj->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL ? true : false;
	EG_Coord_t WidthSet = pObj->GetStyleWidth(EG_PART_MAIN);
	EG_Coord_t HeightSet = pObj->GetStyleHeight(EG_PART_MAIN);
	bool AutoWidth = ((WidthSet == EG_SIZE_CONTENT) && !pObj->m_WidthLayout) ? true : false;
	EG_Coord_t ContentWidth = pObj->GetContentWidth();
	m_GridWidth = GridAlign(ContentWidth, AutoWidth, GetColumnAlign(pObj), ColumnGap, m_ColumCount, m_pColumnWidths, m_pColumnPositions, Reverse);
	bool AutoHeight = (HeightSet == EG_SIZE_CONTENT && !pObj->m_HeightLayout) ? true : false;
	EG_Coord_t ContentHeight = pObj->GetContentHeight();
	m_GridHeight = GridAlign(ContentHeight, AutoHeight, GetRowAlign(pObj), RowGap, m_RowCount, m_pRowHeights, m_pRowPositions, false);
	EG_ASSERT_MEM_INTEGRITY();
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::FreeBuffers(void)
{
	EG_FreeMem(m_pColumnPositions);
	EG_FreeMem(m_pRowPositions);
	EG_FreeMem(m_pColumnWidths);
	EG_FreeMem(m_pRowHeights);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::CalculateColumns(EGObject *pObj)
{
uint32_t i;
bool SubGrid = false;
EG_Coord_t *pSubColumn;

	const EG_Coord_t *pColumnParams = GetColumnParams(pObj);
  if(pColumnParams == nullptr) {
    EGObject *pParent = pObj->GetParent();
    pColumnParams = GetColumnParams(pParent);
    if(pColumnParams == nullptr) {
      EG_LOG_WARN("No col descriptor found even on the parent");
      return;
    }
    EG_Coord_t ColumnPosition = GetColumnPosition(pObj);
    EG_Coord_t ColumnSpan = GetColumnSpan(pObj);
    pSubColumn = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * (ColumnSpan + 1));
    EG_CopyMem(pSubColumn, &pColumnParams[ColumnPosition], sizeof(EG_Coord_t) * ColumnSpan);
    pSubColumn[ColumnSpan] = EG_GRID_TEMPLATE_LAST;
    pColumnParams = pSubColumn;
    SubGrid = true;
  }
	EG_Coord_t ContentWidth = pObj->GetContentWidth();
	m_ColumCount = CountTracks(pColumnParams);
	m_pColumnPositions = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * m_ColumCount);
	m_pColumnWidths = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * m_ColumCount);
	for(i = 0; i < m_ColumCount; i++) {    // Set sizes for CONTENT cells
		EG_Coord_t Size = EG_COORD_MIN;
		if(IS_CONTENT(pColumnParams[i])) {
			for(uint32_t j = 0; j < pObj->GetChildCount(); j++) {			// Check the Size of children of this cell
				EGObject *pItem = pObj->GetChild(j);
				if(pItem->HasAnyFlagSet(EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
				if(GetColumnSpan(pItem) != 1) continue;
				if(GetColumnPosition(pItem) != i) continue;
				Size = EG_MAX(Size, pItem->GetWidth());
			}
			if(Size >= 0)	m_pColumnWidths[i] = Size;
			else m_pColumnWidths[i] = 0;
		}
	}
	uint32_t ColumnFRCount = 0;
	EG_Coord_t GridWidth = 0;
	for(i = 0; i < m_ColumCount; i++) {
		EG_Coord_t x = pColumnParams[i];
		if(IS_FR(x)) {
			ColumnFRCount += GET_FR(x);
		}
		else if(IS_CONTENT(x)) {
			GridWidth += m_pColumnWidths[i];
		}
		else {
			m_pColumnWidths[i] = x;
			GridWidth += x;
		}
	}
	EG_Coord_t ColumnGap = pObj->GetStylePadColumn(EG_PART_MAIN);
	ContentWidth -= ColumnGap * (m_ColumCount - 1);
	EG_Coord_t FreeWidth = ContentWidth - GridWidth;
	if(FreeWidth < 0) FreeWidth = 0;
	for(i = 0; i < m_ColumCount && ColumnFRCount; i++) {
		EG_Coord_t x = pColumnParams[i];
		if(IS_FR(x)) {
			EG_Coord_t f = GET_FR(x);
			m_pColumnWidths[i] = DivRoundClosest(FreeWidth * f, ColumnFRCount);
      ColumnFRCount -= f;
      FreeWidth -= m_pColumnWidths[i]; 
		}
	}
  if(SubGrid) EG_FreeMem((void *)pSubColumn);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::CalculateRows(EGObject *pObj)
{
uint32_t i;
bool SubGrid = false;
EG_Coord_t *pSubRow;

	const EG_Coord_t *pRowParams = GetRowParams(pObj);
  if(pRowParams == nullptr) {
      EGObject *pParent = pObj->GetParent();
      pRowParams = GetRowParams(pParent);
      if(pRowParams == nullptr) {
          EG_LOG_WARN("No row descriptor found even on the parent");
          return;
      }
      EG_Coord_t RowPosition = GetRowPosition(pObj);
      EG_Coord_t RowSpan = GetRowSpan(pObj);
      pSubRow = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * (RowSpan + 1));
      EG_CopyMem(pSubRow, &pRowParams[RowPosition], sizeof(int32_t) * RowSpan);
      pSubRow[RowSpan] = EG_GRID_TEMPLATE_LAST;
      pRowParams = pSubRow;
      SubGrid = true;
  }
	m_RowCount = CountTracks(pRowParams);
	m_pRowPositions = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * m_RowCount);
	m_pRowHeights = (EG_Coord_t*)EG_AllocMem(sizeof(EG_Coord_t) * m_RowCount);
	for(i = 0; i < m_RowCount; i++) {	// Set sizes for CONTENT cells
		EG_Coord_t Size = EG_COORD_MIN;
		if(IS_CONTENT(pRowParams[i])) {
			for(uint32_t j = 0; j < pObj->GetChildCount(); j++) {	// Check the Size of children of this cell
				EGObject *pItem = pObj->GetChild(j);
				if(pItem->HasAnyFlagSet(EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) continue;
				if(GetRowSpan(pItem) != 1) continue;
				if(GetRowPosition(pItem) != i) continue;
				Size = EG_MAX(Size, pItem->GetHeight());
			}
			if(Size >= 0)	m_pRowHeights[i] = Size;
			else m_pRowHeights[i] = 0;
		}
	}
	uint32_t RowFRCount = 0;
	EG_Coord_t GridHeight = 0;
	for(i = 0; i < m_RowCount; i++) {
		EG_Coord_t x = pRowParams[i];
		if(IS_FR(x)) {
			RowFRCount += GET_FR(x);
		}
		else if(IS_CONTENT(x)) {
			GridHeight += m_pRowHeights[i];
		}
		else {
			m_pRowHeights[i] = x;
			GridHeight += x;
		}
	}
	EG_Coord_t RowGap = pObj->GetStylePadRow(EG_PART_MAIN);
	EG_Coord_t ContentHeight = pObj->GetContentHeight() - RowGap * (m_RowCount - 1);
	EG_Coord_t FreeHeight = ContentHeight - GridHeight;
	if(FreeHeight < 0) FreeHeight = 0;
	for(i = 0; i < m_RowCount && RowFRCount; i++) {
		EG_Coord_t x = pRowParams[i];
		if(IS_FR(x)) {
			EG_Coord_t f = GET_FR(x);
			m_pRowHeights[i] = DivRoundClosest(FreeHeight * f, RowFRCount);
      RowFRCount -= f;
      FreeHeight -= m_pRowHeights[i]; 
		}
	}
  if(SubGrid) EG_FreeMem((void *)pSubRow);
}

///////////////////////////////////////////////////////////////////////////////

void EGGridLayout::RepositionItem(EGObject *pItem, EGPoint *pGridPosition)
{
	if(pItem->HasAnyFlagSet(EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) return;
	uint32_t ColumnSpan = GetColumnSpan(pItem);
	uint32_t RowSpan = GetRowSpan(pItem);
	if(RowSpan == 0 || ColumnSpan == 0) return;
	uint32_t ColumnPosition = GetColumnPosition(pItem);
	uint32_t RowPosition = GetRowPosition(pItem);
	EG_GridAlign_e ColumnAlign = (EG_GridAlign_e)GetCellColumnAlign(pItem);
	EG_GridAlign_e RowAlign = (EG_GridAlign_e)GetCellRowAlign(pItem);
	EG_Coord_t ColumnLeft = m_pColumnPositions[ColumnPosition];
	EG_Coord_t ColumnRight = m_pColumnPositions[ColumnPosition + ColumnSpan - 1] + m_pColumnWidths[ColumnPosition + ColumnSpan - 1];
	EG_Coord_t ColumnWidth = ColumnRight - ColumnLeft;
	EG_Coord_t RowTop = m_pRowPositions[RowPosition];
	EG_Coord_t RowBottom = m_pRowPositions[RowPosition + RowSpan - 1] + m_pRowHeights[RowPosition + RowSpan - 1];
	EG_Coord_t RowHeight = RowBottom - RowTop;
 	if(pItem->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) {	//If the pItem has RTL base dir switch start and end
		if(ColumnAlign == EG_GRID_ALIGN_START) ColumnAlign = EG_GRID_ALIGN_END;
		else if(ColumnAlign == EG_GRID_ALIGN_END)	ColumnAlign = EG_GRID_ALIGN_START;
	}
	EG_Coord_t x, y;
	EG_Coord_t ItemWidth = pItem->GetWidth();
	EG_Coord_t ItemHeight = pItem->GetHeight();
	switch(ColumnAlign) {
		default:
		case EG_GRID_ALIGN_START:
			x = m_pColumnPositions[ColumnPosition];
			pItem->m_WidthLayout = 0;
			break;
		case EG_GRID_ALIGN_STRETCH:
			x = m_pColumnPositions[ColumnPosition];
			ItemWidth = ColumnWidth;
			pItem->m_WidthLayout = 1;
			break;
		case EG_GRID_ALIGN_CENTER:
			x = m_pColumnPositions[ColumnPosition] + (ColumnWidth - ItemWidth) / 2;
			pItem->m_WidthLayout = 0;
			break;
		case EG_GRID_ALIGN_END:
			x = m_pColumnPositions[ColumnPosition] + ColumnWidth - pItem->GetWidth();
			pItem->m_WidthLayout = 0;
			break;
	}
	switch(RowAlign) {
		default:
		case EG_GRID_ALIGN_START:
			y = m_pRowPositions[RowPosition];
			pItem->m_HeightLayout = 0;
			break;
		case EG_GRID_ALIGN_STRETCH:
			y = m_pRowPositions[RowPosition];
			ItemHeight = RowHeight;
			pItem->m_HeightLayout = 1;
			break;
		case EG_GRID_ALIGN_CENTER:
			y = m_pRowPositions[RowPosition] + (RowHeight - ItemHeight) / 2;
			pItem->m_HeightLayout = 0;
			break;
		case EG_GRID_ALIGN_END:
			y = m_pRowPositions[RowPosition] + RowHeight - pItem->GetHeight();
			pItem->m_HeightLayout = 0;
			break;
	}
	if(pItem->GetWidth() != ItemWidth || pItem->GetHeight() != ItemHeight) {	// Set a new Size if required
		EGRect Rect(pItem->m_Rect);
		pItem->Invalidate();
		pItem->m_Rect.SetWidth(ItemWidth);
		pItem->m_Rect.SetHeight(ItemHeight);
		pItem->Invalidate();
		EGEvent::EventSend(pItem, EG_EVENT_SIZE_CHANGED, &Rect);
		EGEvent::EventSend(pItem->GetParent(), EG_EVENT_CHILD_CHANGED, pItem);
	}
	EG_Coord_t TrackX = pItem->GetStyleTranslateX(EG_PART_MAIN);	// Handle percentage value of translate
	EG_Coord_t TrackY = pItem->GetStyleTranslateY(EG_PART_MAIN);
	if(EG_COORD_IS_PCT(TrackX)) TrackX = (pItem->GetWidth() * EG_COORD_GET_PCT(TrackX)) / 100;
	if(EG_COORD_IS_PCT(TrackY)) TrackY = (pItem->GetHeight() * EG_COORD_GET_PCT(TrackY)) / 100;
	x += TrackX;
	y += TrackY;
	EG_Coord_t DiffX = pGridPosition->m_X + x - pItem->m_Rect.GetX1();
	EG_Coord_t DiffY = pGridPosition->m_Y + y - pItem->m_Rect.GetY1();
	if(DiffX || DiffY) {
		pItem->Invalidate();
		pItem->m_Rect.Move(DiffX, DiffY);
		pItem->Invalidate();
		pItem->MoveChildrenBy(DiffX, DiffY, false);
	}
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGGridLayout::GridAlign(EG_Coord_t ContentSize, bool AutoSize, uint8_t Alignment, EG_Coord_t Gap, uint32_t TrackCount,
														 EG_Coord_t *pSizeArray, EG_Coord_t *pPositionArray, bool Reverse)
{
EG_Coord_t GridSize = 0;
uint32_t i;

	if(AutoSize) {
		pPositionArray[0] = 0;
	}
	else{		// With spaced alignment Gap will be calculated from the remaining space
		if(Alignment == EG_GRID_ALIGN_SPACE_AROUND || Alignment == EG_GRID_ALIGN_SPACE_BETWEEN || Alignment == EG_GRID_ALIGN_SPACE_EVENLY) {
			Gap = 0;
			if(TrackCount == 1) Alignment = EG_GRID_ALIGN_CENTER;
		}
		for(i = 0; i < TrackCount; i++) {		// Get the full grid Size with Gap
			GridSize += pSizeArray[i] + Gap;
		}
		GridSize -= Gap;
		switch(Alignment) {		// Calculate the position of the first pItem and set Gap is necessary
			case EG_GRID_ALIGN_START:
				pPositionArray[0] = 0;
				break;
			case EG_GRID_ALIGN_CENTER:
				pPositionArray[0] = (ContentSize - GridSize) / 2;
				break;
			case EG_GRID_ALIGN_END:
				pPositionArray[0] = ContentSize - GridSize;
				break;
			case EG_GRID_ALIGN_SPACE_BETWEEN:
				pPositionArray[0] = 0;
				Gap = (EG_Coord_t)(ContentSize - GridSize) / (EG_Coord_t)(TrackCount - 1);
				break;
			case EG_GRID_ALIGN_SPACE_AROUND:
				Gap = (EG_Coord_t)(ContentSize - GridSize) / (EG_Coord_t)(TrackCount);
				pPositionArray[0] = Gap / 2;
				break;
			case EG_GRID_ALIGN_SPACE_EVENLY:
				Gap = (EG_Coord_t)(ContentSize - GridSize) / (EG_Coord_t)(TrackCount + 1);
				pPositionArray[0] = Gap;
				break;
		}
	}
	for(i = 0; i < TrackCount - 1; i++) {	// Set the position of all tracks from the start position, gaps and track sizes
		pPositionArray[i + 1] = pPositionArray[i] + pSizeArray[i] + Gap;
	}
	EG_Coord_t TotalGridSize = pPositionArray[TrackCount - 1] + pSizeArray[TrackCount - 1] - pPositionArray[0];
	if(Reverse) {
		for(i = 0; i < TrackCount; i++) {
			pPositionArray[i] = ContentSize - pPositionArray[i] - pSizeArray[i];
		}
	}
	return TotalGridSize;	// Return the full Size of the grid
}

///////////////////////////////////////////////////////////////////////////////

uint32_t EGGridLayout::CountTracks(const EG_Coord_t *pTracks)
{
uint32_t i;

	for(i = 0; pTracks[i] != EG_GRID_TEMPLATE_LAST; i++);
	return i;
}

#endif 
