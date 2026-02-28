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

#include "extra/widgets/EG_ColorWheel.h"

#if EG_USE_COLORWHEEL

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

#define LV_CPICKER_DEF_QF 3

/**
 * The OUTER_MASK_WIDTH define is required to assist with the placing of a mask over the outer ring of the widget as when the
 * multicoloured radial lines are calculated for the outer ring of the widget their lengths are jittering because of the
 * integer based arithmetic. From tests the maximum delta was found to be 2 so the current value is set to 3 to achieve
 * appropriate masking.
 */
#define OUTER_MASK_WIDTH 3

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_ColorWheelClass = {
  .pBaseClassType = &c_ObjectClass,
  .pEventCB = EGColorWheel::EventCB, 
  .WidthDef = EG_DPI_DEF * 2, 
  .HeightDef = EG_DPI_DEF * 2, 
  .IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = NULL,
#endif
};

bool EGColorWheel::m_CreateKnobRecolor = false;

///////////////////////////////////////////////////////////////////////////////////////

EGColorWheel::EGColorWheel(void) : EGObject()
{
	m_HSV.Hue = 0;
	m_HSV.Sat = 100;
	m_HSV.Val = 100;
	m_Mode = EG_COLORWHEEL_MODE_HUE;
	m_FixedMode = 0;
	m_LastClickTime = 0;
	m_LastChangeTime = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

EGColorWheel::EGColorWheel(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_ColorWheelClass*/) : EGObject()
{
	m_HSV.Hue = 0;
	m_HSV.Sat = 100;
	m_HSV.Val = 100;
	m_Mode = EG_COLORWHEEL_MODE_HUE;
	m_FixedMode = 0;
	m_LastClickTime = 0;
	m_LastChangeTime = 0;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::Configure(void)
{
  EGObject::Configure();
	m_HSV.Hue = 0;
	m_HSV.Sat = 100;
	m_HSV.Val = 100;
	m_Mode = EG_COLORWHEEL_MODE_HUE;
	m_FixedMode = 0;
	m_LastClickTime = 0;
	m_LastChangeTime = 0;
	m_Knob.Recolor = m_CreateKnobRecolor;
	AddFlag(EG_OBJ_FLAG_ADV_HITTEST);
	ClearFlag(EG_OBJ_FLAG_SCROLL_CHAIN);
	RefreshKnobPosition();
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGColorWheel::SetHSV(EG_ColorHSV_t HSV)
{
	if(HSV.Hue > 360) HSV.Hue %= 360;
	if(HSV.Sat > 100) HSV.Sat = 100;
	if(HSV.Val > 100) HSV.Val = 100;
	if(m_HSV.Hue == HSV.Hue && m_HSV.Sat == HSV.Sat && m_HSV.Val == HSV.Val) return false;
	m_HSV = HSV;
	RefreshKnobPosition();
	Invalidate();
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGColorWheel::SetRGB(EG_Color_t Color)
{
EG_Color32_t c32;

	c32.full = EG_ColorTo32(Color);
	return SetHSV(EG_RGBToHSV(c32.ch.red, c32.ch.green, c32.ch.blue));
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::SetMode(EG_ColorWheelMode_e Mode)
{
	m_Mode = Mode;
	RefreshKnobPosition();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::SetFixedMode(bool Fixed)
{
	m_FixedMode = Fixed;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ColorHSV_t EGColorWheel::GetHSV(void)
{
	return m_HSV;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGColorWheel::GetRGB(void)
{
	return EG_HSVToRGB(m_HSV.Hue, m_HSV.Sat, m_HSV.Val);
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ColorWheelMode_e EGColorWheel::GetMode(void)
{
	return m_Mode;
}

///////////////////////////////////////////////////////////////////////////////////////

bool EGColorWheel::GetFixedMode(void)
{
	return m_FixedMode;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(&c_ColorWheelClass) != EG_RES_OK) return;// Call the ancestor's event handler
	EGColorWheel *pColorWheel = (EGColorWheel*)pEvent->GetTarget();
  pColorWheel->Event(pEvent);  // Dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_REFR_EXT_DRAW_SIZE: {
      EG_Coord_t left = GetStylePadLeft(EG_PART_KNOB);
      EG_Coord_t right = GetStylePadRight(EG_PART_KNOB);
      EG_Coord_t top = GetStylePadTop(EG_PART_KNOB);
      EG_Coord_t bottom = GetStylePadBottom(EG_PART_KNOB);
      EG_Coord_t knob_pad = EG_MAX4(left, right, top, bottom) + 2;
      EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();
      *pSize = EG_MAX(*pSize, knob_pad);
      break;
    }
    case EG_EVENT_SIZE_CHANGED: {
      EGRect *pRect = (EGRect*)pEvent->GetParam();
      // Refresh extended draw area to make knob visible*/
      if(GetWidth() != pRect->GetWidth() || GetHeight() != pRect->GetHeight()) {
        RefreshKnobPosition();
      }
      break;
    }
    case EG_EVENT_STYLE_CHANGED: {
      // Refresh extended draw area to make knob visible*/
      RefreshKnobPosition();
      break;
    }
    case EG_EVENT_KEY: {
      uint32_t Key = *((uint32_t *)pEvent->GetParam()); // uint32_t because can be UTF-8*/
      if(Key == EG_KEY_RIGHT || Key == EG_KEY_UP) {
        EG_ColorHSV_t ChangedHSV = m_HSV;
        switch(m_Mode) {
          case EG_COLORWHEEL_MODE_HUE:
            ChangedHSV.Hue = (m_HSV.Hue + 1) % 360;
            break;
          case EG_COLORWHEEL_MODE_SATURATION:
            ChangedHSV.Sat = (m_HSV.Sat + 1) % 100;
            break;
          case EG_COLORWHEEL_MODE_VALUE:
            ChangedHSV.Val = (m_HSV.Val + 1) % 100;
            break;
        }
        if(SetHSV(ChangedHSV)) EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, NULL);
      }
      else if(Key == EG_KEY_LEFT || Key == EG_KEY_DOWN) {
        EG_ColorHSV_t ChangedHSV;
        ChangedHSV = m_HSV;
        switch(m_Mode) {
          case EG_COLORWHEEL_MODE_HUE:
            ChangedHSV.Hue = m_HSV.Hue > 0 ? (m_HSV.Hue - 1) : 360;
            break;
          case EG_COLORWHEEL_MODE_SATURATION:
            ChangedHSV.Sat = m_HSV.Sat > 0 ? (m_HSV.Sat - 1) : 100;
            break;
          case EG_COLORWHEEL_MODE_VALUE:
            ChangedHSV.Val = m_HSV.Val > 0 ? (m_HSV.Val - 1) : 100;
            break;
        }
        if(SetHSV(ChangedHSV)) EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, NULL);
      }
      break;
    }
    case EG_EVENT_PRESSED: {
      m_LastChangeTime = EG_GetTick();
      EGInputDevice::GetActive()->GetPoint(&m_LastPressPoint);
      if(ResetDoubleClick() != EG_RES_OK) return;
      break;
    }
    case EG_EVENT_PRESSING: {
      EGInputDevice *pInput = EGInputDevice::GetActive();
      if(pInput == NULL) return;
      EG_InDeviceType_e InputType = pInput->GetType();
      EGPoint Point;
      if(InputType == EG_INDEV_TYPE_ENCODER || InputType == EG_INDEV_TYPE_KEYPAD) {
        Point.m_X = m_Rect.GetX1() + GetWidth() / 2;
        Point.m_Y = m_Rect.GetY1() + GetHeight() / 2;
      }
      else pInput->GetPoint(&Point);
      EG_Coord_t drag_limit = pInput->m_pDriver->m_ScrollLimit;
      if((EG_ABS(Point.m_X - m_LastPressPoint.m_X) > drag_limit) ||
        (EG_ABS(Point.m_Y - m_LastPressPoint.m_Y) > drag_limit)) {
        m_LastChangeTime = EG_GetTick();
        m_LastPressPoint.m_X = Point.m_X;
        m_LastPressPoint.m_Y = Point.m_Y;
      }
      Point.m_X -= m_Rect.GetX1();
      Point.m_Y -= m_Rect.GetY1();
      uint16_t Width = GetWidth();      // Ignore pressing in the inner area*/
      int16_t angle = 0;
      EG_Coord_t cir_w = GetStyleArcWidth(EG_PART_MAIN);
      EG_Coord_t r_in = Width / 2;
      Point.m_X -= r_in;
      Point.m_Y -= r_in;
      bool on_ring = true;
      r_in -= cir_w;
      if(r_in > EG_DPI_DEF / 2) {
        EG_Coord_t inner = cir_w / 2;
        r_in -= inner;
        if(r_in < EG_DPI_DEF / 2) r_in = EG_DPI_DEF / 2;
      }
      if(Point.m_X * Point.m_X + Point.m_Y * Point.m_Y < r_in * r_in) {
        on_ring = false;
      }
      uint32_t diff = EG_TickElapse(m_LastChangeTime);      // If the inner area is being pressed, go to the next color mode on long press*/
      if(!on_ring && diff > pInput->m_pDriver->m_LongPressTime && !m_FixedMode) {
        NextColorMode();
        EGInputDevice::GetActive()->WaitRelease();
        return;
      }
      if(!on_ring) return;      // Set the angle only if pressed on the ring*/
      angle = EG_Atan2(Point.m_X, Point.m_Y) % 360;
      EG_ColorHSV_t ChangedHSV;
      ChangedHSV = m_HSV;
      switch(m_Mode) {
        case EG_COLORWHEEL_MODE_HUE:
          ChangedHSV.Hue = angle;
          break;
        case EG_COLORWHEEL_MODE_SATURATION:
          ChangedHSV.Sat = (angle * 100) / 360;
          break;
        case EG_COLORWHEEL_MODE_VALUE:
          ChangedHSV.Val = (angle * 100) / 360;
          break;
      }
      if(SetHSV(ChangedHSV)) EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, NULL);
      break;
    }
    case EG_EVENT_HIT_TEST: {
      EG_HitTestState_t *info = (EG_HitTestState_t*)pEvent->GetParam();
      info->Result = m_Rect.IsPointIn(info->pPoint, EG_RADIUS_CIRCLE);      // Valid clicks can be only in the circle*/
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      DrawDiscGrad(pEvent);
      DrawKnob(pEvent);
      break;
    }
    case EG_EVENT_COVER_CHECK: {
      EG_CoverCheckInfo_t *info = (EG_CoverCheckInfo_t*)pEvent->GetParam();
      if(info->Result != EG_COVER_RES_MASKED) info->Result = EG_COVER_RES_NOT_COVER;
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::DrawDiscGrad(EGEvent *pEvent)
{
	EGDrawContext *draw_ctx = pEvent->GetDrawContext();
	EG_Coord_t Width = GetWidth();
	EG_Coord_t h = GetHeight();
	EG_Coord_t cx = m_Rect.GetX1() + Width / 2;
	EG_Coord_t cy = m_Rect.GetY1() + h / 2;
	EG_Coord_t Radius = Width / 2;

	EGDrawLine DrawLine;
	InititialseDrawLine(EG_PART_MAIN, &DrawLine);
	DrawLine.m_Width = (Radius * 628 / (256 / LV_CPICKER_DEF_QF)) / 100;
	DrawLine.m_Width += 2;
	uint16_t i;
	uint32_t a = 0;
	EG_Coord_t cir_w = GetStyleArcWidth(EG_PART_MAIN);
#if EG_DRAW_COMPLEX
	// Mask outer and inner ring of widget to tidy up ragged edges of lines while drawing outer ring*/
	MaskRadiusParam_t MaskOutParam;
	DrawMaskSetRadius(&MaskOutParam, &m_Rect, EG_RADIUS_CIRCLE, false);
	int16_t MaskOutID = DrawMaskAdd(&MaskOutParam, 0);
	EGRect MaskRect(m_Rect);
	MaskRect.Deflate(cir_w, cir_w);
	MaskRadiusParam_t MaskInParam;
	DrawMaskSetRadius(&MaskInParam, &MaskRect, EG_RADIUS_CIRCLE, true);
	int16_t MaskInID = DrawMaskAdd(&MaskInParam, 0);
	/*The inner and outer line ends will be masked out. So make lines a little bit longer because the masking makes a more even result*/
	EG_Coord_t cir_w_extra = DrawLine.m_Width;
#else
	EG_Coord_t cir_w_extra = 0;
#endif
	for(i = 0; i <= 256; i += LV_CPICKER_DEF_QF, a += 360 * LV_CPICKER_DEF_QF) {
		DrawLine.m_Color = AngleToModeColorFast(i);
		uint16_t angle_trigo = (uint16_t)(a >> 8); // i * 360 / 256 is the scale to apply, but we can skip multiplication here*/
		EGPoint Point[2];
		Point[0].m_X = cx + ((Radius + cir_w_extra) * EG_TrigoSin(angle_trigo) >> EG_TRIGO_SHIFT);
		Point[0].m_Y = cy + ((Radius + cir_w_extra) * lv_trigo_cos(angle_trigo) >> EG_TRIGO_SHIFT);
		Point[1].m_X = cx + ((Radius - cir_w - cir_w_extra) * EG_TrigoSin(angle_trigo) >> EG_TRIGO_SHIFT);
		Point[1].m_Y = cy + ((Radius - cir_w - cir_w_extra) * lv_trigo_cos(angle_trigo) >> EG_TRIGO_SHIFT);
		DrawLine.Draw(draw_ctx, &Point[0], &Point[1]);
	}
#if EG_DRAW_COMPLEX
	DrawMaskFreeParam(&MaskOutParam);
	DrawMaskFreeParam(&MaskInParam);
	DrawMaskRemove(MaskOutID);
	DrawMaskRemove(MaskInID);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::DrawKnob(EGEvent *pEvent)
{
	EGDrawContext *draw_ctx = pEvent->GetDrawContext();
	EGDrawRect DrawRect;
	InititialseDrawRect(EG_PART_KNOB, &DrawRect);
	DrawRect.m_Radius = EG_RADIUS_CIRCLE;
	if(m_Knob.Recolor) DrawRect.m_BackgroundColor = GetRGB();
	EGRect Rect = GetKnobArea();
	DrawRect.Draw(draw_ctx, &Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::InvalidateKnob(void)
{
	EGRect Rect = GetKnobArea();

	InvalidateArea(&Rect);
}

///////////////////////////////////////////////////////////////////////////////////////

EGRect EGColorWheel::GetKnobArea(void)
{
	// Get knob's radius*/
	uint16_t Radius = GetStyleArcWidth(EG_PART_MAIN) / 2;
	EG_Coord_t left = GetStylePadLeft(EG_PART_KNOB);
	EG_Coord_t right = GetStylePadRight(EG_PART_KNOB);
	EG_Coord_t top = GetStylePadTop(EG_PART_KNOB);
	EG_Coord_t bottom = GetStylePadBottom(EG_PART_KNOB);

	EGRect Rect;
	Rect.SetX1(m_Rect.GetX1() + m_Knob.Position.m_X - Radius - left);
	Rect.SetY1(m_Rect.GetY1() + m_Knob.Position.m_Y - Radius - right);
	Rect.SetX2(m_Rect.GetX1() + m_Knob.Position.m_X + Radius + top);
	Rect.SetY2(m_Rect.GetY1() + m_Knob.Position.m_Y + Radius + bottom);

	return Rect;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::NextColorMode(void)
{
	switch(m_Mode){
    case EG_COLORWHEEL_MODE_HUE: 
      m_Mode = EG_COLORWHEEL_MODE_SATURATION;
      break;
    case EG_COLORWHEEL_MODE_SATURATION:
      m_Mode = EG_COLORWHEEL_MODE_VALUE;
      break;
    case EG_COLORWHEEL_MODE_VALUE:
      m_Mode = EG_COLORWHEEL_MODE_HUE;
      break;
  }
 	RefreshKnobPosition();
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGColorWheel::RefreshKnobPosition(void)
{
	InvalidateKnob();
	EG_Coord_t Width = GetWidth();
	EG_Coord_t scale_w = GetStyleArcWidth(EG_PART_MAIN);
	EG_Coord_t Radius = (Width - scale_w) / 2;
	uint16_t angle = GetAngle();
	m_Knob.Position.m_X = (((int32_t)Radius * EG_TrigoSin(angle)) >> EG_TRIGO_SHIFT);
	m_Knob.Position.m_Y = (((int32_t)Radius * lv_trigo_cos(angle)) >> EG_TRIGO_SHIFT);
	m_Knob.Position.m_X = m_Knob.Position.m_X + Width / 2;
	m_Knob.Position.m_Y = m_Knob.Position.m_Y + Width / 2;
	InvalidateKnob();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGColorWheel::ResetDoubleClick(void)
{
	EGInputDevice *pInput = EGInputDevice::GetActive();
	// Double clicked? Use long press time as double click time out*/
	if(EG_TickElapse(m_LastClickTime) < pInput->m_pDriver->m_LongPressTime) {
		EG_ColorHSV_t ChangedHSV;
		ChangedHSV = m_HSV;
		switch(m_Mode) {
			case EG_COLORWHEEL_MODE_HUE:
				ChangedHSV.Hue = 0;
				break;
			case EG_COLORWHEEL_MODE_SATURATION:
				ChangedHSV.Sat = 100;
				break;
			case EG_COLORWHEEL_MODE_VALUE:
				ChangedHSV.Val = 100;
				break;
		}
		pInput->WaitRelease();
		if(SetHSV(ChangedHSV)) {
			EG_Result_t Result = EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, NULL);
      if(Result != EG_RES_OK) return Result;
		}
	}
	m_LastClickTime = EG_GetTick();
	return EG_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////////////

#define SWAPPTR(A, B) \
	do {                \
		uint8_t *t = A;   \
		A = B;            \
		B = t;            \
	} while(0)

#define HSV_PTR_SWAP(sextant, Radius, g, b) \
	if((sextant)&2) {                    \
		SWAPPTR((Radius), (b));                 \
	}                                    \
	if((sextant)&4) {                    \
		SWAPPTR((g), (b));                 \
	}                                    \
	if(!((sextant)&6)) {                 \
		if(!((sextant)&1)) {               \
			SWAPPTR((Radius), (g));               \
		}                                  \
	}                                    \
	else {                               \
		if((sextant)&1) {                  \
			SWAPPTR((Radius), (g));               \
		}                                  \
	}

///////////////////////////////////////////////////////////////////////////////////////

/**
 * Based on the idea from https://www.vagrearg.org/content/hsvrgb
 * Here we want to compute an approximate RGB value from a HSV input color space. We don't want to be accurate
 * (for that, there's EG_HSVToRGB), but we want to be fast.
 *
 * Few tricks are used here: Hue is in range [0; 6 * 256] (so that the sextant is in the high byte and the fractional part is in the low byte)
 * both s and v are in [0; 255] range (very convenient to avoid divisions).
 *
 * We fold all symmetry by swapping the R, G, B pointers so that the code is the same for all sextants.
 * We replace division by 255 by a division by 256, a.k.a a shift right by 8 bits.
 * This is wrong, but since this is only used to compute the pixels on the screen and not the final color, it's ok.
 */
void EGColorWheel::FastHSV2RGB(uint16_t Hue, uint8_t Sat, uint8_t Val, uint8_t *pRed, uint8_t *pGreen, uint8_t *pBlue)
{
	if(!Sat) {
		*pRed = *pGreen = *pBlue = Val;
		return;
	}
	uint8_t sextant = Hue >> 8;
	HSV_PTR_SWAP(sextant, pRed, pGreen, pBlue); // Swap pointers so the conversion code is the same*/
	*pGreen = Val;
	uint8_t bb = ~Sat;
	uint16_t ww = Val * bb; // Don't try to be precise, but instead, be fast*/
	*pBlue = ww >> 8;
	uint8_t h_frac = Hue & 0xff;
	if(!(sextant & 1)) {		// Up slope*/
		ww = !h_frac ? ((uint16_t)Sat << 8) : (Sat * (uint8_t)(-h_frac)); // Skip multiply if not required*/
	}
	else ww = Sat * h_frac;		// Down slope*/
	bb = ww >> 8;
	bb = ~bb;
	ww = Val * bb;
	*pRed = ww >> 8;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Color_t EGColorWheel::AngleToModeColorFast(uint16_t Angle)
{
	uint8_t Radius = 0, g = 0, b = 0;
	static uint16_t Hue = 0;
	static uint8_t Sat = 0, Val = 0, m = 255;
	static uint16_t angle_saved = 0xffff;

	// If the angle is different recalculate scaling*/
	if(angle_saved != Angle) m = 255;
	angle_saved = Angle;
	switch(m_Mode) {
		default:
		case EG_COLORWHEEL_MODE_HUE:
			// Don't recompute costly scaling if it does not change*/
			if(m != m_Mode) {
				Sat = (uint8_t)(((uint16_t)m_HSV.Sat * 51) / 20);
				Val = (uint8_t)(((uint16_t)m_HSV.Val * 51) / 20);
				m = m_Mode;
			}
			FastHSV2RGB(Angle * 6, Sat, Val, &Radius, &g, &b); // A smart compiler will replace x * 6 by (x << 2) + (x << 1) if it's more efficient*/
			break;
		case EG_COLORWHEEL_MODE_SATURATION:
			// Don't recompute costly scaling if it does not change*/
			if(m != m_Mode) {
				Hue = (uint16_t)(((uint32_t)m_HSV.Hue * 6 * 256) / 360);
				Val = (uint8_t)(((uint16_t)m_HSV.Val * 51) / 20);
				m = m_Mode;
			}
			FastHSV2RGB(Hue, Angle, Val, &Radius, &g, &b);
			break;
		case EG_COLORWHEEL_MODE_VALUE:
			// Don't recompute costly scaling if it does not change*/
			if(m != m_Mode) {
				Hue = (uint16_t)(((uint32_t)m_HSV.Hue * 6 * 256) / 360);
				Sat = (uint8_t)(((uint16_t)m_HSV.Sat * 51) / 20);
				m = m_Mode;
			}
			FastHSV2RGB(Hue, Sat, Angle, &Radius, &g, &b);
			break;
	}
	return EG_MixColor(Radius, g, b);
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGColorWheel::GetAngle(void)
{
	uint16_t angle;
	switch(m_Mode) {
		default:
		case EG_COLORWHEEL_MODE_HUE:
			angle = m_HSV.Hue;
			break;
		case EG_COLORWHEEL_MODE_SATURATION:
			angle = (m_HSV.Sat * 360) / 100;
			break;
		case EG_COLORWHEEL_MODE_VALUE:
			angle = (m_HSV.Val * 360) / 100;
			break;
	}
	return angle;
}

#endif 
