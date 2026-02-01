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

#pragma once

#include "../EG_IntrnlConfig.h"

#if EG_USE_TABLE != 0

#if EG_USE_LABEL == 0   // Testing of dependencies
#error "lv_table: lv_label is required. Enable it in EG_Config.h (EG_USE_LABEL 1)"
#endif

#include "../core/EG_Object.h"
#include "EG_Label.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_TABLE_CELL_NONE 0XFFFF
EG_EXPORT_CONST_INT(EG_TABLE_CELL_NONE);

///////////////////////////////////////////////////////////////////////////////////////

enum {
    EG_TABLE_CELL_CTRL_NONE = 0,
    EG_TABLE_CELL_CTRL_MERGE_RIGHT = 1 << 0,
    EG_TABLE_CELL_CTRL_TEXT_CROP   = 1 << 1,
    EG_TABLE_CELL_CTRL_CUSTOM_1    = 1 << 4,
    EG_TABLE_CELL_CTRL_CUSTOM_2    = 1 << 5,
    EG_TABLE_CELL_CTRL_CUSTOM_3    = 1 << 6,
    EG_TABLE_CELL_CTRL_CUSTOM_4    = 1 << 7,
};

typedef uint8_t EG_TableCellCtrl_t;

typedef struct {
    EG_TableCellCtrl_t Control;
#if EG_USE_USER_DATA
    void      *pUserData; //  Custom user data
#endif
    char      Text[];
} EG_TableCell_t;

typedef enum {
    EG_TABLE_DRAW_PART_CELL,       //  A cell
} EG_TableDrawPartType_e;

extern const EG_ClassType_t c_TableClass;

///////////////////////////////////////////////////////////////////////////////////////

// Data of table
class EGTable : public EGObject
{
public:
                      EGTable(void);
                      EGTable(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_TableClass);
                      ~EGTable(void);
  virtual void        Configure(void);
  void                SetCellValue(uint16_t Row, uint16_t Column, const char *pText);
  void                SetCellValue(uint16_t Row, uint16_t Column, const char *pFormat, ...);
  void                SetRowCount(uint16_t RowCount);
  void                SetColumnCount(uint16_t ColumnCount);
  void                SetColumnWidth( uint16_t ColumnIndex, EG_Coord_t Width);
  void                AddCellControl(uint16_t Row, uint16_t Column, EG_TableCellCtrl_t Control);
  void                ClearCellControl(uint16_t Row, uint16_t Column, EG_TableCellCtrl_t Control);
  const char*         GetCellValue(uint16_t Row, uint16_t Column);
  uint16_t            GetRowCount(void);
  uint16_t            GetColumnCount(void);
  EG_Coord_t          GetColumnWidth(uint16_t Column);
  bool                CellHasControl(uint16_t Row, uint16_t Column, EG_TableCellCtrl_t Control);
  void                GetSelectedCell(uint16_t *pRow, uint16_t *pColumn);
  void                Event(EGEvent *pEvent);
#if EG_USE_USER_DATA
  void                SetCellUserData(uint16_t Row, uint16_t Column, void *pUserData);
  void*               GetCellUserData(uint16_t Row, uint16_t Column);
#endif
  bool                IsCellEmpty(void *pCell);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  uint16_t            m_ColumnCount;
  uint16_t            m_RowCount;
  EG_TableCell_t    **m_ppCellData;
  EG_Coord_t         *m_pRowHeight;
  EG_Coord_t         *m_pColumnWidth;
  uint16_t            m_ActiveColumn;
  uint16_t            m_ActiveRow;

private:
  void                DrawMain(EGEvent *pEvent);
  void                RefreshSizeFormRow(uint32_t StartRow);
  void                RefreshCellSize(uint32_t Row, uint32_t Column);
  EG_Coord_t          GetRowHeight(uint16_t RowIndex, const EG_Font_t *pFont, EG_Coord_t Kerning, EG_Coord_t LineSpace,
                        EG_Coord_t cell_left, EG_Coord_t cell_right, EG_Coord_t cell_top, EG_Coord_t cell_bottom);
  EG_Result_t         GetPressedCell(uint16_t *pRow, uint16_t *pColumn);
  size_t              GetCellTextLength(const char *pText);
  void                CopyCellText(EG_TableCell_t *pDest, const char *pText);
  void                GetCellRect(uint16_t Row, uint16_t Column, EGRect *pRect);
  void                ScrollToSelected(void);

};

///////////////////////////////////////////////////////////////////////////////////////

inline bool EGTable::IsCellEmpty(void *pCell)
{
	return pCell == nullptr;
}


#endif 