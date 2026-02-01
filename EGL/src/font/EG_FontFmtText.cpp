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

#include "font/EG_Font.h"
#include "font/EG_FontFmtText.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Types.h"
#include "misc/lv_gc.h"
#include "misc/EG_Log.h"
#include "misc/EG_Utilities.h"
#include "misc/EG_Memory.h"

typedef enum {
	RLE_STATE_SINGLE = 0,
	RLE_STATE_REPEATE,
	RLE_STATE_COUNTER,
} rle_state_t;

static uint32_t GetGlyphDiscriptorID(const EG_Font_t *pFont, uint32_t letter);
static int8_t GetKerningValue(const EG_Font_t *pFont, uint32_t gid_left, uint32_t gid_right);
static int32_t UnicodeListCompare(const void *ref, const void *element);
static int32_t KernPair8Compare(const void *ref, const void *element);
static int32_t KernPair16Compare(const void *ref, const void *element);

#if EG_USE_FONT_COMPRESSED
static void Decompress(const uint8_t *in, uint8_t *out, EG_Coord_t Width, EG_Coord_t Height, uint8_t BitsPerPixel, bool prefilter);
static inline void DecompressLine(uint8_t *out, EG_Coord_t Width);
static inline uint8_t GetBits(const uint8_t *in, uint32_t bit_pos, uint8_t len);
static inline void WriteBits(uint8_t *out, uint32_t bit_pos, uint8_t val, uint8_t len);
static inline void InitialiseRLE(const uint8_t *in, uint8_t BitsPerPixel);
static inline uint8_t NextRLE(void);
#endif /*EG_USE_FONT_COMPRESSED*/

#if EG_USE_FONT_COMPRESSED
static uint32_t rle_rdp;
static const uint8_t *rle_in;
static uint8_t rle_bpp;
static uint8_t rle_prev_v;
static uint8_t rle_cnt;
static rle_state_t rle_state;
#endif /*EG_USE_FONT_COMPRESSED*/

/**
 * Used as `GetGlyphBitmapCB` callback in LittelvGL's native font format if the font is uncompressed.
 * @param pFont pointer to font
 * @param UnicodeChar a unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t* EG_FontGetBitmapFmtText(const EG_Font_t *pFont, uint32_t UnicodeChar)
{
	if(UnicodeChar == '\t') UnicodeChar = ' ';
	EG_FontFmtTextProps_t *fdsc = (EG_FontFmtTextProps_t *)pFont->pProperties;
	uint32_t gid = GetGlyphDiscriptorID(pFont, UnicodeChar);
	if(!gid) return NULL;
	const EG_FontFmtTextGlyphProps_t *gdsc = &fdsc->pGlyphProps[gid];
	if(fdsc->BitmapFormat == EG_FONT_FMT_TXT_PLAIN) {
		return &fdsc->GlyphBitmap[gdsc->BitmapIndex];
	}
	/*Handle compressed bitmap*/
	else {
#if EG_USE_FONT_COMPRESSED
		static size_t LastBufferSize = 0;
		if(EG_GC_ROOT(_lv_font_decompr_buf) == NULL) LastBufferSize = 0;

		uint32_t gsize = gdsc->BoxWidth * gdsc->BoxHeight;
		if(gsize == 0) return NULL;

		uint32_t buf_size = gsize;
		/*Compute memory size needed to hold decompressed glyph, rounding up*/
		switch(fdsc->BitsPerPixel) {
			case 1:
				buf_size = (gsize + 7) >> 3;
				break;
			case 2:
				buf_size = (gsize + 3) >> 2;
				break;
			case 3:
				buf_size = (gsize + 1) >> 1;
				break;
			case 4:
				buf_size = (gsize + 1) >> 1;
				break;
		}

		if(LastBufferSize < buf_size) {
			uint8_t *tmp = (uint8_t*)EG_ReallocMem(EG_GC_ROOT(_lv_font_decompr_buf), buf_size);
			EG_ASSERT_MALLOC(tmp);
			if(tmp == NULL) return NULL;
			EG_GC_ROOT(_lv_font_decompr_buf) = tmp;
			LastBufferSize = buf_size;
		}

		bool prefilter = fdsc->BitmapFormat == EG_FONT_FMT_TXT_COMPRESSED ? true : false;
		Decompress(&fdsc->GlyphBitmap[gdsc->BitmapIndex], EG_GC_ROOT(_lv_font_decompr_buf), gdsc->BoxWidth, gdsc->BoxHeight,
							 (uint8_t)fdsc->BitsPerPixel, prefilter);
		return EG_GC_ROOT(_lv_font_decompr_buf);
#else /*!EG_USE_FONT_COMPRESSED*/
		EG_LOG_WARN("Compressed fonts is used but EG_USE_FONT_COMPRESSED is not enabled in EG_Config.Height");
		return NULL;
#endif
	}

	/*If not returned earlier then the letter is not found in this font*/
	return NULL;
}

/**
 * Used as `GetGlyphPropsCB` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter a UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
bool EG_FontGetGlyphPropsFmtText(const EG_Font_t *pFont, EG_FontGlyphProps_t *dsc_out, uint32_t UnicodeChar,
																	 uint32_t unicode_letter_next)
{
	/*It fixes a strange compiler optimization issue: https://github.com/lvgl/lvgl/issues/4370*/
	bool is_tab = UnicodeChar == '\t';
	if(is_tab) {
		UnicodeChar = ' ';
	}
	EG_FontFmtTextProps_t *fdsc = (EG_FontFmtTextProps_t *)pFont->pProperties;
	uint32_t gid = GetGlyphDiscriptorID(pFont, UnicodeChar);
	if(!gid) return false;

	int8_t kvalue = 0;
	if(fdsc->pKernProps) {
		uint32_t gid_next = GetGlyphDiscriptorID(pFont, unicode_letter_next);
		if(gid_next) {
			kvalue = GetKerningValue(pFont, gid, gid_next);
		}
	}

	/*Put together a glyph dsc*/
	const EG_FontFmtTextGlyphProps_t *gdsc = &fdsc->pGlyphProps[gid];

	int32_t kv = ((int32_t)((int32_t)kvalue * fdsc->KernScale) >> 4);

	uint32_t AdvWidth = gdsc->AdvWidth;
	if(is_tab) AdvWidth *= 2;

	AdvWidth += kv;
	AdvWidth = (AdvWidth + (1 << 3)) >> 4;

	dsc_out->AdvWidth = AdvWidth;
	dsc_out->BoxHeight = gdsc->BoxHeight;
	dsc_out->BoxWidth = gdsc->BoxWidth;
	dsc_out->OffsetX = gdsc->OffsetX;
	dsc_out->OffsetY = gdsc->OffsetY;
	dsc_out->BitsPerPixel = (uint8_t)fdsc->BitsPerPixel;
	dsc_out->IsPlaceholder = false;

	if(is_tab) dsc_out->BoxWidth = dsc_out->BoxWidth * 2;

	return true;
}

/**
 * Free the allocated memories.
 */
void EG_FontCleanUpFmtText(void)
{
#if EG_USE_FONT_COMPRESSED
	if(EG_GC_ROOT(_lv_font_decompr_buf)) {
		EG_FreeMem(EG_GC_ROOT(_lv_font_decompr_buf));
		EG_GC_ROOT(_lv_font_decompr_buf) = NULL;
	}
#endif
}

static uint32_t GetGlyphDiscriptorID(const EG_Font_t *pFont, uint32_t letter)
{
	if(letter == '\0') return 0;

	EG_FontFmtTextProps_t *fdsc = (EG_FontFmtTextProps_t *)pFont->pProperties;

	/*Check the pCache first*/
	if(fdsc->pCache && letter == fdsc->pCache->last_letter) return fdsc->pCache->last_glyph_id;

	uint16_t i;
	for(i = 0; i < fdsc->CmapNumber; i++) {
		/*Relative code point*/
		uint32_t rcp = letter - fdsc->pCmaps[i].RangeStart;
		if(rcp > fdsc->pCmaps[i].RangeLength) continue;
		uint32_t glyph_id = 0;
		if(fdsc->pCmaps[i].Type == EG_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
			glyph_id = fdsc->pCmaps[i].GlyphIDStart + rcp;
		}
		else if(fdsc->pCmaps[i].Type == EG_FONT_FMT_TXT_CMAP_FORMAT0_FULL) {
			const uint8_t *gid_ofs_8 = (uint8_t*)fdsc->pCmaps[i].pGlyphIDOffsetList;
			glyph_id = fdsc->pCmaps[i].GlyphIDStart + gid_ofs_8[rcp];
		}
		else if(fdsc->pCmaps[i].Type == EG_FONT_FMT_TXT_CMAP_SPARSE_TINY) {
			uint16_t key = rcp;
			uint16_t *p = (uint16_t*)_lv_utils_bsearch(&key, fdsc->pCmaps[i].UnicodeList, fdsc->pCmaps[i].ListLength, sizeof(fdsc->pCmaps[i].UnicodeList[0]), UnicodeListCompare);

			if(p) {
				eg_uintptr_t ofs = p - fdsc->pCmaps[i].UnicodeList;
				glyph_id = fdsc->pCmaps[i].GlyphIDStart + ofs;
			}
		}
		else if(fdsc->pCmaps[i].Type == EG_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
			uint16_t key = rcp;
			uint16_t *p = (uint16_t*)_lv_utils_bsearch(&key, fdsc->pCmaps[i].UnicodeList, fdsc->pCmaps[i].ListLength, sizeof(fdsc->pCmaps[i].UnicodeList[0]), UnicodeListCompare);

			if(p) {
				eg_uintptr_t ofs = p - fdsc->pCmaps[i].UnicodeList;
				const uint16_t *gid_ofs_16 = (uint16_t*)fdsc->pCmaps[i].pGlyphIDOffsetList;
				glyph_id = fdsc->pCmaps[i].GlyphIDStart + gid_ofs_16[ofs];
			}
		}

		/*Update the pCache*/
		if(fdsc->pCache) {
			fdsc->pCache->last_letter = letter;
			fdsc->pCache->last_glyph_id = glyph_id;
		}
		return glyph_id;
	}

	if(fdsc->pCache) {
		fdsc->pCache->last_letter = letter;
		fdsc->pCache->last_glyph_id = 0;
	}
	return 0;
}

static int8_t GetKerningValue(const EG_Font_t *pFont, uint32_t gid_left, uint32_t gid_right)
{
	EG_FontFmtTextProps_t *fdsc = (EG_FontFmtTextProps_t *)pFont->pProperties;

	int8_t value = 0;

	if(fdsc->KernClasses == 0) {
		/*Kern pairs*/
		const EG_FontFmtKernPair_t *kdsc = (EG_FontFmtKernPair_t*)fdsc->pKernProps;
		if(kdsc->glyph_ids_size == 0) {
			/*Use binary search to find the kern value.
             *The pairs are ordered left_id first, then right_id secondly.*/
			const uint16_t *g_ids = (uint16_t*)kdsc->glyph_ids;
			uint16_t g_id_both = (gid_right << 8) + gid_left; /*Create one number from the ids*/
			uint16_t *kid_p = (uint16_t*)_lv_utils_bsearch(&g_id_both, g_ids, kdsc->pair_cnt, 2, KernPair8Compare);

			/*If the `g_id_both` were found get its index from the pointer*/
			if(kid_p) {
				eg_uintptr_t ofs = kid_p - g_ids;
				value = kdsc->values[ofs];
			}
		}
		else if(kdsc->glyph_ids_size == 1) {
			/*Use binary search to find the kern value.
             *The pairs are ordered left_id first, then right_id secondly.*/
			const uint32_t *g_ids = (uint32_t*)kdsc->glyph_ids;
			uint32_t g_id_both = (gid_right << 16) + gid_left; /*Create one number from the ids*/
			uint32_t *kid_p = (uint32_t*)_lv_utils_bsearch(&g_id_both, g_ids, kdsc->pair_cnt, 4, KernPair16Compare);

			/*If the `g_id_both` were found get its index from the pointer*/
			if(kid_p) {
				eg_uintptr_t ofs = kid_p - g_ids;
				value = kdsc->values[ofs];
			}
		}
		else {
			/*Invalid value*/
		}
	}
	else {
		/*Kern classes*/
		const EG_FontFmtKernClasses_t *kdsc = (EG_FontFmtKernClasses_t*)fdsc->pKernProps;
		uint8_t left_class = kdsc->left_class_mapping[gid_left];
		uint8_t right_class = kdsc->right_class_mapping[gid_right];

		/*If class = 0, kerning not exist for that glyph
         *else got the value form `class_pair_values` 2D array*/
		if(left_class > 0 && right_class > 0) {
			value = kdsc->class_pair_values[(left_class - 1) * kdsc->right_class_cnt + (right_class - 1)];
		}
	}
	return value;
}

static int32_t KernPair8Compare(const void *ref, const void *element)
{
	const uint8_t *ref8_p = (uint8_t*)ref;
	const uint8_t *element8_p = (uint8_t*)element;

	/*If the MSB is different it will matter. If not return the diff. of the LSB*/
	if(ref8_p[0] != element8_p[0])
		return (int32_t)ref8_p[0] - element8_p[0];
	else
		return (int32_t)ref8_p[1] - element8_p[1];
}

static int32_t KernPair16Compare(const void *ref, const void *element)
{
const uint16_t *ref16_p = (uint16_t*)ref;
const uint16_t *element16_p = (uint16_t*)element;

	/*If the MSB is different it will matter. If not return the diff. of the LSB*/
	if(ref16_p[0] != element16_p[0])
		return (int32_t)ref16_p[0] - element16_p[0];
	else
		return (int32_t)ref16_p[1] - element16_p[1];
}

#if EG_USE_FONT_COMPRESSED
/**
 * The compress a glyph's bitmap
 * @param in the compressed bitmap
 * @param out buffer to store the result
 * @param px_num number of pixels in the glyph (width * height)
 * @param BitsPerPixel bit per pixel (BitsPerPixel = 3 will be converted to BitsPerPixel = 4)
 * @param prefilter true: the lines are XORed
 */
static void Decompress(const uint8_t *in, uint8_t *out, EG_Coord_t Width, EG_Coord_t Height, uint8_t BitsPerPixel, bool prefilter)
{
	uint32_t wrp = 0;
	uint8_t wr_size = BitsPerPixel;
	if(BitsPerPixel == 3) wr_size = 4;

	InitialiseRLE(in, BitsPerPixel);

	uint8_t *line_buf1 = (uint8_t*)EG_GetBufferMem(Width);

	uint8_t *line_buf2 = NULL;

	if(prefilter) {
		line_buf2 = (uint8_t*)EG_GetBufferMem(Width);
	}

	DecompressLine(line_buf1, Width);

	EG_Coord_t y;
	EG_Coord_t x;

	for(x = 0; x < Width; x++) {
		WriteBits(out, wrp, line_buf1[x], BitsPerPixel);
		wrp += wr_size;
	}

	for(y = 1; y < Height; y++) {
		if(prefilter) {
			DecompressLine(line_buf2, Width);

			for(x = 0; x < Width; x++) {
				line_buf1[x] = line_buf2[x] ^ line_buf1[x];
				WriteBits(out, wrp, line_buf1[x], BitsPerPixel);
				wrp += wr_size;
			}
		}
		else {
			DecompressLine(line_buf1, Width);

			for(x = 0; x < Width; x++) {
				WriteBits(out, wrp, line_buf1[x], BitsPerPixel);
				wrp += wr_size;
			}
		}
	}

	EG_ReleaseBufferMem(line_buf1);
	EG_ReleaseBufferMem(line_buf2);
}

/**
 * Decompress one line. Store one pixel per byte
 * @param out output buffer
 * @param Width width of the line in pixel count
 */
static inline void DecompressLine(uint8_t *out, EG_Coord_t Width)
{
	EG_Coord_t i;
	for(i = 0; i < Width; i++) {
		out[i] = NextRLE();
	}
}

/**
 * Read bits from an input buffer. The read can cross byte boundary.
 * @param in the input buffer to read from.
 * @param bit_pos index of the first bit to read.
 * @param len number of bits to read (must be <= 8).
 * @return the read bits
 */
static inline uint8_t GetBits(const uint8_t *in, uint32_t bit_pos, uint8_t len)
{
	uint8_t bit_mask;
	switch(len) {
		case 1:
			bit_mask = 0x1;
			break;
		case 2:
			bit_mask = 0x3;
			break;
		case 3:
			bit_mask = 0x7;
			break;
		case 4:
			bit_mask = 0xF;
			break;
		case 8:
			bit_mask = 0xFF;
			break;
		default:
			bit_mask = (uint16_t)((uint16_t)1 << len) - 1;
	}

	uint32_t byte_pos = bit_pos >> 3;
	bit_pos = bit_pos & 0x7;

	if(bit_pos + len >= 8) {
		uint16_t in16 = (in[byte_pos] << 8) + in[byte_pos + 1];
		return (in16 >> (16 - bit_pos - len)) & bit_mask;
	}
	else {
		return (in[byte_pos] >> (8 - bit_pos - len)) & bit_mask;
	}
}

/**
 * Write `val` data to `bit_pos` position of `out`. The write can NOT cross byte boundary.
 * @param out buffer where to write
 * @param bit_pos bit index to write
 * @param val value to write
 * @param len length of bits to write from `val`. (Counted from the LSB).
 * @note `len == 3` will be converted to `len = 4` and `val` will be upscaled too
 */
static inline void WriteBits(uint8_t *out, uint32_t bit_pos, uint8_t val, uint8_t len)
{
	if(len == 3) {
		len = 4;
		switch(val) {
			case 0:
				val = 0;
				break;
			case 1:
				val = 2;
				break;
			case 2:
				val = 4;
				break;
			case 3:
				val = 6;
				break;
			case 4:
				val = 9;
				break;
			case 5:
				val = 11;
				break;
			case 6:
				val = 13;
				break;
			case 7:
				val = 15;
				break;
		}
	}

	uint16_t byte_pos = bit_pos >> 3;
	bit_pos = bit_pos & 0x7;
	bit_pos = 8 - bit_pos - len;

	uint8_t bit_mask = (uint16_t)((uint16_t)1 << len) - 1;
	out[byte_pos] &= ((~bit_mask) << bit_pos);
	out[byte_pos] |= (val << bit_pos);
}

static inline void InitialiseRLE(const uint8_t *in, uint8_t BitsPerPixel)
{
	rle_in = in;
	rle_bpp = BitsPerPixel;
	rle_state = RLE_STATE_SINGLE;
	rle_rdp = 0;
	rle_prev_v = 0;
	rle_cnt = 0;
}

static inline uint8_t NextRLE(void)
{
	uint8_t v = 0;
	uint8_t ret = 0;

	if(rle_state == RLE_STATE_SINGLE) {
		ret = GetBits(rle_in, rle_rdp, rle_bpp);
		if(rle_rdp != 0 && rle_prev_v == ret) {
			rle_cnt = 0;
			rle_state = RLE_STATE_REPEATE;
		}

		rle_prev_v = ret;
		rle_rdp += rle_bpp;
	}
	else if(rle_state == RLE_STATE_REPEATE) {
		v = GetBits(rle_in, rle_rdp, 1);
		rle_cnt++;
		rle_rdp += 1;
		if(v == 1) {
			ret = rle_prev_v;
			if(rle_cnt == 11) {
				rle_cnt = GetBits(rle_in, rle_rdp, 6);
				rle_rdp += 6;
				if(rle_cnt != 0) {
					rle_state = RLE_STATE_COUNTER;
				}
				else {
					ret = GetBits(rle_in, rle_rdp, rle_bpp);
					rle_prev_v = ret;
					rle_rdp += rle_bpp;
					rle_state = RLE_STATE_SINGLE;
				}
			}
		}
		else {
			ret = GetBits(rle_in, rle_rdp, rle_bpp);
			rle_prev_v = ret;
			rle_rdp += rle_bpp;
			rle_state = RLE_STATE_SINGLE;
		}
	}
	else if(rle_state == RLE_STATE_COUNTER) {
		ret = rle_prev_v;
		rle_cnt--;
		if(rle_cnt == 0) {
			ret = GetBits(rle_in, rle_rdp, rle_bpp);
			rle_prev_v = ret;
			rle_rdp += rle_bpp;
			rle_state = RLE_STATE_SINGLE;
		}
	}

	return ret;
}
#endif 

/** Code Comparator.
 *
 *  Compares the value of both input arguments.
 *
 *  @param[in]  pRef        Pointer to the reference.
 *  @param[in]  pElement    Pointer to the element to compare.
 *
 *  @return Result of comparison.
 *  @retval < 0   Reference is less than element.
 *  @retval = 0   Reference is equal to element.
 *  @retval > 0   Reference is greater than element.
 *
 */
static int32_t UnicodeListCompare(const void *ref, const void *element)
{
	return ((int32_t)(*(uint16_t *)ref)) - ((int32_t)(*(uint16_t *)element));
}
