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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

///////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------------------------------------------------------------
/    Added normal JPG support [7/10/2020]
/    ----------
/    SJPEG_t is a custom created modified JPEG file format for small embedded platforms.
/    It will contain multiple JPEG fragments all embedded into a single file with a custom header.
/    This makes JPEG decoding easier using any JPEG library. Overall file size will be almost
/    similar to the parent jpeg file. We can generate pSJPEG from any jpeg using a python script
/    provided along with this project.
/                                                                                     (by vinodstanur | 2020 )
/    SJPEG_t FILE STRUCTURE
/    --------------------------------------------------------------------------------------------------------------------------------
/    Bytes                       |   Value                                                                                           |
/    --------------------------------------------------------------------------------------------------------------------------------
/
/    0 - 7                       |   "_SJPG__" followed by '\0'
/
/    8 - 13                      |   "V1.00" followed by '\0'       [VERSION OF SJPG FILE for future compatibiliby]
/
/    14 - 15                     |   X_RESOLUTION (width)            [little endian]
/
/    16 - 17                     |   Y_RESOLUTION (height)           [little endian]
/
/    18 - 19                     |   TOTAL_FRAMES inside pSJPEG       [little endian]
/
/    20 - 21                     |   JPEG BLOCK WIDTH (16 normally)  [little endian]
/
/    22 - [(TOTAL_FRAMES*2 )]    |   SIZE OF EACH JPEG SPLIT FRAGMENTS   (FRAME_INFO_ARRAY)
/
/   SJPEG_t m_pData                   |   Each JPEG frame can be extracted from SJPEG_t m_pData by parsing the FRAME_INFO_ARRAY one time.
/
/----------------------------------------------------------------------------------------------------------------------------------
/                   JPEG DECODER
/                   ------------
/   We are using TJpgDec - Tiny JPEG Decompressor library from ELM-CHAN for decoding each split-jpeg fragments.
/   The tjpgd.c and tjpgd.h is not modified and those are used as it is. So if any update comes for the tiny-jpeg,
/   just replace those files with updated files.
/---------------------------------------------------------------------------------------------------------------------------------*/

#include "EGL.h"
#if EG_USE_SJPG

#include "extra/libs/tjpgd.h"
#include "extra/libs/EG_SJPG.h"
#include "misc/EG_FileSystem.h"

///////////////////////////////////////////////////////////////////////////////

#define TJPGD_WORKBUFF_SIZE 4096  //Recommended by TJPGD libray

//NEVER EDIT THESE OFFSET VALUES
#define SJPEG_VERSION_OFFSET 8
#define SJPEG_X_RES_OFFSET 14
#define SJPEG_y_RES_OFFSET 16
#define SJPEG_TOTAL_FRAMES_OFFSET 18
#define SJPEG_BLOCK_WIDTH_OFFSET 20
#define SJPEG_FRAME_INFO_ARRAY_OFFSET 22

enum io_source_type {
	SJPEG_IO_SOURCE_C_ARRAY,
	SJPEG_IO_SOURCE_DISK,
};

typedef struct IO_Source_t{
	enum io_source_type Type;
	EGFileSystem File;
	uint8_t *pCacheBuffer;
	int CacheResX;
	int CacheResY;
	uint8_t *pRawData;                //Used when Type==SJPEG_IO_SOURCE_C_ARRAY.
	uint32_t RawDataSize;           //Num bytes pointed to by pRawData.
	uint32_t RawDataPosition;  //Used for all types.
} IO_Source_t;

typedef struct SJPEG_t{
	uint8_t *pData;
	uint32_t DataSize;
	int ResolutionX;
	int ResolutionY;
	int TotalFrames;
	int SingleFrameHeight;
	int CacheFrameIndex;
	uint8_t **ppFrameBaseArray;  //to save base address of each split frames upto TotalFrames.
	int *pFrameBaseOffset;      //to save base Offset for fseek
	uint8_t *pFrameCache;
	uint8_t *pWorkBuffer;  //JPG work buffer for jpeg library
	JDEC *pJDEC;
	IO_Source_t pIO;
} SJPEG_t;

EGDecoderSJPG DecoderSJPG;

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderSJPG::Info(const void *pSource, EG_ImageHeader_t *pHeader)
{
EG_Result_t ret = EG_RES_OK;

	//  Read the SJPG/JPG header and find `width` and `height` 
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);

	if(SourceType == EG_IMG_SRC_VARIABLE) {
		const EGImageBuffer *pImageBuffer = (EGImageBuffer*)pSource;
		uint8_t *raw_sjpeg_data = (uint8_t *)pImageBuffer->m_pData;
		const uint32_t raw_sjpeg_data_size = pImageBuffer->m_DataSize;
		if(!strncmp((char *)raw_sjpeg_data, "_SJPG__", strlen("_SJPG__"))) {
			raw_sjpeg_data += 14;  //seek to res info ... refer pSJPEG format
			pHeader->AlwaysZero = 0;
			pHeader->ColorFormat = EG_IMG_CF_RAW;
			pHeader->Width = *raw_sjpeg_data++;
			pHeader->Width |= *raw_sjpeg_data++ << 8;
			pHeader->Height = *raw_sjpeg_data++;
			pHeader->Height |= *raw_sjpeg_data++ << 8;
			return ret;
		}
		else if(IsJPG(raw_sjpeg_data, raw_sjpeg_data_size) == true) {
			pHeader->AlwaysZero = 0;
			pHeader->ColorFormat = EG_IMG_CF_RAW;
			uint8_t *pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
			if(!pWorkBuffer) return EG_RES_INVALID;
			IO_Source_t io_source_temp;
			io_source_temp.Type = SJPEG_IO_SOURCE_C_ARRAY;
			io_source_temp.pRawData = raw_sjpeg_data;
			io_source_temp.RawDataSize = raw_sjpeg_data_size;
			io_source_temp.RawDataPosition = 0;
			JDEC TempJDEC;
			JRESULT rc = jd_prepare(&TempJDEC, InputFunc, pWorkBuffer, (size_t)TJPGD_WORKBUFF_SIZE, &io_source_temp);
			if(rc == JDR_OK) {
				pHeader->Width = TempJDEC.width;
				pHeader->Height = TempJDEC.height;
			}
			else {
				ret = EG_RES_INVALID;
				goto end;
			}

end:
			EG_FreeMem(pWorkBuffer);
			return ret;
		}
	}
	else if(SourceType == EG_IMG_SRC_FILE) {
		const char *pFileName = (char*)pSource;
		if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "sjpg") == 0) {
			uint8_t Buffer[22];
			memset(Buffer, 0, sizeof(Buffer));
			EGFileSystem File;
			EG_FSResult_e res = File.Open(pFileName, EG_FS_MODE_RD);
			if(res != EG_FS_RES_OK) return 78;
			uint32_t rn;
			res = File.Read(Buffer, 8, &rn);
			if(res != EG_FS_RES_OK || rn != 8) {
				File.Close();
				return EG_RES_INVALID;
			}
			if(strcmp((char *)Buffer, "_SJPG__") == 0) {
				File.Seek(14, EG_FS_SEEK_SET);
				res = File.Read(Buffer, 4, &rn);
				if(res != EG_FS_RES_OK || rn != 4) {
					File.Close();
					return EG_RES_INVALID;
				}
				pHeader->AlwaysZero = 0;
				pHeader->ColorFormat = EG_IMG_CF_RAW;
				uint8_t *raw_sjpeg_data = Buffer;
				pHeader->Width = *raw_sjpeg_data++;
				pHeader->Width |= *raw_sjpeg_data++ << 8;
				pHeader->Height = *raw_sjpeg_data++;
				pHeader->Height |= *raw_sjpeg_data++ << 8;
				File.Close();
				return EG_RES_OK;
			}
		}
		else if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "jpg") == 0) {
			EGFileSystem File;
			EG_FSResult_e res = File.Open(pFileName, EG_FS_MODE_RD);
			if(res != EG_FS_RES_OK) return 78;
			uint8_t *pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
			if(!pWorkBuffer) {
				File.Close();
				return EG_RES_INVALID;
			}
			IO_Source_t io_source_temp;
			io_source_temp.Type = SJPEG_IO_SOURCE_DISK;
			io_source_temp.RawDataPosition = 0;
			io_source_temp.pCacheBuffer = NULL;
			io_source_temp.File = File;
			JDEC TempJDEC;
			JRESULT rc = jd_prepare(&TempJDEC, InputFunc, pWorkBuffer, (size_t)TJPGD_WORKBUFF_SIZE, &io_source_temp);
			EG_FreeMem(pWorkBuffer);
			File.Close();
			if(rc == JDR_OK) {
				pHeader->AlwaysZero = 0;
				pHeader->ColorFormat = EG_IMG_CF_RAW;
				pHeader->Width = TempJDEC.width;
				pHeader->Height = TempJDEC.height;
				return EG_RES_OK;
			}
		}
	}
	return EG_RES_INVALID;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderSJPG::Open(ImageDecoderDescriptor_t  *pDescriptor)
{
EG_Result_t Result = EG_RES_OK;

	if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		uint8_t *m_pData;
		SJPEG_t *pSJPEG = (SJPEG_t *)pDescriptor->pExtParam;
		const uint32_t raw_sjpeg_data_size = ((EGImageBuffer *)pDescriptor->pSource)->m_DataSize;
		if(pSJPEG == NULL) {
			pSJPEG = (SJPEG_t*)EG_AllocMem(sizeof(SJPEG_t));
			if(!pSJPEG) return EG_RES_INVALID;
			memset(pSJPEG, 0, sizeof(SJPEG_t));
			pDescriptor->pExtParam = pSJPEG;
			pSJPEG->pData = (uint8_t *)((EGImageBuffer *)(pDescriptor->pSource))->m_pData;
			pSJPEG->DataSize = ((EGImageBuffer *)(pDescriptor->pSource))->m_DataSize;
		}
		if(!strncmp((char *)pSJPEG->pData, "_SJPG__", strlen("_SJPG__"))) {
			m_pData = pSJPEG->pData;
			m_pData += 14;
			pSJPEG->ResolutionX = *m_pData++;
			pSJPEG->ResolutionX |= *m_pData++ << 8;
			pSJPEG->ResolutionY = *m_pData++;
			pSJPEG->ResolutionY |= *m_pData++ << 8;
			pSJPEG->TotalFrames = *m_pData++;
			pSJPEG->TotalFrames |= *m_pData++ << 8;
			pSJPEG->SingleFrameHeight = *m_pData++;
			pSJPEG->SingleFrameHeight |= *m_pData++ << 8;
			pSJPEG->ppFrameBaseArray = (uint8_t**)EG_AllocMem(sizeof(uint8_t*) * pSJPEG->TotalFrames);
			if(!pSJPEG->ppFrameBaseArray) {
				Cleanup(pSJPEG);
				pSJPEG = NULL;
				return EG_RES_INVALID;
			}
			pSJPEG->pFrameBaseOffset = NULL;
			uint8_t *img_frame_base = m_pData + pSJPEG->TotalFrames * 2;
			pSJPEG->ppFrameBaseArray[0] = img_frame_base;
			for(int i = 1; i < pSJPEG->TotalFrames; i++) {
				int Offset = *m_pData++;
				Offset |= *m_pData++ << 8;
				pSJPEG->ppFrameBaseArray[i] = pSJPEG->ppFrameBaseArray[i - 1] + Offset;
			}
			pSJPEG->CacheFrameIndex = -1;
			pSJPEG->pFrameCache = (uint8_t*)EG_AllocMem(pSJPEG->ResolutionX * pSJPEG->SingleFrameHeight * 3 /*2*/);
			if(!pSJPEG->pFrameCache) {
				Cleanup(pSJPEG);
				pSJPEG = NULL;
				return EG_RES_INVALID;
			}
			pSJPEG->pIO.pCacheBuffer = pSJPEG->pFrameCache;
			pSJPEG->pIO.CacheResX = pSJPEG->ResolutionX;
			pSJPEG->pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
			if(!pSJPEG->pWorkBuffer) {
				Cleanup(pSJPEG);
				pSJPEG = NULL;
				return EG_RES_INVALID;
			}

			pSJPEG->pJDEC = (JDEC*)EG_AllocMem(sizeof(JDEC));
			if(!pSJPEG->pJDEC) {
				Cleanup(pSJPEG);
				pSJPEG = NULL;
				return EG_RES_INVALID;
			}
			pSJPEG->pIO.Type = SJPEG_IO_SOURCE_C_ARRAY;
			pSJPEG->pIO.File.m_pFile = nullptr;
			pDescriptor->pImageData = nullptr;
			return Result;
		}
		else if(IsJPG(pSJPEG->pData, raw_sjpeg_data_size) == true) {
			uint8_t *pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
			if(!pWorkBuffer) {
				Cleanup(pSJPEG);
				pSJPEG = NULL;
				return EG_RES_INVALID;
			}
			IO_Source_t io_source_temp;
			io_source_temp.Type = SJPEG_IO_SOURCE_C_ARRAY;
			io_source_temp.pRawData = pSJPEG->pData;
			io_source_temp.RawDataSize = pSJPEG->DataSize;
			io_source_temp.RawDataPosition = 0;
			JDEC TempJDEC;
			JRESULT rc = jd_prepare(&TempJDEC, InputFunc, pWorkBuffer, (size_t)TJPGD_WORKBUFF_SIZE, &io_source_temp);
			EG_FreeMem(pWorkBuffer);
			if(rc == JDR_OK) {
				pSJPEG->ResolutionX = TempJDEC.width;
				pSJPEG->ResolutionY = TempJDEC.height;
				pSJPEG->TotalFrames = 1;
				pSJPEG->SingleFrameHeight = TempJDEC.height;
				pSJPEG->ppFrameBaseArray = (uint8_t**)EG_AllocMem(sizeof(uint8_t *) * pSJPEG->TotalFrames);
				if(!pSJPEG->ppFrameBaseArray) {
					Cleanup(pSJPEG);
					pSJPEG = NULL;
					return EG_RES_INVALID;
				}
				pSJPEG->pFrameBaseOffset = NULL;
				uint8_t *img_frame_base = pSJPEG->pData;
				pSJPEG->ppFrameBaseArray[0] = img_frame_base;
				pSJPEG->CacheFrameIndex = -1;
				pSJPEG->pFrameCache = (uint8_t*)EG_AllocMem(pSJPEG->ResolutionX * pSJPEG->SingleFrameHeight * 3);
				if(!pSJPEG->pFrameCache) {
					Cleanup(pSJPEG);
					pSJPEG = NULL;
					return EG_RES_INVALID;
				}
				pSJPEG->pIO.pCacheBuffer = pSJPEG->pFrameCache;
				pSJPEG->pIO.CacheResX = pSJPEG->ResolutionX;
				pSJPEG->pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
				if(!pSJPEG->pWorkBuffer) {
					Cleanup(pSJPEG);
					pSJPEG = NULL;
					return EG_RES_INVALID;
				}
				pSJPEG->pJDEC = (JDEC*)EG_AllocMem(sizeof(JDEC));
				if(!pSJPEG->pJDEC) {
					Cleanup(pSJPEG);
					pSJPEG = NULL;
					return EG_RES_INVALID;
				}
				pSJPEG->pIO.Type = SJPEG_IO_SOURCE_C_ARRAY;
				pSJPEG->pIO.File.m_pFile= nullptr;
				pDescriptor->pImageData = nullptr;
				return Result;
			}
			else {
				Result = EG_RES_INVALID;
				goto end;
			}

end:
			EG_FreeMem(pWorkBuffer);
			return Result;
		}
	}
	else if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
		//  If all fine, then the file will be kept open 
		const char *pFileName = (char*)pDescriptor->pSource;
		uint8_t *m_pData;
		if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "sjpg") == 0) {
			uint8_t Buffer[22];
			memset(Buffer, 0, sizeof(Buffer));
			EGFileSystem File;
			EG_FSResult_e res = File.Open(pFileName, EG_FS_MODE_RD);
			if(res != EG_FS_RES_OK) {
				return 78;
			}
			uint32_t rn;
			res = File.Read(Buffer, 22, &rn);
			if(res != EG_FS_RES_OK || rn != 22) {
				File.Close();
				return EG_RES_INVALID;
			}
			if(strcmp((char *)Buffer, "_SJPG__") == 0) {
				SJPEG_t *pSJPEG = (SJPEG_t *)pDescriptor->pExtParam;
				if(pSJPEG == NULL) {
					pSJPEG = (SJPEG_t*)EG_AllocMem(sizeof(SJPEG_t));
					if(!pSJPEG) {
						File.Close();
						return EG_RES_INVALID;
					}
					memset(pSJPEG, 0, sizeof(SJPEG_t));
					pDescriptor->pExtParam = pSJPEG;
					pSJPEG->pData = (uint8_t*)((EGImageBuffer *)(pDescriptor->pSource))->m_pData;
					pSJPEG->DataSize = ((EGImageBuffer *)(pDescriptor->pSource))->m_DataSize;
				}
				m_pData = Buffer;
				m_pData += 14;
				pSJPEG->ResolutionX = *m_pData++;
				pSJPEG->ResolutionX |= *m_pData++ << 8;
				pSJPEG->ResolutionY = *m_pData++;
				pSJPEG->ResolutionY |= *m_pData++ << 8;
				pSJPEG->TotalFrames = *m_pData++;
				pSJPEG->TotalFrames |= *m_pData++ << 8;
				pSJPEG->SingleFrameHeight = *m_pData++;
				pSJPEG->SingleFrameHeight |= *m_pData++ << 8;
				pSJPEG->ppFrameBaseArray = NULL;  //EG_AllocMem( sizeof(uint8_t *) * pSJPEG->TotalFrames );
				pSJPEG->pFrameBaseOffset = (int*)EG_AllocMem(sizeof(int) * pSJPEG->TotalFrames);
				if(!pSJPEG->pFrameBaseOffset) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				int img_frame_start_offset = (SJPEG_FRAME_INFO_ARRAY_OFFSET + pSJPEG->TotalFrames * 2);
				pSJPEG->pFrameBaseOffset[0] = img_frame_start_offset;  //pointer used to save integer for now...
				for(int i = 1; i < pSJPEG->TotalFrames; i++) {
					res = File.Read(Buffer, 2, &rn);
					if(res != EG_FS_RES_OK || rn != 2) {
						File.Close();
						return EG_RES_INVALID;
					}
					m_pData = Buffer;
					int Offset = *m_pData++;
					Offset |= *m_pData++ << 8;
					pSJPEG->pFrameBaseOffset[i] = pSJPEG->pFrameBaseOffset[i - 1] + Offset;
				}
				pSJPEG->CacheFrameIndex = -1;  //INVALID AT BEGINNING for a forced compare mismatch at first time.
				pSJPEG->pFrameCache = (uint8_t*)EG_AllocMem(pSJPEG->ResolutionX * pSJPEG->SingleFrameHeight * 3);
				if(!pSJPEG->pFrameCache) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				pSJPEG->pIO.pCacheBuffer = pSJPEG->pFrameCache;
				pSJPEG->pIO.CacheResX = pSJPEG->ResolutionX;
				pSJPEG->pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
				if(!pSJPEG->pWorkBuffer) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}

				pSJPEG->pJDEC = (JDEC*)EG_AllocMem(sizeof(JDEC));
				if(!pSJPEG->pJDEC) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				pSJPEG->pIO.Type = SJPEG_IO_SOURCE_DISK;
				pSJPEG->pIO.File = File;
				pDescriptor->pImageData = NULL;
				return EG_RES_OK;
			}
		}
		else if(strcmp(EGFileSystem::GetExt((const char *)pFileName), "jpg") == 0) {
			EGFileSystem File;
			EG_FSResult_e res = File.Open( pFileName, EG_FS_MODE_RD);
			if(res != EG_FS_RES_OK) {
				return EG_RES_INVALID;
			}
			SJPEG_t *pSJPEG = (SJPEG_t *)pDescriptor->pExtParam;
			if(pSJPEG == NULL) {
				pSJPEG = (SJPEG_t*)EG_AllocMem(sizeof(SJPEG_t));
				if(!pSJPEG) {
					File.Close();
					return EG_RES_INVALID;
				}
				memset(pSJPEG, 0, sizeof(SJPEG_t));
				pDescriptor->pExtParam = pSJPEG;
				pSJPEG->pData = (uint8_t *)((EGImageBuffer *)(pDescriptor->pSource))->m_pData;
				pSJPEG->DataSize = ((EGImageBuffer *)(pDescriptor->pSource))->m_DataSize;
			}
			uint8_t *pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
			if(!pWorkBuffer) {
				File.Close();
				Cleanup(pSJPEG);
				return EG_RES_INVALID;
			}
			IO_Source_t io_source_temp;
			io_source_temp.Type = SJPEG_IO_SOURCE_DISK;
			io_source_temp.RawDataPosition = 0;
			io_source_temp.pCacheBuffer = NULL;
			io_source_temp.File = File;
			JDEC TempJDEC;
			JRESULT rc = jd_prepare(&TempJDEC, InputFunc, pWorkBuffer, (size_t)TJPGD_WORKBUFF_SIZE, &io_source_temp);
			EG_FreeMem(pWorkBuffer);
			if(rc == JDR_OK) {
				pSJPEG->ResolutionX = TempJDEC.width;
				pSJPEG->ResolutionY = TempJDEC.height;
				pSJPEG->TotalFrames = 1;
				pSJPEG->SingleFrameHeight = TempJDEC.height;
				pSJPEG->ppFrameBaseArray = NULL;
				pSJPEG->pFrameBaseOffset = (int*)EG_AllocMem(sizeof(uint8_t *) * pSJPEG->TotalFrames);
				if(!pSJPEG->pFrameBaseOffset) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				int img_frame_start_offset = 0;
				pSJPEG->pFrameBaseOffset[0] = img_frame_start_offset;
				pSJPEG->CacheFrameIndex = -1;
				pSJPEG->pFrameCache = (uint8_t*)EG_AllocMem(pSJPEG->ResolutionX * pSJPEG->SingleFrameHeight * 3);
				if(!pSJPEG->pFrameCache) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				pSJPEG->pIO.pCacheBuffer = pSJPEG->pFrameCache;
				pSJPEG->pIO.CacheResX = pSJPEG->ResolutionX;
				pSJPEG->pWorkBuffer = (uint8_t*)EG_AllocMem(TJPGD_WORKBUFF_SIZE);
				if(!pSJPEG->pWorkBuffer) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				pSJPEG->pJDEC = (JDEC*)EG_AllocMem(sizeof(JDEC));
				if(!pSJPEG->pJDEC) {
					File.Close();
					Cleanup(pSJPEG);
					return EG_RES_INVALID;
				}
				pSJPEG->pIO.Type = SJPEG_IO_SOURCE_DISK;
				pSJPEG->pIO.File = File;
				pDescriptor->pImageData = NULL;
				return EG_RES_OK;
			}
			else {
				if(pDescriptor->pExtParam) EG_FreeMem(pDescriptor->pExtParam);
				File.Close();
				return EG_RES_INVALID;
			}
		}
	}
	return EG_RES_INVALID;
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderSJPG::ReadLine(ImageDecoderDescriptor_t *pDescriptor, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Length, uint8_t *pBuffer)
{
	if(pDescriptor->SourceType == EG_IMG_SRC_VARIABLE) {
		SJPEG_t *pSJPEG = (SJPEG_t *)pDescriptor->pExtParam;
		JRESULT rc;
		int sjpeg_req_frame_index = Y / pSJPEG->SingleFrameHeight;
		// If line not from pCache, refresh pCache 
		if(sjpeg_req_frame_index != pSJPEG->CacheFrameIndex) {
			pSJPEG->pIO.pRawData = pSJPEG->ppFrameBaseArray[sjpeg_req_frame_index];
			if(sjpeg_req_frame_index == (pSJPEG->TotalFrames - 1)) {
				// This is the last frame. 
				const uint32_t frame_offset = (uint32_t)(pSJPEG->pIO.pRawData - pSJPEG->pData);
				pSJPEG->pIO.RawDataSize = pSJPEG->DataSize - frame_offset;
			}
			else {
				pSJPEG->pIO.RawDataSize =	(uint32_t)(pSJPEG->ppFrameBaseArray[sjpeg_req_frame_index + 1] - pSJPEG->pIO.pRawData);
			}
			pSJPEG->pIO.RawDataPosition = 0;
			rc = jd_prepare(pSJPEG->pJDEC, InputFunc, pSJPEG->pWorkBuffer, (size_t)TJPGD_WORKBUFF_SIZE, &(pSJPEG->pIO));
			if(rc != JDR_OK) return EG_RES_INVALID;
			rc = jd_decomp(pSJPEG->pJDEC, ImageDataCB, 0);
			if(rc != JDR_OK) return EG_RES_INVALID;
			pSJPEG->CacheFrameIndex = sjpeg_req_frame_index;
		}
		int Offset = 0;
		uint8_t *pCache = (uint8_t *)pSJPEG->pFrameCache + X * 3 + (Y % pSJPEG->SingleFrameHeight) * pSJPEG->ResolutionX * 3;
#if EG_COLOR_DEPTH == 32
		for(int i = 0; i < Length; i++) {
			pBuffer[Offset + 3] = 0xff;
			pBuffer[Offset + 2] = *pCache++;
			pBuffer[Offset + 1] = *pCache++;
			pBuffer[Offset + 0] = *pCache++;
			Offset += 4;
		}
#elif EG_COLOR_DEPTH == 16
		for(int i = 0; i < Length; i++) {
			uint16_t col_16bit = (*pCache++ & 0xf8) << 8;
			col_16bit |= (*pCache++ & 0xFC) << 3;
			col_16bit |= (*pCache++ >> 3);
#if EG_BIG_ENDIAN_SYSTEM == 1 || EG_COLOR_16_SWAP == 1
			pBuffer[Offset++] = col_16bit >> 8;
			pBuffer[Offset++] = col_16bit & 0xff;
#else
			pBuffer[Offset++] = col_16bit & 0xff;
			pBuffer[Offset++] = col_16bit >> 8;
#endif  // EG_BIG_ENDIAN_SYSTEM
		}
#elif EG_COLOR_DEPTH == 8
		for(int i = 0; i < Length; i++) {
			uint8_t col_8bit = (*pCache++ & 0xC0);
			col_8bit |= (*pCache++ & 0xe0) >> 2;
			col_8bit |= (*pCache++ & 0xe0) >> 5;
			pBuffer[Offset++] = col_8bit;
		}
#else
#error Unsupported EG_COLOR_DEPTH
#endif  // EG_COLOR_DEPTH
		return EG_RES_OK;
	}
	else if(pDescriptor->SourceType == EG_IMG_SRC_FILE) {
		SJPEG_t *pSJPEG = (SJPEG_t *)pDescriptor->pExtParam;
		JRESULT rc;
		int sjpeg_req_frame_index = Y / pSJPEG->SingleFrameHeight;
		EGFileSystem *pFile = &(pSJPEG->pIO.File);
		if(!pFile) goto end;
		// If line not from pCache, refresh pCache 
		if(sjpeg_req_frame_index != pSJPEG->CacheFrameIndex) {
			pSJPEG->pIO.RawDataPosition = (int)(pSJPEG->pFrameBaseOffset[sjpeg_req_frame_index]);
			pSJPEG->pIO.File.Seek(pSJPEG->pIO.RawDataPosition, EG_FS_SEEK_SET);
			rc = jd_prepare(pSJPEG->pJDEC, InputFunc, pSJPEG->pWorkBuffer, (size_t)TJPGD_WORKBUFF_SIZE, &(pSJPEG->pIO));
			if(rc != JDR_OK) return EG_RES_INVALID;
			rc = jd_decomp(pSJPEG->pJDEC, ImageDataCB, 0);
			if(rc != JDR_OK) return EG_RES_INVALID;
			pSJPEG->CacheFrameIndex = sjpeg_req_frame_index;
		}
		int Offset = 0;
		uint8_t *pCache = (uint8_t *)pSJPEG->pFrameCache + X * 3 + (Y % pSJPEG->SingleFrameHeight) * pSJPEG->ResolutionX * 3;
#if EG_COLOR_DEPTH == 32
		for(int i = 0; i < Length; i++) {
			pBuffer[Offset + 3] = 0xff;
			pBuffer[Offset + 2] = *pCache++;
			pBuffer[Offset + 1] = *pCache++;
			pBuffer[Offset + 0] = *pCache++;
			Offset += 4;
		}
#elif EG_COLOR_DEPTH == 16
		for(int i = 0; i < Length; i++) {
			uint16_t col_8bit = (*pCache++ & 0xf8) << 8;
			col_8bit |= (*pCache++ & 0xFC) << 3;
			col_8bit |= (*pCache++ >> 3);
#if EG_BIG_ENDIAN_SYSTEM == 1 || EG_COLOR_16_SWAP == 1
			pBuffer[Offset++] = col_8bit >> 8;
			pBuffer[Offset++] = col_8bit & 0xff;
#else
			pBuffer[Offset++] = col_8bit & 0xff;
			pBuffer[Offset++] = col_8bit >> 8;
#endif  // EG_BIG_ENDIAN_SYSTEM
		}

#elif EG_COLOR_DEPTH == 8
		for(int i = 0; i < Length; i++) {
			uint8_t col_8bit = (*pCache++ & 0xC0);
			col_8bit |= (*pCache++ & 0xe0) >> 2;
			col_8bit |= (*pCache++ & 0xe0) >> 5;
			pBuffer[Offset++] = col_8bit;
		}
#else
#error Unsupported EG_COLOR_DEPTH
#endif  // EG_COLOR_DEPTH
		return EG_RES_OK;
	}
end:
	return EG_RES_INVALID;
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderSJPG::Close(ImageDecoderDescriptor_t *pDescriptor)
{
	// Free all allocated m_pData
	SJPEG_t *pSJPEG = (SJPEG_t *)pDescriptor->pExtParam;
	if(!pSJPEG) return;
	switch(pDescriptor->SourceType) {
		case EG_IMG_SRC_FILE:
			if(pSJPEG->pIO.File.m_pFile) {
				pSJPEG->pIO.File.Close();
			}
			Cleanup(pSJPEG);
			break;
		case EG_IMG_SRC_VARIABLE:
			Cleanup(pSJPEG);
			break;
		default:;
	}
}

///////////////////////////////////////////////////////////////////////////////

int EGDecoderSJPG::ImageDataCB(JDEC *pJdec, void *m_pData, JRECT *pRect)
{
	IO_Source_t *pIO = (IO_Source_t*)pJdec->device;
	uint8_t *pCache = pIO->pCacheBuffer;
	const int ResolutionX = pIO->CacheResX;
	uint8_t *pBuffer = (uint8_t*)m_pData;
	const int INPUT_PIXEL_SIZE = 3;
	const int RowWidth = pRect->right - pRect->left + 1;  // Row width in pixels.
	const int RowSize = RowWidth * INPUT_PIXEL_SIZE;      // Row size (bytes).
	for(int y = pRect->top; y <= pRect->bottom; y++) {
		int RowOffset = y * ResolutionX * INPUT_PIXEL_SIZE + pRect->left * INPUT_PIXEL_SIZE;
		memcpy(pCache + RowOffset, pBuffer, RowSize);
		pBuffer += RowSize;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

size_t EGDecoderSJPG::InputFunc(JDEC *pJdec, uint8_t *Buffer, size_t FetchCount)
{
	IO_Source_t *pIO = (IO_Source_t*)pJdec->device;
	if(!pIO) return 0;
	if(pIO->Type == SJPEG_IO_SOURCE_C_ARRAY) {
		const uint32_t bytes_left = pIO->RawDataSize - pIO->RawDataPosition;
		const uint32_t to_read = FetchCount <= bytes_left ? (uint32_t)FetchCount : bytes_left;
		if(to_read == 0) return 0;
		if(Buffer) {
			memcpy(Buffer, pIO->pRawData + pIO->RawDataPosition, to_read);
		}
		pIO->RawDataPosition += to_read;
		return to_read;
	}
	else if(pIO->Type == SJPEG_IO_SOURCE_DISK) {
		EGFileSystem *pFile = &(pIO->File);
		if(Buffer) {
			uint32_t rn = 0;
			pFile->Read(Buffer, (uint32_t)FetchCount, &rn);
			return rn;
		}
		else {
			uint32_t pos;
			pFile->Tell(&pos);
			pFile->Seek((uint32_t)(FetchCount + pos), EG_FS_SEEK_SET);
			return FetchCount;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int EGDecoderSJPG::IsJPG(const uint8_t *raw_data, size_t Length)
{
	const uint8_t jpg_signature[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46};
	if(Length < sizeof(jpg_signature)) return false;
	return memcmp(jpg_signature, raw_data, sizeof(jpg_signature)) == 0;
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderSJPG::Cleanup(SJPEG_t *pSJPEG)
{
	if(!pSJPEG) return;
	Free(pSJPEG);
	EG_FreeMem(pSJPEG);
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderSJPG::Free(SJPEG_t *pSJPEG)
{
	if(pSJPEG->pFrameCache) EG_FreeMem(pSJPEG->pFrameCache);
	if(pSJPEG->ppFrameBaseArray) EG_FreeMem(pSJPEG->ppFrameBaseArray);
	if(pSJPEG->pFrameBaseOffset) EG_FreeMem(pSJPEG->pFrameBaseOffset);
	if(pSJPEG->pJDEC) EG_FreeMem(pSJPEG->pJDEC);
	if(pSJPEG->pWorkBuffer) EG_FreeMem(pSJPEG->pWorkBuffer);
}


#endif 
