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

#include "extra/others/EG_Snapshot.h"
#if EG_USE_SNAPSHOT

#include <stdbool.h>
#include "core/EG_Display.h"
#include "core/EG_Refresh.h"

///////////////////////////////////////////////////////////////////////////////

uint32_t EG_SnapshotBufferSizeRequired(EGObject *pObj, EG_ImageColorFormat_t ColorFormat)
{
	EG_ASSERT_NULL(pObj);
	switch(ColorFormat) {
		case EG_IMG_CF_TRUE_COLOR:
		case EG_IMG_CF_TRUE_COLOR_ALPHA:
		case EG_IMG_CF_ALPHA_1BIT:
		case EG_IMG_CF_ALPHA_2BIT:
		case EG_IMG_CF_ALPHA_4BIT:
		case EG_IMG_CF_ALPHA_8BIT:
			break;
		default:
			return 0;
	}
	pObj->UpdateLayout();

	/*Width and height determine snapshot image size.*/
	EG_Coord_t Wide = pObj->GetWidth();
	EG_Coord_t Height = pObj->GetHeight();
	EG_Coord_t ext_size = pObj->GetExtDrawSize();
	Wide += ext_size * 2;
	Height += ext_size * 2;
	uint8_t px_size = EGDrawImage::GetPixelSize(ColorFormat);
	return Wide * Height * ((px_size + 7) >> 3);
}

///////////////////////////////////////////////////////////////////////////////

EG_Result_t EG_TakeSnapshotToBuffer(EGObject *pObj, EG_ImageColorFormat_t ColorFormat, EGImageBuffer *pImage, void *pBuffer, uint32_t BufferSize)
{
	EG_ASSERT_NULL(pObj);
	EG_ASSERT_NULL(pImage);
	EG_ASSERT_NULL(pBuffer);

	switch(ColorFormat) {
		case EG_IMG_CF_TRUE_COLOR:
		case EG_IMG_CF_TRUE_COLOR_ALPHA:
		case EG_IMG_CF_ALPHA_1BIT:
		case EG_IMG_CF_ALPHA_2BIT:
		case EG_IMG_CF_ALPHA_4BIT:
		case EG_IMG_CF_ALPHA_8BIT:
			break;
		default:
			return EG_RES_INVALID;
	}
	uint32_t SizeNeeded = EG_SnapshotBufferSizeRequired(pObj, ColorFormat);
	if(SizeNeeded == 0 || BufferSize < SizeNeeded) return EG_RES_INVALID;
	/*Width and height determine snapshot image size.*/
	EG_Coord_t Wide = pObj->GetWidth();
	EG_Coord_t Height = pObj->GetHeight();
	EG_Coord_t ext_size = pObj->GetExtDrawSize();
	Wide += ext_size * 2;
	Height += ext_size * 2;
	EGRect SnapRect;
	pObj->m_Rect.Copy(&SnapRect);	// Save the original coordinates
	SnapRect.Inflate(ext_size, ext_size);
	EG_SetMem(pBuffer, 0x00, BufferSize);
	EGDisplay *pDisplay = pObj->GetDisplay();
	EGDisplayDriver Driver;
	/*In lack of a better idea use the resolution of the object's display*/
	Driver.m_HorizontalRes = pDisplay->GetHorizontalRes();
	Driver.m_VerticalRes = pDisplay->GetVerticalRes();
	pDisplay->UseGenericSetPixelCB(&Driver, ColorFormat);
	EGDisplay FakeDisplay;
	FakeDisplay.m_pDriver = &Driver;
	EGDrawContext *pContext = new  EGDrawContext;
	if(pContext == NULL) return EG_RES_INVALID;
	FakeDisplay.m_pDriver->InitialiseContext(pContext);
	FakeDisplay.m_pDriver->m_pContext = pContext;
	pContext->m_pClipRect = &SnapRect;
	pContext->m_pDrawRect = &SnapRect;
	pContext->m_pDrawBuffer = (void *)pBuffer;
	EGDisplay *pRefreshDisplay = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	RedrawObject(pContext, pObj);
	SetRefreshingDisplay(pRefreshDisplay);
	FakeDisplay.m_pDriver->DeinitialiseContext(pContext);
	delete pContext;
	pImage->m_pData = (uint8_t*)pBuffer;
	pImage->m_DataSize = SizeNeeded;
	pImage->m_Header.Width = Wide;
	pImage->m_Header.Height = Height;
	pImage->m_Header.ColorFormat = ColorFormat;
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////

EGImageBuffer* EG_TakeSnapshot(EGObject *pObj, EG_ImageColorFormat_t ColorFormat)
{
	EG_ASSERT_NULL(pObj);
	uint32_t BufferSize = EG_SnapshotBufferSizeRequired(pObj, ColorFormat);
	void *pBuffer = EG_AllocMem(BufferSize);
	EG_ASSERT_MALLOC(pBuffer);
	if(pBuffer == NULL){
		return NULL;
	}
	EGImageBuffer *pImageBuffer = new EGImageBuffer;
	if(pImageBuffer == NULL){
    EG_FreeMem(pBuffer);
		return NULL;
	}
	if(EG_TakeSnapshotToBuffer(pObj, ColorFormat, pImageBuffer, pBuffer, BufferSize) == EG_RES_INVALID) {
		EG_FreeMem(pBuffer);
		EG_FreeMem(pImageBuffer);
		return NULL;
	}
	return pImageBuffer;
}

///////////////////////////////////////////////////////////////////////////////

void EG_FreeSnapshot(EGImageBuffer *pImageBuffer)
{
	if(!pImageBuffer)	return;
	if(pImageBuffer->m_pData)	EG_FreeMem((void *)pImageBuffer->m_pData);
	delete pImageBuffer;
}

#endif
