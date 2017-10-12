/**
 * @file
 * @brief	Header file for the GRAY_FONT structure
 *
 * Header file for the GRAY_FONT structure representing a 2-bit 
 * Grayscale Font.
 *
 * @author  Lennie Araki
 * @version 1.0
 *
 * @section LICENSE
 * Copyright © 2010-2011. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */
#ifndef __GRAYFONT_H__
#define __GRAYFONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "canvas.h"

typedef unsigned char FONT_U8;		/*!< 8-bit Unsigned Font Data Type */
typedef unsigned short FONT_U16;	/*!< 16-bit Unsigned Font Data Type */

#define GRAY_MAX_CHARS		256

/**
 *	GRAY_LEVEL is an enumeration for the 4 2-bit grayscale codes used in
 *  a GRAY_FONT file or in memory.
 */
typedef enum gray_level
{
	GRAY_TRANSPARENT = 0, 	/*!< Transparent (100% back) */
	GRAY_33PERCENT = 1,		/*!< 33% opaque (33% back 67% text) */
	GRAY_67PERCENT = 2,		/*!< 66% opaque (67% back 33% text) */
	GRAY_OPAQUE = 3,		/*!< 100% opaque (100% text) */
	GRAY_LEVELS = 4			/*!< Number of Gray Levels */
} GRAY_LEVEL;

/**
 *  @brief Grayscale font structure
 *
 *	struct gray_font consists of a 4-byte header followed by the font data.
 *  The font data consists of a "width table" (array of FONT_U8[lastChar-firstChar+1]
 *  followed by the "gray scale bits" (packed 2 bits per pixel, MSB first).
 *  Since all fields are 8-bit bytes, the file format is endian independent.
 *
 *  Note: an "fast" implementation would parse this byte stream into a gray_work
 *  structure so that the "width table" doesn't have to be scanned to find
 *  the "pixel offset" for each character.  For simplicity & memory efficiency
 *  this reference imlementation directly operations on the byte stream
 *  trading off memory for speed.
 */
typedef struct gray_font
{
	const FONT_U8   grayIdG;        /*!< Gray Font Id: 'G' */
	const FONT_U8   grayIdF;        /*!< Gray Font Id: 'F' */
	const FONT_U8	fontHeight;		/*!< Font Height in pixels */
	const FONT_U8	fontAscent;		/*!< Font Ascent in pixels */
	const FONT_U8	fontWidth[GRAY_MAX_CHARS];	/*!< Font "width table" */
	const FONT_U8	fontPixels[1];	/*!< Font "pixel data" */

} GRAY_FONT;


typedef struct gray_work
{
	FONT_U8			fontHeight;		/*!< Font Height in pixels */
	FONT_U8			fontAscent;		/*!< Font Ascent in pixels */
	FONT_U16		fontOffsetTable[GRAY_MAX_CHARS];	/*!< Font "offset table" + "pixel data" */
	FONT_U8*        fontBits;       /*!< Pointer to "font bits" */

} GRAY_WORK;


int DrawGrayChar(CANVAS canvas, const GRAY_FONT* pFont, int x, int y, WCHAR ch);
int DrawGrayText(CANVAS canvas, const GRAY_FONT* pFont, int x, int y, LPCTSTR lpString, int count);

#ifdef WIN32

void SetGrayColors(LPBITMAPINFO pBMI, COLORREF rgbBack, COLORREF rgbText);
LPBITMAPINFO InitGrayFont(const GRAY_FONT* pFont);
BOOL DrawGrayFont(CANVAS canvas, LPBITMAPINFO pBMI, const GRAY_FONT* pFont, WCHAR ch, int x, int y, int zoom);
void FreeGrayFont(LPBITMAPINFO pBits);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __GRAYFONT_H__ */
