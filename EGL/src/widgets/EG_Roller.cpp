
#include "widgets/EG_Roller.h"
#if EG_USE_ROLLER != 0

#include "misc/EG_Assert.h"
#include "draw/EG_DrawContext.h"
#include "core/EG_Group.h"
#include "core/EG_InputDevice.h"

///////////////////////////////////////////////////////////////////////////////////////

#define ROLLER_CLASS &c_RollerClass
#define ROLLER_LABEL_CLASS &c_RollerLabelClass

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_RollerClass = {
  .pBaseClassType = &c_ObjectClass,
	.pEventCB = EGRoller::EventCB,
	.WidthDef = EG_SIZE_CONTENT,
	.HeightDef = EG_DPI_DEF,
	.IsEditable = EG_OBJ_CLASS_EDITABLE_TRUE,
	.GroupDef = EG_OBJ_CLASS_GROUP_DEF_TRUE,
#if EG_USE_USER_DATA
  .pExtData = nullptr
#endif
};

const EG_ClassType_t c_RollerLabelClass = {
  .pBaseClassType = &c_LabelClass,
	.pEventCB = EGRoller::LabelEventCB,
	.WidthDef = 0,
	.HeightDef = 0,
  .IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_USER_DATA
  .pExtData = nullptr
#endif
};

///////////////////////////////////////////////////////////////////////////////////////

EGRoller::EGRoller(void) : EGObject(),
	m_ItemCount(0),
	m_CurrentItemIndex(0),
	m_FocusedItemIndex(0),
	m_Mode(EG_ROLLER_MODE_NORMAL),
  m_IsMoved(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////

EGRoller::EGRoller(EGObject *pParent, const EG_ClassType_t *pClassCnfg /*= &c_RollerClass*/) : EGObject(),
	m_ItemCount(0),
	m_CurrentItemIndex(0),
	m_FocusedItemIndex(0),
	m_Mode(EG_ROLLER_MODE_NORMAL),
  m_IsMoved(0)
{
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::Configure(void)
{
  EGObject::Configure();
	m_Mode = EG_ROLLER_MODE_NORMAL;
	m_ItemCount = 0;
	m_CurrentItemIndex = 0;
	m_FocusedItemIndex = 0;
	ClearFlag(EG_OBJ_FLAG_SCROLLABLE);
	ClearFlag(EG_OBJ_FLAG_SCROLL_CHAIN_VER);
	new EGLabel(this, &c_RollerLabelClass);
	SetItems( "Item 1\nItem 2\nItem 3\nItem 4\nItem 5", EG_ROLLER_MODE_NORMAL);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::SetItems(const char *pItems, EG_RollerMode_t Mode)
{
	EG_ASSERT_NULL(pItems);
	EGLabel *pLabel = GetLabel();
	m_CurrentItemIndex = 0;
	m_FocusedItemIndex = 0;
	// Count the '\n'-pSize to determine the number of pItems
	m_ItemCount = 0;
	uint32_t cnt;
	for(cnt = 0; pItems[cnt] != '\0'; cnt++) {
		if(pItems[cnt] == '\n') m_ItemCount++;
	}
	m_ItemCount++; // Last item has no `\n`
	if(Mode == EG_ROLLER_MODE_NORMAL) {
		m_Mode = EG_ROLLER_MODE_NORMAL;
		pLabel->SetText(pItems);
	}
	else {
		m_Mode = EG_ROLLER_MODE_INFINITE;
		size_t ItemLength = strlen(pItems) + 1; // +1 to add '\n' after item lists
		char *NewItems = (char*)EG_GetBufferMem(ItemLength * EG_ROLLER_INF_PAGES);
		uint8_t i;
		for(i = 0; i < EG_ROLLER_INF_PAGES; i++) {
			strcpy(&NewItems[ItemLength * i], pItems);
			NewItems[ItemLength * (i + 1) - 1] = '\n';
		}
		NewItems[ItemLength * EG_ROLLER_INF_PAGES - 1] = '\0';
		pLabel->SetText(NewItems);
		EG_ReleaseBufferMem(NewItems);
		m_CurrentItemIndex = ((EG_ROLLER_INF_PAGES / 2) + 0) * m_ItemCount;
		m_ItemCount = m_ItemCount * EG_ROLLER_INF_PAGES;
		Normalize();
	}
	m_FocusedItemIndex = m_CurrentItemIndex;
	pLabel->RefreshExtDrawSize();	// If the selected text has larger font the pLabel needs some extra draw padding to draw it.
}

///////////////////////////////////////////////////////////////////////////////////////

	/* Set the value even if it'pSize the same as the current value because
     *if moving to the next item with an animation which was just deleted in the PRESS Call the ancestor'pSize event handler
     *nothing will continue the animation.*/
void EGRoller::SetSelected(uint16_t SelectedItem, EG_AnimateEnable_e Enable)
{
	// In infinite mode interpret the new index relative to the currently visible "page"
	if(m_Mode == EG_ROLLER_MODE_INFINITE) {
		uint32_t RealCount = m_ItemCount / EG_ROLLER_INF_PAGES;
		uint16_t CurrentPage = m_CurrentItemIndex / RealCount;
		// Set by the user to e.g. 0, 1, 2, 3... *Upscale the value to the current page
		if(SelectedItem < RealCount) {
			uint16_t ActiveItem = m_CurrentItemIndex - CurrentPage * RealCount;
			int32_t SelectedItemSigned = SelectedItem;
			// Huge jump? Probably from last to first or first to last item.
			if(EG_ABS((int16_t)ActiveItem - SelectedItem) > RealCount / 2) {
				if(ActiveItem > SelectedItem)	SelectedItemSigned += RealCount;
				else SelectedItemSigned -= RealCount;
			}
			SelectedItem = SelectedItemSigned + RealCount * CurrentPage;
		}
	}
	m_CurrentItemIndex = SelectedItem < m_ItemCount ? SelectedItem : m_ItemCount - 1;
	m_FocusedItemIndex = m_CurrentItemIndex;
	RefreshPosition(Enable);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::SetVisibleRowCount(uint8_t RowCount)
{
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t BorderWidth = GetStyleBorderWidth(EG_PART_MAIN);
	SetHeight((EG_FontGetLineHeight(pFont) + LineSpace) * RowCount + 2 * BorderWidth);
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGRoller::GetSelected(void)
{
	if(m_Mode == EG_ROLLER_MODE_INFINITE) {
		uint16_t RealCount = m_ItemCount / EG_ROLLER_INF_PAGES;
		return m_CurrentItemIndex % RealCount;
	}
	return m_CurrentItemIndex;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::GetSelectedText(char *pBuffer, uint32_t BufferSize)
{
	EGLabel *pLabel = GetLabel();
	uint32_t i;
	uint16_t line = 0;
	const char *opt_txt = pLabel->GetText();
	size_t txt_len = strlen(opt_txt);
	for(i = 0; i < txt_len && line != m_CurrentItemIndex; i++) {
		if(opt_txt[i] == '\n') line++;
	}
	uint32_t c;
	for(c = 0; i < txt_len && opt_txt[i] != '\n'; c++, i++) {
		if(BufferSize && c >= BufferSize - 1) {
			EG_LOG_WARN("lv_dropdown_get_selected_str: the buffer was too small");
			break;
		}
		pBuffer[c] = opt_txt[i];
	}

	pBuffer[c] = '\0';
}

///////////////////////////////////////////////////////////////////////////////////////

const char* EGRoller::GetItems(void)
{
	return GetLabel()->GetText();
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t EGRoller::GetItemCount(void)
{
	if(m_Mode == EG_ROLLER_MODE_INFINITE) {
		return m_ItemCount / EG_ROLLER_INF_PAGES;
	}
	return m_ItemCount;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
	if(pEvent->Pump(ROLLER_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGRoller *pRoller = (EGRoller*)pEvent->GetTarget();
  pRoller->Event(pEvent);   // dereference once
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::Event(EGEvent *pEvent)
{
  switch(pEvent->GetCode()){	
    case EG_EVENT_GET_SELF_SIZE: {
      EGPoint *pPoint = (EGPoint*)pEvent->GetParam();
      pPoint->m_X = GetSelectedLabelWidth();
      break;
    }
    case EG_EVENT_STYLE_CHANGED: {
      EGLabel *pLabel = GetLabel();
      // Be sure the pLabel'pSize style is updated before processing the pRoller
      if(pLabel) EGEvent::EventSend(pLabel, EG_EVENT_STYLE_CHANGED, nullptr);
      RefreshSelfSize();
      RefreshPosition(EG_ANIM_OFF);
      break;
    }
    case EG_EVENT_SIZE_CHANGED: {
      RefreshPosition(EG_ANIM_OFF);
      break;
    }
    case EG_EVENT_PRESSED: {
      m_IsMoved = 0;
      EGAnimate::Delete(GetLabel(), SetAnimateY);
      break;
    }
    case EG_EVENT_PRESSING: {
      EGInputDevice *pInput = EGInputDevice::GetActive();
      EGPoint Point;
      pInput->GetVector(&Point);
      if(Point.m_Y) {
        EGLabel *pLabel = GetLabel();
        pLabel->SetY(pLabel->GetY() + Point.m_Y);
        m_IsMoved = 1;
      }
      break;
    }
    case EG_EVENT_RELEASED:
    case EG_EVENT_PRESS_LOST: {
      ReleaseHandler();
      break;
    }
    case EG_EVENT_FOCUSED: {
      EGGroup *pGroup = (EGGroup*)GetGroup();
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
      EG_InDeviceType_e InputType = EGInputDevice::GetActive()->GetType();
      if(InputType == EG_INDEV_TYPE_ENCODER) {      // Encoders need special handling
        if(!IsEditing) {        // In navigate mode revert the original value
          if(m_CurrentItemIndex != m_FocusedItemIndex) {
            m_CurrentItemIndex = m_FocusedItemIndex;
            RefreshPosition(EG_ANIM_ON);
          }
        }
        else m_FocusedItemIndex = m_CurrentItemIndex; // Save the current state when entered to edit mode
      }
      else m_FocusedItemIndex = m_CurrentItemIndex; // Save the current value. 
      break;
    }
    case EG_EVENT_DEFOCUSED: {
      // Revert the original state
      if(m_CurrentItemIndex != m_FocusedItemIndex) {
        m_CurrentItemIndex = m_FocusedItemIndex;
        RefreshPosition(EG_ANIM_ON);
      }
      break;
    }
    case EG_EVENT_KEY: {
      char c = *((char *)pEvent->GetParam());
      if(c == EG_KEY_RIGHT || c == EG_KEY_DOWN) {
        if(m_CurrentItemIndex + 1 < m_ItemCount) {
          uint16_t ori_id = m_FocusedItemIndex; // SetSelected will overwrite this
          SetSelected(m_CurrentItemIndex + 1, EG_ANIM_ON);
          m_FocusedItemIndex = ori_id;
        }
      }
      else if(c == EG_KEY_LEFT || c == EG_KEY_UP) {
        if(m_CurrentItemIndex > 0) {
          uint16_t ori_id = m_FocusedItemIndex; // SetSelected will overwrite this

          SetSelected(m_CurrentItemIndex - 1, EG_ANIM_ON);
          m_FocusedItemIndex = ori_id;
        }
      }
      break;
    }
    case EG_EVENT_REFR_EXT_DRAW_SIZE: {
      EGLabel *pLabel = GetLabel();
      pLabel->RefreshExtDrawSize();
      break;
    }
    case EG_EVENT_DRAW_MAIN:
    case EG_EVENT_DRAW_POST: {
      DrawMain(pEvent);
      break;
    }
    default:{
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::DrawMain(EGEvent *pEvent)
{
	EG_EventCode_e Code = pEvent->GetCode();
	EGDrawContext *pContext = pEvent->GetDrawContext();
	if(Code == EG_EVENT_DRAW_MAIN) {		// Draw the select rectangle
		EGRect SelectRect;
		GetSelectionRect(&SelectRect);
		EGDrawRect DrawRect;
		InititialseDrawRect(EG_PART_SELECTED, &DrawRect);
		DrawRect.Draw(pContext, &SelectRect);
	}
	else if(Code == EG_EVENT_DRAW_POST) {	// Post draw when the children are drawn
		EGDrawLabel DrawLabel;
		InititialseDrawLabel(EG_PART_SELECTED, &DrawLabel);
		EGRect SelectRect;		// Redraw the text on the selected area
		GetSelectionRect(&SelectRect);
		EGRect SelectMask;
		if(SelectMask.Intersect(pContext->m_pClipRect, &SelectRect)) {
			EGLabel *pLabel = GetLabel();
			if(pLabel->GetRecolor()) DrawLabel.m_Flag |= EG_TEXT_FLAG_RECOLOR;
			EGPoint ResultPoint;			// Get the size of the "selected text"
			EG_GetTextSize(&ResultPoint, pLabel->GetText(), DrawLabel.m_pFont, DrawLabel.m_Kerning, DrawLabel.m_LineSpace,	GetWidth(), EG_TEXT_FLAG_EXPAND);
			// Move the selected pLabel proportionally with the background pLabel
			EG_Coord_t roller_h = GetHeight();
			int32_t label_y_prop = pLabel->m_Rect.GetY1() - (roller_h / 2 + m_Rect.GetY1()); // pLabel offset from the middle line of the pRoller
			label_y_prop = (label_y_prop * 16384) / pLabel->GetHeight(); // Proportional position from the middle line (upscaled by << 14)
			// Apply a correction with different line heights
			const EG_Font_t *pStyleFont = GetStyleTextFont(EG_PART_MAIN);
			EG_Coord_t corr = (DrawLabel.m_pFont->LineHeight - pStyleFont->LineHeight) / 2;
			ResultPoint.m_Y -= corr;			// Apply the proportional position to the selected text
			int32_t label_sel_y = roller_h / 2 + m_Rect.GetY1();
			label_sel_y += (label_y_prop * ResultPoint.m_Y) >> 14;
			label_sel_y -= corr;
			EG_Coord_t bwidth = GetStyleBorderWidth(EG_PART_MAIN);
			EG_Coord_t pleft = GetStylePadLeft(EG_PART_MAIN);
			EG_Coord_t pright = GetStylePadRight(EG_PART_MAIN);
			EGRect SelectRect;			// Draw the selected text
			SelectRect.SetX1(m_Rect.GetX1() + pleft + bwidth);
			SelectRect.SetY1(label_sel_y);
			SelectRect.SetX2(m_Rect.GetX2() - pright - bwidth);
			SelectRect.SetY2(SelectRect.GetY1() + ResultPoint.m_Y);
			DrawLabel.m_Flag |= EG_TEXT_FLAG_EXPAND;
			const EGRect *pClipRect = pContext->m_pClipRect;
			pContext->m_pClipRect = &SelectMask;
			DrawLabel.Draw(pContext, &SelectRect, pLabel->GetText(), nullptr);
			pContext->m_pClipRect = pClipRect;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::GetSelectionRect(EGRect *pRect)
{
	const EG_Font_t *pFontMain = GetStyleTextFont(EG_PART_MAIN);
	const EG_Font_t *pFontSelect = GetStyleTextFont(EG_PART_SELECTED);
	EG_Coord_t MainHeight = EG_FontGetLineHeight(pFontMain);
	EG_Coord_t SelectHeight = EG_FontGetLineHeight(pFontSelect);
	EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t d = (SelectHeight + MainHeight) / 2 + LineSpace;
	pRect->SetY1(m_Rect.GetY1() + GetHeight() / 2 - d / 2);
	pRect->SetY2(pRect->GetY1() + d);
	pRect->SetX1(m_Rect.GetX1());
	pRect->SetX2(m_Rect.GetX2());
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::RefreshPosition(EG_AnimateEnable_e Enable)
{
	EGLabel *pLabel = GetLabel();
	if(pLabel == nullptr) return;
	EG_TextAlignment_t Alignment = pLabel->CalculateTextAlignment(EG_PART_MAIN, pLabel->GetText());
	switch(Alignment) {
		case EG_TEXT_ALIGN_CENTER:
			pLabel->SetX((GetContentWidth() - pLabel->GetWidth()) / 2);
			break;
		case EG_TEXT_ALIGN_RIGHT:
			pLabel->SetX(GetContentWidth() - pLabel->GetWidth());
			break;
		case EG_TEXT_ALIGN_LEFT:
			pLabel->SetX(0);
			break;
	}
	const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
	EG_Coord_t line_space = GetStyleTextLineSpace(EG_PART_MAIN);
	EG_Coord_t font_h = EG_FontGetLineHeight(pFont);
	EG_Coord_t Height = GetContentHeight();
	uint16_t anim_time = GetStyleAnimationTime(EG_PART_MAIN);
	// Normally the animation'pSize `end_cb` sets correct position of the pRoller if infinite. But without animations do it manually
	if(Enable == EG_ANIM_OFF || anim_time == 0) {
		Normalize();
	}
	int32_t id = m_CurrentItemIndex;
	EG_Coord_t sel_y1 = id * (font_h + line_space);
	EG_Coord_t mid_y1 = Height / 2 - font_h / 2;
	EG_Coord_t new_y = mid_y1 - sel_y1;
	if(Enable == EG_ANIM_OFF || anim_time == 0) {
		EGAnimate::Delete(pLabel, SetAnimateY);
		pLabel->SetY(new_y);
	}
	else {
		EGAnimate Animate;
		Animate.SetItem(pLabel);
		Animate.SetExcCB(SetAnimateY);
		Animate.SetValues(pLabel->GetY(), new_y);
		Animate.SetTime(anim_time);
		Animate.SetEndCB(ScrollAnimateEndCB);
		Animate.SetPathCB(EGAnimate::PathEaseOut);
		EGAnimate::Create(&Animate);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGRoller::ReleaseHandler(void)
{
	EGLabel *pLabel = GetLabel();
	if(pLabel == nullptr) return EG_RES_OK;
	EGInputDevice *pInput = EGInputDevice::GetActive();
	// Leave edit mode once a new item is selected
	EG_InDeviceType_e InputType = pInput->GetType();
	if(InputType == EG_INDEV_TYPE_ENCODER || InputType == EG_INDEV_TYPE_KEYPAD) {
		m_FocusedItemIndex = m_CurrentItemIndex;
		if(InputType == EG_INDEV_TYPE_ENCODER) {
      EGGroup *pGroup = (EGGroup*)GetGroup();
      bool IsEditing = (pGroup != nullptr) ? pGroup->GetEditing() : false;
			if(IsEditing) {
				pGroup->SetEditing(false);
			}
		}
	}
	if(InputType == EG_INDEV_TYPE_POINTER || InputType == EG_INDEV_TYPE_BUTTON) {
		// Search the clicked item (For KEYPAD and ENCODER the new value should be already set)
		int16_t new_opt = -1;
		if(m_IsMoved == 0) {
			new_opt = 0;
			EGPoint Point;
			pInput->GetPoint(&Point);
			Point.m_Y -= pLabel->m_Rect.GetY1();
			Point.m_X -= pLabel->m_Rect.GetX1();
			uint32_t Index = pLabel->GetCharacterAt(&Point);
			const char *pText = pLabel->GetText();
			uint32_t i = 0;
			uint32_t i_prev = 0;
			uint32_t letter_cnt = 0;
			for(letter_cnt = 0; letter_cnt < Index; letter_cnt++) {
				uint32_t letter = EG_TextDecodeNext(pText, &i);
				// Count he lines to reach the clicked letter. But ignore the last '\n' because it still belongs to the clicked line
				if(letter == '\n' && i_prev != Index) new_opt++;
				i_prev = i;
			}
		}
		else {
			// If dragged then align the list to have an element in the middle
			const EG_Font_t *font = GetStyleTextFont(EG_PART_MAIN);
			EG_Coord_t line_space = GetStyleTextLineSpace(EG_PART_MAIN);
			EG_Coord_t font_h = EG_FontGetLineHeight(font);
			EG_Coord_t label_unit = font_h + line_space;
			EG_Coord_t mid = m_Rect.GetY1() + (m_Rect.GetY2() - m_Rect.GetY1()) / 2;
			EG_Coord_t label_y1 = pLabel->m_Rect.GetY1() + pInput->ScrollThrowPredict(EG_DIR_VER);
			int32_t id = (mid - label_y1) / label_unit;
			if(id < 0) id = 0;
			if(id >= m_ItemCount) id = m_ItemCount - 1;
			new_opt = id;
		}
		if(new_opt >= 0) {
			SetSelected(new_opt, EG_ANIM_ON);
		}
	}
	uint32_t id = m_CurrentItemIndex; // Just to use uint32_t in event data
	return EGEvent::EventSend(this, EG_EVENT_VALUE_CHANGED, &id);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::Normalize(void)
{
	if(m_Mode == EG_ROLLER_MODE_INFINITE) {
		uint16_t RealCount = m_ItemCount / EG_ROLLER_INF_PAGES;
		m_CurrentItemIndex = m_CurrentItemIndex % RealCount;
		m_CurrentItemIndex += (EG_ROLLER_INF_PAGES / 2) * RealCount; // Select the middle page

		m_FocusedItemIndex = m_CurrentItemIndex % RealCount;
		m_FocusedItemIndex += (EG_ROLLER_INF_PAGES / 2) * RealCount; // Select the middle page

		// Move to the new id
		const EG_Font_t *pFont = GetStyleTextFont(EG_PART_MAIN);
		EG_Coord_t LineSpace = GetStyleTextLineSpace(EG_PART_MAIN);
		EG_Coord_t FontHeight = EG_FontGetLineHeight(pFont);
		EG_Coord_t Height = GetContentHeight();
		EGObject *pLabel = GetLabel();
		EG_Coord_t SelectY = m_CurrentItemIndex * (FontHeight + LineSpace);
		EG_Coord_t MiddleY = Height / 2 - FontHeight / 2;
		EG_Coord_t SetY = MiddleY - SelectY;
		pLabel->SetY(SetY);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

EGLabel* EGRoller::GetLabel(void)
{
	return (EGLabel*)GetChild(0);
}

///////////////////////////////////////////////////////////////////////////////////////

EG_Coord_t EGRoller::GetSelectedLabelWidth(void)
{
	EGLabel *pLabel = GetLabel();
	if(pLabel == nullptr) return 0;
	const EG_Font_t *font = GetStyleTextFont(EG_PART_SELECTED);
	EG_Coord_t letter_space = GetStyleTextKerning(EG_PART_SELECTED);
	const char *pText = pLabel->GetText();
	EGPoint Size;
	EG_GetTextSize(&Size, pText, font, letter_space, 0, EG_COORD_MAX, EG_TEXT_FLAG_NONE);
	return Size.m_X;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::LabelEventCB(const EG_ClassType_t *pClass, EGEvent *pEvent)
{
	EG_UNUSED(pClass);
  if(pEvent->Pump(ROLLER_LABEL_CLASS) != EG_RES_OK) return;  // Call the ancestor's event handler
	EGLabel *pLabel = (EGLabel*)pEvent->GetTarget();
	EGRoller *pRoller = (EGRoller*)pLabel->GetParent();
	EG_EventCode_e Code = pEvent->GetCode();
	if(Code == EG_EVENT_REFR_EXT_DRAW_SIZE) {
		// If the selected text has a larger font it needs some extra space to draw it
		EG_Coord_t *pSize = (EG_Coord_t*)pEvent->GetParam();
		EG_Coord_t sel_w = pRoller->GetSelectedLabelWidth();
		EG_Coord_t label_w = pLabel->GetWidth();
		*pSize = EG_MAX(*pSize, sel_w - label_w);
	}
	else if(Code == EG_EVENT_SIZE_CHANGED) {
		pRoller->RefreshPosition(EG_ANIM_OFF);
	}
	else if(Code == EG_EVENT_DRAW_MAIN) {
		pRoller->DrawLabel(pEvent, pLabel);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::DrawLabel(EGEvent *pEvent, EGLabel *pLabel)
{
	//  Split the drawing of the pLabel into  an upper (above the selected area) and a lower (below the selected area)
	EGDrawLabel DrawLabel;
  InititialseDrawLabel(EG_PART_MAIN, &DrawLabel);
	if(pLabel->GetRecolor()) DrawLabel.m_Flag |= EG_TEXT_FLAG_RECOLOR;
	EGDrawContext *pContext = pEvent->GetDrawContext();
	/* If the pRoller has shadow or outline it has some ext. draw size
     *therefore the pLabel can overflow the pRoller'pSize boundaries.
     *To solve this limit the clip area to the "plain" pRoller.*/
	const EGRect *pClipRectTemp = pContext->m_pClipRect;
	EGRect ClipRect;
	if(!ClipRect.Intersect(pContext->m_pClipRect, &m_Rect)) return;
	pContext->m_pClipRect = &ClipRect;
	EGRect SelectRect;
	GetSelectionRect(&SelectRect);
	EGRect ClipRect2;
	ClipRect2.SetX1(m_Rect.GetX1());
	ClipRect2.SetY1(m_Rect.GetY1());
	ClipRect2.SetX2(m_Rect.GetX2());
	ClipRect2.SetY2(SelectRect.GetY1());
	if(ClipRect2.Intersect(&ClipRect2, pContext->m_pClipRect)) {
		const EGRect *clip_area_ori2 = pContext->m_pClipRect;
		pContext->m_pClipRect = &ClipRect2;
		DrawLabel.Draw(pContext, &pLabel->m_Rect, pLabel->GetText(), nullptr);
		pContext->m_pClipRect = clip_area_ori2;
	}
	ClipRect2.SetX1(m_Rect.GetX1());
	ClipRect2.SetY1(SelectRect.GetY2());
	ClipRect2.SetX2(m_Rect.GetX2());
	ClipRect2.SetY2(m_Rect.GetY2());
	if(ClipRect2.Intersect(&ClipRect2, pContext->m_pClipRect)) {
		const EGRect *clip_area_ori2 = pContext->m_pClipRect;
		pContext->m_pClipRect = &ClipRect2;
		DrawLabel.Draw(pContext, &pLabel->m_Rect, pLabel->GetText(), nullptr);
		pContext->m_pClipRect = clip_area_ori2;
	}
	pContext->m_pClipRect = pClipRectTemp;
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::ScrollAnimateEndCB(EGAnimate *pAnimate)
{
	EGRoller *pRoller = (EGRoller*)((EGObject*)pAnimate->m_pItem)->GetParent(); // The pLabel is animated
	pRoller->Normalize();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGRoller::SetAnimateY(EGAnimate *pAnimate, int32_t V)
{
	((EGObject*)pAnimate->m_pItem)->SetY(V);
}

#endif
