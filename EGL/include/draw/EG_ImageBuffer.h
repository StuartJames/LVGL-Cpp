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
#include "../misc/EG_Color.h"
#include "../misc/EG_Point.h"
#include "../misc/EG_Rect.h"

//////////////////////////////////////////////////////////////////////////////////////

// If image pixels contains alpha we need to know how much byte is a pixel
#if EG_COLOR_DEPTH == 1 || EG_COLOR_DEPTH == 8
#define EG_IMG_PX_SIZE_ALPHA_BYTE 2
#elif EG_COLOR_DEPTH == 16
#define EG_IMG_PX_SIZE_ALPHA_BYTE 3
#elif EG_COLOR_DEPTH == 32
#define EG_IMG_PX_SIZE_ALPHA_BYTE 4
#endif

#define EG_IMG_BUF_SIZE_TRUE_COLOR(w, h) ((EG_COLOR_SIZE / 8) * w * h)
#define EG_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h) ((EG_COLOR_SIZE / 8) * w * h)
#define EG_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h) (EG_IMG_PX_SIZE_ALPHA_BYTE * w * h)

// + 1: to be sure no fractional row
#define EG_IMG_BUF_SIZE_ALPHA_1BIT(w, h) ((((w / 8) + 1) * h))
#define EG_IMG_BUF_SIZE_ALPHA_2BIT(w, h) ((((w / 4) + 1) * h))
#define EG_IMG_BUF_SIZE_ALPHA_4BIT(w, h) ((((w / 2) + 1) * h))
#define EG_IMG_BUF_SIZE_ALPHA_8BIT(w, h) ((w * h))

// 4 * X: for palette
#define EG_IMG_BUF_SIZE_INDEXED_1BIT(w, h) (EG_IMG_BUF_SIZE_ALPHA_1BIT(w, h) + 4 * 2)
#define EG_IMG_BUF_SIZE_INDEXED_2BIT(w, h) (EG_IMG_BUF_SIZE_ALPHA_2BIT(w, h) + 4 * 4)
#define EG_IMG_BUF_SIZE_INDEXED_4BIT(w, h) (EG_IMG_BUF_SIZE_ALPHA_4BIT(w, h) + 4 * 16)
#define EG_IMG_BUF_SIZE_INDEXED_8BIT(w, h) (EG_IMG_BUF_SIZE_ALPHA_8BIT(w, h) + 4 * 256)

#define _EG_ZOOM_INV_UPSCALE 5

// Image color format
typedef enum EG_ImageColorFormat_t : uint8_t{
    EG_IMG_CF_UNKNOWN = 0,

    EG_IMG_CF_RAW,              // Contains the file as it is. Needs custom decoder function
    EG_IMG_CF_RAW_ALPHA,        // Contains the file as it is. The image has alpha. Needs custom decoder function
    EG_IMG_CF_RAW_CHROMA_KEYED, // Contains the file as it is. The image is chroma keyed. Needs custom decoder function

    EG_IMG_CF_TRUE_COLOR,               // Color format and depth should match with EG_COLOR settings
    EG_IMG_CF_TRUE_COLOR_ALPHA,         // Same as `EG_IMG_CF_TRUE_COLOR` but every pixel has an alpha byte
    EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED,  // Same as `EG_IMG_CF_TRUE_COLOR` but EG_COLOR_TRANSP pixels will be transparent

    EG_IMG_CF_INDEXED_1BIT,             // Can have 2 different colors in a palette (can't be chroma keyed)
    EG_IMG_CF_INDEXED_2BIT,             // Can have 4 different colors in a palette (can't be chroma keyed)
    EG_IMG_CF_INDEXED_4BIT,             // Can have 16 different colors in a palette (can't be chroma keyed)
    EG_IMG_CF_INDEXED_8BIT,             // Can have 256 different colors in a palette (can't be chroma keyed)

    EG_IMG_CF_ALPHA_1BIT,               // Can have one color and it can be drawn or not
    EG_IMG_CF_ALPHA_2BIT,               // Can have one color but 4 different alpha value
    EG_IMG_CF_ALPHA_4BIT,               // Can have one color but 16 different alpha value
    EG_IMG_CF_ALPHA_8BIT,               // Can have one color but 256 different alpha value

    EG_IMG_CF_RGB888,
    EG_IMG_CF_RGBA8888,
    EG_IMG_CF_RGBX8888,
    EG_IMG_CF_RGB565,
    EG_IMG_CF_RGBA5658,
    EG_IMG_CF_RGB565A8,

    EG_IMG_CF_RESERVED_15,              // Reserved for further use.
    EG_IMG_CF_RESERVED_16,              // Reserved for further use.
    EG_IMG_CF_RESERVED_17,              // Reserved for further use.
    EG_IMG_CF_RESERVED_18,              // Reserved for further use.
    EG_IMG_CF_RESERVED_19,              // Reserved for further use.
    EG_IMG_CF_RESERVED_20,              // Reserved for further use.
    EG_IMG_CF_RESERVED_21,              // Reserved for further use.
    EG_IMG_CF_RESERVED_22,              // Reserved for further use.
    EG_IMG_CF_RESERVED_23,              // Reserved for further use.

    EG_IMG_CF_USER_ENCODED_0,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_1,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_2,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_3,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_4,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_5,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_6,           // User holder encoding format.
    EG_IMG_CF_USER_ENCODED_7,           // User holder encoding format.
} EG_ImageColorFormat_t;

#if EG_BIG_ENDIAN_SYSTEM
typedef struct {

    uint32_t Height : 11; // Height of the image map
    uint32_t Width : 11; // Width of the image map
    uint32_t Reserved : 2; // Reserved to be used later
    uint32_t AlwaysZero : 3; // It's the upper bits of the first byte. Always zero to look like a non-printable character
    uint32_t ColorFormat : 5;          // Color format: See `lv_img_color_format_t`
} EG_ImageHeader_t;
#else
typedef struct {

    uint32_t ColorFormat : 5;          // Color format: See `lv_img_color_format_t`
    uint32_t AlwaysZero : 3; // It the upper bits of the first byte. Always zero to look like a non-printable character

    uint32_t Reserved : 2; // Reserved to be used later

    uint32_t Width : 11; // Width of the image map
    uint32_t Height : 11; // Height of the image map
} EG_ImageHeader_t;
#endif

//////////////////////////////////////////////////////////////////////////////////////

class EGImageBuffer
{
public:
                  EGImageBuffer(void);
                  EGImageBuffer(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat);
                  EGImageBuffer(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat, const uint8_t *pData, uint32_t Size = 0);
                  ~EGImageBuffer(void);
  bool            Allocate(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat);
  EG_Color_t      GetPixelColor(EG_Coord_t X, EG_Coord_t Y, EG_Color_t Color);
  EG_OPA_t        GetPixelAlpha(EG_Coord_t X, EG_Coord_t Y);
  void            SetPixelColor(EG_Coord_t X, EG_Coord_t Y, EG_Color_t Color);
  void            SetPixelAlpha(EG_Coord_t X, EG_Coord_t Y, EG_OPA_t OPA);
  void            SetPalette(uint8_t ID, EG_Color_t Color);
  uint32_t        CalculateBufferSize(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat);

  static void     GetTransformedRect(EGRect * res, EG_Coord_t Width, EG_Coord_t Height, int16_t Angle, uint16_t Zoom,
                                        const EGPoint *pPivot);

  EG_ImageHeader_t  m_Header; // A header describing the basics of the image
  uint32_t          m_DataSize;     // Size of the image in bytes
  const uint8_t     *m_pData;   // Pointer to the data of the image


};