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
 *  Based on Rect design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.Rect.1    Original by LVGL Kft
 *
 */

#include "widgets/EG_Canvas.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Math.h"
#include "draw/EG_DrawContext.h"
#include "core/EG_Refresh.h"

#if EG_USE_CANVAS != 0

#include "draw/sw/EG_SoftContext.h"

///////////////////////////////////////////////////////////////////////////////////////

#define CANVAS_CLASS &c_CanvasClass

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_CanvasClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
	.pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGCanvas::EGCanvas(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_LedClass*/) : EGObject()
{
	m_ImageBuffer.m_Header.AlwaysZero = 0;
	m_ImageBuffer.m_Header.ColorFormat = EG_IMG_CF_TRUE_COLOR;
	m_ImageBuffer.m_Header.Height = 0;
	m_ImageBuffer.m_Header.Width = 0;
	m_ImageBuffer.m_DataSize = 0;
	m_ImageBuffer.m_pData = nullptr;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

EGCanvas::~EGCanvas(void)
{
	InvalidateImageCacheSource(&m_ImageBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::Configure(void)
{
  EGObject::Configure();
	m_ImageBuffer.m_Header.AlwaysZero = 0;
	m_ImageBuffer.m_Header.ColorFormat = EG_IMG_CF_TRUE_COLOR;
	m_ImageBuffer.m_Header.Height = 0;
	m_ImageBuffer.m_Header.Width = 0;
	m_ImageBuffer.m_DataSize = 0;
	m_ImageBuffer.m_pData = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::SetBuffer(void *pBuffer, EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat)
{
	EG_ASSERT_NULL(pBuffer);
	m_ImageBuffer.m_Header.ColorFormat = ColorFormat;
	m_ImageBuffer.m_Header.Width = Width;
	m_ImageBuffer.m_Header.Height = Height;
	m_ImageBuffer.m_pData = (uint8_t *)pBuffer;
	InvalidateImageCacheSource(&m_ImageBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::SetPixelColor(EG_Coord_t X, EG_Coord_t Y, EG_Color_t Color)
{
	m_ImageBuffer.SetPixelColor(X, Y, Color);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::SetPixelOPA(EG_Coord_t X, EG_Coord_t Y, EG_OPA_t OPA)
{
	m_ImageBuffer.SetPixelAlpha(X, Y, OPA);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::SetPalette(uint8_t ID, EG_Color_t Color)
{
	m_ImageBuffer.SetPalette(ID, Color);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGCanvas::GetPixel(EG_Coord_t X, EG_Coord_t Y)
{
	EG_Color_t Color = GetStyleImageRecolor(EG_PART_MAIN);
	return m_ImageBuffer.GetPixelColor(X, Y, Color);
}

///////////////////////////////////////////////////////////////////////////////////////

EGImageBuffer* EGCanvas::GetImage(void)
{
	return &m_ImageBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::CopyBuffer(const void *pDest, EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Width, EG_Coord_t Height)
{
	EG_ASSERT_NULL(pDest);
	if(X + Width - 1 >= (EG_Coord_t)m_ImageBuffer.m_Header.Width || Y + Height - 1 >= (EG_Coord_t)m_ImageBuffer.m_Header.Height) {
		EG_LOG_WARN("lv_canvas_copy_buf: X or Y out of the canvas");
		return;
	}

	uint32_t px_size = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat) >> 3;
	uint32_t px = m_ImageBuffer.m_Header.Width * Y * px_size + X * px_size;
	uint8_t *to_copy8 = (uint8_t *)pDest;
	EG_Coord_t i;
	for(i = 0; i < Height; i++) {
		EG_CopyMem((void *)&m_ImageBuffer.m_pData[px], to_copy8, Width * px_size);
		px += m_ImageBuffer.m_Header.Width * px_size;
		to_copy8 += Width * px_size;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::Transform(EGImageBuffer *pSource, int16_t Angle, uint16_t Zoom, EG_Coord_t OffsetX, EG_Coord_t OffsetY,
												 int32_t PivotX, int32_t PivotY, bool Antialias)
{
#if EG_DRAW_COMPLEX
int32_t X;
int32_t Y;

	EG_ASSERT_NULL(pSource);
  EGImageBuffer *pDest = &m_ImageBuffer;
	EGDrawImage LineBuffer;
	LineBuffer.m_Angle = Angle;
	LineBuffer.m_Zoom = Zoom;
	LineBuffer.m_Pivot.m_X = PivotX;
	LineBuffer.m_Pivot.m_Y = PivotY;
	LineBuffer.m_Antialias = Antialias;
	EGRect DestRect;
	DestRect.SetX1(-OffsetX);
	DestRect.SetX2(DestRect.GetX1() + pDest->m_Header.Width - 1);
	DestRect.SetY1(-OffsetY);
	DestRect.SetY2(-OffsetY);
	EG_Color_t *pColorBuffer = (EG_Color_t *)EG_AllocMem(pDest->m_Header.Width * sizeof(EG_Color_t));
	EG_OPA_t *pOpaBuffer = (EG_OPA_t *)EG_AllocMem(pDest->m_Header.Width * sizeof(EG_OPA_t));
	for(Y = 0; Y < pDest->m_Header.Height; Y++) {
		EGSoftContext::DrawTransform(&DestRect, pSource->m_pData, pSource->m_Header.Width, pSource->m_Header.Height, pSource->m_Header.Width,
												 &LineBuffer, (EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat, pColorBuffer, pOpaBuffer);
		for(X = 0; X < pDest->m_Header.Width; X++) {
			if(pOpaBuffer[X]){
				pDest->SetPixelColor(X, Y, pColorBuffer[X]);
				pDest->SetPixelAlpha(X, Y, pOpaBuffer[X]);
			}
		}
		DestRect.IncX1(1);
		DestRect.IncY2(1);
	}
	EG_FreeMem(pColorBuffer);
	EG_FreeMem(pOpaBuffer);
	Invalidate();

#else
	EG_UNUSED(pSource);
	EG_UNUSED(Angle);
	EG_UNUSED(Zoom);
	EG_UNUSED(OffsetX);
	EG_UNUSED(OffsetY);
	EG_UNUSED(PivotX);
	EG_UNUSED(PivotY);
	EG_UNUSED(Antialias);
	EG_LOG_WARN("Can't transform canvas with EG_DRAW_COMPLEX == 0");
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::BlurHorizontal(const EGRect *pRect, uint16_t Radius)
{
	if(Radius == 0) return;
	EGRect Rect;
	if(pRect) {
		pRect->Copy(&Rect);
		if(Rect.GetX1() < 0) Rect.SetX1(0);
		if(Rect.GetY1() < 0) Rect.SetY1(0);
		if(Rect.GetX2() > m_ImageBuffer.m_Header.Width - 1) Rect.SetX2(m_ImageBuffer.m_Header.Width - 1);
		if(Rect.GetY2() > m_ImageBuffer.m_Header.Height - 1) Rect.SetY2(m_ImageBuffer.m_Header.Height - 1);
	}
	else Rect.Set(0, 0, m_ImageBuffer.m_Header.Width - 1, m_ImageBuffer.m_Header.Height - 1);
	EG_Color_t Color = GetStyleImageRecolor(EG_PART_MAIN);
	uint16_t BackRadius = Radius / 2;
	uint16_t FrontRadius = Radius / 2;
	if((Radius & 0x1) == 0) BackRadius--;
	bool HasAlpha = EGDrawImage::HasAlpha((EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat);
	EG_Coord_t LineWidth = m_ImageBuffer.CalculateBufferSize(m_ImageBuffer.m_Header.Width, 1, (EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat);
	EGImageBuffer LineBuffer;
	LineBuffer.m_pData = (uint8_t *)EG_GetBufferMem(LineWidth);
	LineBuffer.m_Header.AlwaysZero = 0;
	LineBuffer.m_Header.Width = m_ImageBuffer.m_Header.Width;
	LineBuffer.m_Header.Height = 1;
	LineBuffer.m_Header.ColorFormat = m_ImageBuffer.m_Header.ColorFormat;
	EG_Coord_t X;
	EG_Coord_t Y;
	EG_Coord_t SafeX;
	for(Y = Rect.GetY1(); Y <= Rect.GetY2(); Y++) {
		uint32_t asum = 0;
		uint32_t rsum = 0;
		uint32_t gsum = 0;
		uint32_t bsum = 0;
		EG_Color_t BlurColor;
		EG_OPA_t BlurOPA = EG_OPA_TRANSP;
		EG_CopyMem(&LineBuffer.m_pData , &m_ImageBuffer.m_pData[Y * LineWidth], LineWidth);
		for(X = Rect.GetX1() - BackRadius; X <= Rect.GetX1() + FrontRadius; X++) {
			SafeX = X < 0 ? 0 : X;
			SafeX = SafeX > m_ImageBuffer.m_Header.Width - 1 ? m_ImageBuffer.m_Header.Width - 1 : SafeX;
			BlurColor = LineBuffer.GetPixelColor(SafeX, 0, Color);
			if(HasAlpha) BlurOPA = LineBuffer.GetPixelAlpha(SafeX, 0);
			rsum += BlurColor.ch.red;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
			gsum += (BlurColor.ch.green_h << 3) + BlurColor.ch.green_l;
#else
			gsum += BlurColor.ch.green;
#endif
			bsum += BlurColor.ch.blue;
			if(HasAlpha) asum += BlurOPA;
		}
		if(HasAlpha == false) asum = EG_OPA_COVER;		// Just to indicate that the px is visible
		for(X = Rect.GetX1(); X <= Rect.GetX2(); X++) {
			if(asum) {
				BlurColor.ch.red = rsum / Radius;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
				uint8_t gtmp = gsum / Radius;
				BlurColor.ch.green_h = gtmp >> 3;
				BlurColor.ch.green_l = gtmp & 0x7;
#else
				BlurColor.ch.green = gsum / Radius;
#endif
				BlurColor.ch.blue = bsum / Radius;
				if(HasAlpha) BlurOPA = asum / Radius;
				m_ImageBuffer.SetPixelColor(X, Y, BlurColor);
			}
			if(HasAlpha) m_ImageBuffer.SetPixelAlpha(X, Y, BlurOPA);
			SafeX = X - BackRadius;
			SafeX = SafeX < 0 ? 0 : SafeX;
			BlurColor = LineBuffer.GetPixelColor(SafeX, 0, Color);
			if(HasAlpha) BlurOPA = LineBuffer.GetPixelAlpha(SafeX, 0);
			rsum -= BlurColor.ch.red;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
			gsum -= (BlurColor.ch.green_h << 3) + BlurColor.ch.green_l;
#else
			gsum -= BlurColor.ch.green;
#endif
			bsum -= BlurColor.ch.blue;
			if(HasAlpha) asum -= BlurOPA;
			SafeX = X + 1 + FrontRadius;
			SafeX = SafeX > m_ImageBuffer.m_Header.Width - 1 ? m_ImageBuffer.m_Header.Width - 1 : SafeX;
			BlurColor = LineBuffer.GetPixelColor(SafeX, 0, EG_ColorWhite());
			if(HasAlpha) BlurOPA = LineBuffer.GetPixelAlpha(SafeX, 0);
			rsum += BlurColor.ch.red;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
			gsum += (BlurColor.ch.green_h << 3) + BlurColor.ch.green_l;
#else
			gsum += BlurColor.ch.green;
#endif
			bsum += BlurColor.ch.blue;
			if(HasAlpha) asum += BlurOPA;
		}
	}
	Invalidate();
	EG_ReleaseBufferMem(&LineBuffer.m_pData);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::BlurVertical(const EGRect *pRect, uint16_t Radius)
{
	if(Radius == 0) return;
	EGRect Rect;
	if(pRect) {
		pRect->Copy(&Rect);
		if(Rect.GetX1() < 0) Rect.SetX1(0);
		if(Rect.GetY1() < 0) Rect.SetY1(0);
		if(Rect.GetX2() > m_ImageBuffer.m_Header.Width - 1) Rect.SetX2(m_ImageBuffer.m_Header.Width - 1);
		if(Rect.GetY2() > m_ImageBuffer.m_Header.Height - 1) Rect.SetY2(m_ImageBuffer.m_Header.Height - 1);
	}
	else Rect.Set(0, 0, m_ImageBuffer.m_Header.Width - 1, m_ImageBuffer.m_Header.Height - 1);
	EG_Color_t Color = GetStyleImageRecolor(EG_PART_MAIN);
	uint16_t BackRadius = Radius / 2;
	uint16_t FrontRadius = Radius / 2;
	if((Radius & 0x1) == 0) BackRadius--;
	bool HasAlpha = EGDrawImage::HasAlpha((EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat);
	EG_Coord_t ColumnWidth = m_ImageBuffer.CalculateBufferSize(1, m_ImageBuffer.m_Header.Height, (EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat);
	EGImageBuffer LineBuffer;
	LineBuffer.m_pData = (uint8_t *)EG_GetBufferMem(ColumnWidth);
	LineBuffer.m_Header.AlwaysZero = 0;
	LineBuffer.m_Header.Width = 1;
	LineBuffer.m_Header.Height = m_ImageBuffer.m_Header.Height;
	LineBuffer.m_Header.ColorFormat = m_ImageBuffer.m_Header.ColorFormat;
	EG_Coord_t X;
	EG_Coord_t Y;
	EG_Coord_t SafeY;
	for(X = Rect.GetX1(); X <= Rect.GetX2(); X++) {
		uint32_t asum = 0;
		uint32_t rsum = 0;
		uint32_t gsum = 0;
		uint32_t bsum = 0;
		EG_Color_t BlurColor;
		EG_OPA_t BlurOPA = EG_OPA_COVER;
		for(Y = Rect.GetY1() - BackRadius; Y <= Rect.GetY1() + FrontRadius; Y++) {
			SafeY = Y < 0 ? 0 : Y;
			SafeY = SafeY > m_ImageBuffer.m_Header.Height - 1 ? m_ImageBuffer.m_Header.Height - 1 : SafeY;
			BlurColor = m_ImageBuffer.GetPixelColor(X, SafeY, Color);
			if(HasAlpha) BlurOPA = m_ImageBuffer.GetPixelAlpha(X, SafeY);
			LineBuffer.SetPixelColor(0, SafeY, BlurColor);
			if(HasAlpha) LineBuffer.SetPixelAlpha(0, SafeY, BlurOPA);
			rsum += BlurColor.ch.red;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
			gsum += (BlurColor.ch.green_h << 3) + BlurColor.ch.green_l;
#else
			gsum += BlurColor.ch.green;
#endif
			bsum += BlurColor.ch.blue;
			if(HasAlpha) asum += BlurOPA;
		}
		if(HasAlpha == false) asum = EG_OPA_COVER;		// Just to indicate that the px is visible
		for(Y = Rect.GetY1(); Y <= Rect.GetY2(); Y++) {
			if(asum) {
				BlurColor.ch.red = rsum / Radius;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
				uint8_t gtmp = gsum / Radius;
				BlurColor.ch.green_h = gtmp >> 3;
				BlurColor.ch.green_l = gtmp & 0x7;
#else
				BlurColor.ch.green = gsum / Radius;
#endif
				BlurColor.ch.blue = bsum / Radius;
				if(HasAlpha) BlurOPA = asum / Radius;
				m_ImageBuffer.SetPixelColor(X, Y, BlurColor);
			}
			if(HasAlpha) m_ImageBuffer.SetPixelAlpha(X, Y, BlurOPA);
			SafeY = Y - BackRadius;
			SafeY = SafeY < 0 ? 0 : SafeY;
			BlurColor = LineBuffer.GetPixelColor(0, SafeY, Color);
			if(HasAlpha) BlurOPA = LineBuffer.GetPixelAlpha(0, SafeY);
			rsum -= BlurColor.ch.red;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
			gsum -= (BlurColor.ch.green_h << 3) + BlurColor.ch.green_l;
#else
			gsum -= BlurColor.ch.green;
#endif
			bsum -= BlurColor.ch.blue;
			if(HasAlpha) asum -= BlurOPA;
			SafeY = Y + 1 + FrontRadius;
			SafeY = SafeY > m_ImageBuffer.m_Header.Height - 1 ? m_ImageBuffer.m_Header.Height - 1 : SafeY;
			BlurColor = m_ImageBuffer.GetPixelColor(X, SafeY, Color);
			if(HasAlpha) BlurOPA = m_ImageBuffer.GetPixelAlpha(X, SafeY);
			LineBuffer.SetPixelColor(0, SafeY, BlurColor);
			if(HasAlpha) LineBuffer.SetPixelAlpha(0, SafeY, BlurOPA);
			rsum += BlurColor.ch.red;
#if EG_COLOR_DEPTH == 16 && EG_COLOR_16_SWAP
			gsum += (BlurColor.ch.green_h << 3) + BlurColor.ch.green_l;
#else
			gsum += BlurColor.ch.green;
#endif
			bsum += BlurColor.ch.blue;
			if(HasAlpha) asum += BlurOPA;
		}
	}
	Invalidate();
	EG_ReleaseBufferMem(&LineBuffer.m_pData);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::FillBackground(EG_Color_t Color, EG_OPA_t BlurOPA)
{
	if(m_ImageBuffer.m_Header.ColorFormat == EG_IMG_CF_INDEXED_1BIT) {
		uint32_t row_byte_cnt = (m_ImageBuffer.m_Header.Width + 7) >> 3;
		// +8 skip the palette
		EG_SetMem((uint8_t *)m_ImageBuffer.m_pData + 8, Color.full ? 0xff : 0x00, row_byte_cnt * m_ImageBuffer.m_Header.Height);
	}
	else if(m_ImageBuffer.m_Header.ColorFormat == EG_IMG_CF_ALPHA_1BIT) {
		uint32_t row_byte_cnt = (m_ImageBuffer.m_Header.Width + 7) >> 3;
		EG_SetMem((uint8_t *)m_ImageBuffer.m_pData, BlurOPA > EG_OPA_50 ? 0xff : 0x00, row_byte_cnt * m_ImageBuffer.m_Header.Height);
	}
	else {
		uint32_t X;
		uint32_t Y;
		for(Y = 0; Y < m_ImageBuffer.m_Header.Height; Y++) {
			for(X = 0; X < m_ImageBuffer.m_Header.Width; X++) {
				m_ImageBuffer.SetPixelColor(X, Y, Color);
				m_ImageBuffer.SetPixelAlpha(X, Y, BlurOPA);
			}
		}
	}
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DrawRect(EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Width, EG_Coord_t Height, EGDrawRect *pDrawRect)
{

	if(m_ImageBuffer.m_Header.ColorFormat >= EG_IMG_CF_INDEXED_1BIT && m_ImageBuffer.m_Header.ColorFormat <= EG_IMG_CF_INDEXED_8BIT) {
		EG_LOG_WARN("lv_canvas_draw_rect: can't draw to EG_IMG_CF_INDEXED canvas");
		return;
	}
	EGDisplay FakeDisplay;	// Create dummy display to fool the lv_draw function.
	EGDisplayDriver DisplayDriver;
	EGRect ClipRect;
	InitFakeDisplay(&FakeDisplay, &DisplayDriver, &ClipRect);
	EGDisplay *pOriginalRefresh = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	EG_Color_t ctransp = EG_COLOR_CHROMA_KEY;
	if(m_ImageBuffer.m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED && pDrawRect->m_BackgroundColor.full == ctransp.full) {
		FakeDisplay.m_pDriver->m_Antialiasing = 0;
	}
	EGRect Rect(X, Y, X + Width - 1, Y + Height - 1);
	pDrawRect->Draw(DisplayDriver.m_pContext, &Rect);
	SetRefreshingDisplay(pOriginalRefresh);
	DeinitFakeDisplay(&FakeDisplay);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DrawText(EG_Coord_t X, EG_Coord_t Y, EG_Coord_t MaxWidth, EGDrawLabel * pDrawLabel, const char *pText)
{
	if(m_ImageBuffer.m_Header.ColorFormat >= EG_IMG_CF_INDEXED_1BIT && m_ImageBuffer.m_Header.ColorFormat <= EG_IMG_CF_INDEXED_8BIT) {
		EG_LOG_WARN("lv_canvas_draw_text: can't draw to EG_IMG_CF_INDEXED canvas");
		return;
	}
	EGDisplay FakeDisplay;	// Create dummy display to fool the lv_draw function.
	EGDisplayDriver DisplayDriver;
	EGRect ClipRect;
	InitFakeDisplay(&FakeDisplay, &DisplayDriver, &ClipRect);
	EGDisplay *pOriginalRefresh = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	EGRect Rect(X, Y, X + MaxWidth - 1, m_ImageBuffer.m_Header.Height - 1);
	pDrawLabel->Draw(DisplayDriver.m_pContext, &Rect, pText, nullptr);
	SetRefreshingDisplay(pOriginalRefresh);
	DeinitFakeDisplay(&FakeDisplay);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DrawImage(EG_Coord_t X, EG_Coord_t Y, const void *pSource, EGDrawImage * pDrawImage)
{
	if(m_ImageBuffer.m_Header.ColorFormat >= EG_IMG_CF_INDEXED_1BIT && m_ImageBuffer.m_Header.ColorFormat <= EG_IMG_CF_INDEXED_8BIT) {
		EG_LOG_WARN("lv_canvas_draw_img: can't draw to EG_IMG_CF_INDEXED canvas");
		return;
	}
	EG_ImageHeader_t m_Header;
	EG_Result_t res = EGImageDecoder::GetInfo(pSource, &m_Header);
	if(res != EG_RES_OK) {
		EG_LOG_WARN("lv_canvas_draw_img: Couldn't get the image data.");
		return;
	}
	EGDisplay FakeDisplay;	// Create dummy display to fool the lv_draw function.
	EGDisplayDriver DisplayDriver;
	EGRect ClipRect;
	InitFakeDisplay(&FakeDisplay, &DisplayDriver, &ClipRect);
	EGDisplay *pOriginalRefresh = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	EGRect Rect(X, Y, X + m_Header.Width - 1, Y + m_Header.Height - 1);
	pDrawImage->Draw(DisplayDriver.m_pContext, &Rect, pSource);
	SetRefreshingDisplay(pOriginalRefresh);
	DeinitFakeDisplay(&FakeDisplay);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DrawLine(const EGPoint Points[], uint32_t PointCount, EGDrawLine *pDrawLine)
{
	if(m_ImageBuffer.m_Header.ColorFormat >= EG_IMG_CF_INDEXED_1BIT && m_ImageBuffer.m_Header.ColorFormat <= EG_IMG_CF_INDEXED_8BIT) {
		EG_LOG_WARN("lv_canvas_draw_line: can't draw to EG_IMG_CF_INDEXED canvas");
		return;
	}
	EGDisplay FakeDisplay;	// Create dummy display to fool the lv_draw function.
	EGDisplayDriver DisplayDriver;
	EGRect ClipRect;
	InitFakeDisplay(&FakeDisplay, &DisplayDriver, &ClipRect);
	EGDisplay *pOriginalRefresh = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	EG_Color_t ctransp = EG_COLOR_CHROMA_KEY;
	if(m_ImageBuffer.m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED && pDrawLine->m_Color.full == ctransp.full) {
		FakeDisplay.m_pDriver->m_Antialiasing = 0;
	}
	for(uint32_t i = 0; i < PointCount - 1; i++) {
		pDrawLine->Draw(DisplayDriver.m_pContext, &Points[i], &Points[i + 1]);
	}
	SetRefreshingDisplay(pOriginalRefresh);
	DeinitFakeDisplay(&FakeDisplay);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DrawPolygon(const EGPoint Points[], uint32_t PointCount, EGDrawRect *pDrawRect)
{
	if(m_ImageBuffer.m_Header.ColorFormat >= EG_IMG_CF_INDEXED_1BIT && m_ImageBuffer.m_Header.ColorFormat <= EG_IMG_CF_INDEXED_8BIT) {
		EG_LOG_WARN("lv_canvas_draw_polygon: can't draw to EG_IMG_CF_INDEXED canvas");
		return;
	}
	EGDisplay FakeDisplay;	// Create dummy display to fool the lv_draw function.
	EGDisplayDriver DisplayDriver;
	EGRect ClipRect;
	InitFakeDisplay(&FakeDisplay, &DisplayDriver, &ClipRect);
	EGDisplay *pOriginalRefresh = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	EG_Color_t ctransp = EG_COLOR_CHROMA_KEY;
	if(m_ImageBuffer.m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED && pDrawRect->m_BackgroundColor.full == ctransp.full) {
		FakeDisplay.m_pDriver->m_Antialiasing = 0;
	}
	EGDrawPolygon DrawPolygon;
  DrawPolygon.Polygon(DisplayDriver.m_pContext, pDrawRect, Points, PointCount);
	SetRefreshingDisplay(pOriginalRefresh);
	DeinitFakeDisplay(&FakeDisplay);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DrawArc(EG_Coord_t X, EG_Coord_t Y, EG_Coord_t Radius, int32_t StartAngle, int32_t EndAngle, EGDrawArc *pDrawArc)
{
#if EG_DRAW_COMPLEX
	if(m_ImageBuffer.m_Header.ColorFormat >= EG_IMG_CF_INDEXED_1BIT && m_ImageBuffer.m_Header.ColorFormat <= EG_IMG_CF_INDEXED_8BIT) {
		EG_LOG_WARN("lv_canvas_draw_arc: can't draw to EG_IMG_CF_INDEXED canvas");
		return;
	}
	EGDisplay FakeDisplay;	// Create dummy display to fool the lv_draw function.
	EGDisplayDriver DisplayDriver;
	EGRect ClipRect;
	InitFakeDisplay(&FakeDisplay, &DisplayDriver, &ClipRect);
	EGDisplay *pOriginalRefresh = GetRefreshingDisplay();
	SetRefreshingDisplay(&FakeDisplay);
	EGPoint Point = {X, Y};
	pDrawArc->Draw(DisplayDriver.m_pContext, &Point, Radius, StartAngle, EndAngle);
	SetRefreshingDisplay(pOriginalRefresh);
	DeinitFakeDisplay(&FakeDisplay);
	Invalidate();
#else
	EG_UNUSED(X);
	EG_UNUSED(Y);
	EG_UNUSED(Radius);
	EG_UNUSED(StartAngle);
	EG_UNUSED(EndAngle);
	EG_UNUSED(pDrawArc);
	EG_LOG_WARN("Can't draw arc with EG_DRAW_COMPLEX == 0");
#endif
}

void EGCanvas::InitFakeDisplay(EGDisplay *pDisplay, EGDisplayDriver *pDriver, EGRect *ClipRect)
{
	ClipRect->Set(0, 0, m_ImageBuffer.m_Header.Width - 1, m_ImageBuffer.m_Header.Height - 1);
	// Allocate the fake DisplayDriver on the stack as the entire display doesn't outlive this function
	EG_ZeroMem(pDisplay, sizeof(EGDisplay));
	pDisplay->m_pDriver = pDriver;
	pDisplay->InitialiseDriver(pDisplay->m_pDriver);
	pDisplay->m_pDriver->m_HorizontalRes = m_ImageBuffer.m_Header.Width;
	pDisplay->m_pDriver->m_VerticalRes = m_ImageBuffer.m_Header.Height;
	EGSoftContext *pContext =  new EGSoftContext;
	if(pContext == nullptr) return;
	pContext->InitialiseContext();
	pDisplay->m_pDriver->m_pContext = pContext;
	pContext->m_pClipRect = ClipRect;
	pContext->m_pDrawRect = ClipRect;
	pContext->m_pDrawBuffer = (void*)m_ImageBuffer.m_pData;
	pDisplay->UseGenericSetPixelCB(pDisplay->m_pDriver, (EG_ImageColorFormat_t)m_ImageBuffer.m_Header.ColorFormat);
	if(EG_COLOR_SCREEN_TRANSP && m_ImageBuffer.m_Header.ColorFormat != EG_IMG_CF_TRUE_COLOR_ALPHA) {
		pDriver->m_ScreenTransparent = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGCanvas::DeinitFakeDisplay(EGDisplay *pDisplay)
{
	delete pDisplay->m_pDriver->m_pContext;
}

#endif
