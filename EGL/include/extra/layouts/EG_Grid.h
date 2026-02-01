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

#pragma once

#include "core/EG_Object.h"

///////////////////////////////////////////////////////////////////////////////

#if EG_USE_GRID

// Can be used as track size to make the track fill the free space.
#define EG_GRID_FR(x)          (EG_COORD_MAX - 100 + x)

#define EG_GRID_CONTENT        (EG_COORD_MAX - 101)
EG_EXPORT_CONST_INT(EG_GRID_CONTENT);

#define EG_GRID_TEMPLATE_LAST  (EG_COORD_MAX)
EG_EXPORT_CONST_INT(EG_GRID_TEMPLATE_LAST);

///////////////////////////////////////////////////////////////////////////////

class EGObject; // Can't include EG_Object.h because it includes this header file

typedef enum : uint8_t {
    EG_GRID_ALIGN_START,
    EG_GRID_ALIGN_STRETCH,
    EG_GRID_ALIGN_CENTER,
    EG_GRID_ALIGN_END,
    EG_GRID_ALIGN_SPACE_EVENLY,
    EG_GRID_ALIGN_SPACE_AROUND,
    EG_GRID_ALIGN_SPACE_BETWEEN,
} EG_GridAlign_e;

///////////////////////////////////////////////////////////////////////////////

class EGGridLayout
{
public:
                              EGGridLayout(void);

  void                        SetStyleRowParams(EGStyle *pStyle, const EG_Coord_t Value[]);
  void                        SetStyleColumnParams(EGStyle *pStyle, const EG_Coord_t Value[]);
  void                        SetStyleRowAlign(EGStyle *pStyle, EG_GridAlign_e Value);
  void                        SetStyleColumnAlign(EGStyle *pStyle, EG_GridAlign_e Value);
  void                        SetStyleCellColumnPosition(EGStyle *pStyle, uint8_t Value);
  void                        SetStyleCellColumnSpan(EGStyle *pStyle, uint8_t Value);
  void                        SetStyleCellRowPosition(EGStyle *pStyle, uint8_t Value);
  void                        SetStyleCellRowSpan(EGStyle *pStyle, uint8_t Value);
  void                        SetStyleCellAlignX(EGStyle *pStyle, uint8_t Value);
  void                        SetStyleCellAlignY(EGStyle *pStyle, uint8_t Value);

  static void                 Initialise(void);
  static void                 UpdateCB(EGObject *pObj, void *pUserData);

  static void                 SetObjGridParams(EGObject *pObj, const EG_Coord_t ColumnProps[], const EG_Coord_t RowProps[]);
  static void                 SetObjAlign(EGObject *pObj, EG_GridAlign_e ColumnAlign, EG_GridAlign_e RowAlign);
  static void                 SetObjCell(EGObject *pObj, EG_GridAlign_e AlignX, uint8_t ColumnPosition, uint8_t ColumnSpan,
                                                         EG_GridAlign_e AlignY, uint8_t RowPosition, uint8_t RowSpan);
  static void                 SetObjStyleRowParams(EGObject *pObj, const EG_Coord_t Value[], EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleColumnParams(EGObject *pObj, const EG_Coord_t Value[], EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleRowAlign(EGObject *pObj, EG_GridAlign_e Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleColumnAlign(EGObject *pObj, EG_GridAlign_e Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleCellColumnPosition(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleCellColumnSpan(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleCellRowPosition(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleCellRowSpan(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleCellAlignX(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags);
  static void                 SetObjStyleCellAlignY(EGObject *pObj, uint8_t Value, EG_StyleFlags_t SelectFlags);
  static const EG_Coord_t*    GetObjRowParams(const EGObject *pObj, uint32_t Part);
  static const EG_Coord_t*    GetObjColumnParams(const EGObject *pObj, uint32_t Part);
  static EG_GridAlign_e       GetObjStyleRowAlign(const EGObject *pObj, uint32_t Part);
  static EG_GridAlign_e       GetObjStyleColumnAlign(const EGObject *pObj, uint32_t Part);
  static uint8_t              GetObjStyleCellColumnPosition(const EGObject *pObj, uint32_t Part);
  static uint8_t              GetObjStyleCellColumnSpan(const EGObject *pObj, uint32_t Part);
  static uint8_t              GetObjStyleCellRowPosition(const EGObject *pObj, uint32_t Part);
  static uint8_t              GetObjStyleCellRowSpan(const EGObject *pObj, uint32_t Part);
  static uint8_t              GetObjStyleCellAlignX(const EGObject *pObj, uint32_t Part);
  static uint8_t              GetObjStyleCellAlignY(const EGObject *pObj, uint32_t Part);

  static inline EG_Coord_t    EGGridFR(uint8_t x){ return EG_GRID_FR(x);};

  static EGStyleProperty_e    STYLE_COLUMN_ARRAY_PROPS;
  static EGStyleProperty_e    STYLE_COLUMN_ALIGN;
  static EGStyleProperty_e    STYLE_ROW_ARRAY_PROPS;
  static EGStyleProperty_e    STYLE_ROW_ALIGN;
  static EGStyleProperty_e    STYLE_CELL_COLUMN_POSITION;
  static EGStyleProperty_e    STYLE_CELL_COLUMN_SPAN;
  static EGStyleProperty_e    STYLE_CELL_ALIGN_X;
  static EGStyleProperty_e    STYLE_CELL_ROW_POSITION;
  static EGStyleProperty_e    STYLE_CELL_ROW_SPAN;
  static EGStyleProperty_e    STYLE_CELL_ALIGN_Y;
  static uint32_t             m_Reference;

private:
  void                        Clear(void);
  void                        Calculate(EGObject *pObj);
  void                        CalculateColumns(EGObject *pObj);
  void                        CalculateRows(EGObject *pObj);
  void                        RepositionItem(EGObject *pItem, EGPoint *pGridPosition);
  EG_Coord_t                  GridAlign(EG_Coord_t ContentSize, bool AutoSize, uint8_t Alignment, EG_Coord_t Gap, uint32_t TrackCount,
														            EG_Coord_t *pSizeArray, EG_Coord_t *pPositionArray, bool Reverse);
  uint32_t                    CountTracks(const EG_Coord_t *pTracks);
  const EG_Coord_t*           GetColumnParams(EGObject *pObj);
  const EG_Coord_t*           GetRowParams(EGObject *pObj);
  uint8_t                     GetColumnPosition(EGObject *pObj);
  uint8_t                     GetRowPosition(EGObject *pObj);
  uint8_t                     GetColumnSpan(EGObject *pObj);
  uint8_t                     GetRowSpan(EGObject *pObj);
  uint8_t                     GetCellColumnAlign(EGObject *pObj);
  uint8_t                     GetCellRowAlign(EGObject *pObj);
  uint8_t                     GetColumnAlign(EGObject *pObj);
  uint8_t                     GetRowAlign(EGObject *pObj);
  void                        FreeBuffers(void);

  inline int32_t              DivRoundClosest(int32_t Dividend, int32_t Divisor);
 
  EG_Coord_t                 *m_pColumnPositions;
	EG_Coord_t                 *m_pRowPositions;
	EG_Coord_t                 *m_pColumnWidths;
	EG_Coord_t                 *m_pRowHeights;
	uint32_t                    m_ColumCount;
	uint32_t                    m_RowCount;
	EG_Coord_t                  m_GridWidth;
	EG_Coord_t                  m_GridHeight;

};

///////////////////////////////////////////////////////////////////////////////

inline int32_t EGGridLayout::DivRoundClosest(int32_t Dividend, int32_t Divisor)
{
    return (Dividend + Divisor / 2) / Divisor;
}

///////////////////////////////////////////////////////////////////////////////

inline const EG_Coord_t* EGGridLayout::GetColumnParams(EGObject *pObj)
{
	return GetObjColumnParams(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline const EG_Coord_t* EGGridLayout::GetRowParams(EGObject *pObj)
{
	return GetObjRowParams(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetColumnPosition(EGObject *pObj)
{
	return GetObjStyleCellColumnPosition(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetRowPosition(EGObject *pObj)
{
	return GetObjStyleCellRowPosition(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetColumnSpan(EGObject *pObj)
{
	return GetObjStyleCellColumnSpan(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetRowSpan(EGObject *pObj)
{
	return GetObjStyleCellRowSpan(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetCellColumnAlign(EGObject *pObj)
{
	return GetObjStyleCellAlignX(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetCellRowAlign(EGObject *pObj)
{
	return GetObjStyleCellAlignY(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetColumnAlign(EGObject *pObj)
{
	return GetObjStyleColumnAlign(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetRowAlign(EGObject *pObj)
{
	return GetObjStyleRowAlign(pObj, 0);
}

///////////////////////////////////////////////////////////////////////////////

inline const EG_Coord_t* EGGridLayout::GetObjRowParams(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_ROW_ARRAY_PROPS);
  return (const EG_Coord_t *)Value.pPtr;
}

///////////////////////////////////////////////////////////////////////////////

inline const EG_Coord_t* EGGridLayout::GetObjColumnParams(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_COLUMN_ARRAY_PROPS);
  return (const EG_Coord_t *)Value.pPtr;
}

///////////////////////////////////////////////////////////////////////////////

inline EG_GridAlign_e EGGridLayout::GetObjStyleRowAlign(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_ROW_ALIGN);
  return (EG_GridAlign_e)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline EG_GridAlign_e EGGridLayout::GetObjStyleColumnAlign(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_COLUMN_ALIGN);
  return (EG_GridAlign_e)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetObjStyleCellColumnPosition(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_CELL_COLUMN_POSITION);
  return (uint8_t)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetObjStyleCellColumnSpan(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_CELL_COLUMN_SPAN);
  return (uint8_t)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetObjStyleCellRowPosition(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_CELL_ROW_POSITION);
  return (uint8_t)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetObjStyleCellRowSpan(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_CELL_ROW_SPAN);
  return (uint8_t)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetObjStyleCellAlignX(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_CELL_ALIGN_X);
  return (uint8_t)Value.Number;
}

///////////////////////////////////////////////////////////////////////////////

inline uint8_t EGGridLayout::GetObjStyleCellAlignY(const EGObject *pObj, uint32_t Part)
{
  EG_StyleValue_t Value = pObj->GetProperty(Part, STYLE_CELL_ALIGN_Y);
  return (uint8_t)Value.Number;
}

#endif