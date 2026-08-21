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
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"

#include <stddef.h>
#include <stdbool.h>

#include "../misc/EG_Style.h"
#include "../misc/EG_Types.h"
//#include "../misc/EG_Point.h"
#include "../misc/EG_Rect.h"
#include "../misc/EG_Color.h"
#include "../misc/EG_Assert.h"
//#include "../misc/EG_Animate.h"
#include "../hal/EG_HAL.h"
#include "../draw/EG_EventDC.h"

// Possible states of a widget. OR-ed values are possible
enum {
    EG_STATE_DEFAULT     =  0x000000,
    EG_STATE_CHECKED     =  0x000001,
    EG_STATE_FOCUSED     =  0x000002,
    EG_STATE_FOCUS_KEY   =  0x000004,
    EG_STATE_EDITED      =  0x000008,
    EG_STATE_HOVERED     =  0x000010,
    EG_STATE_PRESSED     =  0x000020,
    EG_STATE_SCROLLED    =  0x000040,
    EG_STATE_DISABLED    =  0x000080,

    EG_STATE_USER_1      =  0x001000,
    EG_STATE_USER_2      =  0x002000,
    EG_STATE_USER_3      =  0x004000,
    EG_STATE_USER_4      =  0x008000,

    EG_STATE_ANY         = 0x00FFFF,    // Special value can be used in some functions to target all states

/* The possible parts of widgets. The parts can be considered as the internal building block of the widgets.
 * E.g. slider = background + indicator + knob. Not all parts are used by every widget */
    EG_PART_MAIN         = 0x000000,   // A background like rectangle
    EG_PART_SCROLLBAR    = 0x010000,   // The scrollbar(s)
    EG_PART_INDICATOR    = 0x020000,   // Indicator, e.g. for slider, bar, switch, or the tick box of the checkbox
    EG_PART_KNOB         = 0x030000,   // Like handle to grab to adjust the value
    EG_PART_SELECTED     = 0x040000,   // Indicate the currently selected option or section
    EG_PART_ITEMS        = 0x050000,   // Used if the widget has multiple similar elements (e.g. table cells)
    EG_PART_TICKS        = 0x060000,   // Ticks on scale e.g. for a chart or meter
    EG_PART_CURSOR       = 0x070000,   // Mark a specific place e.g. for text area's cursor or on a chart

    EG_PART_CUSTOM_FIRST = 0x080000,    // Extension point for custom widgets

    EG_PART_ANY          = 0x0F0000,    // Special value can be used in some functions to target all parts
};

typedef uint16_t EGState_t;
typedef uint32_t EGPart_t;

/**
 * On/Off features controlling the object's behavior.
 * OR-ed values are possible
 */
 enum EG_ObjectFlag_e : uint32_t {
  EG_OBJ_FLAG_HIDDEN          = (1L << 0),  // Make the object hidden. (Like it wasn't there at all)
  EG_OBJ_FLAG_CLICKABLE       = (1L << 1),  // Make the object clickable by the input devices
  EG_OBJ_FLAG_CLICK_FOCUSABLE = (1L << 2),  // Add focused state to the object when clicked
  EG_OBJ_FLAG_CHECKABLE       = (1L << 3),  // Toggle checked state when the object is clicked
  EG_OBJ_FLAG_SCROLLABLE      = (1L << 4),  // Make the object scrollable
  EG_OBJ_FLAG_SCROLL_ELASTIC  = (1L << 5),  // Allow scrolling inside but with slower speed
  EG_OBJ_FLAG_SCROLL_MOMENTUM = (1L << 6),  // Make the object scroll further when "thrown"
  EG_OBJ_FLAG_SCROLL_ONE      = (1L << 7),  // Allow scrolling only one snappable children
  EG_OBJ_FLAG_SCROLL_CHAIN_HOR = (1L << 8), // Allow propagating the horizontal scroll to a parent
  EG_OBJ_FLAG_SCROLL_CHAIN_VER = (1L << 9), // Allow propagating the vertical scroll to a parent
  EG_OBJ_FLAG_SCROLL_CHAIN     = (EG_OBJ_FLAG_SCROLL_CHAIN_HOR | EG_OBJ_FLAG_SCROLL_CHAIN_VER),
  EG_OBJ_FLAG_SCROLL_ON_FOCUS = (1L << 10),  // Automatically scroll object to make it visible when focused
  EG_OBJ_FLAG_SCROLL_WITH_ARROW  = (1L << 11), // Allow scrolling the focused object with arrow keys
  EG_OBJ_FLAG_SNAPPABLE       = (1L << 12), // If scroll snap is enabled on the parent it can snap to this object
  EG_OBJ_FLAG_PRESS_LOCK      = (1L << 13), // Keep the object pressed even if the press slid from the object
  EG_OBJ_FLAG_EVENT_BUBBLE    = (1L << 14), // Propagate the events to the parent too
  EG_OBJ_FLAG_GESTURE_BUBBLE  = (1L << 15), // Propagate the gestures to the parent
  EG_OBJ_FLAG_ADV_HITTEST     = (1L << 16), // Allow performing more accurate hit (click) test. E.g. consider rounded corners.
  EG_OBJ_FLAG_IGNORE_LAYOUT   = (1L << 17), // Make the object position-able by the layouts
  EG_OBJ_FLAG_FLOATING        = (1L << 18), // Do not scroll the object when the parent scrolls and ignore layout
  EG_OBJ_FLAG_OVERFLOW_VISIBLE = (1L << 19), // Do not clip the children's content to the parent's boundary

  EG_OBJ_FLAG_LAYOUT_1        = (1L << 23), // Custom flag, free to use by layouts
  EG_OBJ_FLAG_LAYOUT_2        = (1L << 24), // Custom flag, free to use by layouts

  EG_OBJ_FLAG_WIDGET_1        = (1L << 25), // Custom flag, free to use by widget
  EG_OBJ_FLAG_WIDGET_2        = (1L << 26), // Custom flag, free to use by widget
  EG_OBJ_FLAG_USER_1          = (1L << 27), // Custom flag, free to use by user
  EG_OBJ_FLAG_USER_2          = (1L << 28), // Custom flag, free to use by user
  EG_OBJ_FLAG_USER_3          = (1L << 29), // Custom flag, free to use by user
  EG_OBJ_FLAG_USER_4          = (1L << 30), // Custom flag, free to use by user
};
typedef uint32_t EG_ObjectFlag_t;

enum EG_DrawPartType_e{
  EG_OBJ_DRAW_PART_RECTANGLE,  // The main rectangle
  EG_OBJ_DRAW_PART_BORDER_POST,// The border if style_border_post = true
  EG_OBJ_DRAW_PART_SCROLLBAR,  // The scrollbar
};

/////////////////////////////////////////////////////////////////////////////

enum EG_TreeWalkResult_e{
	EG_TREE_WALK_NEXT,
	EG_TREE_WALK_SKIP_CHILDREN,
	EG_TREE_WALK_END,
};

typedef EG_TreeWalkResult_e (*ObjTreeWalkCB_t)(EGObject *, void *);

class EGObject;
class EGGroup;
class EGDisplay;

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "EG_ObjTree.h"
#include "EG_ObjPosition.h"
#include "EG_ObjScroll.h"
#include "EG_ObjStyle.h"
#include "EG_ObjClass.h"
#include "EG_Event.h"
#include "EG_Group.h"
#include "EG_Display.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

extern const EG_ClassType_t c_ObjectClass;

///////////////////////////////////////////////////////////////////////////////////////////////////

// Special, rarely used attributes. They are allocated automatically if any elements is set.
typedef struct EGObjAttributes_t{
    EGObjAttributes_t(void) : ppChildren(nullptr), ChildCount(0), pGroup(nullptr), pEventDescriptor(nullptr),
                            pScroll(nullptr), ExtendedClickPadding(0), ExtendedDrawSize(0),
                            EventDescriptorCount(0), ScrollbarMode(EG_SCROLLBAR_MODE_OFF), LayerType(0),
                            ScrollDirection(EG_DIR_NONE), ScrollSnapX(EG_SCROLL_SNAP_NONE), ScrollSnapY(EG_SCROLL_SNAP_NONE){};

    EGObject              **ppChildren;               // Store the pointer of the children in an array.
    uint32_t                ChildCount;               // Number of children
    EGGroup                *pGroup;

    struct EG_EventDescriptor_t *pEventDescriptor;  // Dynamically allocated event callback and user data array
    EGPoint                *pScroll;                // The current X/Y scroll offset

    int32_t                 ExtendedClickPadding;   // Extra click padding in all direction
    int32_t                 ExtendedDrawSize;       // EXTend the size in every direction for drawing.

    uint8_t                 EventDescriptorCount;   // Number of event callbacks stored in `event_dsc` array
    EG_ScrollbarMode_e      ScrollbarMode : 2;      // How to display scrollbars
    uint8_t                 LayerType : 2;          // Cache the layer type here. Element of @EG_intermediate_layer_type_t
    EG_DirType_e            ScrollDirection : 4;    // The allowed scroll direction(s)
    EG_ScrollSnap_e         ScrollSnapX : 2;        // Where to align the snappable children horizontally
    EG_ScrollSnap_e         ScrollSnapY : 2;        // Where to align the snappable children vertically
} EGObjAttributes_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_ASSERT_OBJ
#  define EG_ASSERT_OBJ(pObj, pClassType)                                                               \
    do {                                                                                                \
        EG_ASSERT_MSG(pObj != nullptr, "The object is NULL");                                             \
        EG_ASSERT_MSG(pObj->HasClass(const EG_ClassType_t *pClassType) == true, "Incompatible object type.");         \
        EG_ASSERT_MSG(pObj->IsValid() == true, "The object is invalid, deleted or corrupted?"); \
    } while(0)
# else
#  define EG_ASSERT_OBJ(obj_p, obj_class) do{}while(0)
#endif

#if EG_USE_LOG && EG_LOG_TRACE_OBJ_CREATE
#  define EG_TRACE_OBJ_CREATE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#  define EG_TRACE_OBJ_CREATE(...)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

void             EG_Initialise(void);
void             EG_Deinitialise(void);
EGState_t        GetSelectorState(EG_StyleFlags_t SelectFlags);
EGPart_t         GetSelectorPart(EG_StyleFlags_t SelectFlags);

  ///////////////////////////////////////////////////////////////////////////////////////////////////

class EGObject
{
// Base Section //
public:
							            EGObject(void);
							            EGObject(EGObject *pParent, const EG_ClassType_t *pClassCnfg = &c_ObjectClass);
	virtual			            ~EGObject(void);
  virtual void            Configure(void);
  void                    AddFlag(EG_ObjectFlag_t Flag);
  void                    ClearFlag(EG_ObjectFlag_t Flag);
  bool                    HasFlagSet(EG_ObjectFlag_t Flag) const;
  bool                    HasAnyFlagSet(EG_ObjectFlag_t Flag);
  void                    AddState(EGState_t State);
  void                    ClearState(EGState_t State);
  void                    SetState(EGState_t State, bool Enable = true);
  EGState_t               GetState(void);
  bool                    HasState(EGState_t State);
  EG_StyleStateCmp_e      CompareState(EGState_t State1, EGState_t State2);
  void*                   GetGroup(void);
  void                    AllocateAttribute(void);
  bool                    HasClass(const EG_ClassType_t *pClass);
//  bool                    IsInitialised(void){ return m_Initialized; };
  bool                    IsValid(void);
  bool                    ValidChild(const EGObject *pParent, const EGObject *pFind);
  EGObject*               GetParent(void) const { return m_pParent; }; 
  void                    SetDirectParent(EGObject *pParent){ m_pParent = pParent; };
  void                    SetLocalStyleProperty(EGStyleProperty_e Property, EG_StyleValue_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetLocalStylePropertyMeta(EGStyleProperty_e Property, uint16_t meta, EG_StyleFlags_t SelectFlags);
  EG_Result_t             GetLocalStylelProperty(EGStyleProperty_e Property, EG_StyleValue_t *pValue, EG_StyleFlags_t SelectFlags) const;
  EGStyle*                GetLocalStyle(EG_StyleFlags_t SelectFlags, bool AutoAdd = true);
  bool                    RemoveStyleProperty(EGStyleProperty_e Property, EG_StyleFlags_t SelectFlags);
  EGStyle*                GetTransitionStyle(EG_StyleFlags_t SelectFlags);
  void                    RefreshStyle(EG_StyleFlags_t SelectFlags, EGStyleProperty_e Property);
  void                    RemoveStyle(EGStyle *pStyle, EG_StyleFlags_t SelectFlags);
  void                    RemoveAllStyles(void);
  void                    DrawScrollbar(EGDeviceContext *pDC);
  EG_Result_t             InitialiseScrollbarDrawDsc(EGDrawRect *pDescriptor);

  void                    Draw(EGEvent *pEvent);
  static int32_t       DPX(EGObject *pObj, int32_t DPI);
  static bool             IsKindOf(const EGObject *pObj, const EG_ClassType_t *pClassType);

  EGRect 					    	    m_Rect;
  EG_ObjectFlag_t 			    m_Flags;
  EGState_t 					    	m_State;
  EGObjAttributes_t 	     *m_pAttributes;
  uint8_t 						    	m_StyleCount;
  uint8_t 						    	m_LayoutInvalid : 1;
  uint8_t 						    	m_ReadScrollAfterLayout : 1;
  uint8_t 						    	m_ScreenLayoutInvalid : 1;
  uint8_t 						    	m_SkipTransition : 1;
  uint8_t 						    	m_HeightLayout   : 1;
  uint8_t 						    	m_WidthLayout   : 1;
  uint8_t 						    	m_IsBeingDeleted   : 1;

private:
  void                      RefreshChildrenStyle(void);
  void                      UpdateState(EGState_t NewState);
  static void               DeleteCore(EGObject *pObj);
  static EG_LayerType_e     CalculateLayerType(EGObject *pObj);

  EGObject 				         *m_pParent;

#if EG_USE_EXT_DATA
  void 								     *m_pExtData;
#endif

// Class Section //
public:
	void 				              Initialise(void);
	bool				              IsEditable(void) const;
	bool				              IsGroupDef(void) const;

  static void               Attach(EGObject *pObject, EGObject *pParent, const EG_ClassType_t *pClassCnfg);

  const EG_ClassType_t      *m_pClass;
	void 								      (*m_pEventCB)(const EG_ClassType_t *pClass, EGEvent *pEvent); // Widget type specific event function

// Style Section //
public:
  void                    AddStyle(EGStyle *pStyle, EG_StyleFlags_t SelectFlags);
  EG_TextAlignment_t      CalculateTextAlignment(EGPart_t Part, const char *pText);
  void                    SetPaddingAll(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetHorizontalPadding(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetVerticalPadding(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetPaddingGap(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleSize(int32_t Value, EG_StyleFlags_t SelectFlags);
  EG_StyleValue_t         ApplyColorFilter(uint32_t Part, EG_StyleValue_t Value);
  void                    FadeIn(uint32_t Time, uint32_t Delay);
  void                    FadeOut(uint32_t Time, uint32_t Delay);
  int32_t              GetTransformZoomSafe(const uint32_t part);
  void                    CreateTransition(EGPart_t Part, EGState_t PreviousState, EGState_t NewState, const EG_TransitionDescriptor_t *pTransitionDescriptor);
  bool                    TransitionDelete(EGPart_t Part, EGStyleProperty_e Property, Transition_t *pTransitionLimit);
  EG_StyleValue_t         GetProperty(EGPart_t Part, EGStyleProperty_e Property) const;

  static void             ReportStyleChange(EGStyle *pStyle);
  static EG_StyleResult_t GetPropertyCore(const EGObject *pObj, EGPart_t Part, EGStyleProperty_e Property, EG_StyleValue_t *pValue);
  static EG_OPA_t         GetOPARecursive(EGObject *pObj, EGPart_t Part);
  static void             EnableStyleRefresh(bool Enable);
  static void             TransitionAnimationCB(EGAnimate *pAnimation, int32_t Value);
  static void             TransitionAnimationStartCB(EGAnimate *pAnimation);
  static void             TransitionAnimationEndCB(EGAnimate *pAnimation);
  static void             FadeAnimationCB(EGAnimate *pAnimation, int32_t Value);
  static void             FadeInAnimationEnd(EGAnimate *pAnimation);

  EG_ObjStyle_t          *m_pStyles;
  static bool             g_StyleRefreshEnable;
  static EGList           m_TransitionsList;

private:
  static void             ReportStyleChangeCore(EGStyle *pStyle, EGObject *pObj);

public:
  int32_t              GetStyleWidth(uint32_t Part);
  int32_t              GetStyleMinWidth(uint32_t Part);
  int32_t              GetStyleMaxWidth(uint32_t Part);
  int32_t              GetStyleHeight(uint32_t Part);
  int32_t              GetStyleMinHeight(uint32_t Part);
  int32_t              GetStyleMaxHeight(uint32_t Part);
  int32_t              GetStyleX(uint32_t Part);
  int32_t              GetStyleY(uint32_t Part);
  EG_AlignType_e          GetStyleAlign(uint32_t Part);
  int32_t              GetStyleTransformWidth(uint32_t Part);
  int32_t              GetStyleTransformHeight(uint32_t Part);
  int32_t              GetStyleTranslateX(uint32_t Part);
  int32_t              GetStyleTranslateY(uint32_t Part);
  int32_t              GetStyleTransformZoom(uint32_t Part);
  int32_t              GetStyleTransformAngle(uint32_t Part);
  int32_t              GetStyleTransformPivotX(uint32_t Part);
  int32_t              GetStyleTransformPivotY(uint32_t Part);
  int32_t              GetStylePadTop(uint32_t Part);
  int32_t              GetStylePadBottom(uint32_t Part);
  int32_t              GetStylePadLeft(uint32_t Part);
  int32_t              GetStylePadRight(uint32_t Part);
  int32_t              GetStylePadRow(uint32_t Part);
  int32_t              GetStylePadColumn(uint32_t Part);
  EG_Color_t              GetStyleBackColor(uint32_t Part);
  EG_Color_t              GetStyleBackColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleBckgroundOPA(uint32_t Part);
  EG_Color_t              GetStyleBackGradientColor(uint32_t Part);
  EG_Color_t              GetStyleBackGradientColorFiltered(uint32_t Part);
  EG_GradDirection_e      GetStyleBackGradientDirection(uint32_t Part);
  int32_t              GetStyleBackMainStop(uint32_t Part);
  int32_t              GetStyleBackGradientStop(uint32_t Part);
  const EG_GradDescriptor_t*    GetStyleBackGradient(uint32_t Part);
  EG_DitherMode_t        GetStyleBackDitherMode(uint32_t Part);
  const void*             GetStyleBackImageSource(uint32_t Part);
  EG_OPA_t                GetStyleBackImageOPA(uint32_t Part);
  EG_Color_t              GetStyleBackImageRecolor(uint32_t Part);
  EG_Color_t              GetStyleBackImageRecolorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleBackImageRecolorOPA(uint32_t Part);
  bool                    GetStyleBackImageTiled(uint32_t Part);
  EG_Color_t              GetStyleBorderColor(uint32_t Part);
  EG_Color_t              GetStyleBorderColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleBorderOPA(uint32_t Part);
  int32_t              GetStyleBorderWidth(uint32_t Part);
  EG_BorderSide_t         GetStyleBorderSide(uint32_t Part);
  bool                    GetStyleBorderPost(uint32_t Part);
  int32_t              GetStyleOutlineWidth(uint32_t Part);
  EG_Color_t              GetStyleOutlineColor(uint32_t Part);
  EG_Color_t              GetStyleOutlineColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleOutlineOPA(uint32_t Part);
  int32_t              GetStyleOutlinePadding(uint32_t Part);
  int32_t              GetStyleShadowWidth(uint32_t Part);
  int32_t              GetStyleShadowOffsetX(uint32_t Part);
  int32_t              GetStyleShadowOffsetY(uint32_t Part);
  int32_t              GetStyleShadowSpread(uint32_t Part);
  EG_Color_t              GetStyleShadowColor(uint32_t Part);
  EG_Color_t              GetStyleShadowColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleShadowOPA(uint32_t Part);
  EG_OPA_t                GetStyleImageOPA(uint32_t Part);
  EG_Color_t              GetStyleImageRecolor(uint32_t Part);
  EG_Color_t              GetStyleImageRecolorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleImageRecolorOPA(uint32_t Part);
  int32_t              GetStyleLineWidth(uint32_t Part);
  int32_t              GetStyleLineDashWidth(uint32_t Part);
  int32_t              GetStyleLineDashGap(uint32_t Part);
  bool                    GetStyleLineRounded(uint32_t Part);
  EG_Color_t              GetStyleLineColor(uint32_t Part);
  EG_Color_t              GetStyleLineColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleLineOPA(uint32_t Part);
  int32_t              GetStyleArcWidth(uint32_t Part);
  bool                    GetStyleArcRounded(uint32_t Part);
  EG_Color_t              GetStyleArcColor(uint32_t Part);
  EG_Color_t              GetStyleArcColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleArcOPA(uint32_t Part);
  const void*             GetStyleArcImageSource(uint32_t Part);
  int32_t              GetStylePolyWidth(uint32_t Part);
  EG_Color_t              GetStylePolyColor(uint32_t Part);
  EG_Color_t              GetStylePolyColorFiltered(uint32_t Part);
  EG_OPA_t                GetStylePolyOPA(uint32_t Part);
  EG_Color_t              GetStylePolyFillColor(uint32_t Part);
  EG_Color_t              GetStylePolyFillColorFiltered(uint32_t Part);
  EG_OPA_t                GetStylePolyFillOPA(uint32_t Part);
  EG_Color_t              GetStyleTextColor(uint32_t Part);
  EG_Color_t              GetStyleTextColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleTextOPA(uint32_t Part);
  const EG_Font_t*        GetStyleTextFont(uint32_t Part);
  int32_t              GetStyleTextKerning(uint32_t Part);
  int32_t              GetStyleTextLineSpace(uint32_t Part);
  EG_TextDecor_e          GetStyleTextDecoration(uint32_t Part);
  EG_TextAlignment_t      GetStyleTextAlign(uint32_t Part);
  int32_t              GetStyleRadius(uint32_t Part);
  bool                    GetStyleClipCorner(uint32_t Part);
  EG_OPA_t                GetStyleOPA(uint32_t Part);
  EG_OPA_t                GetStyleOPALayered(uint32_t Part);
  const EG_ColorFilterProps_t*  GetStyleColorFilterDescriptor(uint32_t Part);
  EG_OPA_t                GetStyleColorFilterOPA(uint32_t Part);
  const EGAnimate*        GetStyleAnimation(uint32_t Part);
  uint32_t                GetStyleAnimationTime(uint32_t Part);
  uint32_t                GetStyleAnimationSpeed(uint32_t Part);
  const EG_StyleTransitionDescriptor_t * GetStyleTransition(uint32_t Part);
  EG_BlendMode_e          GetStyleBlendMode(uint32_t Part);
  uint32_t                GetStyleLayout(uint32_t Part);
  EG_BaseDirection_e      GetStyleBaseDirection(uint32_t Part);

  void                    SetStyleWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMinWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMaxWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleHeight(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMinHeight(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMaxHeight(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleX(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleY(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAlign(EG_AlignType_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformHeight(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTranslateX(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTranslateY(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformZoom(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformAngle(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformPivotX(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformPivotY(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadTop(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadBottom(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadLeft(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadRight(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadRow(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadColumn(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradientColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradientDirection(EG_GradDirection_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackMainStop(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradientStop(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradient(const EG_GradDescriptor_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackDitherMode(EG_DitherMode_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageSource(const void * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageRecolor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageRecolorOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageTiled(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderSide(EG_BorderSide_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderPost(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlineWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlineColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlineOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlinePad(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowOffsetX(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowOffsetY(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowSpread(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleImageOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleImageRecolor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleImageRecolorOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineDashWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineDashGap(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineRounded(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcRounded(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcImageSource(const void * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePolyWidth(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePolyColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePolyOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePolyFillColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePolyFillOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextFont(const EG_Font_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextKerning(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextLineSpace(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextDecoration(EG_TextDecor_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextAlign(EG_TextAlignment_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleRadius(int32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleClipCorner(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOPALayered(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleColorFilterDescriptor(const EG_ColorFilterProps_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleColorFilterOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAnimate(const EGAnimate *pValue, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAnimateTime(uint32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAnimateSpeed(uint32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransition(const EG_StyleTransitionDescriptor_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBlendMode(EG_BlendMode_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLayout(uint32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBaseDirection(EG_BaseDirection_e Value, EG_StyleFlags_t SelectFlags);

// Tree Section //
public:
  void                    DeleteDelayed(EGObject *obj, uint32_t delay_ms);
  void                    DeleteAsync(EGObject *obj);
  void                    SetParent(EGObject *pParent);
  void                    MoveToIndex(int32_t Index);
  EGObject*               GetScreen(void);
  EGDisplay*              GetDisplay(void);
  EGObject*               GetChild(int32_t Index);
  uint32_t                GetChildCount(void);
  uint32_t                GetIndex(void) const;

#if EG_USE_EXT_DATA
  void                    SetExtData(void *pUserData);
  void*                   GetExtData(void);
#endif

  static void             TreeWalk(EGObject *pObj, ObjTreeWalkCB_t WalkCB, void *pUserData);
  static void             EventCB(const EG_ClassType_t *pClass, EGEvent *pEvent);

  static void             Delete(EGObject *pObj);
  static void             Clean(EGObject *pObj);
  static void             Swap(EGObject *pObj1, EGObject *pObj2);
  static void             DeleteAsyncCB(void *pObj);
  static void             DeleteAnimationEndCB(EGAnimate *pAnimate);
  static EG_TreeWalkResult_e WalkCore(EGObject *pObj, ObjTreeWalkCB_t TreeWalkCB, void *pUserData);

private:
  void                    Event(EGEvent *pEvent);
 
// Position Section //
public:
  void                    SetPosition(int32_t X, int32_t Y);
  void                    SetX(int32_t X);
  void                    SetY(int32_t Y);
  bool                    RefreshSize(void);
  void                    SetSize(int32_t Width, int32_t Height);
  void                    SetWidth(int32_t Width);
  void                    SetHeight(int32_t Height);
  void                    SetContentWidth(int32_t Width);
  void                    SetContentHeight(int32_t Height);
  void                    SetLayout(uint32_t Layout);
  bool                    IsLayoutPositioned(void);
  void                    MarkLayoutDirty(void);
  void                    UpdateLayout(void);
  void                    SetAlign(EG_AlignType_e Align);
  void                    Align(EG_AlignType_e Align, int32_t OfsetX, int32_t OfsetY);
  void                    AlignTo(EGObject *pAnchor, EG_AlignType_e Align, int32_t OfsetX, int32_t OfsetY);
  void                    Center(void);
  int32_t              GetX(void);
  int32_t              GetX2(void);
  int32_t              GetY(void);
  int32_t              GetY2(void);
  int32_t              GetAlignedX(void);
  int32_t              GetAlignedY(void);
  int32_t              GetWidth(void);
  int32_t              GetHeight(void);
  int32_t              GetContentWidth();
  int32_t              GetContentHeight();
  void                    GetContentArea(EGRect *pRect);
  int32_t              GetSelfWidth(void);
  int32_t              GetSelfHeight(void);
  bool                    RefreshSelfSize(void);
  void                    RefreshPosition(void);
  void                    MoveTo(int32_t X, int32_t Y);
  void                    MoveChildrenBy(int32_t DifferenceX, int32_t DifferenceY, bool IgnoreFloating);
  void                    TransformPoint(EGPoint *pPoint, bool Recursive, bool Invert);
  void                    GetTransformedArea(EGRect *pRect, bool Recursive, bool Invert);
  void                    InvalidateArea(const EGRect *pRect);
  void                    Invalidate(void);
  bool                    AreaIsVisible(EGRect *pRect);
  bool                    IsVisible(void);
  void                    SetExtClickArea(int32_t Size);
  void                    GetClickArea(EGRect *pRect);
  bool                    HitTest(const EGPoint *pPoint);
  int32_t              ClampWidth(int32_t Width, int32_t MinWidth, int32_t MaxWidth, int32_t ReferenceWidth);
  int32_t              ClampHeight(int32_t Height, int32_t MinHeight, int32_t MaxHeight, int32_t ReferenceHeight);

  static uint32_t         LayoutRegister(EG_LayoutUpdateCB_t UpdateCB, void *pUserData);

  static EGList           m_LayoutList;

private:
  int32_t              CalcContentWidth(void);
  int32_t              CalcContentHeight(void);
  void                    LayoutUpdateCore(void);
  void                    TransformCore(EGPoint *pPoint, bool Invert);

  static uint32_t         m_LayoutCount;


// Descriptor Section //
public:
  void                    InititialseDrawRect(uint32_t Part, EGDrawRect *pDrawRect); // Initialize a rectangle draw descriptor from an object's styles in its current state
  void                    InititialseDrawLabel(uint32_t Part, EGDrawLabel *pDrawLabel); // Initialize a label draw descriptor from an object's styles in its current state
  void                    InititialseDrawImage(uint32_t Part, EGDrawImage *pDrawImage); // Initialize an image draw descriptor from an object's styles in its current state
  void                    InititialseDrawLine(uint32_t Part, EGDrawLine *pDrawLine); // Initialize a line draw descriptor from an object's styles in its current state
  void                    InititialseDrawArc(uint32_t Part, EGDrawArc *pDrawArc); // Initialize an arc draw descriptor from an object's styles in its current state
  void                    InititialseDrawPoly(uint32_t Part, EGDrawPolygon *pDrawPoly); // Initialize a polygon draw descriptor from an object's styles in its current state
  int32_t              CalculateExtDrawSize(uint32_t Part); // Get the required extra size (around the object's part) to draw shadow, outline, value etc.
  bool                    DrawPartCheckType(EGEventDC *pDescriptor, const EG_ClassType_t *pClass, uint32_t Type); // Check the type obj a part draw descriptor
  void                    RefreshExtDrawSize(void); // Send a 'EG_EVENT_REFR_EXT_DRAW_SIZE' Call the ancestor's event handler to the object to refresh the value of the extended draw size.
  int32_t              GetExtDrawSize(void) const ; // Get the extended draw area of an object.
  EG_LayerType_e          GetLayerType(void) const;
  uint32_t                GetEventCount(void);

// Scroll Section //
public:
  void                    SetScrollbarMode(EG_ScrollbarMode_e Mode); // Set how the scrollbars should behave.
  void                    SetScrollDirection(EG_DirType_e Direction); // Set the object in which directions can be scrolled
  void                    SetScrollSnapX(EG_ScrollSnap_e Align); // Set where to snap the children when scrolling ends horizontally
  void                    SetScrollSnapY(EG_ScrollSnap_e Align); // Set where to snap the children when scrolling ends vertically
  EG_ScrollbarMode_e      GetScrollbarMode(void); // Get the current scroll mode (when to hide the scrollbars)
  EG_DirType_e            GetScrollDirection(void); // Get the object in which directions can be scrolled
  EG_ScrollSnap_e         GetScrollSnapX(void); // Get where to snap the children when scrolling ends horizontally
  EG_ScrollSnap_e         GetScrollSnapY(void); // Get where to snap the children when scrolling ends vertically
  int32_t              GetScrollX(void); // Get current X scroll position.
  int32_t              GetScrollY(void); // Get current Y scroll position.
  int32_t              GetScrollTop(void); // Return the height of the area above the object.
  int32_t              GetScrollBottom(void); // Return the height of the area below the object.
  int32_t              GetScrollLeft(void); // Return the width of the area on the left the object.
  int32_t              GetScrollRight(void); // Return the width of the area on the right the object.
  void                    GetScrollEnd(EGPoint *pEnd); // Get the X and Y coordinates where the scrolling will end for this object if a scrolling animation is in progress.
  void                    ScrollBy(int32_t SizeX, int32_t SizeY, EG_AnimateEnable_e AnimateEnable); // Scroll by a given amount of pixels
  void                    ScrollByBounded(int32_t SizeX, int32_t SizeY, EG_AnimateEnable_e AnimateEnable); // Scroll by a given amount of pixels.
  void                    ScrollTo(int32_t PosX, int32_t PosY, EG_AnimateEnable_e AnimateEnable); // Scroll to a given coordinate on an object.
  void                    ScrollToX(int32_t PosX, EG_AnimateEnable_e AnimateEnable); // Scroll to a given X coordinate on an object.
  void                    ScrollToY(int32_t PosY, EG_AnimateEnable_e AnimateEnable); // Scroll to a given Y coordinate on an object
  void                    ScrollToView(EG_AnimateEnable_e AnimateEnable); // Scroll to an object until it becomes visible on its parent
  void                    ScrollToViewRecursive(EG_AnimateEnable_e AnimateEnable); // Scroll to an object until it becomes visible on its parent.
  EG_Result_t             ScrollByRaw(int32_t x, int32_t y); // Low level function to scroll by given x and y coordinates.
  bool                    IsScrolling(void); // Tell whether an object is being scrolled or not at this moment
  void                    UpdateSnap(EG_AnimateEnable_e AnimateEnable); // Check the children of `obj` and scroll `obj` to fulfill the scroll_snap settings
  void                    GetScrollbarArea(EGRect * hor, EGRect * ver); // Get the area of the scrollbars
  void                    ScrollbarInvalidate(void); // Invalidate the area of the scrollbars
  void                    ReadjustScroll(EG_AnimateEnable_e AnimateEnable); // Checks if the content is scrolled "in" and adjusts it to a normal position.

  static void             ScrollAnimatedX(EGAnimate *pAnimate, int32_t v);
  static void             ScrollAnimatedY(EGAnimate *pAnimate, int32_t v);
  static void             ScrollAnimatedEndCB(EGAnimate *pAnimate);

private:
  void                    ScrollAreaIntoView(const EGRect *pRect, EGObject *pChild, EGPoint *pScrollValue, EG_AnimateEnable_e AnimateEnable);

};

////////////////////////////////////////////////////////////////////////////////

#include "EG_ObjStyle.inl"

