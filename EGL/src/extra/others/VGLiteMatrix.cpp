/**
 * MIT License
 *
 * Copyright 2022, 2023 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next paragraph)
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "EG_IntrnlConfig.h"

#if EG_USE_GPU_NXP_VG_LITE

#include <math.h>
#include <string.h>
#include "extra/others/VGLite.h"

#define VG_SW_BLIT_PRECISION_OPT 1

//////////////////////////////////////////////////////////////////////////////////////

VGLiteError_e vg_lite_identity(VGLiteMatrix_t *matrix)
{
	// Set identify matrix.
	matrix->m[0][0] = 1.0f;
	matrix->m[0][1] = 0.0f;
	matrix->m[0][2] = 0.0f;
	matrix->m[1][0] = 0.0f;
	matrix->m[1][1] = 1.0f;
	matrix->m[1][2] = 0.0f;
	matrix->m[2][0] = 0.0f;
	matrix->m[2][1] = 0.0f;
	matrix->m[2][2] = 1.0f;
#if VG_SW_BLIT_PRECISION_OPT
	matrix->scaleX = 1.0f;
	matrix->scaleY = 1.0f;
	matrix->angle = 0.0f;
#endif
	return VG_LITE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////

static void multiply(VGLiteMatrix_t *matrix, VGLiteMatrix_t *mult)
{
VGLiteMatrix_t temp;
int row, column;

	for(row = 0; row < 3; row++) {    // Process all rows and columns.
		for(column = 0; column < 3; column++) {		// Compute matrix entry.
			temp.m[row][column] = (matrix->m[row][0] * mult->m[0][column]) + (matrix->m[row][1] * mult->m[1][column]) + (matrix->m[row][2] * mult->m[2][column]);
		}
	}
#if VG_SW_BLIT_PRECISION_OPT
	memcpy(matrix, &temp, sizeof(vg_lite_float_t) * 9);  // Copy temporary matrix into result.
  #else
	memcpy(matrix, &temp, sizeof(temp));
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

VGLiteError_e vg_lite_translate(vg_lite_float_t x, vg_lite_float_t y, VGLiteMatrix_t *matrix)
{
	VGLiteMatrix_t t = {{    // Set translation matrix.
    {1.0f, 0.0f, x},
      {0.0f, 1.0f, y},
      {0.0f, 0.0f, 1.0f},
    },
    0.0f,
    0.0f,
    0.0f
  };
	multiply(matrix, &t);	// Multiply with current matrix.

	return VG_LITE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////

VGLiteError_e vg_lite_scale(vg_lite_float_t scale_x, vg_lite_float_t scale_y, VGLiteMatrix_t *matrix)
{
	VGLiteMatrix_t s = {{    // Set scale matrix.
    {scale_x, 0.0f, 0.0f},
      {0.0f, scale_y, 0.0f},
      {0.0f, 0.0f, 1.0f},
    },
    0.0f,
    0.0f,
    0.0f
  };
	multiply(matrix, &s);	// Multiply with current matrix.
#if VG_SW_BLIT_PRECISION_OPT
	matrix->scaleX = matrix->scaleX * scale_x;
	matrix->scaleY = matrix->scaleY * scale_y;
#endif  // VG_SW_BLIT_PRECISION_OPT
	return VG_LITE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////

VGLiteError_e vg_lite_rotate(vg_lite_float_t degrees, VGLiteMatrix_t *matrix)
{
	vg_lite_float_t angle = (degrees / 180.0f) * 3.141592654f;  // Convert degrees into radians.
	vg_lite_float_t cos_angle = cosf(angle);                    // Compute cosine and sine values.
	vg_lite_float_t sin_angle = sinf(angle);
	VGLiteMatrix_t r = {{    // Set rotation matrix.
    {cos_angle, -sin_angle, 0.0f},
      {sin_angle, cos_angle, 0.0f},
      {0.0f, 0.0f, 1.0f},
    },
    0.0f,
    0.0f,
    0.0f
  };
	multiply(matrix, &r);	// Multiply with current matrix.
#if VG_SW_BLIT_PRECISION_OPT
	matrix->angle = matrix->angle + degrees;
	if(matrix->angle >= 360) {
		vg_lite_uint32_t count = (vg_lite_uint32_t)matrix->angle / 360;
		matrix->angle = matrix->angle - count * 360;
	}
#endif
	return VG_LITE_SUCCESS;
}

#endif
