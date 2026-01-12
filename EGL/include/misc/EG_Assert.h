/**
 * @file EG_Assert.h
 *
 */

#pragma once

#include "EG_IntrnlConfig.h"
#include "misc/EG_Log.h"
#include "misc/EG_Memory.h"
#include EG_ASSERT_HANDLER_INCLUDE

#define EG_ASSERT(expr)                                        \
    do {                                                       \
        if(!(expr)) {                                          \
            EG_LOG_ERROR("Asserted at expression: %s", #expr); \
            EG_ASSERT_HANDLER                                  \
        }                                                      \
    } while(0)

#define EG_ASSERT_MSG(expr, msg)                                         \
    do {                                                                 \
        if(!(expr)) {                                                    \
            EG_LOG_ERROR("Asserted at expression: %s (%s)", #expr, msg); \
            EG_ASSERT_HANDLER                                            \
        }                                                                \
    } while(0)

#if EG_USE_ASSERT_NULL
#   define EG_ASSERT_NULL(p) EG_ASSERT_MSG(p != NULL, "NULL pointer");
#else
#   define EG_ASSERT_NULL(p)
#endif

#if EG_USE_ASSERT_MALLOC
#   define EG_ASSERT_MALLOC(p) EG_ASSERT_MSG(p != NULL, "Out of memory");
#else
#   define EG_ASSERT_MALLOC(p)
#endif

#if EG_USE_ASSERT_MEM_INTEGRITY
#   define EG_ASSERT_MEM_INTEGRITY() EG_ASSERT_MSG(EG_TestMem() == EG_RES_OK, "Memory integrity error");
#else
#   define EG_ASSERT_MEM_INTEGRITY()
#endif
