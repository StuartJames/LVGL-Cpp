/**
 * @file lv_ffmpeg.h
 *
 */
#pragma once

#include "EGL.h"
#if EG_USE_FFMPEG != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
struct ffmpeg_context_s;

extern const EG_ClassType_t lv_ffmpeg_player_class;

typedef struct {
    lv_img_t img;
    EGTimer * timer;
    lv_img_dsc_t imgdsc;
    bool auto_restart;
    struct ffmpeg_context_s * ffmpeg_ctx;
} lv_ffmpeg_player_t;

typedef enum {
    EG_FFMPEG_PLAYER_CMD_START,
    EG_FFMPEG_PLAYER_CMD_STOP,
    EG_FFMPEG_PLAYER_CMD_PAUSE,
    EG_FFMPEG_PLAYER_CMD_RESUME,
    _EG_FFMPEG_PLAYER_CMD_LAST
} lv_ffmpeg_player_cmd_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register FFMPEG image decoder
 */
void lv_ffmpeg_init(void);

/**
 * Get the number of frames contained in the file
 * @param path image or video file name
 * @return Number of frames, less than 0 means failed
 */
int lv_ffmpeg_get_frame_num(const char * path);

/**
 * Create ffmpeg_player object
 * @param parent pointer to an object, it will be the parent of the new player
 * @return pointer to the created ffmpeg_player
 */
EGObject * lv_ffmpeg_player_create(EGObject * parent);

/**
 * Set the path of the file to be played
 * @param obj pointer to a ffmpeg_player object
 * @param path video file path
 * @return EG_RES_OK: no error; EG_RES_INVALID: can't get the info.
 */
EG_Result_t lv_ffmpeg_player_set_src(EGObject * obj, const char * path);

/**
 * Set command control video player
 * @param obj pointer to a ffmpeg_player object
 * @param cmd control commands
 */
void lv_ffmpeg_player_set_cmd(EGObject * obj, lv_ffmpeg_player_cmd_t cmd);

/**
 * Set the video to automatically replay
 * @param obj pointer to a ffmpeg_player object
 * @param en true: enable the auto restart
 */
void lv_ffmpeg_player_set_auto_restart(EGObject * obj, bool en);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*EG_USE_FFMPEG*/
