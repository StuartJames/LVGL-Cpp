/**
 * @file EG_HAL.h
 *
 */

#pragma once

#include "EG_HALDisplay.h"
#include "EG_HALInputDevice.h"
#include "EG_HALTick.h"

/**
 * Same as Android's DIP. (Different name is chosen to avoid mistype between EG_DPI and EG_DIP)
 * 1 dip is 1 px on a 160 DPI screen
 * 1 dip is 2 px on a 320 DPI screen
 * https://stackoverflow.com/questions/2025282/what-is-the-difference-between-px-dip-dp-and-sp
 */
#define _EG_DPX_CALC(dpi, n)   ((n) == 0 ? 0 :EG_MAX((( (dpi) * (n) + 80) / 160), 1)) /*+80 for rounding*/
#define EG_DPX(n)   _EG_DPX_CALC(EGDisplay::GetDPI(NULL), n)

