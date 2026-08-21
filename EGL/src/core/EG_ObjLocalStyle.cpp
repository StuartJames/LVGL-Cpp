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
* SJ    2025/08/18   8.4.0    Original by LVGL Kft
*
*/

#include "core/EG_Object.h"

//////////////////////////////////////////////////////////////////////////////

void EGObject::SetStyleWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleMinWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_MIN_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleMaxWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_MAX_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleHeight(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_HEIGHT, v, SelectFlags);
}

void EGObject::SetStyleMinHeight(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_MIN_HEIGHT, v, SelectFlags);
}

void EGObject::SetStyleMaxHeight(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_MAX_HEIGHT, v, SelectFlags);
}

void EGObject::SetStyleX(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_X, v, SelectFlags);
}

void EGObject::SetStyleY(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_Y, v, SelectFlags);
}

void EGObject::SetStyleAlign(EG_AlignType_e Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_ALIGN, v, SelectFlags);
}

void EGObject::SetStyleTransformWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSFORM_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleTransformHeight(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSFORM_HEIGHT, v, SelectFlags);
}

void EGObject::SetStyleTranslateX(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSLATE_X, v, SelectFlags);
}

void EGObject::SetStyleTranslateY(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSLATE_Y, v, SelectFlags);
}

void EGObject::SetStyleTransformZoom(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSFORM_ZOOM, v, SelectFlags);
}

void EGObject::SetStyleTransformAngle(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSFORM_ANGLE, v, SelectFlags);
}

void EGObject::SetStyleTransformPivotX(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSFORM_PIVOT_X, v, SelectFlags);
}

void EGObject::SetStyleTransformPivotY(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TRANSFORM_PIVOT_Y, v, SelectFlags);
}

void EGObject::SetStylePadTop(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_PAD_TOP, v, SelectFlags);
}

void EGObject::SetStylePadBottom(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_PAD_BOTTOM, v, SelectFlags);
}

void EGObject::SetStylePadLeft(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_PAD_LEFT, v, SelectFlags);
}

void EGObject::SetStylePadRight(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_PAD_RIGHT, v, SelectFlags);
}

void EGObject::SetStylePadRow(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_PAD_ROW, v, SelectFlags);
}

void EGObject::SetStylePadColumn(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_PAD_COLUMN, v, SelectFlags);
}

void EGObject::SetStyleBackColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_COLOR, v, SelectFlags);
}

void EGObject::SetStyleBackOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_OPA, v, SelectFlags);
}

void EGObject::SetStyleBackGradientColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_GRAD_COLOR, v, SelectFlags);
}

void EGObject::SetStyleBackGradientDirection(EG_GradDirection_e Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_GRAD_DIR, v, SelectFlags);
}

void EGObject::SetStyleBackMainStop(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_MAIN_STOP, v, SelectFlags);
}

void EGObject::SetStyleBackGradientStop(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_GRAD_STOP, v, SelectFlags);
}

void EGObject::SetStyleBackGradient(const EG_GradDescriptor_t * Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_GRAD, v, SelectFlags);
}

void EGObject::SetStyleBackDitherMode(EG_DitherMode_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_DITHER_MODE, v, SelectFlags);
}

void EGObject::SetStyleBackImageSource(const void * Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_IMG_SRC, v, SelectFlags);
}

void EGObject::SetStyleBackImageOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_IMG_OPA, v, SelectFlags);
}

void EGObject::SetStyleBackImageRecolor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_IMG_RECOLOR, v, SelectFlags);
}

void EGObject::SetStyleBackImageRecolorOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_IMG_RECOLOR_OPA, v, SelectFlags);
}

void EGObject::SetStyleBackImageTiled(bool Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BG_IMG_TILED, v, SelectFlags);
}

void EGObject::SetStyleBorderColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_BORDER_COLOR, v, SelectFlags);
}

void EGObject::SetStyleBorderOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BORDER_OPA, v, SelectFlags);
}

void EGObject::SetStyleBorderWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BORDER_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleBorderSide(EG_BorderSide_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BORDER_SIDE, v, SelectFlags);
}

void EGObject::SetStyleBorderPost(bool Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BORDER_POST, v, SelectFlags);
}

void EGObject::SetStyleOutlineWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_OUTLINE_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleOutlineColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_OUTLINE_COLOR, v, SelectFlags);
}

void EGObject::SetStyleOutlineOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_OUTLINE_OPA, v, SelectFlags);
}

void EGObject::SetStyleOutlinePad(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_OUTLINE_PAD, v, SelectFlags);
}

void EGObject::SetStyleShadowWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_SHADOW_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleShadowOffsetX(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_SHADOW_OFS_X, v, SelectFlags);
}

void EGObject::SetStyleShadowOffsetY(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_SHADOW_OFS_Y, v, SelectFlags);
}

void EGObject::SetStyleShadowSpread(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_SHADOW_SPREAD, v, SelectFlags);
}

void EGObject::SetStyleShadowColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_SHADOW_COLOR, v, SelectFlags);
}

void EGObject::SetStyleShadowOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_SHADOW_OPA, v, SelectFlags);
}

void EGObject::SetStyleImageOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_IMG_OPA, v, SelectFlags);
}

void EGObject::SetStyleImageRecolor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_IMG_RECOLOR, v, SelectFlags);
}

void EGObject::SetStyleImageRecolorOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_IMG_RECOLOR_OPA, v, SelectFlags);
}

void EGObject::SetStyleLineWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_LINE_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleLineDashWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_LINE_DASH_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleLineDashGap(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_LINE_DASH_GAP, v, SelectFlags);
}

void EGObject::SetStyleLineRounded(bool Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_LINE_ROUNDED, v, SelectFlags);
}

void EGObject::SetStyleLineColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_LINE_COLOR, v, SelectFlags);
}

void EGObject::SetStyleLineOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_LINE_OPA, v, SelectFlags);
}

void EGObject::SetStyleArcWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_ARC_WIDTH, v, SelectFlags);
}

void EGObject::SetStyleArcRounded(bool Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_ARC_ROUNDED, v, SelectFlags);
}

void EGObject::SetStyleArcColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_ARC_COLOR, v, SelectFlags);
}

void EGObject::SetStyleArcOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_ARC_OPA, v, SelectFlags);
}

void EGObject::SetStylePolyWidth(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_POLY_WIDTH, v, SelectFlags);
}

void EGObject::SetStylePolyColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_POLY_COLOR, v, SelectFlags);
}

void EGObject::SetStylePolyOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_POLY_OPA, v, SelectFlags);
}

void EGObject::SetStylePolyFillColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_POLY_FILL_COLOR, v, SelectFlags);
}

void EGObject::SetStylePolyFillOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_POLY_FILL_OPA, v, SelectFlags);
}

void EGObject::SetStyleArcImageSource(const void *pValue, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = pValue
  };
  SetLocalStyleProperty(EG_STYLE_ARC_IMG_SRC, v, SelectFlags);
}

void EGObject::SetStyleTextColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Color = Value
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_COLOR, v, SelectFlags);
}

void EGObject::SetStyleTextOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_OPA, v, SelectFlags);
}

void EGObject::SetStyleTextFont(const EG_Font_t *pValue, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = pValue
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_FONT, v, SelectFlags);
}

void EGObject::SetStyleTextKerning(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_LETTER_SPACE, v, SelectFlags);
}

void EGObject::SetStyleTextLineSpace(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_LINE_SPACE, v, SelectFlags);
}

void EGObject::SetStyleTextDecoration(EG_TextDecor_e Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_DECOR, v, SelectFlags);
}

void EGObject::SetStyleTextAlign(EG_TextAlignment_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_TEXT_ALIGN, v, SelectFlags);
}

void EGObject::SetStyleRadius(int32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_RADIUS, v, SelectFlags);
}

void EGObject::SetStyleClipCorner(bool Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_CLIP_CORNER, v, SelectFlags);
}

void EGObject::SetStyleOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
      .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_OPA, v, SelectFlags);
}

void EGObject::SetStyleOPALayered(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_OPA_LAYERED, v, SelectFlags);
}

void EGObject::SetStyleColorFilterDescriptor(const EG_ColorFilterProps_t *pValue, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = pValue
  };
  SetLocalStyleProperty(EG_STYLE_COLOR_FILTER_DSC, v, SelectFlags);
}

void EGObject::SetStyleColorFilterOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_COLOR_FILTER_OPA, v, SelectFlags);
}

void EGObject::SetStyleAnimate(const EGAnimate *pValue, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = pValue
  };
  SetLocalStyleProperty(EG_STYLE_ANIM, v, SelectFlags);
}

void EGObject::SetStyleAnimateTime(uint32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_ANIM_TIME, v, SelectFlags);
}

void EGObject::SetStyleAnimateSpeed(uint32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_ANIM_SPEED, v, SelectFlags);
}

void EGObject::SetStyleTransition(const EG_StyleTransitionDescriptor_t *pValue, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .pPtr = pValue
  };
  SetLocalStyleProperty(EG_STYLE_TRANSITION, v, SelectFlags);
}

void EGObject::SetStyleBlendMode(EG_BlendMode_e Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BLEND_MODE, v, SelectFlags);
}

void EGObject::SetStyleLayout(uint32_t Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_LAYOUT, v, SelectFlags);
}

void EGObject::SetStyleBaseDirection(EG_BaseDirection_e Value, EG_StyleFlags_t SelectFlags)
{
  EG_StyleValue_t v = {
    .Number = (int32_t)Value
  };
  SetLocalStyleProperty(EG_STYLE_BASE_DIR, v, SelectFlags);
}
