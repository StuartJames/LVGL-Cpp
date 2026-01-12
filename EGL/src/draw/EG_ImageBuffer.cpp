/*/ 
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
 

#include <stddef.h>
#include <string.h>
#include "draw/EG_ImageBuffer.h"
#include "draw/EG_DrawImage.h"
#include "misc/EG_Math.h"
#include "misc/EG_Log.h"
#include "misc/EG_Memory.h"

//////////////////////////////////////////////////////////////////////////////////////

EGImageBuffer::EGImageBuffer(void) :
  m_DataSize(0),
  m_pData(nullptr)
{
}

//////////////////////////////////////////////////////////////////////////////////////

EGImageBuffer::EGImageBuffer(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat)
{
  Allocate(Width, Height, ColorFormat);
}

//////////////////////////////////////////////////////////////////////////////////////

EGImageBuffer::EGImageBuffer(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat, const uint8_t *pData, uint32_t Size /*= 0*/)
{
	if(Size == 0) m_DataSize = CalculateBufferSize(Width, Height, ColorFormat);	// Get image data size
  else m_DataSize = Size;
	m_Header.AlwaysZero = 0;	      // Fill in header
	m_Header.Width = Width;
	m_Header.Height = Height;
	m_Header.ColorFormat = ColorFormat;
	m_Header.Reserved = 0;
  m_pData = pData;
}

//////////////////////////////////////////////////////////////////////////////////////

EGImageBuffer::~EGImageBuffer(void)
{
	if(m_pData != nullptr)	EG_FreeMem((void *)m_pData);
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGImageBuffer::Allocate(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat)
{
	m_DataSize = CalculateBufferSize(Width, Height, ColorFormat);	// Get image data size
	if(m_DataSize == 0) return false;
	m_pData = (uint8_t *)EG_AllocMem(m_DataSize);	// Allocate raw buffer
	if(m_pData == nullptr) return false;
	EG_ZeroMem((uint8_t *)m_pData, m_DataSize);
	m_Header.AlwaysZero = 0;	      // Fill in header
	m_Header.Width = Width;
	m_Header.Height = Height;
	m_Header.ColorFormat = ColorFormat;
	m_Header.Reserved = 0;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGImageBuffer::GetPixelColor(EG_Coord_t x, EG_Coord_t y, EG_Color_t color)
{
	EG_Color_t p_color = EG_ColorBlack();
	uint8_t *buf_u8 = (uint8_t *)m_pData;

	if(m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR || m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED ||
		 m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_ALPHA || m_Header.ColorFormat == EG_IMG_CF_RGB565A8) {
		uint8_t px_size = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)m_Header.ColorFormat) >> 3;
		uint32_t px = m_Header.Width * y * px_size + x * px_size;
		EG_CopyMemSmall(&p_color, &buf_u8[px], sizeof(EG_Color_t));
#if EG_COLOR_SIZE == 32
		p_color.ch.alpha = 0xFF; // Only the color should be get so use a default alpha value
#endif
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_1BIT) {
		buf_u8 += 4 * 2;
		uint8_t bit = x & 0x7;
		x = x >> 3;
	// m_Header.Width + 7 means rounding up to 8 because the lines are byte aligned so the possible real width are 8, 16, 24 ...
		uint32_t px = ((m_Header.Width + 7) >> 3) * y + x;
		p_color.full = (buf_u8[px] & (1 << (7 - bit))) >> (7 - bit);
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_2BIT) {
		buf_u8 += 4 * 4;
		uint8_t bit = (x & 0x3) * 2;
		x = x >> 2;
	// m_Header.Width + 3 means rounding up to 4 because the lines are byte aligned so the possible real width are 4, 8, 12 ...
		uint32_t px = ((m_Header.Width + 3) >> 2) * y + x;
		p_color.full = (buf_u8[px] & (3 << (6 - bit))) >> (6 - bit);
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_4BIT) {
		buf_u8 += 4 * 16;
		uint8_t bit = (x & 0x1) * 4;
		x = x >> 1;
	// m_Header.Width + 1 means rounding up to 2 because the lines are byte aligned so the possible real width are 2, 4, 6 ...
		uint32_t px = ((m_Header.Width + 1) >> 1) * y + x;
		p_color.full = (buf_u8[px] & (0xF << (4 - bit))) >> (4 - bit);
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_8BIT) {
		buf_u8 += 4 * 256;
		uint32_t px = m_Header.Width * y + x;
		p_color.full = buf_u8[px];
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_1BIT || m_Header.ColorFormat == EG_IMG_CF_ALPHA_2BIT ||
					m_Header.ColorFormat == EG_IMG_CF_ALPHA_4BIT || m_Header.ColorFormat == EG_IMG_CF_ALPHA_8BIT) {
		p_color = color;
	}
	return p_color;
}

//////////////////////////////////////////////////////////////////////////////////////

EG_OPA_t EGImageBuffer::GetPixelAlpha(EG_Coord_t x, EG_Coord_t y)
{
	uint8_t *buf_u8 = (uint8_t *)m_pData;

	if(m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_ALPHA) {
		uint32_t px = m_Header.Width * y * EG_IMG_PX_SIZE_ALPHA_BYTE + x * EG_IMG_PX_SIZE_ALPHA_BYTE;
		return buf_u8[px + EG_IMG_PX_SIZE_ALPHA_BYTE - 1];
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_1BIT) {
		uint8_t bit = x & 0x7;
		x = x >> 3;
	// m_Header.Width + 7 means rounding up to 8 because the lines are byte aligned so the possible real width are 8 ,16, 24 ...
		uint32_t px = ((m_Header.Width + 7) >> 3) * y + x;
		uint8_t px_opa = (buf_u8[px] & (1 << (7 - bit))) >> (7 - bit);
		return px_opa ? EG_OPA_TRANSP : EG_OPA_COVER;
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_2BIT) {
		const uint8_t opa_table[4] = {0, 85, 170, 255}; // Opacity mapping with BitsPerPixel = 2
		uint8_t bit = (x & 0x3) * 2;
		x = x >> 2;
	// m_Header.Width + 4 means rounding up to 8 because the lines are byte aligned so the possible real width are 4 ,8, 12 ...
		uint32_t px = ((m_Header.Width + 3) >> 2) * y + x;
		uint8_t px_opa = (buf_u8[px] & (3 << (6 - bit))) >> (6 - bit);
		return opa_table[px_opa];
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_4BIT) {
		const uint8_t opa_table[16] = {0, 17, 34, 51, // Opacity mapping with BitsPerPixel = 4
																	 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};
		uint8_t bit = (x & 0x1) * 4;
		x = x >> 1;
	// m_Header.Width + 1 means rounding up to 8 because the lines are byte aligned so the possible real width are 2 ,4, 6 ...
		uint32_t px = ((m_Header.Width + 1) >> 1) * y + x;
		uint8_t px_opa = (buf_u8[px] & (0xF << (4 - bit))) >> (4 - bit);
		return opa_table[px_opa];
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_8BIT) {
		uint32_t px = m_Header.Width * y + x;
		return buf_u8[px];
	}
	return EG_OPA_COVER;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGImageBuffer::SetPixelAlpha(EG_Coord_t x, EG_Coord_t y, EG_OPA_t opa)
{
	uint8_t *buf_u8 = (uint8_t *)m_pData;

	if(m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_ALPHA) {
		uint8_t px_size = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)m_Header.ColorFormat) >> 3;
		uint32_t px = m_Header.Width * y * px_size + x * px_size;
		buf_u8[px + px_size - 1] = opa;
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_1BIT) {
		opa = opa >> 7; // opa -> [0,1]
		uint8_t bit = x & 0x7;
		x = x >> 3;
	// m_Header.Width + 7 means rounding up to 8 because the lines are byte aligned so the possible real width are 8 ,16, 24 ...
		uint32_t px = ((m_Header.Width + 7) >> 3) * y + x;
		buf_u8[px] = buf_u8[px] & ~(1 << (7 - bit));
		buf_u8[px] = buf_u8[px] | ((opa & 0x1) << (7 - bit));
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_2BIT) {
		opa = opa >> 6; // opa -> [0,3]
		uint8_t bit = (x & 0x3) * 2;
		x = x >> 2;
	// m_Header.Width + 4 means rounding up to 8 because the lines are byte aligned so the possible real width are 4 ,8, 12 ...
		uint32_t px = ((m_Header.Width + 3) >> 2) * y + x;
		buf_u8[px] = buf_u8[px] & ~(3 << (6 - bit));
		buf_u8[px] = buf_u8[px] | ((opa & 0x3) << (6 - bit));
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_4BIT) {
		opa = opa >> 4; // opa -> [0,15]
		uint8_t bit = (x & 0x1) * 4;
		x = x >> 1;
	// m_Header.Width + 1 means rounding up to 8 because the lines are byte aligned so the possible real width are 2 ,4, 6 ...
		uint32_t px = ((m_Header.Width + 1) >> 1) * y + x;
		buf_u8[px] = buf_u8[px] & ~(0xF << (4 - bit));
		buf_u8[px] = buf_u8[px] | ((opa & 0xF) << (4 - bit));
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_ALPHA_8BIT) {
		uint32_t px = m_Header.Width * y + x;
		buf_u8[px] = opa;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGImageBuffer::SetPixelColor(EG_Coord_t x, EG_Coord_t y, EG_Color_t c)
{
uint8_t *buf_u8 = (uint8_t *)m_pData;

	if(m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR || m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED) {
		uint8_t px_size = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)m_Header.ColorFormat) >> 3;
		uint32_t px = m_Header.Width * y * px_size + x * px_size;
		EG_CopyMemSmall(&buf_u8[px], &c, px_size);
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_TRUE_COLOR_ALPHA) {
		uint8_t px_size = EGDrawImage::GetPixelSize((EG_ImageColorFormat_t)m_Header.ColorFormat) >> 3;
		uint32_t px = m_Header.Width * y * px_size + x * px_size;
		EG_CopyMemSmall(&buf_u8[px], &c, px_size - 1); // -1 to not overwrite the alpha value
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_1BIT) {
		buf_u8 += sizeof(EG_Color32_t) * 2; // Skip the palette
		uint8_t bit = x & 0x7;
		x = x >> 3;
	// m_Header.Width + 7 means rounding up to 8 because the lines are byte aligned so the possible real width are 8 ,16, 24 ...
		uint32_t px = ((m_Header.Width + 7) >> 3) * y + x;
		buf_u8[px] = buf_u8[px] & ~(1 << (7 - bit));
		buf_u8[px] = buf_u8[px] | ((c.full & 0x1) << (7 - bit));
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_2BIT) {
		buf_u8 += sizeof(EG_Color32_t) * 4; // Skip the palette
		uint8_t bit = (x & 0x3) * 2;
		x = x >> 2;
	// m_Header.Width + 3 means rounding up to 4 because the lines are byte aligned so the possible real width are 4, 8 ,12 ...
		uint32_t px = ((m_Header.Width + 3) >> 2) * y + x;
		buf_u8[px] = buf_u8[px] & ~(3 << (6 - bit));
		buf_u8[px] = buf_u8[px] | ((c.full & 0x3) << (6 - bit));
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_4BIT) {
		buf_u8 += sizeof(EG_Color32_t) * 16; // Skip the palette
		uint8_t bit = (x & 0x1) * 4;
		x = x >> 1;
	// m_Header.Width + 1 means rounding up to 2 because the lines are byte aligned so the possible real width are 2 ,4, 6 ...
		uint32_t px = ((m_Header.Width + 1) >> 1) * y + x;
		buf_u8[px] = buf_u8[px] & ~(0xF << (4 - bit));
		buf_u8[px] = buf_u8[px] | ((c.full & 0xF) << (4 - bit));
	}
	else if(m_Header.ColorFormat == EG_IMG_CF_INDEXED_8BIT) {
		buf_u8 += sizeof(EG_Color32_t) * 256; // Skip the palette
		uint32_t px = m_Header.Width * y + x;
		buf_u8[px] = c.full;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGImageBuffer::SetPalette(uint8_t id, EG_Color_t c)
{
	if((m_Header.ColorFormat == EG_IMG_CF_ALPHA_1BIT && id > 1) || (m_Header.ColorFormat == EG_IMG_CF_ALPHA_2BIT && id > 3) ||
		 (m_Header.ColorFormat == EG_IMG_CF_ALPHA_4BIT && id > 15) || (m_Header.ColorFormat == EG_IMG_CF_ALPHA_8BIT)) {
		EG_LOG_WARN("lv_img_buf_set_px_alpha: invalid 'id'");
		return;
	}

	EG_Color32_t c32;
	c32.full = EG_ColorTo32(c);
	uint8_t *buf = (uint8_t *)m_pData;
	EG_CopyMemSmall(&buf[id * sizeof(c32)], &c32, sizeof(c32));
}

//////////////////////////////////////////////////////////////////////////////////////

uint32_t EGImageBuffer::CalculateBufferSize(EG_Coord_t Width, EG_Coord_t Height, EG_ImageColorFormat_t ColorFormat)
{
	switch(ColorFormat) {
		case EG_IMG_CF_TRUE_COLOR:
			return EG_IMG_BUF_SIZE_TRUE_COLOR(Width, Height);
		case EG_IMG_CF_TRUE_COLOR_ALPHA:
		case EG_IMG_CF_RGB565A8:
			return EG_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(Width, Height);
		case EG_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
			return EG_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(Width, Height);
		case EG_IMG_CF_ALPHA_1BIT:
			return EG_IMG_BUF_SIZE_ALPHA_1BIT(Width, Height);
		case EG_IMG_CF_ALPHA_2BIT:
			return EG_IMG_BUF_SIZE_ALPHA_2BIT(Width, Height);
		case EG_IMG_CF_ALPHA_4BIT:
			return EG_IMG_BUF_SIZE_ALPHA_4BIT(Width, Height);
		case EG_IMG_CF_ALPHA_8BIT:
			return EG_IMG_BUF_SIZE_ALPHA_8BIT(Width, Height);
		case EG_IMG_CF_INDEXED_1BIT:
			return EG_IMG_BUF_SIZE_INDEXED_1BIT(Width, Height);
		case EG_IMG_CF_INDEXED_2BIT:
			return EG_IMG_BUF_SIZE_INDEXED_2BIT(Width, Height);
		case EG_IMG_CF_INDEXED_4BIT:
			return EG_IMG_BUF_SIZE_INDEXED_4BIT(Width, Height);
		case EG_IMG_CF_INDEXED_8BIT:
			return EG_IMG_BUF_SIZE_INDEXED_8BIT(Width, Height);
		default:
			return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGImageBuffer::GetTransformedRect(EGRect *pRect, EG_Coord_t Width, EG_Coord_t Height, int16_t Angle, uint16_t Zoom,
                                        const EGPoint *pPivot)
{
#if EG_DRAW_COMPLEX
EGPoint Point1(0,0);
EGPoint Point2(Width,0);
EGPoint Point3(0,Height);
EGPoint Point4(Width,Height);

  if(Angle == 0 && Zoom == EG_IMG_ZOOM_NONE) {
		pRect->SetX1(0);
		pRect->SetY1(0);
		pRect->SetX2(Width - 1);
		pRect->SetY2(Height - 1);
		return;
	}
	Point1.PointTransform(Angle, Zoom, pPivot);
	Point2.PointTransform(Angle, Zoom, pPivot);
	Point3.PointTransform(Angle, Zoom, pPivot);
	Point4.PointTransform(Angle, Zoom, pPivot);
	pRect->SetX1(EG_MIN4(Point1.m_X, Point2.m_X, Point3.m_X, Point4.m_X) - 2);
	pRect->SetX2(EG_MAX4(Point1.m_X, Point2.m_X, Point3.m_X, Point4.m_X) + 2);
	pRect->SetY1(EG_MIN4(Point1.m_Y, Point2.m_Y, Point3.m_Y, Point4.m_Y) - 2);
	pRect->SetY2(EG_MAX4(Point1.m_Y, Point2.m_Y, Point3.m_Y, Point4.m_Y) + 2);
#else
	EG_UNUSED(Angle);
	EG_UNUSED(Zoom);
	EG_UNUSED(pPivot);
	pRect->SetX1(0);
	pRect->SetY1(0);
	pRect->SetX2(Width - 1);
	pRect->SetY2(Height - 1);
#endif
}
