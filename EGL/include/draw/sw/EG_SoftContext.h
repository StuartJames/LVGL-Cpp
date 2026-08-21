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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

//#include "EG_DrawSoftBlend.h"
#include "../EG_DeviceContext.h"
#include "../../misc/EG_Point.h"
#include "../../misc/EG_Rect.h"
#include "../../misc/EG_Color.h"
//#include "hal/EG_HALDisplay.h"

class EGSoftBlend;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	const EGPoint    *pCenter;
	int32_t           Radius;
	uint16_t          StartAngle;
	uint16_t          EndAngle;
	uint16_t          StartQuarter;
	uint16_t          EndQuarter;
	int32_t           Width;
	const EGRect     *pDrawRect;
	EGDrawRect       *pDrawRec;
	EGDrawArc        *pDrawArc;
  const EGDeviceContext    *pDC;
} QuadrantDescriptor_t;

#define SPLIT_RADIUS_LIMIT 10    // With a radius greater than this the arc will drawn in quarters. A quarter is drawn only if there is arc in it
#define SPLIT_ANGLE_GAP_LIMIT 60 // With small gaps in the arc don't bother with splitting because there is nothing to skip.

//////////////////////////////////////////////////////////////////////////////////////

class EGSoftContext : public EGDeviceContext
{
public:
                EGSoftContext() : EGDeviceContext(){};
                ~EGSoftContext();
  virtual void  InitialiseContext(void);

  static void   SoftWaitForFinish(void);
  static void   BufferCopy(void *pDestBuffer, int32_t DestStride, EGRect *pDestRect, void *pSourceBuffer, int32_t SourceStride, EGRect *pSourceArea);
  static void   DrawLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
  static void   DrawArc(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
  static void   DrawRect(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawBackground(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void   DrawCharacter(const EGDrawLabel *pDrawAlpha, const EGPoint *pPos, uint32_t Char);
  static void   DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
  static void   DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGPoint *pVertices, uint16_t VerticesCount);
  static void   DrawTransform(const EGRect *pRect, const void *pSrceBuffer, int32_t SourceWidth, int32_t SourceHeight,
                  int32_t SourceStride, const EGDrawImage *pImage, EG_ImageColorFormat_t cf, EG_Color_t *pColorBuffer, EG_OPA_t *pOpaBuffer);
  static bool   DrawLayerCreate(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags);
  static void   DrawLayerAdjust(EGLayerContext *pDrawLayer, EGDrawLayerFlags_e flags);
  static void   DrawLayerBlend(EGLayerContext *pDrawLayer, EGDrawImage *pDrawImage);
  static void   DrawLayerDestroy(EGLayerContext *pDrawLayer);

private:
// LINE //
  void          HorizontalLine(EGDrawLine *pLine, const EGPoint *point1, const EGPoint *point2);
  void          VerticalLine(EGDrawLine *pLine, const EGPoint *point1, const EGPoint *point2);
  void          SkewLine(EGDrawLine *pLine, const EGPoint *point1, const EGPoint *point2);

// RECT //
  void          DrawRectBackground(const EGDrawRect *pDrawRect, const EGRect *pRect);
  void          DrawBackgroundImage(const EGDrawRect *pDrawRect, const EGRect *pRect);
  void          DrawBorder(const EGDrawRect *pDrawRect, const EGRect *pRect);
  void          DrawOutline(const EGDrawRect *pDrawRect, const EGRect *pRect);
  #if EG_DRAW_COMPLEX
  void          DrawShadow(const EGDrawRect *pDrawRect, const EGRect *pRect);
  void          ShadowCornerDrawBuffer(const EGRect *pRect, uint16_t *sh_buf, int32_t s, int32_t r);
  void          BlurShadowCorner(int32_t size, int32_t sw, uint16_t *sh_ups_buf);
  #endif
  void          DrawBorderGeneric(const EGDrawRect *pDrawRect, const EGRect *pOuterArea, const EGRect *pInnerArea, int32_t OutsideRadius,
                                  int32_t InsideRadius, EG_Color_t Color, EG_OPA_t BackOPA, EG_BlendMode_e BlendMode);
  void          DrawBorderSimple(const EGDrawRect *pDrawRect, const EGRect *pOuterArea, const EGRect *pInnerArea, EG_Color_t Color, EG_OPA_t BackOPA);

// ARC //
#if EG_DRAW_COMPLEX
  void          DrawQuadrant0(QuadrantDescriptor_t *qQuadrant);
  void          DrawQuadrant1(QuadrantDescriptor_t *qQuadrant);
  void          DrawQuadrant2(QuadrantDescriptor_t *qQuadrant);
  void          DrawQuadrant3(QuadrantDescriptor_t *qQuadrant);
  void          GetRoundedArea(int16_t Angle, int32_t Radius, uint8_t Thickness, EGRect *pRect);
#endif

// ALPHA //
  void          DrawNormal(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, EG_FontGlyphProps_t *pGlyph, const uint8_t *pMap);
#if EG_DRAW_COMPLEX && EG_USE_FONT_SUBPX
  void          DrawSubpixel(EGDrawLabel *pDrawAlpha, const EGPoint *pos,	EG_FontGlyphProps_t *g, const uint8_t *map_p);
#endif

};
