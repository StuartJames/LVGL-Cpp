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

#include "misc/EG_LRU.h"
#include "misc/EG_Math.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Log.h"

/////////////////////////////////////////////////////////////////////////////

static uint32_t EG_LRU_Hash(lv_lru_t *cache, const void *key, uint32_t key_length);

/** compare a key against an existing item's key */
static int EG_LRU_CompareKeys(EG_LRU_Item_t *item, const void *key, uint32_t key_length);

/** remove an item and push it to the free items queue */
static void EG_LRU_RemoveItem(lv_lru_t *cache, EG_LRU_Item_t *prev, EG_LRU_Item_t *item, uint32_t hash_index);

/** pop an existing item off the free queue, or create a new one */
static EG_LRU_Item_t *EG_LRU_PopOrCreateItem(lv_lru_t *cache);

/////////////////////////////////////////////////////////////////////////////

/* error helpers */
#define error_for(conditions, error) \
	if(conditions) {                   \
		return error;                    \
	}
#define test_for_missing_cache() error_for(!cache, EG_LRU_MISSING_CACHE)
#define test_for_missing_key() error_for(!key, EG_LRU_MISSING_KEY)
#define test_for_missing_value() error_for(!value || value_length == 0, EG_LRU_MISSING_VALUE)
#define test_for_value_too_large() error_for(value_length > cache->total_memory, EG_LRU_VALUE_TOO_LARGE)

/////////////////////////////////////////////////////////////////////////////

lv_lru_t *EG_LRUCreate(size_t cache_size, size_t average_length, lv_lru_free_t *value_free, lv_lru_free_t *key_free)
{
	lv_lru_t *cache = (lv_lru_t *)EG_AllocMem(sizeof(lv_lru_t));
	EG_ZeroMem(cache, sizeof(lv_lru_t));
	if(!cache) {
		EG_LOG_WARN("LRU Cache unable to create cache object");
		return NULL;
	}
	cache->hash_table_size = cache_size / average_length;
	cache->average_item_length = average_length;
	cache->free_memory = cache_size;
	cache->total_memory = cache_size;
	cache->seed = EG_Rand(1, UINT32_MAX);
	cache->value_free = value_free ? value_free : EG_FreeMem;
	cache->key_free = key_free ? key_free : EG_FreeMem;
	// size the hash table to a guestimate of the number of slots required (assuming a perfect hash)
	cache->items = (EG_LRU_Item_t **)EG_AllocMem(sizeof(EG_LRU_Item_t *) * cache->hash_table_size);
	EG_ZeroMem(cache->items, sizeof(EG_LRU_Item_t *) * cache->hash_table_size);
	if(!cache->items) {
		EG_LOG_WARN("LRU Cache unable to create cache hash table");
		EG_FreeMem(cache);
		return NULL;
	}
	return cache;
}

/////////////////////////////////////////////////////////////////////////////

void EG_LRUDelete(lv_lru_t *cache)
{
	EG_ASSERT_NULL(cache);
	// free each of the cached items, and the hash table
	EG_LRU_Item_t *item = NULL, *next = NULL;
	uint32_t i = 0;
	if(cache->items) {
		for(; i < cache->hash_table_size; i++) {
			item = cache->items[i];
			while(item) {
				next = (EG_LRU_Item_t *)item->next;
				cache->value_free(item->value);
				cache->key_free(item->key);
				cache->free_memory += item->value_length;
				EG_FreeMem(item);
				item = next;
			}
		}
		EG_FreeMem(cache->items);
	}
	if(cache->free_items) {
		item = cache->free_items;
		while(item) {
			next = (EG_LRU_Item_t *)item->next;
			EG_FreeMem(item);
			item = next;
		}
	}
  EG_FreeMem(cache);	// free the cache
}

/////////////////////////////////////////////////////////////////////////////

EG_LRU_Res_e EG_LRUSet(lv_lru_t *cache, const void *key, size_t key_length, void *value, size_t value_length)
{
	test_for_missing_cache();
	test_for_missing_key();
	test_for_missing_value();
	test_for_value_too_large();
	uint32_t hash_index = EG_LRU_Hash(cache, key, key_length);	// see if the key already exists
	int required = 0;
	EG_LRU_Item_t *item = NULL, *prev = NULL;
	item = cache->items[hash_index];
	while(item && EG_LRU_CompareKeys(item, key, key_length)) {
		prev = item;
		item = (EG_LRU_Item_t *)item->next;
	}
	if(item) {
		// update the value and value_lengths
		required = (int)(value_length - item->value_length);
		cache->value_free(item->value);
		item->value = value;
		item->value_length = value_length;
	}
	else {
		// insert a new item
		item = EG_LRU_PopOrCreateItem(cache);
		item->value = value;
		item->key = EG_AllocMem(key_length);
		memcpy(item->key, key, key_length);
		item->value_length = value_length;
		item->key_length = key_length;
		required = (int)value_length;
		if(prev)	prev->next = item;
		else cache->items[hash_index] = item;
	}
	item->access_count = ++cache->access_count;
	// remove as many items as necessary to free enough space
	if(required > 0 && (size_t)required > cache->free_memory) {
		while(cache->free_memory < (size_t)required)
			EG_RemoveLRUItem(cache);
	}
	cache->free_memory -= required;
	return EG_LRU_OK;
}

/////////////////////////////////////////////////////////////////////////////

EG_LRU_Res_e EG_LRUGet(lv_lru_t *cache, const void *key, size_t key_size, void **value)
{
	test_for_missing_cache();
	test_for_missing_key();
	// loop until we find the item, or hit the end of a chain
	uint32_t hash_index = EG_LRU_Hash(cache, key, key_size);
	EG_LRU_Item_t *item = cache->items[hash_index];
	while(item && EG_LRU_CompareKeys(item, key, key_size))	item = (EG_LRU_Item_t *)item->next;
	if(item) {
		*value = item->value;
		item->access_count = ++cache->access_count;
	}
	else *value = NULL;
	return EG_LRU_OK;
}

/////////////////////////////////////////////////////////////////////////////

EG_LRU_Res_e EG_LRURemove(lv_lru_t *cache, const void *key, size_t key_size)
{
	test_for_missing_cache();
	test_for_missing_key();
	// loop until we find the item, or hit the end of a chain
	EG_LRU_Item_t *item = NULL, *prev = NULL;
	uint32_t hash_index = EG_LRU_Hash(cache, key, key_size);
	item = cache->items[hash_index];
	while(item && EG_LRU_CompareKeys(item, key, key_size)) {
		prev = item;
		item = (EG_LRU_Item_t *)item->next;
	}
	if(item) {
		EG_LRU_RemoveItem(cache, prev, item, hash_index);
	}
	return EG_LRU_OK;
}

/////////////////////////////////////////////////////////////////////////////

void EG_RemoveLRUItem(lv_lru_t *cache)
{
EG_LRU_Item_t *min_item = NULL, *min_prev = NULL;
EG_LRU_Item_t *item = NULL, *prev = NULL;
uint32_t i = 0, min_index = -1;
uint64_t min_access_count = -1;

	for(; i < cache->hash_table_size; i++) {
		item = cache->items[i];
		prev = NULL;
		while(item) {
			if(item->access_count < min_access_count || (int64_t)min_access_count == -1) {
				min_access_count = item->access_count;
				min_item = item;
				min_prev = prev;
				min_index = i;
			}
			prev = item;
			item = item->next;
		}
	}
	if(min_item) {
		EG_LRU_RemoveItem(cache, min_prev, min_item, min_index);
	}
}

/////////////////////////////////////////////////////////////////////////////

static uint32_t EG_LRU_Hash(lv_lru_t *cache, const void *key, uint32_t key_length)
{
uint32_t m = 0x5bd1e995;
uint32_t r = 24;
uint32_t h = cache->seed ^ key_length;
char *data = (char *)key;

	while(key_length >= 4) {
		uint32_t k = *(uint32_t *)data;
		k *= m;
		k ^= k >> r;
		k *= m;
		h *= m;
		h ^= k;
		data += 4;
		key_length -= 4;
	}
	if(key_length >= 3) {
		h ^= data[2] << 16;
	}
	if(key_length >= 2) {
		h ^= data[1] << 8;
	}
	if(key_length >= 1) {
		h ^= data[0];
		h *= m;
	}
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h % cache->hash_table_size;
}

/////////////////////////////////////////////////////////////////////////////

static int EG_LRU_CompareKeys(EG_LRU_Item_t *item, const void *key, uint32_t key_length)
{
	if(key_length != item->key_length) return 1;
  return memcmp(key, item->key, key_length);
}

/////////////////////////////////////////////////////////////////////////////

static void EG_LRU_RemoveItem(lv_lru_t *cache, EG_LRU_Item_t *prev, EG_LRU_Item_t *item, uint32_t hash_index)
{
	if(prev) prev->next = item->next;
	else cache->items[hash_index] = (EG_LRU_Item_t *)item->next;
	// free memory and update the free memory counter
	cache->free_memory += item->value_length;
	cache->value_free(item->value);
	cache->key_free(item->key);
	// push the item to the free items queue
	EG_ZeroMem(item, sizeof(EG_LRU_Item_t));
	item->next = cache->free_items;
	cache->free_items = item;
}

/////////////////////////////////////////////////////////////////////////////

static EG_LRU_Item_t* EG_LRU_PopOrCreateItem(lv_lru_t *cache)
{
	EG_LRU_Item_t *item = NULL;
	if(cache->free_items) {
		item = cache->free_items;
		cache->free_items = item->next;
		EG_ZeroMem(item, sizeof(EG_LRU_Item_t));
	}
	else {
		item = (EG_LRU_Item_t *)EG_AllocMem(sizeof(EG_LRU_Item_t));
		EG_ZeroMem(item, sizeof(EG_LRU_Item_t));
	}
	return item;
}
