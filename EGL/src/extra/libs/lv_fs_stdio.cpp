/**
 * @file lv_fs_stdio.c
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
	HANDLE dir_p;
	char next_fn[MAX_PATH_LEN];
#else
	DIR *dir_p;
#endif
} dir_handle_t;

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
void lv_fs_stdio_init(void)
{
	/*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

	/*Add a simple drive to open images*/
	static EG_FS_Driver_t fs_drv; /*A driver descriptor*/
	lv_fs_drv_init(&fs_drv);

	/*Set up fields...*/
	fs_drv.letter = EG_FS_STDIO_LETTER;
	fs_drv.cache_size = EG_FS_STDIO_CACHE_SIZE;

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
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to FIL struct or NULL in case of fail
 */
static void *fs_open(EG_FS_Driver_t *drv, const char *path, EG_FS_Mode_e mode)
{
	EG_UNUSED(drv);

	const char *flags = "";

	if(mode == EG_FS_MODE_WR)
		flags = "wb";
	else if(mode == EG_FS_MODE_RD)
		flags = "rb";
	else if(mode == (EG_FS_MODE_WR | EG_FS_MODE_RD))
		flags = "rb+";

	/*Make the path relative to the current directory (the projects root folder)*/

	char buf[MAX_PATH_LEN];
	eg_snprintf(buf, sizeof(buf), EG_FS_STDIO_PATH "%s", path);

	return fopen(buf, flags);
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
	fclose(file_p);
	return EG_FS_RES_OK;
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
	*br = fread(buf, 1, btr, file_p);
	return (int32_t)(*br) < 0 ? EG_FS_RES_UNKNOWN : EG_FS_RES_OK;
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
	*bw = fwrite(buf, 1, btw, file_p);
	return (int32_t)(*bw) < 0 ? EG_FS_RES_UNKNOWN : EG_FS_RES_OK;
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
	fseek(file_p, pos, whence);
	return EG_FS_RES_OK;
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
	*pos_p = ftell(file_p);
	return EG_FS_RES_OK;
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
#ifndef WIN32
	/*Make the path relative to the current directory (the projects root folder)*/
	char buf[MAX_PATH_LEN];
	eg_snprintf(buf, sizeof(buf), EG_FS_STDIO_PATH "%s", path);
	handle->dir_p = opendir(buf);
	if(handle->dir_p == NULL) {
		EG_FreeMem(handle);
		return NULL;
	}
	return handle;
#else
	handle->dir_p = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA fdata;

	/*Make the path relative to the current directory (the projects root folder)*/
	char buf[MAX_PATH_LEN];
	eg_snprintf(buf, sizeof(buf), EG_FS_STDIO_PATH "%s\\*", path);

	strcpy(handle->next_fn, "");
	handle->dir_p = FindFirstFileA(buf, &fdata);
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
	} while(FindNextFileA(handle->dir_p, &fdata));

	if(handle->dir_p == INVALID_HANDLE_VALUE) {
		EG_FreeMem(handle);
		return INVALID_HANDLE_VALUE;
	}
	return handle;
#endif
}

/**
 * Read the next filename form a directory.
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
#ifndef WIN32
	struct dirent *entry;
	do {
		entry = readdir(handle->dir_p);
		if(entry) {
			if(entry->d_type == DT_DIR)
				eg_snprintf(fn, MAX_PATH_LEN, "/%s", entry->d_name);
			else
				strcpy(fn, entry->d_name);
		}
		else {
			strcpy(fn, "");
		}
	} while(strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);
#else
	strcpy(fn, handle->next_fn);

	strcpy(handle->next_fn, "");
	WIN32_FIND_DATAA fdata;

	if(FindNextFileA(handle->dir_p, &fdata) == false) return EG_FS_RES_OK;
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
	} while(FindNextFileA(handle->dir_p, &fdata));

#endif
	return EG_FS_RES_OK;
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
#ifndef WIN32
	closedir(handle->dir_p);
#else
	FindClose(handle->dir_p);
#endif
	EG_FreeMem(handle);
	return EG_FS_RES_OK;
}

#else /*EG_USE_FS_STDIO == 0*/

#if defined(EG_FS_STDIO_LETTER) && EG_FS_STDIO_LETTER != '\0'
#warning "EG_USE_FS_STDIO is not enabled but EG_FS_STDIO_LETTER is set"
#endif

#endif /*EG_USE_FS_POSIX*/
