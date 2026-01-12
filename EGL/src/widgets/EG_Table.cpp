

#include "widgets/EG_Table.h"
#if EG_USE_TABLE != 0

#include "core/EG_InputDevice.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Text.h"
#include "misc/lv_txt_ap.h"
#include "misc/EG_Math.h"
#include "misc/lv_printf.h"
#include "draw/EG_DrawContext.h"

#define TABLE_CLASS &c_TableClass

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_TableClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGTable::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGTable::EGTable(void) : EGObject(),
	m_ColumnCount(1),
	m_RowCount(1)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGTable::EGTable(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_TableClass*/) : EGObject(),
	m_ColumnCount(1),
	m_RowCount(1)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGTable::~EGTable()
{
	for(uint16_t i = 0; i < m_ColumnCount * m_RowCount; i++) {
		if(m_ppCellData[i]) {
#if EG_USE_USER_DATA
			if(m_ppCellData[i]->pUserData) {
				EG_FreeMem(m_ppCellData[i]->pUserData);
				m_ppCellData[i]->pUserData = nullptr;
			}
#endif
			EG_FreeMem(m_ppCellData[i]);
			m_ppCellData[i] = nullptr;
		}
	}
	if(m_ppCellData) EG_FreeMem(m_ppCellData);
	if(m_pRowHeight) EG_FreeMem(m_pRowHeight);
	if(m_pColumnWidth) EG_FreeMem(m_pColumnWidth);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::Configure(void)
{
  EGObject::Configure();
	m_ColumnCount = 1;
	m_RowCount = 1;
	m_pColumnWidth = (EG_Coord_t*)EG_AllocMem(m_ColumnCount * sizeof(m_pColumnWidth[0]));
	m_pRowHeight = (EG_Coord_t*)EG_AllocMem(m_RowCount * sizeof(m_pRowHeight[0]));
	m_pColumnWidth[0] = EG_DPI_DEF;
	m_pRowHeight[0] = EG_DPI_DEF;
	m_ppCellData = (EG_TableCell_t**)EG_ReallocMem(m_ppCellData, m_RowCount * m_ColumnCount * sizeof(EG_TableCell_t *));
	m_ppCellData[0] = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::SetCellValue(uint16_t Row, uint16_t Column, const char *pText)
{
	EG_ASSERT_NULL(pText);
	if(Column >= m_ColumnCount) SetColumnCount(Column + 1);	// Auto expand
	if(Row >= m_RowCount) SetRowCount(Row + 1);
	uint32_t Cell = Row * m_ColumnCount + Column;
	EG_TableCellCtrl_t Control = EG_TABLE_CELL_CTRL_NONE;
	if(m_ppCellData[Cell]) Control = m_ppCellData[Cell]->Control;	// Save the control byte
#if EG_USE_USER_DATA
	void *pUserData = nullptr;
	if(m_ppCellData[Cell]) pUserData = m_ppCellData[Cell]->pUserData;	// Save the user data
#endif
	size_t to_allocate = GetCellTextLength(pText);

	m_ppCellData[Cell] = (EG_TableCell_t*)EG_ReallocMem(m_ppCellData[Cell], to_allocate);
	EG_ASSERT_MALLOC(m_ppCellData[Cell]);
	if(m_ppCellData[Cell] == nullptr) return;
	CopyCellText(m_ppCellData[Cell], pText);
	m_ppCellData[Cell]->Control = Control;
#if EG_USE_USER_DATA
	m_ppCellData[Cell]->pUserData = pUserData;
#endif
	RefreshCellSize(Row, Column);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::SetCellValue(uint16_t Row, uint16_t Column, const char *pFormat, ...)
{
	EG_ASSERT_NULL(pFormat);
	if(Column >= m_ColumnCount) SetColumnCount(Column + 1);
	if(Row >= m_RowCount) SetRowCount(Row + 1);	// Auto expand
	uint32_t Cell = Row * m_ColumnCount + Column;
	EG_TableCellCtrl_t Control = EG_TABLE_CELL_CTRL_NONE;
	if(m_ppCellData[Cell]) Control = m_ppCellData[Cell]->Control;	// Save the control byte
#if EG_USE_USER_DATA
	void *pUserData = nullptr;
	if(m_ppCellData[Cell]) pUserData = m_ppCellData[Cell]->pUserData;	// Save the pUserData
#endif
	va_list ap, ap2;
	va_start(ap, pFormat);
	va_copy(ap2, ap);
	uint32_t len = eg_vsnprintf(NULL, 0, pFormat, ap);	// Allocate space for the new text by using trick from C99 standard section 7.19.6.12
	va_end(ap);
#if EG_USE_ARABIC_PERSIAN_CHARS
	// Put together the text according to the format string
	char *raw_txt = GetBufferMem(len + 1);
	EG_ASSERT_MALLOC(raw_txt);
	if(raw_txt == NULL) {
		va_end(ap2);
		return;
	}
	eg_vsnprintf(raw_txt, len + 1, pFormat, ap2);
	size_t len_ap = _lv_txt_ap_calc_bytes_cnt(raw_txt);// Get the size of the Arabic text and process it
	m_ppCellData[Cell] = EG_ReallocMem(m_ppCellData[Cell], sizeof(EG_TableCell_t) + len_ap + 1);
	EG_ASSERT_MALLOC(m_ppCellData[Cell]);
	if(m_ppCellData[Cell] == NULL) {
		va_end(ap2);
		return;
	}
	_lv_txt_ap_proc(raw_txt, m_ppCellData[Cell]->pText);
	EG_ReleaseBufferMem(raw_txt);
#else
	m_ppCellData[Cell] = (EG_TableCell_t*)EG_ReallocMem(m_ppCellData[Cell],	sizeof(EG_TableCell_t) + len + 1); // +1: trailing '\0; 
	EG_ASSERT_MALLOC(m_ppCellData[Cell]);
	if(m_ppCellData[Cell] == nullptr) {
		va_end(ap2);
		return;
	}
	m_ppCellData[Cell]->Text[len] = 0; // Ensure NULL termination
	eg_vsnprintf(m_ppCellData[Cell]->Text, len + 1, pFormat, ap2);
#endif
	va_end(ap2);
	m_ppCellData[Cell]->Control = Control;
#if EG_USE_USER_DATA
	m_ppCellData[Cell]->pUserData = pUserData;
#endif
	RefreshCellSize(Row, Column);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::SetRowCount(uint16_t RowCount)
{
	if(m_RowCount == RowCount) return;
	uint16_t old_row_cnt = m_RowCount;
	m_RowCount = RowCount;
	m_pRowHeight = (EG_Coord_t*)EG_ReallocMem(m_pRowHeight, m_RowCount * sizeof(m_pRowHeight[0]));
	EG_ASSERT_MALLOC(m_pRowHeight);
	if(m_pRowHeight == nullptr) return;
	if(old_row_cnt > RowCount) {	// Free the unused cells
		uint16_t old_cell_cnt = old_row_cnt * m_ColumnCount;
		uint32_t new_cell_cnt = m_ColumnCount * m_RowCount;
		uint32_t i;
		for(i = new_cell_cnt; i < old_cell_cnt; i++) {
#if EG_USE_USER_DATA
			if(m_ppCellData[i]->pUserData) {
				EG_FreeMem(m_ppCellData[i]->pUserData);
				m_ppCellData[i]->pUserData = nullptr;
			}
#endif
			EG_FreeMem(m_ppCellData[i]);
		}
	}
	m_ppCellData = (EG_TableCell_t**)EG_ReallocMem(m_ppCellData, m_RowCount * m_ColumnCount * sizeof(EG_TableCell_t *));
	EG_ASSERT_MALLOC(m_ppCellData);
	if(m_ppCellData == nullptr) return;
	if(old_row_cnt < RowCount) {	// Initialize the new fields
		uint32_t old_cell_cnt = old_row_cnt * m_ColumnCount;
		uint32_t new_cell_cnt = m_ColumnCount * m_RowCount;
		EG_ZeroMem(&m_ppCellData[old_cell_cnt], (new_cell_cnt - old_cell_cnt) * sizeof(m_ppCellData[0]));
	}
	RefreshSizeFormRow(0);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::SetColumnCount(uint16_t ColumnCount)
{
	if(m_ColumnCount == ColumnCount) return;
	uint16_t old_col_cnt = m_ColumnCount;
	m_ColumnCount = ColumnCount;
	EG_TableCell_t **new_cell_data = (EG_TableCell_t**)EG_AllocMem(m_RowCount * m_ColumnCount * sizeof(EG_TableCell_t *));
	EG_ASSERT_MALLOC(new_cell_data);
	if(new_cell_data == nullptr) return;
	uint32_t new_cell_cnt = m_ColumnCount * m_RowCount;
	EG_ZeroMem(new_cell_data, new_cell_cnt * sizeof(m_ppCellData[0]));
	uint32_t old_col_start;	// The new column(s) messes up the mapping of `cell_data`
	uint32_t new_col_start;
	uint32_t min_col_cnt = EG_MIN(old_col_cnt, ColumnCount);
	uint32_t Row;
	for(Row = 0; Row < m_RowCount; Row++) {
		old_col_start = Row * old_col_cnt;
		new_col_start = Row * ColumnCount;
		EG_CopyMemSmall(&new_cell_data[new_col_start], &m_ppCellData[old_col_start], sizeof(new_cell_data[0]) * min_col_cnt);
		// Free the old cells (only if the table becomes smaller)
		int32_t i;
		for(i = 0; i < (int32_t)old_col_cnt - ColumnCount; i++) {
			uint32_t idx = old_col_start + min_col_cnt + i;
#if EG_USE_USER_DATA
			if(m_ppCellData[idx]->pUserData) {
				EG_FreeMem(m_ppCellData[idx]->pUserData);
				m_ppCellData[idx]->pUserData = nullptr;
			}
#endif
			EG_FreeMem(m_ppCellData[idx]);
			m_ppCellData[idx] = nullptr;
		}
	}
	EG_FreeMem(m_ppCellData);
	m_ppCellData = new_cell_data;
	m_pColumnWidth = (EG_Coord_t*)EG_ReallocMem(m_pColumnWidth, ColumnCount * sizeof(m_pColumnWidth[0]));
	EG_ASSERT_MALLOC(m_pColumnWidth);
	if(m_pColumnWidth == nullptr) return;
	uint32_t Column;
	for(Column = old_col_cnt; Column < ColumnCount; Column++) {
		m_pColumnWidth[Column] = EG_DPI_DEF;
	}
	RefreshSizeFormRow(0);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::SetColumnWidth( uint16_t ColumnIndex, EG_Coord_t Width)
{
	if(ColumnIndex >= m_ColumnCount) SetColumnCount(ColumnIndex + 1);
	m_pColumnWidth[ColumnIndex] = Width;
	RefreshSizeFormRow(0);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::AddCellControl(uint16_t Row, uint16_t Column, EG_TableCellCtrl_t Control)
{
	if(Column >= m_ColumnCount) SetColumnCount(Column + 1);
	if(Row >= m_RowCount) SetRowCount(Row + 1);
	uint32_t Cell = Row * m_ColumnCount + Column;
	if(IsCellEmpty(m_ppCellData[Cell])) {
		m_ppCellData[Cell] = (EG_TableCell_t*)EG_AllocMem(sizeof(EG_TableCell_t) + 1); // +1: trailing '\0 
		EG_ASSERT_MALLOC(m_ppCellData[Cell]);
		if(m_ppCellData[Cell] == nullptr) return;
		m_ppCellData[Cell]->Control = EG_TABLE_CELL_CTRL_NONE;
#if EG_USE_USER_DATA
		m_ppCellData[Cell]->pUserData = nullptr;
#endif
		m_ppCellData[Cell]->Text[0] = '\0';
	}
	m_ppCellData[Cell]->Control |= Control;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::ClearCellControl(uint16_t Row, uint16_t Column, EG_TableCellCtrl_t Control)
{
	if(Column >= m_ColumnCount) SetColumnCount(Column + 1);
	if(Row >= m_RowCount) SetRowCount(Row + 1);
	uint32_t Cell = Row * m_ColumnCount + Column;
	if(IsCellEmpty(m_ppCellData[Cell])) {
		m_ppCellData[Cell] = (EG_TableCell_t*)EG_AllocMem(sizeof(EG_TableCell_t) + 1); // +1: trailing '\0 
		EG_ASSERT_MALLOC(m_ppCellData[Cell]);
		if(m_ppCellData[Cell] == nullptr) return;
		m_ppCellData[Cell]->Control = 0;
#if EG_USE_USER_DATA
		m_ppCellData[Cell]->pUserData = nullptr;
#endif
		m_ppCellData[Cell]->Text[0] = '\0';
	}
	m_ppCellData[Cell]->Control &= (~Control);
}

#if EG_USE_USER_DATA
#endif

///////////////////////////////////////////////////////////////////////////////////////

const char* EGTable::GetCellValue(uint16_t Row, uint16_t Column)
{
	if(Row >= m_RowCount || Column >= m_ColumnCount) {
		EG_LOG_WARN("invalid Row or column");
		return "";
	}
	uint32_t Cell = Row * m_ColumnCount + Column;
	if(IsCellEmpty(m_ppCellData[Cell])) return "";
	return m_ppCellData[Cell]->Text;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGTable::GetRowCount(void)
{
	return m_RowCount;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGTable::GetColumnCount(void)
{
	return m_ColumnCount;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGTable::GetColumnWidth(uint16_t Column)
{
	if(Column >= m_ColumnCount) {
		EG_LOG_WARN("lv_table_set_col_width: too big 'col_id'. Must be < EG_TABLE_COL_MAX.");
		return 0;
	}
	return m_pColumnWidth[Column];
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGTable::CellHasControl(uint16_t Row, uint16_t Column, EG_TableCellCtrl_t Control)
{
	if(Row >= m_RowCount || Column >= m_ColumnCount) {
		EG_LOG_WARN("lv_table_get_cell_crop: invalid Row or column");
		return false;
	}
	uint32_t Cell = Row * m_ColumnCount + Column;
	if(IsCellEmpty(m_ppCellData[Cell]))	return false;
	return (m_ppCellData[Cell]->Control & Control) == Control;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::GetSelectedCell(uint16_t *pRow, uint16_t *pColumn)
{
	*pRow = m_ActiveRow;
	*pColumn = m_ActiveColumn;
}

///////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_USER_DATA

void EGTable::SetCellUserData(uint16_t Row, uint16_t Column, void *pUserData)
{
	if(Column >= m_ColumnCount) SetColumnCount(Column + 1);
	if(Row >= m_RowCount) SetRowCount(Row + 1);
	uint32_t Cell = Row * m_ColumnCount + Column;
	if(IsCellEmpty(m_ppCellData[Cell])) {
		m_ppCellData[Cell] = (EG_TableCell_t*)EG_AllocMem(sizeof(EG_TableCell_t) + 1); // +1: trailing '\0 
		EG_ASSERT_MALLOC(m_ppCellData[Cell]);
		if(m_ppCellData[Cell] == nullptr) return;
		m_ppCellData[Cell]->Control = 0;
		m_ppCellData[Cell]->pUserData = nullptr;
		m_ppCellData[Cell]->Text[0] = '\0';
	}
	if(m_ppCellData[Cell]->pUserData) {
		EG_FreeMem(m_ppCellData[Cell]->pUserData);
	}
	m_ppCellData[Cell]->pUserData = pUserData;
}

///////////////////////////////////////////////////////////////////////////////////////

void* EGTable::GetCellUserData(uint16_t Row, uint16_t Column)
{
	if(Row >= m_RowCount || Column >= m_ColumnCount) {
		EG_LOG_WARN("invalid Row or column");
		return nullptr;
	}
	uint32_t Cell = Row * m_ColumnCount + Column;
	if(IsCellEmpty(m_ppCellData[Cell])) return nullptr;
	return m_ppCellData[Cell]->pUserData;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(TABLE_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGTable *pTable = (EGTable*)pEvent->GetTarget();
  pTable->Event(pEvent); // dereference once
}

void EGTable::Event(EGEvent *pEvent)
{
  switch(pEvent->GetCode()){
    case EG_EVENT_STYLE_CHANGED: {
      RefreshSizeFormRow(0);
      break;
    }
    case EG_EVENT_GET_SELF_SIZE: {
      EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
      uint32_t i;
      EG_Coord_t Width = 0;
      for(i = 0; i < m_ColumnCount; i++) Width += m_pColumnWidth[i];
      EG_Coord_t Height = 0;
      for(i = 0; i < m_RowCount; i++) Height += m_pRowHeight[i];
      pPoint->m_X = Width - 1;
      pPoint->m_Y = Height - 1;
      break;
    }
    case EG_EVENT_PRESSED:
    case EG_EVENT_PRESSING: {
      uint16_t Column;
      uint16_t Row;
      EG_Result_t pr_res = GetPressedCell(&Row, &Column);
      if(pr_res == EG_RES_OK && (m_ActiveColumn != Column || m_ActiveRow != Row)) {
        m_ActiveColumn = Column;
        m_ActiveRow = Row;
        Invalidate();
      }
      break;
    }
    case EG_EVENT_RELEASED: {
      Invalidate();
      EGInputDevice *pInput = EGInputDevice::GetActive();
      EGObject *pScrollObj = pInput->GetScrollObj();
      if(m_ActiveColumn != EG_TABLE_CELL_NONE && m_ActiveRow != EG_TABLE_CELL_NONE && pScrollObj == nullptr) {
        if(EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr) != EG_RES_OK) return;
      }
      EG_InDeviceType_e InputType = pInput->GetType();
      if(InputType == EG_INDEV_TYPE_POINTER || InputType == EG_INDEV_TYPE_BUTTON) {
        m_ActiveColumn = EG_TABLE_CELL_NONE;
        m_ActiveRow = EG_TABLE_CELL_NONE;
      }
      break;
    }
    case EG_EVENT_FOCUSED: {
      Invalidate();
      break;
    }
    case EG_EVENT_KEY: {
      int32_t Key = *((int32_t *)pEvent->GetParam());
      int32_t Column = m_ActiveColumn;
      int32_t Row = m_ActiveRow;
      if(Column == EG_TABLE_CELL_NONE || Row == EG_TABLE_CELL_NONE) {
        m_ActiveColumn = 0;
        m_ActiveRow = 0;
        ScrollToSelected();
        Invalidate();
        return;
      }
      if(Column >= m_ColumnCount) Column = 0;
      if(Row >= m_RowCount) Row = 0;
      switch(Key){
        case EG_KEY_LEFT: Column--; break;
        case EG_KEY_RIGHT: Column++; break;
        case EG_KEY_UP: Row--; break;
        case EG_KEY_DOWN: Row++; break;
        default: return;
      }
      if(Column >= m_ColumnCount) {
        if(Row < m_RowCount - 1) {
          Column = 0;
          Row++;
        }
        else Column = m_ColumnCount - 1;
      }
      else if(Column < 0) {
        if(Row != 0) {
          Column = m_ColumnCount - 1;
          Row--;
        }
        else Column = 0;
      }
      if(Row >= m_RowCount) {
        Row = m_RowCount - 1;
      }
      else if(Row < 0) Row = 0;
      if(m_ActiveColumn != Column || m_ActiveRow != Row) {
        m_ActiveColumn = Column;
        m_ActiveRow = Row;
        Invalidate();
        ScrollToSelected();
        if(EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, nullptr) != EG_RES_OK) return;
      }
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      DrawMain(pEvent);
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::DrawMain(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	EGRect ClipRect;
	if(!ClipRect.Intersect(&m_Rect, pContext->m_pClipRect)) return;
	const EGRect *pClipRectOriginal = pContext->m_pClipRect;
	pContext->m_pClipRect = &ClipRect;
	EGPoint TextSize;
	EGRect CellRect;
	EG_Coord_t m_BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t PaddingTop = GetStylePadTop(EG_PART_MAIN);
	EG_Coord_t PaddingBottom = GetStylePadBottom(EG_PART_MAIN);
	EG_Coord_t PaddingLeft = GetStylePadLeft(EG_PART_MAIN);
	EG_Coord_t PaddingRight = GetStylePadRight(EG_PART_MAIN);
	EGState_t OriginalState = m_State;
	m_State = EG_STATE_DEFAULT;
	m_SkipTransition = 1;
	EGDrawRect DrawRectDef;
	EGDrawRect DrawRectActive; // Passed to the event to modify it
	InititialseDrawRect(EG_PART_ITEMS, &DrawRectDef);
	EGDrawLabel DrawLabelDef;
	EGDrawLabel DrawLabelActive; // Passed to the event to modify it
	InititialseDrawLabel(EG_PART_ITEMS, &DrawLabelDef);
	m_State = OriginalState;
	m_SkipTransition = 0;
	uint16_t Column;
	uint16_t Row;
	uint16_t Cell = 0;
	CellRect.SetY2(m_Rect.GetY1() + PaddingTop - 1 - GetScrollY() + m_BorderWidth);
	EG_Coord_t scroll_x = GetScrollX();
	bool RTL = GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL;
	EGDrawDiscriptor PartDrawDiscriptor;	// Handle custom drawer
	InitDrawDescriptor(&PartDrawDiscriptor, pContext);
	PartDrawDiscriptor.m_Part = EG_PART_ITEMS;
	PartDrawDiscriptor.m_pClass = m_pClass;
	PartDrawDiscriptor.m_Type = EG_TABLE_DRAW_PART_CELL;
	PartDrawDiscriptor.m_pDrawRect = &DrawRectActive;
	PartDrawDiscriptor.m_pDrawLabel = &DrawLabelActive;
	for(Row = 0; Row < m_RowCount; Row++) {
		EG_Coord_t RowHeight = m_pRowHeight[Row];
		CellRect.SetY1(CellRect.GetY2() + 1);
		CellRect.SetY2(CellRect.GetY1() + RowHeight - 1);
		if(CellRect.GetY1() > ClipRect.GetY2()) break;
		if(RTL)	CellRect.SetX1(m_Rect.GetX2() - PaddingRight - 1 - scroll_x - m_BorderWidth);
		else CellRect.SetX2(m_Rect.GetX1() + PaddingLeft - 1 - scroll_x + m_BorderWidth);
		for(Column = 0; Column < m_ColumnCount; Column++) {
			EG_TableCellCtrl_t Control = 0;
			if(m_ppCellData[Cell]) Control = m_ppCellData[Cell]->Control;
			if(RTL) {
				CellRect.SetX2(CellRect.GetX1() - 1);
				CellRect.SetX1(CellRect.GetX2() - m_pColumnWidth[Column] + 1);
			}
			else {
				CellRect.SetX1(CellRect.GetX2() + 1);
				CellRect.SetX2(CellRect.GetX1() + m_pColumnWidth[Column] - 1);
			}
			uint16_t col_merge = 0;
			for(col_merge = 0; col_merge + Column < m_ColumnCount - 1; col_merge++) {
				EG_TableCell_t *next_cell_data = m_ppCellData[Cell + col_merge];
				if(IsCellEmpty(next_cell_data)) break;
				EG_TableCellCtrl_t merge_ctrl = (EG_TableCellCtrl_t)next_cell_data->Control;
				if(merge_ctrl & EG_TABLE_CELL_CTRL_MERGE_RIGHT) {
					EG_Coord_t Offset = m_pColumnWidth[Column + col_merge + 1];
					if(RTL)	CellRect.DecX1(Offset);
					else CellRect.IncX2(Offset);
				}
				else break;
			}

			if(CellRect.GetY2() < ClipRect.GetY1()) {
				Cell += col_merge + 1;
				Column += col_merge;
				continue;
			}
			// Expand the Cell pRect with a half border to avoid drawing 2 borders next to each other
			EGRect CellBorderRect(CellRect);
			if((DrawRectDef.m_BorderSide & EG_BORDER_SIDE_LEFT) && CellBorderRect.GetX1() > m_Rect.GetX1() + PaddingLeft) {
				CellBorderRect.DecX1(DrawRectDef.m_BorderWidth / 2);
			}
			if((DrawRectDef.m_BorderSide & EG_BORDER_SIDE_TOP) && CellBorderRect.GetY1() > m_Rect.GetY1() + PaddingTop) {
				CellBorderRect.DecY1(DrawRectDef.m_BorderWidth / 2);
			}
			if((DrawRectDef.m_BorderSide & EG_BORDER_SIDE_RIGHT) && CellBorderRect.GetX2() < m_Rect.GetX2() - PaddingRight - 1) {
				CellBorderRect.IncX2(DrawRectDef.m_BorderWidth / 2 + (DrawRectDef.m_BorderWidth & 0x1));
			}
			if((DrawRectDef.m_BorderSide & EG_BORDER_SIDE_BOTTOM) &&
				 CellBorderRect.GetY2() < m_Rect.GetY2() - PaddingBottom - 1) {
				CellBorderRect.IncY2(DrawRectDef.m_BorderWidth / 2 + (DrawRectDef.m_BorderWidth & 0x1));
			}
			EGState_t CellState = EG_STATE_DEFAULT;
			if(Row == m_ActiveRow && Column == m_ActiveColumn) {
				if(!(m_State & EG_STATE_SCROLLED) && (m_State & EG_STATE_PRESSED)) CellState |= EG_STATE_PRESSED;
				if(m_State & EG_STATE_FOCUSED) CellState |= EG_STATE_FOCUSED;
				if(m_State & EG_STATE_FOCUS_KEY) CellState |= EG_STATE_FOCUS_KEY;
				if(m_State & EG_STATE_EDITED) CellState |= EG_STATE_EDITED;
			}
			// Set up the draw descriptors
			if(CellState == EG_STATE_DEFAULT) {
				EG_CopyMem(&DrawRectActive, &DrawRectDef, sizeof(EGDrawRect));
				EG_CopyMem(&DrawLabelActive, &DrawLabelDef, sizeof(EGDrawLabel));
			}
			// In other cases get the styles directly without caching them
			else {
				m_State = CellState;
				m_SkipTransition = 1;
				InititialseDrawRect(EG_PART_ITEMS, &DrawRectActive);
				InititialseDrawLabel(EG_PART_ITEMS, &DrawLabelActive);
				m_State = OriginalState;
				m_SkipTransition = 0;
			}

			PartDrawDiscriptor.m_pRect = &CellBorderRect;
			PartDrawDiscriptor.m_Index = Row * m_ColumnCount + Column;
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &PartDrawDiscriptor);
			DrawRectActive.Draw(pContext, &CellBorderRect);
			if(m_ppCellData[Cell]) {
				const EG_Coord_t CellLeft = GetStylePadLeft(EG_PART_ITEMS);
				const EG_Coord_t CellRight = GetStylePadRight(EG_PART_ITEMS);
				const EG_Coord_t CellTop = GetStylePadTop(EG_PART_ITEMS);
				const EG_Coord_t CellBottom = GetStylePadBottom(EG_PART_ITEMS);
				EG_TextFlag_t txt_flags = EG_TEXT_FLAG_NONE;
				EGRect TextRect;
				TextRect.SetX1(CellRect.GetX1() + CellLeft);
				TextRect.SetX2(CellRect.GetX2() - CellRight);
				TextRect.SetY1(CellRect.GetY1() + CellTop);
				TextRect.SetY2(CellRect.GetY2() - CellBottom);
				// Align the content to the middle if not cropped
				bool crop = Control & EG_TABLE_CELL_CTRL_TEXT_CROP ? true : false;
				if(crop) txt_flags = EG_TEXT_FLAG_EXPAND;
				EG_GetTextSize(&TextSize, m_ppCellData[Cell]->Text, DrawLabelDef.m_pFont,
												DrawLabelActive.m_Kerning, DrawLabelActive.m_LineSpace,
												TextRect.GetWidth(), txt_flags);
				// Align the content to the middle if not cropped
				if(!crop) {
					TextRect.SetY1(CellRect.GetY1() + RowHeight / 2 - TextSize.m_Y / 2);
					TextRect.SetY2(CellRect.GetY1() + RowHeight / 2 + TextSize.m_Y / 2);
				}
				EGRect LabelClipRect;
				if(LabelClipRect.Intersect(&ClipRect, &CellRect)) {
					pContext->m_pClipRect = &LabelClipRect;
					DrawLabelActive.Draw(pContext, &TextRect, m_ppCellData[Cell]->Text, nullptr);
					pContext->m_pClipRect = &ClipRect;
				}
			}
			EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &PartDrawDiscriptor);
			Cell += col_merge + 1;
			Column += col_merge;
		}
	}
	pContext->m_pClipRect = pClipRectOriginal;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::RefreshSizeFormRow(uint32_t StartRow)
{
	const EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_ITEMS);
	const EG_Coord_t PadRight = GetStylePadRight(EG_PART_ITEMS);
	const EG_Coord_t PadTop = GetStylePadTop(EG_PART_ITEMS);
	const EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_ITEMS);

	EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_ITEMS);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_ITEMS);
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_ITEMS);

	const EG_Coord_t minh = GetStyleMinHeight(EG_PART_ITEMS);
	const EG_Coord_t maxh = GetStyleMaxHeight(EG_PART_ITEMS);

	uint32_t i;
	for(i = StartRow; i < m_RowCount; i++) {
		EG_Coord_t RowHeight = GetRowHeight(i, pFont, Kerning, LineSpace, PadLeft, PadRight, PadTop, PadBottom);
		m_pRowHeight[i] = LV_CLAMP(minh, RowHeight, maxh);
	}
	RefreshSelfSize();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::RefreshCellSize(uint32_t Row, uint32_t Column)
{
	const EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_ITEMS);
	const EG_Coord_t PadRight = GetStylePadRight(EG_PART_ITEMS);
	const EG_Coord_t PadTop = GetStylePadTop(EG_PART_ITEMS);
	const EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_ITEMS);

	EG_Coord_t Kerning = GetStyleTextKerning(EG_PART_ITEMS);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_ITEMS);
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_ITEMS);

	const EG_Coord_t minh = GetStyleMinHeight(EG_PART_ITEMS);
	const EG_Coord_t maxh = GetStyleMaxHeight(EG_PART_ITEMS);

	EG_Coord_t RowHeight = GetRowHeight(Row, pFont, Kerning, LineSpace, PadLeft, PadRight, PadTop, PadBottom);
	EG_Coord_t PreviousRowHeight = m_pRowHeight[Row];
	m_pRowHeight[Row] = LV_CLAMP(minh, RowHeight, maxh);

	// If the Row height havn't changed invalidate only this Cell
	if(PreviousRowHeight == m_pRowHeight[Row]) {
		EGRect CellRect;
		GetCellRect(Row, Column, &CellRect);
		CellRect.Move(m_Rect.GetX1(), m_Rect.GetY1());
		InvalidateArea(&CellRect);
	}
	else {
		RefreshSelfSize();
		Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGTable::GetRowHeight(uint16_t RowIndex, const EG_Font_t *pFont, EG_Coord_t Kerning, EG_Coord_t LineSpace,
                    EG_Coord_t CellLeft, EG_Coord_t CellRight, EG_Coord_t CellTop, EG_Coord_t CellBottom)
{
	EG_Coord_t MaxHeight = EG_FontGetLineHeight(pFont) + CellTop + CellBottom;
	uint16_t RowStart = RowIndex * m_ColumnCount;	//  Calculate the cell_data index where to start 
	uint16_t Cell;	//  Traverse the cells in the RowIndex Row 
	uint16_t Column;
	for(Cell = RowStart, Column = 0; Cell < RowStart + m_ColumnCount; Cell++, Column++) {
		EG_TableCell_t *cell_data = m_ppCellData[Cell];
		if(IsCellEmpty(cell_data)) {
			continue;
		}
		EG_Coord_t TextWidth = m_pColumnWidth[Column];
		uint16_t col_merge = 0;
		for(col_merge = 0; col_merge + Column < m_ColumnCount - 1; col_merge++) {
			EG_TableCell_t *next_cell_data = m_ppCellData[Cell + col_merge];
			if(IsCellEmpty(next_cell_data)) break;
			EG_TableCellCtrl_t Control = (EG_TableCellCtrl_t)next_cell_data->Control;
			if(Control & EG_TABLE_CELL_CTRL_MERGE_RIGHT) TextWidth += m_pColumnWidth[Column + col_merge + 1];
			else break;
		}
		EG_TableCellCtrl_t Control = (EG_TableCellCtrl_t)cell_data->Control;
		// When cropping the text we can assume the Row height is equal to the line height
		if(Control & EG_TABLE_CELL_CTRL_TEXT_CROP) {
			MaxHeight = EG_MAX(EG_FontGetLineHeight(pFont) + CellTop + CellBottom, MaxHeight);
		}
		else {		// Else we have to calculate the height of the Cell text
			EGPoint TextSize;
			TextWidth -= CellLeft + CellRight;
			EG_GetTextSize(&TextSize, m_ppCellData[Cell]->Text, pFont,	Kerning, LineSpace, TextWidth, EG_TEXT_FLAG_NONE);
			MaxHeight = EG_MAX(TextSize.m_Y + CellTop + CellBottom, MaxHeight);
			Cell += col_merge;			// Skip until one element after the last merged column
			Column += col_merge;
		}
	}
	return MaxHeight;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGTable::GetPressedCell(uint16_t *pRow, uint16_t *pColumn)
{
	EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
	if(InputType != EG_INDEV_TYPE_POINTER && InputType != EG_INDEV_TYPE_BUTTON) {
		if(pColumn) *pColumn = EG_TABLE_CELL_NONE;
		if(pRow) *pRow = EG_TABLE_CELL_NONE;
		return EG_RES_INVALID;
	}
	EGPoint pPoint;
	EGInputDevice::GetActive()->GetPoint(&pPoint);
	EG_Coord_t Temp;
	if(pColumn) {
		EG_Coord_t x = pPoint.m_X + GetScrollX();
		if(GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL) {
			x = m_Rect.GetX2() - GetStylePadRight(EG_PART_MAIN) - x;
		}
		else {
			x -= m_Rect.GetX1();
			x -= GetStylePadLeft(EG_PART_MAIN);
		}
		*pColumn = 0;
		Temp = 0;
		for(*pColumn = 0; *pColumn < m_ColumnCount; (*pColumn)++) {
			Temp += m_pColumnWidth[*pColumn];
			if(x < Temp) break;
		}
	}
	if(pRow) {
		EG_Coord_t y = pPoint.m_Y + GetScrollY();
		y -= m_Rect.GetY1();
		y -= GetStylePadTop(EG_PART_MAIN);
		*pRow = 0;
		Temp = 0;
		for(*pRow = 0; *pRow < m_RowCount; (*pRow)++) {
			Temp += m_pRowHeight[*pRow];
			if(y < Temp) break;
		}
	}

	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

size_t EGTable::GetCellTextLength(const char *pText)
{
size_t retval = 0;

#if EG_USE_ARABIC_PERSIAN_CHARS
	retval = sizeof(EG_TableCell_t) + _lv_txt_ap_calc_bytes_cnt(pText) + 1;
#else
	retval = sizeof(EG_TableCell_t) + strlen(pText) + 1;
#endif
	return retval;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::CopyCellText(EG_TableCell_t *pDest, const char *pText)
{
#if EG_USE_ARABIC_PERSIAN_CHARS
	_lv_txt_ap_proc(pText, dst->pText);
#else
	strcpy(pDest->Text, pText);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::GetCellRect(uint16_t Row, uint16_t Column, EGRect *pRect)
{
	uint32_t Key;
	pRect->SetX1(0);
	for(Key = 0; Key < Column; Key++) pRect->IncX1(m_pColumnWidth[Key]);
	bool RTL = GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL;
	if(RTL) {
		pRect->IncX1(GetScrollX());
		EG_Coord_t Width = GetWidth();
		pRect->SetX2(Width - pRect->GetX1() - GetStylePadRight(0));
		pRect->SetX1(pRect->GetX2() - m_pColumnWidth[Column]);
	}
	else {
		pRect->DecX1(GetScrollX());
		pRect->IncX1(GetStylePadLeft(0));
		pRect->SetX2(pRect->GetX1() + m_pColumnWidth[Column] - 1);
	}
	uint32_t r;
	pRect->SetY1(0);
	for(r = 0; r < Row; r++) pRect->IncY1(m_pRowHeight[r]);
	pRect->IncY1(GetStylePadTop(0));
	pRect->DecY1(GetScrollY());
	pRect->SetY2(pRect->GetY1() + m_pRowHeight[Row] - 1);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGTable::ScrollToSelected(void)
{
	EGRect Rect;
	GetCellRect(m_ActiveRow, m_ActiveColumn, &Rect);
	if(Rect.GetX1() < 0) {
		ScrollByBounded(-Rect.GetX1(), 0, EG_ANIM_ON);
	}
	else if(Rect.GetX2() > GetWidth()) {
		ScrollByBounded(GetWidth() - Rect.GetX2(), 0, EG_ANIM_ON);
	}

	if(Rect.GetY1() < 0) {
		ScrollByBounded(0, -Rect.GetY1(), EG_ANIM_ON);
	}
	else if(Rect.GetY2() > GetHeight()) {
		ScrollByBounded(0, GetHeight() - Rect.GetY2(), EG_ANIM_ON);
	}
}

#endif
