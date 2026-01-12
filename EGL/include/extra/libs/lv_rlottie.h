/**
 * @file lv_rlottie.h
 *
 */

#pragma once

#include "EGL.h"
#if EG_USE_RLOTTIE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LV_RLOTTIE_CTRL_FORWARD  = 0,
    LV_RLOTTIE_CTRL_BACKWARD = 1,
    LV_RLOTTIE_CTRL_PAUSE    = 2,
    LV_RLOTTIE_CTRL_PLAY     = 0, /* Yes, play = 0 is the default mode */
    LV_RLOTTIE_CTRL_LOOP     = 8,
} lv_rlottie_ctrl_t;

/** definition in lottieanimation_capi.c */
struct Lottie_Animation_S;
typedef struct {
    lv_img_t img_ext;
    struct Lottie_Animation_S * animation;
    EGTimer * task;
    lv_img_dsc_t imgdsc;
    size_t total_frames;
    size_t current_frame;
    size_t framerate;
    uint32_t * allocated_buf;
    size_t allocated_buffer_size;
    size_t scanline_width;
    lv_rlottie_ctrl_t play_ctrl;
    size_t dest_frame;
} lv_rlottie_t;

extern const EG_ClassType_t lv_rlottie_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

EGObject * lv_rlottie_create_from_file(EGObject * parent, EG_Coord_t width, EG_Coord_t height, const char * path);

EGObject * lv_rlottie_create_from_raw(EGObject * parent, EG_Coord_t width, EG_Coord_t height,
                                      const char * rlottie_desc);

void lv_rlottie_set_play_mode(EGObject * rlottie, const lv_rlottie_ctrl_t ctrl);
void lv_rlottie_set_current_frame(EGObject * rlottie, const size_t goto_frame);

/**********************
 *      MACROS
 **********************/

#endif 