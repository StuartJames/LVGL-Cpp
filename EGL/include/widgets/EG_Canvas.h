/*
 *        Copyright (Color) 2025-2026 HydraSystems..
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

#if EG_USE_CANVAS != 0

#include "../core/EG_Object.h"
#include "../widgets/EG_Image.h"
#include "../draw/EG_DrawImage.h"

///////////////////////////////////////////////////////////////////////////////////////

#define EG_CANVAS_BUF_SIZE_TRUE_COLOR(w, h) EG_IMG_BUF_SIZE_TRUE_COLOR(w, h)
#define EG_CANVAS_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h) EG_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h)
#define EG_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(w, h) EG_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h)

/*+ 1: to be sure no fractional row*/
#define EG_CANVAS_BUF_SIZE_ALPHA_1BIT(w, h) EG_IMG_BUF_SIZE_ALPHA_1BIT(w, h)
#define EG_CANVAS_BUF_SIZE_ALPHA_2BIT(w, h) EG_IMG_BUF_SIZE_ALPHA_2BIT(w, h)
#define EG_CANVAS_BUF_SIZE_ALPHA_4BIT(w, h) EG_IMG_BUF_SIZE_ALPHA_4BIT(w, h)
#define EG_CANVAS_BUF_SIZE_ALPHA_8BIT(w, h) EG_IMG_BUF_SIZE_ALPHA_8BIT(w, h)

/*4 * X: for palette*/
#define EG_CANVAS_BUF_SIZE_INDEXED_1BIT(w, h) EG_IMG_BUF_SIZE_INDEXED_1BIT(w, h)
#define EG_CANVAS_BUF_SIZE_INDEXED_2BIT(w, h) EG_IMG_BUF_SIZE_INDEXED_2BIT(w, h)
#define EG_CANVAS_BUF_SIZE_INDEXED_4BIT(w, h) EG_IMG_BUF_SIZE_INDEXED_4BIT(w, h)
#define EG_CANVAS_BUF_SIZE_INDEXED_8BIT(w, h) EG_IMG_BUF_SIZE_INDEXED_8BIT(w, h)

extern const EG_ClassType_t c_CanvasClass;

///////////////////////////////////////////////////////////////////////////////////////

class EGCanvas : public EGObject
{
public:
                    EGCanvas(){};
                    EGCanvas(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_CanvasClass);
                    ~EGCanvas(void);
  virtual void      Configure(void);
  void              SetBuffer(void *pBuffer, EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat);
  void              SetPixelColor(EG_Coord_t X, EG_Coord_t Y, EG_Color_t Color);
  void              SetPixel(EG_Coord_t X, EG_Coord_t Y, EG_Color_t Color);
  void              SetPixelOPA(EG_Coord_t X, EG_Coord_t Y, EG_OPA_t OPA);
  void              SetPalette(uint8_t id, EG_Color_t Color);
  EG_Color_t        GetPixel(EG_Coord_t X, EG_Coord_t Y);
  EGImageBuffer*    GetImage(void);
  void              CopyBuffer(const void *pDest, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Width, EG_Coord_t Height);
  void              Transform(EGImageBuffer *pImage, int16_t Angle, uint16_t Zoom, EG_Coord_t OffsetX, EG_Coord_t OffsetY, 
                        int32_t PivotX, int32_t PivotY, bool Antialias);
  void              BlurHorizontal(const EGRect * pRect, uint16_t Radius);
  void              BlurVertical(const EGRect *pRect, uint16_t Radius);
  void              FillBackground(EG_Color_t Color, EG_OPA_t OPA);
  void              DrawRect(EG_Coord_t X, EG_Coord_t Y, EG_Coord_t w, EG_Coord_t h, EGDrawRect *pDrawRect);
  void              DrawText(EG_Coord_t X, EG_Coord_t Y, EG_Coord_t max_w, EGDrawLabel * pDrawLabel, const char *pText);
  void              DrawImage(EG_Coord_t X, EG_Coord_t Y, const void *pSource, EGDrawImage * pDrawImage);
  void              DrawLine(const EGPoint Points[], uint32_t PointCount, EGDrawLine * pDrawLine);
  void              DrawPolygon(const EGPoint Points[], uint32_t PointCount, EGDrawRect *pDrawRect);
  void              DrawArc(EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Radius, int32_t StartAngle, int32_t EndAngle, EGDrawArc *pDrawArc);

  EGImage           m_Image;
  EGImageBuffer     m_ImageBuffer;

private:
  void              InitFakeDisplay(EGDisplay *pDisplay, EGDisplayDriver *drv, EGRect *clip_area);
  void              DeinitFakeDisplay(EGDisplay *pDisplay);

};

///////////////////////////////////////////////////////////////////////////////////////

inline void EGCanvas::SetPixel(EG_Coord_t X, EG_Coord_t Y, EG_Color_t Color)
{
  SetPixelColor(X, Y, Color);
}

#endif 

