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

#include "misc/EG_Style.h"

void EGStyle::SetWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value };

  SetProperty(EG_STYLE_WIDTH, StyleValue);
}

void EGStyle::SetMinWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = { .Number = (int32_t)Value };
  SetProperty(EG_STYLE_MIN_WIDTH, StyleValue);
}

void EGStyle::SetMaxWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_MAX_WIDTH, StyleValue);
}

void EGStyle::SetHeight(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_HEIGHT, StyleValue);
}

void EGStyle::SetMinHeight(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_MIN_HEIGHT, StyleValue);
}

void EGStyle::SetMaxHeight(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_MAX_HEIGHT, StyleValue);
}

void EGStyle::SetX(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_X, StyleValue);
}

void EGStyle::SetY(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_Y, StyleValue);
}

void EGStyle::SetAlign(EG_AlignType_e Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_ALIGN, StyleValue);
}

void EGStyle::SetTransformWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSFORM_WIDTH, StyleValue);
}

void EGStyle::SetTransformHeight(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSFORM_HEIGHT, StyleValue);
}

void EGStyle::SetTranslateX(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSLATE_X, StyleValue);
}

void EGStyle::SetTranslateY(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSLATE_Y, StyleValue);
}

void EGStyle::SetTransformZoom(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSFORM_ZOOM, StyleValue);
}

void EGStyle::SetTransformAngle(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSFORM_ANGLE, StyleValue);
}

void EGStyle::SetTransformPivotX(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSFORM_PIVOT_X, StyleValue);
}

void EGStyle::SetTransformPivotY(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TRANSFORM_PIVOT_Y, StyleValue);
}

void EGStyle::SetPaddingTop(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_PAD_TOP, StyleValue);
}

void EGStyle::SetPaddingBottom(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_PAD_BOTTOM, StyleValue);
}

void EGStyle::SetPaddingLeft(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_PAD_LEFT, StyleValue);
}

void EGStyle::SetPaddingRight(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_PAD_RIGHT, StyleValue);
}

void EGStyle::SetPaddingRow(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_PAD_ROW, StyleValue);
}

void EGStyle::SetPaddingColumn(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_PAD_COLUMN, StyleValue);
}

void EGStyle::SetBackColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_BG_COLOR, StyleValue);
}

void EGStyle::SetBackOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_OPA, StyleValue);
}

void EGStyle::SetBackGradientColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_BG_GRAD_COLOR, StyleValue);
}

void EGStyle::SetBackGradientDirection(EG_GradDirection_e Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_GRAD_DIR, StyleValue);
}

void EGStyle::SetBackMainStop(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_MAIN_STOP, StyleValue);
}

void EGStyle::SetBackGradientStop(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_GRAD_STOP, StyleValue);
}

void EGStyle::SetBackGradient(const EG_GradDescriptor_t * Value)
{
EG_StyleValue_t StyleValue = {.pPtr = Value};

  SetProperty(EG_STYLE_BG_GRAD, StyleValue);
}

void EGStyle::SetBackDitherMode(EG_DitherMode_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_DITHER_MODE, StyleValue);
}

void EGStyle::SetBackImageSource(const void * Value)
{
EG_StyleValue_t StyleValue = {.pPtr = Value};

  SetProperty(EG_STYLE_BG_IMG_SRC, StyleValue);
}

void EGStyle::SetBackImageOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_IMG_OPA, StyleValue);
}

void EGStyle::SetBackImageRecolor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_BG_IMG_RECOLOR, StyleValue);
}

void EGStyle::SetBackImageRecolorOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_IMG_RECOLOR_OPA, StyleValue);
}

void EGStyle::SetBackImageTiled(bool Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BG_IMG_TILED, StyleValue);
}

void EGStyle::SetBorderColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_BORDER_COLOR, StyleValue);
}

void EGStyle::SetBorderOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BORDER_OPA, StyleValue);
}

void EGStyle::SetBorderWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BORDER_WIDTH, StyleValue);
}

void EGStyle::SetBorderSide(EG_BorderSide_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BORDER_SIDE, StyleValue);
}

void EGStyle::SetBorderPost(bool Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BORDER_POST, StyleValue);
}

void EGStyle::SetOutlineWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_OUTLINE_WIDTH, StyleValue);
}

void EGStyle::SetOutlineColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_OUTLINE_COLOR, StyleValue);
}

void EGStyle::SetOutlineOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_OUTLINE_OPA, StyleValue);
}

void EGStyle::SetOutlinePad(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_OUTLINE_PAD, StyleValue);
}

void EGStyle::SetShadowWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_SHADOW_WIDTH, StyleValue);
}

void EGStyle::SetShadowOffsetX(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_SHADOW_OFS_X, StyleValue);
}

void EGStyle::SetShadowOffsetY(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_SHADOW_OFS_Y, StyleValue);
}

void EGStyle::SetShadowSpread(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_SHADOW_SPREAD, StyleValue);
}

void EGStyle::SetShadowColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_SHADOW_COLOR, StyleValue);
}

void EGStyle::SetShadowOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_SHADOW_OPA, StyleValue);
}

void EGStyle::SetImageOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_IMG_OPA, StyleValue);
}

void EGStyle::SetImageRecolor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_IMG_RECOLOR, StyleValue);
}

void EGStyle::SetimageRecolorOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_IMG_RECOLOR_OPA, StyleValue);
}

void EGStyle::SetLineWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_LINE_WIDTH, StyleValue);
}

void EGStyle::SetLineDashWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_LINE_DASH_WIDTH, StyleValue);
}

void EGStyle::SetLineDashGap(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_LINE_DASH_GAP, StyleValue);
}

void EGStyle::SetLineRounded(bool Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_LINE_ROUNDED, StyleValue);
}

void EGStyle::SetLineColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_LINE_COLOR, StyleValue);
}

void EGStyle::SetLineOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_LINE_OPA, StyleValue);
}

void EGStyle::SetArcWidth(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_ARC_WIDTH, StyleValue);
}

void EGStyle::SetArcRounded(bool Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_ARC_ROUNDED, StyleValue);
}

void EGStyle::SetArcColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_ARC_COLOR, StyleValue);
}

void EGStyle::SetArcOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_ARC_OPA, StyleValue);
}

void EGStyle::SetArcImageSource(const void * Value)
{
EG_StyleValue_t StyleValue = {.pPtr = Value};

  SetProperty(EG_STYLE_ARC_IMG_SRC, StyleValue);
}

void EGStyle::SetTextColor(EG_Color_t Value)
{
EG_StyleValue_t StyleValue = {.Color = Value};

  SetProperty(EG_STYLE_TEXT_COLOR, StyleValue);
}

void EGStyle::SetTextOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TEXT_OPA, StyleValue);
}

void EGStyle::SetTextFont(const EG_Font_t * Value)
{
EG_StyleValue_t StyleValue = {.pPtr = Value};

  SetProperty(EG_STYLE_TEXT_FONT, StyleValue);
}

void EGStyle::SetTextKerning(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TEXT_LETTER_SPACE, StyleValue);
}

void EGStyle::SetTextLineSpace(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TEXT_LINE_SPACE, StyleValue);
}

void EGStyle::SetTextDecoration(EG_TextDecor_e Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TEXT_DECOR, StyleValue);
}

void EGStyle::SetTextAlign(EG_TextAlignment_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_TEXT_ALIGN, StyleValue);
}

void EGStyle::SetRadius(EG_Coord_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_RADIUS, StyleValue);
}

void EGStyle::SetClipCorner(bool Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_CLIP_CORNER, StyleValue);
}

void EGStyle::SetOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_OPA, StyleValue);
}

void EGStyle::SetOPALayered(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_OPA_LAYERED, StyleValue);
}

void EGStyle::SetColorFilterDiscriptor(const EG_ColorFilterProps_t * Value)
{
EG_StyleValue_t StyleValue = {.pPtr = Value};

  SetProperty(EG_STYLE_COLOR_FILTER_DSC, StyleValue);
}

void EGStyle::SetColorFilterOPA(EG_OPA_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_COLOR_FILTER_OPA, StyleValue);
}

void EGStyle::SetAnimate(const EGAnimate *pValue)
{
EG_StyleValue_t StyleValue = {.pPtr = pValue};

  SetProperty(EG_STYLE_ANIM, StyleValue);
}

void EGStyle::SetAnimateTime(uint32_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_ANIM_TIME, StyleValue);
}

void EGStyle::SetAnimateSpeed(uint32_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_ANIM_SPEED, StyleValue);
}

void EGStyle::SetTransition(const EG_StyleTransitionDiscriptor_t * Value)
{
EG_StyleValue_t StyleValue = {.pPtr = Value};

  SetProperty(EG_STYLE_TRANSITION, StyleValue);
}

void EGStyle::SetBlendMode(EG_BlendMode_e Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BLEND_MODE, StyleValue);
}

void EGStyle::SetLayout(uint16_t Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_LAYOUT, StyleValue);
}

void EGStyle::SetBaseDirection(EG_BaseDirection_e Value)
{
EG_StyleValue_t StyleValue = {.Number = (int32_t)Value};

  SetProperty(EG_STYLE_BASE_DIR, StyleValue);
}
