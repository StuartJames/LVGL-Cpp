/**
 * @file lv_imgfont.c
 *
 */

#include "extra/others/lv_imgfont.h"

#if EG_USE_IMGFONT

#define LV_IMGFONT_PATH_MAX_LEN 64

typedef struct {
	EG_Font_t *font;
	lv_get_imgfont_path_cb_t path_cb;
	char path[LV_IMGFONT_PATH_MAX_LEN];
} imgfont_dsc_t;

static const uint8_t *imgfont_get_glyph_bitmap(const EG_Font_t *font, uint32_t unicode);
static bool imgfont_get_glyph_dsc(const EG_Font_t *font, EG_FontGlyphProps_t *dsc_out,
																	uint32_t unicode, uint32_t unicode_next);


EG_Font_t *lv_imgfont_create(uint16_t height, lv_get_imgfont_path_cb_t path_cb)
{
	EG_ASSERT_MSG(LV_IMGFONT_PATH_MAX_LEN > sizeof(lv_img_dsc_t),
								"LV_IMGFONT_PATH_MAX_LEN must be greater than sizeof(lv_img_dsc_t)");

	size_t size = sizeof(imgfont_dsc_t) + sizeof(EG_Font_t);
	imgfont_dsc_t *dsc = (imgfont_dsc_t *)EG_AllocMem(size);
	if(dsc == NULL) return NULL;
	EG_ZeroMem(dsc, size);

	dsc->font = (EG_Font_t *)(((char *)dsc) + sizeof(imgfont_dsc_t));
	dsc->path_cb = path_cb;

	EG_Font_t *font = dsc->font;
	font->dsc = dsc;
	font->GetGlyphPropsCB = imgfont_get_glyph_dsc;
	font->GetGlyphBitmapCB = imgfont_get_glyph_bitmap;
	font->subpx = EG_FONT_SUBPX_NONE;
	font->LineHeight = height;
	font->BaseLine = 0;
	font->UnderlinePosition = 0;
	font->UnderlineThickness = 0;

	return dsc->font;
}

void lv_imgfont_destroy(EG_Font_t *font)
{
	if(font == NULL) {
		return;
	}

	imgfont_dsc_t *dsc = (imgfont_dsc_t *)font->dsc;
	EG_FreeMem(dsc);
}

static const uint8_t *imgfont_get_glyph_bitmap(const EG_Font_t *font, uint32_t unicode)
{
	EG_UNUSED(unicode);
	EG_ASSERT_NULL(font);
	imgfont_dsc_t *dsc = (imgfont_dsc_t *)font->dsc;
	return (uint8_t *)dsc->path;
}

static bool imgfont_get_glyph_dsc(const EG_Font_t *font, EG_FontGlyphProps_t *dsc_out,
																	uint32_t unicode, uint32_t unicode_next)
{
	EG_ASSERT_NULL(font);

	imgfont_dsc_t *dsc = (imgfont_dsc_t *)font->dsc;
	EG_ASSERT_NULL(dsc);
	if(dsc->path_cb == NULL) return false;

	if(!dsc->path_cb(dsc->font, dsc->path, LV_IMGFONT_PATH_MAX_LEN, unicode, unicode_next)) {
		return false;
	}

	EG_ImageHeader_t header;
	if(lv_img_decoder_get_info(dsc->path, &header) != EG_RES_OK) {
		return false;
	}

	dsc_out->IsPlaceholder = 0;
	dsc_out->AdvWidth = header.w;
	dsc_out->BoxWidth = header.w;
	dsc_out->BoxHeight = header.h;
	dsc_out->BitsPerPixel = LV_IMGFONT_BPP; /* is image identifier */
	dsc_out->OffsetX = 0;
	dsc_out->OffsetY = 0;

	return true;
}

#endif 
