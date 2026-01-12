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
#if EG_USE_FS_STDIO != '\0'

#include <stdio.h>
#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#define MAX_PATH_LEN 256

typedef struct {
#ifdef WIN32
	HANDLE pDir;
	char next_fn[MAX_PATH_LEN];
#else
	DIR *pDir;
#endif
} dir_handle_t;

/////////////////////////////////////////////////////////////////////////////////////////

static void* FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode);
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile);
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount);
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount);
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t Pos, EG_FS_Seek_e Mode);
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos);
static void *FS_OpenDir(EGFileDriver *pDriver, const char *pPath);
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName);
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir);

/////////////////////////////////////////////////////////////////////////////////////////

void EG_FSInitialise_Stdio(void)
{
	static EGFileDriver FSDriver; // A driver class
	FSDriver.m_DriverID = EG_FS_STDIO_LETTER;
	FSDriver.m_CacheSize = EG_FS_STDIO_CACHE_SIZE;
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

/**
 * Open a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath pPath to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param Mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to FIL struct or NULL in case of fail
 */
static void *FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode)
{
EG_UNUSED(pDriver);
const char *flags = "";

	if(Mode == EG_FS_MODE_WR)	flags = "wb";
	else if(Mode == EG_FS_MODE_RD) flags = "rb";
	else if(Mode == (EG_FS_MODE_WR | EG_FS_MODE_RD)) flags = "rb+";
	char pBuffer[MAX_PATH_LEN];
	eg_snprintf(pBuffer, sizeof(pBuffer), EG_FS_STDIO_PATH "%s", pPath);
	return fopen(pBuffer, flags);
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable. (opened with fs_open)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile)
{
	EG_UNUSED(pDriver);
	fclose(pFile);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read data from an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable.
 * @param pBuffer pointer to a memory block where to store the read data
 * @param Count number of Bytes To Read
 * @param pReadCount the real number of read bytes (Byte Read)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount)
{
	EG_UNUSED(pDriver);
	*pReadCount = fread(pBuffer, 1, Count, pFile);
	return (int32_t)(*pReadCount) < 0 ? EG_FS_RES_UNKNOWN : EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Write into a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable
 * @param pBuffer pointer to a buffer with the bytes to write
 * @param Count Bytes To Write
 * @param pWriteCount the number of real written bytes (Bytes Written). NULL if unused.
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount)
{
	EG_UNUSED(pDriver);
	*pWriteCount = fwrite(pBuffer, 1, Count, pFile);
	return (int32_t)(*pWriteCount) < 0 ? EG_FS_RES_UNKNOWN : EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable. (opened with fs_open )
 * @param Pos the new position of read write pointer
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t Pos, EG_FS_Seek_e Mode)
{
	EG_UNUSED(pDriver);
	fseek(pFile, Pos, Mode);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Give the position of the read write pointer
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable.
 * @param pPos pointer to to store the result
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos)
{
	EG_UNUSED(pDriver);
	*pPos = ftell(pFile);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize a 'DIR' or 'HANDLE' variable for directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath pPath to a directory
 * @return pointer to an initialized 'DIR' or 'HANDLE' variable
 */
static void *FS_OpenDir(EGFileDriver *pDriver, const char *pPath)
{
EG_UNUSED(pDriver);
char pBuffer[MAX_PATH_LEN];

	dir_handle_t *handle = (dir_handle_t *)EG_AllocMem(sizeof(dir_handle_t));
	eg_snprintf(pBuffer, sizeof(pBuffer), EG_FS_STDIO_PATH "%s", pPath);
#ifndef WIN32
	/*Make the pPath relative to the current directory (the projects root folder)*/
	handle->pDir = opendir(pBuffer);
	if(handle->pDir == NULL) {
		EG_FreeMem(handle);
		return NULL;
	}
	return handle;
#else
	handle->pDir = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA fdata;
	strcpy(handle->next_fn, "");
	handle->pDir = FindFirstFileA(pBuffer, &fdata);
	do {
		if(strcmp(fdata.cFileName, ".") == 0 || strcmp(fdata.cFileName, "..") == 0) {
			continue;
		}
		else {
			if(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				eg_snprintf(handle->next_fn, sizeof(handle->next_fn), "/%s", fdata.cFileName);
			}
			else {
				eg_snprintf(handle->next_fn, sizeof(handle->next_fn), "%s", fdata.cFileName);
			}
			break;
		}
	} while(FindNextFileA(handle->pDir, &fdata));

	if(handle->pDir == INVALID_HANDLE_VALUE) {
		EG_FreeMem(handle);
		return INVALID_HANDLE_VALUE;
	}
	return handle;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' or 'HANDLE' variable
 * @param pFileName pointer to a buffer to store the filename
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName)
{
EG_UNUSED(pDriver);

	dir_handle_t *handle = (dir_handle_t *)pDir;
#ifndef WIN32
	struct dirent *entry;
	do {
		entry = readdir(handle->pDir);
		if(entry) {
			if(entry->d_type == DT_DIR)
				eg_snprintf(pFileName, MAX_PATH_LEN, "/%s", entry->d_name);
			else
				strcpy(pFileName, entry->d_name);
		}
		else {
			strcpy(pFileName, "");
		}
	} while(strcmp(pFileName, "/.") == 0 || strcmp(pFileName, "/..") == 0);
#else
	strcpy(pFileName, handle->next_fn);

	strcpy(handle->next_fn, "");
	WIN32_FIND_DATAA fdata;

	if(FindNextFileA(handle->pDir, &fdata) == false) return EG_FS_RES_OK;
	do {
		if(strcmp(fdata.cFileName, ".") == 0 || strcmp(fdata.cFileName, "..") == 0) {
			continue;
		}
		else {
			if(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				eg_snprintf(handle->next_fn, sizeof(handle->next_fn), "/%s", fdata.cFileName);
			}
			else {
				eg_snprintf(handle->next_fn, sizeof(handle->next_fn), "%s", fdata.cFileName);
			}
			break;
		}
	} while(FindNextFileA(handle->pDir, &fdata));

#endif
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close the directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' or 'HANDLE' variable
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir)
{
	EG_UNUSED(pDriver);
	dir_handle_t *handle = (dir_handle_t *)pDir;
#ifndef WIN32
	closedir(handle->pDir);
#else
	FindClose(handle->pDir);
#endif
	EG_FreeMem(handle);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

#else /*EG_USE_FS_STDIO == 0*/

#if defined(EG_FS_STDIO_LETTER) && EG_FS_STDIO_LETTER != '\0'
#warning "EG_USE_FS_STDIO is not enabled but EG_FS_STDIO_LETTER is set"
#endif

#endif /*EG_USE_FS_POSIX*/
