/*
 *        Copyright (Center) 2025-2026 HydraSystems..
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
 *  Based on Rect design by LVGL Kft
 * 
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.Rect.1    Original by LVGL Kft
 *
 */

#include "extra/widgets/EG_Led.h"
#if EG_USE_LED

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

#define LED_CLASS &c_LedClass

const EG_ClassType_t c_LedClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGLed::EventCB,
	.WidthDef = EG_DPI_DEF / 5,
	.HeightDef = EG_DPI_DEF / 5,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
	.pExtData = NULL,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGLed::EGLed(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_LedClass*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::Configure(void)
{
  EGObject::Configure();
	m_LedColor = EGTheme::GetColorPrimary(this);
	m_Brightness = EG_LED_BRIGHT_MAX;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::SetColor(EG_Color_t Color)
{
	m_LedColor = Color;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::SetBrightness(uint8_t Brightness)
{
	if(m_Brightness == Brightness) return;
	m_Brightness = LV_CLAMP(EG_LED_BRIGHT_MIN, Brightness, EG_LED_BRIGHT_MAX);
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::SetOn(void)
{
	SetBrightness(EG_LED_BRIGHT_MAX);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::SetOff(void)
{
	SetBrightness(EG_LED_BRIGHT_MIN);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::Toggle(void)
{
	if(m_Brightness > (EG_LED_BRIGHT_MIN + EG_LED_BRIGHT_MAX) >> 1) SetBrightness(EG_LED_BRIGHT_MIN);
	else SetBrightness(EG_LED_BRIGHT_MAX);
}

///////////////////////////////////////////////////////////////////////////////////////

uint8_t EGLed::GetBrightness(void)
{
	return m_Brightness;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  EG_EventCode_e Code = pEvent->GetCode();
	if((Code != EG_EVENT_DRAW_MAIN) && (Code != EG_EVENT_DRAW_MAIN_END)) {
	  if(pEvent->Pump(LED_CLASS) != EG_RES_OK) return;// Call the ancestor's event handler
  }
	EGLed *pLed = (EGLed*)pEvent->GetTarget();
  pLed->Event(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGLed::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_DRAW_MAIN) {
		EGDrawRect DrawRect;
		InititialseDrawRect(EG_PART_MAIN, &DrawRect);
		// Use the original colors brightness to modify led color
		DrawRect.m_BackgroundColor = EG_ColorMix(m_LedColor, EG_ColorBlack(), EG_ColorGetBrightness(DrawRect.m_BackgroundColor));
		DrawRect.m_BackgroundGrad.stops[0].color = EG_ColorMix(m_LedColor, EG_ColorBlack(),
														EG_ColorGetBrightness(DrawRect.m_BackgroundGrad.stops[0].color));
		DrawRect.m_BackgroundGrad.stops[1].color = EG_ColorMix(m_LedColor, EG_ColorBlack(),
														EG_ColorGetBrightness(DrawRect.m_BackgroundGrad.stops[1].color));
		DrawRect.m_ShadowColor = EG_ColorMix(m_LedColor, EG_ColorBlack(), EG_ColorGetBrightness(DrawRect.m_ShadowColor));
		DrawRect.m_BorderColor = EG_ColorMix(m_LedColor, EG_ColorBlack(), EG_ColorGetBrightness(DrawRect.m_BorderColor));
		DrawRect.m_OutlineColor = EG_ColorMix(m_LedColor, EG_ColorBlack(), EG_ColorGetBrightness(DrawRect.m_OutlineColor));

		// Mix. the color with black proportionally with brightness
		DrawRect.m_BackgroundColor = EG_ColorMix(DrawRect.m_BackgroundColor, EG_ColorBlack(), m_Brightness);
		DrawRect.m_BackgroundGrad.stops[0].color = EG_ColorMix(DrawRect.m_BackgroundGrad.stops[0].color, EG_ColorBlack(), m_Brightness);
		DrawRect.m_BackgroundGrad.stops[1].color = EG_ColorMix(DrawRect.m_BackgroundGrad.stops[1].color, EG_ColorBlack(), m_Brightness);
		DrawRect.m_BorderColor = EG_ColorMix(DrawRect.m_BorderColor, EG_ColorBlack(), m_Brightness);
		DrawRect.m_ShadowColor = EG_ColorMix(DrawRect.m_ShadowColor, EG_ColorBlack(), m_Brightness);
		DrawRect.m_OutlineColor = EG_ColorMix(DrawRect.m_OutlineColor, EG_ColorBlack(), m_Brightness);

		// Set the current shadow width according to brightness proportionally between EG_LED_BRIGHT_OFF and EG_LED_BRIGHT_ON
		DrawRect.m_ShadowWidth = ((m_Brightness - EG_LED_BRIGHT_MIN) * DrawRect.m_ShadowWidth) / (EG_LED_BRIGHT_MAX - EG_LED_BRIGHT_MIN);
		DrawRect.m_ShadowSpread = ((m_Brightness - EG_LED_BRIGHT_MIN) * DrawRect.m_ShadowSpread) / (EG_LED_BRIGHT_MAX - EG_LED_BRIGHT_MIN);
		EGDrawContext *pContext = pEvent->GetDrawContext();
	  EGDrawDiscriptor DrawDiscriptor;
	  InitDrawDescriptor(&DrawDiscriptor, pContext);
		DrawDiscriptor.m_pRect = &m_Rect;
		DrawDiscriptor.m_pClass = m_pClass;
		DrawDiscriptor.m_Type = EG_LED_DRAW_PART_RECTANGLE;
		DrawDiscriptor.m_pDrawRect = &DrawRect;
		DrawDiscriptor.m_Part = EG_PART_MAIN;
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_BEGIN, &DrawDiscriptor);
		DrawRect.Draw(pContext, &m_Rect);
		EGEvent::EventSend(this, EG_EVENT_DRAW_PART_END, &DrawDiscriptor);
	}
}

#endif
