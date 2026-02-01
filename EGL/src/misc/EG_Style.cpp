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

#include "misc/EG_Style.h"
#include "misc/lv_gc.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Types.h"

const uint8_t StyleBuiltinPropertyFlagsTable[_EG_STYLE_NUM_BUILT_IN_PROPS] = {
    [EG_STYLE_PROP_INV] = 0,
    // Group 0
		[EG_STYLE_WIDTH] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_MIN_WIDTH] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_MAX_WIDTH] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_HEIGHT] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_MIN_HEIGHT] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_MAX_HEIGHT] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_X] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_Y] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_ALIGN] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_LAYOUT] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_RADIUS] = 0,
	
    // Group 1
		[EG_STYLE_PAD_TOP] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_PAD_BOTTOM] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_PAD_LEFT] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_PAD_RIGHT] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_PAD_ROW] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_PAD_COLUMN] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_BASE_DIR] = EG_STYLE_PROP_INHERIT | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_CLIP_CORNER] = 0,

    // Group 2
		[EG_STYLE_BG_COLOR] = 0,
		[EG_STYLE_BG_OPA] = 0,
		[EG_STYLE_BG_GRAD_COLOR] = 0,
		[EG_STYLE_BG_GRAD_DIR] = 0,
		[EG_STYLE_BG_MAIN_STOP] = 0,
		[EG_STYLE_BG_GRAD_STOP] = 0,
		[EG_STYLE_BG_GRAD] = 0,
		[EG_STYLE_BG_DITHER_MODE] = 0,
		[EG_STYLE_BG_IMG_SRC] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_BG_IMG_OPA] = 0,
		[EG_STYLE_BG_IMG_RECOLOR] = 0,
		[EG_STYLE_BG_IMG_RECOLOR_OPA] = 0,
		[EG_STYLE_BG_IMG_TILED] = 0,

    // Group 3
		[EG_STYLE_BORDER_COLOR] = 0,
		[EG_STYLE_BORDER_OPA] = 0,
		[EG_STYLE_BORDER_WIDTH] = EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_BORDER_SIDE] = 0,
		[EG_STYLE_BORDER_POST] = 0,
		[EG_STYLE_OUTLINE_WIDTH] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_OUTLINE_COLOR] = 0,
		[EG_STYLE_OUTLINE_OPA] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_OUTLINE_PAD] = EG_STYLE_PROP_EXT_DRAW,

    // Group 4
		[EG_STYLE_SHADOW_WIDTH] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_SHADOW_OFS_X] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_SHADOW_OFS_Y] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_SHADOW_SPREAD] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_SHADOW_COLOR] = 0,
		[EG_STYLE_SHADOW_OPA] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_IMG_OPA] = 0,
		[EG_STYLE_IMG_RECOLOR] = 0,
		[EG_STYLE_IMG_RECOLOR_OPA] = 0,
		[EG_STYLE_LINE_WIDTH] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_LINE_DASH_WIDTH] = 0,
		[EG_STYLE_LINE_DASH_GAP] = 0,
		[EG_STYLE_LINE_ROUNDED] = 0,
		[EG_STYLE_LINE_COLOR] = 0,
		[EG_STYLE_LINE_OPA] = 0,

		// Group 5
		[EG_STYLE_ARC_WIDTH] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_ARC_ROUNDED] = 0,
		[EG_STYLE_ARC_COLOR] = 0,
		[EG_STYLE_ARC_OPA] = 0,
		[EG_STYLE_ARC_IMG_SRC] = 0,
		[EG_STYLE_TEXT_COLOR] = EG_STYLE_PROP_INHERIT,
		[EG_STYLE_TEXT_OPA] = EG_STYLE_PROP_INHERIT,
		[EG_STYLE_TEXT_FONT] = EG_STYLE_PROP_INHERIT | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_TEXT_LETTER_SPACE] = EG_STYLE_PROP_INHERIT | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_TEXT_LINE_SPACE] = EG_STYLE_PROP_INHERIT | EG_STYLE_PROP_LAYOUT_REFRESH,
		[EG_STYLE_TEXT_DECOR] = EG_STYLE_PROP_INHERIT,
		[EG_STYLE_TEXT_ALIGN] = EG_STYLE_PROP_INHERIT | EG_STYLE_PROP_LAYOUT_REFRESH,

    // Group 6
		[EG_STYLE_OPA] = 0,
		[EG_STYLE_OPA_LAYERED] = EG_STYLE_PROP_LAYER_REFR,
		[EG_STYLE_COLOR_FILTER_DSC] = EG_STYLE_PROP_INHERIT,
		[EG_STYLE_COLOR_FILTER_OPA] = EG_STYLE_PROP_INHERIT,
		[EG_STYLE_ANIM] = 0,
		[EG_STYLE_ANIM_TIME] = 0,
		[EG_STYLE_ANIM_SPEED] = 0,
		[EG_STYLE_TRANSITION] = 0,
		[EG_STYLE_BLEND_MODE] = EG_STYLE_PROP_LAYER_REFR,
		[EG_STYLE_TRANSFORM_WIDTH] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_TRANSFORM_HEIGHT] = EG_STYLE_PROP_EXT_DRAW,
		[EG_STYLE_TRANSLATE_X] = EG_STYLE_PROP_LAYOUT_REFRESH | EG_STYLE_PROP_PARENT_LAYOUT_REFRESH,
		[EG_STYLE_TRANSLATE_Y] = EG_STYLE_PROP_LAYOUT_REFRESH | EG_STYLE_PROP_PARENT_LAYOUT_REFRESH,
		[EG_STYLE_TRANSFORM_ZOOM] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYER_REFR,
		[EG_STYLE_TRANSFORM_ANGLE] = EG_STYLE_PROP_EXT_DRAW | EG_STYLE_PROP_LAYER_REFR,
		[EG_STYLE_TRANSFORM_PIVOT_X] = 0,
		[EG_STYLE_TRANSFORM_PIVOT_Y] = 0,
};

uint32_t StyleCustomPropertyFlagsTableSize = 0;

uint16_t EGStyle::m_LastCustomPropertyID = (uint16_t)_EG_STYLE_LAST_BUILT_IN_PROP;

/////////////////////////////////////////////////////////////////////////////////////////

EGStyle::EGStyle() :
  m_SingleProperty(0),
  m_HasGroup(0),
  m_PropertyCount(0)
{
  m_VP.m_SingleValue.Number = 0;
#if EG_USE_ASSERT_STYLE
	if(m_Sentinel == EG_STYLE_SENTINEL_VALUE && m_PropertyCount > 1) {
		EG_LOG_WARN("Style might be already inited. (Potential memory leak)");
	}
	m_Sentinel = EG_STYLE_SENTINEL_VALUE;
#endif
  Initialise();
}

/////////////////////////////////////////////////////////////////////////////////////////

EGStyle::~EGStyle()
{
	if(m_PropertyCount > 1) EG_FreeMem(m_VP.m_pValuesProperties);
  m_PropertyCount = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGStyle::Initialise(void)
{
  m_VP.m_pValuesProperties = nullptr;
	m_VP.m_SingleValue.Number = 0;
  m_SingleProperty = 0;
  m_HasGroup = 0;
  m_PropertyCount = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGStyle::Reset(void)
{
	if(m_SingleProperty == EG_STYLE_PROP_ANY) {
		EG_LOG_ERROR("Cannot reset const style");
		return;
	}
//	EG_LOG_WARN("Style: %p, Count: %d, Properties: %p", (void*)this, m_PropertyCount, m_VP.m_pValuesProperties);
	if(m_PropertyCount > 1) EG_FreeMem(m_VP.m_pValuesProperties);
  m_VP.m_pValuesProperties = nullptr;
	m_VP.m_SingleValue.Number = 0;    // also set pointers to null
  m_SingleProperty = 0;
  m_HasGroup = 0;
  m_PropertyCount = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

EGStyleProperty_e EGStyle::RegisterProperty(uint8_t Flag)
{
	if(EG_GC_ROOT(StyleCustomPropertyFlagLookupTable) == NULL) {
		StyleCustomPropertyFlagsTableSize = 0;
		m_LastCustomPropertyID = (uint16_t)_EG_STYLE_LAST_BUILT_IN_PROP;
	}
	if(((m_LastCustomPropertyID + 1) & EG_STYLE_PROP_META_MASK) != 0) {
		EG_LOG_ERROR("No more custom property IDs available");
		return EG_STYLE_PROP_INV;
	}
	// Allocate the lookup table if it's not yet available.
	size_t required_size = (m_LastCustomPropertyID + 1 - _EG_STYLE_LAST_BUILT_IN_PROP);
	if(StyleCustomPropertyFlagsTableSize < required_size) {
		required_size = (required_size + 31) & ~31;		// Round required_size up to the nearest 32-byte value 
		EG_ASSERT_MSG(required_size > 0, "required size has become 0?");
		uint8_t *old_p = EG_GC_ROOT(StyleCustomPropertyFlagLookupTable);
		uint8_t *new_p = (uint8_t*)EG_ReallocMem(old_p, required_size * sizeof(uint8_t));
		if(new_p == NULL) {
			EG_LOG_ERROR("Unable to allocate space for custom property lookup table");
			return EG_STYLE_PROP_INV;
		}
		EG_GC_ROOT(StyleCustomPropertyFlagLookupTable) = new_p;
		StyleCustomPropertyFlagsTableSize = required_size;
	}
	m_LastCustomPropertyID++;
	// This should never happen - we should bail out above 
	EG_ASSERT_NULL(EG_GC_ROOT(StyleCustomPropertyFlagLookupTable));
	EG_GC_ROOT(StyleCustomPropertyFlagLookupTable)
	[m_LastCustomPropertyID - _EG_STYLE_NUM_BUILT_IN_PROPS] = Flag;
	return (EGStyleProperty_e)m_LastCustomPropertyID;
}

/////////////////////////////////////////////////////////////////////////////////////////

EGStyleProperty_e EGStyle::GetCustomPropertyCount(void)
{
	return (EGStyleProperty_e)(m_LastCustomPropertyID - _EG_STYLE_LAST_BUILT_IN_PROP);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EGStyle::RemoveProperty(EGStyleProperty_e Property)
{
	if(m_SingleProperty == EG_STYLE_PROP_ANY) {
		EG_LOG_ERROR("Cannot remove Property from const style");
		return false;
	}
	if(m_PropertyCount == 0) return false;
	if(m_PropertyCount == 1) {
		if(EG_STYLE_PROP_ID_MASK(m_SingleProperty) == Property) {
			m_SingleProperty = EG_STYLE_PROP_INV;
			m_PropertyCount = 0;
			return true;
		}
		return false;
	}
	uint8_t *pTemp = m_VP.m_pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t);
  uint16_t *pOldProperties = (uint16_t*)pTemp;
	for(uint32_t i = 0; i < m_PropertyCount; i++) {
		if(EG_STYLE_PROP_ID_MASK(pOldProperties[i]) == Property) {
			EG_StyleValue_t *pOldValues = (EG_StyleValue_t*)m_VP.m_pValuesProperties;
			if(m_PropertyCount == 2) {  // make remaining property a single one
				m_PropertyCount = 1;
				m_SingleProperty = (i == 0) ? pOldProperties[1] : pOldProperties[0];
				m_VP.m_SingleValue = (i == 0) ? pOldValues[1] : pOldValues[0];
			}
			else {
				size_t Size = (m_PropertyCount - 1) * (sizeof(EG_StyleValue_t) + sizeof(uint16_t));
				uint8_t *pValuesProperties = (uint8_t*)EG_AllocMem(Size);
				if(pValuesProperties == nullptr) return false;
				m_VP.m_pValuesProperties = pValuesProperties;
				m_PropertyCount--;
				uint16_t *pNewProperties = (uint16_t *)((uint8_t*)pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t));
				EG_StyleValue_t *pNewValues = (EG_StyleValue_t *)pValuesProperties;
				uint32_t j = 0;
				for(i = 0; j <= m_PropertyCount; j++) { // because prop_cnt already reduced but all the...
					if(pOldProperties[j] != Property) {       // old props. needs to be checked.
						pNewValues[i] = pOldValues[j];
						pNewProperties[i++] = pOldProperties[j];
					}
				}
			}
			EG_FreeMem(pOldValues);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGStyle::SetProperty(EGStyleProperty_e Property, EG_StyleValue_t Value)
{
	SetPropertyCore(Property, Value, SetPropertyHelper);
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGStyle::SetPropertyMeta(EGStyleProperty_e Property, uint16_t Meta)
{
	SetPropertyCore((EGStyleProperty_e)((uint16_t)Property | Meta), m_StyleNullValue, SetPropertyMetaHelper);
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_StyleResult_t EGStyle::GetProperty(EGStyleProperty_e Property, EG_StyleValue_t *pValue)
{
	return GetPropertyCore(Property, pValue);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool EGStyle::IsEmpty(void)
{
	return m_PropertyCount == 0 ? true : false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EGStyle::SetPropertyCore(EGStyleProperty_e PropertyMeta, EG_StyleValue_t Value,
							void (*SetValueHelper)(EGStyleProperty_e, EG_StyleValue_t, uint16_t *, EG_StyleValue_t *))
{
	if(m_SingleProperty == EG_STYLE_PROP_ANY) {
		EG_LOG_ERROR("Cannot set property of constant style");
		return;
	}
	EGStyleProperty_e PropertyID = EG_STYLE_PROP_ID_MASK(PropertyMeta);
//  if(PropertyMeta == 96) ESP_LOGI("[Style ]", "Set Core Property:%d, Style:%p, Count:%d, Val:%d", PropertyID, (void*)this, m_PropertyCount, Value.Number);
	if(m_PropertyCount > 1) {   //  Already has multiple values so insert the new one
		uint16_t *pProperties = (uint16_t*)((uint8_t*)m_VP.m_pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t));
		for(int32_t i = m_PropertyCount - 1; i >= 0; i--) { // Check for property already present
			if(EG_STYLE_PROP_ID_MASK(pProperties[i]) == PropertyID) {
				EG_StyleValue_t *pValues = (EG_StyleValue_t*)m_VP.m_pValuesProperties;
				SetValueHelper(PropertyMeta, Value, &pProperties[i], &pValues[i]); // update with new value
				return;
			}
		}
		size_t Size = (m_PropertyCount + 1) * (sizeof(EG_StyleValue_t) + sizeof(uint16_t));
		uint8_t *pValuesProperties = (uint8_t*)EG_ReallocMem(m_VP.m_pValuesProperties, Size);
		if(pValuesProperties == nullptr) return;
		m_VP.m_pValuesProperties = pValuesProperties;
		pProperties = (uint16_t*)((uint8_t*)pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t));
		for(int32_t i = m_PropertyCount - 1; i >= 0; i--) {		// Shift all props to make space for the new item
			pProperties[i + sizeof(EG_StyleValue_t) / sizeof(uint16_t)] = pProperties[i];
		}
		m_PropertyCount++;
		pProperties = (uint16_t*)((uint8_t*)pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t));
		EG_StyleValue_t *pValues = (EG_StyleValue_t *)pValuesProperties;   // Set the new property and Value
		SetValueHelper(PropertyMeta, Value, &pProperties[m_PropertyCount - 1], &pValues[m_PropertyCount - 1]);
	}
	else if(m_PropertyCount == 1) {       // already hase a single property
		if(EG_STYLE_PROP_ID_MASK(m_SingleProperty) == PropertyID) {     // if it's the same just update the value
			SetValueHelper(PropertyMeta, Value, &m_SingleProperty, &m_VP.m_SingleValue);
			return;
		}                         // change from single to multiple properties
		m_PropertyCount++;
		size_t Size = (m_PropertyCount) * (sizeof(EG_StyleValue_t) + sizeof(uint16_t));
		uint8_t *pValuesProperties = (uint8_t*)EG_AllocMem(Size);
		if(pValuesProperties == nullptr) return;
		EG_StyleValue_t SingleValue = m_VP.m_SingleValue;
		m_VP.m_pValuesProperties = pValuesProperties;
		uint16_t *pProperties = (uint16_t*)((uint8_t*)pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t));
		EG_StyleValue_t *pValues = (EG_StyleValue_t*)pValuesProperties;
		pProperties[0] = m_SingleProperty;
		pValues[0] = SingleValue;
		SetValueHelper(PropertyMeta, Value, &pProperties[1], &pValues[1]);
	}
	else {            // Just a single value
		m_PropertyCount = 1;
		SetValueHelper(PropertyMeta, Value, &m_SingleProperty, &m_VP.m_SingleValue);
	}
	uint8_t Group = GetPropertyGroup(PropertyID);
	m_HasGroup |= 1 << Group;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitialiseTransitionDiscriptor(EG_StyleTransitionDiscriptor_t *pTransitionDiscriptor, const EGStyleProperty_e Properties[],
																	EG_AnimatePathCB_t PathCB, uint32_t Time, uint32_t Delay, void *pUserData)
{
	EG_ZeroMem(pTransitionDiscriptor, sizeof(EG_StyleTransitionDiscriptor_t));
	pTransitionDiscriptor->pProperties = Properties;
	pTransitionDiscriptor->PathCB = PathCB == nullptr ? EGAnimate::PathLinear : PathCB;
	pTransitionDiscriptor->Time = Time;
	pTransitionDiscriptor->Delay = Delay;
	pTransitionDiscriptor->m_pParam = pUserData;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_StyleValue_t GetDefaultProperty(EGStyleProperty_e Property)
{
EG_StyleValue_t Value;

	switch(Property) {
		case EG_STYLE_TRANSFORM_ZOOM:
			Value.Number = EG_SCALE_NONE;
			break;
		case EG_STYLE_BG_COLOR:
			Value.Color = EG_ColorWhite();
			break;
		case EG_STYLE_BG_GRAD_COLOR:
		case EG_STYLE_BORDER_COLOR:
		case EG_STYLE_SHADOW_COLOR:
		case EG_STYLE_OUTLINE_COLOR:
		case EG_STYLE_ARC_COLOR:
		case EG_STYLE_LINE_COLOR:
		case EG_STYLE_TEXT_COLOR:
		case EG_STYLE_IMG_RECOLOR:
			Value.Color = EG_ColorBlack();
			break;
		case EG_STYLE_OPA:
		case EG_STYLE_OPA_LAYERED:
		case EG_STYLE_BORDER_OPA:
		case EG_STYLE_TEXT_OPA:
		case EG_STYLE_IMG_OPA:
		case EG_STYLE_BG_IMG_OPA:
		case EG_STYLE_OUTLINE_OPA:
		case EG_STYLE_SHADOW_OPA:
		case EG_STYLE_LINE_OPA:
		case EG_STYLE_ARC_OPA:
			Value.Number = EG_OPA_COVER;
			break;
		case EG_STYLE_BG_GRAD_STOP:
			Value.Number = 255;
			break;
		case EG_STYLE_BORDER_SIDE:
			Value.Number = EG_BORDER_SIDE_FULL;
			break;
		case EG_STYLE_TEXT_FONT:
			Value.pPtr = EG_FONT_DEFAULT;
			break;
		case EG_STYLE_MAX_WIDTH:
		case EG_STYLE_MAX_HEIGHT:
			Value.Number = EG_COORD_MAX;
			break;
		default:
			Value.pPtr = nullptr;
			Value.Number = 0;
			break;
	}
	return Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

uint8_t LookupPropertyFlags(EGStyleProperty_e Property)
{
	extern const uint8_t StyleBuiltinPropertyFlagsTable[];
	extern uint32_t StyleCustomPropertyFlagsTableSize;
	if(Property == EG_STYLE_PROP_ANY) return EG_STYLE_PROP_ALL; // Any Property can have any flag
	if(Property == EG_STYLE_PROP_INV) return 0;

	if(Property < _EG_STYLE_NUM_BUILT_IN_PROPS)	return StyleBuiltinPropertyFlagsTable[Property];
	Property = (EGStyleProperty_e)((int)Property - _EG_STYLE_NUM_BUILT_IN_PROPS);
	if(EG_GC_ROOT(StyleCustomPropertyFlagLookupTable) != NULL && Property < StyleCustomPropertyFlagsTableSize)
		return EG_GC_ROOT(StyleCustomPropertyFlagLookupTable)[Property];
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

uint8_t GetPropertyGroup(EGStyleProperty_e Property)
{
	uint16_t Group = (Property & 0x1FF) >> 4;
	if(Group > 7) Group = 7; // The MSB marks all the custom properties
	return (uint8_t)Group;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SetPropertyHelper(EGStyleProperty_e Property, EG_StyleValue_t Value, uint16_t *pPropertyStorage, EG_StyleValue_t *pValueStorage)
{
	*pPropertyStorage = Property;
	*pValueStorage = Value;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SetPropertyMetaHelper(EGStyleProperty_e Property, EG_StyleValue_t Value, uint16_t *pPropertyStorage,	EG_StyleValue_t *pValueStorage)
{
	EG_UNUSED(Value);
	EG_UNUSED(pValueStorage);
	*pPropertyStorage = Property; // meta is OR-ed into the Property ID already 
}


