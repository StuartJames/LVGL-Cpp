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

#include "core/EG_Object.h"

#if EG_USE_TILEVIEW

///////////////////////////////////////////////////////////////////////////////////////

class EGTileViewTile : public EGObject
{
public:
							 EGTileViewTile(EGObject *pParent);

  EG_DirType_e m_Direction;
};

extern const EG_ClassType_t c_TileViewClass;
extern const EG_ClassType_t c_TileViewTitleClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGTileView : public EGObject
{
public:
                    EGTileView(void);
                    EGTileView(EGObject *pParent);
  virtual void      Configure(void);
  EGTileViewTile*   AddTile(uint8_t ColumnIndex, uint8_t RowIndex, EG_DirType_e Direction);
  EGObject*         GetActiveTile(void){ return m_pActiveTile; };
  void              SetTile(EGTileViewTile *pTile, EG_AnimateEnable_e Enable);
  void              SetTileIndex(uint32_t ColumnIndex, uint32_t RowIndex, EG_AnimateEnable_e Enable);

  static void        EventCB(EGEvent *pEvent);

  EGTileViewTile     *m_pActiveTile;

private:
  static EG_DirType_e m_CreateDirection;
  static uint32_t     m_CreateColumnIndex;
  static uint32_t     m_CreateRowIndex;

};



#endif 
