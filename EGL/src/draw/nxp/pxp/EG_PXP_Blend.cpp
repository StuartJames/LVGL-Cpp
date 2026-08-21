/**
 * MIT License
 *
 * Copyright 2020-2023 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next paragraph)
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

 #include "draw/nxp/pxp/EG_PXP_Blend.h"

#if EG_USE_GPU_NXP_PXP

#include "fsl_pxp.h"

//////////////////////////////////////////////////////////////////////

#if EG_COLOR_16_SWAP
#error Color swap not implemented. Disable EG_COLOR_16_SWAP feature.
#endif

#if EG_COLOR_DEPTH == 16
#define PXP_OUT_PIXEL_FORMAT kPXP_OutputPixelFormatRGB565
#define PXP_AS_PIXEL_FORMAT kPXP_AsPixelFormatRGB565
#define PXP_PS_PIXEL_FORMAT kPXP_PsPixelFormatRGB565
#define PXP_TEMP_COLOR_SIZE 2U
#elif EG_COLOR_DEPTH == 32
#define PXP_OUT_PIXEL_FORMAT kPXP_OutputPixelFormatARGB8888
#define PXP_AS_PIXEL_FORMAT kPXP_AsPixelFormatARGB8888
#if(!(defined(FSL_FEATURE_PXP_HAS_NO_EXTEND_PIXEL_FORMAT) && FSL_FEATURE_PXP_HAS_NO_EXTEND_PIXEL_FORMAT)) && \
	(!(defined(FSL_FEATURE_PXP_V3) && FSL_FEATURE_PXP_V3))
#define PXP_PS_PIXEL_FORMAT kPXP_PsPixelFormatARGB8888
#else
#define PXP_PS_PIXEL_FORMAT kPXP_PsPixelFormatRGB888
#endif
#define PXP_TEMP_COLOR_SIZE 4U
#elif
#error Only 16bit and 32bit color depth are supported. Set EG_COLOR_DEPTH to 16 or 32.
#endif

//////////////////////////////////////////////////////////////////////////////////////

EGPXPBlend::EGPXPBlend(const EGPXPContext *pDC) : EGBlendBase((EGDeviceContext*)pDC)
{
}

//////////////////////////////////////////////////////////////////////////////////////

EGPXPBlend::~EGPXPBlend(void)
{
}

//////////////////////////////////////////////////////////////////////

void EGPXPBlend::Fill(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep, EG_Color_t Color, EG_OPA_t OPA)
{
	int32_t Width = pDestRect->GetWidth();
	int32_t Height = pDestRect->GetHeight();
	PXP_ResetGPU();
	pxp_output_buffer_config_t outputConfig = {    // Configure OUT buffer
		.pixelFormat = PXP_OUT_PIXEL_FORMAT,
		.interlacedMode = kPXP_OutputProgressive,
		.buffer0Addr = (uint32_t)(pDest + DestStep * pDestRect->GetY1() + pDestRect->GetX1()),
		.buffer1Addr = (uint32_t)NULL,
		.pitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t)),
		.width = (uint16_t)Width,
		.height = (uint16_t)Height
  };
	PXP_SetOutputBufferConfig(EG_GPU_NXP_PXP_ID, &outputConfig);
	if(OPA >= (EG_OPA_t)EG_OPA_MAX) {	// Simple color fill without opacity - AS disabled
		PXP_SetAlphaSurfacePosition(EG_GPU_NXP_PXP_ID, 0xFFFFU, 0xFFFFU, 0U, 0U);
	}
	else {	// Fill with opacity - AS used as source (same as OUT)
		pxp_as_buffer_config_t asBufferConfig = {
			.pixelFormat = PXP_AS_PIXEL_FORMAT,
			.bufferAddr = (uint32_t)outputConfig.buffer0Addr,
			.pitchBytes = outputConfig.pitchBytes
    };
		PXP_SetAlphaSurfaceBufferConfig(EG_GPU_NXP_PXP_ID, &asBufferConfig);
		PXP_SetAlphaSurfacePosition(EG_GPU_NXP_PXP_ID, 0U, 0U, Width - 1U, Height - 1U);
	}

	// Disable PS, use as color generator
	PXP_SetProcessSurfacePosition(EG_GPU_NXP_PXP_ID, 0xFFFFU, 0xFFFFU, 0U, 0U);
	PXP_SetProcessSurfaceBackGroundColor(EG_GPU_NXP_PXP_ID, EG_ColorTo32(Color));

	/* Configure Porter-Duff blending - src settings are unused for fill without opacity (opa = 0xff).
   *
   * Note: srcFactorMode and dstFactorMode are inverted in fsl_pxp.h:
   * srcFactorMode is actually applied on PS alpha value
   * dstFactorMode is actually applied on AS alpha value */
	pxp_porter_duff_config_t pdConfig = {
		.enable = 1,
    .srcFactorMode = (OPA >= (EG_OPA_t)EG_OPA_MAX) ? kPXP_PorterDuffFactorStraight : kPXP_PorterDuffFactorInversed,
    .dstGlobalAlphaMode = kPXP_PorterDuffGlobalAlpha,
    .dstAlphaMode = kPXP_PorterDuffAlphaStraight, // don't care
		.dstColorMode = kPXP_PorterDuffColorNoAlpha,
    .dstFactorMode = kPXP_PorterDuffFactorStraight,
    .srcGlobalAlphaMode = kPXP_PorterDuffGlobalAlpha,
    .srcAlphaMode = kPXP_PorterDuffAlphaStraight,  // don't care
		.srcColorMode = kPXP_PorterDuffColorNoAlpha,
		.dstGlobalAlpha = OPA,
		.srcGlobalAlpha = OPA
	};
	PXP_SetPorterDuffConfig(EG_GPU_NXP_PXP_ID, &pdConfig);
	PXP_RunGPU();
}

//////////////////////////////////////////////////////////////////////

void EGPXPBlend::Blit(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
												 const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
												 EG_OPA_t OPA, EG_DisplayRotation_t angle)
{
	int32_t DestWidth = pDestRect->GetWidth();
	int32_t DestHeight = pDestRect->GetHeight();
	int32_t SrceWidth = pSrceRect->GetWidth();
	int32_t SrceHeight = pSrceRect->GetHeight();
	PXP_ResetGPU();
	pxp_rotate_degree_t pxp_rot;	// convert rotation angle
	switch(angle) {
		case EG_DISP_ROT_NONE:
			pxp_rot = kPXP_Rotate0;
			break;
		case EG_DISP_ROT_90:
			pxp_rot = kPXP_Rotate90;
			break;
		case EG_DISP_ROT_180:
			pxp_rot = kPXP_Rotate180;
			break;
		case EG_DISP_ROT_270:
			pxp_rot = kPXP_Rotate270;
			break;
		default:
			pxp_rot = kPXP_Rotate0;
			break;
	}
	PXP_SetRotateConfig(EG_GPU_NXP_PXP_ID, kPXP_RotateOutputBuffer, pxp_rot, kPXP_FlipDisable);
	pxp_as_blend_config_t asBlendConfig = {
		.alpha = OPA,
		.invertAlpha = false,
		.alphaMode = kPXP_AlphaRop,
		.ropMode = kPXP_RopMergeAs
  };
	if(OPA >= (EG_OPA_t)EG_OPA_MAX) {	// Simple blit, no effect - Disable PS buffer
		PXP_SetProcessSurfacePosition(EG_GPU_NXP_PXP_ID, 0xFFFFU, 0xFFFFU, 0U, 0U);
	}
	else {
		pxp_ps_buffer_config_t psBufferConfig = {
			.pixelFormat = PXP_PS_PIXEL_FORMAT,
			.swapByte = false,
			.bufferAddr = (uint32_t)(pDest + DestStep * pDestRect->GetY1() + pDestRect->GetX1()),
			.bufferAddrU = 0U,
			.bufferAddrV = 0U,
			.pitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t))
    };
		asBlendConfig.alphaMode = kPXP_AlphaOverride;
		PXP_SetProcessSurfaceBufferConfig(EG_GPU_NXP_PXP_ID, &psBufferConfig);
		PXP_SetProcessSurfacePosition(EG_GPU_NXP_PXP_ID, 0U, 0U, DestWidth - 1U, DestHeight - 1U);
	}
	pxp_as_buffer_config_t asBufferConfig = {  // AS buffer - source image
		.pixelFormat = PXP_AS_PIXEL_FORMAT,
		.bufferAddr = (uint32_t)(pSrce + SrceStep * pSrceRect->GetY1() + pSrceRect->GetX1()),
		.pitchBytes = (uint16_t)(SrceStep * sizeof(EG_Color_t))
  };
	PXP_SetAlphaSurfaceBufferConfig(EG_GPU_NXP_PXP_ID, &asBufferConfig);
	PXP_SetAlphaSurfacePosition(EG_GPU_NXP_PXP_ID, 0U, 0U, SrceWidth - 1U, SrceHeight - 1U);
	PXP_SetAlphaSurfaceBlendConfig(EG_GPU_NXP_PXP_ID, &asBlendConfig);
	PXP_EnableAlphaSurfaceOverlayColorKey(EG_GPU_NXP_PXP_ID, false);
	pxp_output_buffer_config_t outputBufferConfig = {
		.pixelFormat = (pxp_output_pixel_format_t)PXP_OUT_PIXEL_FORMAT,
		.interlacedMode = kPXP_OutputProgressive,
		.buffer0Addr = (uint32_t)(pDest + DestStep * pDestRect->GetY1() + pDestRect->GetX1()),
		.buffer1Addr = (uint32_t)0U,
		.pitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t)),
		.width = (uint16_t)DestWidth,
		.height = (uint16_t)DestHeight
  };
	PXP_SetOutputBufferConfig(EG_GPU_NXP_PXP_ID, &outputBufferConfig);
	PXP_RunGPU();
}

//////////////////////////////////////////////////////////////////////

void EGPXPBlend::BlitTransform(EG_Color_t *pDest, EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrce,
        const EGRect *pSrceRect, int32_t SrceStep, const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat)
{
	bool HasRecolor = (pImage->m_RecolorOPA != EG_OPA_TRANSP);
	bool HasRotation = (pImage->m_Angle != 0);
	if(HasRecolor || HasRotation) {
		if(pImage->m_OPA >= (EG_OPA_t)EG_OPA_MAX && !EGDrawImage::HasAlpha(ColorFormat) && !pImage->IsChromaKeyed(ColorFormat)) {
			BlitCover(pDest, pDestRect, DestStep, pSrce, pSrceRect, SrceStep, pImage, ColorFormat);
			return;
		}
		else {
			/*Recolor and/or rotation with alpha or opacity is done in two steps.*/
			BlitOPA(pDest, pDestRect, DestStep, pSrce, pSrceRect, SrceStep, pImage, ColorFormat);
			return;
		}
	}
	BlitColorFormat(pDest, pDestRect, DestStep, pSrce, pSrceRect, SrceStep, pImage, ColorFormat);
}

//////////////////////////////////////////////////////////////////////

void EGPXPBlend::BufferCopy(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
																const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep)
{
	PXP_ResetGPU();
	const pxp_pic_copy_config_t picCopyConfig = {
		.srcPicBaseAddr = (uint32_t)pSrce,
		.srcPitchBytes = (uint16_t)(SrceStep * sizeof(EG_Color_t)),
		.srcOffsetX = (uint16_t)pSrceRect->GetX1(),
		.srcOffsetY = (uint16_t)pSrceRect->GetY1(),
		.destPicBaseAddr = (uint32_t)pDest,
		.destPitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t)),
		.destOffsetX = (uint16_t)pDestRect->GetX1(),
		.destOffsetY = (uint16_t)pDestRect->GetY1(),
		.width = (uint16_t)pSrceRect->GetWidth(),
		.height = (uint16_t)pSrceRect->GetHeight(),
		.pixelFormat = PXP_AS_PIXEL_FORMAT};
	PXP_StartPictureCopy(EG_GPU_NXP_PXP_ID, &picCopyConfig);
	EGPXPContext::WaitForFinish();
}

//////////////////////////////////////////////////////////////////////
 
void EGPXPBlend::BlitOPA(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
														const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
														const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat)
{
uint8_t *pTempBuffer;

  EGRect TempRect(pDestRect);
	int32_t Step = DestStep;
	int32_t Width = TempRect.GetWidth();
	int32_t Height = TempRect.GetHeight();
  pTempBuffer = (uint8_t*)aligned_alloc(pDestRect->GetSize() * PXP_TEMP_COLOR_SIZE, sizeof(EG_Color_t) * 8);
	// Step 1: Transform with full opacity to temporary buffer
	BlitCover((EG_Color_t *)pTempBuffer, &TempRect, Step, pSrce, pSrceRect, SrceStep, pImage, ColorFormat);
	if(pImage->m_Angle == 900 || pImage->m_Angle == 2700) {  // Switch width and height if angle requires it
		TempRect.SetX2(TempRect.GetX1() + Height - 1);
		TempRect.SetY2(TempRect.GetY1() + Width - 1);
	}
	// Step 2: Blit temporary result with required opacity to output
	BlitColorFormat(pDest, &TempRect, DestStep, (EG_Color_t *)pTempBuffer, &TempRect, Step, pImage, ColorFormat);
  free(pTempBuffer);
}

//////////////////////////////////////////////////////////////////////

 void EGPXPBlend::BlitCover(EG_Color_t *pDest, EGRect *pDestRect, int32_t DestStep,
															const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
															const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat)
{
	int32_t DestWidth = pDestRect->GetWidth();
	int32_t DestHeight = pDestRect->GetHeight();
	int32_t SrceWidth = pSrceRect->GetWidth();
	int32_t SrceHeight = pSrceRect->GetHeight();
	bool HasRecolor = (pImage->m_RecolorOPA != EG_OPA_TRANSP);
	bool HasRotation = (pImage->m_Angle != 0);
	EGPoint Pivot = pImage->m_Pivot;
	int32_t PivotOffsetX;
	int32_t PivotOffsetY;
	PXP_ResetGPU();
	if(HasRotation) {
		pxp_rotate_degree_t PxpAngle; // Convert rotation angle and calculate offsets caused by Pivot
		switch(pImage->m_Angle) {
			case 0:
				PxpAngle = kPXP_Rotate0;
				PivotOffsetX = 0;
				PivotOffsetY = 0;
				break;
			case 900:
				PivotOffsetX = Pivot.m_X + Pivot.m_Y - DestHeight;
				PivotOffsetY = Pivot.m_Y - Pivot.m_X;
				PxpAngle = kPXP_Rotate90;
				break;
			case 1800:
				PivotOffsetX = 2 * Pivot.m_X - DestWidth;
				PivotOffsetY = 2 * Pivot.m_Y - DestHeight;
				PxpAngle = kPXP_Rotate180;
				break;
			case 2700:
				PivotOffsetX = Pivot.m_X - Pivot.m_Y;
				PivotOffsetY = Pivot.m_X + Pivot.m_Y - DestWidth;
				PxpAngle = kPXP_Rotate270;
				break;
			default:
				PivotOffsetX = 0;
				PivotOffsetY = 0;
				PxpAngle = kPXP_Rotate0;
		}
		PXP_SetRotateConfig(EG_GPU_NXP_PXP_ID, kPXP_RotateOutputBuffer, PxpAngle, kPXP_FlipDisable);
		pDestRect->Move(PivotOffsetX, PivotOffsetY);
	}
	pxp_as_buffer_config_t asBufferConfig = {
		.pixelFormat = PXP_AS_PIXEL_FORMAT,
		.bufferAddr = (uint32_t)(pSrce + SrceStep * pSrceRect->GetY1() + pSrceRect->GetX1()),
		.pitchBytes = (uint16_t)(SrceStep * sizeof(EG_Color_t))
  };
	PXP_SetAlphaSurfaceBufferConfig(EG_GPU_NXP_PXP_ID, &asBufferConfig);
	PXP_SetAlphaSurfacePosition(EG_GPU_NXP_PXP_ID, 0U, 0U, SrceWidth - 1U, SrceHeight - 1U);
	PXP_SetProcessSurfacePosition(EG_GPU_NXP_PXP_ID, 0xFFFFU, 0xFFFFU, 0U, 0U);	// Disable PS buffer
	if(HasRecolor)	PXP_SetProcessSurfaceBackGroundColor(EG_GPU_NXP_PXP_ID, EG_ColorTo32(pImage->m_Recolor));	// Use as color generator
	pxp_output_buffer_config_t outputBufferConfig = {
		.pixelFormat = (pxp_output_pixel_format_t)PXP_OUT_PIXEL_FORMAT,
		.interlacedMode = kPXP_OutputProgressive,
		.buffer0Addr = (uint32_t)(pDest + DestStep * pDestRect->GetY1() + pDestRect->GetX1()),
		.buffer1Addr = (uint32_t)0U,
		.pitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t)),
		.width = (uint16_t)DestWidth,
		.height = (uint16_t)DestHeight
  };
	PXP_SetOutputBufferConfig(EG_GPU_NXP_PXP_ID, &outputBufferConfig);
	if(HasRecolor || EGDrawImage::HasAlpha(ColorFormat)) {
		/**
     * Configure Porter-Duff blending.
     *
     * Note: srcFactorMode and dstFactorMode are inverted in fsl_pxp.h:
     * srcFactorMode is actually applied on PS alpha value
     * dstFactorMode is actually applied on AS alpha value */
		pxp_porter_duff_config_t pdConfig = {
			.enable = 1,
      .srcFactorMode = kPXP_PorterDuffFactorInversed,
      .dstGlobalAlphaMode = kPXP_PorterDuffGlobalAlpha,
      .dstAlphaMode = kPXP_PorterDuffAlphaStraight, // don't care
			.dstColorMode = kPXP_PorterDuffColorWithAlpha,
      .dstFactorMode = kPXP_PorterDuffFactorStraight,
      .srcGlobalAlphaMode = EGDrawImage::HasAlpha(ColorFormat) ? kPXP_PorterDuffLocalAlpha : kPXP_PorterDuffGlobalAlpha,
      .srcAlphaMode = kPXP_PorterDuffAlphaStraight,
			.srcColorMode = kPXP_PorterDuffColorNoAlpha,
			.dstGlobalAlpha = HasRecolor ? (uint32_t)pImage->m_RecolorOPA : 0,
			.srcGlobalAlpha = 0xff
    };
		PXP_SetPorterDuffConfig(EG_GPU_NXP_PXP_ID, &pdConfig);
	}
	PXP_RunGPU();
}

//////////////////////////////////////////////////////////////////////

 void EGPXPBlend::BlitColorFormat(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep,
													 const EG_Color_t *pSrce, const EGRect *pSrceRect, int32_t SrceStep,
													 const EGDrawImage *pImage, EG_ImageColorFormat_t ColorFormat)
{
	int32_t DestWidth = pDestRect->GetWidth();
	int32_t DestHeight = pDestRect->GetHeight();
	int32_t SrceWidth = pSrceRect->GetWidth();
	int32_t SrceHeight = pSrceRect->GetHeight();
	PXP_ResetGPU();
	pxp_as_blend_config_t asBlendConfig = {
		.alpha = pImage->m_OPA,
		.invertAlpha = false,
		.alphaMode = kPXP_AlphaRop,
		.ropMode = kPXP_RopMergeAs
  };
	if(pImage->m_OPA >= (EG_OPA_t)EG_OPA_MAX && !pImage->IsChromaKeyed(ColorFormat) && !EGDrawImage::HasAlpha(ColorFormat)) {
		PXP_SetProcessSurfacePosition(EG_GPU_NXP_PXP_ID, 0xFFFFU, 0xFFFFU, 0U, 0U);	// Simple blit, no effect - Disable PS buffer
	}
	else {
		// PS must be enabled to fetch background pixels. PS and OUT buffers are the same, blend will be done in-place
		pxp_ps_buffer_config_t psBufferConfig = {
			.pixelFormat = PXP_PS_PIXEL_FORMAT,
			.swapByte = false,
			.bufferAddr = (uint32_t)(pDest + DestStep * pDestRect->GetY1() + pDestRect->GetX1()),
			.bufferAddrU = 0U,
			.bufferAddrV = 0U,
			.pitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t))
    };
		if(pImage->m_OPA >= (EG_OPA_t)EG_OPA_MAX) {
			asBlendConfig.alphaMode = EGDrawImage::HasAlpha(ColorFormat) ? kPXP_AlphaEmbedded : kPXP_AlphaOverride;
		}
		else {
			asBlendConfig.alphaMode = EGDrawImage::HasAlpha(ColorFormat) ? kPXP_AlphaMultiply : kPXP_AlphaOverride;
		}
		PXP_SetProcessSurfaceBufferConfig(EG_GPU_NXP_PXP_ID, &psBufferConfig);
		PXP_SetProcessSurfacePosition(EG_GPU_NXP_PXP_ID, 0U, 0U, DestWidth - 1U, DestHeight - 1U);
	}
	pxp_as_buffer_config_t asBufferConfig = {   // AS buffer - source image
		.pixelFormat = PXP_AS_PIXEL_FORMAT,
		.bufferAddr = (uint32_t)(pSrce + SrceStep * pSrceRect->GetY1() + pSrceRect->GetX1()),
		.pitchBytes = (uint16_t)(SrceStep * sizeof(EG_Color_t))
  };
	PXP_SetAlphaSurfaceBufferConfig(EG_GPU_NXP_PXP_ID, &asBufferConfig);
	PXP_SetAlphaSurfacePosition(EG_GPU_NXP_PXP_ID, 0U, 0U, SrceWidth - 1U, SrceHeight - 1U);
	PXP_SetAlphaSurfaceBlendConfig(EG_GPU_NXP_PXP_ID, &asBlendConfig);
	if(pImage->IsChromaKeyed(ColorFormat)) {
		EG_Color_t colorKeyLow = EG_COLOR_CHROMA_KEY;
		EG_Color_t colorKeyHigh = EG_COLOR_CHROMA_KEY;
    bool HasRecolor = (pImage->m_RecolorOPA != EG_OPA_TRANSP);
		if(HasRecolor) {			// New color key after recoloring
			EG_Color_t colorKey = EG_ColorMix(pImage->m_Recolor, EG_COLOR_CHROMA_KEY, pImage->m_RecolorOPA);
			EG_COLOR_SET_R(colorKeyLow, colorKey.ch.red != 0 ? colorKey.ch.red - 1 : 0);
			EG_COLOR_SET_G(colorKeyLow, colorKey.ch.green != 0 ? colorKey.ch.green - 1 : 0);
			EG_COLOR_SET_B(colorKeyLow, colorKey.ch.blue != 0 ? colorKey.ch.blue - 1 : 0);
#if EG_COLOR_DEPTH == 16
			EG_COLOR_SET_R(colorKeyHigh, colorKey.ch.red != 0x1f ? colorKey.ch.red + 1 : 0x1f);
			EG_COLOR_SET_G(colorKeyHigh, colorKey.ch.green != 0x3f ? colorKey.ch.green + 1 : 0x3f);
			EG_COLOR_SET_B(colorKeyHigh, colorKey.ch.blue != 0x1f ? colorKey.ch.blue + 1 : 0x1f);
#else // EG_COLOR_DEPTH == 32
			EG_COLOR_SET_R(colorKeyHigh, colorKey.ch.red != 0xff ? colorKey.ch.red + 1 : 0xff);
			EG_COLOR_SET_G(colorKeyHigh, colorKey.ch.green != 0xff ? colorKey.ch.green + 1 : 0xff);
			EG_COLOR_SET_B(colorKeyHigh, colorKey.ch.blue != 0xff ? colorKey.ch.blue + 1 : 0xff);
#endif
		}
		PXP_SetAlphaSurfaceOverlayColorKey(EG_GPU_NXP_PXP_ID, EG_ColorTo32(colorKeyLow), EG_ColorTo32(colorKeyHigh));
	}
	PXP_EnableAlphaSurfaceOverlayColorKey(EG_GPU_NXP_PXP_ID, pImage->IsChromaKeyed(ColorFormat));
	pxp_output_buffer_config_t outputBufferConfig = {
		.pixelFormat = (pxp_output_pixel_format_t)PXP_OUT_PIXEL_FORMAT,
		.interlacedMode = kPXP_OutputProgressive,
		.buffer0Addr = (uint32_t)(pDest + DestStep * pDestRect->GetY1() + pDestRect->GetX1()),
		.buffer1Addr = (uint32_t)0U,
		.pitchBytes = (uint16_t)(DestStep * sizeof(EG_Color_t)),
		.width = (uint16_t)DestWidth,
		.height = (uint16_t)DestHeight
  };
	PXP_SetOutputBufferConfig(EG_GPU_NXP_PXP_ID, &outputBufferConfig);
	PXP_RunGPU();
}

#endif
