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
 * @see https://github.com/willcannings/C-LRU-Cache
 * 
 * Author: Austin Appleby
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= =====================================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#pragma once

#include "../EG_IntrnlConfig.h"
#include "EG_Types.h"
#include <stdint.h>
#include <stddef.h>

/////////////////////////////////////////////////////////////////////////////

typedef enum {
    EG_LRU_OK = 0,
    EG_LRU_MISSING_CACHE,
    EG_LRU_MISSING_KEY,
    EG_LRU_MISSING_VALUE,
    EG_LRU_LOCK_ERROR,
    EG_LRU_VALUE_TOO_LARGE
} EG_LRU_Res_e;

typedef void (lv_lru_free_t)(void * v);

typedef struct EG_LRU_Item_t {
	void *value;
	void *key;
	size_t value_length;
	size_t key_length;
	uint64_t access_count;
	EG_LRU_Item_t *next;
} EG_LRU_Item_t;


typedef struct lv_lru_t {
    EG_LRU_Item_t ** items;
    uint64_t access_count;
    size_t free_memory;
    size_t total_memory;
    size_t average_item_length;
    size_t hash_table_size;
    uint32_t seed;
    lv_lru_free_t * value_free;
    lv_lru_free_t * key_free;
    EG_LRU_Item_t * free_items;
} lv_lru_t;

/////////////////////////////////////////////////////////////////////////////

lv_lru_t*       EG_LRUCreate(size_t cache_size, size_t average_length, lv_lru_free_t * value_free, lv_lru_free_t * key_free);
void            EG_LRUDelete(lv_lru_t * cache);
EG_LRU_Res_e    EG_LRUSet(lv_lru_t * cache, const void * key, size_t key_length, void * value, size_t value_length);
EG_LRU_Res_e    EG_LRUGet(lv_lru_t * cache, const void * key, size_t key_size, void ** value);
EG_LRU_Res_e    EG_LRURemove(lv_lru_t * cache, const void * key, size_t key_size);
void            EG_RemoveLRUItem(lv_lru_t * cache);
