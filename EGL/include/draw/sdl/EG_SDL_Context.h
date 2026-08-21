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

#pragma once

#ifndef __EGSDLCONTEXT__
#define __EGSDLCONTEXT__

#include "../../EG_IntrnlConfig.h"

#if EG_USE_GPU_SDL

//#include EG_GPU_SDL_INCLUDE_PATH
#include "SDL.h"

#include "draw/EG_DeviceContext.h"
#include "hal/EG_HALDisplay.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Color.h"
#include "misc/EG_LRU.h"
#include "misc/EG_Misc.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define EG_DRAW_SDL_TEXTURE_FORMAT SDL_PIXELFORMAT_ARGB8888
#else
#define EG_DRAW_SDL_TEXTURE_FORMAT SDL_PIXELFORMAT_RGBA8888
#endif

///////////////////////////////////////////////////////////////////////////////////////

enum EG_SDL_CompositeTextures_e {
    EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_STREAM0,
    EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_STREAM1,
    EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TARGET0,
    EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TARGET1,
    EG_DRAW_SDL_COMPOSITE_TEXTURE_ID_TRANSFORM0,
};

#define EG_DRAW_SDL_DEC_DSC_TEXTURE_HEAD "@EGLTex"

enum EG_SDL_CacheKeyMagic_e{
  EG_GPU_CACHE_KEY_MAGIC_ARC = 0x01,
  EG_GPU_CACHE_KEY_MAGIC_IMG = 0x11,
  EG_GPU_CACHE_KEY_MAGIC_IMAGE_ROUNDED_CORNERS = 0x12,
  EG_GPU_CACHE_KEY_MAGIC_LINE = 0x21,
  EG_GPU_CACHE_KEY_MAGIC_RECT_BG = 0x31,
  EG_GPU_CACHE_KEY_MAGIC_RECT_SHADOW = 0x32,
  EG_GPU_CACHE_KEY_MAGIC_RECT_BORDER = 0x33,
  EG_GPU_CACHE_KEY_MAGIC_RECT_GRAD = 0x34,
  EG_GPU_CACHE_KEY_MAGIC_FONT_GLYPH = 0x41,
  EG_GPU_CACHE_KEY_MAGIC_MASK = 0x51,
};

enum EG_SDL_CacheFlag_e{
  EG_DRAW_SDL_CACHE_FLAG_NONE = 0,
  EG_DRAW_SDL_CACHE_FLAG_MANAGED = 1,
};

enum EG_RoundesImagePart_e{
  ROUNDED_IMAGE_PART_LEFT = 0,
  ROUNDED_IMAGE_PART_HCENTER = 1,
  ROUNDED_IMAGE_PART_RIGHT = 2,
  ROUNDED_IMAGE_PART_TOP = 3,
  ROUNDED_IMAGE_PART_VCENTER = 4,
  ROUNDED_IMAGE_PART_BOTTOM = 5,
};

enum EG_RoundesImageCorner_e{
  ROUNDED_IMAGE_CORNER_TOP_LEFT = 0,
  ROUNDED_IMAGE_CORNER_TOP_RIGHT = 1,
  ROUNDED_IMAGE_CORNER_BOTTOM_RIGHT = 2,
  ROUNDED_IMAGE_CORNER_BOTTOM_LEFT = 3,
};

typedef struct {
    SDL_Renderer  *pRenderer;
    void          *pExtData;
} EG_SDL_DriverParam_t;

typedef struct EG_SDL_Dec_ExtData_t{
  char            Head[8];
  SDL_Texture     *pTexture;
  SDL_Rect        Rect;
  bool            TextureManaged;
  bool            TextureReferenced;
} EG_SDL_Dec_ExtData_t;

typedef struct EG_CacheKeyHeadImage_t{
  EG_SDL_CacheKeyMagic_e Magic;
  EG_ImageSource_e  Type;
  int32_t           FrameID;
} EG_CacheKeyHeadImage_t;

typedef struct EG_SDL_RectHeader_t {
  EG_ImageHeader_t Base;
  SDL_Rect        Rect;
} EG_SDL_RectHeader_t;

typedef struct EG_SDL_ImageHeader_t {
  EG_ImageHeader_t Base;
  SDL_Rect        Rect;
  bool            Managed;
} EG_SDL_ImageHeader_t;

typedef struct CompositeKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  EG_SDL_CompositeTextures_e Type;
} CompositeKey_t;

typedef struct DrawCacheValue_t{
  SDL_Texture *pTexture;
  void        *pExtData;
  EG_LRU_Free_t *pExtDataFree;
  EG_SDL_CacheFlag_e Flags;
} DrawCacheValue_t;

typedef struct EG_FontGlyphKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  const EG_Font_t       *pFont;
  uint32_t              Character;
} EG_FontGlyphKey_t;

typedef struct EG_LineKey_t{
	EG_SDL_CacheKeyMagic_e Magic;
	int32_t     Length;
	int32_t     Width;
	uint8_t     Round;
} EG_LineKey_t;

typedef struct EG_ImageRoundedKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  const SDL_Texture     *pTexture;
  int32_t               Width;
  int32_t               Height;
  int32_t               Radius;
} EG_ImageRoundedKey_t;

typedef struct EG_RectBackgroundKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  int32_t             Radius;
  int32_t             Size;
} EG_RectBackgroundKey_t;

typedef struct EG_RectGradStripKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  EG_GradientStop_t   Stops[EG_GRADIENT_MAX_STOPS];
  uint8_t             StopCount;
  EG_GradDirection_e  Dir;
} EG_RectGradStripKey_t;

typedef struct EG_RectGradFragKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  EG_GradientStop_t   Stops[EG_GRADIENT_MAX_STOPS];
  uint8_t             StopCount;
  EG_GradDirection_e  Dir;
  int32_t             Width;
  int32_t             Height;
  int32_t             Radius;
} EG_RectGradFragKey_t;

typedef struct EG_RectShadowKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  int32_t             Radius;
  int32_t             Size;
  int32_t             Blur;
} EG_RectShadowKey_t;

typedef struct EG_RectBorderKey_t{
  EG_SDL_CacheKeyMagic_e Magic;
  int32_t           RadiusOut;
  int32_t           RadiusIn;
  EGRect            Offsets;
} EG_RectBorderKey_t;

///////////////////////////////////////////////////////////////////////////////////////

class EG_SDLLayerContext : public EGLayerContext
{
public:
                      EG_SDLLayerContext();
                     ~EG_SDLLayerContext(){};
  SDL_Texture        *m_pOrigTarget;
  SDL_Texture        *m_pTarget;
  SDL_Rect            m_TargetRect;
  bool                m_InCache;
  EGDrawLayerFlags_e  m_Flags;
};


///////////////////////////////////////////////////////////////////////////////////////

class EGSDLContext : public EGDeviceContext
{
public:
                  EGSDLContext() : EGDeviceContext(){};
  virtual         ~EGSDLContext(void);
  void            InitialiseContext(EGDisplayDriver *pDriver);

  SDL_Texture*    CreateScreenTexture(int32_t Horizontal, int32_t Vertical);

private:
  static void           DrawLine(EGDrawLine *pDrawLine, const EGPoint *pPoint1, const EGPoint *pPoint2);
  static void           DrawArc(EGDrawArc *pDrawArc, const EGPoint *pCenter, uint16_t Radius,  uint16_t StartAngle, uint16_t EndAngle);
  static void           DrawRect(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static void           DrawCharacter(const EGDrawLabel *pDrawLabel,  const EGPoint *pPos, uint32_t Char);
  static void           DrawPolygon(const EGDrawPolygon *pDrawPolygon, const EGPoint *pPoints, uint16_t PointCount);
  static void           DrawBackground(const EGDrawRect *pDrawRect, const EGRect *pRect);
  static EG_Result_t    DrawImage(EGDrawImage *pDrawImage, const EGRect *pRect, const void *pSourceBuffer);
  static bool           DrawLayerCreate(EGLayerContext *pDrawLayer,	EGDrawLayerFlags_e Flags);
  static void           DrawLayerBlend(EGLayerContext *pDrawLayer, EGDrawImage *pImage);
  static void           DrawLayerDestroy(EGLayerContext *pDrawLayer);


  SDL_Renderer          *m_pRenderer;
  static EG_LRU_t       *m_pTextureCache;
  static SDL_Texture    *m_pMask;
  static SDL_Texture    *m_pComposition;
  static bool            m_CompositionCached;
  static SDL_Texture    *m_pTargetBackup;
  static uint8_t         m_TransformCount;

// Utils //
public:
  void                  UtilsInit();
  void                  UtilsDeinit();
  static void           RectToSDLRect(const EGRect *pIn, SDL_Rect *pOut);
  static void           ColorToSDLColor(const EG_Color_t *pIn, SDL_Color *pOut);
  static void           ScaleToSDLRect(const EGRect *pIn, SDL_Rect *pOut, EGScale Scale, const EGPoint *pPivot);
  SDL_Palette*          AllocPaletteForBPP(const uint8_t *pMapping, uint8_t BPP);
  static SDL_Surface*   CreateOPASurface(EG_OPA_t * opa, int32_t Width, int32_t Height, int32_t Step);
  static SDL_Texture*   CreateOPATexture(SDL_Renderer *pRenderer, EG_OPA_t *pPixels, int32_t Width, int32_t Height, int32_t Step);
  static void           SDLTo8BPP(uint8_t *pDest, const uint8_t *pSrce, int Width, int Height, int Step, uint8_t BPP);

// Composite //
public:
  bool                  CompositeBegin(const EGRect *pRectIn, const EGRect *pClipIn, const EGRect *pExtRect, EG_BlendMode_e BlendMode, EGRect *pRectOut, EGRect *pClipOut, EGRect *pApplyRect);
  void                  CompositeEnd(const EGRect *pApplyRect, EG_BlendMode_e BlendMode);
  SDL_Texture*          CompositeGetTexture(EG_SDL_CompositeTextures_e ID, int32_t Width, int32_t Height, bool *pTextureCached);

private:
  CompositeKey_t        MaskKeyCreate(EG_SDL_CompositeTextures_e Type);
  int32_t               NextPowOf2(int32_t Value);
  void                  DumpMasks(SDL_Texture *pTexture, const EGRect *pRect);

// Texture //
public:
  static void           TextureCacheInit(void);
  static void           TextureCacheDeinit(void);
  static SDL_Texture*   TextureCacheGet(const void *pKey, size_t KeyLength, bool *pFound);
  static SDL_Texture*   TextureCacheGetWithExtData(const void *pKey, size_t KeyLength, bool *pFound, void **ppExtData);
  static bool           TextureCachePut(const void *pKey, size_t KeyLength, SDL_Texture *pTexture);
  static bool           TextureCachePutAdvanced(const void *pKey, size_t KeyLength, SDL_Texture *pTexture, void *pExtData, void userdata_free(void *), EG_SDL_CacheFlag_e Flags);
  static EG_CacheKeyHeadImage_t* CreateTextureImageKey(const void *pSrce, int32_t FrameID, size_t *pSize);

private:
  static void            DrawCacheFreeValue(DrawCacheValue_t *pValue);
  static DrawCacheValue_t*  DrawCacheGetEntry(const void *pKey, size_t KeyLength, bool *pFound);

// Stack Blur //
public:
  void                  StackBlurGrayscale(EG_OPA_t *pBuffer, uint16_t Width, uint16_t Height, uint16_t Radius);

private:
  void                  StackBlurJob(EG_OPA_t *pSrce, unsigned int Width, unsigned int Height, unsigned int Radius, int Cores, int Core, int Step);

// Arc //
private:
  void                  ArcDumpMasks(SDL_Texture *pTexture, const EGRect *pRect, const int16_t *pIDs, int16_t IDCount, const int16_t *pCaps);
  void                  GetCapRect(int16_t Angle, int32_t Width, uint16_t Radius, const EGPoint *pCenter, EGRect *pRect);

// Background //
private:
  void                  DrawBackgroundColor(const EGRect *pRect, const EGRect *pFillRect, const EGDrawRect *pDrawRect);
  void                  DrawBackgroundImage(const EGRect *pRect, const EGRect *pFillRect, const EGDrawRect *pDrawRect);

// Rect //
public:
  SDL_Texture*          RectBackgroundGetFrag(int32_t Radius, bool *pInCache);
  SDL_Texture*          RectGradGetFrag(const EG_GradDescriptor_t *pGrad, int32_t Width, int32_t Height, int32_t Radius, bool *pInCache);
  SDL_Texture*          RectGradGetStrip(const EG_GradDescriptor_t *pGrad, bool *pInCache);
  void                  RectBackgroundFragDrawCorners(SDL_Texture *pFrag, int32_t FragSize, const EGRect *pRect, const EGRect *pClipRect, bool Full);

private:
  void                  DrawRectBackColor(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj);
  void                  DrawRectBackGradSimple(const EGRect *pRect, const EGRect *pDrawRect, const EG_GradDescriptor_t *pGrad, bool BlendMod);
  void                  DrawRectBackGradRadius(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj);
  void                  DrawRectBackImage(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj);
  void                  DrawRectBorder(const EGRect *pRect, const EGRect *pDrawRect, const EGDrawRect *pDrawObj);
  void                  DrawRectShadow(const EGRect *pRect, const EGRect *pClipRect, const EGDrawRect *pDrawObj);
  void                  DrawRectOutline(const EGRect *pRect, const EGRect *pClipRect, const EGDrawRect *pDrawObj);
  void                  DrawRectBorderGeneric(const EGRect *pOuterRect, const EGRect *pInnerRect, const EGRect *pClipRect,
                                  int32_t RadiusOut, int32_t RadiusIn, EG_Color_t Color, EG_OPA_t OPA, EG_BlendMode_e BlendMode);
  void                  FragRenderBorders(SDL_Texture *pFrag, int32_t FragSize, const EGRect *pRect, const EGRect *pClipped, bool full);
  void                  FragRenderCenter(SDL_Texture *pFrag, int32_t FragSize, const EGRect *pRect, const EGRect *pClipped, bool full);
  EG_RectBackgroundKey_t CreateRectBackgroundKey(int32_t Radius, int32_t Size);
  EG_RectGradFragKey_t  CreateRectGradFragKey(const EG_GradDescriptor_t *pGrad, int32_t Width, int32_t Height, int32_t Radius);
  EG_RectGradStripKey_t CreateRectGradStripKey(const EG_GradDescriptor_t *pGrad);
  EG_RectShadowKey_t    CreateRectShadowKey(int32_t Radius, int32_t Size, int32_t Blur);
  EG_RectBorderKey_t    CreateRectBorderKey(int32_t RadiusOut, int32_t RadiusIn, const EGRect *pOuterRect, const EGRect *pInnerRect);

// Image //
public:
  bool                  LoadImageTexture(EG_CacheKeyHeadImage_t *pKey, size_t KeySize, const void *pSrce,
                            int32_t FrameID, SDL_Texture **ppTexture, EG_SDL_ImageHeader_t **ppHeader, bool *pInCache);
private:
  SDL_Texture*          UploadImageTexture(ImageDecoderDescriptor_t *pDecoderDSC);
  SDL_Texture*          UploadImageTextureFallback(ImageDecoderDescriptor_t *pDecoderDSC);
  bool                  CheckMaskSimpleRadius(const EGRect *pRect, int32_t *pRadius);
  void                  DrawImageSimple(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader,
                              const EGDrawImage *pDrawImage, const EGRect *pRect, const EGRect *pClip);
  void                  DrawImageRounded(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader,
                              const EGDrawImage *pDrawImage, const EGRect *pRect, const EGRect *pClip, int32_t Radius);
  SDL_Texture*          GetImageRoundedFrag(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader, int Width, int Height, int32_t Radius, bool *pInCache);
  EG_ImageRoundedKey_t  CreateRoundedKey(const SDL_Texture *pTexture, int32_t Width, int32_t Height, int32_t Radius);
  void                  CalcDrawPart(SDL_Texture *pTexture, const EG_SDL_ImageHeader_t *pHeader, const EGRect *pRect,
                            const EGRect *pClip, SDL_Rect *pClippedSrce, SDL_Rect *pClippedDest);
  void                  ApplyRecolorOPA(SDL_Texture *pTexture, const EGDrawImage *pDrawImage);

// Label //
private:
  EG_FontGlyphKey_t     CreateFontGlyphKey(const EG_Font_t *pFont, uint32_t Char);

// Line //
private:
  EG_LineKey_t          CreateLineKey(const EGDrawLine *pDrawLine, int32_t Length);
  SDL_Texture*          CreateLineTexture(const EGDrawLine *pDrawLine, int32_t Length);

// Polygon //
private:
  void                  PolyDumpMasks(SDL_Texture *pTexture, const EGRect *pRect);

// Mask //
public:
  static EG_OPA_t*      MaskDumpOPA(const EGRect *pRect, const int16_t * pIDs, int16_t IDCount);
  SDL_Texture*          MaskDumpTexture(const EGRect *pRect, const int16_t *pIDs, int16_t IDCount);

// Layer //
public:
  void                  TransformAreasOffset(bool HasComposite, EGRect *ApplyRect,  EGRect *pRect, EGRect *pClip) const;

};

#endif
#endif