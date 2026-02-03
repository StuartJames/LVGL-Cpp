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

#include "widgets/EG_Image.h"
#if EG_USE_IMG != 0

#include "core/EG_Display.h"
#include "misc/EG_Assert.h"
#include "draw/EG_ImageDecoder.h"
#include "misc/EG_FileSystem.h"
#include "misc/EG_Text.h"
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"

///////////////////////////////////////////////////////////////////////////////////////

#define IMAGE_CLASS &c_Imageclass

const EG_ClassType_t c_Imageclass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGImage::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_SIZE_CONTENT,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGImage::EGImage(void) : EGObject(),
	m_pSource(nullptr),
	m_Offset(0, 0),
	m_Pivot(0, 0),
	m_Width(0),
	m_Height(0),
	m_Rotation(0),
	m_Zoom(EG_SCALE_NONE),
	m_SourceType(EG_IMG_SRC_UNKNOWN),
	m_ColorFormat(EG_COLOR_FORMAT_UNKNOWN),
	m_SizeMode(EG_IMG_SIZE_MODE_VIRTUAL)
{
	m_Antialias = EG_COLOR_DEPTH > 8 ? 1 : 0;
// 	ESP_LOGI("[Image ]", "New: %p", (void*)pParent);
}

///////////////////////////////////////////////////////////////////////////////////////

EGImage::EGImage(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= IMAGE_CLASS*/) : EGObject(),
	m_pSource(nullptr),
	m_Offset(0, 0),
	m_Pivot(0, 0),
	m_Width(0),
	m_Height(0),
	m_Rotation(0),
	m_Zoom(EG_SCALE_NONE),
	m_SourceType(EG_IMG_SRC_UNKNOWN),
	m_ColorFormat(EG_COLOR_FORMAT_UNKNOWN),
	m_SizeMode(EG_IMG_SIZE_MODE_VIRTUAL)
{
	m_Antialias = EG_COLOR_DEPTH > 8 ? 1 : 0;
// 	ESP_LOGI("[Image ]", "New: %p", (void*)pParent);
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGImage::~EGImage(void)
{
	if(m_SourceType == EG_IMG_SRC_FILE || m_SourceType == EG_IMG_SRC_SYMBOL) {
		EG_FreeMem((void *)m_pSource);
		m_pSource = nullptr;
		m_SourceType = EG_IMG_SRC_UNKNOWN;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::Configure(void)
{
  EGObject::Configure();
	m_pSource = nullptr;
	m_SourceType = EG_IMG_SRC_UNKNOWN;
	m_ColorFormat = EG_COLOR_FORMAT_UNKNOWN;
	m_Width = GetWidth();
	m_Height = GetHeight();
	m_Rotation = 0;
	m_Zoom = EG_SCALE_NONE;
	m_Antialias = EG_COLOR_DEPTH > 8 ? 1 : 0;
	m_Offset.Set(0, 0);
	m_Pivot.Set(0, 0);
	m_SizeMode = EG_IMG_SIZE_MODE_VIRTUAL;
	ClearFlag(EG_OBJ_FLAG_CLICKABLE);
	AddFlag(EG_OBJ_FLAG_ADV_HITTEST);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetSource(const void *pSource)
{
	Invalidate();
	EG_ImageSource_t SourceType = EGDrawImage::GetType(pSource);
#if EG_USE_LOG && EG_LOG_LEVEL >= EG_LOG_LEVEL_INFO
	switch(SourceType) {
		case EG_IMG_SRC_FILE:
			EG_LOG_TRACE("lv_img_set_src: `EG_IMG_SRC_FILE` type found");
			break;
		case EG_IMG_SRC_VARIABLE:
			EG_LOG_TRACE("lv_img_set_src: `EG_IMG_SRC_VARIABLE` type found");
			break;
		case EG_IMG_SRC_SYMBOL:
			EG_LOG_TRACE("lv_img_set_src: `EG_IMG_SRC_SYMBOL` type found");
			break;
		default:
			EG_LOG_WARN("lv_img_set_src: unknown type");
	}
#endif

	// If the new source type is unknown free the memories of the old source
	if(SourceType == EG_IMG_SRC_UNKNOWN) {
		EG_LOG_WARN("Image: unknown image type");
		if(m_SourceType == EG_IMG_SRC_SYMBOL || m_SourceType == EG_IMG_SRC_FILE) {
			EG_FreeMem((void *)m_pSource);
		}
		m_pSource = nullptr;
		m_SourceType = EG_IMG_SRC_UNKNOWN;
		return;
	}
	EG_ImageHeader_t Header;
	EGImageDecoder::GetInfo(pSource, &Header);
	if(SourceType == EG_IMG_SRC_VARIABLE) {	// Save the source
		// If memory was allocated because of the previous `SourceType` then free it
		if(m_SourceType == EG_IMG_SRC_FILE || m_SourceType == EG_IMG_SRC_SYMBOL) {
			EG_FreeMem((void *)m_pSource);
		}
		m_pSource = pSource;
	}
	else if(SourceType == EG_IMG_SRC_FILE || SourceType == EG_IMG_SRC_SYMBOL) {
		// If the new and the old source are the same then it was only a refresh.
		if(m_pSource != pSource) {
			const void *pOldSource = nullptr;
			/*If memory was allocated because of the previous `SourceType` then save its pointer and free after allocation.
             *It's important to allocate first to be sure the new data will be on a new address,
             *otherwise `img_cache` wouldn't see the change in source.*/
			if(m_SourceType == EG_IMG_SRC_FILE || m_SourceType == EG_IMG_SRC_SYMBOL) {
				pOldSource = m_pSource;
			}
			char *pPath = (char*)EG_AllocMem(strlen((char*)pSource) + 1); // ?
			EG_ASSERT_MALLOC(pPath);
			if(pPath == nullptr) return;
			strcpy(pPath, (char*)pSource);
			m_pSource = pPath;
			if(pOldSource) EG_FreeMem((void *)pOldSource);
		}
	}
	if(SourceType == EG_IMG_SRC_SYMBOL) {
		// `lv_img_dsc_get_info` couldn't set the width and height of a font so set it here
		const EG_Font_t *font = GetStyleTextFont(EG_PART_MAIN);
		EG_Coord_t letter_space = GetStyleTextKerning(EG_PART_MAIN);
		EG_Coord_t line_space = GetStyleTextLineSpace(EG_PART_MAIN);
		EGPoint size;
		EG_GetTextSize(&size, (char*)pSource, font, letter_space, line_space, EG_COORD_MAX, EG_TEXT_FLAG_NONE);
		Header.Width = size.m_X;
		Header.Height = size.m_Y;
	}
	m_SourceType = SourceType;
	m_Width = Header.Width;
	m_Height = Header.Height;
	m_ColorFormat = Header.ColorFormat;
	m_Pivot.m_X = Header.Width / 2;
	m_Pivot.m_Y = Header.Height / 2;
	RefreshSelfSize();
	if(m_Rotation || m_Zoom != EG_SCALE_NONE) RefreshExtDrawSize();	// Provide enough room for the rotated corners
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetOffsetX(EG_Coord_t X)
{
	m_Offset.m_X = X;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetOffsetY(EG_Coord_t Y)
{
	m_Offset.m_Y = Y;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetRotation(int16_t Angle)
{
	while(Angle >= 3600) Angle -= 3600;
	while(Angle < 0) Angle += 3600;
	if(Angle == m_Rotation) return;
	UpdateLayout(); // Be sure the object's size is calculated
	EG_Coord_t Width = GetWidth();
	EG_Coord_t Height = GetHeight();
	EGRect Rect;
	EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
	Rect.Move(m_Rect.GetX1(), m_Rect.GetY1());
	InvalidateArea( &Rect);
	m_Rotation = Angle;
	//  Disable invalidations because lv_obj_refresh_ext_draw_size would invalidate the whole ext draw area 
	EGDisplay *pDisplay = GetDisplay();
	EGDisplay::EnableInvalidation(pDisplay, false);
	RefreshExtDrawSize();
	EGDisplay::EnableInvalidation(pDisplay, true);
	EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
	Rect.Move(m_Rect.GetX1(), m_Rect.GetY1());
	InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetPivot(EG_Coord_t X, EG_Coord_t Y)
{
  SetPivot(EGPoint(X, Y));
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetPivot(EGPoint Pivot)
{
	if((m_Pivot.m_X == Pivot.m_X) && (m_Pivot.m_Y == Pivot.m_Y)) return;
	UpdateLayout(); // Be sure the object's size is calculated
	EG_Coord_t Width = GetWidth();
	EG_Coord_t Height = GetHeight();
	EGRect Rect;
	EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
	Rect.Move(m_Rect.GetX1(), m_Rect.GetY1());
	InvalidateArea(&Rect);
	m_Pivot = Pivot;
	// Disable invalidations because lv_obj_refresh_ext_draw_size would invalidate the whole ext draw area 
	EGDisplay *pDisplay = GetDisplay();
	EGDisplay::EnableInvalidation(pDisplay, false);
	RefreshExtDrawSize();
	EGDisplay::EnableInvalidation(pDisplay, true);
	EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
	Rect.Move(m_Rect.GetX1(), m_Rect.GetY1());
	InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetZoom(uint16_t Zoom)
{
	if(Zoom == m_Zoom) return;
	if(Zoom == 0) Zoom = 1;
	UpdateLayout(); // Be sure the object'pSize size is calculated
	EG_Coord_t Width = GetWidth();
	EG_Coord_t Height = GetHeight();
	EGRect Rect;
	EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom >> 8, &m_Pivot);
	Rect.IncX1(m_Rect.GetX1() - 1);
	Rect.IncY1(m_Rect.GetY1() - 1);
	Rect.IncX2(m_Rect.GetX1() + 1);
	Rect.IncY2(m_Rect.GetY1() + 1);
	InvalidateArea(&Rect);
	m_Zoom = Zoom;
	EGDisplay *pDisplay = GetDisplay();
	EGDisplay::EnableInvalidation(pDisplay, false);
	RefreshExtDrawSize();
	EGDisplay::EnableInvalidation(pDisplay, true);
	EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
	Rect.IncX1(m_Rect.GetX1() - 1);
	Rect.IncY1(m_Rect.GetY1() - 1);
	Rect.IncX2(m_Rect.GetX1() + 1);
	Rect.IncY2(m_Rect.GetY1() + 1);
	InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetScale(uint32_t Zoom)
{
  if(m_Align > EG_IMAGE_ALIGN_AUTO_TRANSFORM) return;   // If scale is set internally, do no overwrite it
  if(Zoom == m_ScaleX && Zoom == m_ScaleY) return;
  if(Zoom == 0) Zoom = 1;
  ScaleUpdate(Zoom, Zoom);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetScaleX(uint32_t Zoom)
{
  if(m_Align > EG_IMAGE_ALIGN_AUTO_TRANSFORM) return;   // If scale is set internally, do no overwrite it
  if(Zoom == m_ScaleX) return;
  if(Zoom == 0) Zoom = 1;
  ScaleUpdate(Zoom, m_ScaleY);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetScaleY(uint32_t Zoom)
{
  if(m_Align > EG_IMAGE_ALIGN_AUTO_TRANSFORM) return;   // If scale is set internally, do no overwrite it
  if(Zoom == m_ScaleY) return;
  if(Zoom == 0) Zoom = 1;
  ScaleUpdate(m_ScaleX, Zoom);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetAntialias(bool Antialias)
{
	if(Antialias == m_Antialias) return;
	m_Antialias = Antialias;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetInnerAlign(EG_ImageAlign_e Align)
{
  if(Align == m_Align) return;
  // If we're removing STRETCH, reset the scale
  if(m_Align == EG_IMAGE_ALIGN_STRETCH || m_Align == EG_IMAGE_ALIGN_CONTAIN ||
    m_Align == EG_IMAGE_ALIGN_COVER) {
    SetScale(EG_SCALE_NONE);
  }
  m_Align = Align;
  UpdateAlign();
  Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::SetSizeMode(EG_ImageSizeMode_e Mode)
{
	if(Mode == m_SizeMode) return;
	m_SizeMode = Mode;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EGPoint EGImage::GetTransformedSize(void)
{
EGRect Rect;

	EGImageBuffer::GetTransformedRect(&Rect, m_Width, m_Height, m_Rotation, m_Zoom, &m_Pivot);
	return EGPoint(Rect.GetWidth(), Rect.GetHeight());
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	EG_EventCode_e Code = pEvent->GetCode();
	if(Code != EG_EVENT_DRAW_MAIN && Code != EG_EVENT_DRAW_POST) {
  	if(pEvent->Pump(IMAGE_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	}
	EGImage *pImage = (EGImage*)pEvent->GetTarget();
  pImage->Event(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::Event(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_STYLE_CHANGED) {
		if(m_SourceType == EG_IMG_SRC_SYMBOL) {		// Refresh the file name to refresh the symbol text size
			SetSource(m_pSource);
		}
		else RefreshExtDrawSize();			// With transformation it might change
	}
	else if(Code == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();
		if(m_Rotation || m_Zoom != EG_SCALE_NONE) {		// If the image has Angle provide enough room for the rotated corners
			EGRect Rect;
			EG_Coord_t Width = GetWidth();
			EG_Coord_t Height = GetHeight();
			EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
			*pSize = EG_MAX(*pSize, -Rect.GetX1());
			*pSize = EG_MAX(*pSize, -Rect.GetY1());
			*pSize = EG_MAX(*pSize, Rect.GetX2() - Width);
			*pSize = EG_MAX(*pSize, Rect.GetY2() - Height);
		}
	}
	else if(Code == EG_EVENT_HIT_TEST) {
		EG_HitTestState_t *pHitTest = (EG_HitTestState_t*)pEvent->GetParam();
		// If the object is exactly image sized (not cropped, not mosaic) and transformed perform hit test on its transformed area
		if(m_Width == GetWidth() && m_Height == GetHeight() && (m_Zoom != EG_SCALE_NONE || m_Rotation != 0 ||
      m_Pivot.m_X != m_Width / 2 || m_Pivot.m_Y != m_Height / 2)) {
			EG_Coord_t Width = GetWidth();
			EG_Coord_t Height = GetHeight();
			EGRect Rect;
			EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, m_Zoom, &m_Pivot);
			Rect.IncX1(m_Rect.GetX1());
			Rect.IncY1(m_Rect.GetY1());
			Rect.IncX2(m_Rect.GetX1());
			Rect.IncY2(m_Rect.GetY1());
			pHitTest->Result = Rect.IsPointIn(pHitTest->pPoint, 0);
		}
		else {
			EGRect Rect;
			GetClickArea(&Rect);
			pHitTest->Result = Rect.IsPointIn(pHitTest->pPoint, 0);
		}
	}
	else if(Code == EG_EVENT_GET_SELF_SIZE) {
		EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
		if(m_SizeMode == EG_IMG_SIZE_MODE_REAL) {
			*pPoint = GetTransformedSize();
		}
		else {
			pPoint->m_X = m_Width;
			pPoint->m_Y = m_Height;
		}
	}
	else if(Code == EG_EVENT_DRAW_MAIN || Code == EG_EVENT_DRAW_POST || Code == EG_EVENT_COVER_CHECK) {
		Draw(pEvent);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::Draw(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_COVER_CHECK) {
		EG_CoverCheckInfo_t *pHitTest = (EG_CoverCheckInfo_t*)pEvent->GetParam();
		if(pHitTest->Result == EG_COVER_RES_MASKED) return;
		if(m_SourceType == EG_IMG_SRC_UNKNOWN || m_SourceType == EG_IMG_SRC_SYMBOL) {
			pHitTest->Result = EG_COVER_RES_NOT_COVER;
			return;
		}
		// Non true color format might have "holes"
		if(m_ColorFormat != EG_COLOR_FORMAT_NATIVE && m_ColorFormat != EG_COLOR_FORMAT_RAW) {
			pHitTest->Result = EG_COVER_RES_NOT_COVER;
			return;
		}
		// With not EG_OPA_COVER images can't cover an area 
		if(GetStyleImageOPA(EG_PART_MAIN) != EG_OPA_COVER) {
			pHitTest->Result = EG_COVER_RES_NOT_COVER;
			return;
		}
		if(m_Rotation != 0) {
			pHitTest->Result = EG_COVER_RES_NOT_COVER;
			return;
		}
		const EGRect *pClip = (EGRect*)pEvent->GetParam();
		if(m_Zoom == EG_SCALE_NONE) {
			if(pClip->IsInside(&m_Rect, 0) == false) {
				pHitTest->Result = EG_COVER_RES_NOT_COVER;
				return;
			}
		}
		else {
			EGRect Rect;
			EGImageBuffer::GetTransformedRect(&Rect, GetWidth(), GetHeight(), 0, m_Zoom, &m_Pivot);
			Rect.IncX1(m_Rect.GetX1());
			Rect.IncY1(m_Rect.GetY1());
			Rect.IncX2(m_Rect.GetX1());
			Rect.IncY2(m_Rect.GetY1());
			if(pClip->IsInside(&Rect, 0) == false) {
				pHitTest->Result = EG_COVER_RES_NOT_COVER;
				return;
			}
		}
	}
	else if(Code == EG_EVENT_DRAW_MAIN || Code == EG_EVENT_DRAW_POST) {
		EG_Coord_t Width = GetWidth();
		EG_Coord_t Height = GetHeight();
		EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
		EG_Coord_t PadLeft = GetStylePadLeft(EG_PART_MAIN) + BorderWidth;
		EG_Coord_t PadRight = GetStylePadRight(EG_PART_MAIN) + BorderWidth;
		EG_Coord_t PadTop = GetStylePadTop(EG_PART_MAIN) + BorderWidth;
		EG_Coord_t PadBottom = GetStylePadBottom(EG_PART_MAIN) + BorderWidth;
		EGPoint BackgroundPivot;
		BackgroundPivot.m_X = m_Pivot.m_X + PadLeft;
		BackgroundPivot.m_Y = m_Pivot.m_Y + PadTop;
		EGRect BackgroundRect;
		if(m_SizeMode == EG_IMG_SIZE_MODE_REAL) {
    	m_Rect.Copy(&BackgroundRect);	// Object size equals to transformed image size
		}
		else {
			EGImageBuffer::GetTransformedRect(&BackgroundRect, Width, Height, m_Rotation, m_Zoom, &BackgroundPivot);
			BackgroundRect.Move(m_Rect.GetX1(), m_Rect.GetY1()); // Modify the coordinates to draw the background for the rotated and scaled coordinates
		}
		EGRect OriginalRect(m_Rect);
		BackgroundRect.Copy(&m_Rect);
		EG_Result_t Result = pEvent->Pump(m_pClass);	// Call the ancestor'pSize event handler
		if(Result != EG_RES_OK) return;
		OriginalRect.Copy(&m_Rect);
		if(Code == EG_EVENT_DRAW_MAIN) {
			if(m_Height == 0 || m_Width == 0) return;
			if(m_Zoom == 0) return;
			EGDrawContext *pContext = pEvent->GetDrawContext();
			EGRect ImageRect(m_Rect);
			EGPoint ImageSize = GetTransformedSize();
			if(m_SizeMode == EG_IMG_SIZE_MODE_REAL) {
				ImageRect.Move(-(((m_Width - ImageSize.m_X) + 1) / 2), -(((m_Height - ImageSize.m_Y) + 1) / 2));
			}
			else {
				ImageRect.SetX2(ImageRect.GetX1() + BackgroundRect.GetWidth() - 1);
				ImageRect.SetY2(ImageRect.GetY1() + BackgroundRect.GetHeight() - 1);
			}
			ImageRect.Deflate(PadLeft, PadRight, PadTop, PadBottom);
			if(m_SourceType == EG_IMG_SRC_FILE || m_SourceType == EG_IMG_SRC_VARIABLE) {
				EGDrawImage DrawImage;
				InititialseDrawImage(EG_PART_MAIN, &DrawImage);
				DrawImage.m_Zoom = m_Zoom;
				DrawImage.m_Angle = m_Rotation;
				DrawImage.m_Pivot.m_X = m_Pivot.m_X;
				DrawImage.m_Pivot.m_Y = m_Pivot.m_Y;
				DrawImage.m_Antialias = m_Antialias;
				EGRect ClipRect(BackgroundRect.GetX1() + PadLeft, BackgroundRect.GetY1() + PadTop, BackgroundRect.GetX2() - PadRight, BackgroundRect.GetY2() - PadBottom);
				const EGRect *pOriginalClip = pContext->m_pClipRect;
				if(!ClipRect.Intersect(&ClipRect, pContext->m_pClipRect)) return;
				pContext->m_pClipRect = &ClipRect;
				EGRect SegmentRect;
				EG_Coord_t OffsetX = m_Offset.m_X % m_Width;
				EG_Coord_t OffsetY = m_Offset.m_Y % m_Height;
				SegmentRect.SetY1(ImageRect.GetY1() + OffsetY);
				if(SegmentRect.GetY1() > ImageRect.GetY1()) SegmentRect.DecY1(m_Height);
				SegmentRect.SetY2(SegmentRect.GetY1() + m_Height - 1);
				for(; SegmentRect.GetY1() < ImageRect.GetY2(); SegmentRect.IncY1(ImageSize.m_Y), SegmentRect.IncY2(ImageSize.m_Y)) {
					SegmentRect.SetX1(ImageRect.GetX1() + OffsetX);
					if(SegmentRect.GetX1() > ImageRect.GetX1()) SegmentRect.DecX1(m_Width);
					SegmentRect.SetX2(SegmentRect.GetX1() + m_Width - 1);
					for(; SegmentRect.GetX1() < ImageRect.GetX2(); SegmentRect.IncX1(ImageSize.m_X), SegmentRect.IncX2(ImageSize.m_X)) {
						DrawImage.Draw(pContext, &SegmentRect, m_pSource);
					}
				}
				pContext->m_pClipRect = pOriginalClip;
			}
			else if(m_SourceType == EG_IMG_SRC_SYMBOL) {
				EGDrawLabel DrawLabel;
				InititialseDrawLabel(EG_PART_MAIN, &DrawLabel);
				DrawLabel.Draw(pContext, &m_Rect, (char*)m_pSource, nullptr);
			}
			else {		// Trigger the error handler of image draw
				EG_LOG_WARN("Image source type is unknown");
				EGDrawImage DrawImage;
				DrawImage.Draw(pContext, &m_Rect, nullptr);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::ScaleUpdate(int32_t ScaleX, int32_t ScaleY)
{
    UpdateLayout();  /*Be sure the object's size is calculated*/
    int32_t Width = GetWidth();
    int32_t Height = GetHeight();
    EGRect Rect;
    EGPoint PivotPoint = m_Pivot;
    EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, /*m_ScaleX,*/ m_ScaleY, &PivotPoint);
    Rect.Move(m_Rect.GetX1() - 1, m_Rect.GetY1() - 1, m_Rect.GetX1() + 1, m_Rect.GetY1() + 1);
    InvalidateArea(&Rect);
    m_ScaleX = ScaleX;
    m_ScaleY = ScaleY;
    // Disable invalidations because the following would invalidate the whole ext draw area 
    EGDisplay *pDisp = GetDisplay();
    EGDisplay::EnableInvalidation(pDisp, false);
    RefreshExtDrawSize();
    EGDisplay::EnableInvalidation(pDisp, true);
    EGImageBuffer::GetTransformedRect(&Rect, Width, Height, m_Rotation, /*m_ScaleX,*/ m_ScaleY, &PivotPoint);
    Rect.Move(m_Rect.GetX1() - 1, m_Rect.GetY1() - 1, m_Rect.GetX1() + 1, m_Rect.GetY1() + 1);
    InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImage::UpdateAlign(void)
{
  switch(m_Align){
    case EG_IMAGE_ALIGN_STRETCH:{
      SetRotation(0);
      SetPivot(0, 0);
      if(m_Width != 0 && m_Height != 0) {
        UpdateLayout();
        int32_t ScaleX = GetWidth() * EG_SCALE_NONE / m_Width;
        int32_t ScaleY = GetHeight() * EG_SCALE_NONE / m_Height;
        int32_t Scale = EG_MIN(ScaleX, ScaleY);   // REMOVE THIS
        ScaleUpdate(Scale, Scale);
      }
      break;
    }
    case EG_IMAGE_ALIGN_CONTAIN: {
      SetRotation(0);
      SetPivot(0, 0);
      if(m_Width != 0 && m_Height != 0) {
        UpdateLayout();
        int32_t ScaleX = GetWidth() * EG_SCALE_NONE / m_Width;
        int32_t ScaleY = GetHeight() * EG_SCALE_NONE / m_Height;
        int32_t Scale = EG_MIN(ScaleX, ScaleY);
        ScaleUpdate(Scale, Scale);
      }
      break;
    }
    case EG_IMAGE_ALIGN_COVER: {
      SetRotation(0);
      SetPivot(0, 0);
      if(m_Width != 0 && m_Height != 0) {
        UpdateLayout();
        int32_t ScaleX = GetWidth() * EG_SCALE_NONE / m_Width;
        int32_t ScaleY = GetHeight() * EG_SCALE_NONE / m_Height;
        int32_t Scale = EG_MAX(ScaleX, ScaleY);
        ScaleUpdate(Scale, Scale);
      }
      break;
    }
    case EG_IMAGE_ALIGN_TILE: {
      SetRotation(0);
      SetPivot(0, 0);
      ScaleUpdate(EG_SCALE_NONE, EG_SCALE_NONE);
      break;
    }
    default: break;
  }
}

#endif
