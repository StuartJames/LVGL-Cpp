/*
 * Copyright (C) 2010-2023 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*********************
 *      INCLUDES
 *********************/
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wimplicit-float-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wfloat-conversion"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wint-conversion"
#endif

#include "draw/arm2d/EG_GPU_ARM2D.h"
#include "core/EG_Refresh.h"

#if EG_USE_GPU_ARM2D

#define __ARM_2D_IMPL__
#include "arm_2d.h"
#include "__arm_2d_impl.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__IS_COMPILER_ARM_COMPILER_5__)
#pragma diag_suppress 174, 177, 188, 68, 513, 144, 1296
#elif defined(__IS_COMPILER_IAR__)
#pragma diag_suppress = Pa093
#elif defined(__IS_COMPILER_GCC__)
//#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#if(!defined(__ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__) || !__ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__) && EG_COLOR_DEPTH == 32 && !defined(__ARM_2D_EGL_CFG_NO_WARNING__)
#warning Please set macro __ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__ to 1 to get more acceleration opportunities. Or you can define macro __ARM_2D_EGL_CFG_NO_WARNING__ to suppress this warning.
#endif

#define MAX_BUF_SIZE (uint32_t) GetRefreshingDisplay()->GetHorizontalRes()

#if EG_COLOR_DEPTH == 16
#define arm_2d_fill_colour arm_2d_rgb16_fill_colour
#define arm_2d_fill_colour_with_alpha arm_2d_rgb565_fill_colour_with_alpha
#define arm_2d_fill_colour_with_mask arm_2d_rgb565_fill_colour_with_mask
#define arm_2d_fill_colour_with_mask_and_opacity arm_2d_rgb565_fill_colour_with_mask_and_opacity
#define arm_2d_tile_copy arm_2d_rgb16_tile_copy
#define arm_2d_alpha_blending arm_2d_rgb565_alpha_blending
#define arm_2d_tile_copy_with_src_mask arm_2d_rgb565_tile_copy_with_src_mask
#define arm_2d_color_t arm_2d_color_rgb565_t

// arm-2d direct mode APIs
#define __arm_2d_impl_colour_filling __arm_2d_impl_rgb16_colour_filling
#define __arm_2d_impl_colour_filling_with_opacity __arm_2d_impl_rgb565_colour_filling_with_opacity
#define __arm_2d_impl_colour_filling_mask __arm_2d_impl_rgb565_colour_filling_mask
#define __arm_2d_impl_colour_filling_mask_opacity __arm_2d_impl_rgb565_colour_filling_mask_opacity
#define __arm_2d_impl_copy __arm_2d_impl_rgb16_copy
#define __arm_2d_impl_alpha_blending __arm_2d_impl_rgb565_alpha_blending
#define __arm_2d_impl_src_msk_copy __arm_2d_impl_rgb565_src_msk_copy
#define __arm_2d_impl_src_chn_msk_copy __arm_2d_impl_rgb565_src_chn_msk_copy
#define __arm_2d_impl_cl_key_copy __arm_2d_impl_rgb16_cl_key_copy
#define __arm_2d_impl_alpha_blending_colour_keying __arm_2d_impl_rgb565_alpha_blending_colour_keying
#define arm_2d_tile_transform_with_src_mask_and_opacity_prepare arm_2dp_rgb565_tile_transform_with_src_mask_and_opacity_prepare
#define arm_2d_tile_transform_with_opacity_prepare arm_2dp_rgb565_tile_transform_with_opacity_prepare
#define arm_2d_tile_transform_only_with_opacity_prepare arm_2dp_rgb565_tile_transform_only_with_opacity_prepare
#define arm_2d_tile_transform_prepare arm_2dp_rgb565_tile_transform_prepare

#define __ARM_2D_PIXEL_BLENDING_OPA __ARM_2D_PIXEL_BLENDING_OPA_RGB565

#define color_int uint16_t

#elif EG_COLOR_DEPTH == 32
#define arm_2d_fill_colour arm_2d_rgb32_fill_colour
#define arm_2d_fill_colour_with_alpha arm_2d_cccn888_fill_colour_with_alpha
#define arm_2d_fill_colour_with_mask arm_2d_cccn888_fill_colour_with_mask
#define arm_2d_fill_colour_with_mask_and_opacity arm_2d_cccn888_fill_colour_with_mask_and_opacity
#define arm_2d_tile_copy arm_2d_rgb32_tile_copy
#define arm_2d_alpha_blending arm_2d_cccn888_alpha_blending
#define arm_2d_tile_copy_with_src_mask arm_2d_cccn888_tile_copy_with_src_mask
#define arm_2d_color_t arm_2d_color_cccn888_t

// arm-2d direct mode APIs
#define __arm_2d_impl_colour_filling __arm_2d_impl_rgb32_colour_filling
#define __arm_2d_impl_colour_filling_with_opacity __arm_2d_impl_cccn888_colour_filling_with_opacity
#define __arm_2d_impl_colour_filling_mask	__arm_2d_impl_cccn888_colour_filling_mask
#define __arm_2d_impl_colour_filling_mask_opacity __arm_2d_impl_cccn888_colour_filling_mask_opacity
#define __arm_2d_impl_copy __arm_2d_impl_rgb32_copy
#define __arm_2d_impl_alpha_blending __arm_2d_impl_cccn888_alpha_blending
#define __arm_2d_impl_src_msk_copy __arm_2d_impl_cccn888_src_msk_copy
#define __arm_2d_impl_src_chn_msk_copy __arm_2d_impl_cccn888_src_chn_msk_copy
#define __arm_2d_impl_cl_key_copy __arm_2d_impl_rgb32_cl_key_copy
#define __arm_2d_impl_alpha_blending_colour_keying __arm_2d_impl_cccn888_alpha_blending_colour_keying
#define arm_2d_tile_transform_with_src_mask_and_opacity_prepare arm_2dp_cccn888_tile_transform_with_src_mask_and_opacity_prepare
#define arm_2d_tile_transform_with_opacity_prepare arm_2dp_cccn888_tile_transform_with_opacity_prepare
#define arm_2d_tile_transform_only_with_opacity_prepare arm_2dp_cccn888_tile_transform_only_with_opacity_prepare
#define arm_2d_tile_transform_prepare arm_2dp_cccn888_tile_transform_prepare

#define __ARM_2D_PIXEL_BLENDING_OPA __ARM_2D_PIXEL_BLENDING_OPA_CCCN888

#define color_int uint32_t

#else
#error The specified EG_COLOR_DEPTH is not supported by this version of EG_GPU_ARM2D.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#define __PREPARE_LL_ACCELERATION__()                                                                                   \
	int32_t SrceStep = pRect->GetWidth();                                                                                 \
	uint8_t px_size_byte = ColorFormat == EG_COLOR_FORMAT_NATIVE_ALPHA ? EG_IMG_PX_SIZE_ALPHA_BYTE : sizeof(EG_Color_t);  \
	const uint8_t *pSrceBufTemp = pSrceBuffer;                                                                            \
	pSrceBufTemp += SrceStep * (DrawRect.GetY1() - pRect->GetY1()) * px_size_byte;                                        \
	pSrceBufTemp += (DrawRect.GetX1() - pRect->GetX1()) * px_size_byte;                                                   \
	EGRect BlendRect2;                                                                                                    \
	if(!BlendRect2.Intersect(&DrawRect, m_pClipRect)) return;                                                             \
	int32_t DestStep = m_pDrawRect->GetWidth();                                                                           \
	EG_Color_t *pDestBuffer = (EG_Color_t*)m_pDrawBuffer;                                                                 \
	pDestBuffer += DestStep * (BlendRect2.GetY1() - m_pDrawRect->GetY1()) + (BlendRect2.GetX1() - m_pDrawRect->GetX1());  \
	arm_2d_size_t copy_size = {                                                                                           \
		.iWidth =  (int16_t)BlendRect2.GetWidth(),                                                                          \
		.iHeight = (int16_t)BlendRect2.GetHeight(),                                                                         \
	}

///////////////////////////////////////////////////////////////////////////////////////

#define __PREPARE_TARGET_TILE__(__BlendRect)                                                        \
	static arm_2d_tile_t TargetTile;                                                                  \
	static arm_2d_region_t TargetRegion;                                                              \
	EG_Color_t *pDestBuffer = (EG_Color_t*)m_pDrawBuffer;                                             \
	TargetTile = (arm_2d_tile_t){                                                                     \
		.tRegion = {                                                                                    \
			.tSize = {                                                                                    \
				.iWidth = m_pDrawRect->GetWidth(),                                                          \
				.iHeight = m_pDrawRect->GetHeight(),                                                        \
			},                                                                                            \
		},                                                                                              \
		.tInfo.bIsRoot = true,                                                                          \
		.phwBuffer = (uint16_t *)m_pDrawBuffer,                                                         \
	};                                                                                                \
	TargetRegion = (arm_2d_region_t){                                                                 \
		.tLocation = {                                                                                  \
			.iX = (__BlendRect).GetX1() - m_pDrawRect->GetX1(),                                           \
			.iY = (__BlendRect).GetY1() - m_pDrawRect->GetY1(),                                           \
		},                                                                                              \
		.tSize = {                                                                                      \
			.iWidth = (__BlendRect).GetWidth(), .iHeight = (__BlendRect).GetHeight(),                     \
		},                                                                                              \
	}

///////////////////////////////////////////////////////////////////////////////////////

#define __PREPARE_SOURCE_TILE__(pBase, __BlendRect)                                                 \
	static arm_2d_tile_t source_tile_orig;                                                            \
	static arm_2d_tile_t SourceTile;                                                                  \
	const EG_Color_t *pSrceBuffer = (pBase)->m_pSourceBuffer;                                         \
	if(pSrceBuffer) {                                                                                 \
		source_tile_orig = (arm_2d_tile_t){                                                             \
			.tRegion = {                                                                                  \
				.tSize = {                                                                                  \
					.iWidth = (int16_t)((pBase)->m_pRect->GetWidth()),                                        \
					.iHeight = (int16_t)((pBase)->m_pRect->GetHeight()),                                      \
				},                                                                                          \
			},                                                                                            \
			.tInfo.bIsRoot = true,                                                                        \
			.phwBuffer = (uint16_t *)pSrceBuffer,                                                         \
		};                                                                                              \
		arm_2d_tile_generate_child(                                                                     \
			&source_tile_orig,                                                                            \
			(arm_2d_region_t[]){                                                                          \
				{                                                                                           \
					.tLocation = {                                                                            \
						.iX = (int16_t)(__BlendRect).GetX1() - (pBase)->m_pRect->GetX1(),                       \
						.iY = (int16_t)(__BlendRect).GetY1() - (pBase)->m_pRect->GetY1(),                       \
					},                                                                                        \
					.tSize = source_tile_orig.tRegion.tSize,                                                  \
				}},                                                                                         \
			&SourceTile, false);                                                                          \
		SourceTile.tInfo.bDerivedResource = true;                                                       \
	}

///////////////////////////////////////////////////////////////////////////////////////

#define __PREPARE_MASK_TILE__(pBase, __BlendRect, __mask, __is_chn)                                   \
	static arm_2d_tile_t mask_tile_orig;                                                                \
	static arm_2d_tile_t MaskTile;                                                                     \
	if(nullptr != (__mask)) {                                                                           \
		mask_tile_orig = (arm_2d_tile_t){                                                                 \
			.tRegion = {                                                                                    \
				.tSize = {                                                                                    \
					.iWidth = (int16_t)(pBase)->m_pMaskRect->GetWidth(),                                        \
					.iHeight = (int16_t)(pBase)->m_pMaskRect->GetHeight(),                                      \
				},                                                                                            \
			},                                                                                              \
			.tInfo = {                                                                                      \
				.bIsRoot = true, .bHasEnforcedColour = true, .tColourInfo = {                                 \
					.chScheme = (__is_chn) ? ARM_2D_CHANNEL_8in32 : ARM_2D_COLOUR_8BIT,                         \
				},                                                                                            \
			},                                                                                              \
			.pchBuffer = ((uint8_t *)(__mask)) + (__is_chn) ? 3 : 0,                                        \
		};                                                                                                \
		arm_2d_tile_generate_child(                                                                       \
			&mask_tile_orig,                                                                                \
			(arm_2d_region_t[]){                                                                            \
				{                                                                                             \
					.tLocation = {                                                                              \
						.iX = (int16_t)((pBase)->m_pMaskRect->GetX1() - (__BlendRect).GetX1()),                   \
						.iY = (int16_t)((pBase)->m_pMaskRect->GetY1() - (__BlendRect).GetY1()),                   \
					},                                                                                          \
					.tSize = mask_tile_orig.tRegion.tSize,                                                      \
				}                                                                                             \
      },                                                                                              \
			&MaskTile, false);                                                                             \
		MaskTile.tInfo.bDerivedResource = true;                                                          \
	}

///////////////////////////////////////////////////////////////////////////////////////

#define __RECOLOUR_WRAPPER(...)                                                                       \
	do {                                                                                                \
		EG_Color_t *pTempBufRGB = nullptr;                                                                \
		if(pDrawImage->m_RecolorOPA > EG_OPA_MIN) {                                                       \
			pTempBufRGB = (EG_Color_t*)EG_GetBufferMem(SrceWidth * SrceHeight * sizeof(EG_Color_t));        \
			if(nullptr == pTempBufRGB) {                                                                    \
				EG_LOG_WARN("Failed to allocate memory for accelerating recolour, "                           \
					"use normal route instead.");                                                               \
				break;                                                                                        \
			}                                                                                               \
			EG_CopyMem(pTempBufRGB, pSrceBuffer, SrceWidth *SrceHeight * sizeof(EG_Color_t));               \
			arm_2d_size_t copy_size = {                                                                     \
				.iWidth = (int16_t)SrceWidth,                                                                 \
				.iHeight = (int16_t)SrceHeight,                                                               \
			};                                                                                              \
			__arm_2d_impl_colour_filling_with_opacity((color_int*)pTempBufRGB, SrceWidth,  &copy_size,      \
				(color_int)pDrawImage->m_Recolor.full, pDrawImage->m_RecolorOPA);                             \
			pSrceBuffer = (const uint8_t*)pTempBufRGB;                                                      \
		}                                                                                                 \
		do {                                                                                              \
			__VA_ARGS__                                                                                     \
		} while(0);                                                                                       \
		if(nullptr != pTempBufRGB) {                                                                      \
			EG_ReleaseBufferMem(pTempBufRGB);                                                               \
		}                                                                                                 \
	} while(0);                                                                                         \
	pSrceBuffer = pSrceBufOrig;

///////////////////////////////////////////////////////////////////////////////////////

#define __RECOLOUR_BEGIN()                                                                            \
	do {                                                                                                \
		EG_Color_t *pTempBufRGB = nullptr;                                                                \
		if(pDrawImage->m_RecolorOPA > EG_OPA_MIN) {                                                       \
			pTempBufRGB = (EG_Color_t*)EG_GetBufferMem(SrceWidth * SrceHeight * sizeof(EG_Color_t));        \
			if(nullptr == pTempBufRGB) {                                                                    \
				EG_LOG_WARN("Failed to allocate memory for accelerating recolour, "                           \
					"use normal route instead.");                                                               \
				break;                                                                                        \
			}                                                                                               \
			EG_CopyMem(pTempBufRGB, pSrceBuffer, SrceWidth *SrceHeight * sizeof(EG_Color_t));               \
			arm_2d_size_t copy_size = {                                                                     \
				.iWidth = SrceWidth,                                                                          \
				.iHeight = SrceHeight,                                                                        \
			};                                                                                              \
			__arm_2d_impl_colour_filling_with_opacity(                                                      \
				(color_int*)pTempBufRGB,                                                                      \
				SrceWidth,                                                                                    \
				&copy_size,                                                                                   \
				(color_int)pDrawImage->m_Recolor.full,                                                        \
				pDrawImage->m_RecolorOPA);                                                                    \
			pSrceBuffer = (const uint8_t*)pTempBufRGB;                                                      \
		}                                                                                                 \
		do {

///////////////////////////////////////////////////////////////////////////////////////

#define __RECOLOUR_END()                                                              \
	}                                                                                   \
	while(0);                                                                           \
	if(nullptr != pTempBufRGB) {                                                        \
		EG_ReleaseBufferMem(pTempBufRGB);                                                 \
	}                                                                                   \
	}                                                                                   \
	while(0);                                                                           \
	pSrceBuffer = pSrceBufOrig;

  ///////////////////////////////////////////////////////////////////////////////////////

#define __ARM_2D_PREPARE_TRANS_AND_TARGET_REGION(__TRANS_PREPARE, ...)                \
	do {                                                                                \
		__TRANS_PREPARE(                                                                  \
			nullptr,                                                                        \
			__VA_ARGS__);                                                                   \
		TargetRegion = (arm_2d_region_t){                                                 \
			.tLocation = {                                                                  \
				.iX = (int16_t)(pRect->GetX1() - m_pClipRect->GetX1()),                       \
				.iY = (int16_t)(pRect->GetY1() - m_pClipRect->GetY1()),                       \
			},                                                                              \
			.tSize = {                                                                      \
				.iWidth = (int16_t)pRect->GetWidth(),                                         \
        .iHeight = (int16_t)pRect->GetHeight(),                                       \
			},                                                                              \
		};                                                                                \
		arm_2d_size_t tTransSize = ARM_2D_CTRL.DefaultOP                                  \
																 .tTransform.Source.ptTile->tRegion.tSize;            \
                                                                                      \
		if(TargetRegion.tSize.iWidth < tTransSize.iWidth) {                               \
			int16_t iDelta = tTransSize.iWidth - TargetRegion.tSize.iWidth;                 \
			TargetRegion.tLocation.iX -= iDelta / 2;                                        \
			TargetRegion.tSize.iWidth = tTransSize.iWidth;                                  \
		}                                                                                 \
		if(TargetRegion.tSize.iHeight < tTransSize.iHeight) {                             \
			int16_t iDelta = tTransSize.iHeight - TargetRegion.tSize.iHeight;               \
			TargetRegion.tLocation.iY -= iDelta / 2;                                        \
			TargetRegion.tSize.iHeight = tTransSize.iHeight;                                \
		}                                                                                 \
	} while(0)

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////

void EGARM2DContext::InitialiseContext(void)
{
	arm_2d_init();
  EGSoftContext::InitialiseContext();   // call the base class
	BlendProc = Blend;
	WaitForFinishProc = WaitForFinish;
#if !__ARM_2D_HAS_HW_ACC__
  DrawImageDecodedProc = DrawImageDecoded;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

extern void test_flush(EG_Color_t *color_p);

///////////////////////////////////////////////////////////////////////////////////////

#if __ARM_2D_HAS_HW_ACC__
void EG_ATTRIBUTE_FAST_MEM EGARM2DContext::Blend(EGBlendBase *pBlend);
{
const EG_OPA_t *pMask;

  EGARM2DContext *pDC = (EGARM2DContext*)pBlend->m_pContext;
	if(pBlend->m_pMaskBuffer == nullptr) pMask = nullptr;
	if(pBlend->m_pMaskBuffer && pBlend->m_MaskResult == EG_DRAW_MASK_RESULT_TRANSP)	return;
	else if(pBlend->m_MaskResult == EG_DRAW_MASK_RESULT_FULL_COVER)	pMask = nullptr;
	else pMask = pBlend->m_pMaskBuffer;
	EGRect BlendRect;
	if(!BlendRect.Intersect(pBlend->m_pBlendRect, m_pClipRect)) {
		return;
	}
	bool IsAccelerated = false;
	if((pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL) && (BlendRect.GetSize() > 100)) {
		__PREPARE_TARGET_TILE__(BlendRect);
		__PREPARE_SOURCE_TILE__(pBlend, BlendRect);
		__PREPARE_MASK_TILE__(pBlend, BlendRect, pMask, false);
		if(pSrceBuffer) {
			IsAccelerated = pDC->TileCopy(&TargetTile, &TargetRegion,	&SourceTile, pBlend->m_OPA,	(nullptr == pMask) ? nullptr : &MaskTile);
		}
		else {
			IsAccelerated = pDC->FillColour(&TargetTile, &TargetRegion, pBlend->color, pBlend->m_OPA,	(nullptr == pMask) ? nullptr : &MaskTile);
		}
	}
	if(!IsAccelerated) {
		EGSoftBlend::BlendBasic(pBlend);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

bool EG_ATTRIBUTE_FAST_MEM EGARM2DContext::FillColour(const arm_2d_tile_t *TargetTile, const arm_2d_region_t *region, EG_Color_t color, EG_OPA_t OPA, const arm_2d_tile_t *MaskTile)
{
	arm_fsm_rt_t Result = (arm_fsm_rt_t)ARM_2D_ERR_NONE;
	if(nullptr == MaskTile) {
		if(OPA >= EG_OPA_MAX) Result = arm_2d_fill_colour(TargetTile, region, color.full);
		else {
#if EG_COLOR_SCREEN_TRANSP
			return false;
#else
			Result = arm_2d_fill_colour_with_alpha(TargetTile, region, (arm_2d_color_t){color.full}, OPA);
#endif
		}
	}
	else {
		if(OPA >= EG_OPA_MAX) {
			Result = arm_2d_fill_colour_with_mask(TargetTile, region, MaskTile, (arm_2d_color_t){color.full});
		}
		else {
#if EG_COLOR_SCREEN_TRANSP
			return false;
#else
			Result = arm_2d_fill_colour_with_mask_and_opacity(TargetTile, region, MaskTile, (arm_2d_color_t){color.full}, OPA);
#endif
		}
	}
	if(Result < 0) return false;	// error detected
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EG_ATTRIBUTE_FAST_MEM EGARM2DContext::TileCopy(const arm_2d_tile_t *TargetTile, const arm_2d_region_t *region, arm_2d_tile_t *SourceTile, EG_OPA_t OPA, arm_2d_tile_t *MaskTile)
{
	arm_fsm_rt_t Result = (arm_fsm_rt_t)ARM_2D_ERR_NONE;
	if(nullptr == MaskTile) {
		if(OPA >= EG_OPA_MAX) {
			Result = arm_2d_tile_copy(SourceTile, TargetTile, region, ARM_2D_CP_MODE_COPY);
		}
#if EG_COLOR_SCREEN_TRANSP
		else return false; // not supported
#else
		else Result = arm_2d_alpha_blending(SourceTile, TargetTile, region, OPA);
#endif
	}
	else {
#if EG_COLOR_SCREEN_TRANSP
		return false; // not support
#else
		if(OPA >= EG_OPA_MAX) Result = arm_2d_tile_copy_with_src_mask(SourceTile, MaskTile, TargetTile, region, ARM_2D_CP_MODE_COPY);
		else return false;
#endif
	}
	if(Result < 0) return false;	// error detected
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGARM2DContext::WaitForFinish(void)
{
  EGDisplay *pDisp = GetRefreshingDisplay();
	arm_2d_op_wait_async(nullptr);
	if(pDisp->m_pDriver && pDisp->m_pDriver->WaitCB) {
		pDisp->m_pDriver->WaitCB(pDisp->m_pDriver);
	}
	SoftWaitForFinish();
}

#else

///////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGARM2DContext::Blend(EGBlendBase *pBlend)
{
const EG_OPA_t *pMask;

  EGARM2DContext *pDC = (EGARM2DContext*)pBlend->m_pContext;
	if(pBlend->m_pMaskBuffer == nullptr) pMask = nullptr;
	if(pBlend->m_pMaskBuffer && pBlend->m_MaskResult == EG_DRAW_MASK_RESULT_TRANSP)
		return;
	else if(pBlend->m_MaskResult == EG_DRAW_MASK_RESULT_FULL_COVER)	pMask = nullptr;
	else pMask = pBlend->m_pMaskBuffer;
	int32_t DestStep = pDC->m_pDrawRect->GetWidth();
	EGRect BlendRect;
	if(!BlendRect.Intersect(pBlend->m_pRect, pDC->m_pClipRect)) return;
	//lv_disp_t * pDisp = _lv_refr_get_disp_refreshing();
	bool IsAccelerated = false;
	do {
		// target buffer
		EG_Color_t *pDestBuffer = (EG_Color_t*)pDC->m_pDrawBuffer;
    EGDisplay *pDisp = GetRefreshingDisplay();
		if(pDisp->m_pDriver->m_ScreenTransparent == 0) {
			pDestBuffer += DestStep * (BlendRect.GetY1() - pDC->m_pDrawRect->GetY1()) + (BlendRect.GetX1() - pDC->m_pDrawRect->GetX1());
		}
		else {
			// With EG_COLOR_DEPTH 16 it means ARGB8565 (3 bytes format)
			uint8_t *pDestBuffer8 = (uint8_t *)pDestBuffer;
			pDestBuffer8 += DestStep * (BlendRect.GetY1() - pDC->m_pDrawRect->GetY1()) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			pDestBuffer8 += (BlendRect.GetX1() - pDC->m_pDrawRect->GetX1()) * EG_IMG_PX_SIZE_ALPHA_BYTE;
			pDestBuffer = (EG_Color_t *)pDestBuffer8;
		}
		// source buffer
		const EG_Color_t *pSrceBuffer = pBlend->m_pSourceBuffer;
		int32_t SrceStep;
		if(pSrceBuffer) {
			SrceStep = pBlend->m_pRect->GetWidth();
			pSrceBuffer += SrceStep * (BlendRect.GetY1() - pBlend->m_pRect->GetY1()) + (BlendRect.GetX1() - pBlend->m_pRect->GetX1());
		}
		else SrceStep = 0;
		int32_t MaskStep;
		if(pMask) {
			MaskStep = pBlend->m_pMaskRect->GetWidth();
			pMask += MaskStep * (BlendRect.GetY1() - pBlend->m_pMaskRect->GetY1()) + (BlendRect.GetX1() - pBlend->m_pMaskRect->GetX1());
		}
		else MaskStep = 0;
		BlendRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
		if(pDisp->m_pDriver->m_ScreenTransparent) break;
		if(pBlend->m_pSourceBuffer == nullptr) {
			if(pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL) {
				IsAccelerated = pDC->FillNormal(pDestBuffer, &BlendRect, DestStep, pBlend->m_Color, pBlend->m_OPA, pMask, MaskStep);
			}
		}
		else {
			if(pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL) {
				IsAccelerated = pDC->CopyNormal(pDestBuffer, &BlendRect, DestStep, pSrceBuffer, SrceStep, pBlend->m_OPA, pMask, MaskStep);
			}
		}
	} while(0);
	if(!IsAccelerated) EGSoftBlend::BlendBasic(pBlend);
}

///////////////////////////////////////////////////////////////////////////////////////

bool EG_ATTRIBUTE_FAST_MEM EGARM2DContext::FillNormal(EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep, EG_Color_t Color,
                                                      EG_OPA_t OPA, const EG_OPA_t *pMask, int32_t MaskStep)
{
	arm_2d_size_t target_size = {
		.iWidth = (int16_t)pDestRect->GetWidth(),
		.iHeight = (int16_t)pDestRect->GetHeight(),
	};
	if(pMask == nullptr) {    // No pMask 
		if(OPA >= EG_OPA_MAX) __arm_2d_impl_colour_filling((color_int *)pDestBuffer, DestStep, &target_size, Color.full);
		else 	__arm_2d_impl_colour_filling_with_opacity((color_int *)pDestBuffer, DestStep, &target_size, Color.full, OPA); // Has opacity 
	}
	else {    // Masked
		// Only the pMask matters
		if(OPA >= EG_OPA_MAX) __arm_2d_impl_colour_filling_mask((color_int *)pDestBuffer, DestStep, (uint8_t *)pMask, MaskStep, &target_size, Color.full);
		else __arm_2d_impl_colour_filling_mask_opacity((color_int *)pDestBuffer, DestStep, (uint8_t *)pMask, MaskStep, &target_size, Color.full, OPA);  // With opacity
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EG_ATTRIBUTE_FAST_MEM EGARM2DContext::CopyNormal(EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrceBuffer, int32_t SrceStep, 
                                                      EG_OPA_t OPA, const EG_OPA_t *pMask, int32_t MaskStep)
{
arm_2d_size_t copy_size = {
  .iWidth = pDestRect->GetWidth(),
  .iHeight = pDestRect->GetHeight(),
};

	if(pMask == nullptr) {    // Simple fill (maybe with opacity), no masking
		if(OPA >= EG_OPA_MAX) __arm_2d_impl_copy((color_int *)pSrceBuffer, SrceStep, (color_int *)pDestBuffer, DestStep, &copy_size);
		else __arm_2d_impl_alpha_blending((color_int *)pSrceBuffer, SrceStep, (color_int *)pDestBuffer, DestStep, &copy_size, OPA);
	}
	else {    // Masked
		if(OPA > EG_OPA_MAX) __arm_2d_impl_src_msk_copy((color_int *)pSrceBuffer, SrceStep, (uint8_t *)pMask, MaskStep, &copy_size, (color_int *)pDestBuffer, DestStep, &copy_size);
		else {     // Handle OPA and pMask values too
			__arm_2d_impl_gray8_colour_filling_with_opacity((uint8_t *)pMask, MaskStep, &copy_size, 0x00, 255 - OPA);
			__arm_2d_impl_src_msk_copy((color_int *)pSrceBuffer, SrceStep, (uint8_t *)pMask, MaskStep, &copy_size, (color_int *)pDestBuffer, DestStep, &copy_size);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGARM2DContext::DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSrceBuffer, EG_ImageColorFormat_t ColorFormat)
{
  EGARM2DContext *pDC = (EGARM2DContext*)pDrawImage->m_pContext;
	pDC->PaintImage(pDrawImage, pRect, pSrceBuffer, ColorFormat);
}

///////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGARM2DContext::PaintImage(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSrceBuffer, EG_ImageColorFormat_t ColorFormat)
{
	// Use the clip area as draw area
	EGRect DrawRect(m_pClipRect);
	const uint8_t *pSrceBufOrig = pSrceBuffer;
	bool MaskAny = HasAnyDrawMask(&DrawRect);
	bool Transform = pDrawImage->m_Angle != 0 || pDrawImage->m_Scale.IsScaled() ? true : false;
	EGRect BlendRect;
	EGSoftBlend BlendObj((EGSoftContext*)pDrawImage->m_pContext);
	BlendObj.m_OPA = pDrawImage->m_OPA;
	BlendObj.m_BlendMode = pDrawImage->m_BlendMode;
	BlendObj.m_pRect = &BlendRect;
	if(pDrawImage->IsChromaKeyed(ColorFormat))	ColorFormat = EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED;
	else if(ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT) {
	}
	else if(ColorFormat == EG_COLOR_FORMAT_RGB565A8) {
	}
	else if(EGDrawImage::HasAlpha(ColorFormat)) ColorFormat = EG_COLOR_FORMAT_NATIVE_ALPHA;
	else ColorFormat = EG_COLOR_FORMAT_NATIVE;
	// The simplest case just copy the pixels into the draw_buf
	if(!MaskAny && !Transform && ColorFormat == EG_COLOR_FORMAT_NATIVE && pDrawImage->m_RecolorOPA == EG_OPA_TRANSP) {
		BlendObj.m_pSourceBuffer = (const EG_Color_t *)pSrceBuffer;
		BlendObj.m_pRect = pRect;
		BlendProc(&BlendObj);
	}
	else if(!MaskAny && !Transform && ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT) {
		EGRect ClippedRect;
		if(!ClippedRect.Intersect(pRect, m_pClipRect)) return;
		BlendObj.m_pMaskBuffer = (EG_OPA_t *)pSrceBuffer;
		BlendObj.m_pMaskRect = pRect;
		BlendObj.m_pSourceBuffer = nullptr;
		BlendObj.m_Color = pDrawImage->m_Recolor;
		BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
		BlendObj.m_pRect = pRect;
		BlendProc(&BlendObj);
	}
#if EG_COLOR_DEPTH == 16
	else if(!MaskAny && !Transform && ColorFormat == EG_COLOR_FORMAT_RGB565A8 && pDrawImage->m_RecolorOPA == EG_OPA_TRANSP && BlendObj.m_OPA >= EG_OPA_MAX) {
		int32_t SrceWidth = pRect->GetWidth();
		int32_t SrceHeight = pRect->GetHeight();
		BlendObj.m_pSourceBuffer = (const EG_Color_t *)pSrceBuffer;
		BlendObj.m_pMaskBuffer = (EG_OPA_t *)pSrceBuffer;
		BlendObj.m_pMaskBuffer += sizeof(EG_Color_t) * SrceWidth * SrceHeight;
		BlendObj.m_pRect = pRect;
		BlendObj.m_pMaskRect = pRect;
		BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
		BlendProc(&BlendObj);
	}
#endif
	// In the other cases every pixel need to be checked one-by-one
	else {
		BlendRect = m_pClipRect;
		int32_t SrceWidth = pRect->GetWidth();
		int32_t SrceHeight = pRect->GetHeight();
		int32_t BlendHeight = BlendRect.GetHeight();
		int32_t BlendWidth = BlendRect.GetWidth();
		uint32_t MaxBufferSize = MAX_BUF_SIZE;
		uint32_t BlendSize = BlendRect.GetSize();
		uint32_t BufferHeight;
		uint32_t BufferWidth = BlendWidth;
		if(BlendSize <= MaxBufferSize) {
			BufferHeight = BlendHeight;
		}
		else BufferHeight = MaxBufferSize / BlendWidth;			// Round to full lines
		// Create buffers and masks
		uint32_t BufferSize = BufferWidth * BufferHeight;
		EG_Color_t *pBufferRGB = (EG_Color_t*)EG_GetBufferMem(BufferSize * sizeof(EG_Color_t));
		EG_OPA_t *m_pMaskBuffer = (EG_OPA_t*)EG_GetBufferMem(BufferSize);
		BlendObj.m_pMaskBuffer = m_pMaskBuffer;
		BlendObj.m_pMaskRect = &BlendRect;
		BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
		BlendObj.m_pSourceBuffer = pBufferRGB;
		int32_t y_last = BlendRect.GetY2();
		BlendRect.SetY2(BlendRect.GetY1() + BufferHeight - 1);
		BlendObj.m_MaskResult = (ColorFormat != EG_COLOR_FORMAT_NATIVE || pDrawImage->m_Angle || pDrawImage->m_Scale.IsScaled()) ?
			                                  EG_DRAW_MASK_RESULT_CHANGED :	EG_DRAW_MASK_RESULT_FULL_COVER;
		if(ColorFormat == EG_COLOR_FORMAT_ALPHA_8BIT) {
			// original code: lv_color_fill(pBufferRGB, pDrawImage->m_Recolor, BufferSize);
			arm_2d_size_t copy_size = {
				.iWidth = (int16_t)BufferWidth,
				.iHeight = (int16_t)BufferHeight,
			};
			__arm_2d_impl_colour_filling((color_int *)pBufferRGB, BufferWidth, &copy_size, (color_int)pDrawImage->m_Recolor.full);		// apply re-colour 
		}
    int32_t Scale = pDrawImage->m_Scale.m_X;
    bool IsAccelerated = false;
		if(!Transform) {
			if(EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED == ColorFormat) {
				__RECOLOUR_WRAPPER(EG_Color_t chrome_key = EG_COLOR_CHROMA_KEY;					// calculate new chrome-key colour
					if(pDrawImage->m_RecolorOPA > EG_OPA_MIN) {
						__ARM_2D_PIXEL_BLENDING_OPA((color_int *)&(pDrawImage->m_Recolor.full), (color_int *)&(chrome_key.full), pDrawImage->m_RecolorOPA);
					}
					__PREPARE_LL_ACCELERATION__();
					if(BlendObj.m_OPA >= EG_OPA_MAX) {
						__arm_2d_impl_cl_key_copy((color_int *)pSrceBufTemp, SrceStep, (color_int *)pDestBuffer, DestStep, &copy_size, (color_int)chrome_key.full);
					}
          else {
						__arm_2d_impl_alpha_blending_colour_keying((color_int *)pSrceBufTemp, SrceStep, (color_int *)pDestBuffer, DestStep, &copy_size, BlendObj.m_OPA, (color_int)chrome_key.full);
					}
          IsAccelerated = true;)
			}
			else if((EG_COLOR_DEPTH == 32) && !MaskAny && (EG_COLOR_FORMAT_NATIVE_ALPHA == ColorFormat)) {
				__RECOLOUR_WRAPPER(__PREPARE_LL_ACCELERATION__();
					uint8_t *pMaskBufTemp = nullptr;
					if(BlendObj.m_OPA < EG_OPA_MAX) {
						pMaskBufTemp = (uint8_t*)EG_GetBufferMem(copy_size.iHeight * copy_size.iWidth);
						if(nullptr == pMaskBufTemp) {
							EG_LOG_WARN("Failed to allocate memory for alpha pMask, use normal route instead.");
							break;
						}
						EG_ZeroMem(pMaskBufTemp, copy_size.iHeight * copy_size.iWidth);
						__arm_2d_impl_gray8_colour_filling_channel_mask_opacity(pMaskBufTemp, SrceStep, (uint32_t *)((uintptr_t)pSrceBufTemp + EG_IMG_PX_SIZE_ALPHA_BYTE - 1), SrceStep, &copy_size, 0xFF, BlendObj.m_OPA);
						__arm_2d_impl_src_msk_copy((color_int *)pSrceBufTemp, SrceStep, pMaskBufTemp, SrceStep, &copy_size, (color_int *)pDestBuffer, DestStep, &copy_size);
						EG_ReleaseBufferMem(pMaskBufTemp);
					}
          else {
						__arm_2d_impl_src_chn_msk_copy((color_int *)pSrceBufTemp, SrceStep, (uint32_t *)((uintptr_t)pSrceBufTemp + EG_IMG_PX_SIZE_ALPHA_BYTE - 1), SrceStep, &copy_size, (color_int *)pDestBuffer, DestStep, &copy_size);
					}
					IsAccelerated = true;)
			}
			else if(!MaskAny && (EG_COLOR_FORMAT_RGB565A8 == ColorFormat)) {
				// accelerate copy-with-source-masks-and-opacity 
				uint8_t *pMaskAfterRGB = const_cast<uint8_t*>(pSrceBuffer + sizeof(EG_Color_t) * SrceWidth * SrceHeight);
        __RECOLOUR_WRAPPER(__PREPARE_LL_ACCELERATION__();
        uint8_t *pMaskBufTemp = nullptr;
        if(BlendObj.m_OPA < EG_OPA_MAX) {
          pMaskBufTemp = (uint8_t*)EG_GetBufferMem(copy_size.iHeight * copy_size.iWidth);
          if(nullptr == pMaskBufTemp) {
            EG_LOG_WARN("Failed to allocate memory for alpha pMask, use normal route instead.");
            break;
          }
          EG_ZeroMem(pMaskBufTemp, copy_size.iHeight * copy_size.iWidth);
          __arm_2d_impl_gray8_colour_filling_mask_opacity(pMaskBufTemp, SrceStep, pMaskAfterRGB, SrceStep, &copy_size, 0xFF, BlendObj.m_OPA);
          __arm_2d_impl_src_msk_copy((color_int*)pSrceBufTemp, SrceStep, pMaskBufTemp, SrceStep, &copy_size, (color_int*)pDestBuffer, DestStep, &copy_size);
          EG_ReleaseBufferMem(pMaskBufTemp);
        }
        else {
          __arm_2d_impl_src_msk_copy((color_int*)pSrceBufTemp, SrceStep, pMaskAfterRGB, SrceStep, &copy_size, (color_int*)pDestBuffer, DestStep, &copy_size);
        }
        IsAccelerated = true;)
			}
			else if(!MaskAny && (ColorFormat == EG_COLOR_FORMAT_NATIVE)) {
				// accelerate copy-with-source-masks-and-opacity 
				__RECOLOUR_WRAPPER(__PREPARE_LL_ACCELERATION__();
					if(BlendObj.m_OPA >= EG_OPA_MAX) {
						__arm_2d_impl_copy((color_int*)pSrceBufTemp, SrceStep, (color_int*)pDestBuffer, DestStep, &copy_size);
					}
          else {
						__arm_2d_impl_alpha_blending((color_int*)pSrceBufTemp, SrceStep, (color_int*)pDestBuffer, DestStep, &copy_size, BlendObj.m_OPA);
					}
          IsAccelerated = true;)
			}
		}
		else if(!MaskAny
#if defined(__ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__) && __ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__
			&& (pDrawImage->m_AntiAlias == 1)
#else
			&& (pDrawImage->m_AntiAlias == 0)
#endif
			&& (pDrawImage->m_RecolorOPA == EG_OPA_TRANSP) && (((EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED == ColorFormat) || (EG_COLOR_FORMAT_NATIVE == ColorFormat)) || (EG_COLOR_FORMAT_RGB565A8 == ColorFormat)
#if defined(__ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__) && __ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__
			|| ((EG_COLOR_FORMAT_NATIVE_ALPHA == ColorFormat) && (EG_COLOR_DEPTH == 32))
#endif
			)) {
			uint8_t *pMaskAfterRGB = const_cast<uint8_t*>(pSrceBuffer + sizeof(EG_Color_t) * SrceWidth * SrceHeight);
			__RECOLOUR_WRAPPER(	// accelerate Transform without re-color
				static arm_2d_tile_t TargetTileOrigin = {0};
				static arm_2d_tile_t TargetTile = {0};
				static arm_2d_region_t TargetRegion = {0};
				static arm_2d_tile_t SourceTile = {0};
				static arm_2d_location_t SourceCenter = {0};
//				EG_Color_t *pDestBuffer = (EG_Color_t*)m_pDrawBuffer;
        arm_2d_region_t ClipRegion;
        TargetTileOrigin.tInfo.bIsRoot = true;
        TargetTileOrigin.tRegion.tSize.iWidth = (int16_t)m_pDrawRect->GetWidth();
        TargetTileOrigin.tRegion.tSize.iHeight = (int16_t)m_pDrawRect->GetHeight();
				TargetTileOrigin.phwBuffer = (uint16_t *)m_pDrawBuffer,
				ClipRegion.tLocation.iX = (int16_t)(m_pClipRect->GetX1() - m_pDrawRect->GetX1());
        ClipRegion.tLocation.iY = (int16_t)(m_pClipRect->GetY1() - m_pDrawRect->GetY1());
				ClipRegion.tSize.iWidth = (int16_t)m_pClipRect->GetWidth();
        ClipRegion.tSize.iHeight = (int16_t)m_pClipRect->GetHeight();
				arm_2d_tile_generate_child(&TargetTileOrigin, &ClipRegion, &TargetTile, false);
				SourceTile.tInfo.bIsRoot = true;
				SourceTile.tRegion.tSize.iWidth = (int16_t)SrceWidth;
        SourceTile.tRegion.tSize.iHeight = (int16_t)SrceHeight;
				SourceTile.pchBuffer = (uint8_t *)pSrceBuffer,
        SourceCenter.iX = pDrawImage->m_Pivot.m_X;
        SourceCenter.iY = pDrawImage->m_Pivot.m_Y;
				if(EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED == ColorFormat) {
					__ARM_2D_PREPARE_TRANS_AND_TARGET_REGION(arm_2d_tile_transform_with_opacity_prepare, &SourceTile, SourceCenter, ARM_2D_ANGLE((pDrawImage->m_Angle / 10.0f)), Scale / 256.0f, (color_int)EG_COLOR_CHROMA_KEY.full, BlendObj.m_OPA);
					arm_2d_tile_transform(&TargetTile, &TargetRegion, nullptr);
					IsAccelerated = true;
				}
#if ARM_2D_VERISON >= 10103
				else if(EG_COLOR_FORMAT_NATIVE == ColorFormat) {
					__ARM_2D_PREPARE_TRANS_AND_TARGET_REGION(arm_2d_tile_transform_only_with_opacity_prepare, &SourceTile, SourceCenter, ARM_2D_ANGLE((pDrawImage->m_Angle / 10.0f)), Scale / 256.0f, BlendObj.m_OPA);
					arm_2d_tile_transform(&TargetTile, &TargetRegion, nullptr);
					IsAccelerated = true;
				}
#endif
				else if(EG_COLOR_FORMAT_RGB565A8 == ColorFormat) {
					static arm_2d_tile_t MaskTile = {0};
					MaskTile = SourceTile;
					MaskTile.tInfo.bHasEnforcedColour = true;
					MaskTile.tInfo.tColourInfo.chScheme = ARM_2D_COLOUR_GRAY8;
					MaskTile.pchBuffer = pMaskAfterRGB;
					__ARM_2D_PREPARE_TRANS_AND_TARGET_REGION(arm_2d_tile_transform_with_src_mask_and_opacity_prepare, &SourceTile, &MaskTile, SourceCenter, ARM_2D_ANGLE((pDrawImage->m_Angle / 10.0f)), Scale / 256.0f, BlendObj.m_OPA);
					arm_2d_tile_transform(&TargetTile, &TargetRegion, nullptr);
					IsAccelerated = true;
				}
#if defined(__ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__) && __ARM_2D_CFG_SUPPORT_COLOUR_CHANNEL_ACCESS__
				else if((EG_COLOR_FORMAT_NATIVE_ALPHA == ColorFormat) && (EG_COLOR_DEPTH == 32)) {
					static arm_2d_tile_t MaskTile = {0};
					MaskTile = SourceTile;
					MaskTile.tInfo.bHasEnforcedColour = true;
					MaskTile.tInfo.tColourInfo.chScheme = ARM_2D_CHANNEL_8in32;
					MaskTile.pchBuffer += 3;
					__ARM_2D_PREPARE_TRANS_AND_TARGET_REGION(arm_2d_tile_transform_with_src_mask_and_opacity_prepare, &SourceTile, &MaskTile, SourceCenter, ARM_2D_ANGLE((pDrawImage->m_Angle / 10.0f)), Scale / 256.0f, BlendObj.m_OPA);
					arm_2d_tile_transform(&TargetTile, &TargetRegion, nullptr);
					IsAccelerated = true;
				}
#endif
			)
		}
		if(!IsAccelerated)
			while(BlendRect.GetY1() <= y_last) {
				// Apply transformations if any or separate the channels
				EGRect TransformRect(BlendRect);
				TransformRect.Move(-pRect->GetX1(), -pRect->GetY1());
				if(Transform) {
					DrawTransform(&TransformRect, pSrceBuffer, SrceWidth, SrceHeight, SrceWidth,	pDrawImage, ColorFormat, pBufferRGB, m_pMaskBuffer);
				}
				else {
					Convert(&TransformRect, pSrceBuffer, SrceWidth, SrceHeight, SrceWidth, pDrawImage, ColorFormat, pBufferRGB, m_pMaskBuffer);
				}
				// Apply recolor
				if(pDrawImage->m_RecolorOPA > EG_OPA_MIN) {
					arm_2d_size_t copy_size = {
						.iWidth = (int16_t)BufferWidth,
						.iHeight = (int16_t)BufferHeight,
					};
					// apply re-colour
					__arm_2d_impl_colour_filling_with_opacity((color_int *)pBufferRGB, BufferWidth, &copy_size, (color_int)pDrawImage->m_Recolor.full, pDrawImage->m_RecolorOPA);
				}
#if EG_USE_DRAW_MASKS
				// Apply the masks if any
				if(MaskAny) {
					int32_t y;
					EG_OPA_t *pMaskBufferTemp = m_pMaskBuffer;
					for(y = BlendRect.GetY1(); y <= BlendRect.y2; y++) {
						DrawMaskRes_t mask_res_line;
						mask_res_line = DrawMaskApply(pMaskBufferTemp, BlendRect.GetX1(), y, BlendWidth);
						if(mask_res_line == EG_DRAW_MASK_RESULT_TRANSP) {
							EG_ZeroMem(pMaskBufferTemp, BlendWidth);
							BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
						}
						else if(mask_res_line == EG_DRAW_MASK_RESULT_CHANGED) {
							BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
						}
						pMaskBufferTemp += BlendWidth;
					}
				}
#endif
				BlendProc(&BlendObj);
				BlendRect.SetY1(BlendRect.GetY2() + 1);
				BlendRect.SetY2(BlendRect.GetY1() + BufferHeight - 1);
				if(BlendRect.GetY2() > y_last) BlendRect.SetY2(y_last);
			}
		EG_ReleaseBufferMem(m_pMaskBuffer);
		EG_ReleaseBufferMem(pBufferRGB);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGARM2DContext::WaitForFinish(void)
{
  EGDisplay *pDisp = GetRefreshingDisplay();
	arm_2d_op_wait_async(nullptr);
	if(pDisp->m_pDriver && pDisp->m_pDriver->WaitCB) {
		pDisp->m_pDriver->WaitCB(pDisp->m_pDriver);
	}
	SoftWaitForFinish();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////

// Separate the image channels to RGB and Alpha to match EG_COLOR_DEPTH settings
void EGARM2DContext::Convert(const EGRect *pDestRect, const void *pSrceBuffer, int32_t SrceWidth, int32_t SrceHeight, int32_t SrceStep,
                              const EGDrawImage *pDrawImage, EG_ImageColorFormat_t ColorFormat, EG_Color_t *pColorBuf, EG_OPA_t *abuf)
{
EG_UNUSED(pDrawImage);

	const uint8_t *pSrce8 = (const uint8_t *)pSrceBuffer;
	int32_t y;
	int32_t x;
	if(ColorFormat == EG_COLOR_FORMAT_NATIVE || ColorFormat == EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED) {
		uint32_t PixelCount = pDestRect->GetSize();
		EG_SetMem(abuf, 0xff, PixelCount);
		pSrce8 += (SrceStep * pDestRect->GetY1() * sizeof(EG_Color_t)) + pDestRect->GetX1() * sizeof(EG_Color_t);
		uint32_t DestWidth = pDestRect->GetWidth();
		uint32_t DestWidthBytes = DestWidth * sizeof(EG_Color_t);
		int32_t SrceStepBytes = SrceStep * sizeof(EG_Color_t);
		EG_Color_t *pColorBufTemp = pColorBuf;
		for(y = pDestRect->GetY1(); y <= pDestRect->GetY2(); y++) {
			EG_CopyMem(pColorBufTemp, pSrce8, DestWidthBytes);
			pSrce8 += SrceStepBytes;
			pColorBufTemp += DestWidth;
		}
		// Make "holes" for with Chroma keying
		if(ColorFormat == EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED) {
			uint32_t i;
			EG_Color_t chk = EG_COLOR_CHROMA_KEY;
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
			uint8_t *cbuf_uint = (uint8_t *)pColorBuf;
			uint8_t chk_v = chk.full;
#elif EG_COLOR_DEPTH == 16
			uint16_t *cbuf_uint = (uint16_t *)pColorBuf;
			uint16_t chk_v = chk.full;
#elif EG_COLOR_DEPTH == 32
			uint32_t *cbuf_uint = (uint32_t *)pColorBuf;
			uint32_t chk_v = chk.full;
#endif
			for(i = 0; i < PixelCount; i++) {
				if(chk_v == cbuf_uint[i]) abuf[i] = 0x00;
			}
		}
	}
	else if(ColorFormat == EG_COLOR_FORMAT_NATIVE_ALPHA) {
		pSrce8 += (SrceStep * pDestRect->GetY1() * EG_IMG_PX_SIZE_ALPHA_BYTE) + pDestRect->GetX1() * EG_IMG_PX_SIZE_ALPHA_BYTE;
		int32_t src_new_line_step_px = (SrceStep - pDestRect->GetWidth());
		int32_t src_new_line_step_byte = src_new_line_step_px * EG_IMG_PX_SIZE_ALPHA_BYTE;
		int32_t DestHeight = pDestRect->GetHeight();
		int32_t DestWidth = pDestRect->GetWidth();
		for(y = 0; y < DestHeight; y++) {
			for(x = 0; x < DestWidth; x++) {
				abuf[x] = pSrce8[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
				pColorBuf[x].full = *pSrce8;
#elif EG_COLOR_DEPTH == 16
				pColorBuf[x].full = *pSrce8 + ((*(pSrce8 + 1)) << 8);
#elif EG_COLOR_DEPTH == 32
				pColorBuf[x] = *((EG_Color_t *)pSrce8);
				pColorBuf[x].ch.alpha = 0xff;
#endif
				pSrce8 += EG_IMG_PX_SIZE_ALPHA_BYTE;
			}
			pColorBuf += DestWidth;
			abuf += DestWidth;
			pSrce8 += src_new_line_step_byte;
		}
	}
	else if(ColorFormat == EG_COLOR_FORMAT_RGB565A8) {
		pSrce8 += (SrceStep * pDestRect->GetY1() * sizeof(EG_Color_t)) + pDestRect->GetX1() * sizeof(EG_Color_t);
		int32_t SrceStepBytes = SrceStep * sizeof(EG_Color_t);
		int32_t DestHeight = pDestRect->GetHeight();
		int32_t DestWidth = pDestRect->GetWidth();
		for(y = 0; y < DestHeight; y++) {
			EG_CopyMem(pColorBuf, pSrce8, DestWidth * sizeof(EG_Color_t));
			pColorBuf += DestWidth;
			pSrce8 += SrceStepBytes;
		}
		pSrce8 = (const uint8_t *)pSrceBuffer;
		pSrce8 += sizeof(EG_Color_t) * SrceWidth * SrceHeight;
		pSrce8 += SrceStep * pDestRect->GetY1() + pDestRect->GetX1();
		for(y = 0; y < DestHeight; y++) {
			EG_CopyMem(abuf, pSrce8, DestWidth);
			abuf += DestWidth;
			pSrce8 += SrceStep;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

#if 0
void EGARM2DContext::invalidate_cache(void)
{
    lv_disp_t * pDisp = _lv_refr_get_disp_refreshing();
    if(pDisp->driver->clean_dcache_cb) pDisp->driver->clean_dcache_cb(pDisp->driver);
    else {
#if __CORTEX_M >= 0x07
        if((SCB->CCR) & (uint32_t)SCB_CCR_DC_Msk)
            SCB_CleanInvalidateDCache();
#endif
    }
}
#endif

#endif
