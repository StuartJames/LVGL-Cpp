/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-15     Meco Man     The first version
 */

#ifndef EG_RT_THREAD_CONF_H
#define EG_RT_THREAD_CONF_H

#ifdef __RTTHREAD__

#define EG_RTTHREAD_INCLUDE <rtthread.h>
#include EG_RTTHREAD_INCLUDE


#ifdef RT_USING_HEAP
#  define EG_MEM_CUSTOM 1
#  define EG_MEM_CUSTOM_INCLUDE EG_RTTHREAD_INCLUDE
#  define EG_MEM_CUSTOM_ALLOC   rt_malloc
#  define EG_MEM_CUSTOM_FREE    rt_free
#  define EG_MEM_CUSTOM_REALLOC rt_realloc
#endif


#define EG_TICK_CUSTOM 1
#define EG_TICK_CUSTOM_INCLUDE EG_RTTHREAD_INCLUDE
#define EG_TICK_CUSTOM_SYS_TIME_EXPR (rt_tick_get_millisecond())    /*Expression evaluating to current system time in ms*/

#ifdef PKG_EGL_DISP_REFR_PERIOD
#define EG_DISP_DEF_REFR_PERIOD   PKG_EGL_DISP_REFR_PERIOD
#endif

#define EG_CUSTOM_ASSERT_HANDLER 1
#define EG_ASSERT_HANDLER_INCLUDE EG_RTTHREAD_INCLUDE
#define EG_ASSERT_HANDLER RT_ASSERT(0);

#define EG_SPRINTF_CUSTOM 1
#define EG_SPRINTF_INCLUDE EG_RTTHREAD_INCLUDE
#define eg_snprintf  rt_snprintf
#define eg_vsnprintf rt_vsnprintf
#define EG_SPRINTF_USE_FLOAT 0

#ifdef ARCH_CPU_BIG_ENDIAN
#  define EG_BIG_ENDIAN_SYSTEM 1
#else
#  define EG_BIG_ENDIAN_SYSTEM 0
#endif

#ifdef rt_align /* >= RT-Thread v5.0.0 */
#  define EG_ATTRIBUTE_MEM_ALIGN rt_align(RT_ALIGN_SIZE)
#else
#  define EG_ATTRIBUTE_MEM_ALIGN ALIGN(RT_ALIGN_SIZE)
#endif


#endif

#endif
