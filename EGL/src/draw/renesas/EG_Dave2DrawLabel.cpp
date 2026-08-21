/**
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "draw/EG_DrawLabel.h"
#include "misc/EG_Assert.h"
#include "misc/EG_Memory.h"
#include "core/EG_Refresh.h"
#include "draw/renesas/EG_Dave2Context.h"

#if EG_USE_GPU_RA6M3_G2D

#include EG_GPU_RA6M3_G2D_INCLUDE

//////////////////////////////////////////////////////////////////////////////////////

extern const uint8_t EG_BPP1_OPA_Table[2];
extern const uint8_t EG_BPP2_OPA_Table[4];
extern const uint8_t EG_BPP4_OPA_Table[16];
extern const uint8_t EG_BPP8_OPA_Table[256];

//////////////////////////////////////////////////////////////////////////////////////

void EGDave2Context::DrawCharacter(const EGDrawLabel *pDrawLabel, const EGPoint *pPosition, uint32_t Char)
{
  EGDave2Context *pDC = (EGDave2Context*)pDrawLabel->m_pContext;
	const EG_Font_t *pFont = pDrawLabel->m_pFont;
	EG_OPA_t OPA = pDrawLabel->m_OPA;
	if(OPA < EG_OPA_MIN) return;
	if(OPA > EG_OPA_MAX) OPA = EG_OPA_COVER;
	if(pFont == nullptr) {
		EG_LOG_WARN("DrawCharacter: font is null");
		return;
	}
	EG_FontGlyphProps_t Glyph;
	bool g_ret = EG_FontGetGlyphProps(pFont, &Glyph, Char, '\0');
	if(g_ret == false) {
		// Add warning if the pDrawLabel is not found but do not print warning for non printable ASCII chars (e.Glyph. '\n')
		if(Char >= 0x20 &&
			 Char != 0xf8ff &&  // EG_SYMBOL_DUMMY
			 Char != 0x200c) {  // ZERO WIDTH NON-JOINER
			EG_LOG_WARN("DrawCharacter: glyph pDrawLabel. not found for U+%X", Char);
		}
		return;
	}
	// Don't draw anything if the character is empty. E.Glyph. space
	if((Glyph.BoxHeight == 0) || (Glyph.BoxWidth == 0)) return;
	EGPoint GlyphPos;
	GlyphPos.m_X = pPosition->m_X + Glyph.OffsetX;
	GlyphPos.m_Y = pPosition->m_Y + (pDrawLabel->m_pFont->LineHeight - pDrawLabel->m_pFont->BaseLine) - Glyph.BoxHeight - Glyph.OffsetY;
	// If the Char is completely out of mask don't draw it
	if(GlyphPos.m_X + Glyph.BoxWidth < pDC->m_pClipRect->GetX1() || GlyphPos.m_X > pDC->m_pClipRect->GetX2() ||
		 GlyphPos.m_Y + Glyph.BoxHeight < pDC->m_pClipRect->GetY1() || GlyphPos.m_Y > pDC->m_pClipRect->GetY2()) {
		return;
	}
	const uint8_t *pMap = EG_FontGetGlyphBitmap(pFont, Char);
	if(pMap == nullptr) {
		EG_LOG_WARN("DrawCharacter: character's bitmap not found");
		return;
	}
	if(pFont->SubPixel) {
#if EG_DRAW_COMPLEX && EG_USE_FONT_SUBPX
    pDC->DrawSubpixel(pDrawLabel, GlyphPos, &Glyph, pMap);
#else
		EG_LOG_WARN("Can't draw sub-pixel rendered Char because EG_USE_FONT_SUBPX == 0 in EG_Config.h");
#endif
	}
	else {
		pDC->DrawCharNormal(pDrawLabel, &GlyphPos, &Glyph, pMap);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EG_ATTRIBUTE_FAST_MEM EGDave2Context::DrawCharNormal(const EGDrawLabel *pDrawLabel, const EGPoint *pPos, EG_FontGlyphProps_t *pGlyph, const uint8_t *pMap)
{
const uint8_t *pBitsPerPixelOPATable;
uint32_t BitMaskInit;
uint32_t BitMask;
uint32_t BitsPerPixel = pGlyph->BitsPerPixel;
EG_OPA_t OPA = pDrawLabel->m_OPA;
uint32_t Shades;

  if(BitsPerPixel == 3) BitsPerPixel = 4;
#if EG_USE_IMGFONT
	if(BitsPerPixel == EG_IMGFONT_BPP) {  //is imgfont
		EGRect FillRect;
		FillRect.GetX1() = pPos->m_X;
		FillRect.GetY1() = pPos->m_Y;
		FillRect.GetX2() = pPos->m_X + pGlyph->BoxWidth - 1;
		FillRect.GetY2() = pPos->m_Y + pGlyph->BoxHeight - 1;
		EGDrawImage DrawImage;
		DrawImage.m_Angle = 0;
		DrawImage.m_Scale.Set(EG_IMAGE_SCALE_NONE, EG_IMAGE_SCALE_NONE);
		DrawImage.m_OPA = pDrawLabel->m_OPA;
		DrawImage.m_BlendMode = pDrawLabel->m_BlendMode;
		DrawImage.Draw(DrawImage->m_pContext, &FillRect, pMap);
		return;
	}
#endif
	switch(BitsPerPixel) {
		case 1:
			pBitsPerPixelOPATable = EG_BPP1_OPA_Table;
			BitMaskInit = 0x80;
			Shades = 2;
			break;
		case 2:
			pBitsPerPixelOPATable = EG_BPP2_OPA_Table;
			BitMaskInit = 0xC0;
			Shades = 4;
			break;
		case 4:
			pBitsPerPixelOPATable = EG_BPP4_OPA_Table;
			BitMaskInit = 0xF0;
			Shades = 16;
			break;
		case 8:
			pBitsPerPixelOPATable = EG_BPP8_OPA_Table;
			BitMaskInit = 0xFF;
			Shades = 256;
			break;  // No opa table, pixel value will be used directly
		default:
			EG_LOG_WARN("EG_DrawChar: invalid bpp");
			return;
	}
	if(OPA < EG_OPA_MAX) {
		if(m_PreveviousOPA != OPA || m_PreveviousBPP != BitsPerPixel) {
			uint32_t i;
			for(i = 0; i < Shades; i++) {
				m_OPATable[i] = pBitsPerPixelOPATable[i] == EG_OPA_COVER ? OPA : ((pBitsPerPixelOPATable[i] * OPA) >> 8);
			}
		}
		pBitsPerPixelOPATable = m_OPATable;
		m_PreveviousOPA = OPA;
		m_PreveviousBPP = BitsPerPixel;
	}
	int32_t Column, Row;
	int32_t BoxWidth = pGlyph->BoxWidth;
	int32_t BoxHeight = pGlyph->BoxHeight;
	int32_t WidthBits = BoxWidth * BitsPerPixel;  // Letter width in bits
	// Calculate the Column/Row start/end on the map
	int32_t StartColumn = pPos->m_X >= m_pClipRect->GetX1() ? 0 : m_pClipRect->GetX1() - pPos->m_X;
	int32_t EndColumn = pPos->m_X + BoxWidth <= m_pClipRect->GetX2() ? BoxWidth : m_pClipRect->GetX2() - pPos->m_X + 1;
	int32_t StartRow = pPos->m_Y >= m_pClipRect->GetY1() ? 0 : m_pClipRect->GetY1() - pPos->m_Y;
	int32_t EndRow = pPos->m_Y + BoxHeight <= m_pClipRect->GetY2() ? BoxHeight : m_pClipRect->GetY2() - pPos->m_Y + 1;
	// Move on the map too
	uint32_t BitOffset = (StartRow * WidthBits) + (StartColumn * BitsPerPixel);
	pMap += BitOffset >> 3;
	uint8_t CharIndex;
	uint32_t ColumnBits;
	ColumnBits = BitOffset & 0x7;  // "& 0x7" equals to "% 8" just faster
	EGSoftBlend BlendObj(this);
	BlendObj.m_Color = pDrawLabel->m_Color;
	BlendObj.m_OPA = EG_OPA_COVER;
	BlendObj.m_BlendMode = pDrawLabel->m_BlendMode;
	int32_t HorizontalRes = GetRefreshingDisplay()->GetHorizontalRes();
	uint32_t MaskBufferSize = BoxWidth * BoxHeight > HorizontalRes ? HorizontalRes : BoxWidth * BoxHeight;
	EG_OPA_t *MaskBuffer = (EG_OPA_t *)EG_GetBufferMem(MaskBufferSize);
	BlendObj.m_pMaskBuffer = MaskBuffer;
	int32_t MaskIndex = 0;
	EGRect FillRect(StartColumn + pPos->m_X, StartRow + pPos->m_Y, EndColumn + pPos->m_X - 1, StartRow + pPos->m_Y);
#if EG_DRAW_COMPLEX
  int32_t FillWidth = FillRect.GetWidth();
  EGRect MaskRect(FillRect);
  MaskRect.SetY2(MaskRect.GetY1() + EndRow);
  bool MaskAny = HasAnyDrawMask(&MaskRect);
#endif
  BlendObj.m_pRect = &FillRect;
  BlendObj.m_pMaskRect = &FillRect;
  uint32_t col_bit_max = 8 - BitsPerPixel;
	uint32_t col_bit_row_ofs = (BoxWidth + StartColumn - EndColumn) * BitsPerPixel;
	for(Row = StartRow; Row < EndRow; Row++) {
#if EG_DRAW_COMPLEX
		int32_t pMaskStart = MaskIndex;
#endif
		BitMask = BitMaskInit >> ColumnBits;
		for(Column = StartColumn; Column < EndColumn; Column++) {
			CharIndex = (*pMap & BitMask) >> (col_bit_max - ColumnBits);			// Load the pixel's opacity into the mask
			if(CharIndex)	MaskBuffer[MaskIndex] = pBitsPerPixelOPATable[CharIndex];
			else MaskBuffer[MaskIndex] = 0;
			if(ColumnBits < col_bit_max) {        // Go to the next column
				ColumnBits += BitsPerPixel;
				BitMask = BitMask >> BitsPerPixel;
			}
			else {
				ColumnBits = 0;
				BitMask = BitMaskInit;
				pMap++;
			}
			MaskIndex++;			// Next mask byte
		}
#if EG_DRAW_COMPLEX
		if(MaskAny) {  // Apply masks if any
			BlendObj.m_MaskResult = DrawMaskApply(MaskBuffer + pMaskStart, FillRect.GetX1(), FillRect.GetY2(), FillWidth);
			if(BlendObj.m_MaskResult == EG_DRAW_MASK_RESULT_TRANSP) {
				EG_ZeroMem(MaskBuffer + pMaskStart, FillWidth);
			}
		}
#endif
    if((uint32_t)MaskIndex + (EndColumn - StartColumn) < MaskBufferSize) {
      FillRect.IncY2(1);
    }
    else {
			BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
			BlendObj.DoBlend();
			FillRect.SetY1(FillRect.GetY2() + 1);
			FillRect.SetY2(FillRect.GetY1());
			MaskIndex = 0;
		}
		ColumnBits += col_bit_row_ofs;
		pMap += (ColumnBits >> 3);
		ColumnBits = ColumnBits & 0x7;
	}
	if(FillRect.GetY1() != FillRect.GetY2()) {
		FillRect.DecY2(1);   // decrement
		BlendObj.m_MaskResult = EG_DRAW_MASK_RESULT_CHANGED;
		BlendObj.DoBlend();
		MaskIndex = 0;
	}
	EG_ReleaseBufferMem(MaskBuffer);
}

#endif
