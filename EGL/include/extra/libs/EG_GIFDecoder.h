/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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

#include <stdint.h>
#include "misc/EG_FileSystem.h"

///////////////////////////////////////////////////////////////////////////////

#if EG_USE_GIF

typedef struct GIFPalette_t {
  int       Size;
  uint8_t   Colors[0x100 * 3];
} GIFPalette_t;

typedef struct GlobalControlExt_t {
  uint16_t  Delay;
  uint8_t   TransIndex;
  uint8_t   Disposal;
  int       Input;
  int       Transparency;
} GlobalControlExt_t;

typedef struct GIFEntry_t {
	uint16_t  Length;
	uint16_t  Prefix;
	uint8_t   Suffix;
} GIFEntry_t;


typedef struct GIFTable_t {
	int         Bulk;
	int         Count;
	GIFEntry_t *pEntries;
} GIFTable_t;

///////////////////////////////////////////////////////////////////////////////

class EGDecoderGIF
{
public:
							          EGDecoderGIF(void);
                        ~EGDecoderGIF(void);
  void                  Rewind(void);
  int                   GetFrame(void);
  void                  RenderFrame(uint8_t *pBuffer);

  static EGDecoderGIF*  OpenFile(const char *pFileName);
  static EGDecoderGIF*  OpenData(const void *pData);

  EGFileSystem          m_File;
  const char            *m_pVarData;
  bool                  m_IsFile;
  uint32_t              m_VarReadPos;
  int32_t               m_AnimateStart;
  uint16_t              m_Width;
  uint16_t              m_Height;
  uint16_t              m_Depth;
  uint32_t              m_ImageSize;
  int32_t               m_LoopCount;
  GlobalControlExt_t    m_GlobalControlExt;
  GIFPalette_t          *m_pPalette;
  GIFPalette_t          m_LocalColorTable;
  GIFPalette_t          m_GlobalColorTable;
  uint16_t              m_FrameX;
  uint16_t              m_FrameY;
  uint16_t              m_FrameWidth;
  uint16_t              m_FrameHeight;
  uint8_t               m_BackgroundIndex;
  uint8_t               *m_pFrame;
  uint8_t               *m_pCanvas;

  void (*PlainText)(EGDecoderGIF *pGifDecoder, uint16_t tx, uint16_t ty, uint16_t tw, uint16_t th, uint8_t cw, uint8_t ch, uint8_t fg, uint8_t bg);
  void (*Comment)(EGDecoderGIF *pGifDecoder);
  void (*Application)(EGDecoderGIF *pGifDecoder, char id[8], char auth[3]);

private:
  uint16_t          ReadNumber(void);
  EGDecoderGIF*     Initialise(void);
  bool              FileOpen(const void *pSource, bool IsFile);
  void              FileRead(void *pBuffer, size_t Length);
  int               FileSeek(size_t Offset, EG_FS_Seek_e Mode);
  void              FileClose(void);
  void              DiscardSubblocks(void);
  void              ReadPlainTextExt(void);
  void              ReadGraphicControlExt(void);
  void              ReadCommentExt(void);
  void              ReadApplicationExt(void);
  void              ReadExt(void);
  GIFTable_t*       NewTable(int KeySize);
  int               AddEntry(GIFTable_t **ppTtable, uint16_t Length, uint16_t Prefix, uint8_t Suffix);
  uint16_t          GetKey(int KeySize, uint8_t *pSubLength, uint8_t *pShift, uint8_t *pByte);
  int               InterlacedLineIndex(int h, int y);
  int               ReadImageData(int interlace);
  int               ReadImage(void);
  void              RenderFrameRect(uint8_t *pBuffer);
  void              Dispose(void);

};


#endif 

