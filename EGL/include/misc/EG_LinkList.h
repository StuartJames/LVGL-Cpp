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
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////////////////

typedef uint8_t EG_ListNode_t; // Dummy type to make handling easier

typedef struct {
  uint32_t n_size;
  EG_ListNode_t *head;
  EG_ListNode_t *tail;
} lv_ll_t;

/////////////////////////////////////////////////////////////////////////////////////////

void      _lv_ll_init(lv_ll_t * ll_p, uint32_t node_size);
void*     _lv_ll_ins_head(lv_ll_t * ll_p);
void*     _lv_ll_ins_prev(lv_ll_t * ll_p, void * n_act);
void*     _lv_ll_ins_tail(lv_ll_t * ll_p);
void      _lv_ll_remove(lv_ll_t * ll_p, void * node_p);
void      _lv_ll_clear(lv_ll_t * ll_p);
void      _lv_ll_chg_list(lv_ll_t * ll_ori_p, lv_ll_t * ll_new_p, void * node, bool head);
void*     _lv_ll_get_head(const lv_ll_t * ll_p);
void*     _lv_ll_get_tail(const lv_ll_t * ll_p);
void*     _lv_ll_get_next(const lv_ll_t * ll_p, const void * n_act);
void*     _lv_ll_get_prev(const lv_ll_t * ll_p, const void * n_act);
uint32_t  _lv_ll_get_len(const lv_ll_t * ll_p);
void      _lv_ll_move_before(lv_ll_t * ll_p, void * n_act, void * n_after);
bool      _lv_ll_is_empty(lv_ll_t * ll_p);

/////////////////////////////////////////////////////////////////////////////////////////


#define _LV_LL_READ(list, i) for(i = _lv_ll_get_head(list); i != nullptr; i = _lv_ll_get_next(list, i))

#define _LV_LL_READ_BACK(list, i) for(i = _lv_ll_get_tail(list); i != nullptr; i = _lv_ll_get_prev(list, i))

