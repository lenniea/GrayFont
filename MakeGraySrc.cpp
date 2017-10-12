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

	for (ch = firstChar; ch < lastChar; ++ch)
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

int OutGrayFont(TEXTMETRIC& tm, const TCHAR* szFaceName, WCHAR firstCh, WCHAR lastCh, FILE* fout)
{
	int width = tm.tmMaxCharWidth;
	int height = tm.tmHeight;

	/* First output font header */
#ifdef UNICODE
	_ftprintf(fout, _T("\nconst FONT_U8 font%ls%u[] =\n{\n"), szFaceName, height);
#else
	_ftprintf(fout, _T("\nconst FONT_U8 font%s%u[] =\n{\n"), szFaceName, height);
#endif
	_ftprintf(fout, _T("\t0x%02x,\t\t/* firstChar */\n"), firstCh);
	_ftprintf(fout, _T("\t0x%02x,\t\t/* lastChar */\n"), lastCh);
	_ftprintf(fout, _T("\t%u,\t\t\t/* fontHeight */\n"), tm.tmHeight);
	_ftprintf(fout, _T("\t%u,\t\t\t/* fontAscent */\n"), tm.tmAscent);
	return 0;
}

#ifdef _DEBUG

const TCHAR asciiGray[GRAY_LEVELS] =
{
	' ', '.',':','*'
};

#endif

#define OUT_GRAY(gray) \
	shift = (shift << 2) | (gray); \
	if (((bitcnt += 2) & 7) == 0) \
	{ \
		fprintf(fout, "0x%02x,", shift); \
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

	_ftprintf(fout, _T("\t/* row %u: '%c' to %c' */\n\t"), row, firstChar, lastChar);

	bitcnt = shift = 0;
	for (ch = firstChar; ch <= lastChar; ++ch)
	{
		int col, width, height;
#ifdef UNICODE
		// Get current character & width
		GetTextExtentPoint32(hDC, &ch, 1, &charSize);
#else
		TCHAR buf[4];
		int bytes = wctomb(buf, ch);
		// Get current character & width
		GetTextExtentPoint32(hDC, buf, bytes, &charSize);
#endif
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
		BOOL bOk = TextOut(hDC, 0, 0, buf, bytes);
#endif
		for (col = 0; col < charSize.cx; ++col)
		{
			COLORREF rgb = GetPixel(hDC, col, row);
			int level = GetRValue(rgb) + GetBValue(rgb) + GetGValue(rgb);
			int  gray = (GRAY_LEVELS - 1) - (level / (64*3));
#ifdef _DEBUG
			putc(asciiGray[gray], stdout);
#endif
			OUT_GRAY(gray);
			if ((bitcnt & 127) == 0)
			{
				_ftprintf(fout, _T("\n\t"));
			}
		}
	}
	/* Flush out shift, bitcnt */
	while (bitcnt & 7)
	{
		OUT_GRAY(0);
	}
#ifdef _DEBUG
	putc('\n', stdout);
#endif
	putc('\n', fout);
	return 0;
}

int OutFont(HDC hDC, const TCHAR* szFaceName, int height, FILE* fout, WCHAR firstCh, WCHAR lastCh)
{
	LOGFONT logfont;
	HFONT hFont;
	int result = -1;

	memset(&logfont, 0, sizeof(logfont));
	lstrcpy(logfont.lfFaceName, szFaceName);
	logfont.lfHeight = height;

	hFont = CreateFontIndirect(&logfont);
	if (hFont != NULL)
	{
		TEXTMETRIC tm;
		int row,col;
		WCHAR ch;
		SIZE charSize;

		HGDIOBJ hOldFont = SelectObject(hDC, hFont);

		// Get Text Metrics
		GetTextMetrics(hDC, &tm);
		int width = tm.tmMaxCharWidth;
		int height = tm.tmHeight;

		/* Next output "width table */
		_ftprintf(fout, _T("/*\n")
#ifdef UNICODE
			          _T(" *  ===== Font: face=%ls size=%u =====\n")
#else
			          _T(" *  ===== Font: face=%s size=%u =====\n")
#endif
			          _T(" *  Do not edit! Autogenerated by MakeGrayFont\n")
					  _T(" */\n")
					  _T("#include \"GrayFont.h\"\n"), szFaceName, height);

		/* Finally output font info */
		OutGrayFont(tm, szFaceName, firstCh, lastCh, fout);

		fprintf(fout, "\t/* ===== width table ===== */\n\t");
		col = 0;
		for (ch = firstCh; ch <= lastCh; ++ch)
		{
#ifdef UNICODE
			// Get current character & width
			GetTextExtentPoint32(hDC, &ch, 1, &charSize);
#else
			TCHAR buf[4];
			int bytes = wctomb(buf, ch);
			// Get current character & width
			GetTextExtentPoint32(hDC, buf, bytes, &charSize);
#endif
			fprintf(fout, "%u,", charSize);
		}
		fprintf(fout, "\n");

		/* Output "font bits" */
		for (row = 0; row < height; ++row)
		{
			OutFontRow(hDC, row, firstCh, lastCh, fout);
		}
		fprintf(fout, "\n};\n");


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
	const TCHAR *szFileName;

	fprintf(stderr, "MakeGrayFont v1.00 (12-Nov-2010)\n");
	if (argc < 3 || argc > 6)
	{
		fprintf(stderr, "usage: GrayFont FontName Size [first] [last] [out]");
		return -1;
	}

	setlocale(LC_CTYPE, _T(""));

	switch (argc)
	{
	case 6:
		szFileName = argv[5];
#ifdef UNICODE
		{
			char szAnsiName[_MAX_PATH];
			char *pszStr = szAnsiName;
			int i, bytes;
			for (i = 0; szFileName[i] != '\0'; ++i)
			{
				bytes = wctomb(pszStr, szFileName[i]);
				if (bytes < 0)
					break;
				pszStr += bytes;
			}
			*pszStr = '\0';
			fout = fopen(szAnsiName, "w");
		}
#else
		fout = fopen(szFileName, "w");
#endif
		if (fout == NULL)
		{
			fprintf(stderr, "can't open %s!\n", argv[2]);
			return -2;
		}
	case 5:
		last = Str2Int(argv[4], 16);
	case 4:
		first = Str2Int(argv[3], 16);
	default:
		;
	}

	height = Str2Int(argv[2], 10);
	if (height < 5)
	{
		fprintf(stderr, "Height must be >= 5!\n");
		return -3;
	}

	/* Get Screen Device Con_T */
	HDC hDC = CreateCompatibleDC(NULL);
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 8;

	LPVOID pBits = NULL;
	HBITMAP hBitmap = CreateGrayDIB(hDC, &pBits, 320, 256);
	HGDIOBJ hOldBitmap = SelectObject(hDC, hBitmap);
	OutFont(hDC, argv[1], height, fout, first, last);
	SelectObject(hDC, hOldBitmap);
	DeleteDC(hDC);

	return 0;
}