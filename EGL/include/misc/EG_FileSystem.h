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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

#include <stdint.h>
#include <stdbool.h>
#include "../misc/EG_List.h"

/////////////////////////////////////////////////////////////////////////////////////////

#define EG_FS_MAX_FN_LENGTH 64
#define EG_FS_MAX_PATH_LENGTH 256

// Errors in the file system module.
typedef enum : uint8_t {
    EG_FS_RES_OK = 0,
    EG_FS_RES_HW_ERR,     /*Low level hardware error*/
    EG_FS_RES_FS_ERR,     /*Error in the file system structure*/
    EG_FS_RES_NOT_EX,     /*Driver, file or directory is not exists*/
    EG_FS_RES_FULL,       /*Disk full*/
    EG_FS_RES_LOCKED,     /*The file is already opened*/
    EG_FS_RES_DENIED,     /*Access denied. Check 'fs_open' modes and write protect*/
    EG_FS_RES_BUSY,       /*The file system now can't handle it, try later*/
    EG_FS_RES_TOUT,       /*Process time outed*/
    EG_FS_RES_NOT_IMP,    /*Requested function is not implemented*/
    EG_FS_RES_OUT_OF_MEM, /*Not enough memory for an internal operation*/
    EG_FS_RES_INV_PARAM,  /*Invalid parameter among arguments*/
    EG_FS_RES_UNKNOWN,    /*Other unknown error*/
} EG_FSResult_e;


// File open mode.
typedef enum : uint8_t {
    EG_FS_MODE_WR = 0x01,
    EG_FS_MODE_RD = 0x02,
} EG_FS_Mode_e;


// Seek modes.
typedef enum {
    EG_FS_SEEK_SET = 0x00,      // Set the position from absolutely (from the start of file)
    EG_FS_SEEK_CUR = 0x01,      // Set the position from the current position
    EG_FS_SEEK_END = 0x02,      // Set the position from the end of the file
} EG_FS_Seek_e;


typedef struct {
    uint32_t    Start;
    uint32_t    End;
    uint32_t    FlePosition;
    void       *pBuffer;
} EG_FS_FileCache_t;

/////////////////////////////////////////////////////////////////////////////////////////

class EGFileDriver
{
public:
                            EGFileDriver(void);
                            ~EGFileDriver(void);
  bool                      (*ReadyCB)(EGFileDriver *Driver);
  void *                    (*OpenCB)(EGFileDriver *Driver, const char *pPath, EG_FS_Mode_e Mode);
  EG_FSResult_e             (*CloseCB)(EGFileDriver *Driver, void * pFile);
  EG_FSResult_e             (*ReadCB)(EGFileDriver *Driver, void * pFile, void *pBuffer, uint32_t Count, uint32_t *pReadCount);
  EG_FSResult_e             (*WriteCB)(EGFileDriver *Driver, void * pFile, const void * buf, uint32_t Count, uint32_t *pWriteCount);
  EG_FSResult_e             (*SeekCB)(EGFileDriver *Driver, void * pFile, uint32_t Pos, EG_FS_Seek_e Mode);
  EG_FSResult_e             (*TellCB)(EGFileDriver *Driver, void * pFile, uint32_t *pPos);
  void *                    (*DirOpenCB)(EGFileDriver *Driver, const char *pPath);
  EG_FSResult_e             (*DirReadCB)(EGFileDriver *Driver, void *pDir, char *pFileNames);
  EG_FSResult_e             (*DirCloseCB)(EGFileDriver *Driver, void *pDir);
  void                      operator = (const EGFileDriver &rval);

  char                      m_DriverID;
  uint16_t                  m_CacheSize;
#if EG_USE_USER_DATA
  void                      *m_Param; // Custom file user data
#endif

};
/////////////////////////////////////////////////////////////////////////////////////////

class EGFileSystem
{
public:
                            EGFileSystem(void);
                            ~EGFileSystem(void);

  bool                      IsReady(char DriverID);
  EG_FSResult_e             Open(const char *pPath, EG_FS_Mode_e Mode);
  EG_FSResult_e             Close(void);
  EG_FSResult_e             Read(void *pBuffer, uint32_t Count, uint32_t *pReadCount);
  EG_FSResult_e             Write(const void *pBuffer, uint32_t Count, uint32_t *pWriteCount);
  EG_FSResult_e             Seek(uint32_t Pos, EG_FS_Seek_e Mode);
  EG_FSResult_e             Tell(uint32_t *pPos);
  EG_FSResult_e             DirOpen(const char *pPath);
  EG_FSResult_e             DirRead(char *pFileNames);
  EG_FSResult_e             DirClose(void);
  void                      operator = (const EGFileSystem &rval);

  static void               Initialise(void);
  static void               Register(EGFileDriver *pDriver);
  static EGFileDriver*      GetDriver(char DriverID);
  static const char*        GetExt(const char *pFileName);

    void                    *m_pFile;
    EG_FS_FileCache_t       *m_pCache;
    EGFileDriver            *m_pDriver;
    void                    *m_pDir;

private:
  EG_FSResult_e             ReadCached(char *pBuffer, uint32_t Count, uint32_t *pReadCount);
  char*                     GetDrives(char *pBuffer);
  char*                     Up(char *pPath);
  const char*               GetLast(const char *pPath);
  const char*               IsolatePath(const char *pPath);

  static EGList             m_DriverList;
};

/////////////////////////////////////////////////////////////////////////////////////////
