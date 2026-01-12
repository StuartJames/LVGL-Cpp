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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

//#pragma once

#ifndef __EGSOFTCONTEXT__
#define __EGSOFTCONTEXT__

//#include "EG_DrawSoftBlend.h"     // lv_draw_sw_blend
#include "../EG_DrawContext.h"    // lv_draw
#include "../../misc/EG_Point.h"
#include "../../misc/EG_Rect.h"
#include "../../misc/EG_Color.h"
//#include "../../hal/EG_HALDisplay.h"  // lv_hal_disp

class EGSoftBlend;

extern EGDrawRect *g_pDrawRect;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	const EGPoint    *pCenter;
	EG_Coord_t        Radius;
	uint16_t          StartAngle;
	uint16_t          EndAngle;
	uint16_t          StartQuarter;
	uint16_t          EndQuarter;
	EG_Coord_t        Width;
	EGDrawRect       *pDrawRec;
	const EGRect     *pDrawRect;
	EGDrawArc        *pDrawArc;
  const EGDrawContext    *pContext;        
} QuadrantDiscriptor_t;

#define SPLIT_RADIUS_LIMIT 10    /*With radius greater than this the arc will drawn in quarters. A quarter is drawn only if there is arc in it*/
#define SPLIT_ANGLE_GAP_LIMIT 60 /*With small gaps in the arc don't bother with splitting because there is nothing to skip.*/

//////////////////////////////////////////////////////////////////////////////////////

class EGSoftContext : public EGDrawContext
{
public:
                EGSoftContext(){};
                ~EGSoftContext();
  virtual void  InitialiseContext(void);

  static void   SoftWaitForFinish(void);
  static void   BufferCopy(void *pDestBuffer, EG_Coord_t DestStride, EGRect *pDestArea, void *pSourceBuffer, EG_Coord_t SourceStride, EGRect *pSourceArea);
  static void   DrawLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
  static void   DrawArc(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
  static void   DrawRect(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawBackground(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawCharacter(const EGDrawLabel *pDrawAlpha, const EGPoint * pos_p, uint32_t letter);
  static void   DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
  static void   DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGDrawRect *pRect, const EGPoint *pPoints, uint16_t PointCount);
  static void   DrawTransform(const EGRect *pRect, const void *pSrceBuffer, EG_Coord_t SourceWidth, EG_Coord_t SourceHeight, 
                  EG_Coord_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t cf, EG_Color_t *pColorBuffer, EG_OPA_t *pOpaBuffer);
  static EGLayerContext* DrawLayerCreate(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags);
  static void   DrawLayerAdjust(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e flags);
  static void   DrawLayerBlend(EGLayerContext *pDrawLayer, EGDrawImage *pImage);
  static void   DrawLayerDestroy(EGLayerContext *pDrawLayer);

private:
// LINE //
  static void   HorizontalLine(EGDrawLine *pLine, const EGPoint *point1, const EGPoint *point2);
  static void   VerticalLine(EGDrawLine *pLine, const EGPoint *point1, const EGPoint *point2);
  static void   SkewLine(EGDrawLine *pLine, const EGPoint *point1, const EGPoint *point2);

// RECT //
  static void   DrawRectBackground(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawBackgroundImage(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawBorder(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawOutline(const EGDrawRect *pDrawRect, const EGRect *pRect);
  #if EG_DRAW_COMPLEX
  static void   DrawShadow(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   ShadowCornerDrawBuffer(const EGRect *pRect, uint16_t *sh_buf, EG_Coord_t s, EG_Coord_t r);
  static void   BlurShadowCorner(EG_Coord_t size, EG_Coord_t sw, uint16_t *sh_ups_buf);
  #endif
  static void   DrawBorderGeneric(const EGDrawRect *pDrawRect, const EGRect *pOuterArea, const EGRect *pInnerArea, EG_Coord_t OutsideRadius,
                                  EG_Coord_t InsideRadius, EG_Color_t Color, EG_OPA_t BackOPA, EG_BlendMode_e BlendMode);
  static void   DrawBorderSimple(const EGDrawRect *pDrawRect, const EGRect *pOuterArea, const EGRect *pInnerArea, EG_Color_t Color, EG_OPA_t BackOPA);

// ARC //

#if EG_DRAW_COMPLEX
  static void   DrawQuadrant0(QuadrantDiscriptor_t *qQuadrant);
  static void   DrawQuadrant1(QuadrantDiscriptor_t *qQuadrant);
  static void   DrawQuadrant2(QuadrantDiscriptor_t *qQuadrant);
  static void   DrawQuadrant3(QuadrantDiscriptor_t *qQuadrant);
  static void   GetRoundedArea(int16_t Angle, EG_Coord_t Radius, uint8_t Thickness, EGRect *pRect);
#endif

// ALPHA //
  static void   DrawNormal(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, EG_FontGlyphProps_t *pGlyph, const uint8_t *pMap);
#if EG_DRAW_COMPLEX && EG_USE_FONT_SUBPX
  static void   DrawSubpixel(EGDrawLabel *pDrawAlpha, const EGPoint *pos,	EG_FontGlyphProps_t *g, const uint8_t *map_p);
#endif 

private:

};

#endif