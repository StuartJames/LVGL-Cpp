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

#include "EGL.h"

#if EG_USE_BMP

#include <string.h>

///////////////////////////////////////////////////////////////////////////////

EGDecoderBMP DecoderBMP;

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBMP::Info(const void *pSource, EG_ImageHeader_t *pHeader)
{
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);
	if(SourceType == EG_IMG_SRC_FILE) {	// If it's a BMP file...
		const char *pFileName = (char*)pSource;
		if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "bmp") == 0) { // Check the extension
			EG_FSResult_e res = m_File.Open(pFileName, EG_FS_MODE_RD);
			if(res != EG_FS_RES_OK) return EG_RES_INVALID;
			uint8_t Header[54];
			m_File.Read(Header, 54, nullptr);
			uint32_t Width;
			uint32_t Height;
			memcpy(&Width, Header + 18, 4);
			memcpy(&Height, Header + 22, 4);
			pHeader->Width = Width;
			pHeader->Height = Height;
			pHeader->AlwaysZero = 0;
			m_File.Close();
#if EG_COLOR_DEPTH == 32
			uint16_t BitsPerPixel;
			memcpy(&BitsPerPixel, Header + 28, 2);
			pHeader->ColorFormat = BitsPerPixel == 32 ? EG_COLOR_FORMAT_NATIVE_ALPHA : EG_COLOR_FORMAT_NATIVE;
#else
			pHeader->ColorFormat = EG_COLOR_FORMAT_NATIVE;
#endif
			return EG_RES_OK;
		}
	}
	//  BMP file as data not supported for simplicity. Convert them to compatible C arrays directly. 
	else if(SourceType == EG_IMG_SRC_VARIABLE) {
		return EG_RES_INVALID;
	}
	return EG_RES_INVALID; // If didn't succeeded earlier then it's an error
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBMP::Open(ImageDecoderDescriptor_t  *pDescriptor)
{
	if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
		const char *pFileName = (char*)pDescriptor->pSource;
		if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "bmp") != 0) {
			return EG_RES_INVALID; // Check the extension
		}
		if(m_File.Open(pFileName, EG_FS_MODE_RD) != EG_FS_RES_OK) return EG_RES_INVALID;
		uint8_t Header[54];
		m_File.Read(Header, 54, NULL);
		if(0x42 != Header[0] || 0x4d != Header[1]) {
			m_File.Close();
			return EG_RES_INVALID;
		}
		memcpy(&m_PixelOffset, Header + 10, 4);
		memcpy(&m_PixelWidth, Header + 18, 4);
		memcpy(&m_PixelHeight, Header + 22, 4);
		memcpy(&m_BitsPerPixel, Header + 28, 2);
		m_RowByteSize = ((m_BitsPerPixel * m_PixelWidth + 31) / 32) * 4;
		bool color_depth_error = false;
		if(EG_COLOR_DEPTH == 32 && (m_BitsPerPixel != 32 && m_BitsPerPixel != 24)) {
			EG_LOG_WARN("EG_COLOR_DEPTH == 32 but BitsPerPixel is %d (should be 32 or 24)", m_BitsPerPixel);
			color_depth_error = true;
		}
		else if(EG_COLOR_DEPTH == 16 && m_BitsPerPixel != 16) {
			EG_LOG_WARN("EG_COLOR_DEPTH == 16 but BitsPerPixel is %d (should be 16)", m_BitsPerPixel);
			color_depth_error = true;
		}
		else if(EG_COLOR_DEPTH == 8 && m_BitsPerPixel != 8) {
			EG_LOG_WARN("EG_COLOR_DEPTH == 8 but BitsPerPixel is %d (should be 8)", m_BitsPerPixel);
			color_depth_error = true;
		}
		if(color_depth_error) {
			pDescriptor->pErrorMsg = "Color depth mismatch";
			m_File.Close();
			return EG_RES_INVALID;
		}
		pDescriptor->pExtParam = this;
		pDescriptor->pImageData = NULL;
		return EG_RES_OK;
	}
	//  BMP file as data not supported for simplicity. Convert them to LVGL compatible C arrays directly. 
	else if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		return EG_RES_INVALID;
	}
	return EG_RES_INVALID; // If not returned earlier then it failed
}

///////////////////////////////////////////////////////////////////////////////


EG_Result_t EGDecoderBMP::ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer)
{
	Y = (m_PixelHeight - 1) - Y; // BMP images are stored upside down
	uint32_t Pos = m_PixelOffset + m_RowByteSize * Y;
	Pos += X * (m_BitsPerPixel / 8);
	m_File.Seek(Pos, EG_FS_SEEK_SET);
	m_File.Read(pBuffer, Length * (m_BitsPerPixel / 8), NULL);

#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP == 1
	for(unsigned int i = 0; i < Length * (m_BitsPerPixel / 8); i += 2) {
		pBuffer[i] = pBuffer[i] ^ pBuffer[i + 1];
		pBuffer[i + 1] = pBuffer[i] ^ pBuffer[i + 1];
		pBuffer[i] = pBuffer[i] ^ pBuffer[i + 1];
	}
#elif EG_COLOR_DEPTH == 32
	if(m_BitsPerPixel == 32) {
		for(EG_Coord_t i = 0; i < Length; i++) {
			uint8_t b0 = pBuffer[i * 4];
			uint8_t b1 = pBuffer[i * 4 + 1];
			uint8_t b2 = pBuffer[i * 4 + 2];
			uint8_t b3 = pBuffer[i * 4 + 3];
			EG_Color32_t *pColor = (EG_Color32_t *)&pBuffer[i * 4];
			pColor->ch.red = b2;
			pColor->ch.green = b1;
			pColor->ch.blue = b0;
			pColor->ch.alpha = b3;
		}
	}
	if(m_BitsPerPixel == 24) {
		for(EG_Coord_t i = Length - 1; i >= 0; i--) {
			uint8_t *t = &pBuffer[i * 3];
			EG_Color32_t *pColor = (EG_Color32_t *)&pBuffer[i * 4];
			pColor->ch.red = t[2];
			pColor->ch.green = t[1];
			pColor->ch.blue = t[0];
			pColor->ch.alpha = 0xff;
		}
	}
#endif
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderBMP::Close(ImageDecoderDescriptor_t *pDescriptor)
{
	m_File.Close();
	pDescriptor->pExtParam = nullptr; // unhook, decoder it's deleted elsewhere
}

#endif 
