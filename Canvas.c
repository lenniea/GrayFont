#include <windows.h>
#include "canvas.h"

BOOL DrawPixel(CANVAS* canvas, int x, int y, COLORREF rgb)
{
	return SetPixel(canvas->hDC, x, y, rgb);
}

#define PERCENT_33		3333
#define PERCENT_67		6667
#define PERCENT_100		10000

COLORREF MixColors(int percent, COLORREF rgb1, COLORREF rgb2)
{
	const int remain = PERCENT_100 - percent;
	const int red = MulDiv(GetRValue(rgb1), percent, PERCENT_100) + MulDiv(GetRValue(rgb2), remain, PERCENT_100);
	const int grn = MulDiv(GetGValue(rgb1), percent, PERCENT_100) + MulDiv(GetGValue(rgb2), remain, PERCENT_100);
	const int blu = MulDiv(GetBValue(rgb1), percent, PERCENT_100) + MulDiv(GetBValue(rgb2), remain, PERCENT_100);
	return RGB(red, grn, blu);
}

void UpdateNearestColors(CANVAS* canvas)
{
	COLORREF rgbBack = canvas->m_gray[GRAY_TRANSPARENT];
	COLORREF rgbFore = canvas->m_gray[GRAY_OPAQUE];
	canvas->m_gray[GRAY_33PERCENT] = MixColors(PERCENT_33, rgbBack, rgbFore);
	canvas->m_gray[GRAY_67PERCENT] = MixColors(PERCENT_67, rgbBack, rgbFore);
}

COLORREF SetForeColor(CANVAS* canvas, COLORREF rgb)
{
	COLORREF rgbOld = SetTextColor(canvas->hDC, rgb);
	canvas->m_gray[GRAY_OPAQUE] = rgb;
	UpdateNearestColors(canvas);
	return rgbOld;
}

COLORREF SetBacklor(CANVAS* canvas, COLORREF rgb)
{
	COLORREF rgbOld = SetBkColor(canvas->hDC, rgb);
	canvas->m_gray[GRAY_TRANSPARENT] = rgb;
	UpdateNearestColors(canvas);
	return rgbOld;
}
