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

#include "extra/widgets/EG_AnimatedImage.h"
#if EG_USE_ANIMIMG != 0

#if EG_USE_IMG == 0     // Testing of dependencies
#error "lv_animimg: lv_img is required. Enable it in EG_Config.h (EG_USE_IMG  1) "
#endif

#include "misc/EG_Assert.h"
#include "draw/EG_ImageDecoder.h"
#include "misc/EG_FileSystem.h"
#include "misc/EG_Text.h"
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"
#include "misc/EG_Animate.h"

///////////////////////////////////////////////////////////////////////////////////////

#define LV_OBJX_NAME "lv_animimg"

#define ANIMATE_IMAGE_CLASS &c_AnimateImageClass

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_AnimateImageClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGAnimatedImage::EGAnimatedImage(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= ANIMATE_IMAGE_CLASS*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGAnimatedImage::Configure(void)
{
  EGObject::Configure();
	m_pDicriptors = nullptr;
	m_PictureCount = -1;
	//initial animation
	m_Animate.SetItem(this);
	m_Animate.SetTime(30);
	m_Animate.SetExcCB(EGAnimatedImage::IndexChange);
	m_Animate.SetValues(0, 1);
	m_Animate.SetRepeatCount(EG_ANIM_REPEAT_INFINITE);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGAnimatedImage::SetSource(const void *pDiscriptor[], uint8_t Count)
{
	m_pDicriptors = pDiscriptor;
	m_PictureCount = Count;
	m_Animate.SetValues(0, Count);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGAnimatedImage::Start(void)
{
	EGAnimate::Create(&m_Animate);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGAnimatedImage::SetDuration(uint32_t Duration)
{
	m_Animate.SetTime(Duration);
	m_Animate.SetPlaybackDelay(Duration);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGAnimatedImage::SetRepeatCount(uint16_t Count)
{
	m_Animate.SetRepeatCount(Count);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGAnimatedImage::IndexChange(void *pObj, int32_t Index)
{
	EGAnimatedImage *pAnimateImage = (EGAnimatedImage*)pObj;
	EG_Coord_t Idx = Index % pAnimateImage->m_PictureCount;
	pAnimateImage->m_Image.SetSource(pAnimateImage->m_pDicriptors[Idx]);
}

#endif
