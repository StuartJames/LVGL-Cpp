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

#pragma once

#include "misc/EG_Color.h"
#include "hal/EG_HALDisplay.h"
#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"


///////////////////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_GPU_ARM2D

class EGDisplay;

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGARM2DContext : public EGSoftContext
{
public:
                  EGARM2DContext() : EGSoftContext(){};
  virtual         ~EGARM2DContext(){};
  void            InitialiseContext(void);

  static void     Blend(EGBlendBase *pBlend);
  static void     WaitForFinish(void);
  static void     DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSrceBuffer, EG_ImageColorFormat_t ColorFormat);

#if __ARM_2D_HAS_HW_ACC__
  bool            FillColour(const arm_2d_tile_t *target_tile, const arm_2d_region_t *region, EG_Color_t color, EG_OPA_t opa, const arm_2d_tile_t *mask_tile);
  bool            TileCopy(const arm_2d_tile_t *target_tile, const arm_2d_region_t *region, arm_2d_tile_t *source_tile, EG_OPA_t opa, arm_2d_tile_t *mask_tile);
#else
  void            Convert(const EGRect *pDestRect, const void *pSrceBuffer, int32_t SourceWidth, int32_t SrceHeight, int32_t SrceStep,
                            const EGDrawImage *pDrawImage, EG_ImageColorFormat_t ColorFormat, EG_Color_t *cbuf, EG_OPA_t *abuf);
  bool            FillNormal(EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep, EG_Color_t color,
                             EG_OPA_t opa, const EG_OPA_t *mask, int32_t mask_stride);
  bool            CopyNormal(EG_Color_t *pDestBuffer, const EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrceBuffer, int32_t src_stride, EG_OPA_t opa, const EG_OPA_t *mask, int32_t mask_stride);
#endif

  void            PaintImage(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSrceBuffer, EG_ImageColorFormat_t ColorFormat);

};

#endif

