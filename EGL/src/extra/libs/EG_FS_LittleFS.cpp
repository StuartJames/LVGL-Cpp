/**
 * @file lv_fs_littlefs.c
 *
 */

#include "EGL.h"

#if EG_USE_FS_LITTLEFS
#include "lfs.h"

#if EG_FS_LITTLEFS_LETTER == '\0'
#error "EG_FS_LITTLEFS_LETTER must be an upper case ASCII letter"
#endif

/////////////////////////////////////////////////////////////////////////////////////////

static void EG_InitialiseFS(void);

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

void EG_FSInitialise_LittleFS(void)
{
	EG_InitialiseFS();
	static EGFileDriver FSDriver; // A driver class
	FSDriver.m_DriverID = EG_FS_LITTLEFS_LETTER;
	FSDriver.m_CacheSize = EG_FS_LITTLEFS_CACHE_SIZE;
	FSDriver.OpenCB = FS_Open;
	FSDriver.CloseCB = FS_Close;
	FSDriver.ReadCB = FS_Read;
	FSDriver.WriteCB = FS_Write;
	FSDriver.SeekCB = FS_Seek;
	FSDriver.TellCB = FS_Tell;
	FSDriver.DirOpenCB = FS_OpenDir;
	FSDriver.DirCloseCB = FS_CloseDir;
	FSDriver.DirReadCB = FS_ReadDir;
	FSDriver.m_Param = nullptr;
	EGFileSystem::Register(&FSDriver);
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Convenience function to attach registered driver to lfs_t structure by driver-label
 * @param label     the label assigned to the driver when it was registered
 * @param lfs_p     the pointer to the lfs_t structure initialized by external code/library
 * @return          pointer to a driver descriptor or NULL on error
 */
EGFileDriver* EG_LittleFSSetDriver(char label, void *lfs_p)
{
	EGFileDriver *pDriver = lv_fs_get_drv(label);
	if(drv_p != NULL) pDriver->m_Param = (lfs_t *)lfs_p;
	return drv_p;
}

/////////////////////////////////////////////////////////////////////////////////////////

/*Initialize your Storage device and File system.*/
static void EG_InitialiseFS(void)
{
	/* Initialize the internal flash or SD-card and LittleFS itself.
     * Better to do it in your code to keep this library untouched for easy updating */
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Open a file
 * @param pDriver       pointer to a driver where this function belongs
 * @param pPath      pPath to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param Mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          pointer to a file descriptor or NULL on error
 */
static void* FS_Open(EGFileDriver *pDriver, const char *pPath, EG_FS_Mode_e Mode)
{
lfs_t *lfs_p = pDriver->m_Param;
uint32_t flags = 0;

	flags = Mode == EG_FS_MODE_RD ? LFS_O_RDONLY : Mode == EG_FS_MODE_WR ? LFS_O_WRONLY : Mode == (EG_FS_MODE_WR | EG_FS_MODE_RD) ? LFS_O_RDWR : 0;
	lfs_file_t *pFile = EG_AllocMem(sizeof(lfs_file_t));
	if(pFile == NULL) return NULL;
	int result = lfs_file_open(lfs_p, pFile, pPath, flags);
	if(result != LFS_ERR_OK) {
		EG_FreeMem(pFile);
		return NULL;
	}
	return pFile;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close an opened file
 * @param pDriver       pointer to a driver where this function belongs
 * @param pFile    pointer to a file_t variable. (opened with fs_open)
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_Close(EGFileDriver *pDriver, void *pFile)
{
	lfs_t *lfs_p = pDriver->m_Param;

	int result = lfs_file_close(lfs_p, pFile);
	EG_FreeMem(pFile);
	/*EG_FreeMem( lfs_p );*/ /*allocated and freed by outside-code*/

	if(result != LFS_ERR_OK) return EG_FS_RES_UNKNOWN;
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read data from an opened file
 * @param pDriver       pointer to a driver where this function belongs
 * @param pFile    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_Read(EGFileDriver *pDriver, void *pFile, void *buf, uint32_t btr, uint32_t *br)
{
	lfs_t *lfs_p = pDriver->m_Param;

	lfs_ssize_t result = lfs_file_read(lfs_p, pFile, buf, btr);
	if(result < 0) return EG_FS_RES_UNKNOWN;

	*br = (uint32_t)result;
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Write into a file
 * @param pDriver       pointer to a driver where this function belongs
 * @param pFile    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_Write(EGFileDriver *pDriver, void *pFile, const void *buf, uint32_t btw, uint32_t *bw)
{
#ifndef LFS_READONLY
	lfs_t *lfs_p = pDriver->m_Param;

	lfs_ssize_t result = lfs_file_write(lfs_p, pFile, buf, btw);
	if(result < 0 || lfs_file_sync(lfs_p, pFile) < 0) return EG_FS_RES_UNKNOWN;

	*bw = (uint32_t)result;
	return EG_FS_RES_OK;
#else
	return EG_FS_RES_NOT_IMP;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param pDriver       pointer to a driver where this function belongs
 * @param pFile    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @EG_FS_Seek_e
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_Seek(EGFileDriver *pDriver, void *pFile, uint32_t pos, EG_FS_Seek_e whence)
{
	lfs_t *lfs_p = pDriver->m_Param;

	int lfs_whence = whence == EG_FS_SEEK_SET ? LFS_SEEK_SET : whence == EG_FS_SEEK_CUR ? LFS_SEEK_CUR : whence == EG_FS_SEEK_END ? LFS_SEEK_END : 0;

	lfs_soff_t result = lfs_file_seek(lfs_p, pFile, pos, lfs_whence);
	if(result < 0) return EG_FS_RES_UNKNOWN;

	/*pos = result;*/ /*not supported by lv_fs*/
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Give the position of the read write pointer
 * @param pDriver       pointer to a driver where this function belongs
 * @param pFile    pointer to a file_t variable.
 * @param pos_p     pointer to where to store the result
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_Tell(EGFileDriver *pDriver, void *pFile, uint32_t *pos_p)
{
	lfs_t *lfs_p = pDriver->m_Param;

	lfs_soff_t result = lfs_file_tell(lfs_p, pFile);
	if(result < 0) return EG_FS_RES_UNKNOWN;

	*pos_p = (uint32_t)result;
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize a 'EG_FS_Dir_t' variable for directory reading
 * @param pDriver       pointer to a driver where this function belongs
 * @param pPath      pPath to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void* FS_OpenDir(EGFileDriver *pDriver, const char *pPath)
{
	lfs_t *lfs_p = pDriver->m_Param;

	lfs_dir_t *pDir = EG_AllocMem(sizeof(lfs_dir_t));
	if(pDir == NULL) return NULL;

	int result = lfs_dir_open(lfs_p, pDir, pPath);
	if(result != LFS_ERR_OK) {
		EG_FreeMem(pDir);
		return NULL;
	}

	return pDir;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param pDriver       pointer to a driver where this function belongs
 * @param pDir   pointer to an initialized 'EG_FS_Dir_t' variable
 * @param pFileName        pointer to a buffer to store the filename
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_ReadDir(EGFileDriver *pDriver, void *pDir, char *pFileName)
{
	struct lfs_info info;
	int result;
	lfs_t *lfs_p = pDriver->m_Param;

	info.name[0] = '\0';
	do {
		result = lfs_dir_read(lfs_p, pDir, &info);
		if(result > 0) {
			if(info.type == LFS_TYPE_DIR) {
				pFileName[0] = '/';
				strcpy(&pFileName[1], info.name);
			}
			else strcpy(pFileName, info.name);
		}
		else if(result == 0) pFileName[0] = '\0'; /*dir-scan ended*/
		else return EG_FS_RES_UNKNOWN;
	} while(!strcmp(pFileName, "/.") || !strcmp(pFileName, "/.."));

	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close the directory reading
 * @param pDriver       pointer to a driver where this function belongs
 * @param pDir   pointer to an initialized 'EG_FS_Dir_t' variable
 * @return          EG_FS_RES_OK: no error or  any error from @EG_FSResult_e enum
 */
static EG_FSResult_e FS_CloseDir(EGFileDriver *pDriver, void *pDir)
{
	lfs_t *lfs_p = pDriver->m_Param;
	int result = lfs_dir_close(lfs_p, pDir);
	EG_FreeMem(pDir);
	if(result != LFS_ERR_OK) return EG_FS_RES_UNKNOWN;
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

#else /*EG_USE_FS_LITTLEFS == 0*/

#if defined(EG_FS_LITTLEFS_LETTER) && EG_FS_LITTLEFS_LETTER != '\0'
#warning "EG_USE_FS_LITTLEFS is not enabled but EG_FS_LITTLEFS_LETTER is set"
#endif

#endif /*EG_USE_FS_POSIX*/
