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

#include "extra/layouts/EG_Layouts.h"

///////////////////////////////////////////////////////////////////////////////

#if EG_USE_FLEX

EGStyleProperty_e  EGFlexLayout::m_StyleFlow;
EGStyleProperty_e  EGFlexLayout::m_StyleMainPlace;
EGStyleProperty_e  EGFlexLayout::m_StyleCrossPlace;
EGStyleProperty_e  EGFlexLayout::m_StyleTrackPlace;
EGStyleProperty_e  EGFlexLayout::m_StyleGrow;
uint32_t           EGFlexLayout::m_Reference;

///////////////////////////////////////////////////////////////////////////////

EGFlexLayout::EGFlexLayout(void) :
	m_MainPlace(EG_FLEX_ALIGN_START),
	m_CrossPlace(EG_FLEX_ALIGN_START),
	m_TrackPlace(EG_FLEX_ALIGN_START),
	m_Row(0),
	m_Wrap(0),
	m_Reverse(0)

{
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::Initialise(void)
{
	m_StyleFlow = EGStyle::RegisterProperty(EG_STYLE_PROP_FLAG_NONE);
	m_StyleMainPlace = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	m_StyleCrossPlace = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	m_StyleTrackPlace = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	m_StyleGrow = EGStyle::RegisterProperty(EG_STYLE_PROP_LAYOUT_REFRESH);
	m_Reference = EGObject::LayoutRegister(EGFlexLayout::UpdateCB, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjFlow(EGObject *pObj, EG_FlexFlow_e Flow)
{
	SetObjStyleFlow(pObj, Flow, 0);
	pObj->SetStyleLayout(m_Reference, 0);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjAlign(EGObject *pObj, EG_FlexAlign_e m_MainPlace, EG_FlexAlign_e m_CrossPlace, EG_FlexAlign_e m_TrackPlace)
{
	SetObjStyleMainPlace(pObj, m_MainPlace, 0);
	SetObjStyleCrossPlace(pObj, m_CrossPlace, 0);
	SetObjStyleTrackPlace(pObj, m_TrackPlace, 0);
	pObj->SetStyleLayout(m_Reference, 0);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjGrow(EGObject *pObj, uint8_t Grow)
{
	SetObjStyleGrow(pObj, Grow, 0);
	pObj->GetParent()->MarkLayoutDirty();
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::StyleSetFlow(EGStyle *pStyle, EG_FlexFlow_e value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pStyle->SetProperty(m_StyleFlow, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::StyleSetMainPlace(EGStyle *pStyle, EG_FlexAlign_e value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pStyle->SetProperty(m_StyleMainPlace, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::StyleSetCrossPlace(EGStyle *pStyle, EG_FlexAlign_e value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pStyle->SetProperty(m_StyleCrossPlace, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::StyleSetTrackPlace(EGStyle *pStyle, EG_FlexAlign_e value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pStyle->SetProperty(m_StyleTrackPlace, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::StyleSetGrow(EGStyle *pStyle, uint8_t value)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pStyle->SetProperty(m_StyleGrow, v);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjStyleFlow(EGObject *pObj, EG_FlexFlow_e value, EG_StyleFlags_t selector)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pObj->SetLocalStyleProperty(m_StyleFlow, v, selector);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjStyleMainPlace(EGObject *pObj, EG_FlexAlign_e value, EG_StyleFlags_t selector)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pObj->SetLocalStyleProperty(m_StyleMainPlace, v, selector);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjStyleCrossPlace(EGObject *pObj, EG_FlexAlign_e value, EG_StyleFlags_t selector)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pObj->SetLocalStyleProperty(m_StyleCrossPlace, v, selector);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjStyleTrackPlace(EGObject *pObj, EG_FlexAlign_e value, EG_StyleFlags_t selector)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pObj->SetLocalStyleProperty(m_StyleTrackPlace, v, selector);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::SetObjStyleGrow(EGObject *pObj, uint8_t value, EG_StyleFlags_t selector)
{
EG_StyleValue_t v = {
	.Number = (int32_t)value
};

	pObj->SetLocalStyleProperty(m_StyleGrow, v, selector);
}

///////////////////////////////////////////////////////////////////////////////

void EGFlexLayout::UpdateCB(EGObject *pObj, void *pUserData)
{
	EG_LOG_INFO("update container %p", (void *)pObj);
  EGFlexLayout Flex; 
	EG_FlexFlow_e Flow = GetObjStyleFlow(pObj, EG_PART_MAIN);
	Flex.m_Row = Flow & _EG_FLEX_COLUMN ? 0 : 1;
	Flex.m_Wrap = Flow & _EG_FLEX_WRAP ? 1 : 0;
	Flex.m_Reverse = Flow & _EG_FLEX_REVERSE ? 1 : 0;
	Flex.m_MainPlace = GetObjStyleMainPlace(pObj, EG_PART_MAIN);
	Flex.m_CrossPlace = GetObjStyleCrossPlace(pObj, EG_PART_MAIN);
	Flex.m_TrackPlace = GetObjStyleTrackPlace(pObj, EG_PART_MAIN);
	bool rtl = pObj->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL ? true : false;
	EG_Coord_t track_gap = !Flex.m_Row ? pObj->GetStylePadColumn(EG_PART_MAIN) : pObj->GetStylePadRow(EG_PART_MAIN);
	EG_Coord_t item_gap = Flex.m_Row ? pObj->GetStylePadColumn(EG_PART_MAIN) : pObj->GetStylePadRow(EG_PART_MAIN);
	EG_Coord_t max_main_size = (Flex.m_Row ? pObj->GetContentWidth() : pObj->GetContentHeight());
	EG_Coord_t border_width = pObj->GetStyleBorderWidth(EG_PART_MAIN);
	EG_Coord_t abs_y = pObj->m_Rect.GetY1() + pObj->GetStylePadTop(EG_PART_MAIN) + border_width - pObj->GetScrollY();
	EG_Coord_t abs_x = pObj->m_Rect.GetX1() + pObj->GetStylePadLeft(EG_PART_MAIN) + border_width - pObj->GetScrollX();
	EG_FlexAlign_e track_cross_place = Flex.m_TrackPlace;
	EG_Coord_t *cross_pos = (Flex.m_Row ? &abs_y : &abs_x);
	EG_Coord_t w_set = pObj->GetStyleWidth(EG_PART_MAIN);
	EG_Coord_t h_set = pObj->GetStyleHeight(EG_PART_MAIN);

	// Content sized objects should squeezed the gap between the children, therefore any alignment will look like `START`
	if((Flex.m_Row && h_set == EG_SIZE_CONTENT && pObj->m_HeightLayout == 0) || (!Flex.m_Row && w_set == EG_SIZE_CONTENT && pObj->m_WidthLayout == 0)) {
		track_cross_place = EG_FLEX_ALIGN_START;
	}
	if(rtl && !Flex.m_Row) {
		if(track_cross_place == EG_FLEX_ALIGN_START) track_cross_place = EG_FLEX_ALIGN_END;
		else if(track_cross_place == EG_FLEX_ALIGN_END)	track_cross_place = EG_FLEX_ALIGN_START;
	}
	EG_Coord_t total_track_cross_size = 0;
	EG_Coord_t gap = 0;
	uint32_t track_cnt = 0;
	int32_t track_first_item;
	int32_t next_track_first_item;
	if(track_cross_place != EG_FLEX_ALIGN_START) {
		track_first_item = Flex.m_Reverse ? pObj->m_pAttributes->ChildCount - 1 : 0;
		TrackProps_t t;
		while(track_first_item < (int32_t)pObj->m_pAttributes->ChildCount && track_first_item >= 0) {
			/*Search the first item of the next m_Row*/
			t.grow_dsc_calc = 0;
			next_track_first_item = Flex.FindTrackEnd(pObj, track_first_item, max_main_size, item_gap, &t);
			total_track_cross_size += t.track_cross_size + track_gap;
			track_cnt++;
			track_first_item = next_track_first_item;
		}
		if(track_cnt) total_track_cross_size -= track_gap; /*No gap after the last track*/
		/*Place the tracks to get the start position*/
		EG_Coord_t max_cross_size = (Flex.m_Row ? pObj->GetContentHeight() : pObj->GetContentWidth());
		Flex.PlaceContent(track_cross_place, max_cross_size, total_track_cross_size, track_cnt, cross_pos, &gap);
	}
	track_first_item = Flex.m_Reverse ? pObj->m_pAttributes->ChildCount - 1 : 0;
	if(rtl && !Flex.m_Row) {
		*cross_pos += total_track_cross_size;
	}
	while(track_first_item < (int32_t)pObj->m_pAttributes->ChildCount && track_first_item >= 0) {
		TrackProps_t t;
		t.grow_dsc_calc = 1;
		/*Search the first item of the next m_Row*/
		next_track_first_item = Flex.FindTrackEnd(pObj, track_first_item, max_main_size, item_gap, &t);
		if(rtl && !Flex.m_Row) {
			*cross_pos -= t.track_cross_size;
		}
		Flex.RepositionChildren(pObj, track_first_item, next_track_first_item, abs_x, abs_y, max_main_size, item_gap, &t);
		track_first_item = next_track_first_item;
		EG_ReleaseBufferMem(t.grow_dsc);
		t.grow_dsc = NULL;
		if(rtl && !Flex.m_Row) {
			*cross_pos -= gap + track_gap;
		}
		else {
			*cross_pos += t.track_cross_size + gap + track_gap;
		}
	}
	EG_ASSERT_MEM_INTEGRITY();
	if(w_set == EG_SIZE_CONTENT || h_set == EG_SIZE_CONTENT) {
		pObj->RefreshSize();
	}
	EGEvent::EventSend(pObj, EG_EVENT_LAYOUT_CHANGED, NULL);
	EG_TRACE_LAYOUT("finished");
}

///////////////////////////////////////////////////////////////////////////////

// Find the last item of a track
int32_t EGFlexLayout::FindTrackEnd(EGObject *pObj, int32_t item_start_id, EG_Coord_t max_main_size,	EG_Coord_t item_gap, TrackProps_t *t)
{
	EG_Coord_t w_set = pObj->GetStyleWidth(EG_PART_MAIN);
	EG_Coord_t h_set = pObj->GetStyleHeight(EG_PART_MAIN);
	// Can't wrap if the size if auto (i.e. the size depends on the children)
	if(m_Wrap && ((m_Row && w_set == EG_SIZE_CONTENT) || (!m_Row && h_set == EG_SIZE_CONTENT))) {
		m_Wrap = false;
	}
	t->track_main_size = 0;
	t->track_fix_main_size = 0;
	t->grow_item_cnt = 0;
	t->track_cross_size = 0;
	t->item_cnt = 0;
	t->grow_dsc = NULL;
	int32_t item_id = item_start_id;
	EGObject *pItem = pObj->GetChild(item_id);
	while(pItem) {
		if(item_id != item_start_id && pItem->HasFlagSet(EG_OBJ_FLAG_FLEX_IN_NEW_TRACK)) break;
		if(!pItem->HasAnyFlagSet(EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) {
			uint8_t grow_value = GetObjStyleGrow(pItem, EG_PART_MAIN);
			if(grow_value) {
				t->grow_item_cnt++;
				t->track_fix_main_size += item_gap;
				if(t->grow_dsc_calc) {
					GrowProps_t *new_dsc = (GrowProps_t*)EG_GetBufferMem(sizeof(GrowProps_t) * (t->grow_item_cnt));
					EG_ASSERT_MALLOC(new_dsc);
					if(new_dsc == NULL) return item_id;
					if(t->grow_dsc) {
						EG_CopyMem(new_dsc, t->grow_dsc, sizeof(GrowProps_t) * (t->grow_item_cnt - 1));
						EG_ReleaseBufferMem(t->grow_dsc);
					}
					new_dsc[t->grow_item_cnt - 1].pItem = pItem;
					new_dsc[t->grow_item_cnt - 1].min_size = m_Row ? pItem->GetStyleMinWidth(EG_PART_MAIN) : pItem->GetStyleMinHeight(EG_PART_MAIN);
					new_dsc[t->grow_item_cnt - 1].max_size = m_Row ? pItem->GetStyleMaxWidth(EG_PART_MAIN) : pItem->GetStyleMaxHeight(EG_PART_MAIN);
					new_dsc[t->grow_item_cnt - 1].grow_value = grow_value;
					new_dsc[t->grow_item_cnt - 1].clamped = 0;
					t->grow_dsc = new_dsc;
				}
			}
			else {
				EG_Coord_t item_size = (m_Row) ? pItem->GetWidth() : pItem->GetHeight();
				if(m_Wrap && t->track_fix_main_size + item_size > max_main_size) break;
				t->track_fix_main_size += item_size + item_gap;
			}
			t->track_cross_size = EG_MAX(((!m_Row) ? pItem->GetWidth() : pItem->GetHeight()), t->track_cross_size);
			t->item_cnt++;
		}
		item_id += m_Reverse ? -1 : +1;
		if(item_id < 0) break;
		pItem = pObj->GetChild(item_id);
	}
	if(t->track_fix_main_size > 0) t->track_fix_main_size -= item_gap; // There is no gap after the last item
	// If there is at least one "grow item" the track takes the full space
	t->track_main_size = t->grow_item_cnt ? max_main_size : t->track_fix_main_size;
	// Have at least one item in a m_Row
	if(pItem && item_id == item_start_id) {
		pItem = pObj->m_pAttributes->ppChildren[item_id];
		GetNextItem(pObj, m_Reverse, &item_id);
		if(pItem) {
			t->track_cross_size = (!m_Row) ? pItem->GetWidth() : pItem->GetHeight();
			t->track_main_size = (m_Row) ? pItem->GetWidth() : pItem->GetHeight();
			t->item_cnt = 1;
		}
	}
	return item_id;
}

///////////////////////////////////////////////////////////////////////////////

// Position the children in the same track
void EGFlexLayout::RepositionChildren(EGObject *pObj, int32_t item_first_id, int32_t item_last_id, EG_Coord_t abs_x,
													 EG_Coord_t abs_y, EG_Coord_t max_main_size, EG_Coord_t item_gap, TrackProps_t *t)
{
	//Calculate the size of grow items first
	uint32_t i;
	bool grow_reiterate = true;
	while(grow_reiterate) {
		grow_reiterate = false;
		EG_Coord_t grow_value_sum = 0;
		EG_Coord_t grow_max_size = t->track_main_size - t->track_fix_main_size;
		for(i = 0; i < t->grow_item_cnt; i++) {
			if(t->grow_dsc[i].clamped == 0) {
				grow_value_sum += t->grow_dsc[i].grow_value;
			}
			else {
				grow_max_size -= t->grow_dsc[i].final_size;
			}
		}
		EG_Coord_t grow_unit;

		for(i = 0; i < t->grow_item_cnt; i++) {
			if(t->grow_dsc[i].clamped == 0) {
				EG_ASSERT(grow_value_sum != 0);
				grow_unit = grow_max_size / grow_value_sum;
				EG_Coord_t size = grow_unit * t->grow_dsc[i].grow_value;
				EG_Coord_t size_clamp = EG_CLAMP(t->grow_dsc[i].min_size, size, t->grow_dsc[i].max_size);

				if(size_clamp != size) {
					t->grow_dsc[i].clamped = 1;
					grow_reiterate = true;
				}
				t->grow_dsc[i].final_size = size_clamp;
				grow_value_sum -= t->grow_dsc[i].grow_value;
				grow_max_size -= t->grow_dsc[i].final_size;
			}
		}
	}

	bool rtl = pObj->GetStyleBaseDirection(EG_PART_MAIN) == EG_BASE_DIR_RTL ? true : false;

	EG_Coord_t main_pos = 0;

	EG_Coord_t place_gap = 0;
	PlaceContent(m_MainPlace, max_main_size, t->track_main_size, t->item_cnt, &main_pos, &place_gap);
	if(m_Row && rtl) main_pos += pObj->GetContentWidth();

	EGObject *pItem = pObj->GetChild(item_first_id);
	//Reposition the children
	while(pItem && item_first_id != item_last_id) {
		if(pItem->HasAnyFlagSet(EG_OBJ_FLAG_IGNORE_LAYOUT | EG_OBJ_FLAG_HIDDEN | EG_OBJ_FLAG_FLOATING)) {
			pItem = GetNextItem(pObj, m_Reverse, &item_first_id);
			continue;
		}
		EG_Coord_t grow_size = GetObjStyleGrow(pItem, EG_PART_MAIN);
		if(grow_size) {
			EG_Coord_t s = 0;
			for(i = 0; i < t->grow_item_cnt; i++) {
				if(t->grow_dsc[i].pItem == pItem) {
					s = t->grow_dsc[i].final_size;
					break;
				}
			}
			if(m_Row) {
				pItem->m_WidthLayout = 1;
				pItem->m_HeightLayout = 0;
			}
			else {
				pItem->m_HeightLayout = 1;
				pItem->m_WidthLayout = 0;
			}
			if(s != (m_Row ? pItem->m_Rect.GetWidth() : pItem->m_Rect.GetHeight())) {
				pItem->Invalidate();

				EGRect Rect(pItem->m_Rect);
		    if(m_Row) pItem->m_Rect.SetWidth(s); else pItem->m_Rect.SetHeight(s);
				EGEvent::EventSend(pItem, EG_EVENT_SIZE_CHANGED, &Rect);
				EGEvent::EventSend(pItem->GetParent(), EG_EVENT_CHILD_CHANGED, pItem);
				pItem->Invalidate();
			}
		}
		else {
			pItem->m_WidthLayout = 0;
			pItem->m_HeightLayout = 0;
		}
		EG_Coord_t cross_pos = 0;
		switch(m_CrossPlace) {
			case EG_FLEX_ALIGN_CENTER:{
				/*Round up the cross size to avoid rounding error when dividing by 2
                 *The issue comes up e,g, with column direction with center cross direction if an element's width changes*/
				cross_pos = (((t->track_cross_size + 1) & (~1)) - (!m_Row ? pItem->m_Rect.GetWidth() : pItem->m_Rect.GetHeight())) / 2;
				break;	
      }
			case EG_FLEX_ALIGN_END:{
				cross_pos = t->track_cross_size - (!m_Row ? pItem->m_Rect.GetWidth() : pItem->m_Rect.GetHeight());
				break;
      }
			default:
				break;
		}
		if(m_Row && rtl) main_pos -= (m_Row ? pItem->m_Rect.GetWidth() : pItem->m_Rect.GetHeight());
		/*Handle percentage value of translate*/
		EG_Coord_t tr_x = pItem->GetStyleTranslateX(EG_PART_MAIN);
		EG_Coord_t tr_y = pItem->GetStyleTranslateY(EG_PART_MAIN);
		EG_Coord_t w = pItem->GetWidth();
		EG_Coord_t h = pItem->GetHeight();
		if(EG_COORD_IS_PCT(tr_x)) tr_x = (w * EG_COORD_GET_PCT(tr_x)) / 100;
		if(EG_COORD_IS_PCT(tr_y)) tr_y = (h * EG_COORD_GET_PCT(tr_y)) / 100;
		EG_Coord_t diff_x = abs_x - pItem->m_Rect.GetX1() + tr_x;
		EG_Coord_t diff_y = abs_y - pItem->m_Rect.GetY1() + tr_y;
		diff_x += m_Row ? main_pos : cross_pos;
		diff_y += m_Row ? cross_pos : main_pos;
		if(diff_x || diff_y) {
			pItem->Invalidate();
			pItem->m_Rect.IncX1(diff_x);
			pItem->m_Rect.IncX2(diff_x);
			pItem->m_Rect.IncY1(diff_y);
			pItem->m_Rect.IncY2(diff_y);
			pItem->Invalidate();
			pItem->MoveChildrenBy(diff_x, diff_y, false);
		}
		if(!(m_Row && rtl))	main_pos += (m_Row ? pItem->m_Rect.GetWidth() : pItem->m_Rect.GetHeight()) + item_gap + place_gap;
		else main_pos -= item_gap + place_gap;
		pItem = GetNextItem(pObj, m_Reverse, &item_first_id);
	}
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Tell a start coordinate and gap for a placement type.
 */
void EGFlexLayout::PlaceContent(EG_FlexAlign_e place, EG_Coord_t max_size, EG_Coord_t content_size, EG_Coord_t item_cnt,
													EG_Coord_t *start_pos, EG_Coord_t *gap)
{
	if(item_cnt <= 1) {
		switch(place) {
			case EG_FLEX_ALIGN_SPACE_BETWEEN:
			case EG_FLEX_ALIGN_SPACE_AROUND:
			case EG_FLEX_ALIGN_SPACE_EVENLY:
				place = EG_FLEX_ALIGN_CENTER;
				break;
			default:
				break;
		}
	}
	switch(place) {
		case EG_FLEX_ALIGN_CENTER:
			*gap = 0;
			*start_pos += (max_size - content_size) / 2;
			break;
		case EG_FLEX_ALIGN_END:
			*gap = 0;
			*start_pos += max_size - content_size;
			break;
		case EG_FLEX_ALIGN_SPACE_BETWEEN:
			*gap = (EG_Coord_t)(max_size - content_size) / (EG_Coord_t)(item_cnt - 1);
			break;
		case EG_FLEX_ALIGN_SPACE_AROUND:
			*gap += (EG_Coord_t)(max_size - content_size) / (EG_Coord_t)(item_cnt);
			*start_pos += *gap / 2;
			break;
		case EG_FLEX_ALIGN_SPACE_EVENLY:
			*gap = (EG_Coord_t)(max_size - content_size) / (EG_Coord_t)(item_cnt + 1);
			*start_pos += *gap;
			break;
		default:
			*gap = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////

EGObject* EGFlexLayout::GetNextItem(EGObject *pObj, bool m_Reverse, int32_t *item_id)
{
	if(m_Reverse) {
		(*item_id)--;
		if(*item_id >= 0)	return pObj->m_pAttributes->ppChildren[*item_id];
		else return NULL;
	}
	else {
		(*item_id)++;
		if((*item_id) < (int32_t)pObj->m_pAttributes->ChildCount) return pObj->m_pAttributes->ppChildren[*item_id];
		else return NULL;
	}
}

#endif 
