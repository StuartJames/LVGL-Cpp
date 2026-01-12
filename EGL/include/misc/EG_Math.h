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

/////////////////////////////////////////////////////////////////////////////////

#define LV_TRIGO_SIN_MAX 32767
#define LV_TRIGO_SHIFT 15 /**<  >> LV_TRIGO_SHIFT to normalize*/

#define LV_BEZIER_VAL_MAX 1024 /**< Max time in Bezier functions (not [0..1] to use integers)*/
#define LV_BEZIER_VAL_SHIFT 10 /**< log2(LV_BEZIER_VAL_MAX): used to normalize up scaled values*/

/////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint16_t i;
    uint16_t f;
} lv_sqrt_res_t;

/////////////////////////////////////////////////////////////////////////////////

/**
 * Return with sinus of an angle
 * @param angle
 * @return sinus of 'angle'. sin(-90) = -32767, sin(90) = 32767
 */
int16_t /* EG_ATTRIBUTE_FAST_MEM */ EG_TrigoSin(int16_t angle);

static inline int16_t EG_ATTRIBUTE_FAST_MEM lv_trigo_cos(int16_t angle)
{
    return EG_TrigoSin(angle + 90);
}

/////////////////////////////////////////////////////////////////////////////////


/**
 * Calculate a value of a Cubic Bezier function.
 * @param t time in range of [0..LV_BEZIER_VAL_MAX]
 * @param u0 start values in range of [0..LV_BEZIER_VAL_MAX]
 * @param u1 control value 1 values in range of [0..LV_BEZIER_VAL_MAX]
 * @param u2 control value 2 in range of [0..LV_BEZIER_VAL_MAX]
 * @param u3 end values in range of [0..LV_BEZIER_VAL_MAX]
 * @return the value calculated from the given parameters in range of [0..LV_BEZIER_VAL_MAX]
 */
uint32_t EG_Bezier3(uint32_t t, uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3);

/**
 * Calculate the atan2 of a vector.
 * @param x
 * @param y
 * @return the angle in degree calculated from the given parameters in range of [0..360]
 */
uint16_t EG_Atan2(int x, int y);

//! @cond Doxygen_Suppress

/**
 * Get the square root of a number
 * @param x integer which square root should be calculated
 * @param q store the result here. q->i: integer part, q->f: fractional part in 1/256 unit
 * @param mask optional to skip some iterations if the magnitude of the root is known.
 * Set to 0x8000 by default.
 * If root < 16: mask = 0x80
 * If root < 256: mask = 0x800
 * Else: mask = 0x8000
 */
void /* EG_ATTRIBUTE_FAST_MEM */ EG_Sqrt(uint32_t x, lv_sqrt_res_t * q, uint32_t mask);

//! @endcond

/**
 * Calculate the integer exponents.
 * @param base
 * @param power
 * @return base raised to the power exponent
 */
int64_t EG_Pow(int64_t base, int8_t exp);

/**
 * Get the mapped of a number given an input and output range
 * @param x integer which mapped value should be calculated
 * @param min_in min input range
 * @param max_in max input range
 * @param min_out max output range
 * @param max_out max output range
 * @return the mapped number
 */
int32_t EG_Map(int32_t x, int32_t min_in, int32_t max_in, int32_t min_out, int32_t max_out);

/**
 * Get a pseudo random number in the given range
 * @param min   the minimum value
 * @param max   the maximum value
 * @return return the random number. min <= return_value <= max
 */
uint32_t EG_Rand(uint32_t min, uint32_t max);

/////////////////////////////////////////////////////////////////////////////////

#define EG_MIN(a, b) ((a) < (b) ? (a) : (b))
#define EG_MIN3(a, b, c) (EG_MIN(EG_MIN(a,b), c))
#define EG_MIN4(a, b, c, d) (EG_MIN(EG_MIN(a,b), EG_MIN(c,d)))

#define EG_MAX(a, b) ((a) > (b) ? (a) : (b))
#define EG_MAX3(a, b, c) (EG_MAX(EG_MAX(a,b), c))
#define EG_MAX4(a, b, c, d) (EG_MAX(EG_MAX(a,b), EG_MAX(c,d)))

#define LV_CLAMP(min, val, max) (EG_MAX(min, (EG_MIN(val, max))))

#define LV_ABS(x) ((x) > 0 ? (x) : (-(x)))
#define LV_UDIV255(x) (((x) * 0x8081U) >> 0x17)

#define LV_IS_SIGNED(t) (((t)(-1)) < ((t)0))
#define LV_UMAX_OF(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))
#define LV_SMAX_OF(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))
#define EG_MAX_OF(t) ((unsigned long)(LV_IS_SIGNED(t) ? LV_SMAX_OF(t) : LV_UMAX_OF(t)))

