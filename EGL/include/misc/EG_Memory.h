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
 
#pragma once

#include "../EG_IntrnlConfig.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "EG_Types.h"

//////////////////////////////////////////////////////////////////////////////////////

/**
 * Heap information structure.
 */
typedef struct {
    uint32_t total_size; /**< Total heap size*/
    uint32_t free_cnt;
    uint32_t free_size; /**< Size of available memory*/
    uint32_t free_biggest_size;
    uint32_t used_cnt;
    uint32_t max_used; /**< Max size of Heap memory used*/
    uint8_t used_pct; /**< Percentage used*/
    uint8_t frag_pct; /**< Amount of fragmentation*/
} EG_MonitorMem_t;

typedef struct {
    void       *pBuffer;
    uint16_t    size;
    uint8_t     used : 1;
} EG_MemoryBuffer_t;

typedef EG_MemoryBuffer_t EG_MemoryBufferArr_t[EG_MEM_BUF_MAX_NUM];

/**
 * Initialize the dyn_mem module (work memory and other variables)
 */
void EG_InitMem(void);

/**
 * Clean up the memory buffer which frees all the allocated memories.
 * @note It work only if `EG_MEM_CUSTOM == 0`
 */
void EG_DeinitMem(void);

/**
 * Allocate a memory dynamically
 * @param size size of the memory to allocate in bytes
 * @return pointer to the allocated memory
 */
void * EG_AllocMem(size_t size);

/**
 * Free an allocated data
 * @param data pointer to an allocated memory
 */
void EG_FreeMem(void * data);

/**
 * Reallocate a memory with a new size. The old content will be kept.
 * @param data pointer to an allocated memory.
 * Its content will be copied to the new memory block and freed
 * @param new_size the desired new size in byte
 * @return pointer to the new memory, NULL on failure
 */
void * EG_ReallocMem(void * data_p, size_t new_size);

/**
 *
 * @return
 */
EG_Result_t EG_TestMem(void);

/**
 * Give information about the work memory of dynamic allocation
 * @param mon_p pointer to a EG_MonitorMem_t variable,
 *              the result of the analysis will be stored here
 */
void EG_MonitorMem(EG_MonitorMem_t * mon_p);

/**
 * Get a temporal buffer with the given size.
 * @param size the required size
 */
void * EG_GetBufferMem(uint32_t size);

/**
 * Release a memory buffer
 * @param p buffer to release
 */
void EG_ReleaseBufferMem(void * p);

/**
 * Free all memory buffers
 */
void EG_FreeAllBuffers(void);

//! @cond Doxygen_Suppress

#if EG_MEMCPY_MEMSET_STD

/**
 * Wrapper for the standard memcpy
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
static inline void * EG_CopyMem(void * dst, const void * src, size_t len)
{
    return memcpy(dst, src, len);
}

/**
 * Wrapper for the standard memcpy
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
static inline void * EG_CopyMemSmall(void * dst, const void * src, size_t len)
{
    return memcpy(dst, src, len);
}

/**
 * Wrapper for the standard memset
 * @param dst pointer to the destination buffer
 * @param v value to set [0..255]
 * @param len number of byte to set
 */
static inline void EG_SetMem(void * dst, uint8_t v, size_t len)
{
    memset(dst, v, len);
}

/**
 * Wrapper for the standard memset with fixed 0x00 value
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
static inline void EG_ZeroMem(void * dst, size_t len)
{
    memset(dst, 0x00, len);
}

/**
 * Wrapper for the standard memset with fixed 0xFF value
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
static inline void EG_SetMemFF(void * dst, size_t len)
{
    memset(dst, 0xFF, len);
}

#else
/**
 * Same as `memcpy` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
void * /* EG_ATTRIBUTE_FAST_MEM */ EG_CopyMem(void * dst, const void * src, size_t len);

/**
 * Same as `memcpy` but optimized to copy only a few bytes.
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
static inline void * EG_ATTRIBUTE_FAST_MEM EG_CopyMemSmall(void * dst, const void * src, size_t len)
{
    uint8_t * d8 = (uint8_t *)dst;
    const uint8_t * s8 = (const uint8_t *)src;

    while(len) {
        *d8 = *s8;
        d8++;
        s8++;
        len--;
    }

    return dst;
}

/**
 * Same as `memset` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param v value to set [0..255]
 * @param len number of byte to set
 */
void /* EG_ATTRIBUTE_FAST_MEM */ EG_SetMem(void * dst, uint8_t v, size_t len);

/**
 * Same as `memset(dst, 0x00, len)` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
void /* EG_ATTRIBUTE_FAST_MEM */ EG_ZeroMem(void * dst, size_t len);

/**
 * Same as `memset(dst, 0xFF, len)` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
void /* EG_ATTRIBUTE_FAST_MEM */ EG_SetMemFF(void * dst, size_t len);

//! @endcond

#endif

