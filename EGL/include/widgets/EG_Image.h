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

/**
 * Image size mode, when image size and object size is different
 */
typedef enum {
    EG_IMAGE_ALIGN_DEFAULT = 0,
    EG_IMAGE_ALIGN_TOP_LEFT,
    EG_IMAGE_ALIGN_TOP_MID,
    EG_IMAGE_ALIGN_TOP_RIGHT,
    EG_IMAGE_ALIGN_BOTTOM_LEFT,
    EG_IMAGE_ALIGN_BOTTOM_MID,
    EG_IMAGE_ALIGN_BOTTOM_RIGHT,
    EG_IMAGE_ALIGN_LEFT_MID,
    EG_IMAGE_ALIGN_RIGHT_MID,
    EG_IMAGE_ALIGN_CENTER,
    EG_IMAGE_ALIGN_AUTO_TRANSFORM,
    EG_IMAGE_ALIGN_STRETCH, // Set X and Y scale to fill the Widget's area. 
    EG_IMAGE_ALIGN_TILE,    // Tile image to fill Widget's area. Offset is applied to shift the tiling. 
    EG_IMAGE_ALIGN_CONTAIN, // The image keeps its aspect ratio, but is resized to the maximum size that fits within the Widget's area. 
    EG_IMAGE_ALIGN_COVER,   // The image keeps its aspect ratio and fills the Widget's area. 
} EG_ImageAlign_e;

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
  void                SetRotation(int16_t Angle);
  void                SetPivot(EG_Coord_t X, EG_Coord_t Y);
  void                SetPivot(EGPoint Pivot);
  void                SetZoom(uint16_t Zoom);
  void                SetScale(uint32_t Zoom);
  void                SetScaleX(uint32_t Zoom);
  void                SetScaleY(uint32_t Zoom);
  void                SetAntialias(bool Antialias);
  void                SetInnerAlign(EG_ImageAlign_e Align);
  void                SetSizeMode(EG_ImageSizeMode_e Mode);
  const void*         GetSource(void){ return m_pSource; };
  EG_Coord_t          GetOffsetX(void){ return m_Offset.m_X; };
  EG_Coord_t          GetOffsetY(void){ return m_Offset.m_Y; };
  uint16_t            GetRotation(void){ 	return m_Rotation; };
  EGPoint             GetPivot(void){ return m_Pivot; };
  uint16_t            GetZoom(void){ return m_Zoom; };
  bool                GetAntialias(void){ return m_Antialias ? true : false; };
  EG_ImageSizeMode_e  GetSizeMode(void){ return (EG_ImageSizeMode_e)m_SizeMode; };
  EG_ImageAlign_e     GetInnerAlign(void){ return m_Align; };
  void                Event(EGEvent *pEvent);

  static void         EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  const void*         m_pSource;        // Image source: Pointer to an array or a file or a symbol
  EGPoint             m_Offset;
  EGPoint             m_Pivot;          // rotation center of the image
  EG_Coord_t          m_Width;          // Width of the image (Handled by the library)
  EG_Coord_t          m_Height;         // Height of the image (Handled by the library)
  uint16_t            m_Rotation;          // rotation angle of the image
  uint16_t            m_Zoom;           // 256 means no zoom, 512 double size, 128 half size
  uint8_t             m_SourceType : 2; // See: EG_ImageSource_t
  uint8_t             m_ColorFormat : 5;// Color format from `lv_img_color_format_t`
  uint8_t             m_Antialias : 1;  // Apply anti-aliasing in transformations (rotate, zoom)
  uint8_t             m_SizeMode: 2;    // Image size mode when image size and object size is different.
  EG_ImageAlign_e     m_Align;
  EG_Coord_t          m_ScaleX;
  EG_Coord_t          m_ScaleY;

private:
  void                ScaleUpdate(int32_t ScaleX, int32_t ScaleY);
  void                UpdateAlign(void);
  EGPoint             GetTransformedSize(void);
  void                Draw(EGEvent *pEvent);

};

#endif 