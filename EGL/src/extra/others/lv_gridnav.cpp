/**
 * @file lv_gridnav.c
 *
 */

#include "extra/others/lv_gridnav.h"
#if EG_USE_GRIDNAV

#include "misc/EG_Assert.h"
#include "misc/EG_Math.h"
#include "core/EG_InputDevice.h"

typedef struct {
	lv_gridnav_ctrl_t ctrl;
	EGObject *focused_obj;
} lv_gridnav_dsc_t;

typedef enum {
	FIND_LEFT,
	FIND_RIGHT,
	FIND_TOP,
	FIND_BOTTOM,
	FIND_NEXT_ROW_FIRST_ITEM,
	FIND_PREV_ROW_LAST_ITEM,
	FIND_FIRST_ROW,
	FIND_LAST_ROW,
} find_mode_t;

static void gridnav_event_cb(EGEvent *e);
static EGObject *find_chid(EGObject *obj, EGObject *start_child, find_mode_t mode);
static EGObject *find_first_focusable(EGObject *obj);
static EGObject *find_last_focusable(EGObject *obj);
static bool obj_is_focuable(EGObject *obj);
static EG_Coord_t get_x_center(EGObject *obj);
static EG_Coord_t get_y_center(EGObject *obj);

void lv_gridnav_add(EGObject *obj, lv_gridnav_ctrl_t ctrl)
{
	lv_gridnav_remove(obj); /*Be sure to not add gridnav twice*/

	lv_gridnav_dsc_t *dsc = EG_AllocMem(sizeof(lv_gridnav_dsc_t));
	EG_ASSERT_MALLOC(dsc);
	dsc->ctrl = ctrl;
	dsc->focused_obj = NULL;
	lv_obj_add_event_cb(obj, gridnav_event_cb, EG_EVENT_ALL, dsc);

	lv_obj_clear_flag(obj, EG_OBJ_FLAG_SCROLL_WITH_ARROW);
}

void lv_gridnav_remove(EGObject *obj)
{
	lv_gridnav_dsc_t *dsc = lv_obj_get_event_user_data(obj, gridnav_event_cb);
	if(dsc == NULL) return; /* no gridnav on this object */

	EG_FreeMem(dsc);
	lv_obj_remove_event_cb(obj, gridnav_event_cb);
}

void lv_gridnav_set_focused(EGObject *cont, EGObject *to_focus, EG_AnimateEnable_e anim_en)
{
	EG_ASSERT_NULL(to_focus);
	lv_gridnav_dsc_t *dsc = lv_obj_get_event_user_data(cont, gridnav_event_cb);
	if(dsc == NULL) {
		EG_LOG_WARN("`cont` is not a gridnav container");
		return;
	}

	if(obj_is_focuable(to_focus) == false) {
		EG_LOG_WARN("The object to focus is not focusable");
		return;
	}

	lv_obj_clear_state(dsc->focused_obj, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
	lv_obj_add_state(to_focus, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
	lv_obj_scroll_to_view(to_focus, anim_en);
	dsc->focused_obj = to_focus;
}

static void gridnav_event_cb(EGEvent *e)
{
	EGObject *obj = lv_event_get_current_target(e);
	lv_gridnav_dsc_t *dsc = lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);

	if(code == EG_EVENT_KEY) {
		uint32_t child_cnt = lv_obj_get_child_cnt(obj);
		if(child_cnt == 0) return;

		if(dsc->focused_obj == NULL) dsc->focused_obj = find_first_focusable(obj);
		if(dsc->focused_obj == NULL) return;

		uint32_t key = lv_event_get_key(e);
		EGObject *guess = NULL;

		if(key == EG_KEY_RIGHT) {
			if((dsc->ctrl & LV_GRIDNAV_CTRL_SCROLL_FIRST) && lv_obj_has_flag(dsc->focused_obj, EG_OBJ_FLAG_SCROLLABLE) &&
				 lv_obj_get_scroll_right(dsc->focused_obj) > 0) {
				EG_Coord_t d = lv_obj_get_width(dsc->focused_obj) / 4;
				if(d <= 0) d = 1;
				lv_obj_scroll_by_bounded(dsc->focused_obj, -d, 0, EG_ANIM_ON);
			}
			else {
				guess = find_chid(obj, dsc->focused_obj, FIND_RIGHT);
				if(guess == NULL) {
					if(dsc->ctrl & LV_GRIDNAV_CTRL_ROLLOVER) {
						guess = find_chid(obj, dsc->focused_obj, FIND_NEXT_ROW_FIRST_ITEM);
						if(guess == NULL) guess = find_first_focusable(obj);
					}
					else {
						lv_group_focus_next(lv_obj_get_group(obj));
					}
				}
			}
		}
		else if(key == EG_KEY_LEFT) {
			if((dsc->ctrl & LV_GRIDNAV_CTRL_SCROLL_FIRST) && lv_obj_has_flag(dsc->focused_obj, EG_OBJ_FLAG_SCROLLABLE) &&
				 lv_obj_get_scroll_left(dsc->focused_obj) > 0) {
				EG_Coord_t d = lv_obj_get_width(dsc->focused_obj) / 4;
				if(d <= 0) d = 1;
				lv_obj_scroll_by_bounded(dsc->focused_obj, d, 0, EG_ANIM_ON);
			}
			else {
				guess = find_chid(obj, dsc->focused_obj, FIND_LEFT);
				if(guess == NULL) {
					if(dsc->ctrl & LV_GRIDNAV_CTRL_ROLLOVER) {
						guess = find_chid(obj, dsc->focused_obj, FIND_PREV_ROW_LAST_ITEM);
						if(guess == NULL) guess = find_last_focusable(obj);
					}
					else {
						lv_group_focus_prev(lv_obj_get_group(obj));
					}
				}
			}
		}
		else if(key == EG_KEY_DOWN) {
			if((dsc->ctrl & LV_GRIDNAV_CTRL_SCROLL_FIRST) && lv_obj_has_flag(dsc->focused_obj, EG_OBJ_FLAG_SCROLLABLE) &&
				 lv_obj_get_scroll_bottom(dsc->focused_obj) > 0) {
				EG_Coord_t d = lv_obj_get_height(dsc->focused_obj) / 4;
				if(d <= 0) d = 1;
				lv_obj_scroll_by_bounded(dsc->focused_obj, 0, -d, EG_ANIM_ON);
			}
			else {
				guess = find_chid(obj, dsc->focused_obj, FIND_BOTTOM);
				if(guess == NULL) {
					if(dsc->ctrl & LV_GRIDNAV_CTRL_ROLLOVER) {
						guess = find_chid(obj, dsc->focused_obj, FIND_FIRST_ROW);
					}
					else {
						lv_group_focus_next(lv_obj_get_group(obj));
					}
				}
			}
		}
		else if(key == EG_KEY_UP) {
			if((dsc->ctrl & LV_GRIDNAV_CTRL_SCROLL_FIRST) && lv_obj_has_flag(dsc->focused_obj, EG_OBJ_FLAG_SCROLLABLE) &&
				 lv_obj_get_scroll_top(dsc->focused_obj) > 0) {
				EG_Coord_t d = lv_obj_get_height(dsc->focused_obj) / 4;
				if(d <= 0) d = 1;
				lv_obj_scroll_by_bounded(dsc->focused_obj, 0, d, EG_ANIM_ON);
			}
			else {
				guess = find_chid(obj, dsc->focused_obj, FIND_TOP);
				if(guess == NULL) {
					if(dsc->ctrl & LV_GRIDNAV_CTRL_ROLLOVER) {
						guess = find_chid(obj, dsc->focused_obj, FIND_LAST_ROW);
					}
					else {
						lv_group_focus_prev(lv_obj_get_group(obj));
					}
				}
			}
		}
		else {
			if(lv_group_get_focused(lv_obj_get_group(obj)) == obj) {
				lv_event_send(dsc->focused_obj, EG_EVENT_KEY, &key);
			}
		}

		if(guess && guess != dsc->focused_obj) {
			lv_obj_clear_state(dsc->focused_obj, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
			lv_obj_add_state(guess, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
			lv_obj_scroll_to_view(guess, EG_ANIM_ON);
			dsc->focused_obj = guess;
		}
	}
	else if(code == EG_EVENT_FOCUSED) {
		if(dsc->focused_obj == NULL) dsc->focused_obj = find_first_focusable(obj);
		if(dsc->focused_obj) {
			lv_obj_add_state(dsc->focused_obj, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
			lv_obj_clear_state(dsc->focused_obj, EG_STATE_PRESSED); /*Be sure the focuses obj is not stuck in pressed state*/
			lv_obj_scroll_to_view(dsc->focused_obj, EG_ANIM_OFF);
		}
	}
	else if(code == EG_EVENT_DEFOCUSED) {
		if(dsc->focused_obj) {
			lv_obj_clear_state(dsc->focused_obj, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
		}
	}
	else if(code == EG_EVENT_CHILD_CREATED) {
		EGObject *child = lv_event_get_target(e);
		if(lv_obj_get_parent(child) == obj) {
			if(dsc->focused_obj == NULL) {
				dsc->focused_obj = child;
				if(lv_obj_has_state(obj, EG_STATE_FOCUSED)) {
					lv_obj_add_state(child, EG_STATE_FOCUSED | EG_STATE_FOCUS_KEY);
					lv_obj_scroll_to_view(child, EG_ANIM_OFF);
				}
			}
		}
	}
	else if(code == EG_EVENT_CHILD_DELETED) {
		/*This event bubble, so be sure this object's child was deleted.
         *As we don't know which object was deleted we can't make the next focused.
         *So make the first object focused*/
		EGObject *target = lv_event_get_target(e);
		if(target == obj) {
			dsc->focused_obj = find_first_focusable(obj);
		}
	}
	else if(code == EG_EVENT_DELETE) {
		lv_gridnav_remove(obj);
	}
	else if(code == EG_EVENT_PRESSED || code == EG_EVENT_PRESSING || code == EG_EVENT_PRESS_LOST ||
					code == EG_EVENT_LONG_PRESSED || code == EG_EVENT_LONG_PRESSED_REPEAT ||
					code == EG_EVENT_CLICKED || code == EG_EVENT_RELEASED) {
		if(lv_group_get_focused(lv_obj_get_group(obj)) == obj) {
			/*Forward press/release related event too*/
			lv_indev_type_t t = lv_indev_get_type(lv_indev_get_act());
			if(t == EG_INDEV_TYPE_ENCODER || t == EG_INDEV_TYPE_KEYPAD) {
				lv_event_send(dsc->focused_obj, code, lv_indev_get_act());
			}
		}
	}
}

static EGObject *find_chid(EGObject *obj, EGObject *start_child, find_mode_t mode)
{
	EG_Coord_t x_start = get_x_center(start_child);
	EG_Coord_t y_start = get_y_center(start_child);
	uint32_t child_cnt = lv_obj_get_child_cnt(obj);
	EGObject *guess = NULL;
	EG_Coord_t x_err_guess = EG_COORD_MAX;
	EG_Coord_t y_err_guess = EG_COORD_MAX;
	EG_Coord_t h_half = lv_obj_get_height(start_child) / 2;
	EG_Coord_t h_max = lv_obj_get_height(obj) + lv_obj_get_scroll_top(obj) + lv_obj_get_scroll_bottom(obj);
	uint32_t i;
	for(i = 0; i < child_cnt; i++) {
		EGObject *child = lv_obj_get_child(obj, i);
		if(child == start_child) continue;
		if(obj_is_focuable(child) == false) continue;

		EG_Coord_t x_err = 0;
		EG_Coord_t y_err = 0;
		switch(mode) {
			case FIND_LEFT:
				x_err = get_x_center(child) - x_start;
				y_err = get_y_center(child) - y_start;
				if(x_err >= 0) continue;             /*It's on the right*/
				if(LV_ABS(y_err) > h_half) continue; /*Too far*/
				break;
			case FIND_RIGHT:
				x_err = get_x_center(child) - x_start;
				y_err = get_y_center(child) - y_start;
				if(x_err <= 0) continue;             /*It's on the left*/
				if(LV_ABS(y_err) > h_half) continue; /*Too far*/
				break;
			case FIND_TOP:
				x_err = get_x_center(child) - x_start;
				y_err = get_y_center(child) - y_start;
				if(y_err >= 0) continue; /*It's on the bottom*/
				break;
			case FIND_BOTTOM:
				x_err = get_x_center(child) - x_start;
				y_err = get_y_center(child) - y_start;
				if(y_err <= 0) continue; /*It's on the top*/
				break;
			case FIND_NEXT_ROW_FIRST_ITEM:
				y_err = get_y_center(child) - y_start;
				if(y_err <= 0) continue; /*It's on the top*/
				x_err = lv_obj_get_x(child);
				break;
			case FIND_PREV_ROW_LAST_ITEM:
				y_err = get_y_center(child) - y_start;
				if(y_err >= 0) continue; /*It's on the bottom*/
				x_err = obj->coords.x2 - child->coords.x2;
				break;
			case FIND_FIRST_ROW:
				x_err = get_x_center(child) - x_start;
				y_err = lv_obj_get_y(child);
				break;
			case FIND_LAST_ROW:
				x_err = get_x_center(child) - x_start;
				y_err = h_max - lv_obj_get_y(child);
		}

		if(guess == NULL ||
			 (y_err * y_err + x_err * x_err < y_err_guess * y_err_guess + x_err_guess * x_err_guess)) {
			guess = child;
			x_err_guess = x_err;
			y_err_guess = y_err;
		}
	}
	return guess;
}

static EGObject *find_first_focusable(EGObject *obj)
{
	uint32_t child_cnt = lv_obj_get_child_cnt(obj);
	uint32_t i;
	for(i = 0; i < child_cnt; i++) {
		EGObject *child = lv_obj_get_child(obj, i);
		if(obj_is_focuable(child)) return child;
	}
	return NULL;
}

static EGObject *find_last_focusable(EGObject *obj)
{
	uint32_t child_cnt = lv_obj_get_child_cnt(obj);
	int32_t i;
	for(i = child_cnt - 1; i >= 0; i--) {
		EGObject *child = lv_obj_get_child(obj, i);
		if(obj_is_focuable(child)) return child;
	}
	return NULL;
}

static bool obj_is_focuable(EGObject *obj)
{
	if(lv_obj_has_flag(obj, EG_OBJ_FLAG_HIDDEN)) return false;
	if(lv_obj_has_flag(obj, EG_OBJ_FLAG_CLICKABLE | EG_OBJ_FLAG_CLICK_FOCUSABLE))
		return true;
	else
		return false;
}

static EG_Coord_t get_x_center(EGObject *obj)
{
	return obj->coords.x1 + EG_Rect_get_width(&obj->coords) / 2;
}

static EG_Coord_t get_y_center(EGObject *obj)
{
	return obj->coords.y1 + EG_Rect_get_height(&obj->coords) / 2;
}

#endif 
