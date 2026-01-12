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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "EGL.h"

#if EG_USE_FFMPEG != 0

///////////////////////////////////////////////////////////////////////////////////////

struct FFmpegContext_t;

extern const EG_ClassType_t c_FFMpegPlayerClass;

typedef enum EG_FFMpegCommand_e : uint8_t{
    EG_FFMPEG_PLAYER_CMD_START,
    EG_FFMPEG_PLAYER_CMD_STOP,
    EG_FFMPEG_PLAYER_CMD_PAUSE,
    EG_FFMPEG_PLAYER_CMD_RESUME,
    _EG_FFMPEG_PLAYER_CMD_LAST
} EG_FFMpegCommand_e;

///////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_DEPTH != 32
static void  ConvertColorDepth(uint8_t *pImage, uint32_t PixelCount);
#endif 

///////////////////////////////////////////////////////////////////////////////

class EGDecoderMPEG : public EGImageDecoder
{
public:
							              EGDecoderMPEG(void){};
  EG_Result_t               Info(const void *pSource, EG_ImageHeader_t *pHeader);
  EG_Result_t               Open(ImageDecoderDescriptor_t  *pDescriptor);
  void                      Close(ImageDecoderDescriptor_t *pDescriptor);
  FFmpegContext_t*          OpenFile(const char *pPath);
  void                      CloseFile(FFmpegContext_t *pMpegContext);
  void                      CloseSourceContext(FFmpegContext_t *pMpegContext);
  void                      CloseDestContext(FFmpegContext_t *pMpegContext);
  int                       AllocateImage(FFmpegContext_t *pMpegContext);
  uint8_t*                  GetImageData(FFmpegContext_t *pMpegContext);
  int                       GetImageHeader(const char *pPath, EG_ImageHeader_t *pHeader);
  int                       UpdateNextFrame(FFmpegContext_t *pMpegContext);
  int                       OpenCodecContext(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
  int                       DecodePacket(FFmpegContext_t *pMpegContext);
  int                       OutputVideoFrame(FFmpegContext_t *pMpegContext);
  bool                      PixFormatHasAlpha(enum AVPixelFormat PixelFormat);
  bool                      PixFormatIsYUV(enum AVPixelFormat PixelFormat);
      
private:      

	EGFileSystem              m_File;
	unsigned int              m_PixelOffset;
	int                       m_PixelWidth;
	int                       m_PixelHeight;
	unsigned int              m_BitsPerPixel;
	int                       m_RowByteSize;
};

///////////////////////////////////////////////////////////////////////////////////////

extern EGDecoderMPEG DecoderMPEG;

///////////////////////////////////////////////////////////////////////////////////////

class EGPlayerMPEG : public EGImage
{
public:
                            EGPlayerMPEG(void);
                            EGPlayerMPEG(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_FFMpegPlayerClass);
                            ~EGPlayerMPEG(void);
  virtual void              Configure(void);
  EG_Result_t               SetSourcePath(const char *pPath);
  int                       GetFrameCount(const char *pPath);
  void                      SetCommand(EG_FFMpegCommand_e Command);
  void                      SetAutoRestart(bool Enable);
  FFmpegContext_t*          GetContext(void){ return m_pMpegContext; };

  static void               FrameUpdateCB(EGTimer *pTimer);

private:      
  int                       GetFrameRefreshPeriod(void);


  EGDecoderMPEG             *m_pDecoder;
  EGTimer                   *m_pTimer;
  EGImageBuffer             m_ImageDescriptor;
  bool                      m_AutoRestart;
  FFmpegContext_t           *m_pMpegContext;
};


#endif 
