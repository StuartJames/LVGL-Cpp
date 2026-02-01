/*
 *                EGL 2025-2026 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or   
 *  modify it under the terms of the GNU General Public License as  
 *  published by the Free Software Foundation; either Version 2 of  
 *  the License, or (at your option) any later Version.             
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
#include <stdbool.h>

#include "EGL.h"
#include "misc/EG_FileSystem.h"
#include "font/EG_FontLoader.h"

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	EGFileSystem *pFile;
	int8_t BitPosition;
	uint8_t ByteValue;
} bit_iterator_t;

typedef struct font_header_bin {
	uint32_t Version;
	uint16_t TablesCount;
	uint16_t FontSize;
	uint16_t Ascent;
	int16_t Descent;
	uint16_t TypoAscent;
	int16_t TypoDescent;
	uint16_t TypoLineGap;
	int16_t MinY;
	int16_t MaxY;
	uint16_t DefaultAdvanceWidth;
	uint16_t KerningScale;
	uint8_t LocationFormatIndex;
	uint8_t GlyphIDFormat;
	uint8_t AdvanceWidthFormat;
	uint8_t BitsPerPixel;
	uint8_t XYBits;
	uint8_t WidthHeightBits;
	uint8_t AdvanceWidthBits;
	uint8_t CompressionID;
	uint8_t SubpixelsMode;
	uint8_t Padding;
	int16_t UnderlinePosition;
	uint16_t UnderlineThickness;
} font_header_bin_t;

typedef struct cmap_table_bin {
	uint32_t DataOffset;
	uint32_t RangeStart;
	uint16_t RangeLength;
	uint16_t GlyphIDStart;
	uint16_t DataEntriesCount;
	uint8_t FormatType;
	uint8_t Padding;
} cmap_table_bin_t;

/////////////////////////////////////////////////////////////////////////////////////////

static bit_iterator_t InitaliseBitIterator(EGFileSystem *pFile);
static bool EG_LoadFont(EGFileSystem *pFile, EG_Font_t *pFont);
int32_t LoadKern(EGFileSystem *pFile, EG_FontFmtTextProps_t *pFontProps, uint8_t format, uint32_t start);

static int ReadBitsSigned(bit_iterator_t *it, int n_bits, EG_FSResult_e *res);
static unsigned int ReadBits(bit_iterator_t *it, int n_bits, EG_FSResult_e *res);

/////////////////////////////////////////////////////////////////////////////////////////

EG_Font_t *EG_FontLoad(const char *font_name)
{
	EGFileSystem File;
	EG_FSResult_e res = File.Open(font_name, EG_FS_MODE_RD);
	if(res != EG_FS_RES_OK)
		return NULL;

	EG_Font_t *pFont = (EG_Font_t*)EG_AllocMem(sizeof(EG_Font_t));
	if(pFont) {
		memset(pFont, 0, sizeof(EG_Font_t));
		if(!EG_LoadFont(&File, pFont)) {
			EG_LOG_WARN("Error loading font file: %s\n", font_name);
			/* When `EG_LoadFont` fails it can leak some pointers. All non-null pointers can be assumed as allocated and
       * `EG_FontFree` should free them correctly. */
			EG_FontFree(pFont);
			pFont = nullptr;
		}
	}
	File.Close();
	return pFont;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EG_FontFree(EG_Font_t *pFont)
{
	if(pFont != nullptr) {
		EG_FontFmtTextProps_t *pFontProps = (EG_FontFmtTextProps_t *)pFont->pProperties;
		if(pFontProps != nullptr) {
			if(pFontProps->KernClasses == 0) {
				EG_FontFmtKernPair_t *pKernProps =	(EG_FontFmtKernPair_t *)pFontProps->pKernProps;
				if(NULL != pKernProps) {
					if(pKernProps->glyph_ids)	EG_FreeMem((void *)pKernProps->glyph_ids);
					if(pKernProps->values) EG_FreeMem((void *)pKernProps->values);
					EG_FreeMem((void *)pKernProps);
				}
			}
			else {
				EG_FontFmtKernClasses_t *pKernProps = (EG_FontFmtKernClasses_t *)pFontProps->pKernProps;
				if(NULL != pKernProps) {
					if(pKernProps->class_pair_values)	EG_FreeMem((void *)pKernProps->class_pair_values);
  				if(pKernProps->left_class_mapping) EG_FreeMem((void *)pKernProps->left_class_mapping);
					if(pKernProps->right_class_mapping)	EG_FreeMem((void *)pKernProps->right_class_mapping);
					EG_FreeMem((void *)pKernProps);
				}
			}
			EG_FontFmtTextCmap_t *pCmaps =	(EG_FontFmtTextCmap_t *)pFontProps->pCmaps;
			if(pCmaps != nullptr) {
				for(int i = 0; i < pFontProps->CmapNumber; ++i) {
					if(NULL != pCmaps[i].pGlyphIDOffsetList) EG_FreeMem((void *)pCmaps[i].pGlyphIDOffsetList);
					if(NULL != pCmaps[i].UnicodeList)	EG_FreeMem((void *)pCmaps[i].UnicodeList);
				}
				EG_FreeMem(pCmaps);
			}
			if(NULL != pFontProps->GlyphBitmap) EG_FreeMem((void *)pFontProps->GlyphBitmap);
			if(NULL != pFontProps->pGlyphProps) EG_FreeMem((void *)pFontProps->pGlyphProps);
			EG_FreeMem(pFontProps);
		}
		EG_FreeMem(pFont);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static bit_iterator_t InitaliseBitIterator(EGFileSystem *pFile)
{
	bit_iterator_t it;
	it.pFile = pFile;
	it.BitPosition = -1;
	it.ByteValue = 0;
	return it;
}

/////////////////////////////////////////////////////////////////////////////////////////

static unsigned int ReadBits(bit_iterator_t *it, int n_bits, EG_FSResult_e *res)
{
	unsigned int value = 0;
	while(n_bits--) {
		it->ByteValue = it->ByteValue << 1;
		it->BitPosition--;
		if(it->BitPosition < 0) {
			it->BitPosition = 7;
			*res = it->pFile->Read(&(it->ByteValue), 1, NULL);
			if(*res != EG_FS_RES_OK) {
				return 0;
			}
		}
		int8_t bit = (it->ByteValue & 0x80) ? 1 : 0;
		value |= (bit << n_bits);
	}
	*res = EG_FS_RES_OK;
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ReadBitsSigned(bit_iterator_t *it, int n_bits, EG_FSResult_e *res)
{
	unsigned int value = ReadBits(it, n_bits, res);
	if(value & (1 << (n_bits - 1))) {
		value |= ~0u << n_bits;
	}
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ReadLabel(EGFileSystem *pFile, int start, const char *label)
{
uint32_t Length;
char Buffer[4];

	pFile->Seek(start, EG_FS_SEEK_SET);
	if(pFile->Read(&Length, 4, NULL) != EG_FS_RES_OK || pFile->Read(Buffer, 4, NULL) != EG_FS_RES_OK || memcmp(label, Buffer, 4) != 0) {
		EG_LOG_WARN("Error reading '%s' label.", label);
		return -1;
	}
	return Length;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool LoadCmapsTables(EGFileSystem *pFile, EG_FontFmtTextProps_t *pFontProps,	uint32_t cmaps_start, cmap_table_bin_t *pCmapTable)
{
	if(pFile->Read(pCmapTable, pFontProps->CmapNumber * sizeof(cmap_table_bin_t), NULL) != EG_FS_RES_OK) {
		return false;
	}
	for(unsigned int i = 0; i < pFontProps->CmapNumber; ++i) {
		EG_FSResult_e res = pFile->Seek(cmaps_start + pCmapTable[i].DataOffset, EG_FS_SEEK_SET);
		if(res != EG_FS_RES_OK) {
			return false;
		}
		EG_FontFmtTextCmap_t *pCmap = (EG_FontFmtTextCmap_t *)&(pFontProps->pCmaps[i]);
		pCmap->RangeStart = pCmapTable[i].RangeStart;
		pCmap->RangeLength = pCmapTable[i].RangeLength;
		pCmap->GlyphIDStart = pCmapTable[i].GlyphIDStart;
		pCmap->Type = pCmapTable[i].FormatType;
		switch(pCmapTable[i].FormatType) {
			case EG_FONT_FMT_TXT_CMAP_FORMAT0_FULL: {
				uint8_t ids_size = sizeof(uint8_t) * pCmapTable[i].DataEntriesCount;
				uint8_t *pGlyphIDOffsetList = (uint8_t*)EG_AllocMem(ids_size);
				pCmap->pGlyphIDOffsetList = pGlyphIDOffsetList;
				if(pFile->Read(pGlyphIDOffsetList, ids_size, NULL) != EG_FS_RES_OK) return false;
				pCmap->ListLength = pCmap->RangeLength;
				break;
			}
			case EG_FONT_FMT_TXT_CMAP_FORMAT0_TINY:
				break;
			case EG_FONT_FMT_TXT_CMAP_SPARSE_FULL:
			case EG_FONT_FMT_TXT_CMAP_SPARSE_TINY: {
				uint32_t list_size = sizeof(uint16_t) * pCmapTable[i].DataEntriesCount;
				uint16_t *UnicodeList = (uint16_t *)EG_AllocMem(list_size);
				pCmap->UnicodeList = UnicodeList;
				pCmap->ListLength = pCmapTable[i].DataEntriesCount;
				if(pFile->Read(UnicodeList, list_size, NULL) != EG_FS_RES_OK) return false;
				if(pCmapTable[i].FormatType == EG_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
					uint16_t *buf = (uint16_t*)EG_AllocMem(sizeof(uint16_t) * pCmap->ListLength);
					pCmap->pGlyphIDOffsetList = buf;
					if(pFile->Read(buf, sizeof(uint16_t) * pCmap->ListLength, NULL) != EG_FS_RES_OK) {
						return false;
					}
				}
				break;
			}
			default:
				EG_LOG_WARN("Unknown pCmaps format Type %d.", pCmapTable[i].FormatType);
				return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int32_t LoadCmaps(EGFileSystem *pFile, EG_FontFmtTextProps_t *pFontProps, uint32_t cmaps_start)
{
	int32_t cmaps_length = ReadLabel(pFile, cmaps_start, "cmap");
	if(cmaps_length < 0) return -1;
	uint32_t cmaps_subtables_count;
	if(pFile->Read(&cmaps_subtables_count, sizeof(uint32_t), NULL) != EG_FS_RES_OK) return -1;
	EG_FontFmtTextCmap_t *pCmaps =	(EG_FontFmtTextCmap_t*)EG_AllocMem(cmaps_subtables_count * sizeof(EG_FontFmtTextCmap_t));
	memset(pCmaps, 0, cmaps_subtables_count * sizeof(EG_FontFmtTextCmap_t));
	pFontProps->pCmaps = pCmaps;
	pFontProps->CmapNumber = cmaps_subtables_count;
	cmap_table_bin_t *cmaps_tables = (cmap_table_bin_t*)EG_AllocMem(sizeof(cmap_table_bin_t) * pFontProps->CmapNumber);
	bool success = LoadCmapsTables(pFile, pFontProps, cmaps_start, cmaps_tables);
	EG_FreeMem(cmaps_tables);
	return success ? cmaps_length : -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int32_t LoadGlyph(EGFileSystem *pFile, EG_FontFmtTextProps_t *pFontProps, uint32_t start, uint32_t *glyph_offset, uint32_t loca_count, font_header_bin_t *header)
{
	int32_t glyph_length = ReadLabel(pFile, start, "glyf");
	if(glyph_length < 0) return -1;
	EG_FontFmtTextGlyphProps_t *pGlyphProps = (EG_FontFmtTextGlyphProps_t *)EG_AllocMem(loca_count * sizeof(EG_FontFmtTextGlyphProps_t));
	memset(pGlyphProps, 0, loca_count * sizeof(EG_FontFmtTextGlyphProps_t));
	pFontProps->pGlyphProps = pGlyphProps;
	int cur_bmp_size = 0;
	for(unsigned int i = 0; i < loca_count; ++i) {
		EG_FontFmtTextGlyphProps_t *gdsc = &pGlyphProps[i];
		EG_FSResult_e res = pFile->Seek(start + glyph_offset[i], EG_FS_SEEK_SET);
		if(res != EG_FS_RES_OK) return -1;
		bit_iterator_t bit_it = InitaliseBitIterator(pFile);
		if(header->AdvanceWidthBits == 0) {
			gdsc->AdvWidth = header->DefaultAdvanceWidth;
		}
		else {
			gdsc->AdvWidth = ReadBits(&bit_it, header->AdvanceWidthBits, &res);
			if(res != EG_FS_RES_OK) return -1;
		}
		if(header->AdvanceWidthFormat == 0) gdsc->AdvWidth *= 16;
		gdsc->OffsetX = ReadBitsSigned(&bit_it, header->XYBits, &res);
		if(res != EG_FS_RES_OK) return -1;
		gdsc->OffsetY = ReadBitsSigned(&bit_it, header->XYBits, &res);
		if(res != EG_FS_RES_OK) return -1;
		gdsc->BoxWidth = ReadBits(&bit_it, header->WidthHeightBits, &res);
		if(res != EG_FS_RES_OK) return -1;
		gdsc->BoxHeight = ReadBits(&bit_it, header->WidthHeightBits, &res);
		if(res != EG_FS_RES_OK) return -1;
		int nbits = header->AdvanceWidthBits + 2 * header->XYBits + 2 * header->WidthHeightBits;
		int next_offset = (i < loca_count - 1) ? glyph_offset[i + 1] : (uint32_t)glyph_length;
		int bmp_size = next_offset - glyph_offset[i] - nbits / 8;
		if(i == 0) {
			gdsc->AdvWidth = 0;
			gdsc->BoxWidth = 0;
			gdsc->BoxHeight = 0;
			gdsc->OffsetX = 0;
			gdsc->OffsetY = 0;
		}
		gdsc->BitmapIndex = cur_bmp_size;
		if(gdsc->BoxWidth * gdsc->BoxHeight != 0) {
			cur_bmp_size += bmp_size;
		}
	}

	uint8_t *glyph_bmp = (uint8_t *)EG_AllocMem(sizeof(uint8_t) * cur_bmp_size);

	pFontProps->GlyphBitmap = glyph_bmp;

	cur_bmp_size = 0;

	for(unsigned int i = 1; i < loca_count; ++i) {
		EG_FSResult_e res = pFile->Seek(start + glyph_offset[i], EG_FS_SEEK_SET);
		if(res != EG_FS_RES_OK) {
			return -1;
		}
		bit_iterator_t bit_it = InitaliseBitIterator(pFile);

		int nbits = header->AdvanceWidthBits + 2 * header->XYBits + 2 * header->WidthHeightBits;

		ReadBits(&bit_it, nbits, &res);
		if(res != EG_FS_RES_OK) {
			return -1;
		}

		if(pGlyphProps[i].BoxWidth * pGlyphProps[i].BoxHeight == 0) {
			continue;
		}

		int next_offset = (i < loca_count - 1) ? glyph_offset[i + 1] : (uint32_t)glyph_length;
		int bmp_size = next_offset - glyph_offset[i] - nbits / 8;

		if(nbits % 8 == 0) { /*Fast path*/
			if(pFile->Read(&glyph_bmp[cur_bmp_size], bmp_size, NULL) != EG_FS_RES_OK) {
				return -1;
			}
		}
		else {
			for(int k = 0; k < bmp_size - 1; ++k) {
				glyph_bmp[cur_bmp_size + k] = ReadBits(&bit_it, 8, &res);
				if(res != EG_FS_RES_OK) {
					return -1;
				}
			}
			glyph_bmp[cur_bmp_size + bmp_size - 1] = ReadBits(&bit_it, 8 - nbits % 8, &res);
			if(res != EG_FS_RES_OK) {
				return -1;
			}

			/*The last fragment should be on the MSB but ReadBits() will place it to the LSB*/
			glyph_bmp[cur_bmp_size + bmp_size - 1] = glyph_bmp[cur_bmp_size + bmp_size - 1] << (nbits % 8);
		}

		cur_bmp_size += bmp_size;
	}
	return glyph_length;
}

/////////////////////////////////////////////////////////////////////////////////////////

/*
 * Loads a `EG_Font_t` from a binary file, given a `EGFileSystem`.
 *
 * Memory allocations on `EG_LoadFont` should be immediately zeroed and
 * the pointer should be set on the `EG_Font_t` data before any possible return.
 *
 * When something fails, it returns `false` and the memory on the `EG_Font_t`
 * still needs to be freed using `EG_FontFree`.
 *
 * `EG_FontFree` will assume that all non-null pointers are allocated and
 * should be freed.
 */
static bool EG_LoadFont(EGFileSystem *pFile, EG_Font_t *pFont)
{
	EG_FontFmtTextProps_t *pFontProps = (EG_FontFmtTextProps_t *)EG_AllocMem(sizeof(EG_FontFmtTextProps_t));
	memset(pFontProps, 0, sizeof(EG_FontFmtTextProps_t));
	pFont->pProperties = pFontProps;
	/*header*/
	int32_t header_length = ReadLabel(pFile, 0, "head");
	if(header_length < 0) return false;
	font_header_bin_t font_header;
	if(pFile->Read(&font_header, sizeof(font_header_bin_t), NULL) != EG_FS_RES_OK) return false;
	pFont->BaseLine = -font_header.Descent;
	pFont->LineHeight = font_header.Ascent - font_header.Descent;
	pFont->GetGlyphPropsCB = EG_FontGetGlyphPropsFmtText;
	pFont->GetGlyphBitmapCB = EG_FontGetBitmapFmtText;
	pFont->SubPixel = font_header.SubpixelsMode;
	pFont->UnderlinePosition = font_header.UnderlinePosition;
	pFont->UnderlineThickness = font_header.UnderlineThickness;
	pFontProps->BitsPerPixel = font_header.BitsPerPixel;
	pFontProps->KernScale = font_header.KerningScale;
	pFontProps->BitmapFormat = font_header.CompressionID;
	/*pCmaps*/
	uint32_t cmaps_start = header_length;
	int32_t cmaps_length = LoadCmaps(pFile, pFontProps, cmaps_start);
	if(cmaps_length < 0) return false;
	/*loca*/
	uint32_t loca_start = cmaps_start + cmaps_length;
	int32_t loca_length = ReadLabel(pFile, loca_start, "loca");
	if(loca_length < 0) return false;
	uint32_t loca_count;
	if(pFile->Read(&loca_count, sizeof(uint32_t), NULL) != EG_FS_RES_OK) return false;
	bool failed = false;
	uint32_t *glyph_offset = (uint32_t*)EG_AllocMem(sizeof(uint32_t) * (loca_count + 1));
	if(font_header.LocationFormatIndex == 0) {
		for(unsigned int i = 0; i < loca_count; ++i) {
			uint16_t offset;
			if(pFile->Read(&offset, sizeof(uint16_t), NULL) != EG_FS_RES_OK) {
				failed = true;
				break;
			}
			glyph_offset[i] = offset;
		}
	}
	else if(font_header.LocationFormatIndex == 1) {
		if(pFile->Read(glyph_offset, loca_count * sizeof(uint32_t), NULL) != EG_FS_RES_OK) failed = true;
	}
	else {
		EG_LOG_WARN("Unknown LocationFormatIndex: %d.", font_header.LocationFormatIndex);
		failed = true;
	}
	if(failed){
    EG_FreeMem(glyph_offset);
		return false;
	}
	uint32_t glyph_start = loca_start + loca_length;	/*glyph*/
	int32_t glyph_length = LoadGlyph(pFile, pFontProps, glyph_start, glyph_offset, loca_count, &font_header);
	EG_FreeMem(glyph_offset);
	if(glyph_length < 0) return false;
	if(font_header.TablesCount < 4) {
		pFontProps->pKernProps = NULL;
		pFontProps->KernClasses = 0;
		pFontProps->KernScale = 0;
		return true;
	}
	uint32_t kern_start = glyph_start + glyph_length;
	int32_t kern_length = LoadKern(pFile, pFontProps, font_header.GlyphIDFormat, kern_start);
	return kern_length >= 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int32_t LoadKern(EGFileSystem *pFile, EG_FontFmtTextProps_t *pFontProps, uint8_t format, uint32_t start)
{
	int32_t kern_length = ReadLabel(pFile, start, "kern");
	if(kern_length < 0) return -1;
	uint8_t kern_format_type;
	int32_t Padding;
	if(pFile->Read(&kern_format_type, sizeof(uint8_t), NULL) != EG_FS_RES_OK ||
		 pFile->Read(&Padding, 3 * sizeof(uint8_t), NULL) != EG_FS_RES_OK) {
		return -1;
	}
	if(0 == kern_format_type) { /*sorted pairs*/
		EG_FontFmtKernPair_t *kern_pair = (EG_FontFmtKernPair_t*)EG_AllocMem(sizeof(EG_FontFmtKernPair_t));
		memset(kern_pair, 0, sizeof(EG_FontFmtKernPair_t));
		pFontProps->pKernProps = kern_pair;
		pFontProps->KernClasses = 0;
		uint32_t glyph_entries;
		if(pFile->Read(&glyph_entries, sizeof(uint32_t), NULL) != EG_FS_RES_OK) return -1;
		int ids_size;
		if(format == 0) ids_size = sizeof(int8_t) * 2 * glyph_entries;
		else ids_size = sizeof(int16_t) * 2 * glyph_entries;
		uint8_t *glyph_ids = (uint8_t*)EG_AllocMem(ids_size);
		int8_t *values = (int8_t*)EG_AllocMem(glyph_entries);
		kern_pair->glyph_ids_size = format;
		kern_pair->pair_cnt = glyph_entries;
		kern_pair->glyph_ids = glyph_ids;
		kern_pair->values = values;
		if(pFile->Read(glyph_ids, ids_size, NULL) != EG_FS_RES_OK) return -1;
		if(pFile->Read(values, glyph_entries, NULL) != EG_FS_RES_OK) return -1;
	}
	else if(3 == kern_format_type) { /*array M*N of classes*/
		EG_FontFmtKernClasses_t *KernClasses = (EG_FontFmtKernClasses_t*)EG_AllocMem(sizeof(EG_FontFmtKernClasses_t));
		memset(KernClasses, 0, sizeof(EG_FontFmtKernClasses_t));
		pFontProps->pKernProps = KernClasses;
		pFontProps->KernClasses = 1;
		uint16_t kern_class_mapping_length;
		uint8_t kern_table_rows;
		uint8_t kern_table_cols;
		if(pFile->Read(&kern_class_mapping_length, sizeof(uint16_t), NULL) != EG_FS_RES_OK ||
			 pFile->Read(&kern_table_rows, sizeof(uint8_t), NULL) != EG_FS_RES_OK ||
			 pFile->Read(&kern_table_cols, sizeof(uint8_t), NULL) != EG_FS_RES_OK) {
			return -1;
		}
		int kern_values_length = sizeof(int8_t) * kern_table_rows * kern_table_cols;
		uint8_t *kern_left = (uint8_t*)EG_AllocMem(kern_class_mapping_length);
		uint8_t *kern_right = (uint8_t*)EG_AllocMem(kern_class_mapping_length);
		int8_t *kern_values = (int8_t*)EG_AllocMem(kern_values_length);
		KernClasses->left_class_mapping = kern_left;
		KernClasses->right_class_mapping = kern_right;
		KernClasses->left_class_cnt = kern_table_rows;
		KernClasses->right_class_cnt = kern_table_cols;
		KernClasses->class_pair_values = kern_values;
		if(pFile->Read(kern_left, kern_class_mapping_length, NULL) != EG_FS_RES_OK ||
			 pFile->Read(kern_right, kern_class_mapping_length, NULL) != EG_FS_RES_OK ||
			 pFile->Read(kern_values, kern_values_length, NULL) != EG_FS_RES_OK) {
			return -1;
		}
	}
	else {
		EG_LOG_WARN("Unknown kern_format_type: %d", kern_format_type);
		return -1;
	}
	return kern_length;
}
