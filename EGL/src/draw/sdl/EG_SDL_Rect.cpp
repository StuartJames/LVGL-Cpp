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
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#include "EG_IntrnlConfig.h"

#if EG_USE_GPU_SDL

#include "draw/EG_DrawRect.h"
#include "draw/EG_DrawImage.h"
#include "draw/EG_DrawLabel.h"
#include "draw/EG_DrawMask.h"
#include "core/EG_Refresh.h"
#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

#define FRAG_SPACING 3


#define SKIP_BORDER(dsc) ((dsc)->m_BorderOPA <= EG_OPA_MIN || (dsc)->m_BorderWidth == 0 || (dsc)->m_BorderSide == EG_BORDER_SIDE_NONE || (dsc)->m_BorderPost)
#define SKIP_SHADOW(dsc) ((dsc)->m_ShadowWidth == 0 || (dsc)->m_ShadowOPA <= EG_OPA_MIN || ((dsc)->m_ShadowWidth == 1 && (dsc)->m_ShadowSpread <= 0 && (dsc)->m_ShadowOffsetX == 0 && (dsc)->m_ShadowOffsetY == 0))
#define SKIP_IMAGE(dsc) ((dsc)->m_pBackImageSource == nullptr || (dsc)->m_BackImageOPA <= EG_OPA_MIN)
#define SKIP_OUTLINE(dsc) ((dsc)->m_OutlineOPA <= EG_OPA_MIN || (dsc)->m_OutlineWidth == 0)

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRect(const EGDrawRect *pDrawObj, const EGRect *pRect)
{
  EGSDLContext *pDC = (EGSDLContext*)pDrawObj->m_pContext;
	const EGRect *pClip = pDC->m_pClipRect;
	EGRect ExtRect = {0, 0, 0, 0};
	if(!SKIP_SHADOW(pDrawObj)) {
		int32_t Ext = (int32_t)(pDrawObj->m_ShadowSpread - pDrawObj->m_ShadowWidth / 2 + 1);
		ExtRect.SetX1(EG_MAX(ExtRect.GetX1(), -pDrawObj->m_ShadowOffsetX + Ext));
		ExtRect.SetX2(EG_MAX(ExtRect.GetX2(), pDrawObj->m_ShadowOffsetX + Ext));
		ExtRect.SetY1(EG_MAX(ExtRect.GetY1(), -pDrawObj->m_ShadowOffsetY + Ext));
		ExtRect.SetY2(EG_MAX(ExtRect.GetY2(), pDrawObj->m_ShadowOffsetY + Ext));
	}
	if(!SKIP_OUTLINE(pDrawObj)) {
		int32_t Ext = (int32_t)(pDrawObj->m_OutlinePadding - 1 + pDrawObj->m_OutlineWidth);
		ExtRect.SetX1(EG_MAX(ExtRect.GetX1(), Ext));
		ExtRect.SetX2(EG_MAX(ExtRect.GetX2(), Ext));
		ExtRect.SetY1(EG_MAX(ExtRect.GetY1(), Ext));
		ExtRect.SetY2(EG_MAX(ExtRect.GetY2(), Ext));
	}
	EGRect TempRect(pRect);
  EGRect TempClip(pClip);
  EGRect ApplyRect;
  EGRect TempRect2;
	bool HasComposite = pDC->CompositeBegin(pRect, pClip, &ExtRect, pDrawObj->m_BlendMode, &TempRect, &TempClip, &ApplyRect);
	pDC->TransformAreasOffset(HasComposite, &ApplyRect, &TempRect, &TempClip);
	bool HasContent = TempRect2.Intersect(&TempRect, &TempClip);
	SDL_Rect ClipRect;
	RectToSDLRect(&TempClip, &ClipRect);
	pDC->DrawRectShadow(&TempRect, &TempClip, pDrawObj);
	// Shadows and outlines will also draw in extended area
	if(HasContent) {
		pDC->DrawRectBackColor(&TempRect, &TempRect2, pDrawObj);
		pDC->DrawRectBackImage(&TempRect, &TempRect2, pDrawObj);
		pDC->DrawRectBorder(&TempRect, &TempRect2, pDrawObj);
	}
	pDC->DrawRectOutline(&TempRect, &TempClip, pDrawObj);
	pDC->CompositeEnd(&ApplyRect, pDrawObj->m_BlendMode);
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::RectBackgroundGetFrag(int32_t Radius, bool *pInCache)
{
	EG_RectBackgroundKey_t Key = CreateRectBackgroundKey(Radius, Radius);
	SDL_Texture *pTexture = TextureCacheGet(&Key, sizeof(Key), nullptr);
	if(pTexture == nullptr) {
		EGRect Rect(0, 0, Radius * 2 - 1, Radius * 2 - 1);
		EGRect RectFrag(0, 0, Radius - 1, Radius - 1);
		MaskRadiusParam_t MaskParam;
    DrawMaskSetRadius(&MaskParam, &Rect, Radius, false);
		int16_t MaskID = DrawMaskAdd(&MaskParam, nullptr);
		pTexture = MaskDumpTexture(&RectFrag, &MaskID, 1);
		SDL_assert(pTexture != nullptr);
		DrawMaskRemoveID(MaskID);
		*pInCache = TextureCachePut(&Key, sizeof(Key), pTexture);
	}
	else *pInCache = true;
	return pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::RectGradGetFrag(const EG_GradDescriptor_t *pGrad, int32_t Width, int32_t Height, int32_t Radius, bool *pInCache)
{
	EG_RectGradFragKey_t Key = CreateRectGradFragKey(pGrad, Width, Height, Radius);
	SDL_Texture *pTexture = TextureCacheGet(&Key, sizeof(Key), nullptr);
	if(pTexture == nullptr) {
		EGRect Rect(0, 0, Radius * 2 + FRAG_SPACING - 1, Radius * 2 + FRAG_SPACING - 1);
		pTexture = SDL_CreateTexture(m_pRenderer, EG_DRAW_SDL_TEXTURE_FORMAT, SDL_TEXTUREACCESS_TARGET, Rect.GetWidth(), Rect.GetHeight());
		SDL_assert(pTexture != nullptr);
		MaskRadiusParam_t MaskParam;
		DrawMaskSetRadius(&MaskParam, &Rect, Radius, false);
		int16_t MaskID = DrawMaskAdd(&MaskParam, nullptr);
		SDL_Texture *pMask = MaskDumpTexture(&Rect, &MaskID, 1);
		SDL_assert(pMask != nullptr);
		SDL_SetTextureBlendMode(pMask, SDL_BLENDMODE_NONE);
		DrawMaskRemoveID(MaskID);
		SDL_Texture *target_backup = SDL_GetRenderTarget(m_pRenderer);
		SDL_SetRenderTarget(m_pRenderer, pTexture);
		SDL_RenderCopy(m_pRenderer, pMask, nullptr, nullptr);
		SDL_DestroyTexture(pMask);
		EGRect BlendRect(0, 0, Width - 1, Height - 1);
		EGRect DrawRect(0, 0, Radius - 1, Radius - 1);
		// Align to top left
		Rect.Align(&DrawRect, EG_ALIGN_TOP_LEFT, 0, 0);
		Rect.Align(&BlendRect, EG_ALIGN_TOP_LEFT, 0, 0);
		DrawRectBackGradSimple(&BlendRect, &DrawRect, pGrad, true);
		// Align to top right
		Rect.Align(&DrawRect, EG_ALIGN_TOP_RIGHT, 0, 0);
		Rect.Align(&BlendRect, EG_ALIGN_TOP_RIGHT, 0, 0);
		DrawRectBackGradSimple(&BlendRect, &DrawRect, pGrad, true);
		// Align to bottom right
		Rect.Align(&DrawRect, EG_ALIGN_BOTTOM_RIGHT, 0, 0);
		Rect.Align(&BlendRect, EG_ALIGN_BOTTOM_RIGHT, 0, 0);
		DrawRectBackGradSimple(&BlendRect, &DrawRect, pGrad, true);
		// Align to bottom left
		Rect.Align(&DrawRect, EG_ALIGN_BOTTOM_LEFT, 0, 0);
		Rect.Align(&BlendRect, EG_ALIGN_BOTTOM_LEFT, 0, 0);
		DrawRectBackGradSimple(&BlendRect, &DrawRect, pGrad, true);
		SDL_SetRenderTarget(m_pRenderer, target_backup);
		*pInCache = TextureCachePut(&Key, sizeof(Key), pTexture);
	}
	else *pInCache = true;
	return pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

SDL_Texture* EGSDLContext::RectGradGetStrip(const EG_GradDescriptor_t *pGrad, bool *pInCache)
{
	EG_RectGradStripKey_t Key = CreateRectGradStripKey(pGrad);
	SDL_Texture *pTexture = TextureCacheGet(&Key, sizeof(Key), nullptr);
	if(pTexture == nullptr) {
		Uint32 amask = 0xFF000000;
		Uint32 rmask = 0x00FF0000;
		Uint32 gmask = 0x0000FF00;
		Uint32 bmask = 0x000000FF;
		EG_Color_t Pixels[256];
		for(int i = 0; i < 256; i++) {
			Pixels[i] = EG_GradientCalculate(pGrad, 256, i);
		}
		int Width = pGrad->dir == EG_GRAD_DIR_VER ? 1 : 256;
		int Height = pGrad->dir == EG_GRAD_DIR_VER ? 256 : 1;
		SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(Pixels, Width, Height, EG_COLOR_DEPTH, Width * EG_COLOR_DEPTH / 8, rmask, gmask, bmask, amask);
		pTexture = SDL_CreateTextureFromSurface(m_pRenderer, surface);
		SDL_assert(pTexture != nullptr);
		SDL_FreeSurface(surface);
		*pInCache = TextureCachePut(&Key, sizeof(Key), pTexture);
	}
	else *pInCache = true;
	return pTexture;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::RectBackgroundFragDrawCorners(SDL_Texture *pFrag, int32_t FragSize, const EGRect *pRect, const EGRect *pClipRect, bool Full)
{
	if(!pClipRect) pClipRect = pRect;
	EGRect CornerRect(pRect->GetX1(), pRect->GetY1(), pRect->GetX1() + FragSize - 1, pRect->GetY1() + FragSize - 1);	// Upper left
	EGRect DrawRect;
	if(DrawRect.Intersect(&CornerRect, pClipRect)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);
		int32_t DrawWidth = DrawRect.GetWidth();
    int32_t DrawHeight = DrawRect.GetHeight();
		int32_t sx = (int32_t)(DrawRect.GetX1() - CornerRect.GetY1());
    int32_t sy = (int32_t)(DrawRect.GetY1() - CornerRect.GetY1());
		SDL_Rect SrceRect = {sx, sy, DrawWidth, DrawHeight};
		SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
	}
	CornerRect.SetX1(EG_MAX(pRect->GetX2() - FragSize + 1, pRect->GetX1() + FragSize));	// Upper right, pClipRect right edge if too big
	CornerRect.SetX2(pRect->GetX2());
	if(DrawRect.Intersect(&CornerRect, pClipRect)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);
		int32_t DrawWidth = DrawRect.GetWidth();
    int32_t DrawHeight = DrawRect.GetHeight();
		if(Full) {
			int32_t sx = (int32_t)(DrawRect.GetX1() - CornerRect.GetX1());
			int32_t sy = (int32_t)(DrawRect.GetY1() - CornerRect.GetY1());
			SDL_Rect SrceRect = {FragSize + FRAG_SPACING + sx, sy, DrawWidth, DrawHeight};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			SDL_Rect SrceRect = {CornerRect.GetX2() - DrawRect.GetX2(), DrawRect.GetY1() - CornerRect.GetY1(), DrawWidth, DrawHeight};
			SDL_RenderCopyEx(m_pRenderer, pFrag, &SrceRect, &DestRect, 0, nullptr, SDL_FLIP_HORIZONTAL);
		}
	}
	CornerRect.SetY1(EG_MAX(pRect->GetY2() - FragSize + 1, pRect->GetY1() + FragSize));	// Lower right, pClipRect bottom edge if too big
	CornerRect.SetY2(pRect->GetY2());
	if(DrawRect.Intersect(&CornerRect, pClipRect)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);
		int32_t DrawWidth = DrawRect.GetWidth();
    int32_t DrawHeight = DrawRect.GetHeight();
		if(Full) {
			int32_t sx = (int32_t)(DrawRect.GetX1() - CornerRect.GetX1());
			int32_t sy = (int32_t)(DrawRect.GetY1() - CornerRect.GetY1());
			SDL_Rect SrceRect = {FragSize + FRAG_SPACING + sx, FragSize + FRAG_SPACING + sy, DrawWidth, DrawHeight};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			SDL_Rect SrceRect = {CornerRect.GetX2() - DrawRect.GetX2(), CornerRect.GetY2() - DrawRect.GetY2(), DrawWidth, DrawHeight};
			SDL_RenderCopyEx(m_pRenderer, pFrag, &SrceRect, &DestRect, 0, nullptr, (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
		}
	}
	CornerRect.SetX1(pRect->GetX1());	// Lower left, right edge should not be pClipped
	CornerRect.SetX2(pRect->GetX1() + FragSize - 1);
	if(DrawRect.Intersect(&CornerRect, pClipRect)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);
		int32_t DrawWidth = DrawRect.GetWidth();
    int32_t DrawHeight = DrawRect.GetHeight();
		if(Full) {
			int32_t sx = (int32_t)(DrawRect.GetX1() - CornerRect.GetX1());
			int32_t sy = (int32_t)(DrawRect.GetY1() - CornerRect.GetY1());
			SDL_Rect SrceRect = {sx, FragSize + FRAG_SPACING + sy, DrawWidth, DrawHeight};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			SDL_Rect SrceRect = {DrawRect.GetX1() - CornerRect.GetX1(), CornerRect.GetY2() - DrawRect.GetY2(), DrawWidth, DrawHeight};
			SDL_RenderCopyEx(m_pRenderer, pFrag, &SrceRect, &DestRect, 0, nullptr, SDL_FLIP_VERTICAL);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectBackColor(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj)
{
	if(pDrawObj->m_BackgroundOPA == 0) return;
	int32_t Radius = pDrawObj->m_Radius;
	SDL_Color Color;
	if(pDrawObj->m_BackgroundGrad.dir == EG_GRAD_DIR_NONE)	ColorToSDLColor(&pDrawObj->m_BackgroundColor, &Color);
	else if(pDrawObj->m_BackgroundGrad.StopCount == 1) ColorToSDLColor(&pDrawObj->m_BackgroundGrad.stops[0].color, &Color);
	else {
		if(Radius <= 0) DrawRectBackGradSimple(pRect, pDrawRect, &pDrawObj->m_BackgroundGrad, false);
		else DrawRectBackGradRadius(pRect, pDrawRect, pDrawObj);
		return;
	}
	if(Radius <= 0) {
		SDL_Rect Rect;
		RectToSDLRect(pDrawRect, &Rect);
		SDL_SetRenderDrawColor(m_pRenderer, Color.r, Color.g, Color.b, pDrawObj->m_BackgroundOPA);
		SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(m_pRenderer, &Rect);
		return;
	}
	int32_t Width = pRect->GetWidth();	// A small pTexture with a quarter of the rect is enough
  int32_t Height = pRect->GetHeight();
	int32_t DrawRadius = EG_MIN3(Width / 2, Height / 2, Radius);
	bool InCache = false;
	SDL_Texture *pTexture = RectBackgroundGetFrag(DrawRadius, &InCache);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(pTexture, pDrawObj->m_BackgroundOPA);
	SDL_SetTextureColorMod(pTexture, Color.r, Color.g, Color.b);
	RectBackgroundFragDrawCorners(pTexture, DrawRadius, pRect, pDrawRect, false);
	FragRenderBorders(pTexture, DrawRadius, pRect, pDrawRect, false);
	FragRenderCenter(pTexture, DrawRadius, pRect, pDrawRect, false);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectBackGradSimple(const EGRect *pRect, const EGRect *pDrawRect, const EG_GradDescriptor_t *pGrad, bool BlendMod)
{
SDL_Rect DestRect, SrceRect;

	RectToSDLRect(pDrawRect, &DestRect);
	if(pGrad->dir == EG_GRAD_DIR_VER) {
		int32_t Height = pRect->GetHeight();
		SrceRect.x = 0;
		SrceRect.y = (pDrawRect->GetY1() - pRect->GetY1()) * 255 / Height;
		SrceRect.w = 1;
		SrceRect.h = DestRect.h * 256 / Height;
		if(SrceRect.y < 0 || SrceRect.y > 255) {
			return;
		}
	}
	else {
		int32_t Width = pRect->GetWidth();
		SrceRect.x = (pDrawRect->GetX1() - pRect->GetX1()) * 255 / Width;
		SrceRect.y = 0;
		SrceRect.w = DestRect.w * 256 / Width;
		SrceRect.h = 1;
		if(SrceRect.x < 0 || SrceRect.x > 255) {
			return;
		}
	}
	bool InCache = false;
	SDL_Texture *pTexture = RectGradGetStrip(pGrad, &InCache);
	if(BlendMod) SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_MOD);
	else SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(m_pRenderer, pTexture, &SrceRect, &DestRect);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectBackGradRadius(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj)
{
	int32_t Radius = pDrawObj->m_Radius;
	// A small pTexture with a quarter of the rect is enough
	int32_t Width = pRect->GetWidth();
  int32_t Height = pRect->GetHeight();
	int32_t DrawRadius = EG_MIN3(Width / 2, Height / 2, Radius);
	bool InCache = false;
	SDL_Texture *pTexture = RectGradGetFrag(&pDrawObj->m_BackgroundGrad, Width, Height, Radius,	&InCache);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	RectBackgroundFragDrawCorners(pTexture, DrawRadius, pRect, pDrawRect, true);
	EGRect DrawRect;
	if(Width > Radius * 2) {
    EGRect PartRect(0, Radius, Radius - 1, Height - Radius - 1);		// Draw left, middle, right
		pRect->Align(&PartRect, EG_ALIGN_LEFT_MID, 0, 0);
		DrawRect.Intersect(&PartRect, pDrawRect);
		DrawRectBackGradSimple(pRect, &DrawRect, &pDrawObj->m_BackgroundGrad, false);
		pRect->Align(&PartRect, EG_ALIGN_RIGHT_MID, 0, 0);
		DrawRect.Intersect(&PartRect, pDrawRect);
		DrawRectBackGradSimple(pRect, &DrawRect, &pDrawObj->m_BackgroundGrad, false);
		PartRect.Set(Radius, 0, Width - Radius - 1, Height - 1);
		pRect->Align(&PartRect, EG_ALIGN_CENTER, 0, 0);
		DrawRect.Intersect(&PartRect, pDrawRect);
		DrawRectBackGradSimple(pRect, &DrawRect, &pDrawObj->m_BackgroundGrad, false);
	}
	else if(Height > Radius * 2) {
		EGRect PartRect(Radius, 0, Width - Radius - 1, Radius - 1);		// Draw top, middle, bottom
		pRect->Align(&PartRect, EG_ALIGN_TOP_MID, 0, 0);
		DrawRect.Intersect(&PartRect, pDrawRect);
		DrawRectBackGradSimple(pRect, &DrawRect, &pDrawObj->m_BackgroundGrad, false);
		pRect->Align(&PartRect, EG_ALIGN_BOTTOM_MID, 0, 0);
		DrawRect.Intersect(&PartRect, pDrawRect);
		DrawRectBackGradSimple(pRect, &DrawRect, &pDrawObj->m_BackgroundGrad, false);
		PartRect.Set(0, Radius, Width - 1, Height - Radius - 1);
		pRect->Align(&PartRect, EG_ALIGN_CENTER, 0, 0);
		DrawRect.Intersect(&PartRect, pDrawRect);
		DrawRectBackGradSimple(pRect, &DrawRect, &pDrawObj->m_BackgroundGrad, false);
	}
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectBackImage(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj)
{
	EG_UNUSED(pDrawRect);
	if(SKIP_IMAGE(pDrawObj)) return;
	EG_ImageSource_e SrceType = EGDrawImage::GetType(pDrawObj->m_pBackImageSource);
	if(SrceType == EG_IMG_SRC_SYMBOL) {
		EGSize Size;
		EG_GetTextSize(&Size, (char*)pDrawObj->m_pBackImageSource, (EG_Font_t*)pDrawObj->m_pBackImageSymbolFont, 0, 0, EG_COORD_MAX, EG_TEXT_FLAG_NONE);
		EGRect Rect;
		Rect.SetX1(pRect->GetX1() + pRect->GetWidth() / 2 - Size.m_X / 2);
		Rect.SetX2(Rect.GetX1() + Size.m_X - 1);
		Rect.SetY1(pRect->GetY1() + pRect->GetHeight() / 2 - Size.m_Y / 2);
		Rect.SetY2(Rect.GetY1() + Size.m_Y - 1);
		EGDrawLabel DrawLabel;
		DrawLabel.m_pFont = (EG_Font_t*)pDrawObj->m_pBackImageSymbolFont;
		DrawLabel.m_Color = pDrawObj->m_BackImageRecolor;
		DrawLabel.m_OPA = pDrawObj->m_BackImageOPA;
		DrawLabel.Draw(DrawLabel.m_pContext, &Rect, (char*)pDrawObj->m_pBackImageSource, nullptr);
	}
	else {
		EG_ImageHeader_t Header;
		size_t KeySize;
		EG_CacheKeyHeadImage_t *pKey = CreateTextureImageKey(pDrawObj->m_pBackImageSource, 0, &KeySize);
		bool Found;
		EG_ImageHeader_t *pCacheHeader = nullptr;
		SDL_Texture *pTexture = TextureCacheGetWithExtData(pKey, KeySize, &Found, (void **)&pCacheHeader);
		SDL_free(pKey);
		if(pTexture) Header = *pCacheHeader;
			else if(Found || EGImageDecoder::GetInfo(pDrawObj->m_pBackImageSource, &Header) != EG_RES_OK) {
			// When cache hit but with negative result, use default decoder. If still fail, return.
			EG_LOG_WARN("Couldn't read the background image");
			return;
		}
		EGDrawImage DrawImage;
		DrawImage.m_BlendMode = pDrawObj->m_BlendMode;
		DrawImage.m_Recolor = pDrawObj->m_BackImageRecolor;
		DrawImage.m_RecolorOPA = pDrawObj->m_BackImageRecolorOPA;
		DrawImage.m_OPA = pDrawObj->m_BackImageOPA;
		DrawImage.m_FrameID = 0;
		int16_t MaskID = EG_MASK_ID_INVALID;
		MaskRadiusParam_t Param;
		if(pDrawObj->m_Radius > 0) {
			DrawMaskSetRadius(&Param, pRect, pDrawObj->m_Radius, false);
			MaskID = DrawMaskAdd(&Param, nullptr);
		}
		if(pDrawObj->m_BackImageTiled == false) {      // Center align
			EGRect Rect;
			Rect.SetX1(pRect->GetX1() + pRect->GetWidth() / 2 - Header.Width / 2);
			Rect.SetY1(pRect->GetY1() + pRect->GetHeight() / 2 - Header.Height / 2);
			Rect.SetX2(Rect.GetX1() + Header.Width - 1);
			Rect.SetY2(Rect.GetY1() + Header.Height - 1);
			DrawImage.Draw(DrawImage.m_pContext, &Rect, pDrawObj->m_pBackImageSource);
		}
		else {
			EGRect Rect;
			Rect.SetY1(pRect->GetY1());
			Rect.SetY2(Rect.GetY1() + Header.Height - 1);
			for(; Rect.GetY1() <= pRect->GetY2(); Rect.IncY1(Header.Height), Rect.IncY2(Header.Height)) {
				Rect.SetX1(pRect->GetX1());
				Rect.SetX2(Rect.GetX1() + Header.Width - 1);
				for(; Rect.GetX1() <= pRect->GetX2(); Rect.IncX1(Header.Width), Rect.IncX2(Header.Width)) {
					DrawImage.Draw(DrawImage.m_pContext, &Rect, pDrawObj->m_pBackImageSource);
				}
			}
		}
		if(MaskID != EG_MASK_ID_INVALID) {
			DrawMaskRemoveID(MaskID);
			DrawMaskFreeParam(&Param);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectShadow(const EGRect *pRect, const EGRect *pClipRect, const EGDrawRect *pDrawObj)
{
	// Check whether the shadow is visible
	if(SKIP_SHADOW(pDrawObj)) return;
	int32_t ShadowWidth = pDrawObj->m_ShadowWidth;

	EGRect CoreRect(pRect->GetX1() + pDrawObj->m_ShadowOffsetX - pDrawObj->m_ShadowSpread,
                  pRect->GetY1() + pDrawObj->m_ShadowOffsetY - pDrawObj->m_ShadowSpread,
                  pRect->GetX2() + pDrawObj->m_ShadowOffsetX + pDrawObj->m_ShadowSpread,
                  pRect->GetY2() + pDrawObj->m_ShadowOffsetY + pDrawObj->m_ShadowSpread);

	EGRect ShadowRect(CoreRect.GetX1() - ShadowWidth / 2 - 1, CoreRect.GetY1() - ShadowWidth / 2 - 1,
                    CoreRect.GetX2() + ShadowWidth / 2 + 1, CoreRect.GetY2() + ShadowWidth / 2 + 1);
	EG_OPA_t OPA = pDrawObj->m_ShadowOPA;
	if(OPA > EG_OPA_MAX) OPA = EG_OPA_COVER;
	// Get pClipped draw area which is the real draw area. It is always the same or inside `ShadowRect`
	EGRect DrawRect;
	if(!DrawRect.Intersect(&ShadowRect, pClipRect)) return;
	SDL_Rect SdlCoreRect;
	RectToSDLRect(&ShadowRect, &SdlCoreRect);
	int32_t Radius = pDrawObj->m_Radius;
	// No matter how big the shadow is, what we need is just a corner
	int32_t FragSize = EG_MIN3(CoreRect.GetWidth() / 2, CoreRect.GetHeight() / 2, EG_MAX(ShadowWidth / 2, Radius));
	int32_t BlurGrowth = (int32_t)(ShadowWidth / 2 + 1);	// This is how big the corner is after blurring
	int32_t BlurFragSize = (int32_t)(FragSize + BlurGrowth);
	EG_RectShadowKey_t Key = CreateRectShadowKey(Radius, FragSize, ShadowWidth);
	SDL_Texture *pTexture = TextureCacheGet(&Key, sizeof(Key), nullptr);
	bool InCache = false;
	if(pTexture == nullptr) {
		EGRect MaskRect(BlurGrowth, BlurGrowth, 0, 0);
		MaskRect.SetWidth(FragSize * 2);
		MaskRect.SetHeight(FragSize * 2);
    EGRect MaskRectBlurred;
		MaskRectBlurred.SetWidth(BlurFragSize * 2);
		MaskRectBlurred.SetHeight(BlurFragSize * 2);
		MaskRadiusParam_t MaskParam;
		DrawMaskSetRadius(&MaskParam, &MaskRect, Radius, false);
		int16_t MaskID = DrawMaskAdd(&MaskParam, nullptr);
		EG_OPA_t *pMask = MaskDumpOPA(&MaskRectBlurred, &MaskID, 1);
		StackBlurGrayscale(pMask, MaskRectBlurred.GetWidth(), MaskRectBlurred.GetHeight(), ShadowWidth / 2 + ShadowWidth % 2);
		pTexture = CreateOPATexture(m_pRenderer, pMask, BlurFragSize, BlurFragSize,	MaskRectBlurred.GetWidth());
		EG_ReleaseBufferMem(pMask);
		DrawMaskRemoveID(MaskID);
		SDL_assert(pTexture);
		InCache = TextureCachePut(&Key, sizeof(Key), pTexture);
	}
	else InCache = true;
	SDL_Color ShadowColor;
	ColorToSDLColor(&pDrawObj->m_ShadowColor, &ShadowColor);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(pTexture, OPA);
	SDL_SetTextureColorMod(pTexture, ShadowColor.r, ShadowColor.g, ShadowColor.b);
	RectBackgroundFragDrawCorners(pTexture, BlurFragSize, &ShadowRect, pClipRect, false);
	FragRenderBorders(pTexture, BlurFragSize, &ShadowRect, pClipRect, false);
	FragRenderCenter(pTexture, BlurFragSize, &ShadowRect, pClipRect, false);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectBorder(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj)
{
	if(SKIP_BORDER(pDrawObj)) return;
	SDL_Color BorderColor;
	ColorToSDLColor(&pDrawObj->m_BorderColor, &BorderColor);
	int32_t Width = pRect->GetWidth();
  int32_t Height = pRect->GetHeight();
	int32_t ShortSide = EG_MIN(Width, Height);
	int32_t RadiusOut = EG_MIN(pDrawObj->m_Radius, ShortSide / 2); //Get the inner area
	EGRect InnerRect;
	InnerRect = pRect;  //        increase(&InnerRect, 1, 1);
	InnerRect.IncX1(((pDrawObj->m_BorderSide & EG_BORDER_SIDE_LEFT) ? pDrawObj->m_BorderWidth : -(pDrawObj->m_BorderWidth + RadiusOut)));
	InnerRect.DecX2(((pDrawObj->m_BorderSide & EG_BORDER_SIDE_RIGHT) ? pDrawObj->m_BorderWidth : -(pDrawObj->m_BorderWidth + RadiusOut)));
	InnerRect.IncY1(((pDrawObj->m_BorderSide & EG_BORDER_SIDE_TOP) ? pDrawObj->m_BorderWidth : -(pDrawObj->m_BorderWidth + RadiusOut)));
	InnerRect.DecY2(((pDrawObj->m_BorderSide & EG_BORDER_SIDE_BOTTOM) ? pDrawObj->m_BorderWidth : -(pDrawObj->m_BorderWidth + RadiusOut)));
	int32_t RadiusIn = EG_MAX(RadiusOut - pDrawObj->m_BorderWidth, 0);
	DrawRectBorderGeneric(pRect, &InnerRect, pDrawRect, RadiusOut, RadiusIn, pDrawObj->m_BorderColor, pDrawObj->m_BorderOPA,	pDrawObj->m_BlendMode);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectOutline(const EGRect *pRect, const EGRect *pClipRect, const EGDrawRect *pDrawObj)
{
	if(SKIP_OUTLINE(pDrawObj)) return;
	EG_OPA_t OPA = pDrawObj->m_OutlineOPA;
	if(OPA > EG_OPA_MAX) OPA = EG_OPA_COVER;
	EGRect InnerRect(pRect);	// Get the inner Radius
	// Bring the outline closer to make sure there is no color bleeding with pad=0
	int32_t Pad = pDrawObj->m_OutlinePadding - 1;
	InnerRect.Inflate(Pad, Pad);
	EGRect OuterRect(InnerRect);
	OuterRect.Inflate(pDrawObj->m_OutlineWidth, pDrawObj->m_OutlineWidth);
	EGRect DrawRect;
	if(!DrawRect.Intersect(&OuterRect, pClipRect)) return;
	int32_t inner_w = InnerRect.GetWidth();
	int32_t inner_h = InnerRect.GetHeight();
	int32_t RadiusIn = pDrawObj->m_Radius;
	int32_t ShortSide = EG_MIN(inner_w, inner_h);
	if(RadiusIn > ShortSide >> 1) RadiusIn = ShortSide >> 1;
	int32_t RadiusOut = RadiusIn + pDrawObj->m_OutlineWidth;
	DrawRectBorderGeneric(&OuterRect, &InnerRect, pClipRect, RadiusOut, RadiusIn, pDrawObj->m_OutlineColor, pDrawObj->m_OutlineOPA,	pDrawObj->m_BlendMode);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::DrawRectBorderGeneric(const EGRect *pOuterRect, const EGRect *pInnerRect, const EGRect *pClipRect,
  int32_t RadiusOut, int32_t RadiusIn, EG_Color_t Color, EG_OPA_t OPA, EG_BlendMode_e BlendMode)
{
	OPA = OPA >= EG_OPA_COVER ? EG_OPA_COVER : OPA;
	EG_RectBorderKey_t Key = CreateRectBorderKey(RadiusOut, RadiusIn, pOuterRect, pInnerRect);
	int32_t Radius = EG_MIN3(RadiusOut, pOuterRect->GetWidth() / 2, pOuterRect->GetHeight() / 2);
	int32_t max_side = EG_MAX4(Key.Offsets.GetX1(), Key.Offsets.GetY1(), -Key.Offsets.GetX2(), -Key.Offsets.GetY2());
	int32_t FragSize = EG_MAX(Radius, max_side);
	SDL_Texture *pTexture = TextureCacheGet(&Key, sizeof(Key), nullptr);
	bool InCache;
	if(pTexture == nullptr) {
		// Create a mask pTexture with size of (FragSize * 2 + FRAG_SPACING)
		const EGRect FragRect = {0, 0, FragSize * 2 + FRAG_SPACING - 1, FragSize * 2 + FRAG_SPACING - 1};
		// Create mask for the outer area
		int16_t MaskIDs[2] = {EG_MASK_ID_INVALID, EG_MASK_ID_INVALID};
		MaskRadiusParam_t MaskParam;
		if(RadiusOut > 0) {
			DrawMaskSetRadius(&MaskParam, &FragRect, RadiusOut, false);
			MaskIDs[0] = DrawMaskAdd(&MaskParam, nullptr);
		}
		// Create mask for the inner mask
		if(RadiusIn < 0) RadiusIn = 0;
		const EGRect FragInnerRect(FragRect.GetX1() + Key.Offsets.GetX1(), FragRect.GetY1() + Key.Offsets.GetY1(),
																			 FragRect.GetX2() + Key.Offsets.GetX2(), FragRect.GetY2() + Key.Offsets.GetY2());
    MaskRadiusParam_t mask_rin_param;
		DrawMaskSetRadius(&mask_rin_param, &FragInnerRect, RadiusIn, true);
		MaskIDs[1] = DrawMaskAdd(&mask_rin_param, nullptr);
		pTexture = MaskDumpTexture(&FragRect, MaskIDs, 2);
		DrawMaskRemoveID(MaskIDs[1]);
		DrawMaskRemoveID(MaskIDs[0]);
		SDL_assert(pTexture);
		InCache = TextureCachePut(&Key, sizeof(Key), pTexture);
	}
	else InCache = true;
	SDL_Rect OuterRect;
	RectToSDLRect(pOuterRect, &OuterRect);
	SDL_Color SDLColor;
	ColorToSDLColor(&Color, &SDLColor);
	SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(pTexture, OPA);
	SDL_SetTextureColorMod(pTexture, SDLColor.r, SDLColor.g, SDLColor.b);
	RectBackgroundFragDrawCorners(pTexture, FragSize, pOuterRect, pClipRect, true);
	FragRenderBorders(pTexture, FragSize, pOuterRect, pClipRect, true);
	if(!InCache) {
		EG_LOG_WARN("Texture is not cached, this will impact performance.");
		SDL_DestroyTexture(pTexture);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::FragRenderBorders(SDL_Texture *pFrag, int32_t FragSize, const EGRect *pRect, const EGRect *pClipped, bool Full)
{
	EGRect BorderRect(pRect->GetX1() + FragSize, pRect->GetY1(), pRect->GetX2() - FragSize, pRect->GetY1() + FragSize - 1);
	// Top border
	EGRect DrawRect;
	if(DrawRect.Intersect(&BorderRect, pClipped)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);
		int32_t sy = (int32_t)(DrawRect.GetY1() - BorderRect.GetY1());
		if(Full) {
			SDL_Rect SrceRect = {FragSize + 1, sy, 1, DrawRect.GetHeight()};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			SDL_Rect SrceRect = {FragSize - 1, sy, 1, DrawRect.GetHeight()};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
	}
	// Bottom border
	BorderRect.SetX1(EG_MAX(pRect->GetY2() - FragSize + 1, pRect->GetY1() + FragSize));
	BorderRect.SetY2(pRect->GetY2());
	if(DrawRect.Intersect(&BorderRect, pClipped)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);
		int32_t DrawHeight = DrawRect.GetHeight();
		if(Full) {
			int32_t sy = (int32_t)(DrawRect.GetY1() - BorderRect.GetY1());
			SDL_Rect SrceRect = {FragSize + 1, FragSize + FRAG_SPACING + sy, 1, DrawHeight};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			int32_t sy = (int32_t)(BorderRect.GetY2() - DrawRect.GetY2());
			SDL_Rect SrceRect = {FragSize - 1, sy, 1, DrawHeight};
			SDL_RenderCopyEx(m_pRenderer, pFrag, &SrceRect, &DestRect, 0, nullptr, SDL_FLIP_VERTICAL);
		}
	}
	// Left border
	BorderRect.Set(pRect->GetX1(), pRect->GetY1() + FragSize, pRect->GetX1() + FragSize - 1, pRect->GetY2() - FragSize);
	if(DrawRect.Intersect(&BorderRect, pClipped)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);

		int32_t DrawWidth = DrawRect.GetWidth();
		int32_t sx = (int32_t)(DrawRect.GetX1() - BorderRect.GetX1());
		if(Full) {
			SDL_Rect SrceRect = {sx, FragSize + 1, DrawWidth, 1};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			SDL_Rect SrceRect = {sx, FragSize - 1, DrawWidth, 1};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
	}
	// Right border
	BorderRect.SetX1(EG_MAX(pRect->GetX2() - FragSize + 1, pRect->GetX1() + FragSize));
	BorderRect.SetX2(pRect->GetX2());
	if(DrawRect.Intersect(&BorderRect, pClipped)) {
		SDL_Rect DestRect;
		RectToSDLRect(&DrawRect, &DestRect);

		int32_t DrawWidth = DrawRect.GetWidth();
		if(Full) {
			int32_t sx = (int32_t)(DrawRect.GetX1() - BorderRect.GetX1());
			SDL_Rect SrceRect = {FragSize + FRAG_SPACING + sx, FragSize + 1, DrawWidth, 1};
			SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
		}
		else {
			int32_t sx = (int32_t)(BorderRect.GetX2() - DrawRect.GetX2());
			SDL_Rect SrceRect = {sx, FragSize - 1, DrawWidth, 1};
			SDL_RenderCopyEx(m_pRenderer, pFrag, &SrceRect, &DestRect, 0, nullptr, SDL_FLIP_HORIZONTAL);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::FragRenderCenter(SDL_Texture *pFrag, int32_t FragSize, const EGRect *pRect, const EGRect *pClipped, bool Full)
{
	EGRect CenterRect(pRect->GetX1() + FragSize,	pRect->GetY1() + FragSize,	pRect->GetX2() - FragSize,	pRect->GetY2() - FragSize);
	if(CenterRect.GetX2() < CenterRect.GetX1() || CenterRect.GetY2() < CenterRect.GetY1()) return;
	EGRect DrawRect;
	if(!DrawRect.Intersect(&CenterRect, pClipped)) return;
	SDL_Rect DestRect;
	RectToSDLRect(&DrawRect, &DestRect);
	if(Full) {
		SDL_Rect SrceRect = {FragSize, FragSize, 1, 1};
		SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
	}
	else {
		SDL_Rect SrceRect = {FragSize - 1, FragSize - 1, 1, 1};
		SDL_RenderCopy(m_pRenderer, pFrag, &SrceRect, &DestRect);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_RectBackgroundKey_t EGSDLContext::CreateRectBackgroundKey(int32_t Radius, int32_t Size)
{
	EG_RectBackgroundKey_t Key;
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_RECT_BG;
	Key.Radius = Radius;
	Key.Size = Size;
	return Key;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_RectGradFragKey_t EGSDLContext::CreateRectGradFragKey(const EG_GradDescriptor_t *pGrad, int32_t Width, int32_t Height, int32_t Radius)
{
	EG_RectGradFragKey_t Key;
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_RECT_GRAD;
	Key.StopCount = pGrad->StopCount;
	Key.Dir = pGrad->dir;
	for(uint8_t i = 0; i < pGrad->StopCount; i++) {
		Key.Stops[i].frac = pGrad->stops[i].frac;
		Key.Stops[i].color = pGrad->stops[i].color;
	}
	Key.Width = Width;
	Key.Height = Height;
	Key.Radius = Radius;
	return Key;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_RectGradStripKey_t EGSDLContext::CreateRectGradStripKey(const EG_GradDescriptor_t *pGrad)
{
	EG_RectGradStripKey_t Key;
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_RECT_GRAD;
	Key.StopCount = pGrad->StopCount;
	Key.Dir = pGrad->dir;
	for(uint8_t i = 0; i < pGrad->StopCount; i++) {
		Key.Stops[i].frac = pGrad->stops[i].frac;
		Key.Stops[i].color = pGrad->stops[i].color;
	}
	return Key;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_RectShadowKey_t EGSDLContext::CreateRectShadowKey(int32_t Radius, int32_t Size, int32_t Blur)
{
	EG_RectShadowKey_t Key;
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_RECT_SHADOW;
	Key.Radius = Radius;
	Key.Size = Size;
	Key.Blur = Blur;
	return Key;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_RectBorderKey_t EGSDLContext::CreateRectBorderKey(int32_t RadiusOut, int32_t RadiusIn, const EGRect *pOuterRect, const EGRect *pInnerRect)
{
	EG_RectBorderKey_t Key;
	// VERY IMPORTANT! Padding between members is uninitialized, so we have to wipe them manually
	SDL_memset(&Key, 0, sizeof(Key));
	Key.Magic = EG_GPU_CACHE_KEY_MAGIC_RECT_BORDER;
	Key.RadiusOut = RadiusOut;
	Key.RadiusIn = RadiusIn;
	Key.Offsets.SetX1(pInnerRect->GetX1() - pOuterRect->GetX1());
	Key.Offsets.SetX2(pInnerRect->GetX2() - pOuterRect->GetX2());
	Key.Offsets.SetY1(pInnerRect->GetY1() - pOuterRect->GetY1());
	Key.Offsets.SetY2(pInnerRect->GetY2() - pOuterRect->GetY2());
	return Key;
}

#endif
