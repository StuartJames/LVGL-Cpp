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

#include "draw/EG_ImageDecoder.h"
#include "misc/EG_Assert.h"
#include "draw/EG_DrawImage.h"
#include "misc/EG_List.h"
#include "misc/EG_Misc.h"

///////////////////////////////////////////////////////////////////////////////

#define CF_BUILT_IN_FIRST EG_COLOR_FORMAT_NATIVE
#define CF_BUILT_IN_LAST EG_COLOR_FORMAT_RGB565A8

///////////////////////////////////////////////////////////////////////////////////////

EGDecoderBuiltIn DecoderBuiltIn;
EGList EGImageDecoder::m_DecoderList;

///////////////////////////////////////////////////////////////////////////////
// EGImageDecoder //
///////////////////////////////////////////////////////////////////////////////

EGImageDecoder::EGImageDecoder(void)
{
}

///////////////////////////////////////////////////////////////////////////////

EGImageDecoder::~EGImageDecoder(void)
{
}

///////////////////////////////////////////////////////////////////////////////

void EGImageDecoder::Register(void *pDecoder)
{
	if(pDecoder != nullptr) m_DecoderList.AddHead(pDecoder);
}

///////////////////////////////////////////////////////////////////////////////

void EGImageDecoder::Delete(void *pDecoder)
{
  POSITION Pos = m_DecoderList.Find(pDecoder);
  if(Pos != nullptr) delete (EGImageDecoder*)m_DecoderList.RemoveAt(Pos);
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGImageDecoder::GetInfo(const void *pSource, EG_ImageHeader_t *pHeader)
{
EG_Result_t Result = EG_RES_INVALID;

	EG_ZeroMem(pHeader, sizeof(EG_ImageHeader_t));
	if(pSource == nullptr) return EG_RES_INVALID;
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);
	if(SourceType == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pSource;
		if(pImageBuffer->m_pData == nullptr) return EG_RES_INVALID;
	}
  POSITION Pos = m_DecoderList.GetHeadPosition();
  while(Pos != nullptr){
    EGImageDecoder *pDecoder = (EGImageDecoder*)m_DecoderList.GetNext(Pos);
		Result = pDecoder->Info(pSource, pHeader);
		if(Result == EG_RES_OK) break;
	}
	return Result;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGImageDecoder::Open(ImageDecoderDescriptor_t *pDescriptor, const void *pSource, EG_Color_t Color, int32_t FrameIndex)
{
	if(pSource == nullptr) return EG_RES_INVALID;
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);
	if(SourceType == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pSource;
		if(pImageBuffer->m_pData == nullptr) return EG_RES_INVALID;
	}
	pDescriptor->Color = Color;
	pDescriptor->SourceType = SourceType;
	pDescriptor->FrameIndex = FrameIndex;
	if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
		size_t fnlen = strlen((char*)pSource);
		pDescriptor->pSource = EG_AllocMem(fnlen + 1);
		EG_ASSERT_MALLOC(pDescriptor->pSource);
		if(pDescriptor->pSource == nullptr) {
			EG_LOG_WARN("Image pDecoder open out of memory");
			return EG_RES_INVALID;
		}
		strcpy((char *)pDescriptor->pSource, (char*)pSource);
	}
	else pDescriptor->pSource = pSource;
	EG_Result_t Result = EG_RES_INVALID;
  POSITION Pos = m_DecoderList.GetHeadPosition();
  while(Pos != nullptr){
	  EGImageDecoder *pDecoder = (EGImageDecoder *)m_DecoderList.GetNext(Pos);
		Result = pDecoder->Info(pSource, &pDescriptor->Header);
		if(Result != EG_RES_OK) continue;
		pDescriptor->pDecoder = pDecoder;
		Result = pDecoder->Open(pDescriptor);
		if(Result == EG_RES_OK) return Result;	// Opened successfully. It is a good pDecoder for this image source
		EG_ZeroMem(&pDescriptor->Header, sizeof(EG_ImageHeader_t));		// Prepare for the next loop
		pDescriptor->pErrorMsg = nullptr;
		pDescriptor->pImageData = nullptr;
		pDescriptor->pExtParam = nullptr;
		pDescriptor->OpenDelay = 0;
	}
	if(pDescriptor->SourceType == EG_IMG_SRC_FILE) EG_FreeMem((void *)pDescriptor->pSource);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////

void EGImageDecoder::Close(ImageDecoderDescriptor_t *pDescriptor)
{
  if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
    EG_FreeMem((void *)pDescriptor->pSource);
    pDescriptor->pSource = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGImageDecoder::ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer)
{
  return EG_RES_INVALID;
}

 ///////////////////////////////////////////////////////////////////////////////
// EGDecoderBuiltIn //
///////////////////////////////////////////////////////////////////////////////

EGDecoderBuiltIn::EGDecoderBuiltIn(void)
{
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBuiltIn::Info(const void *pSource, EG_ImageHeader_t *pHeader)
{
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);
	if(SourceType == EG_IMG_SRC_VARIABLE) {
		EG_ImageColorFormat_t ColorFormat = (EG_ImageColorFormat_t)((EGImageBuffer *)pSource)->m_Header.ColorFormat;
		if(ColorFormat < CF_BUILT_IN_FIRST || ColorFormat > CF_BUILT_IN_LAST) return EG_RES_INVALID;
		pHeader->Width = ((EGImageBuffer *)pSource)->m_Header.Width;
		pHeader->Height = ((EGImageBuffer *)pSource)->m_Header.Height;
		pHeader->ColorFormat = ((EGImageBuffer *)pSource)->m_Header.ColorFormat;
	}
	else if(SourceType == EG_IMG_SRC_FILE) {		// Support only "*.bin" files
		if(strcmp(EGFileSystem::GetExt((const char*)pSource), "bin")) return EG_RES_INVALID;
		EGFileSystem File;
		EG_FSResult_e Result = File.Open((char*)pSource, EG_FS_MODE_RD);
		if(Result == EG_FS_RES_OK) {
			uint32_t rn;
			Result = File.Read(pHeader, sizeof(EG_ImageHeader_t), &rn);
			File.Close();
			if(Result != EG_FS_RES_OK || rn != sizeof(EG_ImageHeader_t)) {
				EG_LOG_WARN("Image get info get read file Header");
				return EG_RES_INVALID;
			}
		}
		if(pHeader->ColorFormat < CF_BUILT_IN_FIRST || pHeader->ColorFormat > CF_BUILT_IN_LAST) return EG_RES_INVALID;
	}
	else if(SourceType == EG_IMG_SRC_SYMBOL) {
		// The size depend on the font but it is unknown here. It should be handled outside of the function
		pHeader->Width = 1;
		pHeader->Height = 1;
		/* Symbols always have transparent parts. Important because of cover check in the draw
         *function. The actual value doesn't matter because lv_draw_label will draw it */
		pHeader->ColorFormat = EG_COLOR_FORMAT_ALPHA_1BIT;
	}
	else {
		EG_LOG_WARN("Image get info found unknown pSource type");
		return EG_RES_INVALID;
	}
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBuiltIn::Open(ImageDecoderDescriptor_t *pDescriptor)
{
	if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {		// Support only "*.bin" files
		if(strcmp(EGFileSystem::GetExt((const char*)pDescriptor->pSource), "bin")) return EG_RES_INVALID;
		EGFileSystem File;
		EG_FSResult_e Result = File.Open((char*)pDescriptor->pSource, EG_FS_MODE_RD);
		if(Result != EG_FS_RES_OK) {
			EG_LOG_WARN("Built-in image pDecoder can't open the file");
			return EG_RES_INVALID;
		}
    m_File = File;	// If the file was open successfully save the file descriptor
	}
	else if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		// The variables should have valid data
		if(((EGImageBuffer *)pDescriptor->pSource)->m_pData == nullptr) {
			return EG_RES_INVALID;
		}
	}
	EG_ImageColorFormat_t ColorFormat = (EG_ImageColorFormat_t)pDescriptor->Header.ColorFormat;
	// Process A8,  RGB565A8, need load file to ram after https://github.com/lvgl/lvgl/pull/3337
	if(ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT || ColorFormat == EG_COLOR_FORMAT_RGB565A8) {
		if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
			// In case of uncompressed formats the image stored in the ROM/RAM. So simply give its pointer
			pDescriptor->pImageData = ((EGImageBuffer *)pDescriptor->pSource)->m_pData;
			return EG_RES_OK;
		}
		else {
			// If it's a file, read all to memory
			uint32_t Length = pDescriptor->Header.Width * pDescriptor->Header.Height;
			Length *= ColorFormat == EG_COLOR_FORMAT_RGB565A8 ? 3 : 1;
			uint8_t *fs_buf = (uint8_t*)EG_AllocMem(Length);
			if(fs_buf == nullptr) return EG_RES_INVALID;
			m_File.Seek(4, EG_FS_SEEK_SET); // +4 to skip the Header
			EG_FSResult_e Result = m_File.Read(fs_buf, Length, nullptr);
			if(Result != EG_FS_RES_OK) {
				EG_FreeMem(fs_buf);
				return EG_RES_INVALID;
			}
			pDescriptor->pImageData = fs_buf;
			return EG_RES_OK;
		}
	}	// Process true Color formats
	else if(ColorFormat == EG_COLOR_FORMAT_NATIVE || ColorFormat == EG_COLOR_FORMAT_NATIVE_ALPHA ||
					ColorFormat == EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED) {
		if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
			// In case of uncompressed formats the image stored in the ROM/RAM. So simply give its pointer
			pDescriptor->pImageData = ((EGImageBuffer *)pDescriptor->pSource)->m_pData;
			return EG_RES_OK;
		}
		else {	// If it's a file it need to be read line by line later
			return EG_RES_OK;
		}
	}
	// Process indexed images. Build a palette
	else if(ColorFormat == EG_COLOR_FORMAT_INDEXED_1BIT || ColorFormat == EG_COLOR_FORMAT_INDEXED_2BIT ||
          ColorFormat == EG_COLOR_FORMAT_INDEXED_4BIT || ColorFormat == EG_COLOR_FORMAT_INDEXED_8BIT) {
		uint8_t PixelSize = EGDrawImage::GetPixelSize(ColorFormat);
		uint32_t palette_size = 1 << PixelSize;
		m_pPalette = (EG_Color_t*)EG_AllocMem(palette_size * sizeof(EG_Color_t));
		EG_ASSERT_MALLOC(m_pPalette);
		m_pOPA = (EG_OPA_t*)EG_AllocMem(palette_size * sizeof(EG_OPA_t));
		EG_ASSERT_MALLOC(m_pOPA);
		if(m_pPalette == nullptr || m_pOPA == nullptr) {
			EG_LOG_ERROR("img_decoder_built_in_open: out of memory");
			Close(pDescriptor);
			return EG_RES_INVALID;
		}
		if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
			// Read the palette from file
			m_File.Seek(4, EG_FS_SEEK_SET); // Skip the Header
			EG_Color32_t cur_color;
			uint32_t i;
			for(i = 0; i < palette_size; i++) {
				m_File.Read(&cur_color, sizeof(EG_Color32_t), nullptr);
				m_pPalette[i] = EG_MixColor(cur_color.ch.red, cur_color.ch.green, cur_color.ch.blue);
				m_pOPA[i] = cur_color.ch.alpha;
			}
		}
		else {
			// The palette begins in the beginning of the image data. Just point to it.
			EG_Color32_t *palette_p = (EG_Color32_t *)((EGImageBuffer *)pDescriptor->pSource)->m_pData;
			uint32_t i;
			for(i = 0; i < palette_size; i++) {
				m_pPalette[i] = EG_MixColor(palette_p[i].ch.red, palette_p[i].ch.green, palette_p[i].ch.blue);
				m_pOPA[i] = palette_p[i].ch.alpha;
			}
		}
		return EG_RES_OK;
	}
	// Alpha indexed images.
	else if(ColorFormat == EG_COLOR_FORMAT_ALPHA_1BIT || ColorFormat == EG_COLOR_FORMAT_ALPHA_2BIT || ColorFormat == EG_COLOR_FORMAT_ALPHA_4BIT) {
		return EG_RES_OK; // Nothing to process
	}
	else {	// Unknown format. Can't decode it.
		Close(pDescriptor);		// Free the potentially allocated memories
		EG_LOG_WARN("Image pDecoder open: unknown Color format");
		return EG_RES_INVALID;
	}
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBuiltIn::ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer)
{
	EG_Result_t Result = EG_RES_INVALID;

	if(pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_NATIVE || pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_NATIVE_ALPHA ||
		 pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED) {
		if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
			Result = TrueColor(pDescriptor, X, Y, Length, pBuffer);
		}
	}
	else if(pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_ALPHA_1BIT || pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_ALPHA_2BIT ||
					pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_ALPHA_4BIT || pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT) {
		Result = Alpha(pDescriptor, X, Y, Length, pBuffer);
	}
	else if(pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_INDEXED_1BIT || pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_INDEXED_2BIT ||
					pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_INDEXED_4BIT || pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_INDEXED_8BIT) {
		Result = Indexed(pDescriptor, X, Y, Length, pBuffer);
	}
	else {
		EG_LOG_WARN("Built-in image pDecoder read not supports the Color format");
		return EG_RES_INVALID;
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderBuiltIn::Close(ImageDecoderDescriptor_t *pDescriptor)
{
  if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
    m_File.Close();
  }
  if(m_pPalette) EG_FreeMem(m_pPalette);
  if(m_pOPA) EG_FreeMem(m_pOPA);
  EGImageDecoder::Close(pDescriptor);
  pDescriptor->pExtParam = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBuiltIn::TrueColor(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y,	EG_Coord_t Length, uint8_t *pBuffer)
{
EG_FSResult_e Result;

	uint8_t PixelSize = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)pDescriptor->Header.ColorFormat);
	uint32_t pos = ((Y * pDescriptor->Header.Width + X) * PixelSize) >> 3;
	pos += 4; // Skip the Header
	Result = m_File.Seek(pos, EG_FS_SEEK_SET);
	if(Result != EG_FS_RES_OK) {
		EG_LOG_WARN("Built-in image pDecoder seek failed");
		return EG_RES_INVALID;
	}
	uint32_t btr = Length * (PixelSize >> 3);
	uint32_t br = 0;
	Result = m_File.Read(pBuffer, btr, &br);
	if(Result != EG_FS_RES_OK || btr != br) {
		EG_LOG_WARN("Built-in image pDecoder read failed");
		return EG_RES_INVALID;
	}
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBuiltIn::Alpha(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer)
{
	const EG_OPA_t alpha1_opa_table[2] = {0, 255};          // Opacity mapping with BitsPerPixel = 1 (Just for compatibility)
	const EG_OPA_t alpha2_opa_table[4] = {0, 85, 170, 255}; // Opacity mapping with BitsPerPixel = 2
	const EG_OPA_t alpha4_opa_table[16] = {0, 17, 34, 51,   // Opacity mapping with BitsPerPixel = 4
																				 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};

	// Simply fill the buffer with the Color. Later only the alpha value will be modified.
	EG_Color_t bg_color = pDescriptor->Color;
	EG_Coord_t i;
	for(i = 0; i < Length; i++) {
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE] = bg_color.full;
#elif EG_COLOR_DEPTH == 16
		// Because of Alpha byte 16 bit Color can start on odd address which can cause crash
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE] = bg_color.full & 0xFF;
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE + 1] = (bg_color.full >> 8) & 0xFF;
#elif EG_COLOR_DEPTH == 32
		*((uint32_t *)&pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE]) = bg_color.full;
#else
#error "Invalid EG_COLOR_DEPTH. Check it in EG_Config.h"
#endif
	}
	const EG_OPA_t *opa_table = nullptr;
	uint8_t PixelSize = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)pDescriptor->Header.ColorFormat);
	uint16_t mask = (1 << PixelSize) - 1; // E.g. PixelSize = 2; mask = 0x03
	EG_Coord_t Width = 0;
	uint32_t ofs = 0;
	int8_t pos = 0;
	switch(pDescriptor->Header.ColorFormat) {
		case EG_COLOR_FORMAT_ALPHA_1BIT:
			Width = (pDescriptor->Header.Width + 7) >> 3; // E.g. Width = 20 -> Width = 2 + 1
			ofs += Width * Y + (X >> 3);      // First pixel
			pos = 7 - (X & 0x7);
			opa_table = alpha1_opa_table;
			break;
		case EG_COLOR_FORMAT_ALPHA_2BIT:
			Width = (pDescriptor->Header.Width + 3) >> 2; // E.g. Width = 13 -> Width = 3 + 1 (bytes)
			ofs += Width * Y + (X >> 2);      // First pixel
			pos = 6 - (X & 0x3) * 2;
			opa_table = alpha2_opa_table;
			break;
		case EG_COLOR_FORMAT_ALPHA_4BIT:
			Width = (pDescriptor->Header.Width + 1) >> 1; // E.g. Width = 13 -> Width = 6 + 1 (bytes)
			ofs += Width * Y + (X >> 1);      // First pixel
			pos = 4 - (X & 0x1) * 4;
			opa_table = alpha4_opa_table;
			break;
		case EG_COLOR_FORMAT_ALPHA_8BIT:
			Width = pDescriptor->Header.Width; // E.g. X = 7 -> Width = 7 (bytes)
			ofs += Width * Y + X;  // First pixel
			pos = 0;
			break;
	}
	uint8_t *fs_buf = (uint8_t*)EG_GetBufferMem(Width);
	if(fs_buf == nullptr) return EG_RES_INVALID;
	const uint8_t *data_tmp = nullptr;
	if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pDescriptor->pSource;
		data_tmp = pImageBuffer->m_pData + ofs;
	}
	else {
		m_File.Seek(ofs + 4, EG_FS_SEEK_SET); // +4 to skip the Header
		m_File.Read(fs_buf, Width, nullptr);
		data_tmp = fs_buf;
	}

	for(i = 0; i < Length; i++) {
		uint8_t val_act = (*data_tmp >> pos) & mask;

		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE + EG_IMG_PX_SIZE_ALPHA_BYTE - 1] =
			pDescriptor->Header.ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT ? val_act : opa_table[val_act];

		pos -= PixelSize;
		if(pos < 0) {
			pos = 8 - PixelSize;
			data_tmp++;
		}
	}
	EG_ReleaseBufferMem(fs_buf);
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderBuiltIn::Indexed(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y,EG_Coord_t Length, uint8_t *pBuffer)
{
	uint8_t PixelSize = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)pDescriptor->Header.ColorFormat);
	uint16_t mask = (1 << PixelSize) - 1; // E.g. PixelSize = 2; mask = 0x03

	EG_Coord_t Width = 0;
	int8_t pos = 0;
	uint32_t ofs = 0;
	switch(pDescriptor->Header.ColorFormat) {
		case EG_COLOR_FORMAT_INDEXED_1BIT:
			Width = (pDescriptor->Header.Width + 7) >> 3; // E.g. Width = 20 -> Width = 2 + 1
			ofs += Width * Y + (X >> 3);      // First pixel
			ofs += 8;                     // Skip the palette
			pos = 7 - (X & 0x7);
			break;
		case EG_COLOR_FORMAT_INDEXED_2BIT:
			Width = (pDescriptor->Header.Width + 3) >> 2; // E.g. Width = 13 -> Width = 3 + 1 (bytes)
			ofs += Width * Y + (X >> 2);      // First pixel
			ofs += 16;                    // Skip the palette
			pos = 6 - (X & 0x3) * 2;
			break;
		case EG_COLOR_FORMAT_INDEXED_4BIT:
			Width = (pDescriptor->Header.Width + 1) >> 1; // E.g. Width = 13 -> Width = 6 + 1 (bytes)
			ofs += Width * Y + (X >> 1);      // First pixel
			ofs += 64;                    // Skip the palette
			pos = 4 - (X & 0x1) * 4;
			break;
		case EG_COLOR_FORMAT_INDEXED_8BIT:
			Width = pDescriptor->Header.Width; // E.g. X = 7 -> Width = 7 (bytes)
			ofs += Width * Y + X;  // First pixel
			ofs += 1024;       // Skip the palette
			pos = 0;
			break;
	}
	uint8_t *fs_buf = (uint8_t*)EG_GetBufferMem(Width);
	if(fs_buf == nullptr) return EG_RES_INVALID;
	const uint8_t *data_tmp = nullptr;
	if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pDescriptor->pSource;
		data_tmp = pImageBuffer->m_pData + ofs;
	}
	else {
		m_File.Seek(ofs + 4, EG_FS_SEEK_SET); // +4 to skip the Header
		m_File.Read(fs_buf, Width, nullptr);
		data_tmp = fs_buf;
	}
	for(EG_Coord_t i = 0; i < Length; i++) {
		uint8_t val_act = (*data_tmp >> pos) & mask;

		EG_Color_t Color = m_pPalette[val_act];
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE] = Color.full;
#elif EG_COLOR_DEPTH == 16
		// Because of Alpha byte 16 bit Color can start on odd address which can cause crash
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE] = Color.full & 0xFF;
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE + 1] = (Color.full >> 8) & 0xFF;
#elif EG_COLOR_DEPTH == 32
		*((uint32_t *)&pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE]) = Color.full;
#else
#error "Invalid EG_COLOR_DEPTH. Check it in EG_Config.h"
#endif
		pBuffer[i * EG_IMG_PX_SIZE_ALPHA_BYTE + EG_IMG_PX_SIZE_ALPHA_BYTE - 1] = m_pOPA[val_act];

		pos -= PixelSize;
		if(pos < 0) {
			pos = 8 - PixelSize;
			data_tmp++;
		}
	}
	EG_ReleaseBufferMem(fs_buf);
	return EG_RES_OK;
}
