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

#include "../EG_IntrnlConfig.h"

#if EG_USE_IMG != 0

// Testing of dependencies
#if EG_USE_LABEL == 0
#error "lv_img: lv_label is required. Enable it in EG_Config.h (EG_USE_LABEL 1)"
#endif

#include "../core/EG_Object.h"
#include "../misc/EG_FileSystem.h"
#include "../draw/EG_DrawContext.h"

///////////////////////////////////////////////////////////////////////////////////////

// * Use this macro to declare an image in a C file
#define EG_IMG_DECLARE(var_name) extern const EGImageBuffer var_name;

///////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_Imageclass;

typedef enum {
  /** Zoom doesn't affect the coordinates of the object,
   *  however if zoomed in the image is drawn out of the its coordinates.
   *  The layout's won't change on zoom */
  EG_IMG_SIZE_MODE_VIRTUAL = 0,

  /** If the object size is set to SIZE_CONTENT, then object size equals zoomed image size.
   *  It causes layout recalculation.
   *  If the object size is set explicitly, the image will be cropped when zoomed in.*/
  EG_IMG_SIZE_MODE_REAL,
} EG_ImageSizeMode_e;

///////////////////////////////////////////////////////////////////////////////////////

class EGImage : public EGObject
{
public:
                      EGImage(void);
                      EGImage(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_Imageclass);
                      ~EGImage(void);
  virtual void        Configure(void);
  void                SetSource(const void *pSource);
  void                SetOffsetX(EG_Coord_t X);
  void                SetOffsetY(EG_Coord_t Y);
  void                SetAngle(int16_t Angle);
  void                SetPivot(EG_Coord_t X, EG_Coord_t Y);
  void                SetPivot(EGPoint Pivot);
  void                SetZoom(uint16_t Zoom);
  void                SetAntialias(bool Antialias);
  void                SetSizeMode(EG_ImageSizeMode_e Mode);
  const void*         GetSource(void);
  EG_Coord_t          GetOffsetX(void);
  EG_Coord_t          GetOffsetY(void);
  uint16_t            GetAngle(void);
  void                GetPivot(EGPoint *pPivot);
  uint16_t            GetZoom(void);
  bool                GetAntialias(void);
  EG_ImageSizeMode_e  GetSizeMode(void);
  void                Event(EGEvent *pEvent);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  const void*         m_pSource;        // Image source: Pointer to an array or a file or a symbol
  EGPoint             m_Offset;
  EGPoint             m_Pivot;          // rotation center of the image
  EG_Coord_t          m_Width;          // Width of the image (Handled by the library)
  EG_Coord_t          m_Height;         // Height of the image (Handled by the library)
  uint16_t            m_Angle;          // rotation angle of the image
  uint16_t            m_Zoom;           // 256 means no zoom, 512 double size, 128 half size
  uint8_t             m_SourceType : 2; // See: EG_ImageSource_t
  uint8_t             m_ColorFormat : 5;// Color format from `lv_img_color_format_t`
  uint8_t             m_Antialias : 1;  // Apply anti-aliasing in transformations (rotate, zoom)
  uint8_t             m_SizeMode: 2;    // Image size mode when image size and object size is different.

private:
  EGPoint             GetTransformedSize(void);
  void                Draw(EGEvent *pEvent);

};

#endif 