/**
 * @file EG_DrawSoftDither.h
 *
 */

#pragma once

#include "../../core/EG_ObjPosition.h"

#if EG_COLOR_DEPTH < 32 && EG_DITHER_GRADIENT == 1
#define _DITHER_GRADIENT 1
#else
#define _DITHER_GRADIENT 0
#endif

#if _DITHER_GRADIENT
/*A signed error color component*/
typedef struct {
    int8_t r, g, b;
} EG_SColor24_t;

struct EG_Gradient_t;
typedef void (*EG_DitherFunc_t)(struct EG_Gradient_t * grad, EG_Coord_t x, EG_Coord_t y, EG_Coord_t w);

#endif

#if EG_DRAW_COMPLEX
#if _DITHER_GRADIENT
void EG_ATTRIBUTE_FAST_MEM EG_DitherNone(struct EG_Gradient_t * grad, EG_Coord_t x, EG_Coord_t y,
                                                EG_Coord_t w);

void EG_ATTRIBUTE_FAST_MEM EG_DitherOrderedHorizontal(struct EG_Gradient_t * grad, const EG_Coord_t xs,
                                                       const EG_Coord_t y, const EG_Coord_t w);
void EG_ATTRIBUTE_FAST_MEM EG_DitherOrderedVertical(struct EG_Gradient_t * grad, const EG_Coord_t xs,
                                                       const EG_Coord_t y, const EG_Coord_t w);

#if EG_DITHER_ERROR_DIFFUSION == 1
void EG_ATTRIBUTE_FAST_MEM EG_DitherErrorDiffHorizontal(struct EG_Gradient_t * grad, const EG_Coord_t xs,
                                                        const EG_Coord_t y, const EG_Coord_t w);
void EG_ATTRIBUTE_FAST_MEM EG_DitherErrorDiffVertical(struct EG_Gradient_t * grad, const EG_Coord_t xs,
                                                        const EG_Coord_t y, const EG_Coord_t w);
#endif /* EG_DITHER_ERROR_DIFFUSION */

#endif 
#endif

