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
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   1.a.1    Original by LVGL Kft
 *
 */

#include "misc/EG_Memory.h"
#include "misc/lv_tlsf.h"
#include "misc/EG_Misc.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Log.h"

//////////////////////////////////////////////////////////////////////////////////////

#if EG_MEM_CUSTOM != 0
#include EG_MEM_CUSTOM_INCLUDE
#endif

#ifdef EG_MEM_POOL_INCLUDE
#include EG_MEM_POOL_INCLUDE
#endif

// memset the allocated memories to 0xaa and freed memories to 0xbb (just for testing purposes)*/
#ifndef EG_MEM_ADD_JUNK
#define EG_MEM_ADD_JUNK 0
#endif

#ifdef EG_ARCH_64
#define MEM_UNIT uint64_t
#define ALIGN_MASK 0x7
#else
#define MEM_UNIT uint32_t
#define ALIGN_MASK 0x3
#endif

#define ZERO_MEM_SENTINEL 0xa1b2c3d4

#if EG_MEM_CUSTOM == 0
static void lv_mem_walker(void *ptr, size_t size, int used, void *user);
#endif


#if EG_MEM_CUSTOM == 0
static lv_tlsf_t tlsf;
static uint32_t cur_used;
static uint32_t max_used;
#endif

static uint32_t zero_mem = ZERO_MEM_SENTINEL;  // Give the address of this variable if 0 byte should be allocated*/

#if EG_LOG_TRACE_MEM
#define MEM_TRACE(...) EG_LOG_TRACE(__VA_ARGS__)
#else
#define MEM_TRACE(...)
#endif

#define COPY32 \
	*d32 = *s32; \
	d32++;       \
	s32++;
#define COPY8 \
	*d8 = *s8;  \
	d8++;       \
	s8++;
#define SET32(x) \
	*d32 = x;      \
	d32++;
#define SET8(x) \
	*d8 = x;      \
	d8++;
#define REPEAT8(expr) expr expr expr expr expr expr expr expr

//////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize the dyn_mem module (work memory and other variables)
 */
void EG_InitMem(void)
{
#if EG_MEM_CUSTOM == 0

#if EG_MEM_ADR == 0
#ifdef EG_MEM_POOL_ALLOC
	tlsf = lv_tlsf_create_with_pool((void *)EG_MEM_POOL_ALLOC(EG_MEM_SIZE), EG_MEM_SIZE);
#else
	// Allocate a large array to store the dynamically allocated data*/
	static EG_ATTRIBUTE_LARGE_RAM_ARRAY MEM_UNIT work_mem_int[EG_MEM_SIZE / sizeof(MEM_UNIT)];
	tlsf = lv_tlsf_create_with_pool((void *)work_mem_int, EG_MEM_SIZE);
#endif
#else
	tlsf = lv_tlsf_create_with_pool((void *)EG_MEM_ADR, EG_MEM_SIZE);
#endif
#endif

#if EG_MEM_ADD_JUNK
	EG_LOG_WARN("EG_MEM_ADD_JUNK is enabled which makes LVGL much slower");
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// Clean up the memory buffer which frees all the allocated memories.
void EG_DeinitMem(void)
{
#if EG_MEM_CUSTOM == 0
	lv_tlsf_destroy(tlsf);
	EG_InitMem();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

void *EG_AllocMem(size_t size)
{
	MEM_TRACE("allocating %lu bytes", (unsigned long)size);
	if(size == 0) {
		MEM_TRACE("using zero_mem");
		return &zero_mem;
	}

#if EG_MEM_CUSTOM == 0
	void *alloc = lv_tlsf_malloc(tlsf, size);
#else
	void *alloc = EG_MEM_CUSTOM_ALLOC(size);
#endif

	if(alloc == NULL) {
		EG_LOG_INFO("couldn't allocate memory (%lu bytes)", (unsigned long)size);
#if EG_LOG_LEVEL <= EG_LOG_LEVEL_INFO
		EG_MonitorMem_t mon;
		EG_MonitorMem(&mon);
		EG_LOG_INFO("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d",
								(int)(mon.total_size - mon.free_size), mon.used_pct, mon.frag_pct,
								(int)mon.free_biggest_size);
#endif
	}
#if EG_MEM_ADD_JUNK
	else {
		EG_SetMem(alloc, 0xaa, size);
	}
#endif

	if(alloc) {
#if EG_MEM_CUSTOM == 0
		cur_used += size;
		max_used = EG_MAX(cur_used, max_used);
#endif
		MEM_TRACE("allocated at %p", alloc);
	}
	return alloc;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EG_FreeMem(void *data)
{
	MEM_TRACE("freeing %p", data);
	if(data == &zero_mem) return;
	if(data == NULL) return;

#if EG_MEM_CUSTOM == 0
#if EG_MEM_ADD_JUNK
	EG_SetMem(data, 0xbb, lv_tlsf_block_size(data));
#endif
	size_t size = lv_tlsf_free(tlsf, data);
	if(cur_used > size)
		cur_used -= size;
	else
		cur_used = 0;
#else
	EG_MEM_CUSTOM_FREE(data);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// Reallocate a memory with a new size. The old content will be kept.
void *EG_ReallocMem(void *data_p, size_t new_size)
{
	MEM_TRACE("reallocating %p with %lu size", data_p, (unsigned long)new_size);
	if(new_size == 0) {
		MEM_TRACE("using zero_mem");
		EG_FreeMem(data_p);
		return &zero_mem;
	}

	if(data_p == &zero_mem) return EG_AllocMem(new_size);

#if EG_MEM_CUSTOM == 0
	void *new_p = lv_tlsf_realloc(tlsf, data_p, new_size);
#else
	void *new_p = EG_MEM_CUSTOM_REALLOC(data_p, new_size);
#endif
	if(new_p == NULL) {
		EG_LOG_ERROR("couldn't allocate memory");
		return NULL;
	}

	MEM_TRACE("allocated at %p", new_p);
	return new_p;
}

/////////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EG_TestMem(void)
{
	if(zero_mem != ZERO_MEM_SENTINEL) {
		EG_LOG_WARN("zero_mem is written");
		return EG_RES_INVALID;
	}

#if EG_MEM_CUSTOM == 0
	if(lv_tlsf_check(tlsf)) {
		EG_LOG_WARN("failed");
		return EG_RES_INVALID;
	}

	if(lv_tlsf_check_pool(lv_tlsf_get_pool(tlsf))) {
		EG_LOG_WARN("pool failed");
		return EG_RES_INVALID;
	}
#endif
	MEM_TRACE("passed");
	return EG_RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Give information about the work memory of dynamic allocation
void EG_MonitorMem(EG_MonitorMem_t *mon_p)
{
	/*Init the data*/
	EG_SetMem(mon_p, 0, sizeof(EG_MonitorMem_t));
#if EG_MEM_CUSTOM == 0
	MEM_TRACE("begin");

	lv_tlsf_walk_pool(lv_tlsf_get_pool(tlsf), lv_mem_walker, mon_p);

	mon_p->total_size = EG_MEM_SIZE;
	mon_p->used_pct = 100 - (100U * mon_p->free_size) / mon_p->total_size;
	if(mon_p->free_size > 0) {
		mon_p->frag_pct = mon_p->free_biggest_size * 100U / mon_p->free_size;
		mon_p->frag_pct = 100 - mon_p->frag_pct;
	}
	else {
		mon_p->frag_pct = 0; /*no fragmentation if all the RAM is used*/
	}

	mon_p->max_used = max_used;

	MEM_TRACE("finished");
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// Get a tempory buffer with the given size.
void *EG_GetBufferMem(uint32_t size)
{
	if(size == 0) return NULL;

	MEM_TRACE("begin, getting %d bytes", size);

	// Try to find a free buffer with suitable size*/
	int8_t i_guess = -1;
	for(uint8_t i = 0; i < EG_MEM_BUF_MAX_NUM; i++) {
		if(EG_GC_ROOT(EG_MemBuffer[i]).used == 0 && EG_GC_ROOT(EG_MemBuffer[i]).size >= size) {
			if(EG_GC_ROOT(EG_MemBuffer[i]).size == size) {
				EG_GC_ROOT(EG_MemBuffer[i]).used = 1;
				return EG_GC_ROOT(EG_MemBuffer[i]).pBuffer;
			}
			else if(i_guess < 0) {
				i_guess = i;
			}
			// If size of `i` is closer to `size` prefer it*/
			else if(EG_GC_ROOT(EG_MemBuffer[i]).size < EG_GC_ROOT(EG_MemBuffer[i_guess]).size) {
				i_guess = i;
			}
		}
	}

	if(i_guess >= 0) {
		EG_GC_ROOT(EG_MemBuffer[i_guess]).used = 1;
		MEM_TRACE("returning already allocated buffer (buffer id: %d, address: %p)", i_guess,
							EG_GC_ROOT(EG_MemBuffer[i_guess]).pBuffer);
		return EG_GC_ROOT(EG_MemBuffer[i_guess]).pBuffer;
	}

	// Reallocate a free buffer*/
	for(uint8_t i = 0; i < EG_MEM_BUF_MAX_NUM; i++) {
		if(EG_GC_ROOT(EG_MemBuffer[i]).used == 0) {
			// if this fails you probably need to increase your EG_MEM_SIZE/heap size*/
			void *buf = EG_ReallocMem(EG_GC_ROOT(EG_MemBuffer[i]).pBuffer, size);
			EG_ASSERT_MSG(buf != NULL, "Out of memory, can't allocate a new buffer (increase your EG_MEM_SIZE/heap size)");
			if(buf == NULL) return NULL;

			EG_GC_ROOT(EG_MemBuffer[i]).used = 1;
			EG_GC_ROOT(EG_MemBuffer[i]).size = size;
			EG_GC_ROOT(EG_MemBuffer[i]).pBuffer = buf;
			MEM_TRACE("allocated (buffer id: %d, address: %p)", i, EG_GC_ROOT(EG_MemBuffer[i]).pBuffer);
			return EG_GC_ROOT(EG_MemBuffer[i]).pBuffer;
		}
	}

	EG_LOG_ERROR("no more buffers. (increase EG_MEM_BUF_MAX_NUM)");
	EG_ASSERT_MSG(false, "No more buffers. Increase EG_MEM_BUF_MAX_NUM.");
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void EG_ReleaseBufferMem(void *p)
{
	MEM_TRACE("begin (address: %p)", p);
	for(uint8_t i = 0; i < EG_MEM_BUF_MAX_NUM; i++) {
		if(EG_GC_ROOT(EG_MemBuffer[i]).pBuffer == p) {
			EG_GC_ROOT(EG_MemBuffer[i]).used = 0;
			return;
		}
	}
	EG_LOG_ERROR("p is not a known buffer");
}

/////////////////////////////////////////////////////////////////////////////////////////

void EG_FreeAllBuffers(void)
{
	for(uint8_t i = 0; i < EG_MEM_BUF_MAX_NUM; i++) {
		if(EG_GC_ROOT(EG_MemBuffer[i]).pBuffer) {
			EG_FreeMem(EG_GC_ROOT(EG_MemBuffer[i]).pBuffer);
			EG_GC_ROOT(EG_MemBuffer[i]).pBuffer = NULL;
			EG_GC_ROOT(EG_MemBuffer[i]).used = 0;
			EG_GC_ROOT(EG_MemBuffer[i]).size = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

#if EG_MEMCPY_MEMSET_STD == 0

void *EG_ATTRIBUTE_FAST_MEM EG_CopyMem(void *dst, const void *src, size_t len)
{
	uint8_t *d8 = (uint8_t *)dst;
	const uint8_t *s8 = (uint8_t *)src;

	eg_uintptr_t d_align = (eg_uintptr_t)d8 & ALIGN_MASK;
	eg_uintptr_t s_align = (eg_uintptr_t)s8 & ALIGN_MASK;

	// Byte copy for unaligned memories*/
	if(s_align != d_align) {
		while(len > 32) {
			REPEAT8(COPY8);
			REPEAT8(COPY8);
			REPEAT8(COPY8);
			REPEAT8(COPY8);
			len -= 32;
		}
		while(len) {
			COPY8
			len--;
		}
		return dst;
	}

	// Make the memories aligned*/
	if(d_align) {
		d_align = ALIGN_MASK + 1 - d_align;
		while(d_align && len) {
			COPY8;
			d_align--;
			len--;
		}
	}

	uint32_t *d32 = (uint32_t *)d8;
	const uint32_t *s32 = (uint32_t *)s8;
	while(len > 32) {
		REPEAT8(COPY32)
		len -= 32;
	}

	while(len > 4) {
		COPY32;
		len -= 4;
	}

	d8 = (uint8_t *)d32;
	s8 = (const uint8_t *)s32;
	while(len) {
		COPY8
		len--;
	}

	return dst;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Same as `memset` but optimized for 4 byte operation.
void EG_ATTRIBUTE_FAST_MEM EG_SetMem(void *dst, uint8_t v, size_t len)
{
	uint8_t *d8 = (uint8_t *)dst;

	uintptr_t d_align = (eg_uintptr_t)d8 & ALIGN_MASK;

	// Make the address aligned*/
	if(d_align) {
		d_align = ALIGN_MASK + 1 - d_align;
		while(d_align && len) {
			SET8(v);
			len--;
			d_align--;
		}
	}

	uint32_t v32 = (uint32_t)v + ((uint32_t)v << 8) + ((uint32_t)v << 16) + ((uint32_t)v << 24);

	uint32_t *d32 = (uint32_t *)d8;

	while(len > 32) {
		REPEAT8(SET32(v32));
		len -= 32;
	}

	while(len > 4) {
		SET32(v32);
		len -= 4;
	}

	d8 = (uint8_t *)d32;
	while(len) {
		SET8(v);
		len--;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

// Same as `memset(dst, 0x00, len)` but optimized for 4 byte operation.
void EG_ATTRIBUTE_FAST_MEM EG_ZeroMem(void *dst, size_t len)
{
	uint8_t *d8 = (uint8_t *)dst;
	uintptr_t d_align = (eg_uintptr_t)d8 & ALIGN_MASK;

	// Make the address aligned*/
	if(d_align) {
		d_align = ALIGN_MASK + 1 - d_align;
		while(d_align && len) {
			SET8(0);
			len--;
			d_align--;
		}
	}

	uint32_t *d32 = (uint32_t *)d8;
	while(len > 32) {
		REPEAT8(SET32(0));
		len -= 32;
	}

	while(len > 4) {
		SET32(0);
		len -= 4;
	}

	d8 = (uint8_t *)d32;
	while(len) {
		SET8(0);
		len--;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

// Same as `memset(dst, 0xFF, len)` but optimized for 4 byte operation.
void EG_ATTRIBUTE_FAST_MEM EG_SetMemFF(void *dst, size_t len)
{
	uint8_t *d8 = (uint8_t *)dst;
	uintptr_t d_align = (eg_uintptr_t)d8 & ALIGN_MASK;

	// Make the address aligned*/
	if(d_align) {
		d_align = ALIGN_MASK + 1 - d_align;
		while(d_align && len) {
			SET8(0xFF);
			len--;
			d_align--;
		}
	}

	uint32_t *d32 = (uint32_t *)d8;
	while(len > 32) {
		REPEAT8(SET32(0xFFFFFFFF));
		len -= 32;
	}

	while(len > 4) {
		SET32(0xFFFFFFFF);
		len -= 4;
	}

	d8 = (uint8_t *)d32;
	while(len) {
		SET8(0xFF);
		len--;
	}
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////

#if EG_MEM_CUSTOM == 0
static void lv_mem_walker(void *ptr, size_t size, int used, void *user)
{
	EG_UNUSED(ptr);

	EG_MonitorMem_t *mon_p = (EG_MonitorMem_t*)user;
	if(used) {
		mon_p->used_cnt++;
	}
	else {
		mon_p->free_cnt++;
		mon_p->free_size += size;
		if(size > mon_p->free_biggest_size)
			mon_p->free_biggest_size = size;
	}
}
#endif
