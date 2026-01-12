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
#if EG_USE_FS_WIN32 != '\0'

#include <windows.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////////////

#define MAX_PATH_LEN 256

typedef struct {
	HANDLE pDir;
	char next_fn[MAX_PATH_LEN];
	EG_FSResult_e next_error;
} dir_handle_t;

static bool IsDotsName(const char *pName);
static EG_FSResult_e Win32ErrorToEG(DWORD Error);
static void *OpenCB(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode);
static EG_FSResult_e CloseCB(EGFileDriver *pDriver, void *pFile);
static EG_FSResult_e ReadCB(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount);
static EG_FSResult_e WriteCB(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount);
static EG_FSResult_e SeekCB(EGFileDriver *pDriver, void *pFile, uint32_t pos, EG_FS_Seek_e Mode);
static EG_FSResult_e TellCB(EGFileDriver *pDriver, void *pFile, uint32_t *pPos);
static void *DirOpenCB(EGFileDriver *pDriver, const char *pPath);
static EG_FSResult_e DirReadCB(EGFileDriver *pDriver, void *pDir, char *pFileName);
static EG_FSResult_e DirCloseCB(EGFileDriver *pDriver, void *pDir);

/////////////////////////////////////////////////////////////////////////////////////////

void EG_FSInitialise_Win32(void)
{
	static EGFileDriver FSDriver;  // A driver class
	FSDriver.m_DriverID = EG_FS_WIN32_LETTER;
	FSDriver.m_CacheSize = EG_FS_WIN32_CACHE_SIZE;
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

static bool IsDotsName(const char *pName)
{
	return pName[0] == '.' && (!pName[1] || (pName[1] == '.' && !pName[2]));
}

/////////////////////////////////////////////////////////////////////////////////////////

// Convert Win32 Error code to Error from EG_FSResult_e enum
static EG_FSResult_e Win32ErrorToEG(DWORD Error)
{
	EG_FSResult_e res;

	switch(Error) {
		case ERROR_SUCCESS:
			res = EG_FS_RES_OK;
			break;
		case ERROR_BAD_UNIT:
		case ERROR_NOT_READY:
		case ERROR_CRC:
		case ERROR_SEEK:
		case ERROR_NOT_DOS_DISK:
		case ERROR_WRITE_FAULT:
		case ERROR_READ_FAULT:
		case ERROR_GEN_FAILURE:
		case ERROR_WRONG_DISK:
			res = EG_FS_RES_HW_ERR;
			break;
		case ERROR_INVALID_HANDLE:
		case ERROR_INVALID_TARGET_HANDLE:
			res = EG_FS_RES_FS_ERR;
			break;
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_DRIVE:
		case ERROR_NO_MORE_FILES:
		case ERROR_SECTOR_NOT_FOUND:
		case ERROR_BAD_NETPATH:
		case ERROR_BAD_NET_NAME:
		case ERROR_BAD_PATHNAME:
		case ERROR_FILENAME_EXCED_RANGE:
			res = EG_FS_RES_NOT_EX;
			break;
		case ERROR_DISK_FULL:
			res = EG_FS_RES_FULL;
			break;
		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:
		case ERROR_DRIVE_LOCKED:
			res = EG_FS_RES_LOCKED;
			break;
		case ERROR_ACCESS_DENIED:
		case ERROR_CURRENT_DIRECTORY:
		case ERROR_WRITE_PROTECT:
		case ERROR_NETWORK_ACCESS_DENIED:
		case ERROR_CANNOT_MAKE:
		case ERROR_FAIL_I24:
		case ERROR_SEEK_ON_DEVICE:
		case ERROR_NOT_LOCKED:
		case ERROR_LOCK_FAILED:
			res = EG_FS_RES_DENIED;
			break;
		case ERROR_BUSY:
			res = EG_FS_RES_BUSY;
			break;
		case ERROR_TIMEOUT:
			res = EG_FS_RES_TOUT;
			break;
		case ERROR_NOT_SAME_DEVICE:
		case ERROR_DIRECT_ACCESS_HANDLE:
			res = EG_FS_RES_NOT_IMP;
			break;
		case ERROR_TOO_MANY_OPEN_FILES:
		case ERROR_ARENA_TRASHED:
		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_INVALID_BLOCK:
		case ERROR_OUT_OF_PAPER:
		case ERROR_SHARING_BUFFER_EXCEEDED:
		case ERROR_NOT_ENOUGH_QUOTA:
			res = EG_FS_RES_OUT_OF_MEM;
			break;
		case ERROR_INVALID_FUNCTION:
		case ERROR_INVALID_ACCESS:
		case ERROR_INVALID_DATA:
		case ERROR_BAD_COMMAND:
		case ERROR_BAD_LENGTH:
		case ERROR_INVALID_PARAMETER:
		case ERROR_NEGATIVE_SEEK:
			res = EG_FS_RES_INV_PARAM;
			break;
		default:
			res = EG_FS_RES_UNKNOWN;
			break;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Open a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath pPath to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param Mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to FIL struct or NULL in case of fail
 */
static void* OpenCB(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode)
{
EG_UNUSED(pDriver);

	DWORD desired_access = 0;
	if(Mode & EG_FS_MODE_RD) {
		desired_access |= GENERIC_READ;
	}
	if(Mode & EG_FS_MODE_WR) {
		desired_access |= GENERIC_WRITE;
	}
	// Make the pPath relative to the current directory (the projects root folder)
	char pBuffer[MAX_PATH];
	eg_snprintf(pBuffer, sizeof(pBuffer), EG_FS_WIN32_PATH "%s", pPath);
	return (void *)CreateFileA(
		pBuffer,
		desired_access,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable. (opened with fs_open)
 * @return EG_FS_RES_OK: no Error, the file is read
 *         any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile)
{
EG_UNUSED(pDriver);

	return CloseHandle((HANDLE)pFile) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read data from an opened file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable.
 * @param pBuffer pointer to a memory block where to store the read data
 * @param Count number of Bytes To Read
 * @param pReadCount the real number of read bytes (Byte Read)
 * @return EG_FS_RES_OK: no Error, the file is read
 *         any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount)
{
EG_UNUSED(pDriver);

	return ReadFile((HANDLE)pFile, pBuffer, Count, (LPDWORD)pReadCount, NULL) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Write into a file
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable
 * @param pBuffer pointer to a buffer with the bytes to write
 * @param Count Bytes To Write
 * @param pWriteCount the number of real written bytes (Bytes Written). NULL if unused.
 * @return EG_FS_RES_OK or any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *pBuffer, uint32_t Count, uint32_t *pWriteCount)
{
EG_UNUSED(pDriver);

	return WriteFile((HANDLE)pFile, pBuffer, Count, (LPDWORD)pWriteCount, NULL) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable. (opened with fs_open )
 * @param pos the new position of read write pointer
 * @return EG_FS_RES_OK: no Error, the file is read
 *         any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t pos, EG_FS_Seek_e Mode)
{
EG_UNUSED(pDriver);

	DWORD move_method = (DWORD)-1;
	if(Mode == EG_FS_SEEK_SET) move_method = FILE_BEGIN;
	else if(Mode == EG_FS_SEEK_CUR) move_method = FILE_CURRENT;
	else if(Mode == EG_FS_SEEK_END)	move_method = FILE_END;
	LARGE_INTEGER distance_to_move;
	distance_to_move.QuadPart = pos;
	return SetFilePointerEx((HANDLE)pFile, distance_to_move, NULL, move_method) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Give the position of the read write pointer
 * @param pDriver pointer to a driver where this function belongs
 * @param pFile pointer to a FILE variable.
 * @param pPos pointer to to store the result
 * @return EG_FS_RES_OK: no Error, the file is read
 *         any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pPos)
{
EG_UNUSED(pDriver);

	if(!pPos) return EG_FS_RES_INV_PARAM;
	LARGE_INTEGER file_pointer;
	file_pointer.QuadPart = 0;
	LARGE_INTEGER distance_to_move;
	distance_to_move.QuadPart = 0;
	if(SetFilePointerEx(
			 (HANDLE)pFile,
			 distance_to_move,
			 &file_pointer,
			 FILE_CURRENT)) {
		if(file_pointer.QuadPart > LONG_MAX) {
			return EG_FS_RES_INV_PARAM;
		}
		else {
			*pPos = file_pointer.LowPart;
			return EG_FS_RES_OK;
		}
	}
	return fs_error_from_win32(GetLastError());
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize a 'DIR' or 'HANDLE' variable for directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pPath pPath to a directory
 * @return pointer to an initialized 'DIR' or 'HANDLE' variable
 */
static void* FS_OpenDir(EGFileDriver *pDriver, const char *pPath)
{
EG_UNUSED(pDriver);

	dir_handle_t *handle = (dir_handle_t *)EG_AllocMem(sizeof(dir_handle_t));
	handle->pDir = INVALID_HANDLE_VALUE;
	handle->next_error = EG_FS_RES_OK;
	WIN32_FIND_DATAA fdata;

	/*Make the pPath relative to the current directory (the projects root folder)*/
	char pBuffer[MAX_PATH_LEN];
#ifdef EG_FS_WIN32_PATH
	eg_snprintf(pBuffer, sizeof(pBuffer), EG_FS_WIN32_PATH "%s\\*", pPath);
#else
	eg_snprintf(pBuffer, sizeof(pBuffer), "%s\\*", pPath);
#endif
	strcpy(handle->next_fn, "");
	handle->pDir = FindFirstFileA(pBuffer, &fdata);
	do {
		if(is_dots_name(fdata.cFileName)) {
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
		handle->next_error = fs_error_from_win32(GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	handle->next_error = EG_FS_RES_OK;
	return handle;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the next filename from a directory.
 * The pName of the directories will begin with '/'
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' or 'HANDLE' variable
 * @param pFileName pointer to a buffer to store the filename
 * @return EG_FS_RES_OK or any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName)
{
EG_UNUSED(pDriver);

	dir_handle_t *handle = (dir_handle_t *)pDir;
	strcpy(pFileName, handle->next_fn);
	EG_FSResult_e current_error = handle->next_error;
	strcpy(handle->next_fn, "");
	WIN32_FIND_DATAA fdata;
	while(FindNextFileA(handle->pDir, &fdata)) {
		if(is_dots_name(fdata.cFileName)) {
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
	}
	if(handle->next_fn[0] == '\0') {
		handle->next_error = fs_error_from_win32(GetLastError());
	}
	return current_error;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close the directory reading
 * @param pDriver pointer to a driver where this function belongs
 * @param pDir pointer to an initialized 'DIR' or 'HANDLE' variable
 * @return EG_FS_RES_OK or any Error from EG_FSResult_e enum
 */
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir)
{
	EG_UNUSED(pDriver);
	dir_handle_t *handle = (dir_handle_t *)pDir;
	EG_FSResult_e res = FindClose(handle->pDir) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
	EG_FreeMem(handle);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

#else /*EG_USE_FS_WIN32 == 0*/

#if defined(EG_FS_WIN32_LETTER) && EG_FS_WIN32_LETTER != '\0'
#warning "EG_USE_FS_WIN32 is not enabled but EG_FS_WIN32_LETTER is set"
#endif

#endif
