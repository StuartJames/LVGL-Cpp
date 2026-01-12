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

#pragma once

#include <stdbool.h>
#include "../misc/EG_Point.h"
#include "../misc/EG_Rect.h"
#include "../misc/EG_Color.h"
#include "../misc/EG_Math.h"

//namespace EG_DrawMask
//{
/////////////////////////////////////////////////////////////////////////////////

#define EG_MASK_ID_INVALID  (-1)
#if EG_DRAW_COMPLEX
# define _EG_MASK_MAX_NUM     16
#else
# define _EG_MASK_MAX_NUM     1
#endif

enum {
    EG_DRAW_MASK_RES_TRANSP,
    EG_DRAW_MASK_RES_FULL_COVER,
    EG_DRAW_MASK_RES_CHANGED,
    EG_DRAW_MASK_RES_UNKNOWN
};

/////////////////////////////////////////////////////////////////////////////////

typedef uint8_t DrawMaskRes_t;

typedef struct {
    void *pParam;
    void *pReference;
} EG_DrawMaskList_t;

typedef EG_DrawMaskList_t EG_DrawMaskListArray_t[_EG_MASK_MAX_NUM];

#if EG_DRAW_COMPLEX == 0
 inline  uint8_t DrawMaskGetCount(void)
{
  return 0;
}

 inline bool HasAnyDrawMask(const EGRect * a)
{
  EG_UNUSED(a);
  return false;
}

#endif

#if EG_DRAW_COMPLEX

enum {
    EG_DRAW_MASK_TYPE_LINE,
    EG_DRAW_MASK_TYPE_ANGLE,
    EG_DRAW_MASK_TYPE_RADIUS,
    EG_DRAW_MASK_TYPE_FADE,
    EG_DRAW_MASK_TYPE_MAP,
    EG_DRAW_MASK_TYPE_POLYGON,
};

typedef uint8_t EG_DrawMask_type_t;

enum {
    EG_DRAW_MASK_LINE_SIDE_LEFT = 0,
    EG_DRAW_MASK_LINE_SIDE_RIGHT,
    EG_DRAW_MASK_LINE_SIDE_TOP,
    EG_DRAW_MASK_LINE_SIDE_BOTTOM,
};

/////////////////////////////////////////////////////////////////////////////////

typedef DrawMaskRes_t (*DrawMaskCB)(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, void *pParam);

typedef uint8_t MaskLineSide_t;

typedef struct MaskCommonDiscrpt_t{
  MaskCommonDiscrpt_t(void) : DrawCB(nullptr), Type(EG_DRAW_MASK_LINE_SIDE_LEFT){};
  DrawMaskCB DrawCB;
  EG_DrawMask_type_t Type;
} MaskCommonDiscrpt_t;

typedef struct MaskLineParam_t{
  MaskLineParam_t(void) : SteepXY(0), SteepYX(0), Steep(0), SteepPixel(0), Flat(0), Invert(0){};
  MaskCommonDiscrpt_t dsc;    // The first element must be the common descriptor
  struct {
      EGPoint Point1;        // First point
      EGPoint Point2;        // Second point
      MaskLineSide_t Side : 2;       // Which side to keep?
  } cfg;
  EGPoint Origin;    // A point of the line
  int32_t SteepXY;    // X / (1024*Y) steepness (X is 0..1023 range). What is the change of X in 1024 Y?
  int32_t SteepYX;    // Y / (1024*X) steepness (Y is 0..1023 range). What is the change of Y in 1024 X?
  int32_t Steep;    // Helper which stores yx_steep for flat lines and xy_steep for steep (non flat) lines
  int32_t SteepPixel;    // Steepness in 1 px in 0..255 range. Used only by flat lines.
  uint8_t Flat : 1;    // 1: It's a flat line? (Near to horizontal)
  uint8_t Invert: 1;    // Invert the mask. The default is: Keep the left part.
} MaskLineParam_t;

typedef struct MaskAngleParam_t{
  MaskAngleParam_t(void) : DeltaDeg(0){cfg.StartAngle = 0; cfg.EndAngle = 0;};
  MaskCommonDiscrpt_t dsc;    // The first element must be the common descriptor
  struct {
      EGPoint Vertex;
      EG_Coord_t StartAngle;
      EG_Coord_t EndAngle;
  } cfg;
  MaskLineParam_t StartLine;
  MaskLineParam_t EndLine;
  uint16_t DeltaDeg;
} MaskAngleParam_t;

typedef struct MaskRadiusCircleDiscrpt_t{
  MaskRadiusCircleDiscrpt_t(void): pBuffer(nullptr), pCircleOPA(nullptr), pStartXonY(nullptr), pStartOPAonY(nullptr),
    Life(0), UsedCount(0), Radius(0){};
  uint8_t         *pBuffer;
  EG_OPA_t        *pCircleOPA;         // Opacity of values on the circumference of an 1/4 circle
  uint16_t        *pStartXonY;        // The x coordinate of the circle for each y value
  uint16_t        *pStartOPAonY;      // The index of `cir_opa` for each y value
  int32_t         Life;               // How many times the entry way used
  uint32_t        UsedCount;          // Like a semaphore to count the referencing masks
  EG_Coord_t      Radius;          // The radius of the entry
} MaskRadiusCircleDiscrpt_t;

typedef MaskRadiusCircleDiscrpt_t EG_DrawMaskCircleListArray_t[EG_CIRCLE_CACHE_SIZE];

typedef struct MaskRadiusParam_t{
  MaskRadiusParam_t(void): pCircle(nullptr){ cfg.Radius = 0; cfg.Outer = 0;};
MaskCommonDiscrpt_t dsc;    // The first element must be the common descriptor
struct {
  EGRect        Area;
  EG_Coord_t    Radius;
  uint8_t       Outer: 1;  // Invert the mask. 0: Keep the pixels inside.
} cfg;
MaskRadiusCircleDiscrpt_t *pCircle;
} MaskRadiusParam_t;

typedef struct MaskFadeParam_t{
  MaskFadeParam_t(void){cfg.TopY = 0; cfg.BottomY = 0; cfg.TopOPA = 0; cfg.BottomOPA = 0;}; 
  MaskCommonDiscrpt_t dsc;    // The first element must be the common descriptor
  struct {
      EGRect      Area;
      EG_Coord_t  TopY;
      EG_Coord_t  BottomY;
      EG_OPA_t    TopOPA;
      EG_OPA_t    BottomOPA;
  } cfg;
} MaskFadeParam_t;

typedef struct MaskMapParam_t {
  MaskMapParam_t(void){ cfg.pMap = nullptr;}; 
  MaskCommonDiscrpt_t dsc;   // The first element must be the common descriptor
  struct {
    EGRect          Area;
    const EG_OPA_t  *pMap;
  } cfg;
} MaskMapParam_t;

typedef struct MaskPolygonParam_t{
  MaskPolygonParam_t(void){ cfg.pPoints = nullptr; cfg.PointCount = 0;};
  MaskCommonDiscrpt_t dsc;    // The first element must be the common descriptor
  struct {
    EGPoint       *pPoints;
    uint16_t       PointCount;
  } cfg;
} MaskPolygonParam_t;

/////////////////////////////////////////////////////////////////////////////////

  int16_t         DrawMaskAdd(void *pParam, void *pID);
  DrawMaskRes_t   DrawMaskApply(EG_OPA_t *pMaskBuffer, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length);
  DrawMaskRes_t   DrawMaskApplyIDs(EG_OPA_t *pMaskArray, EG_Coord_t AbsX, EG_Coord_t AbsY, EG_Coord_t Length, const int16_t *pIndexes, int16_t Count);
  void*           DrawMaskRemove(int16_t Index);
  void*           DrawMaskRemoveReferenced(void *pReference);
  void            DrawMaskCleanup(void);
  uint8_t         DrawMaskGetCount(void);
  bool            HasAnyDrawMask(const EGRect *pRect);
  void            DrawMaskSetLineAngle(MaskLineParam_t *pParam, EG_Coord_t PosX, EG_Coord_t PosY, int16_t Angle,  MaskLineSide_t Side);
  void            DrawMaskSetAngle(MaskAngleParam_t *pParam, EG_Coord_t PosX, EG_Coord_t PosY, EG_Coord_t start_angle, EG_Coord_t end_angle);
  void            DrawMaskSetRadius(MaskRadiusParam_t *pParam, const EGRect *pRect, EG_Coord_t Radius, bool Invert);
  void            DrawMaskSetFade(MaskFadeParam_t *pParam, const EGRect *pRect, EG_OPA_t TopOPA, EG_Coord_t TopY,
                                                            EG_OPA_t BottomOPA, EG_Coord_t BottomY);
  void            DrawMaskSetMap(MaskMapParam_t *pParam, const EGRect *pRect, const EG_OPA_t *pMap);
  void            DrawMaskSetPolygon(MaskPolygonParam_t *pParam, const EGPoint *pPoints, uint16_t PointCount);
  void            DrawMaskFreeParam(void *pParam);
  void            DrawMaskSetLinePoints(MaskLineParam_t *pParam, EG_Coord_t PosX1, EG_Coord_t PosY1, EG_Coord_t PosX2,
                                                                            EG_Coord_t PosY2, MaskLineSide_t Side);
#endif 



