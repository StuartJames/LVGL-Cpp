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

//////////////////////////////////////////////////////////////////////////////////////

EGSoftContext::~EGSoftContext()
{
//	lv_draw_sw_ctx_t *draw_sw_ctx = (lv_draw_sw_ctx_t *)draw_ctx;
//	EG_ZeroMem(draw_sw_ctx, sizeof(lv_draw_sw_ctx_t));
//	draw_ctx->layer_instance_size = sizeof(lv_draw_sw_layer_ctx_t);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::InitialiseContext(void)
{
	DrawArcProc = DrawArc;
	DrawRectProc = DrawRect;
	DrawBackgroundProc = DrawBackground;
	DrawCharacterProc = DrawCharacter;
  DrawImageProc = NULL;
	DrawImageDecodedProc = DrawImageDecoded;
	DrawLineProc = DrawLine;
	DrawPolygonProc = DrawPolygon;
#if EG_DRAW_COMPLEX
	TransformProc = DrawTransform;
#endif
	WaitForFinishProc = SoftWaitForFinish;
	CopyBufferProc = BufferCopy;
	IntialiseLayerProc = DrawLayerCreate;
	LayerAdjustProc = DrawLayerAdjust;
	LayerBlendProc = DrawLayerBlend;
	LayerDestroyProc = DrawLayerDestroy;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::BufferCopy(void *pDestBuffer, EG_Coord_t DestInc, EGRect *pDestRect, void *pSourceBuffer, EG_Coord_t SourceInc, EGRect *pSourceRect)
{
	EG_Color_t *pDestCopy = (EG_Color_t*)pDestBuffer;
	EG_Color_t *pSourceCopy = (EG_Color_t*)pSourceBuffer;
	pDestCopy += DestInc * pDestRect->GetY1();	// Get the first pixel of each buffer
	pDestCopy += pDestRect->GetX1();
	pSourceCopy += SourceInc * pSourceRect->GetY1();
	pSourceCopy += pSourceRect->GetX1();
	uint32_t LineLength = pDestRect->GetWidth() * sizeof(EG_Color_t);
	for(EG_Coord_t y = pDestRect->GetY1(); y <= pDestRect->GetY2(); y++) {
		EG_CopyMem(pDestCopy, pSourceCopy, LineLength);
		pDestCopy += DestInc;
		pSourceCopy += SourceInc;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGSoftContext::SoftWaitForFinish(void)
{
  // do nothing
}


