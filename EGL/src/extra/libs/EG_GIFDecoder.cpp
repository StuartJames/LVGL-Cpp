#include "extra/libs/EG_GIFDecoder.h"
#include "misc/EG_Log.h"
#include "misc/EG_Memory.h"
#include "misc/EG_Color.h"

#if EG_USE_GIF

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////

// Source - https://stackoverflow.com/a/58532788
// Posted by Gabriel Staples, modified by community. See post 'Timeline' for change history
// Retrieved 2026-02-28, License - CC BY-SA 4.0

#define MAX(a,b)           \
({                         \
  __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a > _b ? _a : _b;       \
})

#define MIN(a,b)           \
({                         \
  __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b;       \
})

///////////////////////////////////////////////////////////////////////////////

EGDecoderGIF::EGDecoderGIF(void) :
  m_pVarData(nullptr),
  m_IsFile(false),
  m_VarReadPos(0),
  m_AnimateStart(0),
  m_Width(0),
  m_Height(0),
  m_Depth(0),
  m_LoopCount(0),
  m_pPalette(nullptr),
  m_FrameX(0),
  m_FrameY(0),
  m_FrameWidth(0),
  m_FrameHeight(0),
  m_BackgroundIndex(0),
  m_pFrame(nullptr),
  m_pCanvas(nullptr),
  PlainText(nullptr),
  Comment(nullptr),
  Application(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////

EGDecoderGIF::~EGDecoderGIF(void)
{
	FileClose();
  if(m_pCanvas) EG_FreeMem(m_pCanvas);
}

///////////////////////////////////////////////////////////////////////////////

EGDecoderGIF* EGDecoderGIF::OpenFile(const char *pFileName)
{
	EGDecoderGIF *pGIF = new EGDecoderGIF;
	bool res = pGIF->FileOpen(pFileName, true);
	if(!res) return nullptr;
	return pGIF->Initialise();
}

///////////////////////////////////////////////////////////////////////////////

EGDecoderGIF* EGDecoderGIF::OpenData(const void *pData)
{
	EGDecoderGIF *pGIF = new EGDecoderGIF;
	bool res = pGIF->FileOpen(pData, false);
	if(!res) return nullptr;
	return pGIF->Initialise();
}

///////////////////////////////////////////////////////////////////////////////

EGDecoderGIF* EGDecoderGIF::Initialise(void)
{
uint8_t SigVer[3];
uint16_t Width, Height, Depth;
uint8_t FDSZ, BackIndex, AspectRatio;
uint8_t *pBackColor;
int GCTSize, i;

	FileRead(SigVer, 3);	  // Header 
	if(memcmp(SigVer, "GIF", 3) != 0) {
		EG_LOG_WARN("invalid signature\n");
		goto fail;
	}
	FileRead(SigVer, 3);	  // Version 
	if(memcmp(SigVer, "89a", 3) != 0) {
		EG_LOG_WARN("invalid version\n");
		goto fail;
	}
	Width = ReadNumber();	        // Width x Height 
	Height = ReadNumber();
	FileRead(&FDSZ, 1);	    // FDSZ 
	if(!(FDSZ & 0x80)) {	              // Presence of GCT 
		EG_LOG_WARN("no global pColor table\n");
		goto fail;
	}
	Depth = ((FDSZ >> 4) & 7) + 1;	    // Color Space's Depth 
	GCTSize = 1 << ((FDSZ & 0x07) + 1);	// GCT Size. Ignore Sort Flag. 
	FileRead(&BackIndex, 1);	          // Background Color Index 
	FileRead(&AspectRatio, 1);	              // AspectRatio Ratio 
#if EG_COLOR_DEPTH == 32              // Create gd_GIF Structure. 
	m_ImageSize = 5 * Width * Height;
#elif EG_COLOR_DEPTH == 16
	m_ImageSize = 4 * Width * Height;
#elif EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
  m_ImageSize = 3 * Width * Height;
#endif
	m_pCanvas = (uint8_t*)EG_AllocMem(m_ImageSize);
	if(!m_pCanvas) goto fail;
	m_Width = Width;
	m_Height = Height;
	m_Depth = Depth;
	m_GlobalColorTable.Size = GCTSize;
	FileRead(m_GlobalColorTable.Colors, 3 * m_GlobalColorTable.Size);	// Read GCT 
	m_pPalette = &m_GlobalColorTable;
	m_BackgroundIndex = BackIndex;
#if EG_COLOR_DEPTH == 32
	m_pFrame = &m_pCanvas[4 * Width * Height];
#elif EG_COLOR_DEPTH == 16
	m_pFrame = &m_pCanvas[3 * Width * Height];
#elif EG_COLOR_DEPTH == 8 || EG_COLOR_DEPTH == 1
	m_pFrame = &m_pCanvas[2 * Width * Height];
#endif
	if(m_BackgroundIndex) {
		memset(m_pFrame, m_BackgroundIndex, m_Width * m_Height);
	}
	pBackColor = &m_pPalette->Colors[m_BackgroundIndex * 3];
	for(i = 0; i < m_Width * m_Height; i++) {
#if EG_COLOR_DEPTH == 32
		m_pCanvas[i * 4 + 0] = *(pBackColor + 2);
		m_pCanvas[i * 4 + 1] = *(pBackColor + 1);
		m_pCanvas[i * 4 + 2] = *(pBackColor + 0);
		m_pCanvas[i * 4 + 3] = 0xff;
#elif EG_COLOR_DEPTH == 16
		EG_Color_t c = EG_MixColor(*(pBackColor + 0), *(pBackColor + 1), *(pBackColor + 2));
		m_pCanvas[i * 3 + 0] = c.full & 0xff;
		m_pCanvas[i * 3 + 1] = (c.full >> 8) & 0xff;
		m_pCanvas[i * 3 + 2] = 0xff;
#elif EG_COLOR_DEPTH == 8
		EG_Color_t c = EG_MixColor(*(pBackColor + 0), *(pBackColor + 1), *(pBackColor + 2));
		m_pCanvas[i * 2 + 0] = c.full;
		m_pCanvas[i * 2 + 1] = 0xff;
#elif EG_COLOR_DEPTH == 1
		EG_Color_t c = EG_MixColor(*(pBackColor + 0), *(pBackColor + 1), *(pBackColor + 2));
		m_pCanvas[i * 2 + 0] = c.ch.red > 128 ? 1 : 0;
		m_pCanvas[i * 2 + 1] = 0xff;
#endif
	}
	m_AnimateStart = FileSeek(0, EG_FS_SEEK_CUR);
	m_LoopCount = -1;
  return this;

fail:
	FileClose();
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::DiscardSubblocks(void)
{
uint8_t Size;

	do {
		FileRead(&Size, 1);
		FileSeek(Size, EG_FS_SEEK_CUR);
	} while(Size);
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::ReadPlainTextExt(void)
{
	if(PlainText){
		uint16_t tx, ty, tw, th;
		uint8_t cw, ch, fg, bg;
		size_t SubBlock;
		FileSeek(1, EG_FS_SEEK_CUR); /* block size = 12 */
		tx = ReadNumber();
		ty = ReadNumber();
		tw = ReadNumber();
		th = ReadNumber();
		FileRead(&cw, 1);
		FileRead(&ch, 1);
		FileRead(&fg, 1);
		FileRead(&bg, 1);
		SubBlock = FileSeek(0, EG_FS_SEEK_CUR);
		PlainText(this, tx, ty, tw, th, cw, ch, fg, bg);
		FileSeek(SubBlock, EG_FS_SEEK_SET);
	}
	else {
		FileSeek(13, EG_FS_SEEK_CUR);		// Discard plain text metadata. 
	}
	DiscardSubblocks();	// Discard plain text sub-blocks. 
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::ReadGraphicControlExt(void)
{
uint8_t ControlBlock;

	FileSeek(1, EG_FS_SEEK_CUR);	// Discard block size (always 0x04). 
	FileRead(&ControlBlock, 1);
	m_GlobalControlExt.Disposal = (ControlBlock >> 2) & 3;
	m_GlobalControlExt.Input = ControlBlock & 2;
	m_GlobalControlExt.Transparency = ControlBlock & 1;
	m_GlobalControlExt.Delay = ReadNumber();
	FileRead(&m_GlobalControlExt.TransIndex, 1);
	FileSeek(1, EG_FS_SEEK_CUR);	// Skip block terminator. 
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::ReadCommentExt(void)
{
	if(Comment) {
		size_t SubBlock = FileSeek(0, EG_FS_SEEK_CUR);
		Comment(this);
		FileSeek(SubBlock, EG_FS_SEEK_SET);
	}
	DiscardSubblocks();	// Discard comment sub-blocks. 
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::ReadApplicationExt(void)
{
char AppID[8];
char AppAuthCode[3];
uint16_t LoopCount;

	FileSeek(1, EG_FS_SEEK_CUR);	// Discard block size (always 0x0B). 
	FileRead(AppID, 8);	        // Application Identifier. 
	FileRead(AppAuthCode, 3);	  // Application Authentication Code. 
	if(!strncmp(AppID, "NETSCAPE", sizeof(AppID))) {
		FileSeek(2, EG_FS_SEEK_CUR);		// Discard block size (0x03) and constant byte (0x01). 
		LoopCount = ReadNumber();
		if(m_LoopCount < 0) {
			if(LoopCount == 0) m_LoopCount = 0;
			else m_LoopCount = LoopCount + 1;
		}
		FileSeek(1, EG_FS_SEEK_CUR);		// Skip block terminator. 
	}
	else{
    if(Application) {
      size_t SubBlock = FileSeek(0, EG_FS_SEEK_CUR);
      Application(this, AppID, AppAuthCode);
      FileSeek(SubBlock, EG_FS_SEEK_SET);
    }
	  DiscardSubblocks();	// Discard comment sub-blocks. 
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::ReadExt(void)
{
uint8_t Label;

	FileRead(&Label, 1);
	switch(Label) {
		case 0x01:
			ReadPlainTextExt();
			break;
		case 0xF9:
			ReadGraphicControlExt();
			break;
		case 0xFE:
			ReadCommentExt();
			break;
		case 0xFF:
			ReadApplicationExt();
			break;
		default:
			EG_LOG_WARN("unknown extension: %02X\n", Label);
	}
}

///////////////////////////////////////////////////////////////////////////////

GIFTable_t* EGDecoderGIF::NewTable(int KeySize)
{
int Key;
int InitBulk = MAX(1 << (KeySize + 1), 0x100);

	GIFTable_t *pTable = (GIFTable_t*)EG_AllocMem(sizeof(*pTable) + sizeof(GIFEntry_t) * InitBulk);
	if(pTable) {
		pTable->Bulk = InitBulk;
		pTable->Count = (1 << KeySize) + 2;
		pTable->pEntries = (GIFEntry_t *)&pTable[1];
		for(Key = 0; Key < (1 << KeySize); Key++)	pTable->pEntries[Key] = (GIFEntry_t){1, 0xFFF, (uint8_t)Key};
	}
	return pTable;
}

///////////////////////////////////////////////////////////////////////////////

/* Add table Entry. Return value:
 *  0 on success
 *  +1 if Key size must be incremented after this addition
 *  -1 if could not realloc table */
int EGDecoderGIF::AddEntry(GIFTable_t **ppTable, uint16_t Length, uint16_t Prefix, uint8_t Suffix)
{
GIFTable_t *pTable = *ppTable;

	if(pTable->Count == pTable->Bulk) {
		pTable->Bulk *= 2;
		pTable = (GIFTable_t*)EG_ReallocMem(pTable, sizeof(*pTable) + sizeof(GIFEntry_t) * pTable->Bulk);
		if(!pTable) return -1;
		pTable->pEntries = (GIFEntry_t *)&pTable[1];
		*ppTable = pTable;
	}
	pTable->pEntries[pTable->Count] = GIFEntry_t{Length, Prefix, Suffix};
	pTable->Count++;
	if((pTable->Count & (pTable->Count - 1)) == 0) return 1;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t EGDecoderGIF::GetKey(int KeySize, uint8_t *pSubLength, uint8_t *pShift, uint8_t *pByte)
{
	int ReadBits;
	int ReadPad;
	int FragmentSize;
	uint16_t Key = 0;

	for(ReadBits = 0; ReadBits < KeySize; ReadBits += FragmentSize) {
		ReadPad = (*pShift + ReadBits) % 8;
		if(ReadPad == 0) {
			if(*pSubLength == 0) {			// Update pByte. 
				FileRead(pSubLength, 1);  // Must be nonzero! 
				if(*pSubLength == 0) return 0x1000;
			}
			FileRead(pByte, 1);
			(*pSubLength)--;
		}
		FragmentSize = MIN(KeySize - ReadBits, 8 - ReadPad);
		Key |= ((uint16_t)((*pByte) >> ReadPad)) << ReadBits;
	}
	Key &= (1 << KeySize) - 1;	// Clear extra bits to the left. 
	*pShift = (*pShift + KeySize) % 8;
	return Key;
}

///////////////////////////////////////////////////////////////////////////////

// Compute output Index of y-th input line, in frame of Height h. 
int EGDecoderGIF::InterlacedLineIndex(int h, int y)
{
int p; // number of lines in current pass 

	p = (h - 1) / 8 + 1;
	if(y < p) return y * 8;     // pass 1 
	y -= p;
	p = (h - 5) / 8 + 1;
	if(y < p) return y * 8 + 4; // pass 2 
	y -= p;
	p = (h - 3) / 4 + 1;
	if(y < p) return y * 4 + 2; // pass 3 
	y -= p;
	return y * 2 + 1;	          // pass 4 
}

///////////////////////////////////////////////////////////////////////////////

/* Decompress image pixels.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
int EGDecoderGIF::ReadImageData(int interlace)
{
GIFTable_t *pTable;
GIFEntry_t Entry = {.Length = 0, .Prefix = 0, .Suffix = 0}; // compiler complains if not explicit
uint8_t SubLength, Shift, Byte;
int InitKeySize, KeySize, TableIsFull = 0;
int FrameOffset, FrameSize, StringLength = 0, i, p, x, y;
uint16_t Key, Clear, Stop;
int Result;
size_t Start, End;

	FileRead(&Byte, 1);
	KeySize = (int)Byte;
	Start = FileSeek(0, EG_FS_SEEK_CUR);
	DiscardSubblocks();
	End = FileSeek(0, EG_FS_SEEK_CUR);
	FileSeek(Start, EG_FS_SEEK_SET);
	Clear = 1 << KeySize;
	Stop = Clear + 1;
	pTable = NewTable(KeySize);
	KeySize++;
	InitKeySize = KeySize;
	SubLength = Shift = 0;
	Key = GetKey(KeySize, &SubLength, &Shift, &Byte); /* Clear code */
	FrameOffset = 0;
	Result = 0;
	FrameSize = m_FrameWidth * m_FrameHeight;
	while(FrameOffset < FrameSize) {
		if(Key == Clear) {
			KeySize = InitKeySize;
			pTable->Count = (1 << (KeySize - 1)) + 2;
			TableIsFull = 0;
		}
		else if(!TableIsFull) {
			Result = AddEntry(&pTable, StringLength + 1, Key, Entry.Suffix);
			if(Result == -1) {
				EG_FreeMem(pTable);
				return -1;
			}
			if(pTable->Count == 0x1000) {
				Result = 0;
				TableIsFull = 1;
			}
		}
		Key = GetKey(KeySize, &SubLength, &Shift, &Byte);
		if(Key == Clear) continue;
		if(Key == Stop || Key == 0x1000) break;
		if(Result == 1) KeySize++;
		EG_CopyMem(&Entry, &pTable->pEntries[Key], sizeof(GIFEntry_t)); // compiler will not do struct assingment
		StringLength = Entry.Length;
		for(i = 0; i < StringLength; i++) {
			p = FrameOffset + Entry.Length - 1;
			x = p % m_FrameWidth;
			y = p / m_FrameWidth;
			if(interlace)	y = InterlacedLineIndex((int)m_FrameHeight, y);
			m_pFrame[(m_FrameY + y) * m_Width + m_FrameX + x] = Entry.Suffix;
			if(Entry.Prefix == 0xFFF) break;
			else Entry = pTable->pEntries[Entry.Prefix];
		}
		FrameOffset += StringLength;
		if(Key < pTable->Count - 1 && !TableIsFull)
			pTable->pEntries[pTable->Count - 1].Suffix = Entry.Suffix;
	}
	EG_FreeMem(pTable);
	if(Key == Stop) FileRead(&SubLength, 1); /* Must be zero! */
	FileSeek(End, EG_FS_SEEK_SET);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

// Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). 
int EGDecoderGIF::ReadImage(void)
{
uint8_t SortLCTFlags;
int Interlace;

	m_FrameX = ReadNumber();	// Image Descriptor. 
	m_FrameY = ReadNumber();
	m_FrameWidth = ReadNumber();
	m_FrameHeight = ReadNumber();
	FileRead(&SortLCTFlags, 1);
	Interlace = SortLCTFlags & 0x40;
	if(SortLCTFlags & 0x80) {	// Ignore Sort Flag. Local Color GIFTable_t? 
		m_LocalColorTable.Size = 1 << ((SortLCTFlags & 0x07) + 1);
		FileRead(m_LocalColorTable.Colors, 3 * m_LocalColorTable.Size);		// Read LCT 
		m_pPalette = &m_LocalColorTable;
	}
	else m_pPalette = &m_GlobalColorTable;
	return ReadImageData(Interlace);	// Image Data. 
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::RenderFrameRect(uint8_t *pBuffer)
{
int i, j, k;
uint8_t Index, *pColor;

	i = m_FrameY * m_Width + m_FrameX;
	for(j = 0; j < m_FrameHeight; j++) {
		for(k = 0; k < m_FrameWidth; k++) {
			Index = m_pFrame[(m_FrameY + j) * m_Width + m_FrameX + k];
			pColor = &m_pPalette->Colors[Index * 3];
			if(!m_GlobalControlExt.Transparency || Index != m_GlobalControlExt.TransIndex) {
#if EG_COLOR_DEPTH == 32
				pBuffer[(i + k) * 4 + 0] = *(pColor + 2);
				pBuffer[(i + k) * 4 + 1] = *(pColor + 1);
				pBuffer[(i + k) * 4 + 2] = *(pColor + 0);
				pBuffer[(i + k) * 4 + 3] = 0xFF;
#elif EG_COLOR_DEPTH == 16
				EG_Color_t c = EG_MixColor(*(pColor + 0), *(pColor + 1), *(pColor + 2));
				pBuffer[(i + k) * 3 + 0] = c.full & 0xff;
				pBuffer[(i + k) * 3 + 1] = (c.full >> 8) & 0xff;
				pBuffer[(i + k) * 3 + 2] = 0xff;
#elif EG_COLOR_DEPTH == 8
				EG_Color_t c = EG_MixColor(*(pColor + 0), *(pColor + 1), *(pColor + 2));
				pBuffer[(i + k) * 2 + 0] = c.full;
				pBuffer[(i + k) * 2 + 1] = 0xff;
#elif EG_COLOR_DEPTH == 1
				uint8_t b = (*(pColor + 0)) | (*(pColor + 1)) | (*(pColor + 2));
				pBuffer[(i + k) * 2 + 0] = b > 128 ? 1 : 0;
				pBuffer[(i + k) * 2 + 1] = 0xff;
#endif
			}
		}
		i += m_Width;
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::Dispose(void)
{
int i, j, k;
uint8_t *pBackColor;

	switch(m_GlobalControlExt.Disposal) {
		case 2:{                     // Restore to background pColor. 
			pBackColor = &m_pPalette->Colors[m_BackgroundIndex * 3];
			uint8_t opa = 0xff;
			if(m_GlobalControlExt.Transparency) opa = 0x00;
			i = m_FrameY * m_Width + m_FrameX;
			for(j = 0; j < m_FrameHeight; j++) {
				for(k = 0; k < m_FrameWidth; k++) {
#if EG_COLOR_DEPTH == 32
					m_pCanvas[(i + k) * 4 + 0] = *(pBackColor + 2);
					m_pCanvas[(i + k) * 4 + 1] = *(pBackColor + 1);
					m_pCanvas[(i + k) * 4 + 2] = *(pBackColor + 0);
					m_pCanvas[(i + k) * 4 + 3] = opa;
#elif EG_COLOR_DEPTH == 16
					EG_Color_t c = EG_MixColor(*(pBackColor + 0), *(pBackColor + 1), *(pBackColor + 2));
					m_pCanvas[(i + k) * 3 + 0] = c.full & 0xff;
					m_pCanvas[(i + k) * 3 + 1] = (c.full >> 8) & 0xff;
					m_pCanvas[(i + k) * 3 + 2] = opa;
#elif EG_COLOR_DEPTH == 8
					EG_Color_t c = EG_MixColor(*(pBackColor + 0), *(pBackColor + 1), *(pBackColor + 2));
					m_pCanvas[(i + k) * 2 + 0] = c.full;
					m_pCanvas[(i + k) * 2 + 1] = opa;
#elif EG_COLOR_DEPTH == 1
					uint8_t b = (*(pBackColor + 0)) | (*(pBackColor + 1)) | (*(pBackColor + 2));
					m_pCanvas[(i + k) * 2 + 0] = b > 128 ? 1 : 0;
					m_pCanvas[(i + k) * 2 + 1] = opa;
#endif
				}
				i += m_Width;
			}
			break;
    }
		case 3:{               // Restore to previous, i.e., don't update canvas.
			break;
    }
		default:{
			RenderFrameRect(m_pCanvas);	// Add frame non-transparent pixels to canvas.
      break;
    } 
	}
}

///////////////////////////////////////////////////////////////////////////////

// Return 1 if got a frame; 0 if got GIF trailer; -1 if error. 
int EGDecoderGIF::GetFrame(void)
{
char Token;

	Dispose();
	FileRead(&Token, 1);
	while(Token != ',') {
		if(Token == ';') {
			FileSeek(m_AnimateStart, EG_FS_SEEK_SET);
			if(m_LoopCount == 1 || m_LoopCount < 0) {
				return 0;
			}
			else if(m_LoopCount > 1) {
				m_LoopCount--;
			}
		}
		else if(Token == '!') ReadExt();
		else return -1;
		FileRead(&Token, 1);
	}
	if(ReadImage() == -1) return -1;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::RenderFrame(uint8_t *pBuffer)
{
//   uint32_t i;
//   uint32_t j;
//   for(i = 0, j = 0; i < m_Width * m_Height * 3; i+= 3, j+=4) {
//     pBuffer[j + 0] = m_pCanvas[i + 2];
//     pBuffer[j + 1] = m_pCanvas[i + 1];
//     pBuffer[j + 2] = m_pCanvas[i + 0];
//     pBuffer[j + 3] = 0xFF;
//   }
//   memcpy(pBuffer, m_pCanvas, m_Width * m_Height * 3);
	RenderFrameRect(pBuffer);
}

///////////////////////////////////////////////////////////////////////////////

uint16_t EGDecoderGIF::ReadNumber(void)
{
uint8_t bytes[2];

	FileRead(bytes, 2);
	return bytes[0] + (((uint16_t)bytes[1]) << 8);
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::Rewind(void)
{
	m_LoopCount = -1;
	FileSeek(m_AnimateStart, EG_FS_SEEK_SET);
}

///////////////////////////////////////////////////////////////////////////////

bool EGDecoderGIF::FileOpen(const void *pSource, bool IsFile)
{
  m_VarReadPos = 0;
  m_pVarData = nullptr;
  m_IsFile = IsFile;
	const char *pPath = (char*)pSource;
	if(m_IsFile) {
		if(m_File.Open(pPath, EG_FS_MODE_RD) != EG_FS_RES_OK)	return false;
		else return true;
	}
	else {
		m_pVarData = pPath;
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::FileRead(void *pBuffer, size_t Length)
{
	if(m_IsFile) m_File.Read(pBuffer, Length, NULL);
	else {
		memcpy(pBuffer, &m_pVarData[m_VarReadPos], Length);
		m_VarReadPos += Length;
	}
}

///////////////////////////////////////////////////////////////////////////////

int EGDecoderGIF::FileSeek(size_t Offset, EG_FS_Seek_e Mode)
{
uint32_t Pos;

  if(m_IsFile) {
		m_File.Seek(Offset, Mode);
		m_File.Tell(&Pos);
		return Pos;
	}
	else {
		if(Mode == EG_FS_SEEK_CUR)	m_VarReadPos += Offset;
		else if(Mode == EG_FS_SEEK_SET) m_VarReadPos = Offset;
		return m_VarReadPos;
	}
}

///////////////////////////////////////////////////////////////////////////////

void EGDecoderGIF::FileClose(void)
{
	if(m_IsFile) m_File.Close();
}

#endif 
