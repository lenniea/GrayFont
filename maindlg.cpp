// maindlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <atlmisc.h>
#include <atldlgs.h>

#include "resource.h"

#include "GrayFont.h"
#include "StrParse.h"

#include "maindlg.h"

TCHAR szAppName[MAX_PATH];

CMainDlg::CMainDlg()
{
	m_pBitmap = NULL;
	m_pFont = NULL;
	AtlLoadString(IDR_MAINFRAME, szAppName, MAX_PATH);
}

CMainDlg::~CMainDlg()
{
	if (m_pBitmap != NULL)
	{
		FreeGrayFont(m_pBitmap);
		m_pBitmap = NULL;
	}
}

void ColorRect(HDC hDC, LPRECT pRect, COLORREF rgb)
{
    COLORREF rgbOld = ::SetBkColor(hDC, rgb);
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    ::SetBkColor(hDC, rgbOld);
}

COLORREF CMainDlg::GetColorFromId(UINT id)
{
	TCHAR szText[80];
	COLORREF rgb = ~0U;

	if (GetDlgItemText(id, szText, sizeof(szText)))
	{
		rgb = Hex2Int(szText);
		rgb = RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
	}
	return rgb;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	logfont.lfHeight = 24;
	lstrcpy(logfont.lfFaceName, _T("Arial"));
	m_hFont = CreateFontIndirect(&logfont);

	SetDlgItemText(IDC_EDIT1, _T("0041"));
	m_combo = GetDlgItem(IDC_ZOOM);
	m_combo.AddString(_T("1x"));
	m_combo.AddString(_T("2x"));
	m_combo.AddString(_T("4x"));
	m_combo.AddString(_T("8x"));
	m_combo.SetCurSel(2);
	CheckDlgButton(IDC_ZOOM, TRUE);

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

/////////////////////////////////////////////////////////////////
//
//	Message Handlers
//

LRESULT CMainDlg::OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
    HDC hDC = lpDIS->hDC;
    LPRECT pRect = &lpDIS->rcItem;
    int id = lpDIS->itemID;
	COLORREF rgbBack,rgbText;
	int x,y, length, zoom;
	TCHAR szText[80];
	WCHAR ch;

    switch (lpDIS->CtlID)
    {
    case IDC_PREVIEW:
		rgbBack = GetColorFromId(IDC_BACK);
		rgbText = GetColorFromId(IDC_TEXT);
		x = (pRect->left + pRect->right) >> 1;
		y = (pRect->top + pRect->bottom) >> 1;
		length = GetDlgItemText(IDC_EDIT1, szText, sizeof(szText));
		ch = Hex2Int(szText);
#if 0
		ColorRect(hDC, pRect, rgbBack);
		SetBkColor(hDC, rgbBack);
		SetTextColor(hDC, rgbText);
		if (m_pFont)
		{
			DrawGrayChar(hDC, m_pFont, x - 20, y - 10, ch);
		}
#else
		if (m_pBitmap)
		{
			SetGrayColors(m_pBitmap, rgbBack, rgbText);
		}
		m_combo = GetDlgItem(IDC_ZOOM);
		zoom = m_combo.GetCurSel();
		if (m_pFont)
		{
			DrawGrayFont(hDC, m_pBitmap, m_pFont, ch, x, y, zoom);
		}
#endif
        break;
    case IDC_BACK:
    case IDC_TEXT:
		rgbBack = GetColorFromId(lpDIS->CtlID);
		ColorRect(hDC, pRect, rgbBack);
    default:
        ;
    }
	return 0;
}

COLORREF custColors[16];

void CMainDlg::AppError(UINT id, int error)
{
	TCHAR szError[256];
	TCHAR szFormat[80];
	AtlLoadString(id, szFormat, sizeof(szFormat));
	wsprintf(szError, szFormat, error);
	MessageBox(szError, szAppName, MB_ICONERROR);
}

const TCHAR szFilter[] =  "Gray Fonts (*.gfnt)\0*.gfnt\0All Files (*.*)\0*.*\0";

LRESULT CMainDlg::OnFileOpen(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog dlg(TRUE, _T("bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, m_hWnd);
	if (dlg.DoModal() == IDOK)
	{
		LPCTSTR pathname = dlg.m_ofn.lpstrFile;
		FILE* fp = fopen(pathname, "rb");
		if (fp == NULL)
		{
			AppError(IDS_ERR_OPEN_FILE, -1);
		}
		else
		{
			// Free any previous font loaded
			if (m_pFont)
			{
				free(m_pFont);
			}
			// Determinte size of font file
			fseek(fp, 0L, SEEK_END);
			long filesize = ftell(fp);
			fseek(fp, 0L, SEEK_SET);

			// Allocate font buffer & read in file
			m_pFont = (GRAY_FONT*) malloc(filesize);
			if (m_pFont == NULL)
			{
				AppError(IDS_ERR_ALLOC_MEM, filesize);
			}
			else
			{
				fread(m_pFont, 1, filesize, fp);
				m_pBitmap = InitGrayFont(m_pFont);
				InvalidateRect(NULL, TRUE);
			}
			fclose(fp);
		}
	}
	return 0;
}

LRESULT CMainDlg::OnClickedColor(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CColorDialog dlg;
    dlg.m_cc.rgbResult = GetColorFromId(wID);
    dlg.m_cc.Flags |= CC_SOLIDCOLOR | CC_FULLOPEN | CC_RGBINIT;
    dlg.m_cc.lpCustColors = custColors;
    if (dlg.DoModal() == IDOK)
    {
		COLORREF rgb = dlg.m_cc.rgbResult;
		// Convert RGB to 6-digit COLORREF (bbggrr) value
		TCHAR szHex[10];
		wsprintf(szHex, _T("%02x%02x%02x"), GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
        SetDlgItemText(wID, szHex);
		InvalidateRect(NULL);
    }
	return 0;
}

LRESULT CMainDlg::OnClickedZoom(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	InvalidateRect(NULL);
	return 0;
}

LRESULT CMainDlg::OnEditChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	InvalidateRect(NULL);
	return 0;
}
