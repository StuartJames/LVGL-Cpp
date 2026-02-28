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

 #pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hal/EG_HAL.h"
#include "draw/EG_DrawContext.h"
#include "misc/EG_Color.h"
#include "misc/EG_Rect.h"
#include "misc/EG_Timer.h"

/////////////////////////////////////////////////////////////////////////////

#ifndef EG_INVAL_BUF_SIZE
#define EG_INVAL_BUF_SIZE 32 /*Buffer size for invalid areas*/
#endif

#ifndef EG_ATTRIBUTE_FLUSH_READY
#define EG_ATTRIBUTE_FLUSH_READY
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct _EG_DisplayDrawBuffer_t {
    void  *pBuffer1;                // First display buffer.*/
    void  *pBuffer2;                // Second display buffer.*/
    void  *pActiveBuffer;           // Internal, used by the library*/
    uint32_t Size;                  // In pixel count*/
    volatile int Flushing;          // 1: flushing is in progress. It can't be a bit field because when it's
                                    // cleared from IRQ Read-Modify-Write issue might occur
    volatile int FlushingLast;      // 1: It was the last chunk to flush. It can't be a bit field because when
                                    // it's cleared from IRQ Read-Modify-Write issue might occur.
    volatile uint32_t LastArea : 1; // 1: the last area is being rendered*/
    volatile uint32_t LastPart : 1; // 1: the last part of the current area is being rendered*/
} EG_DisplayDrawBuffer_t;


typedef enum EG_DisplayRotation_t : uint8_t{
    EG_DISP_ROT_NONE = 0,
    EG_DISP_ROT_90,
    EG_DISP_ROT_180,
    EG_DISP_ROT_270
} EG_DisplayRotation_t;

/////////////////////////////////////////////////////////////////////////////

class EGDisplayDriver 
{
public:
                  EGDisplayDriver(void);
                  ~EGDisplayDriver(void);
  virtual void    InitialiseContext(EGDrawContext *pDrawContext);
  virtual void    DeinitialiseContext(EGDrawContext *pDrawContext);

  void            (*FlushCB)(EGDisplayDriver *pDriver, const EGRect *pRect, EG_Color_t *pColor); // * MANDATORY: Writes the internal buffer (draw_buf) to the display. 
  void            (*RounderCB)(EGDisplayDriver *pDriver, EGRect *pRect); /// OPTIONAL: Extend the invalidated areas to match with the display drivers requirements
  void            (*SetPixelCB)(EGDisplayDriver *pDriver, uint8_t *pBuffer, EG_Coord_t Width, EG_Coord_t X, EG_Coord_t Y, 
                                EG_Color_t Color, EG_OPA_t OPA); // OPTIONAL: Set a pixel in a buffer
  void            (*ClearCB)(EGDisplayDriver *pDriver, uint8_t *pBuffer, uint32_t Size); // OPTIONAL: Called after every refresh cycle to tell the rendering and flushing time + the number of flushed pixels
  void            (*MonitorCB)(EGDisplayDriver *pDriver, uint32_t time, uint32_t X);
  void            (*WaitCB)(EGDisplayDriver *pDriver); // OPTIONAL: Called periodically while lvgl waits for operation to be completed. For example flushing or GPU
  void            (*CleanDcacheCB)(EGDisplayDriver *pDriver); // OPTIONAL: Called when lvgl needs any CPU cache that affects rendering to be cleaned
  void            (*UpdateCB)(EGDisplayDriver *pDriver); // OPTIONAL: called when driver parameters are updated 
  void            (*RenderStartCB)(EGDisplayDriver *pDriver);  // OPTIONAL: called when start rendering 
//  void            (*InitialiseContext)(EGDisplayDriver *pDriver, EGDrawContext *pContext);
//  void            (*DinitialiseContext)(EGDisplayDriver *pDriver, EGDrawContext *pContext);

  EG_Coord_t          m_HorizontalRes;          // Horizontal resolution.
  EG_Coord_t          m_VerticalRes;            // Vertical resolution.
  EG_Coord_t          m_PhysicalHorizontalRes;  // Horizontal resolution of the full / physical display. Set to -1 for fullscreen mode.
  EG_Coord_t          m_PhysicalVerticalRes;    // Vertical resolution of the full / physical display. Set to -1 for fullscreen mode.
  EG_Coord_t          m_OffsetX;                // Horizontal offset from the full / physical display. Set to 0 for fullscreen mode.
  EG_Coord_t          m_OffsetY;                // Vertical offset from the full / physical display. Set to 0 for fullscreen mode.
  EG_DisplayDrawBuffer_t *m_pDrawBuffers;           // Pointer to a buffer initialized with `lv_disp_draw_buf_init()`.
  uint32_t            m_DirectMode : 1;         // 1: Use screen-sized buffers and draw to absolute coordinates
  uint32_t            m_FullRefresh : 1;        // 1: Always make the whole screen redrawn
  uint32_t            m_SoftRotate : 1;         // 1: use software rotation (slower)
  uint32_t            m_AntiAliasing : 1;       // 1: anti-aliasing is enabled on this display.
  uint32_t            m_Rotated : 2;            // 1: turn the display by 90 degree. @warning Does not update coordinates for you!
  uint32_t            m_ScreenTransparent : 1;  // Handle if the screen doesn't have a solid (opa == EG_OPA_COVER) background. Use only if required because it's slower.
  uint32_t            m_DPI : 10;               // DPI (dot per inch) of the display. Default value is `EG_DPI_DEF`.
  EG_Color_t          m_ColorChromaKey;         // On CHROMA_KEYED images this color will be transparent. `EG_COLOR_CHROMA_KEY` by default. (EG_Config.h)
  EGDrawContext      *m_pContext;
  size_t              m_ContextSize;
 
#if EG_USE_EXT_DATA
  void               *m_pExtData;              // Custom display driver user data
#endif

};



