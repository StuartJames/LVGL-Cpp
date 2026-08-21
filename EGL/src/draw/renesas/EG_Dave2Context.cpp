/**
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "draw/renesas/EG_Dave2Context.h"
#include "core/EG_Refresh.h"
#include <math.h>

#if EG_USE_GPU_RA6M3_G2D

#include EG_GPU_RA6M3_G2D_INCLUDE
#include <fsp_features.h>
#include <bsp_api.h>
//#include <R7FA6M5BH.h>

//////////////////////////////////////////////////////////////////////////////////////

#ifdef LOG_ERRORS
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define ERROR_LIST_SIZE (4)
#define D2_EXEC(a) LogError(a, __func__, __LINE__)
#else
// here is error logging not enabled
#define D2_EXEC(a) a;
#endif

EG_OPA_t EGDave2Context::m_OPATable[256] = {0};
EG_OPA_t EGDave2Context::m_PreveviousOPA = EG_OPA_TRANSP;
uint32_t EGDave2Context::m_PreveviousBPP = 0;

//////////////////////////////////////////////////////////////////////////////////////

EGDave2Context::EGDave2Context() : EGSoftContext()
{
}

//////////////////////////////////////////////////////////////////////////////////////

EGDave2Context::~EGDave2Context()
{
  if(m_pFrameBackground[0] != nullptr) EG_FreeMem(m_pFrameBackground[0]);
  if(m_pFrameBackground[1] != nullptr) EG_FreeMem(m_pFrameBackground[1]);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::InitialiseContext(void)
{
	EGSoftContext::InitialiseContext();  // call the base class
	DrawImageDecodedProc = DrawImageDecoded;
	WaitForFinishProc = WaitForFinish;
	DrawCharacterProc = DrawCharacter;
	BlendProc = Blend;
//  CopyBufferProc = lv_draw_ra6m3_2d_buffer_copy;
	if(m_pD2Handle != nullptr) return;
	m_pD2Handle = d2_opendevice(0);
	if(m_pD2Handle == nullptr) return;
	if(d2_setdlistblocksize(m_pD2Handle, 25) != D2_OK) {  // set blocksize for default displaylist
		EG_LOG_ERROR("Could NOT d2_setdlistblocksize\n");
		d2_closedevice(m_pD2Handle);
		return;
	}
	if(d2_inithw(m_pD2Handle, 0) != D2_OK) {  // bind the hardware
		EG_LOG_ERROR("Could NOT d2_inithw\n");
		d2_closedevice(m_pD2Handle);
		return;
	}
	m_pRenderBuffer = d2_newrenderbuffer(m_pD2Handle, 20, 20);
	if(!m_pRenderBuffer) {
		EG_LOG_ERROR("NO m_pRenderBuffer\n");
		d2_closedevice(m_pD2Handle);
		return;
	}
  m_pFrameBackground[0] = (uint8_t*)EG_AllocMem(FrameBufferSize);
  m_pFrameBackground[1] = (uint8_t*)EG_AllocMem(FrameBufferSize);
  if((m_pFrameBackground[0] == nullptr) || (m_pFrameBackground[1] == nullptr)){
    if(m_pFrameBackground[0] != nullptr) EG_FreeMem(m_pFrameBackground[0]);
		EG_LOG_ERROR("NO m_pFrameBackground\n");
  }
}

//////////////////////////////////////////////////////////////////////////////////////

d2_s32 EGDave2Context::ColorFormatToD2(EG_ImageColorFormat_t cf)
{
	d2_s32 d2_cf;

#if(DLG_EGL_CF == 1)
	switch(cf & ~(1 << 5)) {
#else
	switch(cf) {
#endif  // (DLG_EGL_CF == 1)
		case EG_COLOR_FORMAT_NATIVE:
			d2_cf = d2_mode_rgb565;
			break;
		case EG_COLOR_FORMAT_NATIVE_CHROMA_KEYED:
			d2_cf = d2_mode_rgb565;
			break;
		case EG_COLOR_FORMAT_ALPHA_1BIT:
			d2_cf = d2_mode_alpha1;
			break;
		case EG_COLOR_FORMAT_ALPHA_2BIT:
			d2_cf = d2_mode_alpha2;
			break;
		case EG_COLOR_FORMAT_ALPHA_4BIT:
			d2_cf = d2_mode_alpha4;
			break;
		case EG_COLOR_FORMAT_ALPHA_8BIT:
			d2_cf = d2_mode_alpha8;
			break;
		case EG_COLOR_FORMAT_INDEXED_1BIT:
			d2_cf = d2_mode_i1 | d2_mode_clut;
			break;
		case EG_COLOR_FORMAT_INDEXED_2BIT:
			d2_cf = d2_mode_i2 | d2_mode_clut;
			break;
		case EG_COLOR_FORMAT_INDEXED_4BIT:
			d2_cf = d2_mode_i4 | d2_mode_clut;
			break;
		case EG_COLOR_FORMAT_INDEXED_8BIT:
			d2_cf = d2_mode_i8 | d2_mode_clut;
			break;
#if(DLG_EGL_CF == 1)
		case EG_COLOR_FORMAT_RGB565:
			d2_cf = d2_mode_rgb565;
			break;
		case EG_COLOR_FORMAT_RGB888:
			d2_cf = d2_mode_rgb888;
			break;
		case EG_COLOR_FORMAT_RGBA8888:
			d2_cf = d2_mode_rgba8888;
			break;
#endif  // DLG_EGL_CF
		default:
			return -1;
	}
#if(DLG_EGL_CF == 1)
	return d2_cf | (cf & (1 << 5) ? d2_mode_rle : 0);
#else
	return d2_cf;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGDave2Context::ColorFormatFBValid(d2_s32 cf)
{
	if((cf & (d2_mode_rle | d2_mode_clut)) || cf < 0) return false;
	switch(cf) {
		case d2_mode_alpha8:
		case d2_mode_rgb565:
		case d2_mode_argb8888:
		case d2_mode_argb4444:
		case d2_mode_rgba8888:
		case d2_mode_rgba4444:
			return true;
		default:
			return false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGDave2Context::HasAlpha(d2_s32 cf)
{
	switch(cf & ~(d2_mode_clut | d2_mode_rle)) {
		case d2_mode_argb8888:
		case d2_mode_rgba8888:
		case d2_mode_argb4444:
		case d2_mode_rgba4444:
		case d2_mode_argb1555:
		case d2_mode_rgba5551:
		case d2_mode_ai44:
		case d2_mode_i8:
		case d2_mode_i4:
		case d2_mode_i2:
		case d2_mode_i1:
		case d2_mode_alpha8:
		case d2_mode_alpha4:
		case d2_mode_alpha2:
		case d2_mode_alpha1:
			return true;
		default:
			return false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGDave2Context::IsAlpha(d2_s32 cf)
{
	switch(cf & ~d2_mode_rle) {
		case d2_mode_alpha8:
		case d2_mode_alpha4:
		case d2_mode_alpha2:
		case d2_mode_alpha1:
			return true;
		default:
			return false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

d2_color EGDave2Context::ColorToD2(EG_Color_t Color)
{
	uint8_t alpha, red, green, blue;

	alpha = 0xFF;
	red = Color.ch.red << 3 | Color.ch.red >> 2;
	green = Color.ch.green << 2 | Color.ch.green >> 4;
	blue = Color.ch.blue << 3 | Color.ch.blue >> 2;
	return (alpha) << 24UL | (red) << 16UL | (green) << 8UL | (blue) << 0UL;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::GetRecolorConsts(d2_color *pColorLow, d2_color *pColorHigh)
{
	d2_color Color = ColorToD2(m_DrawImage.m_Recolor);
	d2_alpha Red, Green, Blue, opa = m_DrawImage.m_RecolorOPA > EG_OPA_MAX ? EG_OPA_COVER : m_DrawImage.m_RecolorOPA;
	Red = ((uint32_t)((uint8_t)(Color >> 16UL)) * opa) / 255;
	Green = ((uint32_t)((uint8_t)(Color >> 8UL)) * opa) / 255;
	Blue = ((uint32_t)((uint8_t)(Color >> 0UL)) * opa) / 255;
	*pColorLow = Red << 16UL | Green << 8UL | Blue << 0UL;
	Red += 255 - opa;
	Green += 255 - opa;
	Blue += 255 - opa;
	*pColorHigh = Red << 16UL | Green << 8UL | Blue << 0UL;
}

//////////////////////////////////////////////////////////////////////////////////////

int EGDave2Context::HandleIndexedColor(const EG_Color_t **ppSrce, const d2_color **ppColorLUT, d2_s32 cf)
{
	int LUTLength = 0;

	if(cf & d2_mode_clut) {
		// Calculate CLUT length in entries
		switch(cf & ~(d2_mode_clut | d2_mode_rle)) {
			case d2_mode_i1:
				LUTLength = 2;
				break;
			case d2_mode_i2:
				LUTLength = 4;
				break;
			case d2_mode_i4:
				LUTLength = 16;
				break;
			case d2_mode_i8:
				LUTLength = 256;
				break;
			case d2_mode_ai44:
				LUTLength = 16;
				break;
			default:
				return 0;
		}
		*ppColorLUT = (const d2_color *)*ppSrce;
		*ppSrce = (const EG_Color_t *)((const uint32_t *)*ppSrce + LUTLength);
	}
	return LUTLength;
}

//////////////////////////////////////////////////////////////////////////////////////

int EGDave2Context::ColorFormatBPP(d2_s32 cf)
{
	switch(cf & ~(d2_mode_clut | d2_mode_rle)) {
		case d2_mode_argb8888:
			return 32;
		case d2_mode_rgba8888:
			return 32;
		case d2_mode_rgb888:
			return 32;
		case d2_mode_argb4444:
			return 16;
		case d2_mode_rgba4444:
			return 16;
		case d2_mode_argb1555:
			return 16;
		case d2_mode_rgba5551:
			return 16;
		case d2_mode_rgb565:
			return 16;
		case d2_mode_ai44:
			return 8;
		case d2_mode_i8:
			return 8;
		case d2_mode_i4:
			return 4;
		case d2_mode_i2:
			return 2;
		case d2_mode_i1:
			return 1;
		case d2_mode_alpha8:
			return 8;
		case d2_mode_alpha4:
			return 4;
		case d2_mode_alpha2:
			return 2;
		case d2_mode_alpha1:
			return 1;
		default:
			return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

d2_s32 EGDave2Context::GetDefColorFormat(void)
{
	return d2_mode_rgb565;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::ClearBlitConfig(void)
{
	m_AlphaEn = false;
	m_ColorKeyEn = false;
	m_BlendEn = true;
	m_ColorizeEn = false;
	m_DrawImage.Initialise();
	m_SrceColorFormatVal = GetDefColorFormat();
	m_DestColorFormatVal = GetDefColorFormat();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::InitGPU(void)
{
//	ClearBlitConfig();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::RotatePoint(int *x, int *y, float cos_angle, float sin_angle, int pivot_x, int pivot_y)
{
	float fx, fy;

	*x -= pivot_x;
	*y -= pivot_y;
	fx = ((float)*x) / 16.0f;
	fy = ((float)*y) / 16.0f;
	*x = (int)(((fx * cos_angle) - (fy * sin_angle)) * 16.0f);
	*y = (int)(((fx * sin_angle) + (fy * cos_angle)) * 16.0f);
	*x += pivot_x;
	*y += pivot_y;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::ReleaseHW(void)
{
	if(m_pD2Handle == nullptr) return;
	D2_EXEC(d2_freerenderbuffer(m_pD2Handle, m_pRenderBuffer));
	D2_EXEC(d2_closedevice(m_pD2Handle));
	m_pRenderBuffer = nullptr;
	m_pD2Handle = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::Flush(void)
{
	ReleaseHW();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::StartRender(void)
{
	D2_EXEC(d2_selectrenderbuffer(m_pD2Handle, m_pRenderBuffer));
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::CompleteRender(void)
{
	D2_EXEC(d2_flushframe(m_pD2Handle));
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::WaitForFinish(void)
{
	CompleteRender();
	EGSoftContext::SoftWaitForFinish();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::ExecuteRender(void)
{
	if(m_pD2Handle) {
		D2_EXEC(d2_executerenderbuffer(m_pD2Handle, m_pRenderBuffer, 0));
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::Blit(int32_t X, int32_t Y, EG_Color_t *pDestBuffer, const EGRect *pFillRect)
{
	uint32_t ModeSrc;

	ModeSrc = d2_mode_rgb565;
	int32_t DestWidth = pFillRect->GetWidth();
	int32_t DestHight = pFillRect->GetHeight();
	d2_selectrenderbuffer(m_pD2Handle, m_pRenderBuffer);
	// Generate render operations
	d2_framebuffer(m_pD2Handle, (uint16_t*)&m_pFrameBackground[0], EG_DISP_HORZ_RES, EG_DISP_HORZ_RES, MAX(pFillRect->GetY2() + 1, 2), GetDefColorFormat());
	d2_cliprect(m_pD2Handle, 0, 0, EG_DISP_HORZ_RES - 1, pFillRect->GetY2());
	d2_setblitsrc(m_pD2Handle, (void *)pDestBuffer, DestWidth, DestWidth, DestHight, ModeSrc);
	d2_blitcopy(m_pD2Handle, DestWidth, DestHight, 0, 0, D2_FIX4(DestWidth), D2_FIX4(DestHight), D2_FIX4(pFillRect->GetX1()), D2_FIX4(pFillRect->GetY1()), 0);
	d2_executerenderbuffer(m_pD2Handle, m_pRenderBuffer, 0);  // Execute render operations
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::Fill(EG_Color_t *pDestBuffer, const EGRect *pFillRect, int32_t DestWidth, EG_Color_t Color, EG_OPA_t OPA)
{
	InvalidateCache();
	StartRender();
	D2_EXEC(d2_framebuffer(m_pD2Handle, d1_maptovidmem(m_pD2Handle, pDestBuffer), MAX(DestWidth, 2), MAX(DestWidth, 2),
												 MAX(pFillRect->GetY2() + 1, 2), GetDefColorFormat()));
	D2_EXEC(d2_cliprect(m_pD2Handle, 0, 0, DestWidth - 1, pFillRect->GetY2()));
	D2_EXEC(d2_setalpha(m_pD2Handle, OPA > EG_OPA_MAX ? 0xFF : OPA));
	D2_EXEC(d2_setcolor(m_pD2Handle, 0, ColorToD2(Color)));
	D2_EXEC(d2_renderbox(m_pD2Handle, D2_FIX4(pFillRect->GetX1()), D2_FIX4(pFillRect->GetY1()), D2_FIX4(pFillRect->GetWidth()), D2_FIX4(pFillRect->GetHeight())));
	ExecuteRender();
}

//////////////////////////////////////////////////////////////////////////////////////

bool EGDave2Context::BlitConfig(const EGDrawImage *pImage, EG_ImageColorFormat_t DestCF, EG_ImageColorFormat_t SrceCF, bool AlphaEn, bool ColorKeyEn, bool BlendEn, bool ColorizeEn)
{
	if(BlendEn && pImage->m_BlendMode != EG_BLEND_MODE_NORMAL && pImage->m_BlendMode != EG_BLEND_MODE_ADDITIVE) return false;
	d2_s32 d2_src_cf = ColorFormatToD2(SrceCF);
	d2_s32 d2_dst_cf = ColorFormatToD2(DestCF);
	if(d2_src_cf < 0 || !ColorFormatFBValid(d2_dst_cf)) return false;
	m_SrceColorFormatVal = d2_src_cf;
	m_DestColorFormatVal = d2_dst_cf;
	m_DrawImage = *pImage;
	// Disable alpha if alpha channel does not exist
	m_AlphaEn = HasAlpha(m_SrceColorFormatVal) ? AlphaEn : 0;
	m_ColorKeyEn = ColorKeyEn;
	m_BlendEn = BlendEn;
	m_ColorizeEn = ColorizeEn | IsAlpha(m_SrceColorFormatVal);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::BlitInternal(const EGRect *pDestRect, const EG_Color_t *pSrceBuffer, const EGRect *pSrceRect, d2_u32 flags)
{
const EGRect *pImageRect = pSrceRect;
EGRect ImageRectScaled;
int32_t Width, Height, ImageWidth, ImageHeight;
d2_s32 Pitch;

	int BitsPerPixel = ColorFormatBPP(m_SrceColorFormatVal);
	D2_EXEC(d2_cliprect(m_pD2Handle, pDestRect->GetX1(), pDestRect->GetY1(), pDestRect->GetX2(), pDestRect->GetY2()));
	Pitch = Width = pSrceRect->GetWidth();
	Height = pSrceRect->GetHeight();
	if(m_DrawImage.m_Scale.IsScaled()) {
		ImageRectScaled.SetX1(pSrceRect->GetX1() + ((((int32_t)0 - m_DrawImage.m_Pivot.m_X) * m_DrawImage.m_Scale.m_X) >> 8) + m_DrawImage.m_Pivot.m_X);
		ImageRectScaled.SetX2(pSrceRect->GetX1() + ((((int32_t)Width - m_DrawImage.m_Pivot.m_X) * m_DrawImage.m_Scale.m_X) >> 8) + m_DrawImage.m_Pivot.m_X);
		ImageRectScaled.SetY1(pSrceRect->GetY1() + ((((int32_t)0 - m_DrawImage.m_Pivot.m_Y) * m_DrawImage.m_Scale.m_Y) >> 8) + m_DrawImage.m_Pivot.m_Y);
		ImageRectScaled.SetY2(pSrceRect->GetY1() + ((((int32_t)Height - m_DrawImage.m_Pivot.m_Y) * m_DrawImage.m_Scale.m_Y) >> 8) + m_DrawImage.m_Pivot.m_Y);
		pImageRect = &ImageRectScaled;
	}
	ImageWidth = pImageRect->GetWidth();
	ImageHeight = pImageRect->GetHeight();
	if(0 < BitsPerPixel && BitsPerPixel < 8) Pitch = (Width + (8 - BitsPerPixel)) & (~(8 - BitsPerPixel));
	if(m_DrawImage.m_Angle == 0) {
		D2_EXEC(d2_setblitsrc(m_pD2Handle, (void *)pSrceBuffer, Pitch, Width, Height, m_SrceColorFormatVal));
		D2_EXEC(d2_blitcopy(m_pD2Handle, Width, Height, 0, 0, D2_FIX4(ImageWidth), D2_FIX4(ImageHeight), D2_FIX4(pImageRect->GetX1()), D2_FIX4(pImageRect->GetY1()), flags));
	}
	else {
		int x, y, x1, y1, x2, y2, x3, y3, x4, y4, dxu, dxv, dyu, dyv, xx, xy, yx, yy;
		int PivoyScaledX, PivoyScaledY;
		int tex_offset = (flags & d2_bf_filter) ? -32767 : 0;
		d2_u8 amode, cmode = d2_to_copy;
		float angle = ((float)m_DrawImage.m_Angle / 10) * M_PI / 180;
		float cos_angle = cosf(angle);
		float sin_angle = sinf(angle);
		d2_u8 fillmode_backup;
		// setup texture params
		fillmode_backup = d2_getfillmode(m_pD2Handle);
		D2_EXEC(d2_setfillmode(m_pD2Handle, d2_fm_texture));
		D2_EXEC(d2_settexture(m_pD2Handle, (void *)pSrceBuffer, Pitch, Width, Height, m_SrceColorFormatVal));
		D2_EXEC(d2_settexturemode(m_pD2Handle, flags & (d2_bf_filter | d2_bf_wrap)));
		amode = flags & d2_bf_usealpha ? d2_to_copy : d2_to_one;
		cmode = flags & d2_bf_colorize2 ? d2_to_blend : d2_to_copy;
		D2_EXEC(d2_settextureoperation(m_pD2Handle, amode, cmode, cmode, cmode));
		if(flags & d2_bf_colorize2) {
			d2_color cl = d2_getcolor(m_pD2Handle, 0), ch = d2_getcolor(m_pD2Handle, 1);
			D2_EXEC(d2_settexopparam(m_pD2Handle, d2_cc_red, (uint8_t)(cl >> 16UL), (uint8_t)(ch >> 16UL)));
			D2_EXEC(d2_settexopparam(m_pD2Handle, d2_cc_green, (uint8_t)(cl >> 8UL), (uint8_t)(ch >> 8UL)));
			D2_EXEC(d2_settexopparam(m_pD2Handle, d2_cc_blue, (uint8_t)(cl >> 0UL), (uint8_t)(ch >> 0UL)));
		}
		x = D2_FIX4(pImageRect->GetX1());
		y = D2_FIX4(pImageRect->GetY1());
		// define quad points
		x1 = D2_FIX4(0);
		y1 = D2_FIX4(0);
		x2 = D2_FIX4(ImageWidth);
		y2 = D2_FIX4(0);
		x3 = D2_FIX4(ImageWidth);
		y3 = D2_FIX4(ImageHeight);
		x4 = D2_FIX4(0);
		y4 = D2_FIX4(ImageHeight);
		// rotate points for quad
		PivoyScaledX = (m_DrawImage.m_Pivot.m_X * m_DrawImage.m_Scale.m_X) >> 4;
		PivoyScaledY = (m_DrawImage.m_Pivot.m_Y * m_DrawImage.m_Scale.m_Y) >> 4;
		RotatePoint(&x1, &y1, cos_angle, sin_angle, PivoyScaledX, PivoyScaledY);
		RotatePoint(&x2, &y2, cos_angle, sin_angle, PivoyScaledX, PivoyScaledY);
		RotatePoint(&x3, &y3, cos_angle, sin_angle, PivoyScaledX, PivoyScaledY);
		RotatePoint(&x4, &y4, cos_angle, sin_angle, PivoyScaledX, PivoyScaledY);
		// compute texture increments
		xx = (int)(cos_angle * 65536.0f);
		xy = (int)(sin_angle * 65536.0f);
		yx = (int)(-sin_angle * 65536.0f);
		yy = (int)(cos_angle * 65536.0f);
		dxu = ((D2_FIX16(Width) / D2_FIX4(ImageWidth)) * xx) >> 12;
		dxv = ((D2_FIX16(Width) / D2_FIX4(ImageWidth)) * xy) >> 12;
		dyu = ((D2_FIX16(Height) / D2_FIX4(ImageHeight)) * yx) >> 12;
		dyv = ((D2_FIX16(Height) / D2_FIX4(ImageHeight)) * yy) >> 12;
		// map texture exactly to rotated quad, so texel center is always (0/0) top-left
		D2_EXEC(d2_settexelcenter(m_pD2Handle, 0, 0));
		D2_EXEC(d2_settexturemapping(m_pD2Handle, (d2_point)(x + x1), (d2_point)(y + y1), tex_offset, tex_offset, dxu, dxv, dyu, dyv));
		int minx = MAX(pDestRect->GetX1(), D2_INT4(x + MIN(x1, MIN(x2, MIN(x3, x4)))));
		int maxx = MIN(pDestRect->GetX2(), D2_INT4(x + MAX(x1, MAX(x2, MAX(x3, x4)))));
		int slice = (flags & d2_bf_filter) ? 6 : 8;
		// Perform render operation in slices to acheive better performance
		for(int posx = minx; posx < maxx; posx += slice) {
			D2_EXEC(d2_cliprect(m_pD2Handle, posx, pDestRect->GetY1(), MIN(posx + slice - 1, maxx), pDestRect->GetY2()));
			D2_EXEC(d2_renderquad(m_pD2Handle, (d2_point)(x + x1), (d2_point)(y + y1), (d2_point)(x + x2), (d2_point)(y + y2),
														(d2_point)(x + x3), (d2_point)(y + y3), (d2_point)(x + x4), (d2_point)(y + y4), 0));
		}
		D2_EXEC(d2_setfillmode(m_pD2Handle, fillmode_backup));
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::BlitRA(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrce, const EGRect *pSrceRect, EG_OPA_t OPA)
{
d2_u32 flags = 0;
const d2_color *clut = nullptr;
int clut_len = 0;

	InvalidateCache();
	clut_len = HandleIndexedColor(&pSrce, &clut, m_SrceColorFormatVal);
	StartRender();
	D2_EXEC(d2_framebuffer(m_pD2Handle, d1_maptovidmem(m_pD2Handle, pDest), MAX(DestStep, 2),
												 MAX(pDestRect->GetX2() + 1, 2), MAX(pDestRect->GetY2() + 1, 2), m_DestColorFormatVal));
	flags |= m_AlphaEn ? d2_bf_usealpha : 0;
	D2_EXEC(d2_setalpha(m_pD2Handle, OPA > EG_OPA_MAX ? EG_OPA_COVER : OPA));
	if(clut) {
		D2_EXEC(d2_writetexclut_direct(m_pD2Handle, clut, 0, clut_len));
	}
	flags |= m_ColorKeyEn ? d2_bf_usealpha : 0;
	flags |= (m_ColorizeEn || m_DrawImage.m_RecolorOPA != EG_OPA_TRANSP) ? d2_bf_colorize2 : 0;
	if(m_ColorizeEn) {
		D2_EXEC(d2_setcolor(m_pD2Handle, 0, ColorToD2(m_DrawImage.m_Recolor)));
		D2_EXEC(d2_setcolor(m_pD2Handle, 1, ColorToD2(m_DrawImage.m_Recolor)));
	}
	else if(m_DrawImage.m_RecolorOPA != EG_OPA_TRANSP) {
		d2_color cl = 0, ch = 0;
		GetRecolorConsts(&cl, &ch);
		D2_EXEC(d2_setcolor(m_pD2Handle, 0, cl));
		D2_EXEC(d2_setcolor(m_pD2Handle, 1, ch));
	}
	flags |= ((m_DrawImage.m_Angle || m_DrawImage.m_Scale.IsScaled()) && m_DrawImage.m_AntiAlias) ? d2_bf_filter : 0;

	if(m_BlendEn) {
		D2_EXEC(d2_setblendmode(m_pD2Handle, d2_bm_alpha, m_DrawImage.m_BlendMode != EG_BLEND_MODE_NORMAL ? d2_bm_one : d2_bm_one_minus_alpha));
		D2_EXEC(d2_setalphablendmode(m_pD2Handle, d2_bm_one, d2_bm_one_minus_alpha));
	}
	else {
		D2_EXEC(d2_setblendmode(m_pD2Handle, d2_bm_one, d2_bm_zero));
		D2_EXEC(d2_setalphablendmode(m_pD2Handle, d2_bm_one, d2_bm_zero));
	}
	BlitInternal(pDestRect, pSrce, pSrceRect, flags);
	ExecuteRender();
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::Blend(EGBlendBase *pBlend)
{
EGRect BlendRect;

  EGDave2Context *pDC = (EGDave2Context*)pBlend->m_pContext;
  if(!BlendRect.Intersect(pBlend->m_pRect, pDC->m_pClipRect)) return;  // Fully clipped, nothing to do
	bool Done = false;
	// Fill/Blend only non masked, normal blended
	if(pBlend->m_pMaskBuffer == nullptr && pBlend->m_BlendMode == EG_BLEND_MODE_NORMAL && BlendRect.GetSize() >= 100) {
		int32_t DestStep = pDC->m_pDrawRect->GetWidth();
		EG_Color_t *pDestBuffer = (EG_Color_t*)pDC->m_pDrawBuffer;
		const EG_Color_t *pSrceBuffer = pBlend->m_pSourceBuffer;
		if(pSrceBuffer) {
			EGSoftBlend::BlendBasic(pBlend);
			EGRect pSrceRect;
			pSrceRect.SetX1(BlendRect.GetX1() - (pBlend->m_pRect->GetX1() - pDC->m_pDrawRect->GetX1()));
			pSrceRect.SetY1(BlendRect.GetY1() - (pBlend->m_pRect->GetY1() - pDC->m_pDrawRect->GetY1()));
			pSrceRect.SetX2(pSrceRect.GetX1() + pBlend->m_pRect->GetWidth() - 1);
			pSrceRect.SetY2(pSrceRect.GetY1() + pBlend->m_pRect->GetHeight() - 1);
			pDC->BlitRA(pDestBuffer, &BlendRect, DestStep, pSrceBuffer, &pSrceRect, pBlend->m_OPA);
			Done = true;
		}
		else if(pBlend->m_OPA >= EG_OPA_MAX) {
			BlendRect.Move(-pDC->m_pDrawRect->GetX1(), -pDC->m_pDrawRect->GetY1());
			pDC->Fill(pDestBuffer, &BlendRect, DestStep, pBlend->m_Color, pBlend->m_OPA);
			Done = true;
		}
	}
	if(!Done) EGSoftBlend::BlendBasic(pBlend);
}

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat)
{
	/*TODO basic ARGB8888 image can be handles here*/
	EGSoftContext::DrawImageDecoded(pDrawImage, pRect, pSourceBuffer, ColorFormat);
}

//////////////////////////////////////////////////////////////////////////////////////

#ifdef LOG_ERRORS

void EGDave2Context::EG_PortGpuLogError(d2_s32 Status, const char *pFunc, int Line)
{
	if(status) {
		m_LogErrorList[m_ErrorListIndex].Error = status;
		m_LogErrorList[m_ErrorListIndex].func = pFunc;
		m_LogErrorList[m_ErrorListIndex].line = Line;
		EG_LOG_ERROR("%s\r\n", d2_geterrorstring(m_pD2Handle));
		EG_LOG_ERROR("%d:\t%d - %s : %d\r\n", m_ErrorCount,
								 m_LogErrorList[m_ErrorListIndex].Error,
								 m_LogErrorList[m_ErrorListIndex].pFunc,
								 m_LogErrorList[m_ErrorListIndex].Line);
		m_ErrorCount++;
		m_ErrorListIndex++;
		if(m_ErrorListIndex >= ERROR_LIST_SIZE) {
			m_ErrorListIndex = 0;
		}
	}
}

#endif

#endif
