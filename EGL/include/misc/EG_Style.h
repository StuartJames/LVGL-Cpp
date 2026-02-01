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

#include <stdbool.h>
#include <stdint.h>
#include "../font/EG_Font.h"
#include "EG_Color.h"
#include "EG_Rect.h"
#include "EG_Animate.h"
#include "EG_Text.h"
#include "EG_Types.h"
#include "EG_Assert.h"
#include "lv_bidi.h"

#include "esp_log.h"

///////////////////////////////////////////////////////////////////////////////

#define EG_STYLE_SENTINEL_VALUE             0xAABBCCDD

#define EG_STYLE_PROP_FLAG_NONE             (0)
#define EG_STYLE_PROP_INHERIT               (1 << 0)  // Inherited
#define EG_STYLE_PROP_EXT_DRAW              (1 << 1)  // Requires ext. draw size update when changed
#define EG_STYLE_PROP_LAYOUT_REFRESH        (1 << 2)  // Requires layout update when changed
#define EG_STYLE_PROP_PARENT_LAYOUT_REFRESH (1 << 3)  // Requires layout update on parent when changed
#define EG_STYLE_PROP_LAYER_REFR            (1 << 4)  // Affects layer handling
#define EG_STYLE_PROP_ALL                   (0x1F)     // Indicating all flags

#define EG_SCALE_NONE                    256        // Value for not zooming the image
EG_EXPORT_CONST_INT(EG_SCALE_NONE);

#define EG_STYLE_PROP_META_INHERIT 0x8000
#define EG_STYLE_PROP_META_INITIAL 0x4000
#define EG_STYLE_PROP_META_MASK (EG_STYLE_PROP_META_INHERIT | EG_STYLE_PROP_META_INITIAL)

#define EG_STYLE_PROP_ID_MASK(prop) ((EGStyleProperty_e)((prop) & ~EG_STYLE_PROP_META_MASK))

/////////////////////////////////////////////////////////////////////////////////////////

typedef enum : uint8_t {
  EG_BLEND_MODE_NORMAL,     // Simply mix according to the opacity value
  EG_BLEND_MODE_ADDITIVE,   // Add the respective color channels
  EG_BLEND_MODE_SUBTRACTIVE,// Subtract the foreground from the background
  EG_BLEND_MODE_MULTIPLY,   // Multiply the foreground and background
  EG_BLEND_MODE_REPLACE,    // Replace background with foreground in the area
} EG_BlendMode_e;


typedef enum : uint8_t {
  EG_TEXT_DECOR_NONE          = 0x00,
  EG_TEXT_DECOR_UNDERLINE     = 0x01,
  EG_TEXT_DECOR_STRIKETHROUGH = 0x02,
} EG_TextDecor_e;


typedef enum : uint8_t {
  EG_BORDER_SIDE_NONE     = 0x00,
  EG_BORDER_SIDE_BOTTOM   = 0x01,
  EG_BORDER_SIDE_TOP      = 0x02,
  EG_BORDER_SIDE_LEFT     = 0x04,
  EG_BORDER_SIDE_RIGHT    = 0x08,
  EG_BORDER_SIDE_FULL     = 0x0F,
  EG_BORDER_SIDE_INTERNAL = 0x10, // FOR matrix-like objects (e.g. Button matrix)
}  EG_BorderSide_e;

typedef uint8_t EG_BorderSide_t;


typedef enum : uint8_t {
  EG_GRAD_DIR_NONE, // No gradient (the `grad_color` property is ignored)
  EG_GRAD_DIR_VER,  // Vertical (top to bottom) gradient
  EG_GRAD_DIR_HOR,  // Horizontal (left to right) gradient
} EG_GradDirection_e;


enum {
  EG_DITHER_NONE,     // No dithering, colors are just quantized to the output resolution
  EG_DITHER_ORDERED,  // Ordered dithering. Faster to compute and use less memory but lower quality
  EG_DITHER_ERR_DIFF, // Error diffusion mode. Slower to compute and use more memory but give highest dither quality
};


typedef enum : uint16_t {
  EG_STYLE_PROP_INV               = 0,

  // Group 0
  EG_STYLE_WIDTH                  = 1,
  EG_STYLE_MIN_WIDTH              = 2,
  EG_STYLE_MAX_WIDTH              = 3,
  EG_STYLE_HEIGHT                 = 4,
  EG_STYLE_MIN_HEIGHT             = 5,
  EG_STYLE_MAX_HEIGHT             = 6,
  EG_STYLE_X                      = 7,
  EG_STYLE_Y                      = 8,
  EG_STYLE_ALIGN                  = 9,
  EG_STYLE_LAYOUT                 = 10,
  EG_STYLE_RADIUS                 = 11,

  // Group 1
  EG_STYLE_PAD_TOP                = 12,
  EG_STYLE_PAD_BOTTOM             = 13,
  EG_STYLE_PAD_LEFT               = 14,
  EG_STYLE_PAD_RIGHT              = 15,
  EG_STYLE_PAD_ROW                = 16,
  EG_STYLE_PAD_COLUMN             = 17,
  EG_STYLE_BASE_DIR               = 18,
  EG_STYLE_CLIP_CORNER            = 19,

  // Group 2
  EG_STYLE_BG_COLOR               = 20,
  EG_STYLE_BG_OPA                 = 21,
  EG_STYLE_BG_GRAD_COLOR          = 22,
  EG_STYLE_BG_GRAD_DIR            = 23,
  EG_STYLE_BG_MAIN_STOP           = 24,
  EG_STYLE_BG_GRAD_STOP           = 25,
  EG_STYLE_BG_GRAD                = 26,
  EG_STYLE_BG_DITHER_MODE         = 27,
  EG_STYLE_BG_IMG_SRC             = 28,
  EG_STYLE_BG_IMG_OPA             = 29,
  EG_STYLE_BG_IMG_RECOLOR         = 30,
  EG_STYLE_BG_IMG_RECOLOR_OPA     = 31,
  EG_STYLE_BG_IMG_TILED           = 32,

  // Group 3
  EG_STYLE_BORDER_COLOR           = 33,
  EG_STYLE_BORDER_OPA             = 34,
  EG_STYLE_BORDER_WIDTH           = 35,
  EG_STYLE_BORDER_SIDE            = 36,
  EG_STYLE_BORDER_POST            = 37,
  EG_STYLE_OUTLINE_WIDTH          = 38,
  EG_STYLE_OUTLINE_COLOR          = 39,
  EG_STYLE_OUTLINE_OPA            = 40,
  EG_STYLE_OUTLINE_PAD            = 41,

  // Group 4
  EG_STYLE_SHADOW_WIDTH           = 42,
  EG_STYLE_SHADOW_OFS_X           = 43,
  EG_STYLE_SHADOW_OFS_Y           = 44,
  EG_STYLE_SHADOW_SPREAD          = 45,
  EG_STYLE_SHADOW_COLOR           = 46,
  EG_STYLE_SHADOW_OPA             = 47,
  EG_STYLE_IMG_OPA                = 48,
  EG_STYLE_IMG_RECOLOR            = 49,
  EG_STYLE_IMG_RECOLOR_OPA        = 50,
  EG_STYLE_LINE_WIDTH             = 51,
  EG_STYLE_LINE_DASH_WIDTH        = 52,
  EG_STYLE_LINE_DASH_GAP          = 53,
  EG_STYLE_LINE_ROUNDED           = 54,
  EG_STYLE_LINE_COLOR             = 55,
  EG_STYLE_LINE_OPA               = 56,

  // Group 5
  EG_STYLE_ARC_WIDTH              = 57,
  EG_STYLE_ARC_ROUNDED            = 58,
  EG_STYLE_ARC_COLOR              = 59,
  EG_STYLE_ARC_OPA                = 60,
  EG_STYLE_ARC_IMG_SRC            = 61,
  EG_STYLE_TEXT_COLOR             = 62,
  EG_STYLE_TEXT_OPA               = 63,
  EG_STYLE_TEXT_FONT              = 64,
  EG_STYLE_TEXT_LETTER_SPACE      = 65,
  EG_STYLE_TEXT_LINE_SPACE        = 66,
  EG_STYLE_TEXT_DECOR             = 67,
  EG_STYLE_TEXT_ALIGN             = 68,

  // Group 6
  EG_STYLE_OPA                    = 69,
  EG_STYLE_OPA_LAYERED            = 70,
  EG_STYLE_COLOR_FILTER_DSC       = 71,
  EG_STYLE_COLOR_FILTER_OPA       = 72,
  EG_STYLE_ANIM                   = 73,
  EG_STYLE_ANIM_TIME              = 74,
  EG_STYLE_ANIM_SPEED             = 75,
  EG_STYLE_TRANSITION             = 76,
  EG_STYLE_BLEND_MODE             = 77,
  EG_STYLE_TRANSFORM_WIDTH        = 78,
  EG_STYLE_TRANSFORM_HEIGHT       = 79,
  EG_STYLE_TRANSLATE_X            = 80,
  EG_STYLE_TRANSLATE_Y            = 81,
  EG_STYLE_TRANSFORM_ZOOM         = 82,
  EG_STYLE_TRANSFORM_ANGLE        = 83,
  EG_STYLE_TRANSFORM_PIVOT_X      = 84,
  EG_STYLE_TRANSFORM_PIVOT_Y      = 85,

  _EG_STYLE_LAST_BUILT_IN_PROP     = 85,
  _EG_STYLE_NUM_BUILT_IN_PROPS     = _EG_STYLE_LAST_BUILT_IN_PROP + 1,

  EG_STYLE_PROP_ANY                = 0xFFFF,
  _EG_STYLE_PROP_CONST             = 0xFFFF //  magic value for const styles 
} EGStyleProperty_e;


enum {
  EG_STYLE_RES_NOT_FOUND,
  EG_STYLE_RES_FOUND,
  EG_STYLE_RES_INHERIT
};

/////////////////////////////////////////////////////////////////////////////////////////

typedef uint8_t EG_DitherMode_t;

typedef struct {
    EG_Color_t color;   // The stop color 
    uint8_t    frac;    // The stop position in 1/255 unit 
} EG_GradientStop_t;


typedef struct {
    EG_GradientStop_t   stops[EG_GRADIENT_MAX_STOPS]; // A gradient stop array 
    uint8_t              stops_count;                  // The number of used stops in the array 
    EG_GradDirection_e   dir : 3;                      // The gradient direction.
                                                       // Any of EG_GRAD_DIR_HOR, EG_GRAD_DIR_VER, EG_GRAD_DIR_NONE 
    EG_DitherMode_t     dither : 3;                   // Whether to dither the gradient or not.
                                                       // Any of EG_DITHER_NONE, EG_DITHER_ORDERED, EG_DITHER_ERR_DIFF 
} EG_GradDescriptor_t;


typedef union {
    int32_t     Number;   // Number integer number (opacity, enums, booleans or "normal" numbers)
    const void *pPtr;     // Constant pointers  (font, cone text, etc)
    EG_Color_t  Color;    // Colors
} EG_StyleValue_t;

typedef uint8_t EG_StyleResult_t;


typedef struct {
    const EGStyleProperty_e *pProperties; // An array with the properties to animate.
    void                  *m_pParam;   // A custom user data that will be passed to the animation's user_data 
    EG_AnimatePathCB_t    PathCB;       // A path for the animation.
    uint32_t              Time;         // Duration of the transition in [ms]
    uint32_t              Delay;        // Delay before the transition in [ms]
} EG_StyleTransitionDiscriptor_t;


typedef struct {
    EGStyleProperty_e    Property;
    EG_StyleValue_t      Value;
} EG_StylePropValue_t;

/////////////////////////////////////////////////////////////////////////////////////////

uint8_t           GetPropertyGroup(EGStyleProperty_e Property);
void              SetPropertyHelper(EGStyleProperty_e Property, EG_StyleValue_t Value, uint16_t *pPropertyStorage, EG_StyleValue_t *pValueStorage);
void              SetPropertyMetaHelper(EGStyleProperty_e Property, EG_StyleValue_t Value, uint16_t *pPropertyStorage,	EG_StyleValue_t *pValueStorage);
void              InitialiseTransitionDiscriptor(EG_StyleTransitionDiscriptor_t *pTransitionDiscriptor, const EGStyleProperty_e Properties[],
                    EG_AnimatePathCB_t PathCB, uint32_t Time, uint32_t Delay, void *pUserData);
EG_StyleValue_t   GetDefaultProperty(EGStyleProperty_e Property);
bool              PropertyHasFlag(EGStyleProperty_e Property, uint8_t Flag);
uint8_t           LookupPropertyFlags(EGStyleProperty_e Property);

/////////////////////////////////////////////////////////////////////////////////////////

class EGStyle
{
public:
                    EGStyle();
                    ~EGStyle();
  void              Reset(void);
  void              Initialise(void);
  EGStyleProperty_e GetCustomPropertyCount(void);
  bool              RemoveProperty(EGStyleProperty_e Property);
  void              SetProperty(EGStyleProperty_e Property, EG_StyleValue_t Value);
  void              SetPropertyMeta(EGStyleProperty_e Property, uint16_t Neta);
  EG_Result_t       GetProperty(EGStyleProperty_e Property, EG_StyleValue_t *pValue);
  bool              IsEmpty(void);
  void              ConstantStyleInitialise(uint8_t Count, EG_StylePropValue_t *pPropertyArray);

  void              SetWidth(EG_Coord_t value);
  void              SetMinWidth(EG_Coord_t value);
  void              SetMaxWidth(EG_Coord_t value);
  void              SetHeight(EG_Coord_t value);
  void              SetMinHeight(EG_Coord_t value);
  void              SetMaxHeight(EG_Coord_t value);
  void              SetX(EG_Coord_t value);
  void              SetY(EG_Coord_t value);
  void              SetAlign(EG_AlignType_e value);
  void              SetTransformWidth(EG_Coord_t value);
  void              SetTransformHeight(EG_Coord_t value);
  void              SetTranslateX(EG_Coord_t value);
  void              SetTranslateY(EG_Coord_t value);
  void              SetTransformZoom(EG_Coord_t value);
  void              SetTransformAngle(EG_Coord_t value);
  void              SetTransformPivotX(EG_Coord_t value);
  void              SetTransformPivotY(EG_Coord_t value);
  void              SetPaddingTop(EG_Coord_t value);
  void              SetPaddingBottom(EG_Coord_t value);
  void              SetPaddingLeft(EG_Coord_t value);
  void              SetPaddingRight(EG_Coord_t value);
  void              SetPaddingRow(EG_Coord_t value);
  void              SetPaddingColumn(EG_Coord_t value);
  void              SetBackColor(EG_Color_t value);
  void              SetBackOPA(EG_OPA_t value);
  void              SetBackGradientColor(EG_Color_t value);
  void              SetBackGradientDirection(EG_GradDirection_e value);
  void              SetBackMainStop(EG_Coord_t value);
  void              SetBackGradientStop(EG_Coord_t value);
  void              SetBackGradient(const EG_GradDescriptor_t *value);
  void              SetBackDitherMode(EG_DitherMode_t value);
  void              SetBackImageSource(const void *value);
  void              SetBackImageOPA(EG_OPA_t value);
  void              SetBackImageRecolor(EG_Color_t value);
  void              SetBackImageRecolorOPA(EG_OPA_t value);
  void              SetBackImageTiled(bool value);
  void              SetBorderColor(EG_Color_t value);
  void              SetBorderOPA(EG_OPA_t value);
  void              SetBorderWidth(EG_Coord_t value);
  void              SetBorderSide(EG_BorderSide_t value);
  void              SetBorderPost(bool value);
  void              SetOutlineWidth(EG_Coord_t value);
  void              SetOutlineColor(EG_Color_t value);
  void              SetOutlineOPA(EG_OPA_t value);
  void              SetOutlinePad(EG_Coord_t value);
  void              SetShadowWidth(EG_Coord_t value);
  void              SetShadowOffsetX(EG_Coord_t value);
  void              SetShadowOffsetY(EG_Coord_t value);
  void              SetShadowSpread(EG_Coord_t value);
  void              SetShadowColor(EG_Color_t value);
  void              SetShadowOPA(EG_OPA_t value);
  void              SetImageOPA(EG_OPA_t value);
  void              SetImageRecolor(EG_Color_t value);
  void              SetimageRecolorOPA(EG_OPA_t value);
  void              SetLineWidth(EG_Coord_t value);
  void              SetLineDashWidth(EG_Coord_t value);
  void              SetLineDashGap(EG_Coord_t value);
  void              SetLineRounded(bool value);
  void              SetLineColor(EG_Color_t value);
  void              SetLineOPA(EG_OPA_t value);
  void              SetArcWidth(EG_Coord_t value);
  void              SetArcRounded(bool value);
  void              SetArcColor(EG_Color_t value);
  void              SetArcOPA(EG_OPA_t value);
  void              SetArcImageSource(const void *value);
  void              SetTextColor(EG_Color_t value);
  void              SetTextOPA(EG_OPA_t value);
  void              SetTextFont(const EG_Font_t *value);
  void              SetTextKerning(EG_Coord_t value);
  void              SetTextLineSpace(EG_Coord_t value);
  void              SetTextDecoration(EG_TextDecor_e value);
  void              SetTextAlign(EG_TextAlignment_t value);
  void              SetRadius(EG_Coord_t value);
  void              SetClipCorner(bool value);
  void              SetOPA(EG_OPA_t value);
  void              SetOPALayered(EG_OPA_t value);
  void              SetColorFilterDiscriptor(const EG_ColorFilterProps_t *value);
  void              SetColorFilterOPA(EG_OPA_t value);
  void              SetAnimate(const EGAnimate *value);
  void              SetAnimateTime(uint32_t value);
  void              SetAnimateSpeed(uint32_t value);
  void              SetTransition(const EG_StyleTransitionDiscriptor_t *value);
  void              SetBlendMode(EG_BlendMode_e value);
  void              SetLayout(uint16_t value);
  void              SetBaseDirection(EG_BaseDirection_e value);
  void              SetSize(EG_Coord_t Value);
  void              SetPaddingAll(EG_Coord_t Value);
  void              SetHorizontalPadding(EG_Coord_t Value);
  void              SetVerticalPadding(EG_Coord_t Value);
  void              SetPaddingGap(EG_Coord_t Value);

  static EGStyleProperty_e RegisterProperty(uint8_t Flag);

  #if EG_USE_ASSERT_STYLE
    uint32_t    m_Sentinel;
#endif

  union {                                       // All 32bit values.
    EG_StyleValue_t     m_SingleValue;          // If there is only one property store it directly.
    uint8_t             *m_pValuesProperties;   // For more properties allocate an array
    EG_StylePropValue_t *m_pConstProperty;      // a pointer to a property array
  } m_VP;
  uint16_t            m_SingleProperty; 
  uint8_t             m_HasGroup;
  uint8_t             m_PropertyCount;

private:
  void                SetPropertyCore(EGStyleProperty_e prop_and_meta, EG_StyleValue_t value,
											  void (*SetValueHelper)(EGStyleProperty_e, EG_StyleValue_t, uint16_t *, EG_StyleValue_t *));
  EG_StyleResult_t    GetPropertyCore(EGStyleProperty_e Property, EG_StyleValue_t *pValue);


  static uint16_t         m_LastCustomPropertyID;
  const EG_StyleValue_t  m_StyleNullValue = {.Number = 0};

#if EG_USE_ASSERT_STYLE
	uint32_t                m_Sentinel;
#endif

};

/////////////////////////////////////////////////////////////////////////////////////////

inline EG_StyleResult_t EGStyle::GetPropertyCore(EGStyleProperty_e Property, EG_StyleValue_t *pValue)
{
  if(m_SingleProperty == EG_STYLE_PROP_ANY) {       // Constant Properties
    const EG_StylePropValue_t *pConstProperty;
    for(uint32_t i = 0; i < m_PropertyCount; i++) {
      pConstProperty = m_VP.m_pConstProperty + i;
      EGStyleProperty_e PropertyID = EG_STYLE_PROP_ID_MASK(pConstProperty->Property);
      if(PropertyID == Property) {
        if(pConstProperty->Property & EG_STYLE_PROP_META_INHERIT) return EG_STYLE_RES_INHERIT;
        *pValue = (pConstProperty->Property & EG_STYLE_PROP_META_INITIAL) ? GetDefaultProperty(PropertyID) : pConstProperty->Value;
        return EG_STYLE_RES_FOUND;
      }
    }
    return EG_STYLE_RES_NOT_FOUND;
  }
  if(m_PropertyCount == 0) return EG_STYLE_RES_NOT_FOUND;
  if(m_PropertyCount > 1) {                         // multiple properties
    uint16_t *pProperties = (uint16_t*)((uint8_t*)m_VP.m_pValuesProperties + m_PropertyCount * sizeof(EG_StyleValue_t));
    for(uint32_t i = 0; i < m_PropertyCount; i++) {
      EGStyleProperty_e PropertyID = EG_STYLE_PROP_ID_MASK(pProperties[i]);
      if(PropertyID == Property) {
        if(pProperties[i] & EG_STYLE_PROP_META_INHERIT) return EG_STYLE_RES_INHERIT;
        if(pProperties[i] & EG_STYLE_PROP_META_INITIAL) *pValue = GetDefaultProperty(PropertyID);
        else{
          EG_StyleValue_t *pValues = (EG_StyleValue_t*)m_VP.m_pValuesProperties;
          *pValue = pValues[i];
        }
        return EG_STYLE_RES_FOUND;
      }
    }
  }
  else if(EG_STYLE_PROP_ID_MASK(m_SingleProperty) == Property) {  // only one property set
    if(m_SingleProperty & EG_STYLE_PROP_META_INHERIT) return EG_STYLE_RES_INHERIT;
    *pValue = (m_SingleProperty & EG_STYLE_PROP_META_INITIAL) ? GetDefaultProperty(EG_STYLE_PROP_ID_MASK(m_SingleProperty)) : m_VP.m_SingleValue;
    return EG_STYLE_RES_FOUND;
  }
  return EG_STYLE_RES_NOT_FOUND;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGStyle::ConstantStyleInitialise(uint8_t Count, EG_StylePropValue_t *pPropertyArray) 
{                                  
  m_VP.m_pConstProperty = pPropertyArray; 
  m_HasGroup = 0xFF;                  
  m_SingleProperty = EG_STYLE_PROP_ANY;
  m_PropertyCount = Count;  
}

/////////////////////////////////////////////////////////////////////////////////////////

inline bool PropertyHasFlag(EGStyleProperty_e Property, uint8_t Flag)
{
  return LookupPropertyFlags(Property) & Flag;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGStyle::SetSize(EG_Coord_t Value)
{
  SetWidth(Value);
  SetHeight(Value);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGStyle::SetPaddingAll(EG_Coord_t Value)
{
  SetPaddingLeft(Value);
  SetPaddingRight(Value);
  SetPaddingTop(Value);
  SetPaddingBottom(Value);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGStyle::SetHorizontalPadding(EG_Coord_t Value)
{
  SetPaddingLeft(Value);
  SetPaddingRight(Value);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGStyle::SetVerticalPadding(EG_Coord_t Value)
{
  SetPaddingTop(Value);
  SetPaddingBottom(Value);
}

/////////////////////////////////////////////////////////////////////////////////////////

inline void EGStyle::SetPaddingGap(EG_Coord_t Value)
{
  SetPaddingRow(Value);
  SetPaddingColumn(Value);
}

/////////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_ASSERT_STYLE
#  define EG_ASSERT_STYLE(style_p)                                                                            \
    do {                                                                                                      \
        EG_ASSERT_MSG(style_p != NULL, "The style is NULL");                                                  \
        EG_ASSERT_MSG(style_p->sentinel == EG_STYLE_SENTINEL_VALUE, "Style is not initialized or corrupted"); \
    } while(0)
#else
#  define EG_ASSERT_STYLE(p) do{}while(0)
#endif

