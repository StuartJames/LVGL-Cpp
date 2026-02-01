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

#include "EG_IntrnlConfig.h"

#if EG_USE_BMP

///////////////////////////////////////////////////////////////////////////////

class EGDecoderBMP : public EGImageDecoder
{
public:

							              EGDecoderBMP(void){};
  EG_Result_t               Info(const void *pSource, EG_ImageHeader_t *pHeader);
  EG_Result_t               Open(ImageDecoderDescriptor_t  *pDescriptor);
  EG_Result_t               ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer);
  void                      Close(ImageDecoderDescriptor_t *pDescriptor);

private:
	EGFileSystem                    m_File;
	unsigned int              m_PixelOffset;
	int                       m_PixelWidth;
	int                       m_PixelHeight;
	unsigned int              m_BitsPerPixel;
	int                       m_RowByteSize;
};

extern EGDecoderBMP DecoderBMP;

#endif 

