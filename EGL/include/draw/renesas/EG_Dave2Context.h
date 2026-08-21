/**
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#include "misc/EG_Color.h"
#include "hal/EG_HALDisplay.h"
#include "draw/sw/EG_SoftContext.h"
#include "draw/sw/EG_DrawSoftBlend.h"
#include "core/EG_Refresh.h"     

#if EG_USE_GPU_RA6M3_G2D

#include "hal_data.h"
#include "../../dave2d/inc/dave_driver.h"


#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define M_PI    3.1415926

const uint32_t FrameBufferSize = EG_DISP_HORZ_RES * EG_DISP_VERT_RES * BYTES_PER_PIXEL;

//////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	d2_s32      Error;
	const char *pFunc;
	int         Line;
} LogErrorEntry_t;

class EGSoftBlend;

//////////////////////////////////////////////////////////////////////////////////////

class EGDave2Context : public EGSoftContext
{
public:
                    EGDave2Context();
  virtual           ~EGDave2Context();
  void              InitialiseContext(void);
  static void       InitGPU(void);

  static void       Flush(void);
  static void       Blend(EGBlendBase *pBlend);
  static void       DrawCharacter(const EGDrawLabel *pDrawLabel, const EGPoint *pPosition, uint32_t Char);
  static void       DrawImageDecoded(const EGDrawImage *pDrawImage, const EGRect *pRect, const uint8_t *pSourceBuffer, EG_ImageColorFormat_t ColorFormat);
  static void       WaitForFinish(void);

private:
  void              Blit(int32_t X, int32_t Y, EG_Color_t *pDestBuffer, const EGRect *pFillRect);
  void              BlitInternal(const EGRect *pDestRect, const EG_Color_t *pSrceBuffer, const EGRect *pSrceRect, d2_u32 flags);
  void              BlitRA(EG_Color_t *pDest, const EGRect *pDestRect, int32_t DestStep, const EG_Color_t *pSrce, const EGRect *pSrceRect, EG_OPA_t OPA);
  void              Fill(EG_Color_t *pDestBuffer, const EGRect *pFillRect, int32_t DestWidth, EG_Color_t Color, EG_OPA_t OPA);
  void              DrawCharNormal(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, EG_FontGlyphProps_t *pGlyph, const uint8_t *pMap);
  d2_s32            ColorFormatToD2(EG_ImageColorFormat_t ColorFormat);
  bool              ColorFormatFBValid(d2_s32 ColorFormat);
  bool              HasAlpha(d2_s32 ColorFormat);
  bool              IsAlpha(d2_s32 ColorFormat);
  d2_color          ColorToD2(EG_Color_t Color);
  void              GetRecolorConsts(d2_color *pColorLow, d2_color *pColorHigh);
  int               HandleIndexedColor(const EG_Color_t **ppSrce, const d2_color **ppColorLUT, d2_s32 ColorFormat);
  int               ColorFormatBPP(d2_s32 cf);
  d2_s32            GetDefColorFormat(void);
  void              ClearBlitConfig(void);
  void              RotatePoint(int *x, int *y, float cos_angle, float sin_angle, int pivot_x, int pivot_y);
  void              StartRender(void);
  void              ExecuteRender(void);
  bool              BlitConfig(const EGDrawImage *pImage, EG_ImageColorFormat_t DestCF, EG_ImageColorFormat_t SrceCF, bool AlphaEn, bool ColorKeyRn, bool BlendEn, bool ColorizeEn);

  static void       ReleaseHW(void);
  static void       CompleteRender(void);
  static void       InvalidateCache(void);

  static d2_device       *m_pD2Handle;
  static d2_renderbuffer *m_pRenderBuffer;
  static d2_s32           m_SrceColorFormatVal;
  static d2_s32           m_DestColorFormatVal;
  static EGDrawImage      m_DrawImage;
  static bool             m_ColorKeyEn;
  static bool             m_AlphaEn;
  static bool             m_BlendEn;
  static bool             m_ColorizeEn;

  static uint8_t          *m_pFrameBackground[2] __attribute__((section(".framebuffer"), aligned(64), used));

	static EG_OPA_t   m_OPATable[256];
	static EG_OPA_t   m_PreveviousOPA;
	static uint32_t   m_PreveviousBPP;

#ifdef LOG_ERRORS
  void              LogError(d2_s32 Status, const char *pFunc, int Line);
  LogErrorEntry_t   m_LogErrorList[ERROR_LIST_SIZE];
  int               m_ErrorListIndex;
  int               m_ErrorCount;
#endif
};

//////////////////////////////////////////////////////////////////////////////////////

inline void EGDave2Context::InvalidateCache()
{
  EGDisplay *pDisp = GetRefreshingDisplay();
	if(pDisp->m_pDriver->CleanDcacheCB) pDisp->m_pDriver->CleanDcacheCB(pDisp->m_pDriver);
}

#endif
