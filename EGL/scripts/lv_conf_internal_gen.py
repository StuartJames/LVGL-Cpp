#!/usr/bin/env python3

'''
Generates EG_IntrnlConfig.h from EG_ConfTemplate.h to provide default values
'''

import os
import sys
import re

SCRIPT_DIR = os.path.dirname(__file__)
EG_CONF_TEMPLATE = os.path.join(SCRIPT_DIR, "..", "lv_conf_template.h")
EG_CONF_INTERNAL = os.path.join(SCRIPT_DIR, "..", "src", "EG_IntrnlConfig.h")

if sys.version_info < (3,6,0):
  print("Python >=3.6 is required", file=sys.stderr)
  exit(1)

fin = open(EG_CONF_TEMPLATE)
fout = open(EG_CONF_INTERNAL, "w")

fout.write(
'''/**
 * GENERATED FILE, DO NOT EDIT IT!
 * @file EG_IntrnlConfig.h
 * Make sure all the defines of EG_Config.h have a default value
**/

#ifndef EG_CONF_INTERNAL_H
#define EG_CONF_INTERNAL_H
/* clang-format off */

#include <stdint.h>

/* Handle special Kconfig options */
#ifndef EG_KCONFIG_IGNORE
    #include "lv_conf_kconfig.h"
    #ifdef CONFIG_EG_CONF_SKIP
        #define EG_CONF_SKIP
    #endif
#endif

/*If "EG_Config.h" is available from here try to use it later.*/
#ifdef __has_include
    #if __has_include("EG_Config.h")
        #ifndef EG_CONF_INCLUDE_SIMPLE
            #define EG_CONF_INCLUDE_SIMPLE
        #endif
    #endif
#endif

/*If EG_Config.h is not skipped include it*/
#ifndef EG_CONF_SKIP
    #ifdef EG_CONF_PATH                           /*If there is a path defined for EG_Config.h use it*/
        #define __EG_TO_STR_AUX(x) #x
        #define __EG_TO_STR(x) __EG_TO_STR_AUX(x)
        #include __EG_TO_STR(EG_CONF_PATH)
        #undef __EG_TO_STR_AUX
        #undef __EG_TO_STR
    #elif defined(EG_CONF_INCLUDE_SIMPLE)         /*Or simply include EG_Config.h is enabled*/
        #include "EG_Config.h"
    #else
        #include "../../EG_Config.h"                /*Else assume EG_Config.h is next to the lvgl folder*/
    #endif
    #if !defined(EG_CONF_H) && !defined(EG_CONF_SUPPRESS_DEFINE_CHECK)
        /* #include will sometimes silently fail when __has_include is used */
        /* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80753 */
        #pragma message("Possible failure to include EG_Config.h, please read the comment in this file if you get errors")
    #endif
#endif

#ifdef CONFIG_EG_COLOR_DEPTH
    #define _EG_KCONFIG_PRESENT
#endif

/*----------------------------------
 * Start parsing lv_conf_template.h
 -----------------------------------*/
'''
)

started = 0

for line in fin.read().splitlines():
  if not started:
    if '#define EG_CONF_H' in line:
      started = 1
      continue
    else:
      continue

  if '/*--END OF EG_CONF_H--*/' in line: break

  #Is there a #define in this line?
  r = re.search(r'^([\s]*)#[\s]*(undef|define)[\s]+([^\s]+).*$', line)   # \s means any white space character

  if r:
    indent = r[1]

    name = r[3]
    name = re.sub('\(.*?\)', '', name, 1)    #remove parentheses from macros. E.g. MY_FUNC(5) -> MY_FUNC

    line = re.sub('[\s]*', '', line, 1)

    #If the value should be 1 (enabled) by default use a more complex structure for Kconfig checks because
    #if a not defined CONFIG_... value should be interpreted as 0 and not the LVGL default
    is_one = re.search(r'#[\s]*define[\s]*[A-Z0-9_]+[\s]+1([\s]*$|[\s]+)', line)
    if is_one:
      #1. Use the value if already set from EG_Config.h or anything else (i.e. do nothing)
      #2. In Kconfig environment use the CONFIG_... value if set, else use 0
      #3. In not Kconfig environment use the LVGL's default value

      fout.write(
        f'{indent}#ifndef {name}\n'
        f'{indent}    #ifdef _EG_KCONFIG_PRESENT\n'
        f'{indent}        #ifdef CONFIG_{name.upper()}\n'
        f'{indent}            #define {name} CONFIG_{name.upper()}\n'
        f'{indent}        #else\n'
        f'{indent}            #define {name} 0\n'
        f'{indent}        #endif\n'
        f'{indent}    #else\n'
        f'{indent}        {line}\n'
        f'{indent}    #endif\n'
        f'{indent}#endif\n'
      )
    else:
      #1. Use the value if already set from EG_Config.h or anything else  (i.e. do nothing)
      #2. Use the Kconfig value if set
      #3. Use the LVGL's default value

      fout.write(
        f'{indent}#ifndef {name}\n'
        f'{indent}    #ifdef CONFIG_{name.upper()}\n'
        f'{indent}        #define {name} CONFIG_{name.upper()}\n'
        f'{indent}    #else\n'
        f'{indent}        {line}\n'
        f'{indent}    #endif\n'
        f'{indent}#endif\n'
      )

  elif re.search('^ *typedef .*;.*$', line):
    continue   #ignore typedefs to avoid redeclaration
  else:
    fout.write(f'{line}\n')

fout.write(
'''

/*----------------------------------
 * End of parsing lv_conf_template.h
 -----------------------------------*/

EG_EXPORT_CONST_INT(EG_DPI_DEF);

#undef _EG_KCONFIG_PRESENT


/*Set some defines if a dependency is disabled*/
#if EG_USE_LOG == 0
    #define EG_LOG_LEVEL            EG_LOG_LEVEL_NONE
    #define EG_LOG_TRACE_MEM        0
    #define EG_LOG_TRACE_TIMER      0
    #define EG_LOG_TRACE_INDEV      0
    #define EG_LOG_TRACE_DISP_REFR  0
    #define EG_LOG_TRACE_EVENT      0
    #define EG_LOG_TRACE_OBJ_CREATE 0
    #define EG_LOG_TRACE_LAYOUT     0
    #define EG_LOG_TRACE_ANIM       0
#endif  /*EG_USE_LOG*/


/*If running without EG_Config.h add typedefs with default value*/
#ifdef EG_CONF_SKIP
    #if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)    /*Disable warnings for Visual Studio*/
        #define _CRT_SECURE_NO_WARNINGS
    #endif
#endif  /*defined(EG_CONF_SKIP)*/

#endif  /*EG_CONF_INTERNAL_H*/
'''
)

fin.close()
fout.close()
