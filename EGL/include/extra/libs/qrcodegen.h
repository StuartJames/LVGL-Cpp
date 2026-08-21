/*
 * QR Code generator library (C)
 *
 * Copyright (c) Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/qr-code-generator-library
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * This library creates QR Code symbols, which is a type of two-dimension barcode.
 * Invented by Denso Wave and described in the ISO/IEC 18004 standard.
 * A QR Code structure is an immutable square grid of black and white cells.
 * The library provides functions to create a QR Code from text or binary data.
 * The library covers the QR Code Model 2 specification, supporting all versions (sizes)
 * from 1 to 40, all 4 error correction levels, and 4 character encoding modes.
 *
 * Ways to create a QR Code object:
 * - High level: Take the payload data and call QRCodeGenEncodeText() or QRCodeGenEncodeBinary().
 * - Low level: Custom-make the list of segments and call
 *   QRCodeGenEncodeSegments() or QRCodeGenEncodeSegmentsAdvanced().
 * (Note that all ways require supplying the desired error correction level and various byte buffers.)
 */


/*---- Enum and struct types----*/

/*
 * The error correction level in a QR Code symbol.
 */
enum QRCodeGenEcc_e {
	// Must be declared in ascending order of error protection
	// so that an internal qrcodegen function works properly
	QRCODEGEN_ECC_LOW = 0 ,  // The QR Code can tolerate about  7% erroneous codewords
	QRCODEGEN_ECC_MEDIUM  ,  // The QR Code can tolerate about 15% erroneous codewords
	QRCODEGEN_ECC_QUARTILE,  // The QR Code can tolerate about 25% erroneous codewords
	QRCODEGEN_ECC_HIGH    ,  // The QR Code can tolerate about 30% erroneous codewords
};


/*
 * The mask pattern used in a QR Code symbol.
 */
enum QRCodeGenMask_e {
	// A special value to tell the QR Code encoder to
	// automatically select an appropriate mask pattern
	QRCODEGEN_MASK_AUTO = -1,
	// The eight actual mask patterns
	QRCODEGEN_MASK_0 = 0,
	QRCODEGEN_MASK_1,
	QRCODEGEN_MASK_2,
	QRCODEGEN_MASK_3,
	QRCODEGEN_MASK_4,
	QRCODEGEN_MASK_5,
	QRCODEGEN_MASK_6,
	QRCODEGEN_MASK_e_7,
};


/*
 * Describes how a segment's data bits are interpreted.
 */
enum QRCodeGenMode_e {
	QRCODEGEN_MODE_NUMERIC      = 0x1,
	QRCODEGEN_MODE_ALPHANUMERIC = 0x2,
	QRCODEGEN_MODE_BYTE         = 0x4,
	QRCODEGEN_MODE_KANJI        = 0x8,
	QRCODEGEN_MODE_ECI          = 0x7,
};


/*
 * A segment of character/binary/control data in a QR Code symbol.
 * The mid-level way to create a segment is to take the payload data
 * and call a factory function such as qrcodegen_makeNumeric().
 * The low-level way to create a segment is to custom-make the bit buffer
 * and initialize a QRCodeGenSegment_t struct with appropriate values.
 * Even in the most favorable conditions, a QR Code can only hold 7089 characters of data.
 * Any segment longer than this is meaningless for the purpose of generating QR Codes.
 * Moreover, the maximum allowed bit length is 32767 because
 * the largest QR Code (version 40) has 31329 modules.
 */
struct QRCodeGenSegment_t {
	// The mode indicator of this segment.
	QRCodeGenMode_e mode;
	
	// The length of this segment's unencoded data. Measured in characters for
	// numeric/alphanumeric/kanji mode, bytes for byte mode, and 0 for ECI mode.
	// Always zero or positive. Not the same as the data's bit length.
	int numChars;
	
	// The data bits of this segment, packed in bitwise big endian.
	// Can be null if the bit length is zero.
	uint8_t *data;
	
	// The number of valid data bits used in the buffer. Requires
	// 0 <= bitLength <= 32767, and bitLength <= (capacity of data array) * 8.
	// The character count (numChars) must agree with the mode and the bit buffer length.
	int bitLength;
};



/*---- Macro constants and functions ----*/

#define QRCODEGEN_VERSION_MIN   1  // The minimum version number supported in the QR Code Model 2 standard
#define QRCODEGEN_VERSION_MAX  40  // The maximum version number supported in the QR Code Model 2 standard

// Calculates the number of bytes needed to store any QR Code up to and including the given version number,
// as a compile-time constant. For example, 'uint8_t buffer[QRCODEGEN_BUFFER_LEN_FOR_VERSION(25)];'
// can store any single QR Code from version 1 to 25 (inclusive). The result fits in an int (or int16).
// Requires QRCODEGEN_VERSION_MIN <= n <= QRCODEGEN_VERSION_MAX.
#define QRCODEGEN_BUFFER_LEN_FOR_VERSION(n)  ((((n) * 4 + 17) * ((n) * 4 + 17) + 7) / 8 + 1)

// The worst-case number of bytes needed to store one QR Code, up to and including
// version 40. This value equals 3918, which is just under 4 kilobytes.
// Use this more convenient value to avoid calculating tighter memory bounds for buffers.
#define QRCODEGEN_BUFFER_LEN_MAX  QRCODEGEN_BUFFER_LEN_FOR_VERSION(QRCODEGEN_VERSION_MAX)



/*---- Functions (high level) to generate QR Codes ----*/

/*
 * Encodes the given text string to a QR Code, returning true if encoding succeeded.
 * If the data is too long to fit in any version in the given range
 * at the given ECC level, then false is returned.
 * - The input text must be encoded in UTF-8 and contain no NULs.
 * - The variables ecl and mask must correspond to enum constant values.
 * - Requires 1 <= minVersion <= maxVersion <= 40.
 * - The arrays tempBuffer and qrcode must each have a length
 *   of at least QRCODEGEN_BUFFER_LEN_FOR_VERSION(maxVersion).
 * - After the function returns, tempBuffer contains no useful data.
 * - If successful, the resulting QR Code may use numeric,
 *   alphanumeric, or byte mode to encode the text.
 * - In the most optimistic case, a QR Code at version 40 with low ECC
 *   can hold any UTF-8 string up to 2953 bytes, or any alphanumeric string
 *   up to 4296 characters, or any digit string up to 7089 characters.
 *   These numbers represent the hard upper limit of the QR Code standard.
 * - Please consult the QR Code specification for information on
 *   data capacities per version, ECC level, and text encoding mode.
 */
bool QRCodeGenEncodeText(const char *text, uint8_t tempBuffer[], uint8_t qrcode[],
	QRCodeGenEcc_e ecl, int minVersion, int maxVersion, QRCodeGenMask_e mask, bool boostEcl);


/*
 * Encodes the given binary data to a QR Code, returning true if encoding succeeded.
 * If the data is too long to fit in any version in the given range
 * at the given ECC level, then false is returned.
 * - The input array range dataAndTemp[0 : dataLen] should normally be
 *   valid UTF-8 text, but is not required by the QR Code standard.
 * - The variables ecl and mask must correspond to enum constant values.
 * - Requires 1 <= minVersion <= maxVersion <= 40.
 * - The arrays dataAndTemp and qrcode must each have a length
 *   of at least qrcodegen_BUFFER_LEN_FOR_VERSION(maxVersion).
 * - After the function returns, the contents of dataAndTemp may have changed,
 *   and does not represent useful data anymore.
 * - If successful, the resulting QR Code will use byte mode to encode the data.
 * - In the most optimistic case, a QR Code at version 40 with low ECC can hold any byte
 *   sequence up to length 2953. This is the hard upper limit of the QR Code standard.
 * - Please consult the QR Code specification for information on
 *   data capacities per version, ECC level, and text encoding mode.
 */
bool QRCodeGenEncodeBinary(uint8_t dataAndTemp[], size_t dataLen, uint8_t qrcode[],
	QRCodeGenEcc_e ecl, int minVersion, int maxVersion, QRCodeGenMask_e mask, bool boostEcl);


/*---- Functions (low level) to generate QR Codes ----*/

/*
 * Renders a QR Code representing the given segments at the given error correction level.
 * The smallest possible QR Code version is automatically chosen for the output. Returns true if
 * QR Code creation succeeded, or false if the data is too long to fit in any version. The ECC level
 * of the result may be higher than the ecl argument if it can be done without increasing the version.
 * This function allows the user to create a custom sequence of segments that switches
 * between modes (such as alphanumeric and byte) to encode text in less space.
 * This is a low-level API; the high-level API is QRCodeGenEncodeText() and QRCodeGenEncodeBinary().
 * To save memory, the segments' data buffers can alias/overlap tempBuffer, and will
 * result in them being clobbered, but the QR Code output will still be correct.
 * But the qrcode array must not overlap tempBuffer or any segment's data buffer.
 */
bool QRCodeGenEncodeSegments(const struct QRCodeGenSegment_t segs[], size_t len,
	QRCodeGenEcc_e ecl, uint8_t tempBuffer[], uint8_t qrcode[]);


/*
 * Renders a QR Code representing the given segments with the given encoding parameters.
 * Returns true if QR Code creation succeeded, or false if the data is too long to fit in the range of versions.
 * The smallest possible QR Code version within the given range is automatically
 * chosen for the output. Iff boostEcl is true, then the ECC level of the result
 * may be higher than the ecl argument if it can be done without increasing the
 * version. The mask number is either between 0 to 7 (inclusive) to force that
 * mask, or -1 to automatically choose an appropriate mask (which may be slow).
 * This function allows the user to create a custom sequence of segments that switches
 * between modes (such as alphanumeric and byte) to encode text in less space.
 * This is a low-level API; the high-level API is QRCodeGenEncodeText() and QRCodeGenEncodeBinary().
 * To save memory, the segments' data buffers can alias/overlap tempBuffer, and will
 * result in them being clobbered, but the QR Code output will still be correct.
 * But the qrcode array must not overlap tempBuffer or any segment's data buffer.
 */
bool QRCodeGenEncodeSegmentsAdvanced(const struct QRCodeGenSegment_t segs[], size_t len, QRCodeGenEcc_e ecl,
	int minVersion, int maxVersion, int mask, bool boostEcl, uint8_t tempBuffer[], uint8_t qrcode[]);


/*
 * Tests whether the given string can be encoded as a segment in alphanumeric mode.
 * A string is encodable iff each character is in the following set: 0 to 9, A to Z
 * (uppercase only), space, dollar, percent, asterisk, plus, hyphen, period, slash, colon.
 */
bool QRCodeGenIsAlphanumeric(const char *text);


/*
 * Tests whether the given string can be encoded as a segment in numeric mode.
 * A string is encodable iff each character is in the range 0 to 9.
 */
bool QRCodeGenIsNumeric(const char *text);


/*
 * Returns the number of bytes (uint8_t) needed for the data buffer of a segment
 * containing the given number of characters using the given mode. Notes:
 * - Returns SIZE_MAX on failure, i.e. numChars > INT16_MAX or
 *   the number of needed bits exceeds INT16_MAX (i.e. 32767).
 * - Otherwise, all valid results are in the range [0, ceil(INT16_MAX / 8)], i.e. at most 4096.
 * - It is okay for the user to allocate more bytes for the buffer than needed.
 * - For byte mode, numChars measures the number of bytes, not Unicode code points.
 * - For ECI mode, numChars must be 0, and the worst-case number of bytes is returned.
 *   An actual ECI segment can have shorter data. For non-ECI modes, the result is exact.
 */
size_t QRCodeGenCalcSegmentBufferSize(QRCodeGenMode_e mode, size_t numChars);


/*
 * Returns a segment representing the given binary data encoded in
 * byte mode. All input byte arrays are acceptable. Any text string
 * can be converted to UTF-8 bytes and encoded as a byte mode segment.
 */
struct QRCodeGenSegment_t QRCodeGenMakeBytes(const uint8_t data[], size_t len, uint8_t buf[]);


/*
 * Returns a segment representing the given string of decimal digits encoded in numeric mode.
 */
struct QRCodeGenSegment_t QRCodeGenMakeNumeric(const char *digits, uint8_t buf[]);


/*
 * Returns a segment representing the given text string encoded in alphanumeric mode.
 * The characters allowed are: 0 to 9, A to Z (uppercase only), space,
 * dollar, percent, asterisk, plus, hyphen, period, slash, colon.
 */
struct QRCodeGenSegment_t QRCodeGenMakeAlphanumeric(const char *text, uint8_t buf[]);


/*
 * Returns a segment representing an Extended Channel Interpretation
 * (ECI) designator with the given assignment value.
 */
struct QRCodeGenSegment_t QRCodeGenMakeEci(long assignVal, uint8_t buf[]);


/*---- Functions to extract raw data from QR Codes ----*/

/*
 * Returns the side length of the given QR Code, assuming that encoding succeeded.
 * The result is in the range [21, 177]. Note that the length of the array buffer
 * is related to the side length - every 'uint8_t qrcode[]' must have length at least
 * qrcodegen_BUFFER_LEN_FOR_VERSION(version), which equals ceil(size^2 / 8 + 1).
 */
int QRCodeGenGetSize(const uint8_t qrcode[]);


/*
 * Returns the color of the module (pixel) at the given coordinates, which is false
 * for white or true for black. The top left corner has the coordinates (x=0, y=0).
 * If the given coordinates are out of bounds, then false (white) is returned.
 */
bool QRCodeGenGetModule(const uint8_t qrcode[], int x, int y);

/*
 * Returns the qrcode size of the specified version. Returns -1 on failure
 */
int QRCodeGenVersion2size(int version);
/*
 * Returns the min version of the data that can be stored. Returns -1 on failure
 */
int QRCodeGenGetMinFitVersion(QRCodeGenEcc_e ecl, size_t dataLen);

#ifdef __cplusplus
}
#endif
