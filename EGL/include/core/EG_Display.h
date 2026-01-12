/*
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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../hal/EG_HAL.h"
#include "EG_Object.h"
#include "EG_ObjClass.h"
#include "EG_Theme.h"
//#include "misc/EG_Timer.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum : uint8_t {
    EG_SCR_LOAD_ANIM_NONE,
    EG_SCR_LOAD_ANIM_OVER_LEFT,
    EG_SCR_LOAD_ANIM_OVER_RIGHT,
    EG_SCR_LOAD_ANIM_OVER_TOP,
    EG_SCR_LOAD_ANIM_OVER_BOTTOM,
    EG_SCR_LOAD_ANIM_MOVE_LEFT,
    EG_SCR_LOAD_ANIM_MOVE_RIGHT,
    EG_SCR_LOAD_ANIM_MOVE_TOP,
    EG_SCR_LOAD_ANIM_MOVE_BOTTOM,
    EG_SCR_LOAD_ANIM_FADE_IN,
    EG_SCR_LOAD_ANIM_FADE_OUT,
    EG_SCR_LOAD_ANIM_OUT_LEFT,
    EG_SCR_LOAD_ANIM_OUT_RIGHT,
    EG_SCR_LOAD_ANIM_OUT_TOP,
    EG_SCR_LOAD_ANIM_OUT_BOTTOM,
} EG_ScreenAnimateType_t;

inline EG_Coord_t GetHorizontalResolution(void);
inline EG_Coord_t GetVerticalResolution(void);

///////////////////////////////////////////////////////////////////////////////////////////////////

class EGDisplay
{
public:
                      EGDisplay(void);
                      ~EGDisplay(void);

  static void         LoadScreen(EGObject *pScreen);
  static EGObject*    GetActiveScreen(EGDisplay *pDisplay);
  static EGObject*    GetPrevoiusScreen(EGDisplay *pDisplay);
  static EGObject*    GetTopLayer(EGDisplay *pDisplay);
  static EGObject*    GetSystemLayer(EGDisplay *pDisplay);
  static EGTheme*     GetTheme(EGDisplay *pDisplay);
  static void         SetTheme(EGDisplay *pDisplay, EGTheme *pTheme);
  static void         SetBackgroundColor(EGDisplay *pDisplay, EG_Color_t color);
  static void         SetBackgroundImage(EGDisplay *pDisplay, const void  * img_src);
  static void         SetBackgroundOpa(EGDisplay *pDisplay, EG_OPA_t opa);
  static uint32_t     GetInactiveTime(EGDisplay *pDisplay);
  static void         TriggerActivity(EGDisplay *pDisplay);
  static void         CleanDisplayCache(EGDisplay *pDisplay);
  static void         EnableInvalidation(EGDisplay *pDisplay, bool Enable);
  static EGTimer*     GetRefereshTimer(EGDisplay *pDisplay);
  static bool         IsInvalidationEnabled(EGDisplay *pDisplay);
  static void         LoadAnimation(EGObject *pScreen, EG_ScreenAnimateType_t anim_type, uint32_t time, uint32_t delay, bool auto_del);
  static EGDisplay*   GetDisplay(const EGObject *pScreen);

  EGDisplayDriver       *m_pDriver;                            // Driver to the display
  EGTimer               *m_pRefreshTimer;                       // Timer to check the dirty areas and refreshes them
  EGTheme                *m_pTheme;                             // The theme assigned to the screen
  EGObject              **m_pScreens;                           // Array of screen objects.
  EGObject               *m_pActiveScreen;                      // Currently active screen on this display
  EGObject               *m_pPrevoiusScreen;                    // Previous screen. Used during screen animations
  EGObject               *m_ScreenToLoad;                       // The screen prepared to load in LoadAnimation
  EGObject               *m_pTopLayer;                          // @see GetTopLayer
  EGObject               *m_pSystemLayer;                       // @see GetSystemLayer
  uint32_t                m_ScreenCount;
  uint8_t                 m_DrawOverActive : 1;                 // 1: Draw previous screen over active screen
  uint8_t                 m_DeletePrevious : 1;                 // 1: Automatically delete the previous screen when the screen load anim. is ready
  uint8_t                 m_RenderingInProgress : 1;            // 1: The current screen rendering is in progress
  EG_OPA_t                m_BackgroundOPA;                      // Opacity of the background color or wallpaper
  EG_Color_t              m_BackgroundColor;                    // Default display color when screens are transparent
  const void             *m_BackgroundImage;                    // An image source to display as wallpaper
  EGRect                  m_InvalidAreas[EG_INVAL_BUF_SIZE];      // Invalidated (marked to redraw) areas
  uint8_t                 m_InvalidAreasJoined[EG_INVAL_BUF_SIZE]{0};
  uint16_t                m_InvalidCount;
  int32_t                 m_InvalidEnableCount;
  EGList                  m_SyncAreas;                          // Double buffer sync areas 
  uint32_t                m_LastActivityTime;                   // Last time when there was activity on this display

private:
  static void         LoadScreenPrivate(EGObject * scr);
  static void         LoadAnimationStart(EGAnimate *pAnimate);
  static void         OpaScaleAnimation(void *obj, int32_t v);
  static void         SetAnimationX(void *obj, int32_t v);
  static void         SetAnimationY(void *obj, int32_t v);
  static void         AnimationEnd(EGAnimate *pAnimate);
  static bool         IsOutAnimation(EG_ScreenAnimateType_t a);

// Display HAL Section //
public:
  void                UpdateDriver(EGDisplayDriver *pdriver); // Update the driver in run time.
  EG_Coord_t          GetHorizontalRes(void); // Get the horizontal resolution of a display
  EG_Coord_t          GetVerticalRes(void); // Get the vertical resolution of a display
  EG_Coord_t          GetPhysicalHorizontalRes(void); // Get the full / physical horizontal resolution of a display
  EG_Coord_t          GetPhysicalVerticalRes(void); // Get the full / physical vertical resolution of a display
  EG_Coord_t          GetOffsetX(void); // Get the horizontal offset from the full / physical display
  EG_Coord_t          GetOffsetY(void); // Get the vertical offset from the full / physical display
  bool                GetAntialiasing(void); // Get if anti-aliasing is enabled for a display or not
  void                SetRotation(EG_DisplayRotation_t rotation); 
  EG_DisplayRotation_t    GetRotation(void); // Get the current rotation of this display.
  EG_DisplayDrawBuffer_t* GetDrawBuffer(void); // Get the internal buffer of a display
  void                UseGenericSetPixelCB(EGDisplayDriver *pDriver, EG_ImageColorFormat_t ColorFormat);

  static void         InitialiseDriver(EGDisplayDriver *pDriver); // Initialize a display driver with default values.
  static void         InitialiseDrawBuffers(EG_DisplayDrawBuffer_t *pDrawBuffers, void *pBuffer1, void *pBuffer2, uint32_t SizeInPixels); // Initialize a display buffer
  static EGDisplay*   RegisterDriver(EGDisplayDriver *pDriver); // Register an initialized display driver.
  static void         SetDefault(EGDisplay* pdisplay); // Set a default display. The new screens will be created on it by default.
  static void         FlushReady(EGDisplayDriver * pdriver); // Call in the display driver's `flush_cb` function when the flushing is finished
  static bool         FlushIsLast(EGDisplayDriver * pdriver); // Tell if it's the last area of the refreshing process.
  static void         Remove(EGDisplay *pDisplay); // Remove a display
  static EGDisplay*   GetNext(EGDisplay *pDisplay); // Get the next display.
  static EG_Coord_t   GetDPI(const EGDisplay *pDisplay); // Get the DPI of the display
  static EGDisplay*   GetDefault(void); // Get the default display

  static EGList       m_DisplayList;         // Linked list to store the displays
  static EGDisplay    *m_pDefaultDisplay;

private:
  static EG_TreeWalkResult_e  InvalidateLayoutCB(EGObject *pObj, void *pUserData);
  static void                 SetPixelAlpha1CB(EGDisplayDriver *disp_drv, uint8_t *buf, EG_Coord_t buf_w, EG_Coord_t x, EG_Coord_t y,
                                           EG_Color_t color, EG_OPA_t opa);
  static void                 SetPixelAlpha2CB(EGDisplayDriver *disp_drv, uint8_t *buf, EG_Coord_t buf_w, EG_Coord_t x, EG_Coord_t y,
                                           EG_Color_t color, EG_OPA_t opa);
  static void                 SetPixelAlpha4CB(EGDisplayDriver *disp_drv, uint8_t *buf, EG_Coord_t buf_w, EG_Coord_t x, EG_Coord_t y,
                                           EG_Color_t color, EG_OPA_t opa);
  static void                 SetPixelAlpha8CB(EGDisplayDriver *disp_drv, uint8_t *buf, EG_Coord_t buf_w, EG_Coord_t x, EG_Coord_t y,
                                           EG_Color_t color, EG_OPA_t opa);
  static void                 SetPixelAlphaGeneric(EGImageBuffer *pImage, EG_Coord_t x, EG_Coord_t y, EG_Color_t color, EG_OPA_t opa);
  static void                 SetPixelTrueColorAlpha(EGDisplayDriver *disp_drv, uint8_t *buf, EG_Coord_t buf_w,
                                                  EG_Coord_t x, EG_Coord_t y, EG_Color_t color, EG_OPA_t opa);

};

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline EGObject* EGActiveScreen(void)
{
  return EGDisplay::GetActiveScreen(EGDisplay::GetDefault());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline EGObject* EGTopLayer(void)
{
  return EGDisplay::GetTopLayer(EGDisplay::GetDefault());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline EGObject* EGSystemLayer(void)
{
  return EGDisplay::GetSystemLayer(EGDisplay::GetDefault());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline void EGLoadScreen(EGObject *pScreen)
{
  EGDisplay::LoadScreen(pScreen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t GetHorizontalResolution(void)
{
  return EGDisplay::GetDefault()->GetHorizontalRes();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline EG_Coord_t eg_dpx(EG_Coord_t n)
{
  return EG_DPX(n);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t GetVerticalResolution(void)
{
  return EGDisplay::GetDefault()->GetVerticalRes();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t EG_DisplayDPX(const EGDisplay *pDisplay, EG_Coord_t n)
{
  return _EG_DPX_CALC(EGDisplay::GetDPI(pDisplay), n);
}

#ifndef EG_HORZ_RES
#define EG_HORZ_RES EGDisplay::GetDefault()->GetHorizontalRes()
#endif

#ifndef EG_VERT_RES
#define EG_VERT_RES EGDisplay::GetDefault()->GetVerticalRes()
#endif


