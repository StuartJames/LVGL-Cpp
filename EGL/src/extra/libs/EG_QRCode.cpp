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

#include "extra/libs/EG_QRCode.h"

#if EG_USE_QRCODE

#include "extra/libs/QRCodeGen.h"

///////////////////////////////////////////////////////////////////////////////////////

#define QRCODE_CLASS &c_QRCodeClass

///////////////////////////////////////////////////////////////////////////////////////

const EG_ClassType_t c_QRCodeClass = {
	.pBaseClassType = &c_CanvasClass,
	.pEventCB = nullptr,
	.WidthDef = 0,
	.HeightDef = 0,
	.IsEditable = 0,
	.GroupDef = 0,
#if EG_USE_EXT_DATA
	.pExtData = nullptr
#endif
};


///////////////////////////////////////////////////////////////////////////////////////

EGQRCode::EGQRCode(EGObject *pParent, int32_t Size, EG_Color_t DarkColor, EG_Color_t LightColor, 
  const EG_ClassType_t *pClassCnfg /*= &c_LedClass*/) : EGCanvas()
{
	m_Size = Size;
	m_DarkColor = DarkColor;
	m_LightColor = LightColor;
  Attach(this, pParent, pClassCnfg);
	Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////

void EGQRCode::Configure(void)
{
	uint32_t BufferSize = EG_CANVAS_BUF_SIZE_INDEXED_1BIT(m_Size, m_Size);
	uint8_t *pBuffer = (uint8_t*)EG_AllocMem(BufferSize);
	EG_ASSERT_MALLOC(pBuffer);
	if(pBuffer == nullptr) return;
	SetBuffer(pBuffer, m_Size, m_Size, EG_COLOR_FORMAT_INDEXED_1BIT);
	SetPalette(0, m_DarkColor);
	SetPalette(1, m_LightColor);
}

///////////////////////////////////////////////////////////////////////////////////////

EGQRCode::~EGQRCode()
{
	EGImageBuffer *pImage = GetImage();
	InvalidateImageCacheSource(pImage);
	EG_FreeMem((void *)pImage->m_pData);
	pImage->m_pData = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////

EG_Result_t EGQRCode::Update(const void *pData, uint32_t Length)
{
EG_Color_t Color;
int x, y;

	Color.full = 1;
	FillBackground(Color, EG_OPA_COVER);
	if(Length > QRCODEGEN_BUFFER_LEN_MAX) return EG_RES_INVALID;
	EGImageBuffer *pImage = GetImage();
	int32_t QR_Version = QRCodeGenGetMinFitVersion(QRCODEGEN_ECC_MEDIUM, Length);
	if(QR_Version <= 0) return EG_RES_INVALID;
	int32_t QR_Size = QRCodeGenVersion2size(QR_Version);
	if(QR_Size <= 0) return EG_RES_INVALID;
	int32_t Scale = pImage->m_Header.Width / QR_Size;
	if(Scale <= 0) return EG_RES_INVALID;
	int32_t Remain = pImage->m_Header.Width % QR_Size;
	uint32_t VersionExtend = Remain / (Scale << 2);	// The qr version is incremented by four point
	if(VersionExtend && QR_Version < QRCODEGEN_VERSION_MAX) {
		QR_Version = QR_Version + VersionExtend > QRCODEGEN_VERSION_MAX ? QRCODEGEN_VERSION_MAX :	QR_Version + VersionExtend;
	}
	uint8_t *pQRZero = (uint8_t*)EG_AllocMem(QRCODEGEN_BUFFER_LEN_FOR_VERSION(QR_Version));
	EG_ASSERT_MALLOC(pQRZero);
	uint8_t *pDataTemp = (uint8_t*)EG_AllocMem(QRCODEGEN_BUFFER_LEN_FOR_VERSION(QR_Version));
	EG_ASSERT_MALLOC(pDataTemp);
	EG_CopyMem(pDataTemp, pData, Length);
	bool Res = QRCodeGenEncodeBinary(pDataTemp, Length, pQRZero, QRCODEGEN_ECC_MEDIUM, QR_Version, QR_Version, QRCODEGEN_MASK_AUTO, true);
	if(!Res) {
		EG_FreeMem(pQRZero);
		EG_FreeMem(pDataTemp);
		return EG_RES_INVALID;
	}
	int32_t Width = pImage->m_Header.Width;
	QR_Size = QRCodeGenGetSize(pQRZero);
	Scale = Width / QR_Size;
	int Scaled = QR_Size * Scale;
	int Margin = (Width - Scaled) / 2;
	uint8_t *buf_u8 = (uint8_t *)pImage->m_pData + 8; //+8 skip the palette
	/* Copy the qr code canvas:
     * A simple `SetPixel` would work but it's slow for so many pixels.
     * So buffer 1 byte (8 px) from the qr code and set it in the canvas image */
	uint32_t row_byte_cnt = (pImage->m_Header.Width + 7) >> 3;
	for(y = Margin; y < Scaled + Margin; y += Scale) {
		uint8_t b = 0;
		uint8_t p = 0;
		bool aligned = false;
		for(x = Margin; x < Scaled + Margin; x++) {
			bool a = QRCodeGenGetModule(pQRZero, (x - Margin) / Scale, (y - Margin) / Scale);
			if(aligned == false && (x & 0x7) == 0) aligned = true;
			if(aligned == false) {
				Color.full = a ? 0 : 1;
				SetPixelColor(x, y, Color);
			}
			else {
				if(!a) b |= (1 << (7 - p));
				p++;
				if(p == 8) {
					uint32_t px = row_byte_cnt * y + (x >> 3);
					buf_u8[px] = b;
					b = 0;
					p = 0;
				}
			}
		}
		if(p) {      // Process the last byte of the row
			b |= (1 << (8 - p)) - 1;			// Make the rest of the bits white
			uint32_t px = row_byte_cnt * y + (x >> 3);
			buf_u8[px] = b;
		}
		int s;		// The Qr is probably Scaled so simply to the repeated rows
		const uint8_t *row_ori = buf_u8 + row_byte_cnt * y;
		for(s = 1; s < Scale; s++) {
			EG_CopyMem((uint8_t *)buf_u8 + row_byte_cnt * (y + s), row_ori, row_byte_cnt);
		}
	}
	EG_FreeMem(pQRZero);
	EG_FreeMem(pDataTemp);
	return EG_RES_OK;
}

#endif
