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

static void fs_init(void);

static void*          fs_open(EGFileDriver *Driver, const char *pPath, EG_FS_Mode_e Mode);
static EG_FSResult_e  fs_close(EGFileDriver *Driver, void * pFile);
static EG_FSResult_e  fs_read(EGFileDriver *Driver, void * pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount);
static EG_FSResult_e  fs_write(EGFileDriver *Driver, void * pFile, const void * buf, uint32_t Count, uint32_t *pWriteCount);
static EG_FSResult_e  fs_seek(EGFileDriver *Driver, void * pFile, uint32_t Pos, EG_FS_Seek_e Mode);
static EG_FSResult_e  fs_tell(EGFileDriver *Driver, void * pFile, uint32_t *pPos);
static void*          fs_dir_open(EGFileDriver *Driver, const char *pPath);
static EG_FSResult_e  fs_dir_read(EGFileDriver *Driver, void *pDir, char *pFileName);
static EG_FSResult_e  fs_dir_close(EGFileDriver *Driver, void *pDir);

/////////////////////////////////////////////////////////////////////////////////////////

void lv_fs_fatfs_init(void)
{
    fs_init();    // Initialize your storage device and File System
    static EGFileDriver fs_drv; /*A driver descriptor*/
    fs_drv.m_DriverID = EG_FS_FATFS_LETTER;
    fs_drv.m_CacheSize = EG_FS_FATFS_CACHE_SIZE;
    fs_drv.OpenCB = fs_open;
    fs_drv.CloseCB = fs_close;
    fs_drv.ReadCB = fs_read;
    fs_drv.WriteCB = fs_write;
    fs_drv.SeekCB = fs_seek;
    fs_drv.TellCB = fs_tell;
    fs_drv.DirCloseCB = fs_dir_close;
    fs_drv.DirOpenCB = fs_dir_open;
    fs_drv.DirReadCB = fs_dir_read;
    EGFile::Register(&fs_drv);
}

/////////////////////////////////////////////////////////////////////////////////////////

/*Initialize your Storage device and File system.*/
static void fs_init(void)
{
    /*Initialize the SD card and FatFS itself.
     *Better to do it in your code to keep this library untouched for easy updating*/
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return pointer to FIL struct or NULL in case of fail
 */
static void * fs_open(EGFileDriver *Driver, const char *pPath, EG_FS_Mode_e Mode)
{
    EG_UNUSED(drv);
    uint8_t flags = 0;

    if(mode == EG_FS_MODE_WR) flags = FA_WRITE | FA_OPEN_ALWAYS;
    else if(mode == EG_FS_MODE_RD) flags = FA_READ;
    else if(mode == (EG_FS_MODE_WR | EG_FS_MODE_RD)) flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;

    FIL * f = EG_AllocMem(sizeof(FIL));
    if(f == NULL) return NULL;

    FRESULT res = f_open(f, path, flags);
    if(res == FR_OK) {
        return f;
    }
    else {
        EG_FreeMem(f);
        return NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FIL variable. (opened with fs_open)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_close(EGFileDriver *Driver, void * pFile)
{
    EG_UNUSED(drv);
    f_close(file_p);
    EG_FreeMem(file_p);
    return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FIL variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_read(EGFileDriver *Driver, void * pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount)
{
    EG_UNUSED(drv);
    FRESULT res = f_read(file_p, buf, btr, (UINT *)br);
    if(res == FR_OK) return EG_FS_RES_OK;
    else return EG_FS_RES_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Write into a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FIL variable
 * @param buf pointer to a buffer with the bytes to write
 * @param btw Bytes To Write
 * @param bw the number of real written bytes (Bytes Written). NULL if unused.
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_write(EGFileDriver *Driver, void * pFile, const void * buf, uint32_t Count, uint32_t *pWriteCount)
{
    EG_UNUSED(drv);
    FRESULT res = f_write(file_p, buf, btw, (UINT *)bw);
    if(res == FR_OK) return EG_FS_RES_OK;
    else return EG_FS_RES_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FIL variable. (opened with fs_open )
 * @param pos the new position of read write pointer
 * @param whence only LV_SEEK_SET is supported
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_seek(EGFileDriver *Driver, void * pFile, uint32_t Pos, EG_FS_Seek_e Mode)
{
    EG_UNUSED(drv);
    switch(whence) {
        case EG_FS_SEEK_SET:
            f_lseek(file_p, pos);
            break;
        case EG_FS_SEEK_CUR:
            f_lseek(file_p, f_tell((FIL *)file_p) + pos);
            break;
        case EG_FS_SEEK_END:
            f_lseek(file_p, f_size((FIL *)file_p) + pos);
            break;
        default:
            break;
    }
    return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Give the position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a FIL variable.
 * @param pos_p pointer to to store the result
 * @return EG_FS_RES_OK: no error, the file is read
 *         any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_tell(EGFileDriver *Driver, void * pFile, uint32_t *pPos)
{
    EG_UNUSED(drv);
    *pos_p = f_tell((FIL *)file_p);
    return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize a 'DIR' variable for directory reading
 * @param drv pointer to a driver where this function belongs
 * @param path path to a directory
 * @return pointer to an initialized 'DIR' variable
 */
static void * fs_dir_open(EGFileDriver *Driver, const char *pPath)
{
    EG_UNUSED(drv);
    DIR * d = EG_AllocMem(sizeof(DIR));
    if(d == NULL) return NULL;

    FRESULT res = f_opendir(d, path);
    if(res != FR_OK) {
        EG_FreeMem(d);
        d = NULL;
    }
    return d;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Read the next filename from a directory.
 * The name of the directories will begin with '/'
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'DIR' variable
 * @param fn pointer to a buffer to store the filename
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_dir_read(EGFileDriver *Driver, void *pDir, char *pFileName)
{
    EG_UNUSED(drv);
    FRESULT res;
    FILINFO fno;
    fn[0] = '\0';

    do {
        res = f_readdir(dir_p, &fno);
        if(res != FR_OK) return EG_FS_RES_UNKNOWN;

        if(fno.fattrib & AM_DIR) {
            fn[0] = '/';
            strcpy(&fn[1], fno.fname);
        }
        else strcpy(fn, fno.fname);

    } while(strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);

    return EG_FS_RES_OK;
}


/**
 * Close the directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'DIR' variable
 * @return EG_FS_RES_OK or any error from EG_FSResult_e enum
 */
static EG_FSResult_e fs_dir_close(EGFileDriver *Driver, void *pDir)
{
    EG_UNUSED(drv);
    f_closedir(dir_p);
    EG_FreeMem(dir_p);
    return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

#else 

#if defined(EG_FS_FATFS_LETTER) && EG_FS_FATFS_LETTER != '\0'
    #warning "EG_USE_FS_FATFS is not enabled but EG_FS_FATFS_LETTER is set"
#endif

#endif 

