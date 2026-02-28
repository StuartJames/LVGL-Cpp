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

#include <stdint.h>
#include <stddef.h>
#include "hal/EG_HAL.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Misc.h"
#include "misc/EG_Assert.h"
#include "core/EG_Object.h"
#include "core/EG_Refresh.h"
#include "core/EG_Theme.h"
#include "draw/sw/EG_SoftContext.h"

#if EG_USE_THEME_DEFAULT
#include "extra/themes/EG_DefaultTheme.h"
#endif

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::InitialiseDriver(EGDisplayDriver *pDriver)
{
	pDriver->m_HorizontalRes = 320;
	pDriver->m_VerticalRes = 240;
	pDriver->m_PhysicalHorizontalRes = -1;
	pDriver->m_PhysicalVerticalRes = -1;
	pDriver->m_OffsetX = 0;
	pDriver->m_OffsetY = 0;
	pDriver->m_AntiAliasing = EG_COLOR_DEPTH > 8 ? 1 : 0;
	pDriver->m_ScreenTransparent = 0;
	pDriver->m_DPI = EG_DPI_DEF;
	pDriver->m_ColorChromaKey = EG_COLOR_CHROMA_KEY;

#if EG_USE_GPU_RA6M3_G2D
	pDriver->draw_ctx_init = lv_draw_ra6m3_2d_ctx_init;
	pDriver->draw_ctx_deinit = lv_draw_ra6m3_2d_ctx_init;
	pDriver->draw_ctx_size = sizeof(lv_draw_ra6m3_dma2d_ctx_t);
#elif EG_USE_GPU_STM32_DMA2D
	pDriver->draw_ctx_init = lv_draw_stm32_dma2d_ctx_init;
	pDriver->draw_ctx_deinit = lv_draw_stm32_dma2d_ctx_init;
	pDriver->draw_ctx_size = sizeof(lv_draw_stm32_dma2d_ctx_t);
#elif EG_USE_GPU_SWM341_DMA2D
	pDriver->draw_ctx_init = lv_draw_swm341_dma2d_ctx_init;
	pDriver->draw_ctx_deinit = lv_draw_swm341_dma2d_ctx_init;
	pDriver->draw_ctx_size = sizeof(lv_draw_swm341_dma2d_ctx_t);
#elif EG_USE_GPU_NXP_VG_LITE
	pDriver->draw_ctx_init = lv_draw_vglite_ctx_init;
	pDriver->draw_ctx_deinit = lv_draw_vglite_ctx_deinit;
	pDriver->draw_ctx_size = sizeof(lv_draw_vglite_ctx_t);
#elif EG_USE_GPU_NXP_PXP
	pDriver->draw_ctx_init = lv_draw_pxp_ctx_init;
	pDriver->draw_ctx_deinit = lv_draw_pxp_ctx_deinit;
	pDriver->draw_ctx_size = sizeof(lv_draw_pxp_ctx_t);
#elif EG_USE_GPU_SDL
	pDriver->draw_ctx_init = lv_draw_sdl_init_ctx;
	pDriver->draw_ctx_deinit = lv_draw_sdl_deinit_ctx;
	pDriver->draw_ctx_size = sizeof(lv_draw_sdl_ctx_t);
#elif EG_USE_GPU_ARM2D
	pDriver->draw_ctx_init = lv_draw_arm2d_ctx_init;
	pDriver->draw_ctx_deinit = lv_draw_arm2d_ctx_init;
	pDriver->draw_ctx_size = sizeof(lv_draw_arm2d_ctx_t);
#else
	EGSoftContext *pDrawContext = new EGSoftContext;
  pDrawContext->InitialiseContext();
  pDriver->m_pContext = (EGDrawContext*)pDrawContext;
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool EGDisplay::InitialiseDrawBuffers(EG_DisplayDrawBuffer_t **pDrawBuffers, void *pBuffer1, void *pBuffer2, uint32_t SizeInPixels)
{
	(*pDrawBuffers) = (EG_DisplayDrawBuffer_t*)malloc(sizeof(EG_DisplayDrawBuffer_t));
	if((*pDrawBuffers) == nullptr) return false;
//	ESP_LOGI("[Dsip  ]", "Buffer: %p, Buf1: %p, Buf2: %p, Size: %d.", (void *)(*pDrawBuffers), (void *)pBuffer1, (void *)pBuffer2, SizeInPixels);
	EG_ZeroMem(*pDrawBuffers, sizeof(EG_DisplayDrawBuffer_t));
	(*pDrawBuffers)->pBuffer1 = pBuffer1;
	(*pDrawBuffers)->pBuffer2 = pBuffer2;
	(*pDrawBuffers)->pActiveBuffer = (*pDrawBuffers)->pBuffer1;
	(*pDrawBuffers)->Size = SizeInPixels;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

EGDisplay* EGDisplay::RegisterDriver(EGDisplayDriver *pDriver)
{
  EG_LOG_WARN("Creating new display");
  EGDisplay *pDisplay = new EGDisplay;
	if(pDisplay == nullptr) return nullptr;
	m_DisplayList.AddHead(pDisplay);
	if(pDriver->m_pContext == nullptr) {	// Create a draw context if not created yet
		EGDrawContext *pDrawContext = new EGDrawContext;
		if(pDrawContext == nullptr) return nullptr;
		pDriver->InitialiseContext(pDrawContext);
		pDriver->m_pContext = pDrawContext;
	}
	pDisplay->m_pDriver = pDriver;
	pDisplay->m_InvalidEnableCount = 1;
	pDisplay->m_SyncAreas.Initialise();
	EGDisplay *pDispTemp = m_pDefaultDisplay;
	m_pDefaultDisplay = pDisplay; // Temporarily change the default display to create the default screens on the new display
	pDisplay->m_pRefreshTimer = EGTimer::Create(RefreshTimerCB, EG_DISP_DEF_REFR_PERIOD, pDisplay, true); // Create a refresh timer
//	pDisplay->m_pRefreshTimer = EGTimer::Create(RefreshTimerCB, 1000, pDisplay, true); // Create a refresh timer
	if(pDisplay->m_pRefreshTimer == nullptr) {
		delete pDisplay;
		return nullptr;
	}
	if(pDriver->m_FullRefresh && pDriver->m_pDrawBuffers->Size < (uint32_t)pDriver->m_HorizontalRes * pDriver->m_VerticalRes) {
		pDriver->m_FullRefresh = 0;
		EG_LOG_WARN("FullRefresh requires at least screen sized draw buffer(s)");
	}
	pDisplay->m_BackgroundColor = EG_ColorWhite();
#if EG_COLOR_SCREEN_TRANSP
	pDisplay->m_BackgroundOPA = EG_OPA_TRANSP;
#else
	pDisplay->m_BackgroundOPA = EG_OPA_COVER;
#endif
#if EG_USE_THEME_DEFAULT
  EGDefTheme::SetTheme(EG_MainPalette(EG_PALETTE_BLUE), EG_MainPalette(EG_PALETTE_RED), EG_THEME_DEFAULT_DARK, EG_FONT_DEFAULT, pDisplay);
#endif
  EG_LOG_WARN("Creating default screen & layers");
	pDisplay->m_pActiveScreen = new EGObject(nullptr);   // Create a default screen on the display
	pDisplay->m_pTopLayer = new EGObject(nullptr);       // Create top layer on the display
	pDisplay->m_pSystemLayer = new EGObject(nullptr);    // Create sys layer on the display
	pDisplay->m_pTopLayer->RemoveAllStyles();
	pDisplay->m_pSystemLayer->RemoveAllStyles();
	pDisplay->m_pTopLayer->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	pDisplay->m_pSystemLayer->ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	pDisplay->m_pTopLayer->SetScrollbarMode(EG_SCROLLBAR_MODE_OFF);
	pDisplay->m_pSystemLayer->SetScrollbarMode(EG_SCROLLBAR_MODE_OFF);
	pDisplay->m_pActiveScreen->Invalidate();
  EG_LOG_WARN("Setting the default display");
	m_pDefaultDisplay = pDispTemp;                                // Revert the default display
	if(m_pDefaultDisplay == nullptr) m_pDefaultDisplay = pDisplay; // Initialize the default display
	pDisplay->m_pRefreshTimer->Resume(); // It's now safe to start the timer
	pDisplay->m_pRefreshTimer->MakeReady(); // Be sure the screen will be refreshed immediately on start up
	return pDisplay;
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::UpdateDriver(EGDisplayDriver *pdriver)
{
	m_pDriver = pdriver;
	if(m_pDriver->m_FullRefresh &&
		m_pDriver->m_pDrawBuffers->Size < (uint32_t)m_pDriver->m_HorizontalRes * m_pDriver->m_VerticalRes) {
		m_pDriver->m_FullRefresh = 0;
		EG_LOG_WARN("FullRefresh requires at least screen sized draw buffer(s)");
	}
	EG_Coord_t Width = GetHorizontalRes();
	EG_Coord_t Height = GetVerticalRes();
	uint32_t i;
	for(i = 0; i < m_ScreenCount; i++) {
		EGRect PrevArea;
		m_pScreens[i]->m_Rect.Copy(&PrevArea);
		m_pScreens[i]->m_Rect.SetWidth(Width);
		m_pScreens[i]->m_Rect.SetHeight(Height);
		EGEvent::EventSend(m_pScreens[i], EG_EVENT_SIZE_CHANGED, &PrevArea);
	}
	/* This method is usually called upon orientation change, thus the screen is now a different Size.
  * The object invalidated its previous area. That area is now out of the screen area
  * so we reset all invalidated areas and invalidate the active screen's new area only. */
//	EG_ZeroMem(m_InvalidAreas, sizeof(m_InvalidAreas));
	EG_ZeroMem(m_InvalidAreasJoined, sizeof(m_InvalidAreasJoined));
	m_InvalidCount = 0;
	if(m_pActiveScreen != nullptr) m_pActiveScreen->Invalidate();
	EGObject::TreeWalk(nullptr, InvalidateLayoutCB, nullptr);
	if(m_pDriver->UpdateCB) m_pDriver->UpdateCB(m_pDriver);
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::Remove(EGDisplay *pDisplay)
{
bool WasDefault = false;

	if(pDisplay == GetDefault()) WasDefault = true;
	EGInputDevice *pInputdevice = EGInputDevice::GetNext(nullptr);	// Detach the input devices
	while(pInputdevice) {
		if(pInputdevice->m_pDriver->m_pDisplay == pDisplay) {
			pInputdevice->m_pDriver->m_pDisplay = nullptr;
		}
		pInputdevice = EGInputDevice::GetNext(pInputdevice);
	}
	if(pDisplay->m_pSystemLayer) {	// delete screen and other obj 
		EGObject::Delete(pDisplay->m_pSystemLayer);
		pDisplay->m_pSystemLayer = nullptr;
	}
	if(pDisplay->m_pTopLayer) {
		EGObject::Delete(pDisplay->m_pTopLayer);
		pDisplay->m_pTopLayer = nullptr;
	}
	while(pDisplay->m_ScreenCount != 0) EGObject::Delete(pDisplay->m_pScreens[0]); // Delete the screenst
  POSITION Pos = m_DisplayList.Find(pDisplay);
  if(Pos != nullptr) m_DisplayList.RemoveAt(Pos);
  while(pDisplay->m_SyncAreas.GetCount() > 0) delete (EGRect*)pDisplay->m_SyncAreas.RemoveHead();
	if(pDisplay->m_pRefreshTimer) EGTimer::Delete(pDisplay->m_pRefreshTimer);
	EG_FreeMem(pDisplay);
	if(WasDefault) m_pDefaultDisplay = (EGDisplay*)m_DisplayList.GetHead();
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetDefault(EGDisplay *pDisplay)
{
	m_pDefaultDisplay = pDisplay;
}

///////////////////////////////////////////////////////////////////////////////

EGDisplay* EGDisplay::GetDefault(void)
{
	return m_pDefaultDisplay;
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetHorizontalRes(void)
{
  switch(m_pDriver->m_Rotated) {
    case EG_DISP_ROT_90:
    case EG_DISP_ROT_270:
      return m_pDriver->m_VerticalRes;
    default:
      return m_pDriver->m_HorizontalRes;
  }
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetVerticalRes(void)
{
  switch(m_pDriver->m_Rotated) {
    case EG_DISP_ROT_90:
    case EG_DISP_ROT_270:
      return m_pDriver->m_HorizontalRes;
    default:
      return m_pDriver->m_VerticalRes;
  }
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetPhysicalHorizontalRes(void)
{
  switch(m_pDriver->m_Rotated) {
    case EG_DISP_ROT_90:
    case EG_DISP_ROT_270:
      return m_pDriver->m_PhysicalVerticalRes > 0 ? m_pDriver->m_PhysicalVerticalRes : m_pDriver->m_VerticalRes;
    default:
      return m_pDriver->m_PhysicalHorizontalRes > 0 ? m_pDriver->m_PhysicalHorizontalRes : m_pDriver->m_HorizontalRes;
  }
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetPhysicalVerticalRes(void)
{
  switch(m_pDriver->m_Rotated) {
    case EG_DISP_ROT_90:
    case EG_DISP_ROT_270:
      return m_pDriver->m_PhysicalHorizontalRes > 0 ? m_pDriver->m_PhysicalHorizontalRes : m_pDriver->m_HorizontalRes;
    default:
      return m_pDriver->m_PhysicalVerticalRes > 0 ? m_pDriver->m_PhysicalVerticalRes : m_pDriver->m_VerticalRes;
	}
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetOffsetX(void)
{
  switch(m_pDriver->m_Rotated) {
    case EG_DISP_ROT_90:
      return m_pDriver->m_OffsetY;
    case EG_DISP_ROT_180:
      return GetPhysicalHorizontalRes() - m_pDriver->m_OffsetX;
    case EG_DISP_ROT_270:
      return GetPhysicalHorizontalRes() - m_pDriver->m_OffsetY;
    default:
      return m_pDriver->m_OffsetX;
  }
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetOffsetY(void)
{
  switch(m_pDriver->m_Rotated) {
    case EG_DISP_ROT_90:
      return m_pDriver->m_OffsetX;
    case EG_DISP_ROT_180:
      return GetPhysicalVerticalRes() - m_pDriver->m_OffsetY;
    case EG_DISP_ROT_270:
      return GetPhysicalVerticalRes() - m_pDriver->m_OffsetX;
    default:
      return m_pDriver->m_OffsetY;
  }
}

///////////////////////////////////////////////////////////////////////////////

bool EGDisplay::GetAntialiasing(void)
{
	return m_pDriver->m_AntiAliasing ? true : false;
}

///////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGDisplay::GetDPI(const EGDisplay *pDisplay)
{
	if(pDisplay == nullptr) pDisplay = m_pDefaultDisplay;
	if(pDisplay == nullptr) return EG_DPI_DEF; // Do not return 0 because it might be a divider
	return pDisplay->m_pDriver->m_DPI;
}

///////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FLUSH_READY EGDisplay::FlushReady(EGDisplayDriver *pDriver)
{
	pDriver->m_pDrawBuffers->Flushing = 0;
	pDriver->m_pDrawBuffers->FlushingLast = 0;
}

///////////////////////////////////////////////////////////////////////////////

bool EG_ATTRIBUTE_FLUSH_READY EGDisplay::FlushIsLast(EGDisplayDriver *pDriver)
{
	return pDriver->m_pDrawBuffers->FlushingLast;
}

///////////////////////////////////////////////////////////////////////////////

EGDisplay* EGDisplay::GetNext(EGDisplay *pDisplay)
{
static POSITION Pos = 0;

	if(pDisplay == nullptr)	return (EGDisplay *)m_DisplayList.GetHead(Pos);
	else return (EGDisplay *)m_DisplayList.GetNext(Pos);
}

///////////////////////////////////////////////////////////////////////////////

EG_DisplayDrawBuffer_t* EGDisplay::GetDrawBuffer(void)
{
	return m_pDriver->m_pDrawBuffers;
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetRotation(EG_DisplayRotation_t Rotate)
{
	m_pDriver->m_Rotated = Rotate;
	UpdateDriver(m_pDriver);
}

///////////////////////////////////////////////////////////////////////////////

EG_DisplayRotation_t EGDisplay::GetRotation(void)
{
	return (EG_DisplayRotation_t)m_pDriver->m_Rotated;
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::UseGenericSetPixelCB(EGDisplayDriver *pDriver, EG_ImageColorFormat_t ColorFormat)
{
	switch(ColorFormat) {
		case EG_COLOR_FORMAT_NATIVE_ALPHA:
			pDriver->SetPixelCB = SetPixelTrueColorAlpha;
			break;
		case EG_COLOR_FORMAT_ALPHA_1BIT:
			pDriver->SetPixelCB = SetPixelAlpha1CB;
			break;
		case EG_COLOR_FORMAT_ALPHA_2BIT:
			pDriver->SetPixelCB = SetPixelAlpha2CB;
			break;
		case EG_COLOR_FORMAT_ALPHA_4BIT:
			pDriver->SetPixelCB = SetPixelAlpha4CB;
			break;
		case EG_COLOR_FORMAT_ALPHA_8BIT:
			pDriver->SetPixelCB = SetPixelAlpha8CB;
			break;
		default:
			pDriver->SetPixelCB = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////

EG_TreeWalkResult_e EGDisplay::InvalidateLayoutCB(EGObject *pObj, void *pUserData)
{
	EG_UNUSED(pUserData);
	pObj->MarkLayoutDirty();
	return EG_TREE_WALK_NEXT;
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetPixelAlpha1CB(EGDisplayDriver *pDriver, uint8_t *pBuffer, EG_Coord_t Width, EG_Coord_t x, EG_Coord_t y,
											EG_Color_t color, EG_OPA_t opa)
{
	(void)pDriver; /*Unused*/
	if(opa <= EG_OPA_MIN) return;
	EGImageBuffer Image;
	Image.m_pData = pBuffer;
	Image.m_Header.Width = Width;
	Image.m_Header.ColorFormat = EG_COLOR_FORMAT_ALPHA_1BIT;
	SetPixelAlphaGeneric(&Image, x, y, color, opa);
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetPixelAlpha2CB(EGDisplayDriver *pDriver, uint8_t *pBuffer, EG_Coord_t Width, EG_Coord_t x, EG_Coord_t y,
											EG_Color_t color, EG_OPA_t opa)
{
	(void)pDriver; /*Unused*/
	if(opa <= EG_OPA_MIN) return;
	EGImageBuffer Image;
	Image.m_pData = pBuffer;
	Image.m_Header.Width = Width;
	Image.m_Header.ColorFormat = EG_COLOR_FORMAT_ALPHA_2BIT;
	SetPixelAlphaGeneric(&Image, x, y, color, opa);
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetPixelAlpha4CB(EGDisplayDriver *pDriver, uint8_t *pBuffer, EG_Coord_t Width, EG_Coord_t x, EG_Coord_t y,
											EG_Color_t color, EG_OPA_t opa)
{
	(void)pDriver; /*Unused*/
	if(opa <= EG_OPA_MIN) return;
	EGImageBuffer Image;
	Image.m_pData = pBuffer;
	Image.m_Header.Width = Width;
	Image.m_Header.ColorFormat = EG_COLOR_FORMAT_ALPHA_4BIT;
	SetPixelAlphaGeneric(&Image, x, y, color, opa);
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetPixelAlpha8CB(EGDisplayDriver *pDriver, uint8_t *pBuffer, EG_Coord_t Width, EG_Coord_t x, EG_Coord_t y,
											EG_Color_t color, EG_OPA_t opa)
{
	(void)pDriver; /*Unused*/
	if(opa <= EG_OPA_MIN) return;
	EGImageBuffer Image;
	Image.m_pData = pBuffer;
	Image.m_Header.Width = Width;
	Image.m_Header.ColorFormat = EG_COLOR_FORMAT_ALPHA_8BIT;
	SetPixelAlphaGeneric(&Image, x, y, color, opa);
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetPixelAlphaGeneric(EGImageBuffer *pImage, EG_Coord_t x, EG_Coord_t y, EG_Color_t color, EG_OPA_t opa)
{
	pImage->m_Header.AlwaysZero = 0;
	pImage->m_Header.Height = 1; /*Doesn't matter*/
	uint8_t br = EG_ColorGetBrightness(color);
	if(opa < EG_OPA_MAX) {
		uint8_t bg = pImage->GetPixelAlpha(x, y);
		br = (uint16_t)((uint16_t)br * opa + (bg * (255 - opa))) >> 8;
	}
	pImage->SetPixelAlpha(x, y, br);
}

///////////////////////////////////////////////////////////////////////////////

void EGDisplay::SetPixelTrueColorAlpha(EGDisplayDriver *pDriver, uint8_t *buf, EG_Coord_t buf_w,
														EG_Coord_t x, EG_Coord_t y, EG_Color_t color, EG_OPA_t opa)
{
	(void)pDriver; /*Unused*/
	uint8_t *buf_px = buf + (buf_w * y * EG_IMG_PX_SIZE_ALPHA_BYTE + x * EG_IMG_PX_SIZE_ALPHA_BYTE);
	EG_Color_t bg_color;
	EG_Color_t res_color;
	EG_OPA_t bg_opa = buf_px[EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
#if EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
	bg_color.full = buf_px[0];
	EG_ColorMixWithAlpha(bg_color, bg_opa, color, opa, &res_color, &buf_px[2]);
	if(buf_px[1] <= EG_OPA_MIN) return;
	buf_px[0] = res_color.full;
#elif EG_COLOR_DEPTH == 16
	bg_color.full = buf_px[0] + (buf_px[1] << 8);
	EG_ColorMixWithAlpha(bg_color, bg_opa, color, opa, &res_color, &buf_px[2]);
	if(buf_px[2] <= EG_OPA_MIN) return;
//	buf_px[0] = res_color.full & 0xff;
//	buf_px[1] = res_color.full >> 8;
	buf_px[1] = res_color.full & 0xff;
	buf_px[0] = res_color.full >> 8;
#elif EG_COLOR_DEPTH == 32
	bg_color = *((EG_Color_t *)buf_px);
	EG_ColorMixWithAlpha(bg_color, bg_opa, color, opa, &res_color, &buf_px[3]);
	if(buf_px[3] <= EG_OPA_MIN) return;
	buf_px[0] = res_color.ch.blue;
	buf_px[1] = res_color.ch.green;
	buf_px[2] = res_color.ch.red;
#endif
}

////////////////////////////////////////////////////////////////////////
// DRIVER // 
////////////////////////////////////////////////////////////////////////

EGDisplayDriver::EGDisplayDriver(void)
{
  FlushCB = nullptr;
  RounderCB = nullptr;
  SetPixelCB = nullptr;
  MonitorCB = nullptr;
  WaitCB = nullptr;
  CleanDcacheCB = nullptr;
  UpdateCB = nullptr;
  RenderStartCB = nullptr;
  m_HorizontalRes = 0;          
  m_VerticalRes = 0;            
  m_PhysicalHorizontalRes = 0;  
  m_PhysicalVerticalRes = 0;    
  m_OffsetX = 0;                
  m_OffsetY = 0;                
  m_pDrawBuffers = 0;           
  m_DirectMode = 0;         
  m_FullRefresh = 0;        
  m_SoftRotate = 0;         
  m_AntiAliasing = 0;       
  m_Rotated = 0;            
  m_ScreenTransparent = 0;  
  m_DPI = 0;               
  m_pContext = nullptr;
  m_ContextSize = 0;
}

////////////////////////////////////////////////////////////////////////

EGDisplayDriver::~EGDisplayDriver(void)
{
}

///////////////////////////////////////////////////////////////////////

void EGDisplayDriver::InitialiseContext(EGDrawContext *pDrawContext)
{
}

///////////////////////////////////////////////////////////////////////

void EGDisplayDriver::DeinitialiseContext(EGDrawContext *pDrawContext)
{
}
