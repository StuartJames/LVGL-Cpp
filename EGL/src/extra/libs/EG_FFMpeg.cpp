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

#include "extra/libs/EG_FFMpeg.h"
#if EG_USE_FFMPEG != 0

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>

/////////////////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_DEPTH == 1 || EG_COLOR_DEPTH == 8
#define AV_PIX_FMT_TRUE_COLOR AV_PIX_FMT_RGB8
#elif EG_COLOR_DEPTH == 16
#if EG_COLOR_16_SWAP == 0
#define AV_PIX_FMT_TRUE_COLOR AV_PIX_FMT_RGB565LE
#else
#define AV_PIX_FMT_TRUE_COLOR AV_PIX_FMT_RGB565BE
#endif
#elif EG_COLOR_DEPTH == 32
#define AV_PIX_FMT_TRUE_COLOR AV_PIX_FMT_BGR0
#else
#error Unsupported  EG_COLOR_DEPTH
#endif

#define MY_CLASS &lv_ffmpeg_player_class

#define FRAME_DEF_REFR_PERIOD 33 /*[ms]*/

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct FFmpegContext_t {
	AVFormatContext *pFormatContext;
	AVCodecContext *pVideoDecoderContext;
	AVStream *pVideoStream;
	uint8_t *pVideoSourceData[4];
	uint8_t *VideoDSTData[4];
	struct SwsContext *pSWSContext;
	AVFrame *pFrame;
	AVPacket Packet;
	int VideoStreamIndex;
	int VideoSourceLineSize[4];
	int VideoDSTLineSize[4];
	enum AVPixelFormat VideoDSTPixelFormat;
	bool HasAlpha;
  EGPlayerMPEG  *pPlayer;
} FFmpegContext_t;

#pragma pack(1)

struct EG_ImagePixelColor_s {
	EG_Color_t c;
	uint8_t alpha;
};

#pragma pack()

/////////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_FFMpegPlayerClass = {
  .pBaseClassType = &c_Imageclass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////
// MPEG Decoder
/////////////////////////////////////////////////////////////////////////////////////////

EGDecoderMPEG DecoderMPEG;

/////////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderMPEG::Info(const void *pSource, EG_ImageHeader_t *pHeader)
{
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);	// Get the source Type 
	if(SourceType == EG_IMG_SRC_FILE) {
		const char *pFileName = (const char*)pSource;
		if(GetImageHeader(pFileName, pHeader) < 0) {
			EG_LOG_ERROR("ffmpeg can't get image header");
			return EG_RES_INVALID;
		}
		return EG_RES_OK;
	}
	return EG_RES_INVALID;	// If didn't succeeded, then it's an error 
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGDecoderMPEG::Open(ImageDecoderDescriptor_t  *pDescriptor)
{
	if(pDescriptor->SourceType == EG_IMG_SRC_FILE){
		const char *pFileName = (char*)pDescriptor->pSource;
		FFmpegContext_t *pMpegContext = OpenFile(pFileName);
		if(pMpegContext == nullptr) return EG_RES_INVALID;
		if(AllocateImage(pMpegContext) < 0) {
			EG_LOG_ERROR("ffmpeg image allocate failed");
			CloseFile(pMpegContext);
			return EG_RES_INVALID;
		}
		if(UpdateNextFrame(pMpegContext) < 0) {
			CloseFile(pMpegContext);
			EG_LOG_ERROR("ffmpeg update pFrame failed");
			return EG_RES_INVALID;
		}
		CloseSourceContext(pMpegContext);
		uint8_t *pImageData = GetImageData(pMpegContext);
#if EG_COLOR_DEPTH != 32
		if(pMpegContext->HasAlpha) {
			ConvertColorDepth(pImageData, pDescriptor->Header.Width * pDescriptor->Header.Height);
		}
#endif
		pDescriptor->pExtParam = pMpegContext;
		pDescriptor->pImageData = pImageData;
		return EG_RES_OK;		// The image is fully decoded. Return with its pointer 
	}
	return EG_RES_INVALID;	// If not returned earlier then it failed 
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGDecoderMPEG::Close(ImageDecoderDescriptor_t *pDescriptor)
{
	FFmpegContext_t *pMpegContext = pDescriptor->pExtParam;
	CloseFile(pMpegContext);
}

/////////////////////////////////////////////////////////////////////////////////////////

FFmpegContext_t* EGDecoderMPEG::OpenFile(const char *path)
{
	if(path == NULL || strlen(path) == 0) {
		EG_LOG_ERROR("file path is empty");
    return;
	}
	FFmpegContext_t *pMpegContext = (FFmpegContext_t*)calloc(1, sizeof(FFmpegContext_t));
	if(pMpegContext == NULL) {
		EG_LOG_ERROR("Context malloc failed");
		return;
	}
	// open input file, and allocate format context 
	if(avformat_open_input(&(pMpegContext->pFormatContext), path, NULL, NULL) < 0) {
		EG_LOG_ERROR("Could not open source file %s", path);
		return;
	}
	// retrieve stream information 
	if(avformat_find_stream_info(pMpegContext->pFormatContext, NULL) < 0) {
		EG_LOG_ERROR("Could not find stream information");
  	CloseFile(pMpegContext);
	}
	if(OpenCodecContext(
			 &(pMpegContext->VideoStreamIndex),
			 &(pMpegContext->pVideoDecoderContext),
			 pMpegContext->pFormatContext, AVMEDIA_TYPE_VIDEO) >= 0) {
		pMpegContext->pVideoStream = pMpegContext->pFormatContext->streams[pMpegContext->VideoStreamIndex];
		pMpegContext->HasAlpha = PixFormatHasAlpha(pMpegContext->pVideoDecoderContext->PixelFormat);
		pMpegContext->VideoDSTPixelFormat = (pMpegContext->HasAlpha ? AV_PIX_FMT_BGRA : AV_PIX_FMT_TRUE_COLOR);
	}

#if EG_FFMPEG_AV_DUMP_FORMAT != 0
	/* dump input information to stderr */
	av_dump_format(m_pMpegContext->pFormatContext, 0, path, 0);
#endif
	if(pMpegContext->pVideoStream == NULL) {
		EG_LOG_ERROR("Could not find video stream in the input, aborting");
  	CloseFile(pMpegContext);
	}
  return pMpegContext;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGDecoderMPEG::CloseFile(FFmpegContext_t *pMpegContext)
{
	if(pMpegContext == nullptr) {
		EG_LOG_WARN("m_pMpegContext is NULL");
		return;
	}
	sws_freeContext(pMpegContext->pSWSContext);
	CloseSourceContext(pMpegContext);
	CloseDestContext(pMpegContext);
	free(pMpegContext);
	EG_LOG_INFO("MpegContext closed");
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGDecoderMPEG::CloseSourceContext(FFmpegContext_t *pMpegContext)
{
	avcodec_free_context(&(pMpegContext->pVideoDecoderContext));
	avformat_close_input(&(pMpegContext->pFormatContext));
	av_frame_free(&(pMpegContext->pFrame));
	if(pMpegContext->pVideoSourceData[0] != NULL) {
		av_free(pMpegContext->pVideoSourceData[0]);
		pMpegContext->pVideoSourceData[0] = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGDecoderMPEG::CloseDestContext(FFmpegContext_t *pMpegContext)
{
	if(pMpegContext->VideoDSTData[0] != NULL) {
		av_free(pMpegContext->VideoDSTData[0]);
		pMpegContext->VideoDSTData[0] = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGDecoderMPEG::AllocateImage(FFmpegContext_t *pMpegContext)
{
int Result;

	Result = av_image_alloc(	// allocate image where the decoded image will be put 
		pMpegContext->pVideoSourceData,
		pMpegContext->VideoSourceLineSize,
		pMpegContext->pVideoDecoderContext->width,
		pMpegContext->pVideoDecoderContext->height,
		pMpegContext->pVideoDecoderContext->PixelFormat,
		4);

	if(Result < 0) {
		EG_LOG_ERROR("Could not allocate src raw video buffer");
		return Result;
	}
	EG_LOG_INFO("alloc video_src_bufsize = %d", Result);
	Result = av_image_alloc(
		pMpegContext->VideoDSTData,
		pMpegContext->VideoDSTLineSize,
		pMpegContext->pVideoDecoderContext->width,
		pMpegContext->pVideoDecoderContext->height,
		pMpegContext->VideoDSTPixelFormat,
		4);
	if(Result < 0) {
		EG_LOG_ERROR("Could not allocate dst raw video buffer");
		return Result;
	}
	EG_LOG_INFO("allocate video_dst_bufsize = %d", Result);
	pMpegContext->pFrame = av_frame_alloc();
	if(pMpegContext->pFrame == NULL) {
		EG_LOG_ERROR("Could not allocate pFrame");
		return -1;
	}
	// initialize packet, set data to NULL, let the demuxer fill it 
	av_init_packet(&pMpegContext->Packet);
	pMpegContext->Packet.data = NULL;
	pMpegContext->Packet.size = 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

uint8_t* EGDecoderMPEG::GetImageData(FFmpegContext_t *pMpegContext)
{
	uint8_t *pImageData = pMpegContext->VideoDSTData[0];
	if(pImageData == NULL) {
		EG_LOG_ERROR("ffmpeg video DST data is NULL");
	}
	return pImageData;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGDecoderMPEG::GetImageHeader(const char *pPath, EG_ImageHeader_t *pHeader)
{
	int Result = -1;

	AVFormatContext *pFormatContext = NULL;
	AVCodecContext *pVideoDecoderContext = NULL;
	int VideoStreamIndex;

	/* open input file, and allocate format context */
	if(avformat_open_input(&pFormatContext, pPath, NULL, NULL) < 0) {
		EG_LOG_ERROR("Could not open source file %s", pPath);
		goto failed;
	}

	/* retrieve stream information */
	if(avformat_find_stream_info(pFormatContext, NULL) < 0) {
		EG_LOG_ERROR("Could not find stream information");
		goto failed;
	}

	if(OpenCodecContext(&VideoStreamIndex, &pVideoDecoderContext, pFormatContext, AVMEDIA_TYPE_VIDEO) >= 0) {
		bool HasAlpha = PixFormatHasAlpha(pVideoDecoderContext->PixelFormat);
		pHeader->Width = pVideoDecoderContext->width;		// allocate image where the decoded image will be put 
		pHeader->Height = pVideoDecoderContext->height;
		pHeader->AlwaysZero = 0;
		pHeader->ColorFormat = (HasAlpha ? EG_COLOR_FORMAT_NATIVE_ALPHA : EG_COLOR_FORMAT_NATIVE);
		Result = 0;
	}

failed:
	avcodec_free_context(&pVideoDecoderContext);
	avformat_close_input(&pFormatContext);
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGDecoderMPEG::UpdateNextFrame(FFmpegContext_t *pMpegContext)
{
int Result = 0;

	while(1) {
		/* read frames from the file */
		if(av_read_frame(pMpegContext->pFormatContext, &(pMpegContext->Packet)) >= 0) {
			bool is_image = false;
			// check if the packet belongs to a stream we are interested in, otherwise skip it
 			if(pMpegContext->Packet.StreamIndex == pMpegContext->VideoStreamIndex) {
				Result = DecodePacket(pMpegContext);
				is_image = true;
			}
			av_packet_unref(&(pMpegContext->Packet));
			if(Result < 0) {
				EG_LOG_WARN("video pFrame is empty %d", Result);
				break;
			}
			if(is_image) {			// Used to filter data that is not an image 
				break;
			}
		}
		else {
			Result = -1;
			break;
		}
	}
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGDecoderMPEG::OpenCodecContext(int *pStreamIndex, AVCodecContext **pDecoderContext, AVFormatContext *pFormatContext, enum AVMediaType Type)
{
int Result;
int StreamIndex;
AVStream *pStream;
AVCodec *pDecoder = nullptr;
AVDictionary *pOptions = nullptr;

	Result = av_find_best_stream(pFormatContext, Type, -1, -1, NULL, 0);
	if(Result < 0) {
		EG_LOG_ERROR("Could not find %s stream in input file", av_get_media_type_string(Type));
		return Result;
	}
	else {
		StreamIndex = Result;
		pStream = pFormatContext->streams[StreamIndex];
		pDecoder = avcodec_find_decoder(pStream->codecpar->codec_id);		// find decoder for the stream 
		if(pDecoder == NULL) {
			EG_LOG_ERROR("Failed to find %s codec", av_get_media_type_string(Type));
			return AVERROR(EINVAL);
		}
		*pDecoderContext = avcodec_alloc_context3(pDecoder);		// Allocate a codec context for the decoder 
		if(*pDecoderContext == NULL) {
			EG_LOG_ERROR("Failed to allocate the %s codec context", av_get_media_type_string(Type));
			return AVERROR(ENOMEM);
		}
		// Copy codec parameters from input stream to output codec context 
		if((Result = avcodec_parameters_to_context(*pDecoderContext, pStream->codecpar)) < 0) {
			EG_LOG_ERROR(
				"Failed to copy %s codec parameters to decoder context", av_get_media_type_string(Type));
			return Result;
		}
		if((Result = avcodec_open2(*pDecoderContext, pDecoder, &pOptions)) < 0) {		// Init the decoders 
			EG_LOG_ERROR("Failed to open %s codec", av_get_media_type_string(Type));
			return Result;
		}
		*pStreamIndex = StreamIndex;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGDecoderMPEG::DecodePacket(FFmpegContext_t *pMpegContext)
{
int Result = 0;

  AVCodecContext *pDecoder = pMpegContext->pVideoDecoderContext;
  const AVPacket *pPacket = &(pMpegContext->Packet); 
	Result = avcodec_send_packet(pDecoder, pPacket);	// submit the packet to the decoder 
	if(Result < 0) {
		EG_LOG_ERROR("Error submitting a packet for decoding (%s)", av_err2str(Result));
		return Result;
	}
	while(Result >= 0) {	// get all the available frames from the decoder 
		Result = avcodec_receive_frame(pDecoder, pMpegContext->pFrame);
		if(Result < 0) {
			/* those two returned values are special and mean there is no output pFrame available,
       * but there were no errors during decoding */
			if(Result == AVERROR_EOF || Result == AVERROR(EAGAIN)) return 0;
			EG_LOG_ERROR("Error during decoding (%s)", av_err2str(Result));
			return Result;
		}
		if(pDecoder->codec->Type == AVMEDIA_TYPE_VIDEO) {		// write the pFrame data to output file 
			Result = OutputVideoFrame(pMpegContext);
		}
		av_frame_unref(pMpegContext->pFrame);
		if(Result < 0) {
			EG_LOG_WARN("ffmpeg_decode_packet ended %d", Result);
			return Result;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EGDecoderMPEG::PixFormatHasAlpha(enum AVPixelFormat PixelFormat)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(PixelFormat);

	if(desc == NULL) return false;
	if(PixelFormat == AV_PIX_FMT_PAL8) return true;
	return (desc->flags & AV_PIX_FMT_FLAG_ALPHA) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EGDecoderMPEG::PixFormatIsYUV(enum AVPixelFormat PixelFormat)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(PixelFormat);
	if(desc == NULL) return false;
	return !(desc->flags & AV_PIX_FMT_FLAG_RGB) && desc->nb_components >= 2;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGDecoderMPEG::OutputVideoFrame(FFmpegContext_t *pMpegContext)
{
int Result = -1;

	int width = pMpegContext->pVideoDecoderContext->width;
	int height = pMpegContext->pVideoDecoderContext->height;
	AVFrame *pFrame = pMpegContext->pFrame;
	if(pFrame->width != width || pFrame->height != height || pFrame->format != m_pMpegContext->pVideoDecoderContext->PixelFormat) {
		// To handle this change, one could call av_image_alloc again and decode the following frames into another rawvideo file.
  		EG_LOG_ERROR("Width, height and pixel format have to be "
								 "constant in a rawvideo file, but the width, height or "
								 "pixel format of the input video changed:\n"
								 "old: width = %d, height = %d, format = %s\n"
								 "new: width = %d, height = %d, format = %s\n",
								 width, height,
								 av_get_pix_fmt_name(m_pMpegContext->pVideoDecoderContext->PixelFormat),
								 pFrame->width, pFrame->height,
								 av_get_pix_fmt_name(pFrame->format));
		return Result;
	}
	EG_LOG_TRACE("video_frame coded_n:%d", pFrame->coded_picture_number);
	// copy decoded pFrame to destination buffer: this is required since rawvideo expects non aligned data
	av_image_copy(m_pMpegContext->pVideoSourceData, m_pMpegContext->VideoSourceLineSize,
								(const uint8_t **)(pFrame->data), pFrame->linesize,
								m_pMpegContext->pVideoDecoderContext->PixelFormat, width, height);
	if(m_pMpegContext->pSWSContext == NULL) {
		int swsFlags = SWS_BILINEAR;
		if(PixFormatIsYUV(m_pMpegContext->pVideoDecoderContext->PixelFormat)) {
			/* When the video width and height are not multiples of 8,
             * and there is no size change in the conversion,
             * a blurry screen will appear on the right side
             * This problem was discovered in 2012 and
             * continues to exist in version 4.1.3 in 2019
             * This problem can be avoided by increasing SWS_ACCURATE_RND */
			if((width & 0x7) || (height & 0x7)) {
				EG_LOG_WARN("The width(%d) and height(%d) the image is not a multiple of 8, "
										"the decoding speed may be reduced", width, height);
				swsFlags |= SWS_ACCURATE_RND;
			}
		}
		m_pMpegContext->pSWSContext = sws_getContext(
			width, height, m_pMpegContext->pVideoDecoderContext->PixelFormat,
			width, height, m_pMpegContext->VideoDSTPixelFormat,
			swsFlags,
			NULL, NULL, NULL);
	}
	if(!m_pMpegContext->HasAlpha) {
		int lv_linesize = sizeof(EG_Color_t) * width;
		int dst_linesize = m_pMpegContext->VideoDSTLineSize[0];
		if(dst_linesize != lv_linesize) {
			EG_LOG_WARN("ffmpeg linesize = %d, but lvgl image require %d", dst_linesize, lv_linesize);
			m_pMpegContext->VideoDSTLineSize[0] = lv_linesize;
		}
	}
	Result = sws_scale(
		m_pMpegContext->pSWSContext,
		(const uint8_t *const *)(m_pMpegContext->pVideoSourceData),
		m_pMpegContext->VideoSourceLineSize,
		0,
		height,
		m_pMpegContext->VideoDSTData,
		m_pMpegContext->VideoDSTLineSize);
  return Result;
}

///////////////////////////////////////////////////////////////////////////////
// MPEG Player
/////////////////////////////////////////////////////////////////////////////////////////

EGPlayerMPEG::EGPlayerMPEG(void) : EGImage(),
	m_pTimer(nullptr),
	m_AutoRestart(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGPlayerMPEG::EGPlayerMPEG(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= IMAGE_CLASS*/) : EGImage(),
	m_pTimer(nullptr),
	m_AutoRestart(false)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGPlayerMPEG::~EGPlayerMPEG(void)
{
  if(m_pTimer) EGTimer::Delete(m_pTimer);
  InvalidateImageCacheSource(GetSource());
	DecoderMPEG.CloseFile(m_pMpegContext);
	m_pMpegContext = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGPlayerMPEG::Configure(void)
{
  EGImage::Configure();
 	m_pTimer = EGTimer::Create(FrameUpdateCB, FRAME_DEF_REFR_PERIOD, this, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGPlayerMPEG::SetSourcePath(const char *pPath)
{
	if(m_pMpegContext) {
		DecoderMPEG.CloseFile(m_pMpegContext);
		m_pMpegContext = nullptr;
	}
	m_pTimer->Pause();
	m_pMpegContext = DecoderMPEG.OpenFile(pPath);
	if(!m_pMpegContext) {
		EG_LOG_ERROR("ffmpeg file open failed: %s", pPath);
		return EG_RES_INVALID;
	}

	if(DecoderMPEG.AllocateImage(m_pMpegContext) < 0) {
		EG_LOG_ERROR("ffmpeg image allocate failed");
		DecoderMPEG.CloseFile(m_pMpegContext);
		return EG_RES_INVALID;
	}
	bool HasAlpha = m_pMpegContext->HasAlpha;
	int Width = m_pMpegContext->pVideoDecoderContext->width;
	int Height = m_pMpegContext->pVideoDecoderContext->height;
	uint32_t DataSize = 0;
	if(HasAlpha) {
		DataSize = Width * Height * EG_IMG_PX_SIZE_ALPHA_BYTE;
	}
	else {
		DataSize = Width * Height * EG_COLOR_SIZE / 8;
	}
	m_ImageDescriptor.m_Header.AlwaysZero = 0;
	m_ImageDescriptor.m_Header.Width = Width;
	m_ImageDescriptor.m_Header.Height = Height;
	m_ImageDescriptor.m_DataSize = DataSize;
	m_ImageDescriptor.m_Header.ColorFormat = HasAlpha ? EG_COLOR_FORMAT_NATIVE_ALPHA : EG_COLOR_FORMAT_NATIVE;
	m_ImageDescriptor.m_pData = DecoderMPEG.GetImageData(m_pMpegContext);
	SetSource(&(m_ImageDescriptor));
	int Period = GetFrameRefreshPeriod();
	if(Period > 0) {
		EG_LOG_INFO("pFrame refresh period = %d ms, rate = %d fps", Period, 1000 / Period);
		m_pTimer->SetPeriod(Period);
	}
	else EG_LOG_WARN("unable to get pFrame refresh period");
	return EG_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGPlayerMPEG::GetFrameCount(const char *pPath)
{
	int Result = -1;
	FFmpegContext_t* pMpegContext = DecoderMPEG.OpenFile(pPath);
	if(pMpegContext) {
		Result = pMpegContext->pVideoStream->nb_frames;
		DecoderMPEG.CloseFile(pMpegContext);
	}
	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGPlayerMPEG::SetCommand(EG_FFMpegCommand_e Command)
{

	if(!m_pMpegContext) {
		EG_LOG_ERROR("m_pMpegContext is NULL");
		return;
	}
	switch(Command) {
		case EG_FFMPEG_PLAYER_CMD_START:
			av_seek_frame(player->m_pMpegContext->pFormatContext, 0, 0, AVSEEK_FLAG_BACKWARD);
			m_pTimer->Resume();
			EG_LOG_INFO("ffmpeg player start");
			break;
		case EG_FFMPEG_PLAYER_CMD_STOP:
			av_seek_frame(player->m_pMpegContext->pFormatContext, 0, 0, AVSEEK_FLAG_BACKWARD);
			m_pTimer->Pause();
			EG_LOG_INFO("ffmpeg player stop");
			break;
		case EG_FFMPEG_PLAYER_CMD_PAUSE:
			m_pTimer->Pause();
			EG_LOG_INFO("ffmpeg player pause");
			break;
		case EG_FFMPEG_PLAYER_CMD_RESUME:
			m_pTimer->Resume();
			EG_LOG_INFO("ffmpeg player resume");
			break;
		default:
			EG_LOG_ERROR("Error Command: %d", Command);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGPlayerMPEG::SetAutoRestart(bool Enable)
{
	m_AutoRestart = Enable;
}

/////////////////////////////////////////////////////////////////////////////////////////

int EGPlayerMPEG::GetFrameRefreshPeriod()
{
	int avg_frame_rate_num = m_pMpegContext->pVideoStream->avg_frame_rate.num;
	if(avg_frame_rate_num > 0) {
		int period = 1000 * (int64_t)m_pMpegContext->pVideoStream->avg_frame_rate.den / avg_frame_rate_num;
		return period;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGPlayerMPEG::FrameUpdateCB(EGTimer *pTimer)
{
	EGPlayerMPEG *Player = (EGPlayerMPEG*)pTimer->m_pParam;
  FFmpegContext_t *pMpegContext = Player->GetContext();
	if(pMpegContext == nullptr) return;
	int has_next = DecoderMPEG.UpdateNextFrame(pMpegContext);
	if(has_next < 0) {
		Player->SetCommand(Player->m_AutoRestart ? EG_FFMPEG_PLAYER_CMD_START : EG_FFMPEG_PLAYER_CMD_STOP);
		return;
	}
#if EG_COLOR_DEPTH != 32
	if(Player->GetContext()->HasAlpha) {
		ConvertColorDepth((uint8_t *)(Player->m_ImageDescriptor.m_pData), Player->m_ImageDescriptor.m_Header.Width * Player->m_ImageDescriptor.m_Header.Height);
	}
#endif
	InvalidateImageCacheSource(Player->GetSource());
	Player->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////

#if EG_COLOR_DEPTH != 32
void ConvertColorDepth(uint8_t *pImage, uint32_t PixelCount)
{
	EG_Color32_t *pImageSource = (EG_Color32_t *)pImage;
	struct EG_ImagePixelColor_s *pImageDest = (struct EG_ImagePixelColor_s *)pImage;
	for(uint32_t i = 0; i < PixelCount; i++) {
		EG_Color32_t temp = *pImageSource;
		pImageDest->c = EG_ColorHex(temp.full);
		pImageDest->alpha = temp.ch.alpha;
		pImageSource++;
		pImageDest++;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////

#endif
