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

#include "extra/widgets/EG_ImageButton.h"

#if EG_USE_IMGBTN != 0

///////////////////////////////////////////////////////////////////////////////////////

#define IMAGEBUTTON_CLASS &c_ImageButtonClass

const EG_ClassType_t c_ImageButtonClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGImageButton::EventCB,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = NULL,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGImageButton::EGImageButton(void) : EGObject()
{
	EG_ZeroMem(m_pImageSourceMid, sizeof(m_pImageSourceMid));
	EG_ZeroMem(m_pImsgeSourceLeft, sizeof(m_pImsgeSourceLeft));
	EG_ZeroMem(m_pImageSourceRight, sizeof(m_pImageSourceRight));
	m_ColorFormat = EG_IMG_CF_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////////////

EGImageButton::EGImageButton(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_ImageButtonClass*/) : EGObject()
{
  Attach(this, pParent, pClassCnfg);
	Initialise();

}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::Configure(void)
{
	EGObject::Configure();
	EG_ZeroMem(m_pImageSourceMid, sizeof(m_pImageSourceMid));
	EG_ZeroMem(m_pImsgeSourceLeft, sizeof(m_pImsgeSourceLeft));
	EG_ZeroMem(m_pImageSourceRight, sizeof(m_pImageSourceRight));
	m_ColorFormat = EG_IMG_CF_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::SetSource(EG_ImageButtonState_e State, const void *pSourceLeft, const void *pSourceMid, const void *pSourceRight)
{
	m_pImsgeSourceLeft[State] = pSourceLeft;
	m_pImageSourceMid[State] = pSourceMid;
	m_pImageSourceRight[State] = pSourceRight;
	RefreshImage();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::SetState(EG_ImageButtonState_e State)
{
	EGState_t DefState = EG_STATE_DEFAULT;
	if(State == EG_IMGBTN_STATE_PRESSED || State == EG_IMGBTN_STATE_CHECKED_PRESSED) DefState |= EG_STATE_PRESSED;
	if(State == EG_IMGBTN_STATE_DISABLED || State == EG_IMGBTN_STATE_CHECKED_DISABLED) DefState |= EG_STATE_DISABLED;
	if(State == EG_IMGBTN_STATE_CHECKED_DISABLED || State == EG_IMGBTN_STATE_CHECKED_PRESSED ||
		 State == EG_IMGBTN_STATE_CHECKED_RELEASED) {
		DefState |= EG_STATE_CHECKED;
	}
	ClearState(EG_STATE_CHECKED | EG_STATE_PRESSED | EG_STATE_DISABLED);
	AddState(DefState);
	RefreshImage();
}

///////////////////////////////////////////////////////////////////////////////////////

const void* EGImageButton::GetSourceLeft(EG_ImageButtonState_e State)
{
	return m_pImsgeSourceLeft[State];
}

///////////////////////////////////////////////////////////////////////////////////////

const void* EGImageButton::GetSourceMiddle(EG_ImageButtonState_e State)
{
	return m_pImageSourceMid[State];
}

///////////////////////////////////////////////////////////////////////////////////////

const void* EGImageButton::GetSourceRight(EG_ImageButtonState_e State)
{
	return m_pImageSourceRight[State];
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(IMAGEBUTTON_CLASS) != EG_RES_OK) return;// Call the ancestor's event handler
	EGImageButton *pImageButton = (EGImageButton*)pEvent->GetTarget();
  pImageButton->Event(pEvent);  // Dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_PRESSED:
    case EG_EVENT_RELEASED:
    case EG_EVENT_PRESS_LOST: {
      RefreshImage();
      break;
    }
    case EG_EVENT_DRAW_MAIN: {
      DrawMain(pEvent);
      break;
    }
    case EG_EVENT_COVER_CHECK: {
      EG_CoverCheckInfo_t *info = (EG_CoverCheckInfo_t*)pEvent->GetParam();
      if(info->Result != EG_COVER_RES_MASKED) info->Result = EG_COVER_RES_NOT_COVER;
      break;
    }
    case EG_EVENT_GET_SELF_SIZE: {
      EGPoint *Point = pEvent->GetSelfSizeInfo();
      EG_ImageButtonState_e state = SuggestState(GetState());
      if(m_pImsgeSourceLeft[state] == NULL &&
        m_pImageSourceMid[state] != NULL &&
        m_pImageSourceRight[state] == NULL) {
        EG_ImageHeader_t Header;
        EGImageDecoder::GetInfo(m_pImageSourceMid[state], &Header);
        Point->m_X = EG_MAX(Point->m_X, Header.Width);
      }
      break;
    }
    case EG_EVENT_VALUE_CHANGED: { // Sent when the widget is checked due to EG_OBJ_FLAG_CHECKABLE 
      if(HasState(EG_STATE_CHECKED)) {
        SetState(EG_IMGBTN_STATE_CHECKED_RELEASED);
      }
      else {
        SetState(EG_IMGBTN_STATE_RELEASED);
      }
      break;
    }
    default:{
      break;  // do nothing
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::DrawMain(EGEvent *pEvent)
{
	EGDrawContext *pContext = pEvent->GetDrawContext();
	EG_ImageButtonState_e state = SuggestState(GetState());	// Just draw_main an image*/
	const void *pSource = m_pImsgeSourceLeft[state];	// Simply draw the middle pSource if no tiled*/
	EG_Coord_t tw = GetStyleTransformWidth(EG_PART_MAIN);
	EG_Coord_t th = GetStyleTransformHeight(EG_PART_MAIN);
	EGRect Rect(m_Rect);
	Rect.Inflate(tw, th);
	EGDrawImage DrawImage;
	InititialseDrawImage(EG_PART_MAIN, &DrawImage);
	EG_ImageHeader_t Header;
	EGRect RectPart;
	EG_Coord_t left_w = 0;
	EG_Coord_t right_w = 0;
	if(pSource) {
		EGImageDecoder::GetInfo(pSource, &Header);
		left_w = Header.Width;
		RectPart.SetX1(Rect.GetX1());
		RectPart.SetY1(Rect.GetY1());
		RectPart.SetX2(Rect.GetX1()+ Header.Width - 1);
		RectPart.SetY2(Rect.GetY1()+ Header.Height - 1);
		DrawImage.Draw(pContext, &RectPart, pSource);
	}
	pSource = m_pImageSourceRight[state];
	if(pSource) {
		EGImageDecoder::GetInfo(pSource, &Header);
		right_w = Header.Width;
		RectPart.SetX1(Rect.GetX2() - Header.Width + 1);
		RectPart.SetY1(Rect.GetY1());
		RectPart.SetX2(Rect.GetX2());
		RectPart.SetY2(Rect.GetY1()+ Header.Height - 1);
		DrawImage.Draw(pContext, &RectPart, pSource);
	}
	pSource = m_pImageSourceMid[state];
	if(pSource) {
		EGRect ClipCenter;
		ClipCenter.SetX1(Rect.GetX1()+ left_w);
		ClipCenter.SetX2(Rect.GetX2()- right_w);
		ClipCenter.SetY1(Rect.GetY1());
		ClipCenter.SetY2(Rect.GetY2());
		bool comm_res = ClipCenter.Intersect(&ClipCenter, pContext->m_pClipRect);
		if(comm_res) {
			EGImageDecoder::GetInfo(pSource, &Header);
			const EGRect *pClipRect = pContext->m_pClipRect;
			pContext->m_pClipRect = &ClipCenter;
			RectPart.SetX1(Rect.GetX1() + left_w);
			RectPart.SetY1(Rect.GetY1());
			RectPart.SetX2(RectPart.GetX1()+ Header.Width - 1);
			RectPart.SetY2(RectPart.GetY1()+ Header.Height - 1);
			for(EG_Coord_t i = RectPart.GetX1(); i < (EG_Coord_t)(ClipCenter.GetX2()+ Header.Width - 1); i += Header.Width) {
				DrawImage.Draw(pContext, &RectPart, pSource);
				RectPart.SetX1(RectPart.GetX2() + 1);
				RectPart.IncX2(Header.Width);
			}
			pContext->m_pClipRect = pClipRect;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGImageButton::RefreshImage(void)
{
	EG_ImageButtonState_e state = SuggestState(GetState());
	EG_ImageHeader_t Header;
	const void *pSource = m_pImageSourceMid[state];
	if(pSource == NULL) return;
	EG_Result_t info_res = EG_RES_OK;
	info_res = EGImageDecoder::GetInfo(pSource, &Header);
	if(info_res == EG_RES_OK) {
		m_ColorFormat = (EG_ImageColorFormat_t)Header.ColorFormat;
		RefreshSelfSize();
		SetHeight(Header.Height); // Keep the user defined width*/
	}
	else m_ColorFormat = EG_IMG_CF_UNKNOWN;
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ImageButtonState_e EGImageButton::SuggestState(EG_ImageButtonState_e State)
{
	if(m_pImageSourceMid[State] == NULL) {
		switch(State) {
			case EG_IMGBTN_STATE_PRESSED:
				if(m_pImageSourceMid[EG_IMGBTN_STATE_RELEASED]) return EG_IMGBTN_STATE_RELEASED;
				break;
			case EG_IMGBTN_STATE_CHECKED_RELEASED:
				if(m_pImageSourceMid[EG_IMGBTN_STATE_RELEASED]) return EG_IMGBTN_STATE_RELEASED;
				break;
			case EG_IMGBTN_STATE_CHECKED_PRESSED:
				if(m_pImageSourceMid[EG_IMGBTN_STATE_CHECKED_RELEASED]) return EG_IMGBTN_STATE_CHECKED_RELEASED;
				if(m_pImageSourceMid[EG_IMGBTN_STATE_PRESSED]) return EG_IMGBTN_STATE_PRESSED;
				if(m_pImageSourceMid[EG_IMGBTN_STATE_RELEASED]) return EG_IMGBTN_STATE_RELEASED;
				break;
			case EG_IMGBTN_STATE_DISABLED:
				if(m_pImageSourceMid[EG_IMGBTN_STATE_RELEASED]) return EG_IMGBTN_STATE_RELEASED;
				break;
			case EG_IMGBTN_STATE_CHECKED_DISABLED:
				if(m_pImageSourceMid[EG_IMGBTN_STATE_CHECKED_RELEASED]) return EG_IMGBTN_STATE_CHECKED_RELEASED;
				if(m_pImageSourceMid[EG_IMGBTN_STATE_RELEASED]) return EG_IMGBTN_STATE_RELEASED;
				break;
			default:
				break;
		}
	}

	return State;
}

///////////////////////////////////////////////////////////////////////////////////////

EG_ImageButtonState_e EGImageButton::GetState(void)
{
	EGState_t State = EGObject::GetState();
	if(State & EG_STATE_DISABLED) {
		if(State & EG_STATE_CHECKED) return EG_IMGBTN_STATE_CHECKED_DISABLED;
		return EG_IMGBTN_STATE_DISABLED;
	}
	if(State & EG_STATE_CHECKED) {
		if(State & EG_STATE_PRESSED) return EG_IMGBTN_STATE_CHECKED_PRESSED;
		return EG_IMGBTN_STATE_CHECKED_RELEASED;
	}
	else {
		if(State & EG_STATE_PRESSED) return EG_IMGBTN_STATE_PRESSED;
		return EG_IMGBTN_STATE_RELEASED;
	}
}

#endif
