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

#include "EGL.h"

#if EG_USE_FS_FATFS
#include "ff.h"

#if EG_FS_FATFS_LETTER == '\0'
#error "EG_FS_FATFS_LETTER must be an upper case ASCII letter"
#endif

/////////////////////////////////////////////////////////////////////////////////////////

static void EG_InitialiseFS(void);

static void* FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode);
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile);
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount);
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount);
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t Pos, EG_FS_Seek_e Mode);
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos);
static void* FS_OpenDir(EGFileDriver *pDriver, const char *pPath);
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName);
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir);

/////////////////////////////////////////////////////////////////////////////////////////

void EG_FSInitialise_FAT(void)
{
	EG_InitialiseFS();                  // Initialize your storage device and File System
	static EGFileDriver FSDriver; // A driver class
	FSDriver.m_DriverID = EG_FS_FATFS_LETTER;
	FSDriver.m_CacheSize = EG_FS_FATFS_CACHE_SIZE;
	FSDriver.OpenCB = FS_Open;
	FSDriver.CloseCB = FS_Close;
	FSDriver.ReadCB = FS_Read;
	FSDriver.WriteCB = FS_Write;
	FSDriver.SeekCB = FS_Seek;
	FSDriver.TellCB = FS_Tell;
	FSDriver.DirCloseCB = FS_CloseDir;
	FSDriver.DirOpenCB = FS_OpenDir;
	FSDriver.DirReadCB = FS_ReadDir;
	EGFileSystem::Register(&FSDriver);
}

/////////////////////////////////////////////////////////////////////////////////////////

/*Initialize your Storage device and File system.*/
static void EG_InitialiseFS(void)
{
	/*Initialize the SD card and FAT file system.
     *Better to do it in your code to keep this library untouched for easy updating*/
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Open a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param Mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to FIL struct or NULL in case of fail
 */
static void* FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode)
{
EG_UNUSED(pDriver);
uint8_t flags = 0;

	if(Mode == EG_FS_MODE_WR) flags = FA_WRITE | FA_OPEN_ALWAYS;
	else if(Mode == EG_FS_MODE_RD) flags = FA_READ;
	else if(Mode == (EG_FS_MODE_WR | EG_FS_MODE_RD)) flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
	FIL *pFile = (FIL*)EG_AllocMem(sizeof(FIL));
	if(pFile == nullptr) return nullptr;

	FRESULT res = f_open(pFile, pPath, flags);
	if(res == FR_OK) return pFile;
	else {
		EG_FreeMem(pFile);
		return nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FIL variable. (opened with fs_open)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile)
{
	EG_UNUSED(pDriver);
	f_close((FIL*)pFile);
	EG_FreeMem(pFile);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read data from an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FIL variable.
 * @param pBuffer pointer to a memory block where to store the read data
 * @param Count number of Bytes To Read
 * @param pReadCount the real number of read bytes (Byte Read)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount)
{
EG_UNUSED(pDriver);

	FRESULT Res = f_read((FIL*)pFile, pBuffer, Count, (UINT *)pReadCount);
	if(Res == FR_OK) return EG_FS_RES_OK;
	else return EG_FS_RES_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Write into a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FIL variable
 * @param pBuffer pointer to a buffer with the bytes to write
 * @param Count Bytes To Write
 * @param pWriteCount the number of real written bytes (Bytes Written). NULL if unused.
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount)
{
EG_UNUSED(pDriver);

	FRESULT res = f_write((FIL*)pFile, pBuffer, Count, (UINT *)pWriteCount);
	if(res == FR_OK) return EG_FS_RES_OK;
	else return EG_FS_RES_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FIL variable. (opened with fs_open )
 * @param Pos the new position of read write pointer
 * @param Mode only LV_SEEK_SET is supported
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t Pos, EG_FS_Seek_e Mode)
{
EG_UNUSED(pDriver);

	switch(Mode) {
		case EG_FS_SEEK_SET:
			f_lseek((FIL*)pFile, Pos);
			break;
		case EG_FS_SEEK_CUR:
			f_lseek((FIL*)pFile, f_tell((FIL *)pFile) + Pos);
			break;
		case EG_FS_SEEK_END:
			f_lseek((FIL*)pFile, f_size((FIL *)pFile) + Pos);
			break;
		default:
			break;
	}
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Give the position of the read write pointer
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FIL variable.
 * @param pPos pointer to to store the result
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos)
{
EG_UNUSED(pDriver);

	*pPos = f_tell((FIL*)pFile);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize a 'DIR' variable for directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath path to a directory
 * @return pointer to an initialized 'DIR' variable
 */
static void *FS_OpenDir(EGFileDriver *pDriver, const char *pPath)
{
	EG_UNUSED(pDriver);
	FF_DIR *pDir = (FF_DIR*)EG_AllocMem(sizeof(FF_DIR));
	if(pDir == nullptr) return nullptr;
	FRESULT Res = f_opendir(pDir, pPath);
	if(Res != FR_OK) {
		EG_FreeMem(pDir);
		pDir = nullptr;
	}
	return pDir;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the next filename from a directory.
 * The name of the directories will begin with '/'
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' variable
 * @param pFileName pointer to a buffer to store the filename
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName)
{
EG_UNUSED(pDriver);
FRESULT Res;
FILINFO FileInfo;

  pFileName[0] = '\0';
	do {
		Res = f_readdir((FF_DIR*)pDir, &FileInfo);
		if(Res != FR_OK) return EG_FS_RES_UNKNOWN;
		if(FileInfo.fattrib & AM_DIR) {
			pFileName[0] = '/';
			strcpy(&pFileName[1], FileInfo.fname);
		}
		else strcpy(pFileName, FileInfo.fname);
	} while(strcmp(pFileName, "/.") == 0 || strcmp(pFileName, "/..") == 0);
	return EG_FS_RES_OK;
}


/**
 * Close the directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' variable
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir)
{
EG_UNUSED(pDriver);

	f_closedir((FF_DIR*)pDir);
	EG_FreeMem(pDir);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

#else

#if defined(EG_FS_FATFS_LETTER) && EG_FS_FATFS_LETTER != '\0'
#warning "EG_USE_FS_FATFS is not enabled but EG_FS_FATFS_LETTER is set"
#endif

#endif
