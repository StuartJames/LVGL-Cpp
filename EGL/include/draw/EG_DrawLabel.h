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

#include "../misc/lv_bidi.h"
#include "../misc/EG_Text.h"
#include "../misc/EG_Color.h"
#include "../misc/EG_Style.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#define EG_DRAW_LABEL_NO_TXT_SEL (0xFFFF)

class EGDrawContext;

///////////////////////////////////////////////////////////////////////////////////////////////////

/** Store some info to speed up drawing of very large texts
 * It takes a lot of time to get the first visible character because
 * all the previous characters needs to be checked to calculate the positions.
 * This structure stores an earlier (e.g. at -1000 px) coordinate and the index of that line.
 * Therefore the calculations can start from here.*/
typedef struct EG_DrawLabelHint_t {
    int32_t LineStart;  // Index of the line at `y` coordinate
    int32_t Y;          // Gives 'y' coordinate of the first letter at `line start` index.
                        // Relative to the label's coordinates
    int32_t CoordY;     // The 'y1' coordinate of the label when the hint was saved.
                        // Used to invalidate the hint if the label has moved too much.
} EG_DrawLabelHint_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDrawLabel
{
public:
                  EGDrawLabel(void);
  void            Draw(const EGDrawContext  *pDrawContext, const EGRect *pRect, const char *pText, EG_DrawLabelHint_t *pHint);
  void            DrawChar(const EGDrawContext  *pDrawContext, const EGPoint *pPos, uint32_t Char);
  void            Reset(void);
  void            operator = (const EGDrawLabel &rval);

  const EGDrawContext  *m_pContext;        
  const EG_Font_t  *m_pFont;
  uint32_t          m_SelectStart;
  uint32_t          m_SelectEnd;
  EG_Color_t        m_Color;
  EG_Color_t        m_SelectColor;
  EG_Color_t        m_SelectBackColor;
  EG_Coord_t        m_LineSpace;
  EG_Coord_t        m_Kerning;
  EG_Coord_t        m_OffsetX;
  EG_Coord_t        m_OffsetY;
  EG_OPA_t          m_OPA;
  EG_BaseDirection_e     m_BidiDirection;
  EG_TextAlignment_t   m_Align;
  EG_TextFlag_t     m_Flag;
  EG_TextDecor_e   m_Decoration : 3;
  EG_BlendMode_e    m_BlendMode: 3;

private:

};

