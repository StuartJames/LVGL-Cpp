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
typedef enum : uint32_t {
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
} EG_ObjectFlag_e;

typedef uint32_t EG_ObjectFlag_t;

typedef enum : uint8_t {
  EG_OBJ_DRAW_PART_RECTANGLE,  // The main rectangle
  EG_OBJ_DRAW_PART_BORDER_POST,// The border if style_border_post = true
  EG_OBJ_DRAW_PART_SCROLLBAR,  // The scrollbar
} EG_DrawPartType_e;

/////////////////////////////////////////////////////////////////////////////

typedef enum : uint8_t{
	EG_TREE_WALK_NEXT,
	EG_TREE_WALK_SKIP_CHILDREN,
	EG_TREE_WALK_END,
} EG_TreeWalkResult_e;

typedef EG_TreeWalkResult_e (*ObjTreeWalkCB_t)(EGObject *, void *);

class EGObject;
class EGGroup;
class EGDisplay;

///////////////////////////////////////////////////////////////////////////////////////////////////

#include <esp_log.h>     // ******* REMOVE THIS *******

#include "EG_ObjTree.h"
#include "EG_ObjPosition.h"
#include "EG_ObjScroll.h"
#include "EG_ObjStyle.h"
#include "EG_DrawDiscriptor.h"
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

    struct EG_EventDiscriptor_t *pEventDescriptor;    // Dynamically allocated event callback and user data array
    EGPoint                *pScroll;                   // The current X/Y scroll offset

    EG_Coord_t              ExtendedClickPadding;     // Extra click padding in all direction
    EG_Coord_t              ExtendedDrawSize;         // EXTend the size in every direction for drawing.

    uint8_t                 EventDescriptorCount;     // Number of event callbacks stored in `event_dsc` array
    EG_ScrollbarMode_e     ScrollbarMode : 2;        // How to display scrollbars
    uint8_t                 LayerType : 2;            // Cache the layer type here. Element of @lv_intermediate_layer_type_t 
    EG_DirType_e            ScrollDirection : 4;      // The allowed scroll direction(s)
    EG_ScrollSnap_e        ScrollSnapX : 2;          // Where to align the snappable children horizontally
    EG_ScrollSnap_e        ScrollSnapY : 2;          // Where to align the snappable children vertically
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
  void                    DrawScrollbar(EGDrawContext *pDrawContext);
  EG_Result_t             InitialiseScrollbarDrawDsc(EGDrawRect *pDiscriptor);

  void                    Draw(EGEvent *pEvent);
  static EG_Coord_t       DPX(EGObject *pObj, EG_Coord_t DPI);
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

#if EG_USE_USER_DATA
  void 								     *m_pUserData;
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
  void                    SetPaddingAll(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetHorizontalPadding(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetVerticalPadding(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetPaddingGap(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleSize(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  EG_StyleValue_t         ApplyColorFilter(uint32_t Part, EG_StyleValue_t Value);
  void                    FadeIn(uint32_t Time, uint32_t Delay);
  void                    FadeOut(uint32_t Time, uint32_t Delay);
  EG_Coord_t              GetTransformZoomSafe(const uint32_t part);
  void                    CreateTransition(EGPart_t Part, EGState_t PreviousState, EGState_t NewState, const EG_TransitionDiscriptor_t *pTransitionDiscriptor);
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
  EG_Coord_t              GetStyleWidth(uint32_t Part);
  EG_Coord_t              GetStyleMinWidth(uint32_t Part);
  EG_Coord_t              GetStyleMaxWidth(uint32_t Part);
  EG_Coord_t              GetStyleHeight(uint32_t Part);
  EG_Coord_t              GetStyleMinHeight(uint32_t Part);
  EG_Coord_t              GetStyleMaxHeight(uint32_t Part);
  EG_Coord_t              GetStyleX(uint32_t Part);
  EG_Coord_t              GetStyleY(uint32_t Part);
  EG_AlignType_e          GetStyleAlign(uint32_t Part);
  EG_Coord_t              GetStyleTransformWidth(uint32_t Part);
  EG_Coord_t              GetStyleTransformHeight(uint32_t Part);
  EG_Coord_t              GetStyleTranslateX(uint32_t Part);
  EG_Coord_t              GetStyleTranslateY(uint32_t Part);
  EG_Coord_t              GetStyleTransformZoom(uint32_t Part);
  EG_Coord_t              GetStyleTransformAngle(uint32_t Part);
  EG_Coord_t              GetStyleTransformPivotX(uint32_t Part);
  EG_Coord_t              GetStyleTransformPivotY(uint32_t Part);
  EG_Coord_t              GetStylePadTop(uint32_t Part);
  EG_Coord_t              GetStylePadBottom(uint32_t Part);
  EG_Coord_t              GetStylePadLeft(uint32_t Part);
  EG_Coord_t              GetStylePadRight(uint32_t Part);
  EG_Coord_t              GetStylePadRow(uint32_t Part);
  EG_Coord_t              GetStylePadColumn(uint32_t Part);
  EG_Color_t              GetStyleBackColor(uint32_t Part);
  EG_Color_t              GetStyleBackColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleBckgroundOPA(uint32_t Part);
  EG_Color_t              GetStyleBackGradientColor(uint32_t Part);
  EG_Color_t              GetStyleBackGradientColorFiltered(uint32_t Part);
  EG_GradDirection_e      GetStyleBackGradientDirection(uint32_t Part);
  EG_Coord_t              GetStyleBackMainStop(uint32_t Part);
  EG_Coord_t              GetStyleBackGradientStop(uint32_t Part);
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
  EG_Coord_t              GetStyleBorderWidth(uint32_t Part);
  EG_BorderSide_t         GetStyleBorderSide(uint32_t Part);
  bool                    GetStyleBorderPost(uint32_t Part);
  EG_Coord_t              GetStyleOutlineWidth(uint32_t Part);
  EG_Color_t              GetStyleOutlineColor(uint32_t Part);
  EG_Color_t              GetStyleOutlineColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleOutlineOPA(uint32_t Part);
  EG_Coord_t              GetStyleOutlinePadding(uint32_t Part);
  EG_Coord_t              GetStyleShadowWidth(uint32_t Part);
  EG_Coord_t              GetStyleShadowOffsetX(uint32_t Part);
  EG_Coord_t              GetStyleShadowOffsetY(uint32_t Part);
  EG_Coord_t              GetStyleShadowSpread(uint32_t Part);
  EG_Color_t              GetStyleShadowColor(uint32_t Part);
  EG_Color_t              GetStyleShadowColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleShadowOPA(uint32_t Part);
  EG_OPA_t                GetStyleImageOPA(uint32_t Part);
  EG_Color_t              GetStyleImageRecolor(uint32_t Part);
  EG_Color_t              GetStyleImageRecolorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleImageRecolorOPA(uint32_t Part);
  EG_Coord_t              GetStyleLineWidth(uint32_t Part);
  EG_Coord_t              GetStyleLineDashWidth(uint32_t Part);
  EG_Coord_t              GetStyleLineDashGap(uint32_t Part);
  bool                    GetStyleLineRounded(uint32_t Part);
  EG_Color_t              GetStyleLineColor(uint32_t Part);
  EG_Color_t              GetStyleLineColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleLineOPA(uint32_t Part);
  EG_Coord_t              GetStyleArcWidth(uint32_t Part);
  bool                    GetStyleArcRounded(uint32_t Part);
  EG_Color_t              GetStyleArcColor(uint32_t Part);
  EG_Color_t              GetStyleArcColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleArcOPA(uint32_t Part);
  const void*             GetStyleArcImageSource(uint32_t Part);
  EG_Color_t              GetStyleTextColor(uint32_t Part);
  EG_Color_t              GetStyleTextColorFiltered(uint32_t Part);
  EG_OPA_t                GetStyleTextOPA(uint32_t Part);
  const EG_Font_t*        GetStyleTextFont(uint32_t Part);
  EG_Coord_t              GetStyleTextKerning(uint32_t Part);
  EG_Coord_t              GetStyleTextLineSpace(uint32_t Part);
  EG_TextDecor_e          GetStyleTextDecoration(uint32_t Part);
  EG_TextAlignment_t      GetStyleTextAlign(uint32_t Part);
  EG_Coord_t              GetStyleRadius(uint32_t Part);
  bool                    GetStyleClipCorner(uint32_t Part);
  EG_OPA_t                GetStyleOPA(uint32_t Part);
  EG_OPA_t                GetStyleOPALayered(uint32_t Part);
  const EG_ColorFilterProps_t*  GetStyleColorFilterDiscriptor(uint32_t Part);
  EG_OPA_t                GetStyleColorFilterOPA(uint32_t Part);
  const EGAnimate*        GetStyleAnimation(uint32_t Part);
  uint32_t                GetStyleAnimationTime(uint32_t Part);
  uint32_t                GetStyleAnimationSpeed(uint32_t Part);
  const EG_StyleTransitionDiscriptor_t * GetStyleTransition(uint32_t Part);
  EG_BlendMode_e          GetStyleBlendMode(uint32_t Part);
  uint32_t                GetStyleLayout(uint32_t Part);
  EG_BaseDirection_e      GetStyleBaseDirection(uint32_t Part);

  void                    SetStyleWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMinWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMaxWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleHeight(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMinHeight(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleMaxHeight(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleX(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleY(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAlign(EG_AlignType_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformHeight(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTranslateX(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTranslateY(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformZoom(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformAngle(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformPivotX(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransformPivotY(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadTop(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadBottom(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadLeft(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadRight(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadRow(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStylePadColumn(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradientColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradientDirection(EG_GradDirection_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackMainStop(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradientStop(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackGradient(const EG_GradDescriptor_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackDitherMode(EG_DitherMode_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageSource(const void * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageRecolor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageRecolorOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBackImageTiled(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderSide(EG_BorderSide_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleBorderPost(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlineWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlineColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlineOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOutlinePad(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowOffsetX(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowOffsetY(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowSpread(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleShadowOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleImageOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleImageRecolor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleImageRecolorOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineDashWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineDashGap(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineRounded(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleLineOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcWidth(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcRounded(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleArcImageSource(const void * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextColor(EG_Color_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextFont(const EG_Font_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextKerning(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextLineSpace(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextDecoration(EG_TextDecor_e Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTextAlign(EG_TextAlignment_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleRadius(EG_Coord_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleClipCorner(bool Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleOPALayered(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleColorFilterDiscriptor(const EG_ColorFilterProps_t * Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleColorFilterOPA(EG_OPA_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAnimate(const EGAnimate *pValue, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAnimateTime(uint32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleAnimateSpeed(uint32_t Value, EG_StyleFlags_t SelectFlags);
  void                    SetStyleTransition(const EG_StyleTransitionDiscriptor_t * Value, EG_StyleFlags_t SelectFlags);
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

#if EG_USE_USER_DATA
  void                    SetUserData(void *pUserData);
  void*                   GetUserData(void);
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
  void                    SetPosition(EG_Coord_t X, EG_Coord_t Y);
  void                    SetX(EG_Coord_t X);
  void                    SetY(EG_Coord_t Y);
  bool                    RefreshSize(void);
  void                    SetSize(EG_Coord_t Width, EG_Coord_t Height);
  void                    SetWidth(EG_Coord_t Width);
  void                    SetHeight(EG_Coord_t Height);
  void                    SetContentWidth(EG_Coord_t Width);
  void                    SetContentHeight(EG_Coord_t Height);
  void                    SetLayout(uint32_t Layout);
  bool                    IsLayoutPositioned(void);
  void                    MarkLayoutDirty(void);
  void                    UpdateLayout(void);
  void                    SetAlign(EG_AlignType_e Align);
  void                    Align(EG_AlignType_e Align, EG_Coord_t OfsetX, EG_Coord_t OfsetY);
  void                    AlignTo(EGObject *pBase, EG_AlignType_e Align, EG_Coord_t OfsetX, EG_Coord_t OfsetY);
  void                    Center(void);
  EG_Coord_t              GetX(void);
  EG_Coord_t              GetX2(void);
  EG_Coord_t              GetY(void);
  EG_Coord_t              GetY2(void);
  EG_Coord_t              GetAlignedX(void);
  EG_Coord_t              GetAlignedY(void);
  EG_Coord_t              GetWidth(void);
  EG_Coord_t              GetHeight(void);
  EG_Coord_t              GetContentWidth();
  EG_Coord_t              GetContentHeight();
  void                    GetContentArea(EGRect *pRect);
  EG_Coord_t              GetSelfWidth(void);
  EG_Coord_t              GetSelfHeight(void);
  bool                    RefreshSelfSize(void);
  void                    RefreshPosition(void);
  void                    MoveTo(EG_Coord_t X, EG_Coord_t Y);
  void                    MoveChildrenBy(EG_Coord_t DifferenceX, EG_Coord_t DifferenceY, bool IgnoreFloating);
  void                    TransformPoint(EGPoint *pPoint, bool Recursive, bool Invert);
  void                    GetTransformedArea(EGRect *pRect, bool Recursive, bool Invert);
  void                    InvalidateArea(const EGRect *pRect);
  void                    Invalidate(void);
  bool                    AreaIsVisible(EGRect *pRect);
  bool                    IsVisible(void);
  void                    SetExtClickArea(EG_Coord_t Size);
  void                    GetClickArea(EGRect *pRect);
  bool                    HitTest(const EGPoint *pPoint);
  EG_Coord_t              ClampWidth(EG_Coord_t Width, EG_Coord_t MinWidth, EG_Coord_t MaxWidth, EG_Coord_t ReferenceWidth);
  EG_Coord_t              ClampHeight(EG_Coord_t Height, EG_Coord_t MinHeight, EG_Coord_t MaxHeight, EG_Coord_t ReferenceHeight);

  static uint32_t         LayoutRegister(EG_LayoutUpdateCB_t UpdateCB, void *pUserData);

  static EGList           m_LayoutList;

private:
  EG_Coord_t              CalcContentWidth(void);
  EG_Coord_t              CalcContentHeight(void);
  void                    LayoutUpdateCore(void);
  void                    TransformCore(EGPoint *pPoint, bool Invert);

  static uint32_t         m_LayoutCount;


// Discriptor Section //
public:
  void                    InititialseDrawRect(uint32_t Part, EGDrawRect *pDrawRect); // Initialize a rectangle draw descriptor from an object's styles in its current state
  void                    InititialseDrawLabel(uint32_t Part, EGDrawLabel *pDrawLabel); // Initialize a label draw descriptor from an object's styles in its current state
  void                    InititialseDrawImage(uint32_t Part, EGDrawImage *pDrawImage); // Initialize an image draw descriptor from an object's styles in its current state
  void                    InititialseDrawLine(uint32_t Part, EGDrawLine *pDrawLine); // Initialize a line draw descriptor from an object's styles in its current state
  void                    InititialseDrawArc(uint32_t Part, EGDrawArc *pDrawArc); // Initialize an arc draw descriptor from an object's styles in its current state
  EG_Coord_t              CalculateExtDrawSize(uint32_t Part); // Get the required extra size (around the object's part) to draw shadow, outline, value etc.
  void                    InitDrawDescriptor(EGDrawDiscriptor *pDescriptor, EGDrawContext *pDrawContext); // Initialize a draw descriptor used in events.
  bool                    DrawPartCheckType(EGDrawDiscriptor *pDescriptor, const EG_ClassType_t *pClass, uint32_t Type); // Check the type obj a part draw descriptor
  void                    RefreshExtDrawSize(void); // Send a 'EG_EVENT_REFR_EXT_DRAW_SIZE' Call the ancestor's event handler to the object to refresh the value of the extended draw size.
  EG_Coord_t              GetExtDrawSize(void) const ; // Get the extended draw area of an object.
  EG_LayerType_e          GetLayerType(void) const;

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
  EG_Coord_t              GetScrollX(void); // Get current X scroll position.
  EG_Coord_t              GetScrollY(void); // Get current Y scroll position.
  EG_Coord_t              GetScrollTop(void); // Return the height of the area above the object.
  EG_Coord_t              GetScrollBottom(void); // Return the height of the area below the object.
  EG_Coord_t              GetScrollLeft(void); // Return the width of the area on the left the object.
  EG_Coord_t              GetScrollRight(void); // Return the width of the area on the right the object.
  void                    GetScrollEnd(EGPoint *pEnd); // Get the X and Y coordinates where the scrolling will end for this object if a scrolling animation is in progress.
  void                    ScrollBy(EG_Coord_t SizeX, EG_Coord_t SizeY, EG_AnimateEnable_e AnimateEnable); // Scroll by a given amount of pixels
  void                    ScrollByBounded(EG_Coord_t SizeX, EG_Coord_t SizeY, EG_AnimateEnable_e AnimateEnable); // Scroll by a given amount of pixels.
  void                    ScrollTo(EG_Coord_t PosX, EG_Coord_t PosY, EG_AnimateEnable_e AnimateEnable); // Scroll to a given coordinate on an object.
  void                    ScrollToX(EG_Coord_t PosX, EG_AnimateEnable_e AnimateEnable); // Scroll to a given X coordinate on an object.
  void                    ScrollToY(EG_Coord_t PosY, EG_AnimateEnable_e AnimateEnable); // Scroll to a given Y coordinate on an object
  void                    ScrollToView(EG_AnimateEnable_e AnimateEnable); // Scroll to an object until it becomes visible on its parent
  void                    ScrollToViewRecursive(EG_AnimateEnable_e AnimateEnable); // Scroll to an object until it becomes visible on its parent.
  EG_Result_t             ScrollByRaw(EG_Coord_t x, EG_Coord_t y); // Low level function to scroll by given x and y coordinates.
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

extern EGObject *g_pTabObj;
extern EGObject *g_pItemObj;

///////////////////////////////////////////////////////////////////////////////////////////////////

inline void EGObject::RemoveAllStyles(void)
{
  RemoveStyle(NULL, (EG_StyleFlags_t)EG_PART_ANY | (EG_StyleFlags_t)EG_STATE_ANY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if EG_USE_USER_DATA
inline void EGObject::SetUserData(void *pUserData)
{
  m_pUserData = pUserData;
}

////////////////////////////////////////////////////////////////////////////////

inline void* EGObject::GetUserData(void)
{
  return m_pUserData;
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

////////////////////////////////////////////////////////////////////////////////

#include "EG_ObjStyleGen.h"

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


