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

#include <stdint.h>
#include "EG_ImageBuffer.h"
#include "../misc/EG_FileSystem.h"
#include "../misc/EG_Types.h"
#include "../misc/EG_Rect.h"

///////////////////////////////////////////////////////////////////////////////

typedef enum EG_ImageSource_t : uint8_t{
  EG_IMG_SRC_VARIABLE,  //  Binary/C variable
  EG_IMG_SRC_FILE,      //  File in filesystem
  EG_IMG_SRC_SYMBOL,    //  Symbol (@ref EG_SymbolDef.h)
  EG_IMG_SRC_UNKNOWN,   //  Unknown source
} EG_ImageSource_t;

///////////////////////////////////////////////////////////////////////////////

class EGImageDecoder;
class EGList;

///////////////////////////////////////////////////////////////////////////////

// Describe an image decoding session. Stores data about the decoding
typedef struct _ImageDecoderDescriptor_t {
  EGImageDecoder         *pDecoder;     // The decoder which was able to open the image source
  const void             *pSource;      // The image source. A file path like "S:my_img.png" or pointer to an `lv_img_dsc_t` variable
  EG_Color_t              Color;        // Color to draw the image. USed when the image has alpha channel only
  int32_t                 FrameIndex;   // Frame of the image, using with animated images
  EG_ImageSource_t        SourceType;   // Type of the source: file or variable. Can be set in `open` function if required
  EG_ImageHeader_t        Header;       // Info about the opened image: color format, size, etc. MUST be set in `open` function
  const uint8_t          *pImageData;   //  Pointer to a buffer where the image's data (pixels) are stored in a decoded, plain format.
  uint32_t                OpenDelay;    //  How much time did it take to open the image. [ms]. If not set `lv_img_cache` will measure and set the time to open
  const char             *pErrorMsg;    // A text to display instead of the image when the image can't be opened. Can be set in `open` function or set NULL.
  void                   *pExtParam;
} ImageDecoderDescriptor_t;

///////////////////////////////////////////////////////////////////////////////

class EGImageDecoder 
{
public:
							            EGImageDecoder(void);
	virtual			            ~EGImageDecoder(void);
  virtual EG_Result_t     ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer);
  virtual void            Close(ImageDecoderDescriptor_t *pDescriptor);

  static void             Register(void *pDecoder);
  static EG_Result_t      Open(ImageDecoderDescriptor_t *pDescriptor, const void *pSource, EG_Color_t Color, int32_t FrameIndex);
  static void             Delete(void *pDecoder);
  static EG_Result_t      GetInfo(const void *pSource, EG_ImageHeader_t *pHeader);

#if EG_USE_USER_DATA
  void                   *m_pUserData;
#endif

private:
  virtual EG_Result_t     Info(const void *pSource, EG_ImageHeader_t *pHeader) = 0;
  virtual EG_Result_t     Open(ImageDecoderDescriptor_t *pDescriptor) = 0;

  static EGList           m_DecoderList;

};

///////////////////////////////////////////////////////////////////////////////

class EGDecoderBuiltIn : public EGImageDecoder
{
public:

							              EGDecoderBuiltIn(void);
  EG_Result_t               Info(const void *pSource, EG_ImageHeader_t *pHeader);
  EG_Result_t               Open(ImageDecoderDescriptor_t  *pDescriptor);
  EG_Result_t               ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer);
  void                      Close(ImageDecoderDescriptor_t *pDescriptor);

private:
  EG_Result_t               TrueColor(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y,	EG_Coord_t Length, uint8_t *pBuffer);
  EG_Result_t               Alpha(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer);
  EG_Result_t               Indexed(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y,EG_Coord_t Length, uint8_t *pBuffer);

	EGFileSystem                    m_File;
	EG_Color_t               *m_pPalette;
	EG_OPA_t                 *m_pOPA;

};

