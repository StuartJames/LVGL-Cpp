/*
 *                EGL 2025-2026 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed pIn the hope that it will be useful,
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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#include "EG_IntrnlConfig.h"

#if EG_USE_GPU_SDL

#include "draw/sdl/EG_SDL_Context.h"

#include "draw/EG_DeviceContext.h"
#include "draw/EG_DrawLabel.h"
#include "core/EG_Refresh.h"

///////////////////////////////////////////////////////////////////////////////////////

extern const uint8_t _EG_BPP1_OPA_Table[2];
extern const uint8_t _EG_BPP2_OPA_Table[4];
extern const uint8_t _EG_BPP4_OPA_Table[16];
extern const uint8_t _EG_BPP8_OPA_Table[256];

static int PalletCount = 0;
static SDL_Palette *EG_SDL_PaletteGrayscale8 = nullptr;

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::UtilsInit()
{
	PalletCount++;
	if(PalletCount > 1) return;
	EG_SDL_PaletteGrayscale8 = AllocPaletteForBPP(_EG_BPP8_OPA_Table, 8);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::UtilsDeinit()
{
	if(PalletCount == 0) return;
	PalletCount--;
	if(PalletCount == 0) {
		SDL_FreePalette(EG_SDL_PaletteGrayscale8);
		EG_SDL_PaletteGrayscale8 = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::RectToSDLRect(const EGRect *pIn, SDL_Rect *pOut)
{
	pOut->x = pIn->GetX1();
	pOut->y = pIn->GetY1();
	pOut->w = pIn->GetX2() - pIn->GetX1() + 1;
	pOut->h = pIn->GetY2() - pIn->GetY1() + 1;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::ColorToSDLColor(const EG_Color_t *pIn, SDL_Color *pOut)
{
#if EG_COLOR_DEPTH == 32
	pOut->a = pIn->ch.alpha;
	pOut->r = pIn->ch.red;
	pOut->g = pIn->ch.green;
	pOut->b = pIn->ch.blue;
#else
	uint32_t Color32 = EG_ColorTo32(*pIn);
	EG_Color32_t *pColor32 = (EG_Color32_t *)Color32;
	pOut->a = pColor32->ch.alpha;
	pOut->r = pColor32->ch.red;
	pOut->g = pColor32->ch.green;
	pOut->b = pColor32->ch.blue;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::ScaleToSDLRect(const EGRect *pIn, SDL_Rect *pOut, EGScale Scale, const EGPoint *pPivot)
{
	if(!Scale.IsScaled()) {
		RectToSDLRect(pIn, pOut);
		return;
	}
	EGRect Rect;
	EGImageBuffer::GetTransformedRect(&Rect, pIn->GetWidth(), pIn->GetHeight(), 0, Scale, pPivot);
	Rect.Move(pIn->GetX1(), pIn->GetY1());
	RectToSDLRect(&Rect, pOut);
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Palette* EGSDLContext::AllocPaletteForBPP(const uint8_t *pMapping, uint8_t BPP)
{
	SDL_assert(BPP >= 1 && BPP <= 8);
	int ColorCount = 1 << BPP;
	SDL_Palette *pResult = SDL_AllocPalette(ColorCount);
	SDL_Color Palette[256];
	for(int i = 0; i < ColorCount; i++) {
		Palette[i].r = Palette[i].g = Palette[i].b = 0xFF;
		Palette[i].a = pMapping ? pMapping[i] : i;
	}
	SDL_SetPaletteColors(pResult, Palette, 0, ColorCount);
	return pResult;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Surface* EGSDLContext::CreateOPASurface(EG_OPA_t * opa, int32_t Width, int32_t Height, int32_t Step)
{
	SDL_Surface *pIndexed = SDL_CreateRGBSurfaceFrom(opa, Width, Height, 8, Step, 0, 0, 0, 0);
	SDL_SetSurfacePalette(pIndexed, EG_SDL_PaletteGrayscale8);
	SDL_Surface *pConverted = SDL_ConvertSurfaceFormat(pIndexed, EG_DRAW_SDL_TEXTURE_FORMAT, 0);
	SDL_FreeSurface(pIndexed);
	return pConverted;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::CreateOPATexture(SDL_Renderer *m_pRenderer, EG_OPA_t *pPixels, int32_t Width, int32_t Height, int32_t Step)
{
	SDL_Surface *pIndexed = CreateOPASurface(pPixels, Width, Height, Step);
	SDL_Texture *pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pIndexed);
	SDL_FreeSurface(pIndexed);
	return pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::SDLTo8BPP(uint8_t *pDest, const uint8_t *pSrce, int Width, int Height, int Step, uint8_t BPP)
{
int SrceLength = Width * Height;
int Index = 0;
int Bits;
uint8_t OPAMask;
const uint8_t *pOPATable;

	switch(BPP) {
		case 1:
			OPAMask = 0x1;
			pOPATable = _EG_BPP1_OPA_Table;
			break;
		case 2:
			OPAMask = 0x4;
			pOPATable = _EG_BPP2_OPA_Table;
			break;
		case 4:
			OPAMask = 0xF;
			pOPATable = _EG_BPP4_OPA_Table;
			break;
		case 8:
			OPAMask = 0xFF;
			pOPATable = _EG_BPP8_OPA_Table;
			break;
		default:
			return;
	}
	/* Does this work well on big endian systems? */
	while(Index < SrceLength) {
		Bits = 8 - BPP;
		uint8_t SrceByte = pSrce[Index * BPP / 8];
		while(Bits >= 0 && Index < SrceLength) {
			uint8_t SrceBits = OPAMask & (SrceByte >> Bits);
			pDest[(Index / Width * Step) + (Index % Width)] = pOPATable[SrceBits];
			Bits -= BPP;
			Index++;
		}
	}
}

#endif
