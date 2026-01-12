/**
 * @file lv_fs_win32.c
 *
 */

#include "EGL.h"
#if EG_USE_FS_WIN32 != '\0'

#include <windows.h>
#include <stdio.h>

#define MAX_PATH_LEN 256

typedef struct {
	HANDLE dir_p;
	char next_fn[MAX_PATH_LEN];
	EG_FSResult_e next_error;
} dir_handle_t;

static bool is_dots_name(const char *name);
static EG_FSResult_e fs_error_from_win32(DWORD error);
static void *fs_open(EG_FS_Driver_t *drv, const char *path, EG_FS_Mode_e mode);
static EG_FSResult_e fs_close(EG_FS_Driver_t *drv, void *file_p);
static EG_FSResult_e fs_read(EG_FS_Driver_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static EG_FSResult_e fs_write(EG_FS_Driver_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static EG_FSResult_e fs_seek(EG_FS_Driver_t *drv, void *file_p, uint32_t pos, EG_FS_Seek_e whence);
static EG_FSResult_e fs_tell(EG_FS_Driver_t *drv, void *file_p, uint32_t *pos_p);
static void *fs_dir_open(EG_FS_Driver_t *drv, const char *path);
static EG_FSResult_e fs_dir_read(EG_FS_Driver_t *drv, void *dir_p, char *fn);
static EG_FSResult_e fs_dir_close(EG_FS_Driver_t *drv, void *dir_p);


/**
 * Register a driver for the File system interface
 */
void lv_fs_win32_init(void)
{
	/*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

	/*Add a simple driver to open images*/
	static EG_FS_Driver_t fs_drv; /*A driver descriptor*/
	lv_fs_drv_init(&fs_drv);

	/*Set up fields...*/
	fs_drv.letter = EG_FS_WIN32_LETTER;
	fs_drv.cache_size = EG_FS_WIN32_CACHE_SIZE;

	fs_drv.open_cb = fs_open;
	fs_drv.close_cb = fs_close;
	fs_drv.read_cb = fs_read;
	fs_drv.write_cb = fs_write;
	fs_drv.seek_cb = fs_seek;
	fs_drv.tell_cb = fs_tell;

	fs_drv.dir_close_cb = fs_dir_close;
	fs_drv.dir_open_cb = fs_dir_open;
	fs_drv.dir_read_cb = fs_dir_read;

	lv_fs_drv_register(&fs_drv);
}

/**
 * Check the dots name
 * @param name file or dir name
 * @return true if the name is dots name
 */
static bool is_dots_name(const char *name)
{
	return name[0] == '.' && (!name[1] || (name[1] == '.' && !name[2]));
}

/**
 * Convert Win32 error code to error from EG_FSResult_e enum
 * @param error Win32 error code
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_error_from_win32(DWORD error)
{
	EG_FSResult_e res;

	switch(error) {
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

/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to FIL struct or NULL in case of fail
 */
static void *fs_open(EG_FS_Driver_t *drv, const char *path, EG_FS_Mode_e mode)
{
	EG_UNUSED(drv);

	DWORD desired_access = 0;

	if(mode & EG_FS_MODE_RD) {
		desired_access |= GENERIC_READ;
	}

	if(mode & EG_FS_MODE_WR) {
		desired_access |= GENERIC_WRITE;
	}

	/*Make the path relative to the current directory (the projects root folder)*/

	char buf[MAX_PATH];
	eg_snprintf(buf, sizeof(buf), EG_FS_WIN32_PATH "%s", path);

	return (void *)CreateFileA(
		buf,
		desired_access,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
}

/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FILE variable. (opened with fs_open)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_close(EG_FS_Driver_t *drv, void *file_p)
{
	EG_UNUSED(drv);
	return CloseHandle((HANDLE)file_p) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FILE variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_read(EG_FS_Driver_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
	EG_UNUSED(drv);
	return ReadFile((HANDLE)file_p, buf, btr, (LPDWORD)br, NULL) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/**
 * Write into a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FILE variable
 * @param buf pointer to a buffer with the bytes to write
 * @param btw Bytes To Write
 * @param bw the number of real written bytes (Bytes Written). NULL if unused.
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_write(EG_FS_Driver_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
	EG_UNUSED(drv);
	return WriteFile((HANDLE)file_p, buf, btw, (LPDWORD)bw, NULL) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FILE variable. (opened with fs_open )
 * @param pos the new position of read write pointer
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_seek(EG_FS_Driver_t *drv, void *file_p, uint32_t pos, EG_FS_Seek_e whence)
{
	EG_UNUSED(drv);

	DWORD move_method = (DWORD)-1;
	if(whence == EG_FS_SEEK_SET) {
		move_method = FILE_BEGIN;
	}
	else if(whence == EG_FS_SEEK_CUR) {
		move_method = FILE_CURRENT;
	}
	else if(whence == EG_FS_SEEK_END) {
		move_method = FILE_END;
	}

	LARGE_INTEGER distance_to_move;
	distance_to_move.QuadPart = pos;
	return SetFilePointerEx((HANDLE)file_p, distance_to_move, NULL, move_method) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
}

/**
 * Give the position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FILE variable.
 * @param pos_p pointer to to store the result
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_tell(EG_FS_Driver_t *drv, void *file_p, uint32_t *pos_p)
{
	EG_UNUSED(drv);

	if(!pos_p) {
		return EG_FS_RES_INV_PARAM;
	}

	LARGE_INTEGER file_pointer;
	file_pointer.QuadPart = 0;

	LARGE_INTEGER distance_to_move;
	distance_to_move.QuadPart = 0;
	if(SetFilePointerEx(
			 (HANDLE)file_p,
			 distance_to_move,
			 &file_pointer,
			 FILE_CURRENT)) {
		if(file_pointer.QuadPart > LONG_MAX) {
			return EG_FS_RES_INV_PARAM;
		}
		else {
			*pos_p = file_pointer.LowPart;
			return EG_FS_RES_OK;
		}
	}
	else {
		return fs_error_from_win32(GetLastError());
	}
}

/**
 * Initialize a 'DIR' or 'HANDLE' variable for directory reading
 * @param drv pointer to a driver where this function belongs
 * @param path path to a directory
 * @return pointer to an initialized 'DIR' or 'HANDLE' variable
 */
static void *fs_dir_open(EG_FS_Driver_t *drv, const char *path)
{
	EG_UNUSED(drv);
	dir_handle_t *handle = (dir_handle_t *)EG_AllocMem(sizeof(dir_handle_t));
	handle->dir_p = INVALID_HANDLE_VALUE;
	handle->next_error = EG_FS_RES_OK;
	WIN32_FIND_DATAA fdata;

	/*Make the path relative to the current directory (the projects root folder)*/
	char buf[MAX_PATH_LEN];
#ifdef EG_FS_WIN32_PATH
	eg_snprintf(buf, sizeof(buf), EG_FS_WIN32_PATH "%s\\*", path);
#else
	eg_snprintf(buf, sizeof(buf), "%s\\*", path);
#endif

	strcpy(handle->next_fn, "");
	handle->dir_p = FindFirstFileA(buf, &fdata);
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
	} while(FindNextFileA(handle->dir_p, &fdata));

	if(handle->dir_p == INVALID_HANDLE_VALUE) {
		EG_FreeMem(handle);
		handle->next_error = fs_error_from_win32(GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	else {
		handle->next_error = EG_FS_RES_OK;
		return handle;
	}
}

/**
 * Read the next filename from a directory.
 * The name of the directories will begin with '/'
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'DIR' or 'HANDLE' variable
 * @param fn pointer to a buffer to store the filename
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_dir_read(EG_FS_Driver_t *drv, void *dir_p, char *fn)
{
	EG_UNUSED(drv);
	dir_handle_t *handle = (dir_handle_t *)dir_p;
	strcpy(fn, handle->next_fn);
	EG_FSResult_e current_error = handle->next_error;
	strcpy(handle->next_fn, "");

	WIN32_FIND_DATAA fdata;

	while(FindNextFileA(handle->dir_p, &fdata)) {
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

/**
 * Close the directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'DIR' or 'HANDLE' variable
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_dir_close(EG_FS_Driver_t *drv, void *dir_p)
{
	EG_UNUSED(drv);
	dir_handle_t *handle = (dir_handle_t *)dir_p;
	EG_FSResult_e res = FindClose(handle->dir_p) ? EG_FS_RES_OK : fs_error_from_win32(GetLastError());
	EG_FreeMem(handle);
	return res;
}

#else /*EG_USE_FS_WIN32 == 0*/

#if defined(EG_FS_WIN32_LETTER) && EG_FS_WIN32_LETTER != '\0'
#warning "EG_USE_FS_WIN32 is not enabled but EG_FS_WIN32_LETTER is set"
#endif

#endif
