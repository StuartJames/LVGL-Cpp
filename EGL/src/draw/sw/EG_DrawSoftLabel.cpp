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
  

#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"     // lv_draw_sw_blend
#include "hal/EG_HALDisplay.h"
#include "misc/EG_Math.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Point.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Style.h"
#include "font/EG_Font.h"
#include "core/EG_Refresh.h"

//////////////////////////////////////////////////////////////////////////////////////

const uint8_t _lv_bpp1_opa_table[2] = {0, 255};          // Opacity mapping with BitsPerPixel = 1 (Just for compatibility) 
const uint8_t _lv_bpp2_opa_table[4] = {0, 85, 170, 255}; // Opacity mapping with BitsPerPixel = 2 

const uint8_t _lv_bpp3_opa_table[8] = {0, 36, 73, 109, // Opacity mapping with BitsPerPixel = 3 
																			 146, 182, 219, 255};

const uint8_t _lv_bpp4_opa_table[16] = {0, 17, 34, 51, // Opacity mapping with BitsPerPixel = 4 
																				68, 85, 102, 119,
																				136, 153, 170, 187,
																				204, 221, 238, 255};

const uint8_t _lv_bpp8_opa_table[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
																				 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
																				 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
																				 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
																				 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
																				 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
																				 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
																				 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
																				 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
																				 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
																				 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
																				 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
																				 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
																				 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
																				 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
																				 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGSoftContext::DrawCharacter(const EGDrawLabel *pDrawLabel, const EGPoint *pPosition, uint32_t Character)
{
EG_FontGlyphProps_t Glyph;

	bool g_ret = EG_FontGetGlyphProps(pDrawLabel->m_pFont, &Glyph, Character, '\0');
	if(g_ret == false) {
		// Add warning if the pDrawLabel is not found but do not print warning for non printable ASCII chars 
		if(Character >= 0x20 &&
			 Character != 0xf8ff && // EG_SYMBOL_DUMMY 
			 Character != 0x200c) { // ZERO WIDTH NON-JOINER 
			EG_LOG_WARN("DrawCharacter: glyph discriptor not found for U+%" EG_PRIX32, Character);

#if EG_USE_FONT_PLACEHOLDER
			//  draw placeholder  
			EGRect GlyphArea;
			EGDrawRect GlyphRect;
			EG_Coord_t begin_x = pPosition->m_X + Glyph.OffsetX;
			EG_Coord_t begin_y = pPosition->m_Y + Glyph.OffsetY;
			GlyphArea.Set(begin_x, begin_y, begin_x + Glyph.BoxWidth, begin_y + Glyph.BoxHeight);
			GlyphRect.m_BackgroundOPA = EG_OPA_MIN;
			GlyphRect.m_OutlineOPA = EG_OPA_MIN;
			GlyphRect.m_ShadowOPA = EG_OPA_MIN;
			GlyphRect.m_BackImageOPA = EG_OPA_MIN;
			GlyphRect.m_BorderColor = pDrawLabel->m_Color;
			GlyphRect.m_BorderWidth = 1;
			GlyphRect.Draw(pDrawLabel->m_pContext, &GlyphArea);
#endif
		}
		return;
	}
	// Don't draw anything if the character is empty. E.Glyph. space 
	if((Glyph.BoxHeight == 0) || (Glyph.BoxWidth == 0)) return;
	EGPoint gpos;
	gpos.m_X = pPosition->m_X + Glyph.OffsetX;
	gpos.m_Y = pPosition->m_Y + (pDrawLabel->m_pFont->LineHeight - pDrawLabel->m_pFont->BaseLine) - Glyph.BoxHeight - Glyph.OffsetY;
	// If the Character is completely out of mask don't draw it 
	if(gpos.m_X + Glyph.BoxWidth < pDrawLabel->m_pContext->m_pClipRect->GetX1() ||
		 gpos.m_X > pDrawLabel->m_pContext->m_pClipRect->GetX2() ||
		 gpos.m_Y + Glyph.BoxHeight < pDrawLabel->m_pContext->m_pClipRect->GetY1() ||
		 gpos.m_Y > pDrawLabel->m_pContext->m_pClipRect->GetY2()) {
		return;
	}
	const uint8_t *pMap = EG_FontGetGlyphBitmap(Glyph.ResolvedFont, Character);
	if(pMap == NULL) {
		EG_LOG_WARN("Label: Character bitmap not found");
		return;
	}
	if(Glyph.ResolvedFont->SubPixel) {
#if EG_DRAW_COMPLEX && EG_USE_FONT_SUBPX
		draw_letter_subpx(pDrawLabel, pDrawLabel, &gpos, &Glyph, pMap);
#else
		EG_LOG_WARN("Can't draw sub-pixel rendered Character because EG_USE_FONT_SUBPX == 0 in EG_Config.h");
#endif
	}
	else DrawNormal(pDrawLabel, &gpos, &Glyph, pMap);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::DrawNormal(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, EG_FontGlyphProps_t *pGlyph, const uint8_t *pMap)
{
const uint8_t *bpp_opa_table_p;
uint32_t bitmask_init;
uint32_t bitmask;
uint32_t BitsPerPixel = pGlyph->BitsPerPixel;
EG_OPA_t opa = pDrawLabel->m_OPA;
uint32_t shades;

	if(BitsPerPixel == 3) BitsPerPixel = 4;
#if EG_USE_IMGFONT
	if(BitsPerPixel == EG_IMGFONT_BPP) {  //is imgfont
		const EGRect FillRect(pPos->m_X, pPos->m_Y, pPos->m_X + pGlyph->BoxWidth - 1, pPos->m_Y + pGlyph->BoxHeight - 1);
		EGDrawImage DrawImage;
		DrawImage.m_Angle = 0;
		DrawImage.m_Zoom = EG_SCALE_NONE;
		DrawImage.m_OPA = pDrawLabel->m_OPA;
		DrawImage.m_BlendMode = pDrawLabel->m_BlendMode;
		DrawImage.Draw(pDrawLabel->m_pContext, &FillRect, pMap);
		return;
	}
#endif
	switch(BitsPerPixel) {
		case 1:
			bpp_opa_table_p = _lv_bpp1_opa_table;
			bitmask_init = 0x80;
			shades = 2;
			break;
		case 2:
			bpp_opa_table_p = _lv_bpp2_opa_table;
			bitmask_init = 0xC0;
			shades = 4;
			break;
		case 4:
			bpp_opa_table_p = _lv_bpp4_opa_table;
			bitmask_init = 0xF0;
			shades = 16;
			break;
		case 8:
			bpp_opa_table_p = _lv_bpp8_opa_table;
			bitmask_init = 0xFF;
			shades = 256;
			break; // No opa table, pixel value will be used directly 
		default:
			EG_LOG_WARN("DrawNormal: invalid BitsPerPixel");
			return; // Invalid BitsPerPixel. Can't render the Character 
	}
	static EG_OPA_t opa_table[256];
	static EG_OPA_t prev_opa = EG_OPA_TRANSP;
	static uint32_t prev_bpp = 0;
	if(opa < EG_OPA_MAX) {
		if(prev_opa != opa || prev_bpp != BitsPerPixel) {
			uint32_t i;
			for(i = 0; i < shades; i++) {
				opa_table[i] = bpp_opa_table_p[i] == EG_OPA_COVER ? opa : ((bpp_opa_table_p[i] * opa) >> 8);
			}
		}
		bpp_opa_table_p = opa_table;
		prev_opa = opa;
		prev_bpp = BitsPerPixel;
	}
	int32_t Column, Row;
	int32_t BoxWidth = pGlyph->BoxWidth;
	int32_t BoxHeight = pGlyph->BoxHeight;
	int32_t width_bit = BoxWidth * BitsPerPixel; // Letter width in bits 
	// Calculate the col/row start/end on the map 
	int32_t StartColumn = pPos->m_X >= pDrawLabel->m_pContext->m_pClipRect->GetX1() ? 0 : pDrawLabel->m_pContext->m_pClipRect->GetX1() - pPos->m_X;
	int32_t EndColumn = pPos->m_X + BoxWidth <= pDrawLabel->m_pContext->m_pClipRect->GetX2() ? BoxWidth : pDrawLabel->m_pContext->m_pClipRect->GetX2() - pPos->m_X + 1;
	int32_t StartRow = pPos->m_Y >= pDrawLabel->m_pContext->m_pClipRect->GetY1() ? 0 : pDrawLabel->m_pContext->m_pClipRect->GetY1() - pPos->m_Y;
	int32_t EndRow = pPos->m_Y + BoxHeight <= pDrawLabel->m_pContext->m_pClipRect->GetY2() ? BoxHeight : pDrawLabel->m_pContext->m_pClipRect->GetY2() - pPos->m_Y + 1;
	uint32_t bit_ofs = (StartRow * width_bit) + (StartColumn * BitsPerPixel);	// Move on the map too 
	pMap += bit_ofs >> 3;
	uint8_t CharIndex;
	uint32_t col_bit;
	col_bit = bit_ofs & 0x7; // "& 0x7" equals to "% 8" just faster 
	EGSoftBlend BlendObj((EGSoftContext*)pDrawLabel->m_pContext);
	BlendObj.m_Color = pDrawLabel->m_Color;
	BlendObj.m_OPA = pDrawLabel->m_OPA;
	BlendObj.m_BlendMode = pDrawLabel->m_BlendMode;
	EG_Coord_t HorizontalRes = GetRefreshingDisplay()->GetHorizontalRes();
	uint32_t MaskBufferSize = BoxWidth * BoxHeight > HorizontalRes ? HorizontalRes : BoxWidth * BoxHeight;
	EG_OPA_t *MaskBuffer = (EG_OPA_t *)EG_GetBufferMem(MaskBufferSize);
	BlendObj.m_pMaskBuffer = MaskBuffer;
	int32_t MaskIndex = 0;
	EGRect FillRect(StartColumn + pPos->m_X, StartRow + pPos->m_Y, EndColumn + pPos->m_X - 1, StartRow + pPos->m_Y);
#if EG_DRAW_COMPLEX
	EG_Coord_t FillWidth = FillRect.GetWidth();
	EGRect MaskRect(FillRect);
	MaskRect.SetY2(MaskRect.GetY1() + EndRow);
	bool MaskAny = HasAnyDrawMask(&MaskRect);
#endif
	BlendObj.m_pRect = &FillRect;
	BlendObj.m_pMaskRect = &FillRect;
	uint32_t col_bit_max = 8 - BitsPerPixel;
	uint32_t col_bit_row_ofs = (BoxWidth + StartColumn - EndColumn) * BitsPerPixel;
//  ESP_LOGI("[DrwLbl]", "Pos: %d,%d - %d,%d.", FillRect.GetX1(), FillRect.GetY1(), FillRect.GetX2(), FillRect.GetY2());
	for(Row = StartRow; Row < EndRow; Row++) {
#if EG_DRAW_COMPLEX
		int32_t mask_p_start = MaskIndex;
#endif
		bitmask = bitmask_init >> col_bit;
		for(Column = StartColumn; Column < EndColumn; Column++) {
			CharIndex = (*pMap & bitmask) >> (col_bit_max - col_bit);			// Load the pixel's opacity into the mask 
			if(CharIndex)	MaskBuffer[MaskIndex] = bpp_opa_table_p[CharIndex];
			else MaskBuffer[MaskIndex] = 0;
			if(col_bit < col_bit_max) {		                   // Go to the next column 
				col_bit += BitsPerPixel;
				bitmask = bitmask >> BitsPerPixel;
			}
			else {
				col_bit = 0;
				bitmask = bitmask_init;
				pMap++;
			}
			MaskIndex++;			                              // Next mask byte 
		}
#if EG_DRAW_COMPLEX
		if(MaskAny) {		                                  // Apply masks if any 
			BlendObj.m_MaskResult = DrawMaskApply(MaskBuffer + mask_p_start, FillRect.GetX1(), FillRect.GetY2(), FillWidth);
			if(BlendObj.m_MaskResult == EG_DRAW_MASK_RES_TRANSP) {
				EG_ZeroMem(MaskBuffer + mask_p_start, FillWidth);
			}
		}
#endif
		if((uint32_t)MaskIndex + (EndColumn - StartColumn) < MaskBufferSize) {
			FillRect.IncY2(1);
		}
		else {
			BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
			BlendObj.DoBlend();
			FillRect.SetY1(FillRect.GetY2() + 1);
			FillRect.SetY2(FillRect.GetY1());
			MaskIndex = 0;
		}
		col_bit += col_bit_row_ofs;
		pMap += (col_bit >> 3);
		col_bit = col_bit & 0x7;
	}
	// Flush the last part 
	if(FillRect.GetY1() != FillRect.GetY2()) {
		FillRect.DecY2(1);   // decrement
		BlendObj.m_MaskResult = EG_DRAW_MASK_RES_CHANGED;
		BlendObj.DoBlend();
		MaskIndex = 0;
	}
	EG_ReleaseBufferMem(MaskBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////

#if EG_DRAW_COMPLEX && EG_USE_FONT_SUBPX
void EGSoftContext::DrawSubpixel(EGDrawLabel *pDrawLabel, const EGPoint *pPos, EG_FontGlyphProps_t *pGlyph, const uint8_t *pMap)
{
	const uint8_t *bpp_opa_table;
	uint32_t bitmask_init;
	uint32_t bitmask;
	uint32_t BitsPerPixel = pGlyph->BitsPerPixel;
	EG_OPA_t opa = pDrawLabel->m_OPA;
	if(BitsPerPixel == 3) BitsPerPixel = 4;

	switch(BitsPerPixel) {
		case 1:
			bpp_opa_table = _lv_bpp1_opa_table;
			bitmask_init = 0x80;
			break;
		case 2:
			bpp_opa_table = _lv_bpp2_opa_table;
			bitmask_init = 0xC0;
			break;
		case 4:
			bpp_opa_table = _lv_bpp4_opa_table;
			bitmask_init = 0xF0;
			break;
		case 8:
			bpp_opa_table = _lv_bpp8_opa_table;
			bitmask_init = 0xFF;
			break; // No opa table, pixel value will be used directly 
		default:
			EG_LOG_WARN("lv_draw_letter: invalid BitsPerPixel not found");
			return; // Invalid BitsPerPixel. Can't render the Character 
	}

	int32_t Column, Row;

	int32_t BoxWidth = pGlyph->BoxWidth;
	int32_t BoxHeight = pGlyph->BoxHeight;
	int32_t width_bit = BoxWidth * BitsPerPixel; // Letter width in bits 

	// Calculate the col/row start/end on the map 
	int32_t StartColumn = pPos->m_X >= pDrawLabel->m_pClipRect->GetX1() ? 0 : (pDrawLabel->m_pClipRect->GetX1() - pPos->m_X) * 3;
	int32_t EndColumn = pPos->m_X + BoxWidth / 3 <= pDrawLabel->m_pClipRect->GetX2() ? BoxWidth : (pDrawLabel->m_pClipRect->GetX2() - pPos->m_X + 1) * 3;
	int32_t StartRow = pPos->m_Y >= pDrawLabel->m_pClipRect->GetY1() ? 0 : pDrawLabel->m_pClipRect->GetY1() - pPos->m_Y;
	int32_t EndRow = pPos->m_Y + BoxHeight <= pDrawLabel->m_pClipRect->GetY2() ? BoxHeight : pDrawLabel->m_pClipRect->GetY2() - pPos->m_Y + 1;

	// Move on the map too 
	int32_t bit_ofs = (StartRow * width_bit) + (StartColumn * BitsPerPixel);
	pMap += bit_ofs >> 3;

	uint8_t CharIndex;
	EG_OPA_t px_opa;
	int32_t col_bit;
	col_bit = bit_ofs & 0x7; // "& 0x7" equals to "% 8" just faster 

	EGRect map_area;
	map_area.GetX1() = StartColumn / 3 + pPos->m_X;
	map_area.GetX2() = EndColumn / 3 + pPos->m_X - 1;
	map_area.GetY1() = StartRow + pPos->m_Y;
	map_area.GetY2() = map_area.GetY1();

	if(map_area.GetX2() <= map_area.GetX1()) return;

	EG_Coord_t HorizontalRes = lv_disp_get_hor_res(_lv_refr_get_disp_refreshing());
	int32_t MaskBufferSize = BoxWidth * BoxHeight > HorizontalRes ? HorizontalRes : pGlyph->BoxWidth * pGlyph->BoxHeight;
	EG_OPA_t *MaskBuffer = EG_GetBufferMem(MaskBufferSize);
	int32_t MaskIndex = 0;

	EG_Color_t *color_buf = EG_GetBufferMem(MaskBufferSize * sizeof(EG_Color_t));

	int32_t dest_buf_stride = EG_Rect_get_width(pDrawLabel->buf_area);
	EG_Color_t *dest_buf_tmp = pDrawLabel->buf;

	// Set a pointer on draw_buf to the first pixel of the Character 
	dest_buf_tmp += ((pPos->m_Y - pDrawLabel->buf_area->GetY1()) * dest_buf_stride) + pPos->m_X - pDrawLabel->buf_area->GetX1();

	// If the Character is partially out of mask the move there on draw_buf 
	dest_buf_tmp += (StartRow * dest_buf_stride) + StartColumn / 3;

	EGRect MaskRect;
	EG_Rect_copy(&MaskRect, &map_area);
	MaskRect.GetY2() = MaskRect.GetY1() + EndRow;
	bool MaskAny = lv_draw_mask_is_any(&map_area);
	uint8_t font_rgb[3];

	EG_Color_t color = pDrawLabel->color;
#if EG_COLOR_16_SWAP == 0
	uint8_t txt_rgb[3] = {color.ch.red, color.ch.green, color.ch.blue};
#else
	uint8_t txt_rgb[3] = {color.ch.red, (color.ch.green_h << 3) + color.ch.green_l, color.ch.blue};
#endif

  EGSoftBlend BlendObj((EGSoftContext*)pDrawLabel->m_pContext);
	BlendObj.blend_area = &map_area;
	BlendObj.MaskRect = &map_area;
	BlendObj.src_buf = color_buf;
	BlendObj.MaskBuffer = MaskBuffer;
	BlendObj.m_OPA = opa;
	BlendObj.blend_mode = pDrawLabel->blend_mode;

	for(Row = StartRow; Row < EndRow; Row++) {
		uint32_t subpx_cnt = 0;
		bitmask = bitmask_init >> col_bit;
		int32_t mask_p_start = MaskIndex;

		for(Column = StartColumn; Column < EndColumn; Column++) {
			// Load the pixel's opacity into the mask 
			CharIndex = (*pMap & bitmask) >> (8 - col_bit - BitsPerPixel);
			if(CharIndex != 0) {
				if(opa >= EG_OPA_MAX) {
					px_opa = BitsPerPixel == 8 ? CharIndex : bpp_opa_table[CharIndex];
				}
				else {
					px_opa = BitsPerPixel == 8 ? (uint32_t)((uint32_t)CharIndex * opa) >> 8 : (uint32_t)((uint32_t)bpp_opa_table[CharIndex] * opa) >> 8;
				}
			}
			else {
				px_opa = 0;
			}

			font_rgb[subpx_cnt] = px_opa;

			subpx_cnt++;
			if(subpx_cnt == 3) {
				subpx_cnt = 0;

				EG_Color_t res_color;
#if EG_COLOR_16_SWAP == 0
				uint8_t bg_rgb[3] = {dest_buf_tmp->ch.red, dest_buf_tmp->ch.green, dest_buf_tmp->ch.blue};
#else
				uint8_t bg_rgb[3] = {dest_buf_tmp->ch.red,
														 (dest_buf_tmp->ch.green_h << 3) + dest_buf_tmp->ch.green_l,
														 dest_buf_tmp->ch.blue};
#endif

#if EG_FONT_SUBPX_BGR
				res_color.ch.red = (uint32_t)((uint16_t)txt_rgb[0] * font_rgb[2] + (bg_rgb[0] * (255 - font_rgb[2]))) >> 8;
				res_color.ch.blue = (uint32_t)((uint16_t)txt_rgb[2] * font_rgb[0] + (bg_rgb[2] * (255 - font_rgb[0]))) >> 8;
#else
				res_color.ch.red = (uint32_t)((uint16_t)txt_rgb[0] * font_rgb[0] + (bg_rgb[0] * (255 - font_rgb[0]))) >> 8;
				res_color.ch.blue = (uint32_t)((uint16_t)txt_rgb[2] * font_rgb[2] + (bg_rgb[2] * (255 - font_rgb[2]))) >> 8;
#endif

#if EG_COLOR_16_SWAP == 0
				res_color.ch.green = (uint32_t)((uint32_t)txt_rgb[1] * font_rgb[1] + (bg_rgb[1] * (255 - font_rgb[1]))) >> 8;
#else
				uint8_t green = (uint32_t)((uint32_t)txt_rgb[1] * font_rgb[1] + (bg_rgb[1] * (255 - font_rgb[1]))) >> 8;
				res_color.ch.green_h = green >> 3;
				res_color.ch.green_l = green & 0x7;
#endif

#if EG_COLOR_DEPTH == 32
				res_color.ch.alpha = 0xff;
#endif

				if(font_rgb[0] == 0 && font_rgb[1] == 0 && font_rgb[2] == 0)
					MaskBuffer[MaskIndex] = EG_OPA_TRANSP;
				else
					MaskBuffer[MaskIndex] = EG_OPA_COVER;
				color_buf[MaskIndex] = res_color;

				// Next mask byte 
				MaskIndex++;
				dest_buf_tmp++;
			}

			// Go to the next column 
			if(col_bit < (int32_t)(8 - BitsPerPixel)) {
				col_bit += BitsPerPixel;
				bitmask = bitmask >> BitsPerPixel;
			}
			else {
				col_bit = 0;
				bitmask = bitmask_init;
				pMap++;
			}
		}

		// Apply masks if any 
		if(MaskAny) {
			BlendObj.mask_res = lv_draw_mask_apply(MaskBuffer + mask_p_start, map_area.GetX1(), map_area.GetY2(),
																							EG_Rect_get_width(&map_area));
			if(BlendObj.mask_res == EG_DRAW_MASK_RES_TRANSP) {
				EG_ZeroMem(MaskBuffer + mask_p_start, EG_Rect_get_width(&map_area));
			}
		}

		if((int32_t)MaskIndex + (EndColumn - StartColumn) < MaskBufferSize) {
			map_area.GetY2()++;
		}
		else {
			BlendObj.mask_res = EG_DRAW_MASK_RES_CHANGED;
      BlendObj.DoBlend();

			map_area.GetY1() = map_area.GetY2() + 1;
			map_area.GetY2() = map_area.GetY1();
			MaskIndex = 0;
		}

		col_bit += ((BoxWidth - EndColumn) + StartColumn) * BitsPerPixel;

		pMap += (col_bit >> 3);
		col_bit = col_bit & 0x7;

		// Next row in draw_buf 
		dest_buf_tmp += dest_buf_stride - (EndColumn - StartColumn) / 3;
	}

	// Flush the last part 
	if(map_area.GetY1() != map_area.GetY2()) {
		map_area.GetY2()--;
		BlendObj.mask_res = EG_DRAW_MASK_RES_CHANGED;
    BlendObj.DoBlend()
	}

	EG_ReleaseBufferMem(MaskBuffer);
	EG_ReleaseBufferMem(color_buf);
}
#endif 
