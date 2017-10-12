#include <windows.h>
#include <tchar.h>
#include <locale.h>
#include <stdio.h>
#include "GrayFont.h"
#include "StrParse.h"

#define RGB_WHITE		RGB(255,255,255)

void FillRect(HDC hDC, LPCRECT pRect, COLORREF rgb)
{
	COLORREF rgbOld = SetBkColor(hDC, rgb);
	int mode = SetBkMode(hDC, OPAQUE);
	ExtTextOut(hDC, 0,0, ETO_OPAQUE, pRect, NULL, 0, NULL);
	SetBkMode(hDC, mode);
	SetBkColor(hDC, rgbOld);
}

typedef unsigned short FONTCHAR;

SIZE CalcFontSize(HDC hDC,  FONTCHAR firstChar, FONTCHAR lastChar)
{
	SIZE fontSize;
	FONTCHAR ch;

	fontSize.cx = fontSize.cy = 0;

	for (ch = firstChar; ch <= lastChar; ++ch)
	{
		// Set current character & width
		SIZE size;
		GetTextExtentPoint32(hDC, (TCHAR*) &ch, 1, &size);
		fontSize.cx += size.cx;
		if (size.cy > fontSize.cy)
		{
			fontSize.cy = size.cy;
		}
	}
	return fontSize;
}

#define OUT_BMP		0

#define GRAY_BITS	2
#define GRAYS		(1 << GRAY_BITS)

int OutGrayFontHeader(int width, int height, WCHAR firstCh, WCHAR lastCh, FILE* fout)
{
	const int pixelsPerByte = (8 / GRAY_BITS);
	const int bytes = (width + pixelsPerByte - 1) / pixelsPerByte;
	const int rowBytes = (bytes + 3) & ~3;
#if OUT_BMP
	/*
	 *       Gray fonts are currently defined as a MONOCHROME (1-bit) 
	 *  DOUBLE WIDTH DIB file format.  This is because Windows doesn't 
	 *  support 2-bit DIBs.
	 *
	 *       Using this format has some advantages for debugging: e.g. 
	 *  you can use a standard Bitmap editor to "preview/edit" a grayscale 
	 *  font.  Using the bfOffBits field the "width" table can be stored 
	 *  after the color table & before the bitmap bits but this information 
	 *  will be lost by most bitmap editors...
	 */

	/* First output BITMAPFILEHEADER */
	BITMAPFILEHEADER bfHeader;
	memset(&bfHeader, 0, sizeof(BITMAPFILEHEADER));
	bfHeader.bfType = 'B' | ('M' << 8);
	const int offset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;
	bfHeader.bfOffBits = offset;
	bfHeader.bfSize = rowBytes * height + offset;
	fwrite(&bfHeader, 1, sizeof(bfHeader), fout);

	/* Then output BITMAPINFOHEADER */
	BITMAPINFOHEADER bmiHeader;
	memset(&bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biBitCount = 1;
	bmiHeader.biPlanes = 1;
	bmiHeader.biHeight = -height;
	bmiHeader.biWidth = width * 2;  /* rightside up DIB */
	fwrite(&bmiHeader, 1, sizeof(bmiHeader), fout);

	/* Then write bmiColorTable */
	for (int grey = 0; grey <= 255; grey += 255)
	{
		RGBQUAD rgb;
		rgb.rgbRed = rgb.rgbGreen = rgb.rgbBlue = grey;
		rgb.rgbReserved = 0;
		fwrite(&rgb, 1, sizeof(rgb), fout);
	}
#else
	fputc('G', fout);
	fputc('F', fout);
	fputc(height, fout);
	fputc(0, fout);
#endif
	return 0;
}

#define OUT_GRAY(gray) \
	shift = (shift << GRAY_BITS) | (gray); \
	if (((bitcnt += GRAY_BITS) & 7) == 0) \
	{ \
		fputc(shift, fout); \
		shift = 0; \
	}

int OutFontRow(HDC hDC,  int row, WCHAR firstChar, WCHAR lastChar, FILE* fout)
{
	WCHAR ch;
	SIZE charSize;
	RECT rect;
	TEXTMETRIC tm;
	FONT_U8 shift;
	int bitcnt;

	GetTextMetrics(hDC, &tm);

	bitcnt = shift = 0;
	for (ch = firstChar; ch <= lastChar; ++ch)
	{
		int col, width, height;

		charSize = CalcFontSize(hDC, ch, ch);
		width = charSize.cx;
		height = charSize.cy;

		rect.left = 0;
		rect.top = 0;
		rect.right = tm.tmMaxCharWidth;
		rect.bottom = tm.tmHeight;
		FillRect(hDC, &rect, RGB_WHITE);
#ifdef UNICODE
		BOOL bOk = TextOutW(hDC, 0, 0, &ch, 1);
#else
		BOOL bOk = TextOut(hDC, 0, 0, (const char*) &ch, 1);
#endif
		for (col = 0; col < charSize.cx; ++col)
		{
			COLORREF rgb = GetPixel(hDC, col, row);
			int level = GetRValue(rgb) + GetBValue(rgb) + GetGValue(rgb);
			int  gray = (GRAYS - 1) - (level / ((256/GRAYS)*3));
			OUT_GRAY(gray);
		}
	}
	/* Flush out shift, bitcnt, pad to 64-bit/16-bit boundary */
#if OUT_BMP
	while (bitcnt & 31)
#else
	while (bitcnt & 7)
#endif
	{
		OUT_GRAY(0);
	}

	return bitcnt / GRAY_BITS;
}

int OutFont(HDC hDC, const TCHAR* szFaceName, int height, FILE* fout, WCHAR firstCh, WCHAR lastCh)
{
	LOGFONT logfont;
	HFONT hFont;
	int result = -1;
	int rowPixels;

	memset(&logfont, 0, sizeof(logfont));
	lstrcpy(logfont.lfFaceName, szFaceName);
	logfont.lfHeight = height;

	hFont = CreateFontIndirect(&logfont);
	if (hFont != NULL)
	{
		HGDIOBJ hOldFont = SelectObject(hDC, hFont);

		SIZE fontSize = CalcFontSize(hDC, firstCh, lastCh);
		int width = fontSize.cx;
		int height = fontSize.cy;
		OutGrayFontHeader(width, height, firstCh, lastCh, fout);
#if !OUT_BMP
		/* Output "width" table */
		rowPixels = 0;
		for (int ch = firstCh; ch <= lastCh; ++ch)
		{
			SIZE charSize = CalcFontSize(hDC, ch, ch);
			FONT_U8 width = charSize.cx;
			fputc(width, fout);
			rowPixels += width;
		}
#endif
		/* Output "font bits" */
		for (int row = 0; row < height; ++row)
		{
			rowPixels = OutFontRow(hDC, row, firstCh, lastCh, fout);
		}

		// Restore DC and delete Font
		SelectObject(hDC, hOldFont);
		DeleteObject(hFont);
	}
	return 0;
}

HBITMAP CreateGrayDIB(HDC hDC, LPVOID* pBits, int width, int height)
{
	register int i;

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biClrUsed = 256;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biWidth = width;
	for (i = 0; i < 256; ++i)
	{
		RGBQUAD rgbq;
		rgbq.rgbBlue = rgbq.rgbGreen = rgbq.rgbRed = i;
		rgbq.rgbReserved = 0;
	}
	return CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, pBits, NULL, 0L);
}


#ifdef UNICODE
	int wmain(int argc, const WCHAR* argv[])
#else
	int main(int argc, const char* argv[])
#endif
{
	int height = 0;
	FILE* fout = stdout;
	WCHAR first = 32;
	WCHAR last = 255;

	fprintf(stderr, "MakeGrayFont v1.00 (12-Nov-2010)\n");
	if (argc != 4)
	{
		fprintf(stderr, "usage: GrayFont FontName Size outfile");
		return -1;
	}

	setlocale(LC_CTYPE, _T(""));

	height = Str2Int(argv[2], 10);
	if (height < 5)
	{
		fprintf(stderr, "Height must be >= 5!\n");
		return -3;
	}

	fout = fopen(argv[3], "wb");

	/* Get Screen Device Context */
	HDC hDC = CreateCompatibleDC(NULL);
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 8;

	LPVOID pBits = NULL;
	HBITMAP hBitmap = CreateGrayDIB(hDC, &pBits, 320, 256);
	HGDIOBJ hOldBitmap = SelectObject(hDC, hBitmap);
	OutFont(hDC, argv[1], height, fout, 0, 255);
	SelectObject(hDC, hOldBitmap);
	DeleteDC(hDC);

	fclose(fout);

	return 0;
}
