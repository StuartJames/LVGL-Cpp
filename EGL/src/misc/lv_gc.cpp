/**
 * @file lv_gc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "misc/lv_gc.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

#if(!defined(EG_ENABLE_GC)) || EG_ENABLE_GC == 0
    EG_ROOTS
#endif /*EG_ENABLE_GC*/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void EG_GC_ClearRoots(void)
{
#define LV_CLEAR_ROOT(root_type, root_name) EG_ZeroMem(&EG_GC_ROOT(root_name), sizeof(EG_GC_ROOT(root_name)));
    EG_ITERATE_ROOTS(LV_CLEAR_ROOT)
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
