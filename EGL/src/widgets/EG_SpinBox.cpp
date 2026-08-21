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

#include "widgets/EG_SpinBox.h"

#if EG_USE_SPINBOX

#include "misc/EG_Assert.h"

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_SpinboxClass = {
  .pBaseClassType = &c_EditClass,
	.pEventCB = EGSpinBox::EventCB,
	.WidthDef = EG_DPI_DEF,
	.HeightDef = 0,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
  .pExtData = NULL,
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGSpinBox::EGSpinBox(void) : EGEdit(),
  m_Value(0),
  m_MaxRange(0),
  m_MinRange(0),
  m_Step(0),
  m_DigitCount(0),
  m_DecimalPointPos(0),
  m_Rollover(0),
  m_StepDir(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGSpinBox::EGSpinBox(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_SpinboxClass*/) : EGEdit(),
  m_Value(0),
  m_MaxRange(0),
  m_MinRange(0),
  m_Step(0),
  m_DigitCount(0),
  m_DecimalPointPos(0),
  m_Rollover(0),
  m_StepDir(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::Configure(void)
{
  EGEdit::Configure();
	m_Value = 0;
	m_DecimalPointPos = 0;
	m_DigitCount = 5;
	m_Step = 1;
	m_MaxRange = 99999;
	m_MinRange = -99999;
	m_Rollover = false;
	m_StepDir = EG_DIR_RIGHT;
	SetOneLineMode(true);
	SetCursorClickPos(true);
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetValue(int32_t Value)
{
	m_Value = (Value > m_MaxRange) ? m_MaxRange : (Value < m_MinRange) ? m_MinRange : Value;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetRollover(bool Enable)
{
	m_Rollover = Enable;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetDigitFormat(uint8_t DigitCount, uint8_t SeparatorPosition)
{
	if(DigitCount > EG_SPINBOX_MAX_DIGIT_COUNT) DigitCount = EG_SPINBOX_MAX_DIGIT_COUNT;
	if(SeparatorPosition >= DigitCount) SeparatorPosition = 0;
	if(DigitCount < EG_SPINBOX_MAX_DIGIT_COUNT) {
		int64_t max_val = EG_Pow(10, DigitCount);
		if(m_MaxRange > max_val - 1) m_MaxRange = max_val - 1;
		if(m_MinRange < -max_val + 1) m_MinRange = -max_val + 1;
	}
	m_DigitCount = DigitCount;
	m_DecimalPointPos = SeparatorPosition;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetStep(uint32_t Step)
{
	m_Step = Step;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetRange(int32_t MinRange, int32_t MaxRange)
{
	m_MaxRange = MaxRange;
	m_MinRange = MinRange;
	if(m_Value > m_MaxRange) m_Value = m_MaxRange;
	if(m_Value < m_MinRange) m_Value = m_MinRange;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetCursorPos(uint8_t Pos)
{
int32_t step_limit;


	step_limit = EG_MAX(m_MaxRange, (m_MinRange < 0 ? (-m_MinRange) : m_MinRange));
	int32_t new_step = m_Step * EG_Pow(10, Pos);
	if(Pos <= 0) m_Step = 1;
	else if(new_step <= step_limit)	m_Step = new_step;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::SetStepDirection(EG_DirType_e Direction)
{
	m_StepDir = Direction;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::StepNext(void)
{
	int32_t new_step = m_Step / 10;
	if((new_step) > 0) m_Step = new_step;
	else m_Step = 1;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::StepPrev(void)
{
	int32_t step_limit;
	step_limit = EG_MAX(m_MaxRange, (m_MinRange < 0 ? (-m_MinRange) : m_MinRange));
	int32_t new_step = m_Step * 10;
	if(new_step <= step_limit) m_Step = new_step;
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::Increment(void)
{
	if(m_Value + m_Step <= m_MaxRange) {
		if((m_Value + m_Step) > 0 && m_Value < 0) m_Value = -m_Value;		// Special mode when zero crossing
		m_Value += m_Step;
	}
	else {
		if((m_Rollover) && (m_Value == m_MaxRange))	m_Value = m_MinRange;		// Rollover?
		else m_Value = m_MaxRange;
	}
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::Decrement(void)
{
	if(m_Value - m_Step >= m_MinRange) {
		if((m_Value - m_Step) < 0 && m_Value > 0) m_Value = -m_Value;		// Special mode when zero crossing
		m_Value -= m_Step;
	}
	else {
		if((m_Rollover) && (m_Value == m_MinRange))	m_Value = m_MaxRange;	// Rollover?
		else m_Value = m_MinRange;
	}
	UpdateValue();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(&c_SpinboxClass) != EG_RES_OK) return; // Call the ancestor's event handler
	EGSpinBox *pSpinbox = (EGSpinBox*)pEvent->GetTarget();
	pSpinbox->Event(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::Event(EGEvent *pEvent)
{
  EG_EventCode_e Code = pEvent->GetCode();
  switch(Code){
    case EG_EVENT_RELEASED: {
      // If released with an ENCODER then move to the next digit
      if(EGInputDevice::GetActive()->GetType() == EG_INDEV_TYPE_ENCODER) {
        EGGroup *pGroup = (EGGroup*)GetGroup();
        bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
        if(IsEditing) {
          if(m_DigitCount > 1) {
            if(m_StepDir == EG_DIR_RIGHT) {
              if(m_Step > 1) {
                StepNext();
              }
              else {
                // Restart from the MSB
                m_Step = EG_Pow(10, m_DigitCount - 2);
                StepPrev();
              }
            }
            else {
              if(m_Step < EG_Pow(10, m_DigitCount - 1)) {
                StepPrev();
              }
              else {
                // Restart from the LSB
                m_Step = 10;
                StepNext();
              }
            }
          }
        }
      }
      else {      // The cursor has been positioned to a digit. Set `step` accordingly
        const char *pText = GetText();
        size_t Length = strlen(pText);

        if(pText[m_Cursor.Position] == '.'){
          CursorLeft();
        }
        else if(m_Cursor.Position == (uint32_t)Length) {
          SetCursorPos(Length - 1);
        }
        else if(m_Cursor.Position == 0 && m_MinRange < 0) {
          SetCursorPos(1);
        }
        size_t Digits = m_DigitCount - 1;
        uint16_t CursorPos = m_Cursor.Position;
        if(m_Cursor.Position > m_DecimalPointPos && m_DecimalPointPos != 0) CursorPos--;
        uint32_t DigitPos = Digits - CursorPos;
        if(m_MinRange < 0) DigitPos++;
        m_Step = 1;
        uint16_t i;
        for(i = 0; i < DigitPos; i++) m_Step *= 10;
      }
      break;
    }
    case EG_EVENT_KEY: {
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      uint32_t Key = *((uint32_t *)pEvent->GetParam()); // uint32_t because can be UTF-8
      switch(Key){
        case EG_KEY_RIGHT: {
          if(InputType == EG_INDEV_TYPE_ENCODER) Increment();
          else StepNext();
          break;
        }
        case EG_KEY_LEFT: {
          if(InputType == EG_INDEV_TYPE_ENCODER) Decrement();
          else StepPrev();
          break;
        }
        case EG_KEY_UP: {
          Increment();
          break;
        }
        case EG_KEY_DOWN: {
          Decrement();
          break;
        }
      }
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSpinBox::UpdateValue(void)
{
char Buffer[EG_SPINBOX_MAX_DIGIT_COUNT + 8];

	EG_ZeroMem(Buffer, sizeof(Buffer));
	char *pBuffer = Buffer;
	uint8_t CursorShift = 0;
	if(m_MinRange < 0) {  // hide sign if there are only positive values
		(*pBuffer) = m_Value >= 0 ? '+' : '-';		// Add the sign
		pBuffer++;
	}
	else CursorShift++;	// Cursor need shift to left
	int32_t i;
	char Digits[EG_SPINBOX_MAX_DIGIT_COUNT + 4];
	// Convert the numbers to string (the sign is already handled so always covert positive number)
	eg_snprintf(Digits, sizeof(Digits), "%" EG_PRId32, EG_ABS(m_Value));
	int lz_cnt = m_DigitCount - (int)strlen(Digits);	// Add leading zeros
	if(lz_cnt > 0) {
		for(i = (uint16_t)strlen(Digits); i >= 0; i--) {
			Digits[i + lz_cnt] = Digits[i];
		}
		for(i = 0; i < lz_cnt; i++) {
			Digits[i] = '0';
		}
	}
	int32_t intDigits;
	intDigits = (m_DecimalPointPos == 0) ? m_DigitCount : m_DecimalPointPos;
	for(i = 0; i < intDigits && Digits[i] != '\0'; i++) {	// Add the decimal part
		(*pBuffer) = Digits[i];
		pBuffer++;
	}
	if(m_DecimalPointPos != 0) {
		// Insert the decimal point
		(*pBuffer) = '.';
		pBuffer++;
		for(/*Leave i*/; i < m_DigitCount && Digits[i] != '\0'; i++) {
			(*pBuffer) = Digits[i];
			pBuffer++;
		}
	}
	SetText((char *)Buffer);	// Refresh the text
	int32_t Step = m_Step;	// Set the cursor position
	uint8_t CursorPos = (uint8_t)m_DigitCount;
	while(Step >= 10) {
		Step /= 10;
		CursorPos--;
	}
	if(CursorPos > intDigits) CursorPos++; // Skip the decimal point
	CursorPos -= CursorShift;
	SetCursorPosition(CursorPos);
}

#endif
