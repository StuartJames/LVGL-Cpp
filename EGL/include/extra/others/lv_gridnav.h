/**
 * @file lv_templ.c
 *
 */


/*This typedef exists purely to keep -Wpedantic happy when the file is empty.*/
/*It can be removed.*/
typedef int _keep_pedantic_happy;

/**
 * @file lv_gridnav.h
 *
 */

#pragma once

#include "core/EG_Object.h"

#if EG_USE_GRIDNAV

typedef enum {
    LV_GRIDNAV_CTRL_NONE = 0x0,

    /**
     * If there is no next/previous object in a direction,
     * the focus goes to the object in the next/previous row (on left/right keys)
     * or first/last row (on up/down keys)
     */
    LV_GRIDNAV_CTRL_ROLLOVER = 0x1,

    /**
     * If an arrow is pressed and the focused object can be scrolled in that direction
     * then it will be scrolled instead of going to the next/previous object.
     * If there is no more room for scrolling the next/previous object will be focused normally */
    LV_GRIDNAV_CTRL_SCROLL_FIRST = 0x2,

} lv_gridnav_ctrl_t;

/**
 * Add grid navigation feature to an object. It expects the children to be arranged
 * into a grid-like layout. Although it's not required to have pixel perfect alignment.
 * This feature makes possible to use keys to navigate among the children and focus them.
 * The keys other than arrows and press/release related events
 * are forwarded to the focused child.
 * @param obj       pointer to an object on which navigation should be applied.
 * @param ctrl      control flags from `lv_gridnav_ctrl_t`.
 */
void lv_gridnav_add(EGObject * obj, lv_gridnav_ctrl_t ctrl);

/**
 * Remove the grid navigation support from an object
 * @param obj       pointer to an object
 */
void lv_gridnav_remove(EGObject * obj);

/**
 * Manually focus an object on gridnav container
 * @param cont      pointer to a gridnav container
 * @param to_focus  pointer to an object to focus
 * @param anim_en   EG_ANIM_ON/OFF
 */
void lv_gridnav_set_focused(EGObject * cont, EGObject * to_focus, EG_AnimateEnable_e anim_en);

#endif 