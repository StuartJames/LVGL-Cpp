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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "misc/EG_FileSystem.h"

#include "misc/EG_Assert.h"
#include <string.h>
#include "misc/lv_gc.h"


EGList EGFileSystem::m_DriverList;

/////////////////////////////////////////////////////////////////////////////////////////

EGFileDriver::EGFileDriver(void) :
  m_DriverID(0),
  m_CacheSize(0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

EGFileDriver::~EGFileDriver(void)
{
}

//////////////////////////////////////////////////////////////////////////////////

void EGFileDriver::operator=(const EGFileDriver &rval)
{
  ReadyCB = rval.ReadyCB;
  OpenCB = rval.OpenCB;
  CloseCB = rval.CloseCB;
  ReadCB = rval.ReadCB;
  WriteCB = rval.WriteCB;
  SeekCB = rval.SeekCB;
  TellCB = rval.TellCB;
  DirOpenCB = rval.DirOpenCB;
  DirReadCB = rval.DirReadCB;
  DirCloseCB = rval.DirCloseCB;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

EGFileSystem::EGFileSystem(void) :
 m_pFile(nullptr),
 m_pCache(nullptr),
 m_pDriver(nullptr)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

EGFileSystem::~EGFileSystem(void)
{
}

//////////////////////////////////////////////////////////////////////////////////

void EGFileSystem::operator=(const EGFileSystem &rval)
{
  m_pFile = rval.m_pFile;
  m_pCache = rval.m_pCache;
  m_pDriver = rval.m_pDriver;
  m_pDir = rval.m_pDir;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGFileSystem::Initialise(void)
{
	m_DriverList.Initialise();
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGFileSystem::Register(EGFileDriver *pDriver)
{
  EGFileDriver *pNew = new EGFileDriver;
  if(pNew == nullptr) return;
  pNew = pDriver;       // copy parameters
	m_DriverList.AddHead(pNew);
}

/////////////////////////////////////////////////////////////////////////////////////////

EGFileDriver* EGFileSystem::GetDriver(char DriverID)
{
EGFileDriver *pDriver;

  POSITION Pos = m_DriverList.GetHeadPosition();
	while(Pos != nullptr){
    pDriver = (EGFileDriver*)m_DriverList.GetNext(Pos);
		if(pDriver->m_DriverID == DriverID) {
			return pDriver;
		}
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EGFileSystem::IsReady(char DriverID)
{
	EGFileDriver *pDriver = GetDriver(DriverID);
	if(pDriver == nullptr) return false; // An unknown driver is not ready
	if(pDriver->ReadyCB == nullptr) return true; // Assume the driver is always ready if no handler provided
	return pDriver->ReadyCB(pDriver);
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::Open(const char *pPath, EG_FS_Mode_e Mode)
{
	if(pPath == nullptr) {
		EG_LOG_WARN("Can't open file: path is NULL");
		return EG_FS_RES_INV_PARAM;
	}
	char DriveID = pPath[0];
	EGFileDriver *Driver = GetDriver(DriveID);
	if(Driver == nullptr) {
		EG_LOG_WARN("Can't open file (%s): unknown drive letter", pPath);
		return EG_FS_RES_NOT_EX;
	}
	if(Driver->ReadyCB){
		if(Driver->ReadyCB(Driver) == false) {
			EG_LOG_WARN("Can't open file (%s): driver is not ready", pPath);
			return EG_FS_RES_HW_ERR;
		}
	}
	if(Driver->OpenCB == nullptr) {
		EG_LOG_WARN("Can't open file (%s): open function does not exists", pPath);
		return EG_FS_RES_NOT_IMP;
	}
	const char *pRealPath = IsolatePath(pPath);
	void *pFile = Driver->OpenCB(Driver, pRealPath, Mode);
	if(pFile == nullptr || pFile == (void *)(-1)) return EG_FS_RES_UNKNOWN;
	m_pDriver = Driver;
	m_pFile = pFile;
	if(Driver->m_CacheSize) {
		m_pCache = (EG_FS_FileCache_t*)EG_AllocMem(sizeof(EG_FS_FileCache_t));
		EG_ASSERT_MALLOC(m_pCache);
		EG_ZeroMem(m_pCache, sizeof(EG_FS_FileCache_t));
		m_pCache->Start = UINT32_MAX; // Set an invalid range by default
		m_pCache->End = UINT32_MAX - 1;
	}
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::Close(void)
{
	if(m_pDriver == nullptr) {
		return EG_FS_RES_INV_PARAM;
	}
	if(m_pDriver->CloseCB == nullptr) {
		return EG_FS_RES_NOT_IMP;
	}
	EG_FSResult_e res = m_pDriver->CloseCB(m_pDriver, m_pFile);
	if(m_pDriver->m_CacheSize && m_pCache) {
		if(m_pCache->pBuffer) {
			EG_FreeMem(m_pCache->pBuffer);
		}
		EG_FreeMem(m_pCache);
	}
	m_pFile = nullptr;
	m_pDriver = nullptr;
	m_pCache = nullptr;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::ReadCached(char *pReadBuffer, uint32_t Count, uint32_t *pReadCount)
{
	EG_FSResult_e res = EG_FS_RES_OK;
	uint32_t FlePosition = m_pCache->FlePosition;
	uint32_t Start = m_pCache->Start;
	uint32_t End = m_pCache->End;
	char *pBuffer = (char*)m_pCache->pBuffer;
	uint16_t BufferSize = m_pDriver->m_CacheSize;
	if(Start <= FlePosition && FlePosition < End) {
		uint16_t Offset = FlePosition - Start;		//  Data can be read from cache buffer 
		uint32_t buffer_remaining_length = EG_MIN((uint32_t)BufferSize - Offset, (uint32_t)End - FlePosition);
		if(Count <= buffer_remaining_length) {
			// Data is in cache buffer, and buffer End not reached, no need to read from FS
			EG_CopyMem(pReadBuffer, pBuffer + Offset, Count);
			*pReadCount = Count;
		}
		else {
			// First part of data is in cache buffer, but we need to read rest of data from FS
			EG_CopyMem(pReadBuffer, pBuffer + Offset, buffer_remaining_length);
			uint32_t bytes_read_to_buffer = 0;
			if(Count > BufferSize) {
				// If remaining data chuck is bigger than buffer size, then do not use cache, instead read it directly from FS
				res = m_pDriver->ReadCB(m_pDriver, m_pFile, (void *)(pReadBuffer + buffer_remaining_length),
																	 Count - buffer_remaining_length, &bytes_read_to_buffer);
			}
			else {
				// If remaining data chunk is smaller than buffer size, then read into cache buffer
				res = m_pDriver->ReadCB(m_pDriver, m_pFile, (void *)pBuffer, BufferSize, &bytes_read_to_buffer);
				m_pCache->Start = m_pCache->End;
				m_pCache->End = m_pCache->Start + bytes_read_to_buffer;
				uint16_t data_chunk_remaining = EG_MIN(Count - buffer_remaining_length, bytes_read_to_buffer);
				EG_CopyMem(pReadBuffer + buffer_remaining_length, pBuffer, data_chunk_remaining);
			}
			*pReadCount = EG_MIN(buffer_remaining_length + bytes_read_to_buffer, Count);
		}
	}
	else {
		if(Count > BufferSize) {		// Data is not in cache buffer
			// If bigger data is requested, then do not use cache, instead read it directly
			res = m_pDriver->ReadCB(m_pDriver, m_pFile, (void *)pReadBuffer, Count, pReadCount);
		}
		else {
			if(pBuffer == NULL) {	// If small data is requested, then read from FS into cache buffer
				m_pCache->pBuffer = EG_AllocMem(BufferSize);
				EG_ASSERT_MALLOC(m_pCache->pBuffer);
				pBuffer = (char*)m_pCache->pBuffer;
			}
			uint32_t bytes_read_to_buffer = 0;
			res = m_pDriver->ReadCB(m_pDriver, m_pFile, (void *)pBuffer, BufferSize, &bytes_read_to_buffer);
			m_pCache->Start = FlePosition;
			m_pCache->End = m_pCache->Start + bytes_read_to_buffer;
			*pReadCount = EG_MIN(Count, bytes_read_to_buffer);
			EG_CopyMem(pReadBuffer, pBuffer, *pReadCount);
		}
	}
	if(res == EG_FS_RES_OK)	m_pCache->FlePosition += *pReadCount;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::Read(void *pBuffer, uint32_t Count, uint32_t *pReadCount)
{
EG_FSResult_e Result;
uint32_t ReadCount = 0;

	if(pReadCount != nullptr) *pReadCount = 0;
	if(m_pDriver == nullptr) return EG_FS_RES_INV_PARAM;
	if(m_pDriver->ReadCB == nullptr) return EG_FS_RES_NOT_IMP;
	if(m_pDriver->m_CacheSize) Result = ReadCached((char *)pBuffer, Count, &ReadCount);
	else Result = m_pDriver->ReadCB(m_pDriver, m_pFile, pBuffer, Count, &ReadCount);
	if(pReadCount != nullptr) *pReadCount = ReadCount;
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::Write(const void *pBuffer, uint32_t Count, uint32_t *pWriteCount)
{
uint32_t WriteCount = 0;

	if(pWriteCount != nullptr) *pWriteCount = 0;
	if(m_pDriver == nullptr) return EG_FS_RES_INV_PARAM;
	if(m_pDriver->WriteCB == nullptr) return EG_FS_RES_NOT_IMP;
	EG_FSResult_e res = m_pDriver->WriteCB(m_pDriver, m_pFile, pBuffer, Count, &WriteCount);
	if(pWriteCount != nullptr) *pWriteCount = WriteCount;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::Seek(uint32_t Pos, EG_FS_Seek_e Mode)
{
EG_FSResult_e Result = EG_FS_RES_OK;

	if(m_pDriver == nullptr) return EG_FS_RES_INV_PARAM;
	if(m_pDriver->SeekCB == nullptr) return EG_FS_RES_NOT_IMP;
	if(m_pDriver->m_CacheSize) {
		switch(Mode) {
			case EG_FS_SEEK_SET: {
				m_pCache->FlePosition = Pos;		// FS seek if new position is outside cache buffer
				if(m_pCache->FlePosition < m_pCache->Start || m_pCache->FlePosition > m_pCache->End) {
					Result = m_pDriver->SeekCB(m_pDriver, m_pFile, m_pCache->FlePosition, EG_FS_SEEK_SET);
				}
				break;
			}
			case EG_FS_SEEK_CUR: {
				m_pCache->FlePosition += Pos;		// FS seek if new position is outside cache buffer
				if(m_pCache->FlePosition < m_pCache->Start || m_pCache->FlePosition > m_pCache->End) {
					Result = m_pDriver->SeekCB(m_pDriver, m_pFile, m_pCache->FlePosition, EG_FS_SEEK_SET);
				}
				break;
			}
			case EG_FS_SEEK_END: {
				// Because we don't know the file size, we do a little trick: do a FS seek, then get new file position from FS
				Result = m_pDriver->SeekCB(m_pDriver, m_pFile, Pos, Mode);
				if(Result == EG_FS_RES_OK) {
					uint32_t Position;
					Result = m_pDriver->TellCB(m_pDriver, m_pFile, &Position);
					if(Result == EG_FS_RES_OK) {
						m_pCache->FlePosition = Position;
					}
				}
				break;
			}
		}
	}
	else Result = m_pDriver->SeekCB(m_pDriver, m_pFile, Pos, Mode);
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::Tell(uint32_t *pPos)
{
EG_FSResult_e Result;

	*pPos = 0;
	if(m_pDriver == nullptr) return EG_FS_RES_INV_PARAM;
	if(m_pDriver->TellCB == nullptr) return EG_FS_RES_NOT_IMP;
	if(m_pDriver->m_CacheSize) {
		*pPos = m_pCache->FlePosition;
		Result = EG_FS_RES_OK;
	}
	else Result = m_pDriver->TellCB(m_pDriver, m_pFile, pPos);
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::DirOpen(const char *pPath)
{
	if(pPath == nullptr) return EG_FS_RES_INV_PARAM;
	char DriveID = pPath[0];
	EGFileDriver *Driver = GetDriver(DriveID);
	if(Driver == nullptr)	return EG_FS_RES_NOT_EX;
	if(Driver->ReadyCB){
		if(Driver->ReadyCB(Driver) == false) return EG_FS_RES_HW_ERR;
	}
	if(Driver->DirOpenCB == nullptr) return EG_FS_RES_NOT_IMP;
	const char *pRealPath = IsolatePath(pPath);
	void *pDir = Driver->DirOpenCB(Driver, pRealPath);
	if(pDir == nullptr || pDir == (void *)(-1)) return EG_FS_RES_UNKNOWN;
	m_pDriver = Driver;
	m_pDir = pDir;
	return EG_FS_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::DirRead(char *pFileNames)
{
	if(m_pDriver == nullptr || m_pDir == nullptr) {
		pFileNames[0] = '\0';
		return EG_FS_RES_INV_PARAM;
	}
	if(m_pDriver->DirReadCB == nullptr) {
		pFileNames[0] = '\0';
		return EG_FS_RES_NOT_IMP;
	}
	return m_pDriver->DirReadCB(m_pDriver, m_pDir, pFileNames);
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_FSResult_e EGFileSystem::DirClose(void)
{
	if(m_pDriver == nullptr || m_pDir == nullptr) {
		return EG_FS_RES_INV_PARAM;
	}
	if(m_pDriver->DirCloseCB == nullptr) {
		return EG_FS_RES_NOT_IMP;
	}
	EG_FSResult_e res = m_pDriver->DirCloseCB(m_pDriver, m_pDir);
	m_pDir = nullptr;
	m_pDriver = nullptr;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

char* EGFileSystem::GetDrives(char *pBuffer)
{
EGFileDriver *pDriver;
uint8_t i = 0;

  POSITION Pos = m_DriverList.GetHeadPosition();
 	while(Pos != nullptr){
    pDriver = (EGFileDriver*)m_DriverList.GetNext(Pos);
   	pBuffer[i] = pDriver->m_DriverID;
		i++;
	}
	pBuffer[i] = '\0';
	return pBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////

const char* EGFileSystem::GetExt(const char *pFileName)
{
	for(size_t i = strlen(pFileName); i > 0; i--) {
		if(pFileName[i] == '.') return &pFileName[i + 1];
		else if(pFileName[i] == '/' || pFileName[i] == '\\') {
			return ""; // No extension if a '\' or '/' found
		}
	}
	return ""; // Empty string if no '.' in the file name.
}

/////////////////////////////////////////////////////////////////////////////////////////

char* EGFileSystem::Up(char *pPath)
{
size_t i = 0;

	size_t Length = strlen(pPath);
	if(Length == 0) return pPath;
	Length--; // Go before the trailing '\0'
	// Ignore trailing '/' or '\'
	while(pPath[Length] == '/' || pPath[Length] == '\\') {
		pPath[Length] = '\0';
		if(Length > 0) Length--;
		else return pPath;
	}
	for(i = Length; i > 0; i--) {
		if(pPath[i] == '/' || pPath[i] == '\\') break;
	}
	if(i > 0) pPath[i] = '\0';
	return pPath;
}

/////////////////////////////////////////////////////////////////////////////////////////

const char* EGFileSystem::GetLast(const char *pPath)
{
size_t i = 0;

	size_t Length = strlen(pPath);
	if(Length == 0) return pPath;
	Length--; // Go before the trailing '\0'
	// Ignore trailing '/' or '\'
	while(pPath[Length] == '/' || pPath[Length] == '\\') {
		if(Length > 0) Length--;
		else return pPath;
	}
	for(i = Length; i > 0; i--) {
		if(pPath[i] == '/' || pPath[i] == '\\') break;
	}
	// No '/' or '\' in the path so return with path itself
	if(i == 0) return pPath;
	return &pPath[i + 1];
}

/////////////////////////////////////////////////////////////////////////////////////////

// Skip the drive letter and the possible : after the letter
const char* EGFileSystem::IsolatePath(const char *pPath)
{
	pPath++; // Ignore the driver letter
	if(*pPath == ':') pPath++;
	return pPath;
}
