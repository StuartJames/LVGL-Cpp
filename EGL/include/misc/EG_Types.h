/**
 * @file EG_Types.h
 *
 */

#pragma once

#include <stdint.h>
#include "../EG_IntrnlConfig.h"

////////////////////////////////////////////////////////////////////////////////

#if EG_USE_LARGE_COORD
typedef int32_t EG_Coord_t;
#else
typedef int16_t EG_Coord_t;
#endif

#if EG_USE_LARGE_COORD
#define _EG_COORD_TYPE_SHIFT (29U)
#else
#define _EG_COORD_TYPE_SHIFT (13U)
#endif

#define _EG_COORD_TYPE_MASK (3 << _EG_COORD_TYPE_SHIFT)
#define _EG_COORD_TYPE(x) ((x)&_EG_COORD_TYPE_MASK)     /*Extract type specifiers*/
#define _EG_COORD_PLAIN(x) ((x) & ~_EG_COORD_TYPE_MASK) /*Remove type specifiers*/

#define _EG_COORD_TYPE_PX (0 << _EG_COORD_TYPE_SHIFT)
#define _EG_COORD_TYPE_SPEC (1 << _EG_COORD_TYPE_SHIFT)
#define _EG_COORD_TYPE_PX_NEG (3 << _EG_COORD_TYPE_SHIFT)

#define EG_COORD_IS_PX(x) (_EG_COORD_TYPE(x) == _EG_COORD_TYPE_PX ||        \
															 _EG_COORD_TYPE(x) == _EG_COORD_TYPE_PX_NEG ? \
														 true :                                         \
														 false)
#define EG_COORD_IS_SPEC(x) (_EG_COORD_TYPE(x) == _EG_COORD_TYPE_SPEC ? true : false)

#define EG_COORD_SET_SPEC(x) ((x) | _EG_COORD_TYPE_SPEC)

/*Special coordinates*/
#define _EG_PCT(x) (x < 0 ? EG_COORD_SET_SPEC(1000 - (x)) : EG_COORD_SET_SPEC(x))
#define EG_COORD_IS_PCT(x) ((EG_COORD_IS_SPEC(x) && _EG_COORD_PLAIN(x) <= 2000) ? true : false)
#define EG_COORD_GET_PCT(x) (_EG_COORD_PLAIN(x) > 1000 ? 1000 - _EG_COORD_PLAIN(x) : _EG_COORD_PLAIN(x))
#define EG_SIZE_CONTENT EG_COORD_SET_SPEC(2001)

EG_EXPORT_CONST_INT(EG_SIZE_CONTENT);

/*Max coordinate value*/
#define EG_COORD_MAX ((1 << _EG_COORD_TYPE_SHIFT) - 1)
#define EG_COORD_MIN (-EG_COORD_MAX)

EG_EXPORT_CONST_INT(EG_COORD_MAX);
EG_EXPORT_CONST_INT(EG_COORD_MIN);

#define _EG_TRANSFORM_TRIGO_SHIFT 10

// If __UINTPTR_MAX__ or UINTPTR_MAX are available, use them to determine arch size
#if defined(__UINTPTR_MAX__) && __UINTPTR_MAX__ > 0xFFFFFFFF
#define EG_ARCH_64

#elif defined(UINTPTR_MAX) && UINTPTR_MAX > 0xFFFFFFFF
#define EG_ARCH_64

// Otherwise use compiler-dependent means to determine arch size
#elif defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined (__aarch64__)
#define EG_ARCH_64

#endif


// LEGL error codes.
enum {
    EG_RES_INVALID = 0, // indicates an object has been deleted (become invalid) after the operation
    EG_RES_OK,      // The object is valid (not deleted) after the operation
};
typedef uint8_t EG_Result_t;

#if defined(__cplusplus) || __STDC_VERSION__ >= 199901L
// If c99 or newer,  use the definition of uintptr_t directly from <stdint.h>
typedef uintptr_t eg_uintptr_t;

#else

// Otherwise, use the arch size determination
#ifdef EG_ARCH_64
typedef uint64_t eg_uintptr_t;
#else
typedef uint32_t eg_uintptr_t;
#endif

#endif

#define EG_UNUSED(x) ((void)x)

#define _EG_CONCAT(x, y) x ## y
#define EG_CONCAT(x, y) _EG_CONCAT(x, y)

#define _EG_CONCAT3(x, y, z) x ## y ## z
#define EG_CONCAT3(x, y, z) _EG_CONCAT3(x, y, z)

#if defined(PYCPARSER) || defined(__CC_ARM)
#define EG_FORMAT_ATTRIBUTE(fmtstr, vararg)
#elif defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
#define EG_FORMAT_ATTRIBUTE(fmtstr, vararg) __attribute__((format(gnu_printf, fmtstr, vararg)))
#elif (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__))
#define EG_FORMAT_ATTRIBUTE(fmtstr, vararg) __attribute__((format(printf, fmtstr, vararg)))
#else
#define EG_FORMAT_ATTRIBUTE(fmtstr, vararg)
#endif


