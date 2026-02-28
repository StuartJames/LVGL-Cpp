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

/////////////////////////////////////////////////////////////////////////////

inline void EGObject::RemoveAllStyles(void)
{
  RemoveStyle(NULL, (EG_StyleFlags_t)EG_PART_ANY | (EG_StyleFlags_t)EG_STATE_ANY);
}

/////////////////////////////////////////////////////////////////////////////

#if EG_USE_EXT_DATA
inline void EGObject::SetExtData(void *pExtData)
{
  m_pExtData = pExtData;
}

////////////////////////////////////////////////////////////////////////////////

inline void* EGObject::GetExtData(void)
{
  return m_pExtData;
}
#endif

////////////////////////////////////////////////////////////////////////////////

/* Scale the given number of pixels (a distance or size) relative to a 160 DPI display
 * considering the DPI of the `obj`'s display.
 * It ensures that e.g. `eg_dpx(100)` will have the same physical size regardless to the
 * DPI of the display. */

inline EG_Coord_t EGObject::DPX(EGObject *pObj, EG_Coord_t DPI)
{
  return _EG_DPX_CALC(EGDisplay::GetDPI(pObj->GetDisplay()), DPI);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

inline void EGObject::SetPaddingAll(EG_Coord_t Value, EG_StyleFlags_t SelectFlags)
{
  SetStylePadLeft(Value, SelectFlags);
  SetStylePadRight(Value, SelectFlags);
  SetStylePadTop(Value, SelectFlags);
  SetStylePadBottom(Value, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline void EGObject::SetHorizontalPadding(EG_Coord_t Value, EG_StyleFlags_t SelectFlags)
{
  SetStylePadLeft(Value, SelectFlags);
  SetStylePadRight(Value, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline void EGObject::SetVerticalPadding(EG_Coord_t Value, EG_StyleFlags_t SelectFlags)
{
  SetStylePadTop(Value, SelectFlags);
  SetStylePadBottom(Value, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline void EGObject::SetPaddingGap(EG_Coord_t Value, EG_StyleFlags_t SelectFlags)
{
  SetStylePadRow(Value, SelectFlags);
  SetStylePadColumn(Value, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline void EGObject::SetStyleSize(EG_Coord_t Value, EG_StyleFlags_t SelectFlags)
{
  SetStyleWidth(Value, SelectFlags);
  SetStyleHeight(Value, SelectFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t EGObject::GetTransformZoomSafe(const uint32_t Part)
{
  int16_t Zoom = GetStyleTransformZoom(Part);
  return Zoom != 0 ? Zoom : 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline EGState_t GetSelectorState(EG_StyleFlags_t SelectFlags)
{
	return (EGState_t)(SelectFlags & 0x00FFFF);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

inline EGPart_t GetSelectorPart(EG_StyleFlags_t SelectFlags)
{
	return (EGPart_t)(SelectFlags & 0xFF0000);
}

/////////////////////////////////////////////////////////////////////////////

inline void EGObject::Center(void)
{
  Align(EG_ALIGN_CENTER, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////

inline EG_Coord_t EGObject::GetStyleWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleMinWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_MIN_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleMaxWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_MAX_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleHeight(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_HEIGHT);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleMinHeight(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_MIN_HEIGHT);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleMaxHeight(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_MAX_HEIGHT);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleX(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_X);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleY(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_Y);
  return (EG_Coord_t)Value.Number;
}

inline EG_AlignType_e EGObject::GetStyleAlign(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ALIGN);
  return (EG_AlignType_e)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTransformWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSFORM_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTransformHeight(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSFORM_HEIGHT);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTranslateX(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSLATE_X);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTranslateY(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSLATE_Y);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTransformZoom(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSFORM_ZOOM);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTransformAngle(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSFORM_ANGLE);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTransformPivotX(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSFORM_PIVOT_X);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTransformPivotY(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSFORM_PIVOT_Y);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStylePadTop(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_PAD_TOP);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStylePadBottom(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_PAD_BOTTOM);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStylePadLeft(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_PAD_LEFT);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStylePadRight(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_PAD_RIGHT);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStylePadRow(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_PAD_ROW);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStylePadColumn(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_PAD_COLUMN);
  return (EG_Coord_t)Value.Number;
}

inline EG_Color_t EGObject::GetStyleBackColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleBackColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_BG_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleBckgroundOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Color_t EGObject::GetStyleBackGradientColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_GRAD_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleBackGradientColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_BG_GRAD_COLOR));
  return Value.Color;
}

inline EG_GradDirection_e EGObject::GetStyleBackGradientDirection(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_GRAD_DIR);
  return (EG_GradDirection_e)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleBackMainStop(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_MAIN_STOP);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleBackGradientStop(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_GRAD_STOP);
  return (EG_Coord_t)Value.Number;
}

inline const EG_GradDescriptor_t * EGObject::GetStyleBackGradient(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_GRAD);
  return (const EG_GradDescriptor_t *)Value.pPtr;
}

inline EG_DitherMode_t EGObject::GetStyleBackDitherMode(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_DITHER_MODE);
  return (EG_DitherMode_t)Value.Number;
}

inline const void * EGObject::GetStyleBackImageSource(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_IMG_SRC);
  return (const void *)Value.pPtr;
}

inline EG_OPA_t EGObject::GetStyleBackImageOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_IMG_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Color_t EGObject::GetStyleBackImageRecolor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_IMG_RECOLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleBackImageRecolorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_BG_IMG_RECOLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleBackImageRecolorOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_IMG_RECOLOR_OPA);
  return (EG_OPA_t)Value.Number;
}

inline bool EGObject::GetStyleBackImageTiled(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BG_IMG_TILED);
  return (bool)Value.Number;
}

inline EG_Color_t EGObject::GetStyleBorderColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BORDER_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleBorderColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_BORDER_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleBorderOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BORDER_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleBorderWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BORDER_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_BorderSide_t EGObject::GetStyleBorderSide(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BORDER_SIDE);
  return (EG_BorderSide_t)Value.Number;
}

inline bool EGObject::GetStyleBorderPost(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BORDER_POST);
  return (bool)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleOutlineWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_OUTLINE_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Color_t EGObject::GetStyleOutlineColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_OUTLINE_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleOutlineColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_OUTLINE_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleOutlineOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_OUTLINE_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleOutlinePadding(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_OUTLINE_PAD);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleShadowWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_SHADOW_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleShadowOffsetX(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_SHADOW_OFS_X);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleShadowOffsetY(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_SHADOW_OFS_Y);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleShadowSpread(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_SHADOW_SPREAD);
  return (EG_Coord_t)Value.Number;
}

inline EG_Color_t EGObject::GetStyleShadowColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_SHADOW_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleShadowColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_SHADOW_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleShadowOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_SHADOW_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_OPA_t EGObject::GetStyleImageOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_IMG_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Color_t EGObject::GetStyleImageRecolor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_IMG_RECOLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleImageRecolorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_IMG_RECOLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleImageRecolorOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_IMG_RECOLOR_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleLineWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LINE_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleLineDashWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LINE_DASH_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleLineDashGap(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LINE_DASH_GAP);
  return (EG_Coord_t)Value.Number;
}

inline bool EGObject::GetStyleLineRounded(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LINE_ROUNDED);
  return (bool)Value.Number;
}

inline EG_Color_t EGObject::GetStyleLineColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LINE_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleLineColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_LINE_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleLineOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LINE_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleArcWidth(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ARC_WIDTH);
  return (EG_Coord_t)Value.Number;
}

inline bool EGObject::GetStyleArcRounded(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ARC_ROUNDED);
  return (bool)Value.Number;
}

inline EG_Color_t EGObject::GetStyleArcColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ARC_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleArcColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_ARC_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleArcOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ARC_OPA);
  return (EG_OPA_t)Value.Number;
}

inline const void * EGObject::GetStyleArcImageSource(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ARC_IMG_SRC);
  return (const void *)Value.pPtr;
}

inline EG_Color_t EGObject::GetStyleTextColor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_COLOR);
  return Value.Color;
}

inline EG_Color_t EGObject::GetStyleTextColorFiltered(uint32_t Part)
{
  EG_StyleValue_t Value = ApplyColorFilter(Part, GetProperty(Part, EG_STYLE_TEXT_COLOR));
  return Value.Color;
}

inline EG_OPA_t EGObject::GetStyleTextOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_OPA);
  return (EG_OPA_t)Value.Number;
}

inline const EG_Font_t * EGObject::GetStyleTextFont(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_FONT);
  return (const EG_Font_t *)Value.pPtr;
}

inline EG_Coord_t EGObject::GetStyleTextKerning(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_LETTER_SPACE);
  return (EG_Coord_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleTextLineSpace(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_LINE_SPACE);
  return (EG_Coord_t)Value.Number;
}

inline EG_TextDecor_e EGObject::GetStyleTextDecoration(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_DECOR);
  return (EG_TextDecor_e)Value.Number;
}

inline EG_TextAlignment_t EGObject::GetStyleTextAlign(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TEXT_ALIGN);
  return (EG_TextAlignment_t)Value.Number;
}

inline EG_Coord_t EGObject::GetStyleRadius(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_RADIUS);
  return (EG_Coord_t)Value.Number;
}

inline bool EGObject::GetStyleClipCorner(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_CLIP_CORNER);
  return (bool)Value.Number;
}

inline EG_OPA_t EGObject::GetStyleOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_OPA);
  return (EG_OPA_t)Value.Number;
}

inline EG_OPA_t EGObject::GetStyleOPALayered(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_OPA_LAYERED);
  return (EG_OPA_t)Value.Number;
}

inline const EG_ColorFilterProps_t* EGObject::GetStyleColorFilterDiscriptor(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_COLOR_FILTER_DSC);
  return (const EG_ColorFilterProps_t *)Value.pPtr;
}

inline EG_OPA_t EGObject::GetStyleColorFilterOPA(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_COLOR_FILTER_OPA);
  return (EG_OPA_t)Value.Number;
}

inline const EGAnimate* EGObject::GetStyleAnimation(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ANIM);
  return (const EGAnimate*)Value.pPtr;
}

inline uint32_t EGObject::GetStyleAnimationTime(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ANIM_TIME);
  return (uint32_t)Value.Number;
}

inline uint32_t EGObject::GetStyleAnimationSpeed(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_ANIM_SPEED);
  return (uint32_t)Value.Number;
}

inline const EG_StyleTransitionDiscriptor_t* EGObject::GetStyleTransition(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_TRANSITION);
  return (const EG_StyleTransitionDiscriptor_t *)Value.pPtr;
}

inline EG_BlendMode_e EGObject::GetStyleBlendMode(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BLEND_MODE);
  return (EG_BlendMode_e)Value.Number;
}

inline uint32_t EGObject::GetStyleLayout(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_LAYOUT);
  return (uint32_t)Value.Number;
}

inline EG_BaseDirection_e EGObject::GetStyleBaseDirection(uint32_t Part)
{
  EG_StyleValue_t Value = GetProperty(Part, EG_STYLE_BASE_DIR);
  return (EG_BaseDirection_e)Value.Number;
}

