/*
 *                EGL 2025-2026 HydraSystems.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Based on a design by LVGL Kft
 *
 * =====================================================================
 *
 * Edit     Date     Version       Edit Description
 * ====  ==========  ======= ===========================================
 * SJ    2025/08/18   8.4.0    Original by LVGL Kft
 * SJ    2026/07/20   8.6.0    Modified file layoout & class naming
 *
 */

#include "EG_IntrnlConfig.h"

#if EG_USE_GPU_SDL

#include "draw/sdl/EG_SDL_Context.h"

///////////////////////////////////////////////////////////////////////////////////////

// Based heavily on http://vitiy.info/Code/stackblur.cpp
// See http://vitiy.info/stackblur-algorithm-multi-threaded-blur-for-cpp/
// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>

static unsigned short const StackBlurMultiply[255] = {
	512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292, 512,
	454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292, 273, 512,
	482, 454, 428, 405, 383, 364, 345, 328, 312, 298, 284, 271, 259, 496, 475, 456,
	437, 420, 404, 388, 374, 360, 347, 335, 323, 312, 302, 292, 282, 273, 265, 512,
	497, 482, 468, 454, 441, 428, 417, 405, 394, 383, 373, 364, 354, 345, 337, 328,
	320, 312, 305, 298, 291, 284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456,
	446, 437, 428, 420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335,
	329, 323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265, 261, 512,
	505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428, 422, 417, 411, 405,
	399, 394, 389, 383, 378, 373, 368, 364, 359, 354, 350, 345, 341, 337, 332, 328,
	324, 320, 316, 312, 309, 305, 301, 298, 294, 291, 287, 284, 281, 278, 274, 271,
	268, 265, 262, 259, 257, 507, 501, 496, 491, 485, 480, 475, 470, 465, 460, 456,
	451, 446, 442, 437, 433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388,
	385, 381, 377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335,
	332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297, 294, 292,
	289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259};

static unsigned char const StackBlurShiftRight[255] = {
	9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
	17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24};

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::StackBlurGrayscale(EG_OPA_t *pBuffer, uint16_t Width, uint16_t Height, uint16_t Radius)
{
	StackBlurJob(pBuffer, Width, Height, Radius, 1, 0, 1);
	StackBlurJob(pBuffer, Width, Height, Radius, 1, 0, 2);
}

///////////////////////////////////////////////////////////////////////////////////////

void EGSDLContext::StackBlurJob(EG_OPA_t *pSrce, unsigned int Width, unsigned int Height, unsigned int Radius, int Cores, int Core, int Step)
{
	if(Radius < 2 || Radius > 254) return;  // Silently ignore bad radius

	unsigned int x, y, xp, yp, i;
	unsigned int sp;
	unsigned int stack_start;
	unsigned char *stack_ptr;

	EG_OPA_t *src_ptr;
	EG_OPA_t *dst_ptr;

	unsigned long sum_r;
	unsigned long sum_in_r;
	unsigned long sum_out_r;

	unsigned int wm = Width - 1;
	unsigned int hm = Height - 1;
	unsigned int stride = Width;
	unsigned int div = (Radius * 2) + 1;
	unsigned int mul_sum = StackBlurMultiply[Radius];
	unsigned char shr_sum = StackBlurShiftRight[Radius];
	unsigned char stack[254 * 2 + 1];

	if(Step == 1) {
		unsigned int minY = Core * Height / Cores;
		unsigned int maxY = (Core + 1) * Height / Cores;

		for(y = minY; y < maxY; y++) {
			sum_r =
				sum_in_r =
					sum_out_r = 0;

			src_ptr = pSrce + stride * y;  // start of line (0,y)

			for(i = 0; i <= Radius; i++) {
				stack_ptr = &stack[i];
				stack_ptr[0] = src_ptr[0];
				sum_r += src_ptr[0] * (i + 1);
				sum_out_r += src_ptr[0];
			}

			for(i = 1; i <= Radius; i++) {
				if(i <= wm) src_ptr += 1;
				stack_ptr = &stack[i + Radius];
				stack_ptr[0] = src_ptr[0];
				sum_r += src_ptr[0] * (Radius + 1 - i);
				sum_in_r += src_ptr[0];
			}

			sp = Radius;
			xp = Radius;
			if(xp > wm) xp = wm;
			src_ptr = pSrce + (xp + y * Width);  //   img.pix_ptr(xp, y);
			dst_ptr = pSrce + y * stride;        // img.pix_ptr(0, y);
			for(x = 0; x < Width; x++) {
				dst_ptr[0] = EG_CLAMP((sum_r * mul_sum) >> shr_sum, 0, 255);
				dst_ptr += 1;

				sum_r -= sum_out_r;

				stack_start = sp + div - Radius;
				if(stack_start >= div) stack_start -= div;
				stack_ptr = &stack[stack_start];

				sum_out_r -= stack_ptr[0];

				if(xp < wm) {
					src_ptr += 1;
					++xp;
				}

				stack_ptr[0] = src_ptr[0];

				sum_in_r += src_ptr[0];
				sum_r += sum_in_r;

				++sp;
				if(sp >= div) sp = 0;
				stack_ptr = &stack[sp];

				sum_out_r += stack_ptr[0];
				sum_in_r -= stack_ptr[0];
			}
		}
	}

	// Step 2
	if(Step == 2) {
		unsigned int minX = Core * Width / Cores;
		unsigned int maxX = (Core + 1) * Width / Cores;

		for(x = minX; x < maxX; x++) {
			sum_r =
				sum_in_r =
					sum_out_r = 0;

			src_ptr = pSrce + x;  // x,0
			for(i = 0; i <= Radius; i++) {
				stack_ptr = &stack[i];
				stack_ptr[0] = src_ptr[0];
				sum_r += src_ptr[0] * (i + 1);
				sum_out_r += src_ptr[0];
			}
			for(i = 1; i <= Radius; i++) {
				if(i <= hm) src_ptr += stride;  // +stride

				stack_ptr = &stack[i + Radius];
				stack_ptr[0] = src_ptr[0];
				sum_r += src_ptr[0] * (Radius + 1 - i);
				sum_in_r += src_ptr[0];
			}

			sp = Radius;
			yp = Radius;
			if(yp > hm) yp = hm;
			src_ptr = pSrce + (x + yp * Width);  // img.pix_ptr(x, yp);
			dst_ptr = pSrce + x;                 // img.pix_ptr(x, 0);
			for(y = 0; y < Height; y++) {
				dst_ptr[0] = EG_CLAMP((sum_r * mul_sum) >> shr_sum, 0, 255);
				dst_ptr += stride;

				sum_r -= sum_out_r;

				stack_start = sp + div - Radius;
				if(stack_start >= div) stack_start -= div;
				stack_ptr = &stack[stack_start];

				sum_out_r -= stack_ptr[0];

				if(yp < hm) {
					src_ptr += stride;  // stride
					++yp;
				}

				stack_ptr[0] = src_ptr[0];

				sum_in_r += src_ptr[0];
				sum_r += sum_in_r;

				++sp;
				if(sp >= div) sp = 0;
				stack_ptr = &stack[sp];

				sum_out_r += stack_ptr[0];
				sum_in_r -= stack_ptr[0];
			}
		}
	}
}

#endif
