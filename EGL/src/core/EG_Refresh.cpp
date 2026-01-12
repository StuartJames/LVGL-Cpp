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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include <stddef.h>
#include "core/EG_Refresh.h"
#include "core/EG_Display.h"
#include "hal/EG_HALTick.h"
#include "hal/EG_HALDisplay.h"
#include "misc/EG_Timer.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Math.h"
#include "misc/lv_gc.h"
#include "draw/EG_DrawContext.h"
#include "font/EG_FontFmtText.h"
#include "extra/others/EG_Snapshot.h"

/////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_PERF_MONITOR || EG_USE_MEM_MONITOR
#include "widgets/EG_Label.h"
#endif

#if EG_LOG_TRACE_DISP_REFR
#define REFR_TRACE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define REFR_TRACE(...)
#endif

/////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_PERF_MONITOR
typedef struct {
	uint32_t LastTime;
	uint32_t ElapsSum;
	uint32_t FrameCount;
	uint32_t SumCountFPS;
	uint32_t SumAllFPS;
#if EG_USE_LABEL
	EGLabel *pLabel;
#endif
} PerformanceMonitor_t;  // Performance monitor

  PerformanceMonitor_t  m_Monitor;
  void InitialiseMonitor(PerformanceMonitor_t *pMonitor);
#endif

#if EG_USE_MEM_MONITOR
typedef struct {
	uint32_t LastTime;
#if EG_USE_LABEL
	EGLabel *pLabel;
#endif
} MemoryMonitor_t;  // Memory monitor

  MemoryMonitor_t       m_Monitor;
  void InitialiseMonitor(MemoryMonitor_t *pMonitor);
#endif

/////////////////////////////////////////////////////////////////////////////////////

void          JoinArea(void);
void          SyncAreas(void);
void          InvalidateAreas(void);
void          RefreshArea(const EGRect *pRect);
void          RefreshAreaPart(EGDrawContext *pContext);
EGObject*     GetTopObject(const EGRect *pRect, EGObject *pObj);
void          ObjectAndChildren(EGDrawContext *pContext, EGObject *pObj);
uint32_t      GetMaximumRow(EGDisplay *pDisplay, EG_Coord_t area_w, EG_Coord_t area_h);
void          RotateDrawBuffer180(EGDisplayDriver *pDriver, EGRect *area, EG_Color_t *color_p);
void          RotateDrawBuffer90(bool Invert, EG_Coord_t Width, EG_Coord_t Height, EG_Color_t *pColor, EG_Color_t *pBuffer);
void          RotateDrawBuffer4(EG_Color_t *a, EG_Color_t *b, EG_Color_t *c, EG_Color_t *d);
void          RotateDrawBuffer90Square(bool Is270, EG_Coord_t Width, EG_Color_t *pColor);
void          RotateDrawBuffer(EGRect *pRect, EG_Color_t *pColor);
void          FlushDrawBuffer(EGDisplay *pDisplay);
void          DoFlushCB(EGDisplayDriver *pDriver, const EGRect *pRect, EG_Color_t *pColor);
EG_Result_t   GetLayerRect(const EGDrawContext *pContext, EGObject *pObj, EG_LayerType_e LayerType, EGRect *pRect);
void          LayerAlphaTest(EGObject *pObj, const EGDrawContext *pContext, EGLayerContext *pLayerContext, EGDrawLayerFlags_e flags);
void          RefreshObject(EGDrawContext *pContext, EGObject *pObj);

static EGDisplay *s_pRefreshDisplay = nullptr;
static uint32_t   s_PixelCount = 0;

/////////////////////////////////////////////////////////////////////////////////////

inline void RotateDrawBuffer4(EG_Color_t *a, EG_Color_t *b, EG_Color_t *c, EG_Color_t *d)
{
	EG_Color_t tmp;
	tmp = *a;
	*a = *b;
	*b = *c;
	*c = *d;
	*d = tmp;
}

/////////////////////////////////////////////////////////////////////////////////////

EGDisplay* GetRefreshingDisplay(void)
{
	return s_pRefreshDisplay;
}

/////////////////////////////////////////////////////////////////////////////////////

void SetRefreshingDisplay(EGDisplay *pDisplay)
{
	s_pRefreshDisplay = pDisplay;
}

/////////////////////////////////////////////////////////////////////////////////////

void RefreshInitialise(void)
{
#if EG_USE_PERF_MONITOR
	InitialiseMonitor(&m_Monitor);
#endif
#if EG_USE_MEM_MONITOR
	InitialiseMonitor(&m_Monitor);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////

void RefreshNow(EGDisplay *pDisplay)
{
	EGAnimate::ReferenceNow();
	if(pDisplay) {
		if(pDisplay->m_pRefreshTimer) RefreshTimerCB(pDisplay->m_pRefreshTimer);
	}
	else {
		EGDisplay *pDisp;
		pDisp = EGDisplay::GetNext(nullptr);
		while(pDisp) {
			if(pDisp->m_pRefreshTimer) RefreshTimerCB(pDisp->m_pRefreshTimer);
			pDisp = EGDisplay::GetNext(pDisp);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void RefreshTimerCB(EGTimer *pTimer)
{
uint32_t start = EG_GetTick();
volatile uint32_t elaps = 0;

	REFR_TRACE("begin");
  if(pTimer) {
		s_pRefreshDisplay = (EGDisplay *)pTimer->m_pParam;
#if EG_USE_PERF_MONITOR == 0 && EG_USE_MEM_MONITOR == 0
		pTimer->Pause();		// Ensure the timer does not run again automatically. This is done before refreshing in case refreshing invalidates something else.
#endif
	}
	else s_pRefreshDisplay = EGDisplay::GetDefault();
	s_pRefreshDisplay->m_pActiveScreen->UpdateLayout();	// Refresh the screen's layout if required
	if(s_pRefreshDisplay->m_pPrevoiusScreen) s_pRefreshDisplay->m_pPrevoiusScreen->UpdateLayout();
	s_pRefreshDisplay->m_pTopLayer->UpdateLayout();
 	s_pRefreshDisplay->m_pSystemLayer->UpdateLayout();
 	if(s_pRefreshDisplay->m_pActiveScreen == nullptr) {	// Do nothing if there is no active screen
		s_pRefreshDisplay->m_InvalidCount = 0;
		EG_LOG_WARN("there is no active screen");
		REFR_TRACE("finished");
		return;
	}
	JoinArea();
	SyncAreas();
	InvalidateAreas();
	if(s_pRefreshDisplay->m_InvalidCount != 0) {	// If refresh happened ...
		// Copy invalid areas for sync next refresh in double buffered direct mode
		EG_LOG_WARN("Refreah happend");
		if(s_pRefreshDisplay->m_pDriver->m_DirectMode && s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pBuffer2) {
			for(uint16_t i = 0; i < s_pRefreshDisplay->m_InvalidCount; i++) {
				if(s_pRefreshDisplay->m_InvalidAreasJoined[i]) continue;
        EGRect *pSyncRect = &s_pRefreshDisplay->m_InvalidAreas[i];
        s_pRefreshDisplay->m_SyncAreas.AddTail(pSyncRect);
			}
		}
		s_pRefreshDisplay->m_InvalidCount = 0;
		elaps = EG_TickElapse(start);
		if(s_pRefreshDisplay->m_pDriver->MonitorCB) {		// Call monitor cb if present
			s_pRefreshDisplay->m_pDriver->MonitorCB(s_pRefreshDisplay->m_pDriver, elaps, s_PixelCount);
		}
	}
	EG_FreeAllBuffers();
	EG_FontCleanUpFmtText();
#if EG_DRAW_COMPLEX
	DrawMaskCleanup();
#endif
#if EG_USE_PERF_MONITOR && EG_USE_LABEL
	EGLabel *pLabel = m_Monitor.pLabel;
	if(pLabel == nullptr) {
		pLabel = new EGLabel(EGSystemLayer());
		pLabel->SetStyleBackOPA(EG_OPA_50, 0);
		pLabel->SetStyleBackColor(EG_ColorBlack(), 0);
		pLabel->SetStyleTextColor(EG_ColorWhite(), 0);
		pLabel->SetStylePadTop(3, 0);
		pLabel->SetStylePadBottom(3, 0);
		pLabel->SetStylePadLeft(3, 0);
		pLabel->SetStylePadRight(3, 0);
		pLabel->SetStyleTextAlign(EG_TEXT_ALIGN_RIGHT, 0);
		pLabel->SetText("?");
		pLabel->Align(EG_USE_PERF_MONITOR_POS, 0, 0);
		m_Monitor.pLabel = pLabel;
	}
	if(EG_TickElapse(m_Monitor.LastTime) < 300) {
		if(s_PixelCount > 5000) {
			m_Monitor.ElapsSum += elaps;
			m_Monitor.FrameCount++;
		}
	}
	else {
		m_Monitor.LastTime = EG_GetTick();
		uint32_t FPSLimit;
		uint32_t FPS;

		if(s_pRefreshDisplay->m_pRefreshTimer) FPSLimit = 1000 / s_pRefreshDisplay->m_pRefreshTimer->m_Period;
		else FPSLimit = 1000 / EG_DISP_DEF_REFR_PERIOD;
		if(m_Monitor.ElapsSum == 0) m_Monitor.ElapsSum = 1;
		if(m_Monitor.FrameCount == 0) FPS = FPSLimit;
		else FPS = (1000 * m_Monitor.FrameCount) / m_Monitor.ElapsSum;
		m_Monitor.ElapsSum = 0;
		m_Monitor.FrameCount = 0;
		if(FPS > FPSLimit) FPS = FPSLimit;
		m_Monitor.SumAllFPS += FPS;
		m_Monitor.SumCountFPS++;
		uint32_t cpu = 100 - EGTimer::GetIdle();
		pLabel->SetFormatText("%" EG_PRIu32 " FPS\n%03" EG_PRIu32 "%% CPU", FPS, cpu);
	}
#endif
#if EG_USE_MEM_MONITOR && EG_MEM_CUSTOM == 0 && EG_USE_LABEL
	EGLabel *pLabel = m_Monitor.pLabel;
	if(pLabel == nullptr) {
		pLabel = new EGLabel(EGSystemLayer());
		pLabel->SetStyleBackOPA(EG_OPA_50, 0);
		pLabel->SetStyleBackColor(EG_ColorBlack(), 0);
		pLabel->SetStyleTextColor(EG_ColorWhite(), 0);
		pLabel->SetStylePadTop(3, 0);
		pLabel->SetStylePadBottom(3, 0);
		pLabel->SetStylePadLeft(3, 0);
		pLabel->SetStylePadRight(3, 0);
		pLabel->SetText("?");
		pLabel->Align(EG_USE_MEM_MONITOR_POS, 0, 0);
		m_Monitor.pLabel = pLabel;
	}
	if(EG_TickElapse(m_Monitor.LastTime) > 300) {
		m_Monitor.LastTime = EG_GetTick();
		EG_MonitorMem_t mon;
		EG_MonitorMem(&mon);
		uint32_t used_size = mon.total_size - mon.free_size;
		;
		uint32_t used_kb = used_size / 1024;
		uint32_t used_kb_tenth = (used_size - (used_kb * 1024)) / 102;
		pLabel->SetFormatText(pLabel,
													"%" EG_PRIu32 ".%" EG_PRIu32 " kB used (%d %%)\n"
													"%d%% frag.",
													used_kb, used_kb_tenth, mon.used_pct,
													mon.frag_pct);
	}
#endif
	REFR_TRACE("finished");
}

/////////////////////////////////////////////////////////////////////////////////////

void JoinArea(void)
{
uint32_t JoinFrom;
uint32_t JoinIn;
EGRect JoinedRect;

  
	EG_ZeroMem(s_pRefreshDisplay->m_InvalidAreasJoined, sizeof(s_pRefreshDisplay->m_InvalidAreasJoined));
	for(JoinIn = 0; JoinIn < s_pRefreshDisplay->m_InvalidCount; JoinIn++) {
		if(s_pRefreshDisplay->m_InvalidAreasJoined[JoinIn] != 0) continue;
		// Check all areas to join them in 'JoinIn'
		for(JoinFrom = 0; JoinFrom < s_pRefreshDisplay->m_InvalidCount; JoinFrom++) {
			// Handle only unjoined areas and ignore itself
			if(s_pRefreshDisplay->m_InvalidAreasJoined[JoinFrom] != 0 || JoinIn == JoinFrom) {
				continue;
			}
			// Check if the areas are on each other
			if(s_pRefreshDisplay->m_InvalidAreas[JoinIn].IsOn(&s_pRefreshDisplay->m_InvalidAreas[JoinFrom]) == false) {
				continue;
			}
			s_pRefreshDisplay->m_InvalidAreas[JoinIn].Join(&JoinedRect, &s_pRefreshDisplay->m_InvalidAreas[JoinFrom]);
			// Join two area only if the joined area Size is smaller
			if(JoinedRect.GetSize() < (s_pRefreshDisplay->m_InvalidAreas[JoinIn].GetSize() +
																 s_pRefreshDisplay->m_InvalidAreas[JoinFrom].GetSize())) {
				s_pRefreshDisplay->m_InvalidAreas[JoinIn] = JoinedRect;
				s_pRefreshDisplay->m_InvalidAreasJoined[JoinFrom] = 1;				// Mark 'JoinFrom' is joined into 'JoinIn'
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void SyncAreas(void)
{
	if(!s_pRefreshDisplay->m_pDriver->m_DirectMode ||	// Do not sync if not direct mode
	    (s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pBuffer2 == nullptr) ||	// Do not sync if not double buffered
	    s_pRefreshDisplay->m_SyncAreas.IsEmpty()) return;	// Do not sync if no sync areas
	// The active buffer is the off screen buffer where we will render
	void *OffScreenBuffer = s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer;
	void *OnScreenBuffer = (s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer == s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pBuffer1) ?
       s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pBuffer2 : s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pBuffer1;
	EG_Coord_t Stride = s_pRefreshDisplay->GetHorizontalRes();	// Get Stride for buffer copy
	EG_LOG_WARN("Buffer %p", OnScreenBuffer);
	EGRect Result[4];	// Iterate through invalidated areas to see if sync area should be copied
	int8_t Difference, j;
	EGRect *pSyncRect, *pNewRect;
  POSITION Pos;
	for(uint32_t i = 0; i < s_pRefreshDisplay->m_InvalidCount; i++) {
		if(s_pRefreshDisplay->m_InvalidAreasJoined[i]) continue;		// Skip joined areas
	  pSyncRect = (EGRect *)s_pRefreshDisplay->m_SyncAreas.GetHead(Pos);
		while(pSyncRect != nullptr) {		// Iterate over sync areas
			// Remove intersect of redraw area from sync area and get remaining areas
			Difference = pSyncRect->Difference(Result, &s_pRefreshDisplay->m_InvalidAreas[i]);
			if(Difference != -1) {			// New sub areas created after removing intersect
				for(j = 0; j < Difference; j++) {				// Replace old sync area with new areas
					pNewRect = &Result[j];
					s_pRefreshDisplay->m_SyncAreas.InsertBefore(Pos, pNewRect);
				}
				s_pRefreshDisplay->m_SyncAreas.RemoveAt(Pos);
				delete pSyncRect;
			}
			pSyncRect = (EGRect *)s_pRefreshDisplay->m_SyncAreas.GetNext(Pos);			// Get next sync area
		}
	}
	// Copy sync areas (if any remaining)
  for(pSyncRect = (EGRect*)s_pRefreshDisplay->m_SyncAreas.GetHead(Pos); pSyncRect != nullptr; pSyncRect = (EGRect*)s_pRefreshDisplay->m_SyncAreas.GetNext(Pos)){
		s_pRefreshDisplay->m_pDriver->m_pContext->CopyBufferProc(OffScreenBuffer, Stride, pSyncRect, OnScreenBuffer, Stride, pSyncRect);
	}
	s_pRefreshDisplay->m_SyncAreas.RemoveAll();	// Clear sync areas
}

/////////////////////////////////////////////////////////////////////////////////////

void InvalidateAreas(void)
{
	s_PixelCount = 0;
	if(s_pRefreshDisplay->m_InvalidCount == 0) return;
	int32_t LastInvalid = 0;
	for(int32_t i = s_pRefreshDisplay->m_InvalidCount - 1; i >= 0; i--) {	// Find the last area which will be drawn
		if(s_pRefreshDisplay->m_InvalidAreasJoined[i] == 0) {
			LastInvalid = i;
			break;
		}
	}
	if(s_pRefreshDisplay->m_pDriver->RenderStartCB) {	// Notify the display driven rendering has started
		s_pRefreshDisplay->m_pDriver->RenderStartCB(s_pRefreshDisplay->m_pDriver);
	}
	s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastArea = 0;
	s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart = 0;
	s_pRefreshDisplay->m_RenderingInProgress = 1;
	for(uint32_t i = 0; i < s_pRefreshDisplay->m_InvalidCount; i++) {
		if(s_pRefreshDisplay->m_InvalidAreasJoined[i] == 0) {		// Refresh the unjoined areas
			if(i == LastInvalid) s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastArea = 1;
			s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart = 0;
		  RefreshArea(&s_pRefreshDisplay->m_InvalidAreas[i]);
			s_PixelCount += s_pRefreshDisplay->m_InvalidAreas[i].GetSize();
		}
	}
	s_pRefreshDisplay->m_RenderingInProgress = 0;
}

/////////////////////////////////////////////////////////////////////////////////////

void RefreshArea(const EGRect *pRect)
{
	EGDrawContext *pContext = s_pRefreshDisplay->m_pDriver->m_pContext;
	pContext->m_pDrawBuffer = s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer;
 	// With full refresh just redraw directly into the buffer
	// In direct mode draw directly on the absolute coordinates of the buffer
	if(s_pRefreshDisplay->m_pDriver->m_FullRefresh || s_pRefreshDisplay->m_pDriver->m_DirectMode) {
		EGRect DisplayRect(0, 0, s_pRefreshDisplay->GetHorizontalRes() - 1, s_pRefreshDisplay->GetVerticalRes() - 1);
		pContext->m_pDrawRect = &DisplayRect;
		if(s_pRefreshDisplay->m_pDriver->m_FullRefresh) {
			s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart = 1;
			pContext->m_pClipRect = &DisplayRect;
			RefreshAreaPart(pContext);
		}
		else {
			s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart = s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastArea;
			pContext->m_pClipRect = pRect;
			RefreshAreaPart(pContext);
		}
		return;
	}
	// Normal refresh: draw the screen in parts. Calculate the max row num
	EG_Coord_t Width = pRect->GetWidth();
	EG_Coord_t Height = pRect->GetHeight();
	EG_Coord_t MaxRow = (pRect->GetY2() >= s_pRefreshDisplay->GetVerticalRes()) ? s_pRefreshDisplay->GetVerticalRes() - 1 : pRect->GetY2();
	int32_t RowIncrement = GetMaximumRow(s_pRefreshDisplay, Width, Height);
	EG_Coord_t Row, EndRow = 0;
	EGRect SubRect;
	for(Row = pRect->GetY1(); Row + RowIncrement - 1 <= MaxRow; Row += RowIncrement) {
		SubRect.Set(pRect->GetX1(), Row, pRect->GetX2(), Row + RowIncrement - 1);		// Calc. the next y coordinates of draw_buf
  	pContext->m_pDrawRect = &SubRect;
		pContext->m_pClipRect = &SubRect;
	  pContext->m_pDrawBuffer = s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer;
		if(SubRect.GetY2() > MaxRow) SubRect.SetY2(MaxRow);
		EndRow = SubRect.GetY2();
		if(EndRow == MaxRow) s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart = 1;
		RefreshAreaPart(pContext);
	}
	if(EndRow != MaxRow) {	                                            // If the last y coordinates are not handled yet ...
  	SubRect.Set(pRect->GetX1(), Row, pRect->GetX2(), MaxRow);		      // Calc. the next y coordinates of draw_buf
		pContext->m_pDrawRect = &SubRect;
		pContext->m_pClipRect = &SubRect;
		pContext->m_pDrawBuffer = s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer;
  	s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart = 1;
		RefreshAreaPart(pContext);
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void RefreshAreaPart(EGDrawContext *pContext)
{
	EG_DisplayDrawBuffer_t *pDrawBuffer = s_pRefreshDisplay->GetDrawBuffer();
	if(pContext->InitBuffer) pContext->InitBuffer(pContext);
	// Below the `pRect` area will be redrawn into the draw buffer. In single buffered mode wait here until the buffer is freed.
  // In full double buffered mode wait here while the buffers are swapped and a buffer becomes available
	bool full_sized = (pDrawBuffer->Size == (uint32_t)s_pRefreshDisplay->m_pDriver->m_HorizontalRes * s_pRefreshDisplay->m_pDriver->m_VerticalRes);
	if((pDrawBuffer->pBuffer1 && !pDrawBuffer->pBuffer2) || (pDrawBuffer->pBuffer1 && pDrawBuffer->pBuffer2 && full_sized)) {
		while(pDrawBuffer->Flushing) {
			if(s_pRefreshDisplay->m_pDriver->WaitCB) s_pRefreshDisplay->m_pDriver->WaitCB(s_pRefreshDisplay->m_pDriver);
		}
// If the screen is transparent initialize it when the flushing is ready
#if EG_COLOR_SCREEN_TRANSP
		if(s_pRefreshDisplay->m_pDriver->screen_transp) {
			if(s_pRefreshDisplay->m_pDriver->clear_cb) {
				s_pRefreshDisplay->m_pDriver->clear_cb(s_pRefreshDisplay->m_pDriver, s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer, s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->Size);
			}
			else {
				EG_ZeroMem(s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->pActiveBuffer, s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->Size * EG_IMG_PX_SIZE_ALPHA_BYTE);
			}
		}
#endif
	}
	EGObject *pTopActiveScreen = nullptr;
	EGObject *pTopPreviousScreen = nullptr;
	// Get the most top object which is not covered by others
	pTopActiveScreen = GetTopObject(pContext->m_pDrawRect, EGDisplay::GetActiveScreen(s_pRefreshDisplay));
	if(s_pRefreshDisplay->m_pPrevoiusScreen) {
		pTopPreviousScreen = GetTopObject(pContext->m_pDrawRect, s_pRefreshDisplay->m_pPrevoiusScreen);
	}
	// Draw a display background if there is no top object
	if(pTopActiveScreen == nullptr && pTopPreviousScreen == nullptr) {
		EGRect Rect(0, 0,	s_pRefreshDisplay->GetHorizontalRes() - 1, s_pRefreshDisplay->GetVerticalRes() - 1);
		if(pContext->DrawBackgroundProc) {
			EGDrawRect DrawRect;
			DrawRect.m_pBackImageSource = s_pRefreshDisplay->m_BackgroundImage;
			DrawRect.m_BackImageOPA = s_pRefreshDisplay->m_BackgroundOPA;
			DrawRect.m_BackgroundColor = s_pRefreshDisplay->m_BackgroundColor;
			DrawRect.m_BackgroundOPA = s_pRefreshDisplay->m_BackgroundOPA;
			pContext->DrawBackgroundProc(&DrawRect, &Rect);
		}
		else if(s_pRefreshDisplay->m_BackgroundImage) {
			EG_ImageHeader_t header;
			EG_Result_t Result = EGImageDecoder::GetInfo(s_pRefreshDisplay->m_BackgroundImage, &header);
			if(Result == EG_RES_OK) {
				EGDrawImage DrawImage;
				DrawImage.m_OPA = s_pRefreshDisplay->m_BackgroundOPA;
				DrawImage.Draw(pContext, &Rect, s_pRefreshDisplay->m_BackgroundImage);
			}
			else EG_LOG_WARN("Can't draw the background image");
		}
		else {
			EGDrawRect DrawRect;
			DrawRect.m_BackgroundColor = s_pRefreshDisplay->m_BackgroundColor;
			DrawRect.m_BackImageOPA = s_pRefreshDisplay->m_BackgroundOPA;
			DrawRect.Draw(pContext, pContext->m_pDrawRect);
		}
	}
	if(s_pRefreshDisplay->m_DrawOverActive) {
		if(pTopActiveScreen == nullptr) pTopActiveScreen = s_pRefreshDisplay->m_pActiveScreen;
		ObjectAndChildren(pContext, pTopActiveScreen);
		if(s_pRefreshDisplay->m_pPrevoiusScreen) {		// Refresh the previous screen if any
			if(pTopPreviousScreen == nullptr) pTopPreviousScreen = s_pRefreshDisplay->m_pPrevoiusScreen;
			ObjectAndChildren(pContext, pTopPreviousScreen);
		}
	}
	else {		// Refresh the previous screen if any
		if(s_pRefreshDisplay->m_pPrevoiusScreen) {
			if(pTopPreviousScreen == nullptr) pTopPreviousScreen = s_pRefreshDisplay->m_pPrevoiusScreen;
			ObjectAndChildren(pContext, pTopPreviousScreen);
		}
		if(pTopActiveScreen == nullptr) pTopActiveScreen = s_pRefreshDisplay->m_pActiveScreen;
		ObjectAndChildren(pContext, pTopActiveScreen);
	}
	// Also refresh top and sys layer unconditionally
	ObjectAndChildren(pContext, EGDisplay::GetTopLayer(s_pRefreshDisplay));
	ObjectAndChildren(pContext, EGDisplay::GetSystemLayer(s_pRefreshDisplay));
	FlushDrawBuffer(s_pRefreshDisplay);
}

/////////////////////////////////////////////////////////////////////////////////////

EGObject* GetTopObject(const EGRect *pRect, EGObject *pObj)
{
EGObject *pFound = nullptr;

	if(pRect->IsInside(&pObj->m_Rect, 0) == false) return nullptr;
	if(pObj->HasFlagSet(EG_OBJ_FLAG_HIDDEN)) return nullptr;
	if(pObj->GetLayerType() != EG_LAYER_TYPE_NONE) return nullptr;
	EG_CoverCheckInfo_t Info;	// If this object fully covers the draw area then check the children too
	Info.Result = EG_COVER_RES_COVER;
	Info.pRect = pRect;
	EGEvent::EventSend(pObj, EG_EVENT_COVER_CHECK, &Info);
	if(Info.Result == EG_COVER_RES_MASKED) return nullptr;
  int32_t ChildCount = pObj->GetChildCount();
	for(int32_t i = ChildCount - 1; i >= 0; i--) {
		EGObject *pChild = pObj->m_pAttributes->ppChildren[i];
		pFound = GetTopObject(pRect, pChild);
		if(pFound != nullptr) break;	// If a children is ok then break
	}
	if(pFound == nullptr && Info.Result == EG_COVER_RES_COVER) {	// If no better children use this object
		pFound = pObj;
	}
	return pFound;
}

/////////////////////////////////////////////////////////////////////////////////////

void ObjectAndChildren(EGDrawContext *pContext, EGObject *pTopObj)
{
	// Normally always will be a pTopObj (at least the screen) but in special cases 
  // (e.g. if the screen has alpha) it won't. In this case use the screen directly
	if(pTopObj == nullptr) pTopObj = EGDisplay::GetActiveScreen(s_pRefreshDisplay);
	if(pTopObj == nullptr) return;      // Shouldn't happen
	RefreshObject(pContext, pTopObj);	  // Refresh the top object and its children
	EGObject *pBorder = pTopObj;
	EGObject *pParent = pTopObj->GetParent(); // Draw the 'younger' sibling objects because they can be on pTopObj
	while(pParent != nullptr) {	        // Do until we reach the screen
		bool Go = false;
		uint32_t ChildCount = pParent->GetChildCount();
		for(uint32_t i = 0; i < ChildCount; i++) {
			EGObject *pChild = pParent->m_pAttributes->ppChildren[i];
			if(!Go){
        if(pChild == pBorder) Go = true;
      }
			else RefreshObject(pContext, pChild);				// Refresh the objects
		}
		// Call the post draw function of the parents of the object
		EGEvent::EventSend(pParent, EG_EVENT_DRAW_POST_BEGIN, (void *)pContext);
		EGEvent::EventSend(pParent, EG_EVENT_DRAW_POST, (void *)pContext);
		EGEvent::EventSend(pParent, EG_EVENT_DRAW_POST_END, (void *)pContext);
		pBorder = pParent;	// The new border will be the last parents, so the 'younger' brothers of pParent will be refreshed
		pParent = pParent->GetParent();	// Go a level deeper
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void RefreshObject(EGDrawContext *pContext, EGObject *pObj)
{
	// Do not refresh hidden objects
	if(pObj->HasFlagSet(EG_OBJ_FLAG_HIDDEN)) return;
	EG_LayerType_e LayerType = pObj->GetLayerType();
	if(LayerType == EG_LAYER_TYPE_NONE) RedrawObject(pContext, pObj);
	else {
		EG_OPA_t OPA = pObj->GetStyleOPALayered(0);
		if(OPA < EG_OPA_MIN) return;
		EGRect LayerFullRect;
		if(GetLayerRect(pContext, pObj, LayerType, &LayerFullRect) != EG_RES_OK) return;
		EGDrawLayerFlags_e Flags = EG_DRAW_LAYER_FLAG_HAS_ALPHA;
		if(LayerFullRect.IsInside(&pObj->m_Rect, 0)) {
			EG_CoverCheckInfo_t Info;
			Info.Result = EG_COVER_RES_COVER;
			Info.pRect = &LayerFullRect;
			EGEvent::EventSend(pObj, EG_EVENT_COVER_CHECK, &Info);
			if(Info.Result == EG_COVER_RES_COVER) Flags = (EGDrawLayerFlags_e)(Flags & ~EG_DRAW_LAYER_FLAG_HAS_ALPHA);
		}
		if(LayerType == EG_LAYER_TYPE_SIMPLE) Flags = (EGDrawLayerFlags_e)(Flags | EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE);
		EGLayerContext *pDrawLayer = EGLayerContext::Create(pContext, &LayerFullRect, Flags);
		if(pDrawLayer == nullptr) {
			EG_LOG_WARN("Couldn't create a new layer context");
			return;
		}
		EGPoint Pivot(pObj->GetStyleTransformPivotX(0), pObj->GetStyleTransformPivotY(0));
		if(EG_COORD_IS_PCT(Pivot.m_X)) {
			Pivot.m_X = (EG_COORD_GET_PCT(Pivot.m_X) * pObj->m_Rect.GetWidth()) / 100;
		}
		if(EG_COORD_IS_PCT(Pivot.m_Y)) {
			Pivot.m_Y = (EG_COORD_GET_PCT(Pivot.m_Y) * pObj->m_Rect.GetHeight()) / 100;
		}
		EGDrawImage DrawImage;
		DrawImage.m_OPA = OPA;
		DrawImage.m_Angle = pObj->GetStyleTransformAngle(0);
		if(DrawImage.m_Angle > 3600) DrawImage.m_Angle -= 3600;
		else if(DrawImage.m_Angle < 0) DrawImage.m_Angle += 3600;
		DrawImage.m_Zoom = pObj->GetStyleTransformZoom(0);
		DrawImage.m_BlendMode = pObj->GetStyleBlendMode(0);
		DrawImage.m_Antialias = s_pRefreshDisplay->m_pDriver->m_Antialiasing;
		if(Flags & EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE) {
			pDrawLayer->m_ActiveRect = pDrawLayer->m_FullRect;
			pDrawLayer->m_ActiveRect.SetY2(pDrawLayer->m_ActiveRect.GetY1() + pDrawLayer->m_MaxRowWithoutAlpha - 1);
			if(pDrawLayer->m_ActiveRect.GetY2() > pDrawLayer->m_FullRect.GetY2()) pDrawLayer->m_ActiveRect.SetY2(pDrawLayer->m_FullRect.GetY2());
		}
		while(pDrawLayer->m_ActiveRect.GetY1() <= LayerFullRect.GetY2()) {
		  if(Flags & EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE) {
				LayerAlphaTest(pObj, pContext, pDrawLayer, Flags);
			}
			RedrawObject(pContext, pObj);
			DrawImage.m_Pivot.m_X = pObj->m_Rect.GetX1() + Pivot.m_X - pContext->m_pDrawRect->GetX1();
			DrawImage.m_Pivot.m_Y = pObj->m_Rect.GetY1() + Pivot.m_Y - pContext->m_pDrawRect->GetY1();
			pDrawLayer->Blend(&DrawImage);	// With EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE it should also do the next chunk
			if((Flags & EG_DRAW_LAYER_FLAG_CAN_SUBDIVIDE) == 0) break;
			pDrawLayer->m_ActiveRect.SetY1(pDrawLayer->m_ActiveRect.GetY2() + 1);
			pDrawLayer->m_ActiveRect.SetY2(pDrawLayer->m_ActiveRect.GetY1() + pDrawLayer->m_MaxRowWithoutAlpha - 1);
		}
    delete pDrawLayer;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void RedrawObject(EGDrawContext *pContext, EGObject *pObj)
{
EGRect ObjClipRect;

	const EGRect *pOriginalClipRect = pContext->m_pClipRect;
	EGRect ObjRectExt(pObj->m_Rect);	// Get the object coordinates
	EG_Coord_t ExtDrawSize = pObj->GetExtDrawSize();// Truncate the clip area to `pObj Size + ext Size`
	ObjRectExt.Inflate(ExtDrawSize, ExtDrawSize);
	bool CommonClip = ObjClipRect.Intersect(pOriginalClipRect, &ObjRectExt);
	// If the object is visible on the current clip area OR has overflow visible draw it.
  // With overflow visible drawing should happen to apply the masks which might affect children 
	bool ShouldDraw = (CommonClip || pObj->HasFlagSet(EG_OBJ_FLAG_OVERFLOW_VISIBLE));
	if(ShouldDraw) {
		pContext->m_pClipRect = &ObjClipRect;
		EGEvent::EventSend(pObj, EG_EVENT_DRAW_MAIN_BEGIN, pContext);
		EGEvent::EventSend(pObj, EG_EVENT_DRAW_MAIN, pContext);
		EGEvent::EventSend(pObj, EG_EVENT_DRAW_MAIN_END, pContext);
#if EG_USE_REFR_DEBUG
		EG_Color_t DebugColor = EG_MixColor(EG_Rand(0, 0xFF), EG_Rand(0, 0xFF), EG_Rand(0, 0xFF));
		EGDrawRect DrawRect;
		lv_draw_rect_dsc_init(&DrawRect);
		DrawRect.bg_color.full = DebugColor.full;
		DrawRect.bg_opa = EG_OPA_20;
		DrawRect.border_width = 1;
		DrawRect.border_opa = EG_OPA_30;
		DrawRect.border_color = DebugColor;
		DrawRect.Draw(pContext, &ObjRectExt);
#endif
	}
	// With overflow visible keep the previous clip area to let the children visible out of this object too
  // With not overflow visible limit the clip are to the object's coordinates to clip the children
	EGRect ChildClipRect;
	bool RefreshChildren = true;
	if(pObj->HasFlagSet(EG_OBJ_FLAG_OVERFLOW_VISIBLE)) ChildClipRect = *pOriginalClipRect;
	else{
    if(!ChildClipRect.Intersect(pOriginalClipRect, &pObj->m_Rect)) {
		  RefreshChildren = false;
    }
	}
	if(RefreshChildren) {
		pContext->m_pClipRect = &ChildClipRect;
		uint32_t ChildCount = pObj->GetChildCount();
		for(uint32_t i = 0; i < ChildCount; i++) {
			EGObject *pChild = pObj->m_pAttributes->ppChildren[i];
			RefreshObject(pContext, pChild);
		}
	}
	if(ShouldDraw) {	// If the object was visible on the clip area call the post draw events too
		pContext->m_pClipRect = &ObjClipRect;
		EGEvent::EventSend(pObj, EG_EVENT_DRAW_POST_BEGIN, pContext);	// If all the children are redrawn make 'post draw' draw
		EGEvent::EventSend(pObj, EG_EVENT_DRAW_POST, pContext);
		EGEvent::EventSend(pObj, EG_EVENT_DRAW_POST_END, pContext);
	}
	pContext->m_pClipRect = pOriginalClipRect;
}

/////////////////////////////////////////////////////////////////////////////////////

void InvalidateRect(EGDisplay *pDisplay, const EGRect *pRect)
{
	if(!pDisplay) pDisplay = EGDisplay::GetDefault();
	if(!pDisplay) return;
	if(!EGDisplay::IsInvalidationEnabled(pDisplay)) return;
	if(pDisplay->m_RenderingInProgress) {
		EG_LOG_ERROR("detected modifying dirty areas in render");
		return;
	}
	if(pRect == nullptr) {	// Clear the invalidate buffer if the parameter is nullptr
		pDisplay->m_InvalidCount = 0;
		return;
	}
	EGRect ScreenRect(0, 0, pDisplay->GetHorizontalRes() - 1, pDisplay->GetVerticalRes() - 1);
	EGRect CommonRect;
	bool OnScreen;
	OnScreen = CommonRect.Intersect(pRect, &ScreenRect);
	if(OnScreen == false) return; // Out of the screen
	if(pDisplay->m_pDriver->m_FullRefresh) {// If there were at least 1 invalid area in full refresh mode, redraw the whole screen
		pDisplay->m_InvalidAreas[0] = ScreenRect;
		pDisplay->m_InvalidCount = 1;
		if(pDisplay->m_pRefreshTimer) pDisplay->m_pRefreshTimer->Resume();
		return;
	}
	if(pDisplay->m_pDriver->RounderCB) pDisplay->m_pDriver->RounderCB(pDisplay->m_pDriver, &CommonRect);
	for(uint16_t i = 0; i < pDisplay->m_InvalidCount; i++) {	// Save only if this area is not in one of the saved areas
		if(CommonRect.IsInside(&pDisplay->m_InvalidAreas[i], 0) == true) return;
	}
	if(pDisplay->m_InvalidCount < EG_INVAL_BUF_SIZE) {	// Save the area
		pDisplay->m_InvalidAreas[pDisplay->m_InvalidCount] = CommonRect;
	}
	else { // If no place for the area invalidate the whole screen
		pDisplay->m_InvalidCount = 0;
		pDisplay->m_InvalidAreas[pDisplay->m_InvalidCount] = ScreenRect;
	}
	pDisplay->m_InvalidCount++;
	if(pDisplay->m_pRefreshTimer) pDisplay->m_pRefreshTimer->Resume();
}

/////////////////////////////////////////////////////////////////////////////////////

EG_Result_t GetLayerRect(const EGDrawContext *pContext, EGObject *pObj, EG_LayerType_e LayerType, EGRect *pLayerRect)
{
	EG_Coord_t ExtDrawSize = pObj->GetExtDrawSize();
	EGRect ObjRectExt(pObj->m_Rect);
	ObjRectExt.Inflate(ExtDrawSize, ExtDrawSize);
	if(LayerType == EG_LAYER_TYPE_TRANSFORM) {
		// Get the transformed area and clip it to the current clip area. This area needs to be updated on the screen.
		EGRect ObjClipRect;
		EGRect TransformRect(ObjRectExt);
		pObj->GetTransformedArea(&TransformRect, false, false);
		if(!ObjClipRect.Intersect(pContext->m_pClipRect, &TransformRect)) return EG_RES_INVALID;
		// Transform back (inverse) the transformed area. It will tell which area of the non-transformed
    // widget needs to be redrawn in order to cover transformed area after transformation.
		EGRect InverseObjClipRect;
		pObj->GetTransformedArea(&InverseObjClipRect, false, true);
		if(!InverseObjClipRect.Intersect(&ObjClipRect, &ObjRectExt)) return EG_RES_INVALID;
		*pLayerRect = InverseObjClipRect;
	}
	else if(LayerType == EG_LAYER_TYPE_SIMPLE) {
		EGRect ObjClipRect2;
		if(!ObjClipRect2.Intersect(pContext->m_pClipRect, &ObjRectExt)) {		return EG_RES_INVALID;
		}
		*pLayerRect = ObjClipRect2;
	}
	else {
		EG_LOG_WARN("Unhandled intermediate layer type");
		return EG_RES_INVALID;
	}
	return EG_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////

void LayerAlphaTest(EGObject *pObj, const EGDrawContext *pContext, EGLayerContext *pDrawLayer, EGDrawLayerFlags_e Flags)
{
bool HasAlpha = false;

	// If globally the layer has alpha maybe this smaller section has not (e.g. not on a rounded corner)
  // If turns out that this section has no alpha renderer can choose faster algorithms 
	if(Flags & EG_DRAW_LAYER_FLAG_HAS_ALPHA) {
		// Test for alpha by assuming there is no alpha. If it fails, fall back to rendering with alpha
		HasAlpha = true;
		if(pDrawLayer->m_ActiveRect.IsInside(&pObj->m_Rect, 0)) {
			EG_CoverCheckInfo_t Info;
			Info.Result = EG_COVER_RES_COVER;
			Info.pRect = &pDrawLayer->m_ActiveRect;
			EGEvent::EventSend(pObj, EG_EVENT_COVER_CHECK, &Info);
			if(Info.Result == EG_COVER_RES_COVER) HasAlpha = false;
		}
		if(HasAlpha) {
			pDrawLayer->m_ActiveRect.SetY2(pDrawLayer->m_ActiveRect.GetY1() + pDrawLayer->m_MaxRowWithAlpha - 1);
		}
	}
	if(pDrawLayer->m_ActiveRect.GetY2() > pDrawLayer->m_FullRect.GetY2()) pDrawLayer->m_ActiveRect.SetY2(pDrawLayer->m_FullRect.GetY2());
	pDrawLayer->Adjust(HasAlpha ? EG_DRAW_LAYER_FLAG_HAS_ALPHA : EG_DRAW_LAYER_FLAG_NONE);
}

/////////////////////////////////////////////////////////////////////////////////////

uint32_t GetMaximumRow(EGDisplay *pDisplay, EG_Coord_t AreaWidth, EG_Coord_t AreaHeight)
{
	int32_t RowIncrement = (uint32_t)pDisplay->m_pDriver->m_pDrawBuffers->Size / AreaWidth;
	if(RowIncrement > AreaHeight) RowIncrement = AreaHeight;

	// Round down the lines of draw_buf if rounding is added
	if(s_pRefreshDisplay->m_pDriver->RounderCB) {
		EGRect Rect;
		EG_Coord_t TempHeight = RowIncrement;
		do {
			Rect.SetY2(TempHeight - 1);
			s_pRefreshDisplay->m_pDriver->RounderCB(s_pRefreshDisplay->m_pDriver, &Rect);
			if(Rect.GetHeight() <= RowIncrement) break;			// If this height fits into `RowIncrement` then fine
			TempHeight--;		// Decrement the height of the area until it fits into `RowIncrement` after rounding
		} while(TempHeight > 0);
		if(TempHeight <= 0) {
			EG_LOG_WARN("Can't set draw_buf height using the round function. (Wrong round_cb or to small draw_buf)");
			return 0;
		}
		else RowIncrement = Rect.GetY2() + 1;
	}
	return RowIncrement;
}

/////////////////////////////////////////////////////////////////////////////////////

void RotateDrawBuffer180(EGDisplayDriver *pDriver, EGRect *pRect, EG_Color_t *pColor)
{
	EG_Coord_t AreaWidth = pRect->GetWidth();
	EG_Coord_t AreaHeight = pRect->GetHeight();
	uint32_t total = AreaWidth * AreaHeight;
	// Swap the beginning and end values
	EG_Color_t tmp;
	uint32_t i = total - 1, j = 0;
	while(i > j) {
		tmp = pColor[i];
		pColor[i] = pColor[j];
		pColor[j] = tmp;
		i--;
		j++;
	}
	EG_Coord_t tmp_coord;
	tmp_coord = pRect->GetY2();
	pRect->SetY2(pDriver->m_VerticalRes - pRect->GetY1() - 1);
	pRect->SetY1(pDriver->m_VerticalRes - tmp_coord - 1);
	tmp_coord = pRect->GetX2();
	pRect->SetX2(pDriver->m_HorizontalRes - pRect->GetX1() - 1);
	pRect->SetX1(pDriver->m_HorizontalRes - tmp_coord - 1);
}

/////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM RotateDrawBuffer90(bool Rotation, EG_Coord_t Width, EG_Coord_t Height, EG_Color_t *pColor, EG_Color_t *pBuffer)
{
	uint32_t Invert = (Width * Height) - 1;
	uint32_t Initial = ((Width - 1) * Height);
	for(EG_Coord_t y = 0; y < Height; y++) {
		uint32_t i = Initial + y;
		if(Rotation) i = Invert - i;
		for(EG_Coord_t x = 0; x < Width; x++) {
			pBuffer[i] = *(pColor++);
			if(Rotation) i += Height;
			else i -= Height;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void RotateDrawBuffer90Square(bool Is270, EG_Coord_t Width, EG_Color_t *pColor)
{
	for(EG_Coord_t i = 0; i < Width / 2; i++) {
		for(EG_Coord_t j = 0; j < (Width + 1) / 2; j++) {
			EG_Coord_t inv_i = (Width - 1) - i;
			EG_Coord_t inv_j = (Width - 1) - j;
			if(Is270) {
				RotateDrawBuffer4(&pColor[i * Width + j], &pColor[inv_j * Width + i],
					&pColor[inv_i * Width + inv_j], &pColor[j * Width + inv_i]);
			}
			else {
				RotateDrawBuffer4(&pColor[i * Width + j], &pColor[j * Width + inv_i],
					&pColor[inv_i * Width + inv_j], &pColor[inv_j * Width + i]);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void RotateDrawBuffer(EGRect *pRect, EG_Color_t *pColor)
{
	EGDisplayDriver *pDriver = s_pRefreshDisplay->m_pDriver;
	if(pDriver->m_FullRefresh && pDriver->m_SoftRotate) {
		EG_LOG_ERROR("Cannot rotate a full refreshed display!");
		return;
	}
	if(pDriver->m_Rotated == EG_DISP_ROT_180) {
		RotateDrawBuffer180(pDriver, pRect, pColor);
		DoFlushCB(pDriver, pRect, pColor);
	}
	else if(pDriver->m_Rotated == EG_DISP_ROT_90 || pDriver->m_Rotated == EG_DISP_ROT_270) {
		// Allocate a temporary buffer to store rotated image
		EG_Color_t *pRowBuffer = nullptr;
		EG_DisplayDrawBuffer_t *pDrawBuffer = s_pRefreshDisplay->GetDrawBuffer();
		EG_Coord_t AreaWidth = pRect->GetWidth();
		EG_Coord_t AreaHeight = pRect->GetHeight();
		// Determine the maximum number of rows that can be rotated at a time
		EG_Coord_t RowIncrement = EG_MIN((EG_Coord_t)((EG_DISP_ROT_MAX_BUF / sizeof(EG_Color_t)) / AreaWidth), AreaHeight);
		EG_Coord_t OffsetY = pRect->GetY1();
		if(pDriver->m_Rotated == EG_DISP_ROT_90) {
			pRect->SetY2(pDriver->m_VerticalRes - pRect->GetX1() - 1);
			pRect->SetY1(pRect->GetY2() - AreaWidth + 1);
		}
		else {
			pRect->SetY1(pRect->GetX1());
			pRect->SetY2(pRect->GetY1() + AreaWidth - 1);
		}
		EG_Coord_t Row = 0;		// Rotate the screen in chunks, Flushing after each one
		while(Row < AreaHeight) {
			EG_Coord_t Height = EG_MIN(RowIncrement, AreaHeight - Row);
			pDrawBuffer->Flushing = 1;
			if((Row == 0) && (AreaHeight >= AreaWidth)) {
				Height = AreaWidth;				// Rotate the initial area as a square
				RotateDrawBuffer90Square(pDriver->m_Rotated == EG_DISP_ROT_270, AreaWidth, pColor);
				if(pDriver->m_Rotated == EG_DISP_ROT_90) {
					pRect->SetX1(OffsetY);
					pRect->SetX2(OffsetY + AreaWidth - 1);
				}
				else {
					pRect->SetX2(pDriver->m_HorizontalRes - 1 - OffsetY);
					pRect->SetX1(pRect->GetX2() - AreaWidth + 1);
				}
			}
			else {			// Rotate other areas using a maximum buffer Size
				if(pRowBuffer == nullptr) pRowBuffer = (EG_Color_t *)EG_GetBufferMem(EG_DISP_ROT_MAX_BUF);
				RotateDrawBuffer90(pDriver->m_Rotated == EG_DISP_ROT_270, AreaWidth, Height, pColor, pRowBuffer);

				if(pDriver->m_Rotated == EG_DISP_ROT_90) {
					pRect->SetX1(OffsetY + Row);
					pRect->SetX2(OffsetY + Row + Height - 1);
				}
				else {
					pRect->SetX2(pDriver->m_HorizontalRes - 1 - OffsetY - Row);
					pRect->SetX1(pRect->GetX2() - Height + 1);
				}
			}
			/*  The original part (chunk of the current area) were split into more parts here.
             * Set the original LastPart flag on the last part of rotation. */
			if(Row + Height >= AreaHeight && pDrawBuffer->LastArea && pDrawBuffer->LastPart) {
				pDrawBuffer->FlushingLast = 1;
			}
			else {
				pDrawBuffer->FlushingLast = 0;
			}
			DoFlushCB(pDriver, pRect, pRowBuffer == nullptr ? pColor : pRowBuffer);			// Flush the completed area to the display
			// FIXME: Rotation forces legacy behavior where rendering and Flushing are done serially
			while(pDrawBuffer->Flushing) {
				if(pDriver->WaitCB) pDriver->WaitCB(pDriver);
			}
			pColor += AreaWidth * Height;
			Row += Height;
		}
		if(pRowBuffer != nullptr) EG_ReleaseBufferMem(pRowBuffer);
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void FlushDrawBuffer(EGDisplay *pDisplay)
{
	EG_DisplayDrawBuffer_t *pDrawBuffer = s_pRefreshDisplay->GetDrawBuffer();
	EGDrawContext *pContext = pDisplay->m_pDriver->m_pContext;	// Flush the rendered content to the display
  pContext->WaitForFinish();
	/* In partial double buffered mode wait until the other buffer is freed
     * and driver is ready to receive the new buffer */
	bool full_sized = pDrawBuffer->Size == (uint32_t)s_pRefreshDisplay->m_pDriver->m_HorizontalRes * s_pRefreshDisplay->m_pDriver->m_VerticalRes;
	if(pDrawBuffer->pBuffer1 && pDrawBuffer->pBuffer2 && !full_sized) {
		while(pDrawBuffer->Flushing) {
			if(s_pRefreshDisplay->m_pDriver->WaitCB) s_pRefreshDisplay->m_pDriver->WaitCB(s_pRefreshDisplay->m_pDriver);
		}
	}
	pDrawBuffer->Flushing = 1;
	if(s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastArea && s_pRefreshDisplay->m_pDriver->m_pDrawBuffers->LastPart) pDrawBuffer->FlushingLast = 1;
	else pDrawBuffer->FlushingLast = 0;
	bool LastFlush = pDrawBuffer->FlushingLast;
	if(pDisplay->m_pDriver->FlushCB) {
		// Rotate the buffer to the display's native orientation if necessary
		if(pDisplay->m_pDriver->m_Rotated != EG_DISP_ROT_NONE && pDisplay->m_pDriver->m_SoftRotate) {
			RotateDrawBuffer(pContext->m_pDrawRect, (EG_Color_t *)pContext->m_pDrawBuffer);
		}
		else DoFlushCB(pDisplay->m_pDriver, pContext->m_pDrawRect, (EG_Color_t *)pContext->m_pDrawBuffer);
	}
	// If there are 2 buffers swap them. With direct mode swap only on the last area
	if(pDrawBuffer->pBuffer1 && pDrawBuffer->pBuffer2 && (!pDisplay->m_pDriver->m_DirectMode || LastFlush)) {
		if(pDrawBuffer->pActiveBuffer == pDrawBuffer->pBuffer1)	pDrawBuffer->pActiveBuffer = pDrawBuffer->pBuffer2;
		else pDrawBuffer->pActiveBuffer = pDrawBuffer->pBuffer1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void DoFlushCB(EGDisplayDriver *pDriver, const EGRect *pRect, EG_Color_t *pBuffer)
{
	REFR_TRACE("Calling flush_cb on (%d;%d)(%d;%d) area with %p buffer pointer", pRect->GetX1(), pRect->GetY1(), pRect->GetX2(), pRect->GetY2(), (void *)pBuffer);
  EGRect OffsetRect((EG_Coord_t)(pRect->GetX1() + pDriver->m_OffsetX), (EG_Coord_t)(pRect->GetY1() + pDriver->m_OffsetY),
                    (EG_Coord_t)(pRect->GetX2() + pDriver->m_OffsetX), (EG_Coord_t)(pRect->GetY2() + pDriver->m_OffsetY));
  pDriver->FlushCB(pDriver, &OffsetRect, pBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_PERF_MONITOR

void ResetFPSCounter(void)
{
	m_Monitor.SumAllFPS = 0;
	m_Monitor.SumCountFPS = 0;
}

/////////////////////////////////////////////////////////////////////////////////////

uint32_t GetAverageFPS(void)
{
	if(m_Monitor.SumCountFPS == 0) {
		return 0;
	}
	return m_Monitor.SumAllFPS / m_Monitor.SumCountFPS;
}

/////////////////////////////////////////////////////////////////////////////////////

void InitialiseMonitor(PerformanceMonitor_t *pMonitor)
{
	EG_ASSERT_NULL(pMonitor);
	pMonitor->ElapsSum = 0;
	pMonitor->SumAllFPS = 0;
	pMonitor->SumCountFPS = 0;
	pMonitor->FrameCount = 0;
	pMonitor->LastTime = 0;
	pMonitor->pLabel = nullptr;
}

#endif

/////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_MEM_MONITOR
void InitialiseMonitor(MemoryMonitor_t *pMonitor)
{
	EG_ASSERT_NULL(pMonitor);
	pMonitor->LastTime = 0;
	pMonitor->pLabel = nullptr;
}
#endif
