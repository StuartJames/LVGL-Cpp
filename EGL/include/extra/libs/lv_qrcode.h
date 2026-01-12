/**
 * @file lv_qrcode
 *
 */

#pragma once

#include "EGL.h"
#if EG_USE_QRCODE

/*********************
 *      DEFINES
 *********************/

extern const EG_ClassType_t lv_qrcode_class;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an empty QR code (an `lv_canvas`) object.
 * @param parent point to an object where to create the QR code
 * @param size width and height of the QR code
 * @param dark_color dark color of the QR code
 * @param light_color light color of the QR code
 * @return pointer to the created QR code object
 */
EGObject * lv_qrcode_create(EGObject * parent, EG_Coord_t size, EG_Color_t dark_color, EG_Color_t light_color);

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param data data to display
 * @param data_len length of data in bytes
 * @return EG_RES_OK: if no error; EG_RES_INVALID: on error
 */
EG_Result_t lv_qrcode_update(EGObject * qrcode, const void * data, uint32_t data_len);

/**
 * DEPRECATED: Use normal lv_obj_del instead
 * Delete a QR code object
 * @param qrcode pointer to a QR code object
 */
void lv_qrcode_delete(EGObject * qrcode);

/**********************
 *      MACROS
 **********************/

#endif 

