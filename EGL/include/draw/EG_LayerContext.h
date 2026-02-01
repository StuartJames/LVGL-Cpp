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

#include "../EG_IntrnlConfig.h"
#include "EG_DrawContext.h"

//////////////////////////////////////////////////////////////////////////////////////

class EGDrawContext;

typedef enum : uint8_t {
    EG_DRAW_LAYER_FLAG_NONE,
    EG_DRAW_LAYER_FLAG_HAS_ALPHA,
    EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE,
} EGDrawLayerFlags_e;

//////////////////////////////////////////////////////////////////////////////////////

class EGLayerContext
{
public:
                  EGLayerContext();
                  ~EGLayerContext();
  void            Adjust(EGDrawLayerFlags_e Flags);
  void            Blend(EGDrawImage *pDrawImage);
  void            InitialiseBuffer(void){};

  static EGLayerContext*  Create(EGDrawContext *pContext, const EGRect *pLayerArea, EGDrawLayerFlags_e Flags);

  EGDrawContext  *m_pContext;
  EGRect          m_FullRect;
  EGRect          m_ActiveRect;
  EG_Coord_t      m_MaxRowWithAlpha;
  EG_Coord_t      m_MaxRowWithoutAlpha;
  void           *m_pLayerBuffer;
  struct {
    const EGRect  *pClipRect;
    EGRect        *pBuferArea;
    void          *pBuffer;
    bool          ScreenTransparent;
  } m_Original;

//  EGLayerContext  *m_pBaseDraw;
  uint32_t      m_BufferSizeBytes: 31;
  uint32_t      m_HasAlpha : 1;


};
