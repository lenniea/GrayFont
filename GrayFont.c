/**
 * @file
 * @brief	Implementation file for the GRAY_FONT structure
 *
 * C Implementation file for the GRAY_FONT structure representing
 * a 2-bit Grayscale Font.
 *
 * @author  Lennie Araki
 * @version 1.0
 *
 * @section LICENSE
 * Copyright © 2010. All rights reserved.
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

#include <windows.h>
#include "GrayFont.h"
#include "Canvas.h"

#define PERCENT_33		21845		/*!< 33% Gray constant (numerator) */
#define PERCENT_67		43690		/*!< 67% Gray constant (numerator) */
#define PERCENT_100		65535		/*!< 100% Gray constant (denominator) */

static COLORREF MixColors(int percent, COLORREF rgb1, COLORREF rgb2)
{
	const int remain = PERCENT_100 - percent;
	const int red = MulDiv(GetRValue(rgb1), percent, PERCENT_100) + MulDiv(GetRValue(rgb2), remain, PERCENT_100);
	const int grn = MulDiv(GetGValue(rgb1), percent, PERCENT_100) + MulDiv(GetGValue(rgb2), remain, PERCENT_100);
	const int blu = MulDiv(GetBValue(rgb1), percent, PERCENT_100) + MulDiv(GetBValue(rgb2), remain, PERCENT_100);
	return RGB(red, grn, blu);
}

static GRAY_LEVEL GetGrayLevel(const GRAY_FONT* pFont, int rowBytes, int row, int col)
{
	int index = (row * rowBytes) + (col >> 2);
	const FONT_U8* pixels = &pFont->fontPixels[0];
	const FONT_U8* pGray = pixels + index;
	return (*pGray >> (3 - (col & 3)) * 2) & 3;
}

static long SumFontWidths(const FONT_U8* pWidth, int count)
{
	long sum = 0;
	while (count)
	{
		sum += *pWidth++;
		--count;
	}
	return sum;
}

/**
 *
 *  DrawGrayChar draws one 16-bit character @(x,y) to a canvas
 * @param	canvas - the canvas to draw the character on
 * @param	pFont - pointer to GRAY_FONT structure
 * @param	x - x-coordinate
 * @param	y - y-coordindate
 * @param	ch - 16-bit UNICODE character
 * @return	width of character drawn in pixels (negative for error)
 */
int DrawGrayChar(CANVAS canvas, const GRAY_FONT* pFont, int x, int y, WCHAR ch)
{
	COLORREF rgbGray[GRAY_LEVELS];
	int index, column, rowBytes;
	int width = -1;
	COLORREF rgbBack = GetBkColor(canvas);
	COLORREF rgbText = GetTextColor(canvas);

	/* Scan width array to calculate starting column and rowBytes */
	index = ch;
	column = 0;
	if (index < 0 || index >= GRAY_MAX_CHARS)
		index = 0;
	else
		column = SumFontWidths(pFont->fontWidth, index);

	rowBytes = (column + SumFontWidths(pFont->fontWidth + index, GRAY_MAX_CHARS - index) + 3) >> 2;

	/* Calculate 33% and 67% gray color values */
	rgbGray[GRAY_TRANSPARENT] = rgbBack;
	rgbGray[GRAY_33PERCENT] = MixColors(PERCENT_33, rgbText, rgbBack);
	rgbGray[GRAY_67PERCENT] = MixColors(PERCENT_67, rgbText, rgbBack);
	rgbGray[GRAY_OPAQUE] = rgbText;

	index = ch;
	if (index >= 0 && index < GRAY_MAX_CHARS)
	{
		int row;
		width = pFont->fontWidth[index];
		for (row = 0; row < pFont->fontHeight; ++row)
		{
			int count;
			int colchar = column;
			int xchar = x;
			for (count = width; count > 0; --count)
			{
				UINT gray = GetGrayLevel(pFont, rowBytes, row, colchar++);
				SetPixel(canvas, xchar++, y + row, rgbGray[gray]);
			}
		}
	}
	return width;
}


/**
 *
 *  DrawGrayText draws an array of 8-bit ANSI characters starting @ (x,y) on a canvas
 * @param	canvas - the canvas to draw the character on
 * @param	pFont - pointer to GRAY_FONT structure
 * @param	x - x-coordinate
 * @param	y - y-coordindate
 * @param	lpString - pointer to array of characters to draw
 * @param	count - # of characters in array
 * @return	width of string drawn in pixels (negative for error)
 */
int DrawGrayText(CANVAS canvas, const GRAY_FONT* pFont, int x, int y, LPCTSTR lpString, int count)
{
	int i;
	int width = -1;
	if (count < 0)
	{
		count = strlen(lpString);
	}
	for (i = 0; i < count; ++i)
	{
		width = DrawGrayChar(canvas, pFont, x, y, lpString[i]);
		if (width < 0)
			break;
		x += width;
	}
	return width;
}

#ifdef WIN32

#define RGB_BLACK	RGB(0,0,0)
#define RGB_WHITE	RGB(255,255,255)

typedef struct bitmapinfo4
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[4];
}
BITMAPINFO4, *LPBITMAPINFO4;

RGBQUAD RGBtoQUAD(COLORREF rgb)
{
	RGBQUAD quad;
	quad.rgbRed = GetRValue(rgb);
	quad.rgbGreen = GetGValue(rgb);
	quad.rgbBlue = GetBValue(rgb);
	quad.rgbReserved = 0;
	return quad;
}

void SetGrayColors(LPBITMAPINFO pBMI, COLORREF rgbBack, COLORREF rgbText)
{
	RGBQUAD* pColors = pBMI->bmiColors;
	pColors[0] = RGBtoQUAD(rgbBack);
	pColors[1] = RGBtoQUAD(MixColors(PERCENT_33, rgbText, rgbBack));
	pColors[2] = RGBtoQUAD(MixColors(PERCENT_67, rgbText, rgbBack));
	pColors[3] = RGBtoQUAD(rgbText);
}

LPBITMAPINFO InitGrayFont(const GRAY_FONT* pFont)
{
	int height = pFont->fontHeight;
	int rowPixels = SumFontWidths(pFont->fontWidth, GRAY_MAX_CHARS);
	int rowBytes = (rowPixels + 3) >> 2;
	int outBytes = (rowPixels  + 1) >> 1;
	/* round to 32-bit boundary */
	int outWidth = (outBytes + 3) & ~3;
	size_t bytes = sizeof(BITMAPINFO4) + outWidth * height;
	LPBITMAPINFO pBMI = (LPBITMAPINFO) malloc(bytes);
	LPBYTE pBitmap = (LPBYTE) &pBMI->bmiColors[4];
	LPBYTE pBits;
	int row,col;

	memset(pBMI, 0, sizeof(BITMAPINFO4));
	pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBMI->bmiHeader.biBitCount = 4;
	pBMI->bmiHeader.biPlanes = 1;
	pBMI->bmiHeader.biHeight = -height;
	pBMI->bmiHeader.biWidth = rowPixels;
	pBMI->bmiHeader.biClrUsed = pBMI->bmiHeader.biClrImportant = 4;

	/* Setup color table */
	SetGrayColors(pBMI, RGB_WHITE, RGB_BLACK);

	/* Expand the packed font bits to 4-bit bitmap */
	pBits = pBitmap;
	for (row = 0; row < height; ++row)
	{
		int index = (row * rowBytes);
		const FONT_U8* pixels = pFont->fontPixels;
		const FONT_U8* pGray = pixels + index;

		col = 0;
		while (col < outBytes)
		{
			/* Expand 4 2-bit gray pixels into 2 4-bit DIB pixels */
			int gray = *pGray++;
			pBits[col++] = ((gray >> 2) & 0x30) | ((gray >> 4) & 0x03);
			pBits[col++] = ((gray << 2) & 0x30) | (gray & 0x03);
		}
		pBits += outWidth;
	}
	return pBMI;
}

BOOL DrawGrayFont(CANVAS canvas, LPBITMAPINFO pBMI, const GRAY_FONT* pFont, WCHAR ch, int x, int y, int zoom)
{
	int height = pFont->fontHeight;
	LPBYTE pBits = (LPBYTE) &pBMI->bmiColors[4];

	/* Scan width array to calculate starting column and rowBytes */
	int index = ch;
	int width = pFont->fontWidth[index];
	int column = 0;
	if (index < 0 || index >= GRAY_MAX_CHARS)
		column = 0;
	else
		column = SumFontWidths(pFont->fontWidth, index);

	return StretchDIBits(canvas, x - ((width >> 1) << zoom), y - ((height >> 1) << zoom), width << zoom, height << zoom, 
						 column, /*row*/0, width, height, pBits, pBMI, DIB_RGB_COLORS, SRCCOPY);
}

void FreeGrayFont(LPBITMAPINFO pBMI)
{
	free(pBMI);
}

#endif /* WIN32 */
