/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     the first version
 * 2022-05-10     Meco Man     improve rt-thread initialization process
 */

#ifdef __RTTHREAD__

#include <EGL.h>
#include <rtthread.h>

#define DBG_TAG    "EGL"
#define DBG_EGL    DBG_INFO
#include <rtdbg.h>

#ifndef PKG_EGL_THREAD_STACK_SIZE
#define PKG_EGL_THREAD_STACK_SIZE 4096
#endif /* PKG_EGL_THREAD_STACK_SIZE */

#ifndef PKG_EGL_THREAD_PRIO
#define PKG_EGL_THREAD_PRIO (RT_THREAD_PRIORITY_MAX*2/3)
#endif /* PKG_EGL_THREAD_PRIO */

extern void lv_port_disp_init(void);
extern void lv_port_indev_init(void);
extern void lv_user_gui_init(void);

static struct rt_thread EG_Thread;

#ifdef rt_align
rt_align(RT_ALIGN_SIZE)
#else
ALIGN(RT_ALIGN_SIZE)
#endif
static rt_uint8_t EG_ThreadStack[PKG_EGL_THREAD_STACK_SIZE];

#if EG_USE_LOG
static void EG_RT_Log(const char *buf)
{
  LOG_I(buf);
}
#endif

static void lvgl_thread_entry(void *parameter)
{
#if EG_USE_LOG
  lv_log_register_print_cb(EG_RT_Log);
#endif /* EG_USE_LOG */
  EG_Initialise();
  lv_port_disp_init();
  lv_port_indev_init();
  lv_user_gui_init();
  while(1){    /* handle the tasks of LVGL */
    EG_TaskHandler();
//    rt_thread_mdelay(EG_DISP_DEF_REFR_PERIOD);
  }
}

static int lvgl_thread_init(void)
{
  rt_err_t err;

  err = rt_thread_init(&EG_Thread, "LVGL", lvgl_thread_entry, RT_NULL, &EG_ThreadStack[0], sizeof(EG_ThreadStack), PKG_EGL_THREAD_PRIO, 10);
  if(err != RT_EOK){
    LOG_E("Failed to create LVGL thread");
    return -1;
  }
  rt_thread_startup(&EG_Thread);

  return 0;
}

INIT_ENV_EXPORT(lvgl_thread_init);

#endif /*__RTTHREAD__*/
