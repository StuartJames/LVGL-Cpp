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
#if EG_USE_PNG

#include "extra/libs/EG_PNG.h"
#include "extra/libs/lodepng.h"
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////

EGDecoderPNG DecoderPNG;

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderPNG::Info(const void *pSource, EG_ImageHeader_t *pHeader)
{
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);
	if(SourceType == EG_IMG_SRC_FILE) {	// If it's a PNG file...
		const char *pFileName = (char*)pSource;
		if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "png") == 0) { // Check the extension
			// Read the width and height from the file. They have a constant location: [16..23]: width [24..27]: height
			uint32_t Size[2];
			if(m_File.Open(pFileName, EG_FS_MODE_RD) != EG_FS_RES_OK) return EG_RES_INVALID;
			m_File.Seek(16, EG_FS_SEEK_SET);
			uint32_t ReadCount;
			m_File.Read(&Size, 8, &ReadCount);
			m_File.Close();
			if(ReadCount != 8) return EG_RES_INVALID;
			// Save the m_pData in the pHeader
			pHeader->AlwaysZero = 0;
			pHeader->ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;
			// The width and height are stored in Big endian format so convert them to little endian
			pHeader->Width = (EG_Coord_t)((Size[0] & 0xff000000) >> 24) + ((Size[0] & 0x00ff0000) >> 8);
			pHeader->Height = (EG_Coord_t)((Size[1] & 0xff000000) >> 24) + ((Size[1] & 0x00ff0000) >> 8);
			return EG_RES_OK;
		}
	}
	else if(SourceType == EG_IMG_SRC_VARIABLE) {	// If it's a PNG file in an array...
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pSource;
		const uint32_t m_DataSize = pImageBuffer->m_DataSize;
		const uint32_t *pSize = ((uint32_t *)pImageBuffer->m_pData) + 4;
		const uint8_t Magic[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
		if(m_DataSize < sizeof(Magic)) return EG_RES_INVALID;
		if(memcmp(Magic, pImageBuffer->m_pData, sizeof(Magic))) return EG_RES_INVALID;
		pHeader->AlwaysZero = 0;
		if(pImageBuffer->m_Header.ColorFormat) {
			pHeader->ColorFormat = pImageBuffer->m_Header.ColorFormat; // Save the color format
		}
		else {
			pHeader->ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;
		}
		if(pImageBuffer->m_Header.Width) {
			pHeader->Width = pImageBuffer->m_Header.Width; // Save the image width
		}
		else {
			pHeader->Width = (EG_Coord_t)((pSize[0] & 0xff000000) >> 24) + ((pSize[0] & 0x00ff0000) >> 8);
		}
		if(pImageBuffer->m_Header.Height) {
			pHeader->Height = pImageBuffer->m_Header.Height; // Save the color height
		}
		else {
			pHeader->Height = (EG_Coord_t)((pSize[1] & 0xff000000) >> 24) + ((pSize[1] & 0x00ff0000) >> 8);
		}
		return EG_RES_OK;
	}
	return EG_RES_INVALID; // If didn't succeeded earlier then it's an error
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderPNG::Open(ImageDecoderDescriptor_t  *pDescriptor)
{
	uint32_t error; // For the return values of PNG decoder functions
	uint8_t *pImageData = NULL;
	if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
		const char *pFileName = (char*)pDescriptor->pSource;
		if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "png") == 0) { // Check the extension
			// Load the PNG file into buffer. It's still compressed (not decoded)
			unsigned char *pData; // Pointer to the loaded m_pData. Same as the original file just loaded into the RAM
			size_t DataSize;    // Size of `pData` in bytes
			error = lodepng_load_file(&pData, &DataSize, pFileName); // Load the file
			if(error) {
				EG_LOG_WARN("error %" EG_PRIu32 ": %s\n", error, lodepng_error_text(error));
				return EG_RES_INVALID;
			}
			// Decode the PNG image
			unsigned png_width;  // Will be the width of the decoded image
			unsigned png_height; // Will be the width of the decoded image
			// Decode the loaded image in ARGB8888 
			error = lodepng_decode32(&pImageData, &png_width, &png_height, pData, DataSize);
			EG_FreeMem(pData); // Free the loaded file
			if(error) {
				if(pImageData != NULL) {
					EG_FreeMem(pImageData);
				}
				EG_LOG_WARN("error %" EG_PRIu32 ": %s\n", error, lodepng_error_text(error));
				return EG_RES_INVALID;
			}
			ConvertColorDepth(pImageData, png_width * png_height);			// Convert the image to the system's color depth
			pDescriptor->pImageData = pImageData;
			return EG_RES_OK; // The image is fully decoded. Return with its pointer
		}
	}
	// If it's a PNG file in a  C array...
	else if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pDescriptor->pSource;
		unsigned png_width;  // No used, just required by he decoder
		unsigned png_height; // No used, just required by he decoder
		error = lodepng_decode32(&pImageData, &png_width, &png_height, pImageBuffer->m_pData, pImageBuffer->m_DataSize);	// Decode the image in ARGB8888 
		if(error) {
			if(pImageData != NULL) {
				EG_FreeMem(pImageData);
			}
			return EG_RES_INVALID;
		}
		ConvertColorDepth(pImageData, png_width * png_height);		// Convert the image to the system's color depth
		pDescriptor->pImageData = pImageData;
		return EG_RES_OK; // Return with its pointer
	}
	return EG_RES_INVALID; // If not returned earlier then it failed
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderPNG::Close(ImageDecoderDescriptor_t *pDescriptor)
{
	if(pDescriptor->pImageData) {
		EG_FreeMem((uint8_t *)pDescriptor->pImageData);
		pDescriptor->pImageData = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderPNG::ConvertColorDepth(uint8_t *pImage, uint32_t PixelCount)
{
#if EG_COLOR_DEPTH == 32
	EG_Color32_t *img_argb = (EG_Color32_t *)pImage;
	EG_Color_t c;
	EG_Color_t *img_c = (EG_Color_t *)pImage;
	uint32_t i;
	for(i = 0; i < PixelCount; i++) {
		c = EG_MixColor(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
		img_c[i].ch.red = c.ch.blue;
		img_c[i].ch.blue = c.ch.red;
	}
#elif EG_COLOR_DEPTH == 16
	EG_Color32_t *img_argb = (EG_Color32_t *)pImage;
	EG_Color_t c;
	uint32_t i;
	for(i = 0; i < PixelCount; i++) {
		c = EG_MixColor(img_argb[i].ch.blue, img_argb[i].ch.green, img_argb[i].ch.red);
		pImage[i * 3 + 2] = img_argb[i].ch.alpha;
		pImage[i * 3 + 1] = c.full >> 8;
		pImage[i * 3 + 0] = c.full & 0xFF;
	}
#elif EG_COLOR_DEPTH == 8
	EG_Color32_t *img_argb = (EG_Color32_t *)pImage;
	EG_Color_t c;
	uint32_t i;
	for(i = 0; i < PixelCount; i++) {
		c = EG_MixColor(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
		pImage[i * 2 + 1] = img_argb[i].ch.alpha;
		pImage[i * 2 + 0] = c.full;
	}
#elif EG_COLOR_DEPTH == 1
	EG_Color32_t *img_argb = (EG_Color32_t *)pImage;
	uint8_t b;
	uint32_t i;
	for(i = 0; i < PixelCount; i++) {
		b = img_argb[i].ch.red | img_argb[i].ch.green | img_argb[i].ch.blue;
		pImage[i * 2 + 1] = img_argb[i].ch.alpha;
		pImage[i * 2 + 0] = b > 128 ? 1 : 0;
	}
#endif
}

#endif 
