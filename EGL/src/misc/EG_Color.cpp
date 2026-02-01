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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "misc/EG_Color.h"
#include "misc/EG_Log.h"

/////////////////////////////////////////////////////////////////////////////

constexpr EG_Color_t MainPalette[] = {
  EG_COLOR_MAKE(0xF4, 0x43, 0x36), EG_COLOR_MAKE(0xE9, 0x1E, 0x63), EG_COLOR_MAKE(0x9C, 0x27, 0xB0), EG_COLOR_MAKE(0x67, 0x3A, 0xB7),
  EG_COLOR_MAKE(0x3F, 0x51, 0xB5), EG_COLOR_MAKE(0x21, 0x96, 0xF3), EG_COLOR_MAKE(0x03, 0xA9, 0xF4), EG_COLOR_MAKE(0x00, 0xBC, 0xD4),
  EG_COLOR_MAKE(0x00, 0x96, 0x88), EG_COLOR_MAKE(0x4C, 0xAF, 0x50), EG_COLOR_MAKE(0x8B, 0xC3, 0x4A), EG_COLOR_MAKE(0xCD, 0xDC, 0x39),
  EG_COLOR_MAKE(0xFF, 0xEB, 0x3B), EG_COLOR_MAKE(0xFF, 0xC1, 0x07), EG_COLOR_MAKE(0xFF, 0x98, 0x00), EG_COLOR_MAKE(0xFF, 0x57, 0x22),
  EG_COLOR_MAKE(0x79, 0x55, 0x48), EG_COLOR_MAKE(0x60, 0x7D, 0x8B), EG_COLOR_MAKE(0x9E, 0x9E, 0x9E)
};

constexpr EG_Color_t LightPalette[][5] = {
  {EG_COLOR_MAKE(0xEF, 0x53, 0x50), EG_COLOR_MAKE(0xE5, 0x73, 0x73), EG_COLOR_MAKE(0xEF, 0x9A, 0x9A), EG_COLOR_MAKE(0xFF, 0xCD, 0xD2), EG_COLOR_MAKE(0xFF, 0xEB, 0xEE)},
  {EG_COLOR_MAKE(0xEC, 0x40, 0x7A), EG_COLOR_MAKE(0xF0, 0x62, 0x92), EG_COLOR_MAKE(0xF4, 0x8F, 0xB1), EG_COLOR_MAKE(0xF8, 0xBB, 0xD0), EG_COLOR_MAKE(0xFC, 0xE4, 0xEC)},
  {EG_COLOR_MAKE(0xAB, 0x47, 0xBC), EG_COLOR_MAKE(0xBA, 0x68, 0xC8), EG_COLOR_MAKE(0xCE, 0x93, 0xD8), EG_COLOR_MAKE(0xE1, 0xBE, 0xE7), EG_COLOR_MAKE(0xF3, 0xE5, 0xF5)},
  {EG_COLOR_MAKE(0x7E, 0x57, 0xC2), EG_COLOR_MAKE(0x95, 0x75, 0xCD), EG_COLOR_MAKE(0xB3, 0x9D, 0xDB), EG_COLOR_MAKE(0xD1, 0xC4, 0xE9), EG_COLOR_MAKE(0xED, 0xE7, 0xF6)},
  {EG_COLOR_MAKE(0x5C, 0x6B, 0xC0), EG_COLOR_MAKE(0x79, 0x86, 0xCB), EG_COLOR_MAKE(0x9F, 0xA8, 0xDA), EG_COLOR_MAKE(0xC5, 0xCA, 0xE9), EG_COLOR_MAKE(0xE8, 0xEA, 0xF6)},
  {EG_COLOR_MAKE(0x42, 0xA5, 0xF5), EG_COLOR_MAKE(0x64, 0xB5, 0xF6), EG_COLOR_MAKE(0x90, 0xCA, 0xF9), EG_COLOR_MAKE(0xBB, 0xDE, 0xFB), EG_COLOR_MAKE(0xE3, 0xF2, 0xFD)},
  {EG_COLOR_MAKE(0x29, 0xB6, 0xF6), EG_COLOR_MAKE(0x4F, 0xC3, 0xF7), EG_COLOR_MAKE(0x81, 0xD4, 0xFA), EG_COLOR_MAKE(0xB3, 0xE5, 0xFC), EG_COLOR_MAKE(0xE1, 0xF5, 0xFE)},
  {EG_COLOR_MAKE(0x26, 0xC6, 0xDA), EG_COLOR_MAKE(0x4D, 0xD0, 0xE1), EG_COLOR_MAKE(0x80, 0xDE, 0xEA), EG_COLOR_MAKE(0xB2, 0xEB, 0xF2), EG_COLOR_MAKE(0xE0, 0xF7, 0xFA)},
  {EG_COLOR_MAKE(0x26, 0xA6, 0x9A), EG_COLOR_MAKE(0x4D, 0xB6, 0xAC), EG_COLOR_MAKE(0x80, 0xCB, 0xC4), EG_COLOR_MAKE(0xB2, 0xDF, 0xDB), EG_COLOR_MAKE(0xE0, 0xF2, 0xF1)},
  {EG_COLOR_MAKE(0x66, 0xBB, 0x6A), EG_COLOR_MAKE(0x81, 0xC7, 0x84), EG_COLOR_MAKE(0xA5, 0xD6, 0xA7), EG_COLOR_MAKE(0xC8, 0xE6, 0xC9), EG_COLOR_MAKE(0xE8, 0xF5, 0xE9)},
  {EG_COLOR_MAKE(0x9C, 0xCC, 0x65), EG_COLOR_MAKE(0xAE, 0xD5, 0x81), EG_COLOR_MAKE(0xC5, 0xE1, 0xA5), EG_COLOR_MAKE(0xDC, 0xED, 0xC8), EG_COLOR_MAKE(0xF1, 0xF8, 0xE9)},
  {EG_COLOR_MAKE(0xD4, 0xE1, 0x57), EG_COLOR_MAKE(0xDC, 0xE7, 0x75), EG_COLOR_MAKE(0xE6, 0xEE, 0x9C), EG_COLOR_MAKE(0xF0, 0xF4, 0xC3), EG_COLOR_MAKE(0xF9, 0xFB, 0xE7)},
  {EG_COLOR_MAKE(0xFF, 0xEE, 0x58), EG_COLOR_MAKE(0xFF, 0xF1, 0x76), EG_COLOR_MAKE(0xFF, 0xF5, 0x9D), EG_COLOR_MAKE(0xFF, 0xF9, 0xC4), EG_COLOR_MAKE(0xFF, 0xFD, 0xE7)},
  {EG_COLOR_MAKE(0xFF, 0xCA, 0x28), EG_COLOR_MAKE(0xFF, 0xD5, 0x4F), EG_COLOR_MAKE(0xFF, 0xE0, 0x82), EG_COLOR_MAKE(0xFF, 0xEC, 0xB3), EG_COLOR_MAKE(0xFF, 0xF8, 0xE1)},
  {EG_COLOR_MAKE(0xFF, 0xA7, 0x26), EG_COLOR_MAKE(0xFF, 0xB7, 0x4D), EG_COLOR_MAKE(0xFF, 0xCC, 0x80), EG_COLOR_MAKE(0xFF, 0xE0, 0xB2), EG_COLOR_MAKE(0xFF, 0xF3, 0xE0)},
  {EG_COLOR_MAKE(0xFF, 0x70, 0x43), EG_COLOR_MAKE(0xFF, 0x8A, 0x65), EG_COLOR_MAKE(0xFF, 0xAB, 0x91), EG_COLOR_MAKE(0xFF, 0xCC, 0xBC), EG_COLOR_MAKE(0xFB, 0xE9, 0xE7)},
  {EG_COLOR_MAKE(0x8D, 0x6E, 0x63), EG_COLOR_MAKE(0xA1, 0x88, 0x7F), EG_COLOR_MAKE(0xBC, 0xAA, 0xA4), EG_COLOR_MAKE(0xD7, 0xCC, 0xC8), EG_COLOR_MAKE(0xEF, 0xEB, 0xE9)},
  {EG_COLOR_MAKE(0x78, 0x90, 0x9C), EG_COLOR_MAKE(0x90, 0xA4, 0xAE), EG_COLOR_MAKE(0xB0, 0xBE, 0xC5), EG_COLOR_MAKE(0xCF, 0xD8, 0xDC), EG_COLOR_MAKE(0xEC, 0xEF, 0xF1)},
  {EG_COLOR_MAKE(0xBD, 0xBD, 0xBD), EG_COLOR_MAKE(0xE0, 0xE0, 0xE0), EG_COLOR_MAKE(0xEE, 0xEE, 0xEE), EG_COLOR_MAKE(0xF5, 0xF5, 0xF5), EG_COLOR_MAKE(0xFA, 0xFA, 0xFA)},
};

constexpr EG_Color_t DarkPalette[][4] = {
  {EG_COLOR_MAKE(0xE5, 0x39, 0x35), EG_COLOR_MAKE(0xD3, 0x2F, 0x2F), EG_COLOR_MAKE(0xC6, 0x28, 0x28), EG_COLOR_MAKE(0xB7, 0x1C, 0x1C)},
  {EG_COLOR_MAKE(0xD8, 0x1B, 0x60), EG_COLOR_MAKE(0xC2, 0x18, 0x5B), EG_COLOR_MAKE(0xAD, 0x14, 0x57), EG_COLOR_MAKE(0x88, 0x0E, 0x4F)},
  {EG_COLOR_MAKE(0x8E, 0x24, 0xAA), EG_COLOR_MAKE(0x7B, 0x1F, 0xA2), EG_COLOR_MAKE(0x6A, 0x1B, 0x9A), EG_COLOR_MAKE(0x4A, 0x14, 0x8C)},
  {EG_COLOR_MAKE(0x5E, 0x35, 0xB1), EG_COLOR_MAKE(0x51, 0x2D, 0xA8), EG_COLOR_MAKE(0x45, 0x27, 0xA0), EG_COLOR_MAKE(0x31, 0x1B, 0x92)},
  {EG_COLOR_MAKE(0x39, 0x49, 0xAB), EG_COLOR_MAKE(0x30, 0x3F, 0x9F), EG_COLOR_MAKE(0x28, 0x35, 0x93), EG_COLOR_MAKE(0x1A, 0x23, 0x7E)},
  {EG_COLOR_MAKE(0x1E, 0x88, 0xE5), EG_COLOR_MAKE(0x19, 0x76, 0xD2), EG_COLOR_MAKE(0x15, 0x65, 0xC0), EG_COLOR_MAKE(0x0D, 0x47, 0xA1)},
  {EG_COLOR_MAKE(0x03, 0x9B, 0xE5), EG_COLOR_MAKE(0x02, 0x88, 0xD1), EG_COLOR_MAKE(0x02, 0x77, 0xBD), EG_COLOR_MAKE(0x01, 0x57, 0x9B)},
  {EG_COLOR_MAKE(0x00, 0xAC, 0xC1), EG_COLOR_MAKE(0x00, 0x97, 0xA7), EG_COLOR_MAKE(0x00, 0x83, 0x8F), EG_COLOR_MAKE(0x00, 0x60, 0x64)},
  {EG_COLOR_MAKE(0x00, 0x89, 0x7B), EG_COLOR_MAKE(0x00, 0x79, 0x6B), EG_COLOR_MAKE(0x00, 0x69, 0x5C), EG_COLOR_MAKE(0x00, 0x4D, 0x40)},
  {EG_COLOR_MAKE(0x43, 0xA0, 0x47), EG_COLOR_MAKE(0x38, 0x8E, 0x3C), EG_COLOR_MAKE(0x2E, 0x7D, 0x32), EG_COLOR_MAKE(0x1B, 0x5E, 0x20)},
  {EG_COLOR_MAKE(0x7C, 0xB3, 0x42), EG_COLOR_MAKE(0x68, 0x9F, 0x38), EG_COLOR_MAKE(0x55, 0x8B, 0x2F), EG_COLOR_MAKE(0x33, 0x69, 0x1E)},
  {EG_COLOR_MAKE(0xC0, 0xCA, 0x33), EG_COLOR_MAKE(0xAF, 0xB4, 0x2B), EG_COLOR_MAKE(0x9E, 0x9D, 0x24), EG_COLOR_MAKE(0x82, 0x77, 0x17)},
  {EG_COLOR_MAKE(0xFD, 0xD8, 0x35), EG_COLOR_MAKE(0xFB, 0xC0, 0x2D), EG_COLOR_MAKE(0xF9, 0xA8, 0x25), EG_COLOR_MAKE(0xF5, 0x7F, 0x17)},
  {EG_COLOR_MAKE(0xFF, 0xB3, 0x00), EG_COLOR_MAKE(0xFF, 0xA0, 0x00), EG_COLOR_MAKE(0xFF, 0x8F, 0x00), EG_COLOR_MAKE(0xFF, 0x6F, 0x00)},
  {EG_COLOR_MAKE(0xFB, 0x8C, 0x00), EG_COLOR_MAKE(0xF5, 0x7C, 0x00), EG_COLOR_MAKE(0xEF, 0x6C, 0x00), EG_COLOR_MAKE(0xE6, 0x51, 0x00)},
  {EG_COLOR_MAKE(0xF4, 0x51, 0x1E), EG_COLOR_MAKE(0xE6, 0x4A, 0x19), EG_COLOR_MAKE(0xD8, 0x43, 0x15), EG_COLOR_MAKE(0xBF, 0x36, 0x0C)},
  {EG_COLOR_MAKE(0x6D, 0x4C, 0x41), EG_COLOR_MAKE(0x5D, 0x40, 0x37), EG_COLOR_MAKE(0x4E, 0x34, 0x2E), EG_COLOR_MAKE(0x3E, 0x27, 0x23)},
  {EG_COLOR_MAKE(0x54, 0x6E, 0x7A), EG_COLOR_MAKE(0x45, 0x5A, 0x64), EG_COLOR_MAKE(0x37, 0x47, 0x4F), EG_COLOR_MAKE(0x26, 0x32, 0x38)},
  {EG_COLOR_MAKE(0x75, 0x75, 0x75), EG_COLOR_MAKE(0x61, 0x61, 0x61), EG_COLOR_MAKE(0x42, 0x42, 0x42), EG_COLOR_MAKE(0x21, 0x21, 0x21)},
};

/////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EG_ColorFill(EG_Color_t *pBuffer, EG_Color_t Color, uint32_t PixelCount)
{
#if EG_COLOR_DEPTH == 16
	uintptr_t buf_int = (uintptr_t)pBuffer;
	if(buf_int & 0x3) {
		*pBuffer = Color;
		pBuffer++;
		PixelCount--;
	}
	uint32_t c32 = (uint32_t)Color.full + ((uint32_t)Color.full << 16);
	uint32_t *buf32 = (uint32_t *)pBuffer;
	while(PixelCount > 16) {
		*buf32 = c32;
		buf32++;
		*buf32 = c32;
		buf32++;
		*buf32 = c32;
		buf32++;
		*buf32 = c32;
		buf32++;

		*buf32 = c32;
		buf32++;
		*buf32 = c32;
		buf32++;
		*buf32 = c32;
		buf32++;
		*buf32 = c32;
		buf32++;

		PixelCount -= 16;
	}
	pBuffer = (EG_Color_t *)buf32;
	while(PixelCount) {
		*pBuffer = Color;
		pBuffer++;
		PixelCount--;
	}
#else
	while(PixelCount > 16) {
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;

		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;

		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;

		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;
		*pBuffer = Color;
		pBuffer++;

		PixelCount -= 16;
	}
	while(PixelCount) {
		*pBuffer = Color;
		pBuffer++;
		PixelCount--;
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_LightenColor(EG_Color_t Color, EG_OPA_t Level)
{
	return EG_ColorMix(EG_ColorWhite(), Color, Level);
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_DarkenColor(EG_Color_t Color, EG_OPA_t Level)
{
	return EG_ColorMix(EG_ColorBlack(), Color, Level);
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_ChangeColorBrightness(EG_Color_t Color, EG_OPA_t Level)
{
	// It's better to convert the Color to HSL, change L and convert back to RGB.
	if(Level == EG_OPA_50) return Color;
	else if(Level < EG_OPA_50) return EG_DarkenColor(Color, (EG_OPA_50 - Level) * 2);
	return EG_LightenColor(Color, (Level - EG_OPA_50) * 2);
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_HSVToRGB(uint16_t Hue, uint8_t Sat, uint8_t Val)
{
uint8_t r, g, b;
uint8_t Region, remainder, p, q, t;

	Hue = (uint32_t)((uint32_t)Hue * 255) / 360;
	Sat = (uint16_t)((uint16_t)Sat * 255) / 100;
	Val = (uint16_t)((uint16_t)Val * 255) / 100;
	if(Sat == 0) return EG_MixColor(Val, Val, Val);
	Region = Hue / 43;
	remainder = (Hue - (Region * 43)) * 6;
	p = (Val * (255 - Sat)) >> 8;
	q = (Val * (255 - ((Sat * remainder) >> 8))) >> 8;
	t = (Val * (255 - ((Sat * (255 - remainder)) >> 8))) >> 8;
	switch(Region) {
		case 0:
			r = Val;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = Val;
			b = p;
			break;
		case 2:
			r = p;
			g = Val;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = Val;
			break;
		case 4:
			r = t;
			g = p;
			b = Val;
			break;
		default:
			r = Val;
			g = p;
			b = q;
			break;
	}
	EG_Color_t result = EG_MixColor(r, g, b);
	return result;
}

/////////////////////////////////////////////////////////////////////////////

EG_ColorHSV_t EG_RGBToHSV(uint8_t r8, uint8_t g8, uint8_t b8)
{
	uint16_t r = ((uint32_t)r8 << 10) / 255;
	uint16_t g = ((uint32_t)g8 << 10) / 255;
	uint16_t b = ((uint32_t)b8 << 10) / 255;

	uint16_t rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
	uint16_t rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

	EG_ColorHSV_t HSV;

	// https://en.wikipedia.org/wiki/HSL_and_HSV#Lightness
	HSV.Val = (100 * rgbMax) >> 10;

	int32_t delta = rgbMax - rgbMin;
	if(delta < 3) {
		HSV.Hue = 0;
		HSV.Sat = 0;
		return HSV;
	}

	// https://en.wikipedia.org/wiki/HSL_and_HSV#Saturation
	HSV.Sat = 100 * delta / rgbMax;
	if(HSV.Sat < 3) {
		HSV.Hue = 0;
		return HSV;
	}

	// https://en.wikipedia.org/wiki/HSL_and_HSV#Hue_and_chroma
	int32_t Hue;
	if(rgbMax == r) Hue = (((g - b) << 10) / delta) + (g < b ? (6 << 10) : 0);  // between yellow & magenta
	else if(rgbMax == g) Hue = (((b - r) << 10) / delta) + (2 << 10);  // between cyan & yellow
	else if(rgbMax == b) Hue = (((r - g) << 10) / delta) + (4 << 10);  // between magenta & cyan
	else Hue = 0;
	Hue *= 60;
	Hue >>= 10;
	if(Hue < 0) Hue += 360;
	HSV.Hue = Hue;
	return HSV;
}

/////////////////////////////////////////////////////////////////////////////

EG_ColorHSV_t EG_ColorToHSV(EG_Color_t Color)
{
	EG_Color32_t color32;
	color32.full = EG_ColorTo32(Color);
	return EG_RGBToHSV(color32.ch.red, color32.ch.green, color32.ch.blue);
}

/////////////////////////////////////////////////////////////////////////////

uint32_t EG_ColorToInt(EG_Color_t c)
{
    uint8_t *pTemp = (uint8_t *)&c;
    return pTemp[0] + (pTemp[1] << 8) + (pTemp[2] << 16);
}

/////////////////////////////////////////////////////////////////////////////

bool EG_ColorEqual(EG_Color_t c1, EG_Color_t c2)
{
    return EG_ColorToInt(c1) == EG_ColorToInt(c2);
}

/////////////////////////////////////////////////////////////////////////////

bool EG_ColorEqual32(EG_Color_t c1, EG_Color_t c2)
{
    return *((uint32_t *)&c1) == *((uint32_t *)&c2);
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_MainPalette(EG_Palette_e PaletteIndex)
{
	if(PaletteIndex >= EG_PALETTE_LAST) {
		EG_LOG_WARN("Invalid palette: %d", PaletteIndex);
		return EG_ColorBlack();
	}
	return MainPalette[PaletteIndex];
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_LightPalette(EG_Palette_e PaletteIndex, uint8_t Level)
{
	if(PaletteIndex >= EG_PALETTE_LAST) {
		EG_LOG_WARN("Invalid palette: %d", PaletteIndex);
		return EG_ColorBlack();
	}
	if(Level == 0 || Level > 5) {
		EG_LOG_WARN("Invalid level: %d. Must be 1..5", Level);
		return EG_ColorBlack();
	}
	Level--;
	return LightPalette[PaletteIndex][Level];
}

/////////////////////////////////////////////////////////////////////////////

EG_Color_t EG_DarkPalette(EG_Palette_e PaletteIndex, uint8_t Level)
{
	if(PaletteIndex >= EG_PALETTE_LAST) {
		EG_LOG_WARN("Invalid palette: %d", PaletteIndex);
		return EG_ColorBlack();
	}
	if(Level == 0 || Level > 4) {
		EG_LOG_WARN("Invalid level: %d. Must be 1..4", Level);
		return EG_ColorBlack();
	}
	Level--;
	return DarkPalette[PaletteIndex][Level];
}
