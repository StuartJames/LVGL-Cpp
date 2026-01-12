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

#if EG_USE_FS_POSIX

#include <fcntl.h>
#include <stdio.h>
#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#if EG_FS_POSIX_LETTER == '\0'
#error "EG_FS_POSIX_LETTER must be an upper case ASCII letter"
#endif

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
static char next_fn[256];
#endif

static void *FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode);
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile);
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount);
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount);
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t pos, EG_FS_Seek_e Mode);
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos);
static void *FS_OpenDir(EGFileDriver *pDriver, const char *pPath);
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName);
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir);

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Register a driver for the File system interface
 */
void EG_Initialise_Posix(void)
{
	static EGFileDriver FSDriver;  // A driver class
	/*Set up fields...*/
	FSDriver.m_DriverID = EG_FS_POSIX_LETTER;
	FSDriver.m_CacheSize = EG_FS_POSIX_CACHE_SIZE;
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
 * @return a file handle or -1 in case of fail
 */
static void *FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode)
{
EG_UNUSED(pDriver);
uint32_t flags = 0;

	if(Mode == EG_FS_MODE_WR)	flags = O_WRONLY | O_CREAT;
	else if(Mode == EG_FS_MODE_RD) flags = O_RDONLY;
	else if(Mode == (EG_FS_MODE_WR | EG_FS_MODE_RD)) flags = O_RDWR | O_CREAT;
	// Make the pPath relative to the current directory (the projects root folder)
	char pBuffer[256];
	eg_snprintf(pBuffer, sizeof(pBuffer), EG_FS_POSIX_PATH "%s", pPath);
	int f = open(pBuffer, flags, 0666);
	if(f < 0) return NULL;
	return (void *)(eg_uintptr_t)f;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile a file handle. (opened with fs_open)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile)
{
	EG_UNUSED(pDriver);
	close((eg_uintptr_t)pFile);
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read data from an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile a file handle variable.
 * @param pBuffer pointer to a memory block where to store the read data
 * @param Count number of Bytes To Read
 * @param pReadCount the real number of read bytes (Byte Read)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount)
{
	EG_UNUSED(pDriver);
	*pReadCount = read((eg_uintptr_t)pFile, pBuffer, Count);
	return (int32_t)(*pReadCount) < 0 ? EG_FS_RES_UNKNOWN : EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Write into a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile a file handle variable
 * @param pBuffer pointer to a buffer with the bytes to write
 * @param Count Bytes To Write
 * @param pWriteCount the number of real written bytes (Bytes Written). NULL if unused.
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount)
{
	EG_UNUSED(pDriver);
	*pWriteCount = write((eg_uintptr_t)pFile, pBuffer, Count);
	return (int32_t)(*pWriteCount) < 0 ? EG_FS_RES_UNKNOWN : EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile a file handle variable. (opened with fs_open )
 * @param pos the new position of read write pointer
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t pos, EG_FS_Seek_e Mode)
{
	EG_UNUSED(pDriver);
	off_t offset = lseek((eg_uintptr_t)pFile, pos, Mode);
	return offset < 0 ? EG_FS_RES_FS_ERR : EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Give the position of the read write pointer
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile a file handle variable.
 * @param pPos pointer to to store the result
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos)
{
	EG_UNUSED(pDriver);
	off_t offset = lseek((eg_uintptr_t)pFile, 0, SEEK_CUR);
	*pPos = offset;
	return offset < 0 ? EG_FS_RES_FS_ERR : EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize a 'fs_read_dir_t' variable for directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath pPath to a directory
 * @return pointer to an initialized 'DIR' or 'HANDLE' variable
 */
static void *FS_OpenDir(EGFileDriver *pDriver, const char *pPath)
{
EG_UNUSED(pDriver);
char pBuffer[256];

	// Make the pPath relative to the current directory (the projects root folder)
	eg_snprintf(pBuffer, sizeof(pBuffer), EG_FS_POSIX_PATH "%s", pPath);
#ifndef WIN32
	return opendir(pBuffer);
#else
	HANDLE d = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;

	strcpy(next_fn, "");
	d = FindFirstFile(pBuffer, &fdata);
	do {
		if(strcmp(fdata.cFileName, ".") == 0 || strcmp(fdata.cFileName, "..") == 0) {
			continue;
		}
		else {
			if(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				sprintf(next_fn, "/%s", fdata.cFileName);
			}
			else {
				sprintf(next_fn, "%s", fdata.cFileName);
			}
			break;
		}
	} 
  while(FindNextFileA(d, &fdata));
	return d;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the next filename from a directory.
 * The name of the directories will begin with '/'
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' or 'HANDLE' variable
 * @param pFileName pointer to a buffer to store the filename
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName)
{
	EG_UNUSED(pDriver);

#ifndef WIN32
	struct dirent *pEntry;
	do{
		pEntry = readdir(pDir);
		if(pEntry) {
			if(pEntry->d_type == DT_DIR) sprintf(pFileName, "/%s", pEntry->d_name);
			else strcpy(pFileName, pEntry->d_name);
		}
		else strcpy(pFileName, "");
	}
  while(strcmp(pFileName, "/.") == 0 || strcmp(pFileName, "/..") == 0);
#else
	strcpy(pFileName, next_fn);
	strcpy(next_fn, "");
	WIN32_FIND_DATA fdata;
	if(FindNextFile(pDir, &fdata) == false) return EG_FS_RES_OK;
	do {
		if(strcmp(fdata.cFileName, ".") == 0 || strcmp(fdata.cFileName, "..") == 0) {
			continue;
		}
		else {
			if(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				sprintf(next_fn, "/%s", fdata.cFileName);
			}
			else {
				sprintf(next_fn, "%s", fdata.cFileName);
			}
			break;
		}
	}
  while(FindNextFile(pDir, &fdata));

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
#ifndef WIN32
	closedir(pDir);
#else
	FindClose(pDir);
#endif
	return EG_FS_RES_OK;
}
#else /*EG_USE_FS_POSIX == 0*/

#if defined(EG_FS_POSIX_LETTER) && EG_FS_POSIX_LETTER != '\0'
#warning "EG_USE_FS_POSIX is not enabled but EG_FS_POSIX_LETTER is set"
#endif

#endif /*EG_USE_FS_POSIX*/
