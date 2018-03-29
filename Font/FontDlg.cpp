
// FontDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Font.h"
#include "FontDlg.h"
#include "afxdialogex.h"

#include <string>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define MAX_FONT_SIZE		64


// CFontDlg dialog

static ULONG_PTR s_pGDIplusToken = NULL;


CFontDlg::CFontDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFontDlg::IDD, pParent)
	, m_s32EditFontSize(24)
	, m_csEditInput(_T("※"))
	, m_s32RealWidth(24)
	, m_s32RealHeight(24)
	, m_s32FontStart(0)
	, m_s32FontEnd(0)
	, m_boIsBold(FALSE)
	, m_boIsItalic(FALSE)
	, m_boIsUnderLine(FALSE)
	, m_boIsStrikeOut(FALSE)
{
	memset(&m_stDraw, 0, sizeof(StDraw));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFontDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComboFont);
	DDX_Text(pDX, IDC_EDIT_FONT_SIZE, m_s32EditFontSize);
	DDV_MinMaxInt(pDX, m_s32EditFontSize, 0, MAX_FONT_SIZE);
	DDX_Text(pDX, IDC_EDIT_INPUT, m_csEditInput);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_objEditInput);
	DDX_Text(pDX, IDC_EDIT_REAL_WIDTH, m_s32RealWidth);
	DDV_MinMaxInt(pDX, m_s32RealWidth, 0, MAX_FONT_SIZE);
	DDX_Text(pDX, IDC_EDIT_REAL_HEIGHT, m_s32RealHeight);
	DDV_MinMaxInt(pDX, m_s32RealHeight, 0, MAX_FONT_SIZE);
	DDX_Text(pDX, IDC_EDIT_Font_Start, m_s32FontStart);
	DDX_Text(pDX, IDC_EDIT_Font_End, m_s32FontEnd);
	DDX_Check(pDX, IDC_CHECK_Bold, m_boIsBold);
	DDX_Check(pDX, IDC_CHECK_Italic, m_boIsItalic);
	DDX_Check(pDX, IDC_CHECK_UnderLine, m_boIsUnderLine);
	DDX_Check(pDX, IDC_CHECK_StrikeOut, m_boIsStrikeOut);
}

BEGIN_MESSAGE_MAP(CFontDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CREATE, &CFontDlg::OnBnClickedBtnCreate)
	ON_BN_CLICKED(IDC_BTN_NEW, &CFontDlg::OnBnClickedBtnNew)
	ON_BN_CLICKED(IDC_BTN_Preview, &CFontDlg::OnBnClickedBtnPreview)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_LOADBMP, &CFontDlg::OnBnClickedBtnLoadbmp)
	ON_BN_CLICKED(IDC_BTN_Create_File, &CFontDlg::OnBnClickedBtnCreateFile)
END_MESSAGE_MAP()


static int s_s32TotalCnt = -1;
BOOL CALLBACK EnumFonts(CONST LOGFONT* lplf, CONST TEXTMETRIC *lptm,DWORD dwType,LPARAM lprarm)
{
	CString far *pFontName = (CString far*)lprarm;
	s_s32TotalCnt++;
	pFontName[s_s32TotalCnt] = lplf->lfFaceName;
	return true;
}

void CFontDlg::DrawInit(HWND hWnd)
{
#define CHECK_HANDLE(x)\
	if(x == NULL)\
	{\
	goto err;\
	}
	if(hWnd == NULL)
	{
		return;
	}

	m_stDraw.m_hWnd = hWnd;
	m_stDraw.m_hDC = ::GetDC(hWnd);
	CHECK_HANDLE(m_stDraw.m_hDC);


	RECT stRect;
	::GetClientRect(hWnd, &stRect);
	int s32Height;
	int s32Width;
	stRect.bottom = s32Height = stRect.bottom - stRect.top;
	stRect.right = s32Width = stRect.right - stRect.left;
	stRect.top = stRect.left = 0;
	
	/* memory dc */
	m_stDraw.m_hMemDC = CreateCompatibleDC(m_stDraw.m_hDC);
	CHECK_HANDLE(m_stDraw.m_hMemDC);
	m_stDraw.m_hBmp = CreateCompatibleBitmap(m_stDraw.m_hDC, s32Width, s32Height);
	CHECK_HANDLE(m_stDraw.m_hBmp);
	m_stDraw.m_hOldBmp = (HBITMAP)SelectObject(m_stDraw.m_hMemDC, m_stDraw.m_hBmp);
	CHECK_HANDLE(m_stDraw.m_hOldBmp);

	/* memory transfer dc */
	m_stDraw.m_hTransferMemDC = CreateCompatibleDC(m_stDraw.m_hDC);
	CHECK_HANDLE(m_stDraw.m_hMemDC);
	m_stDraw.m_hTransferBmp = CreateCompatibleBitmap(m_stDraw.m_hDC, s32Width, s32Height);
	CHECK_HANDLE(m_stDraw.m_hBmp);
	m_stDraw.m_hTransferOldBmp = (HBITMAP)SelectObject(m_stDraw.m_hTransferMemDC, m_stDraw.m_hTransferBmp);
	CHECK_HANDLE(m_stDraw.m_hOldBmp);


	SelectObject(m_stDraw.m_hMemDC, GetStockObject(DC_PEN));
	SelectObject(m_stDraw.m_hMemDC, GetStockObject(DC_BRUSH));

	SelectObject(m_stDraw.m_hTransferMemDC, GetStockObject(DC_PEN));
	SelectObject(m_stDraw.m_hTransferMemDC, GetStockObject(DC_BRUSH));

	SetDCPenColor(m_stDraw.m_hTransferMemDC, RGB(255, 255, 255));

	DWORD dwRectSize = s32Height / MAX_FONT_SIZE;

	DWORD dwSize = (s32Height / dwRectSize) * (s32Width / dwRectSize);
	m_stDraw.m_pBoolPrint = new bool [dwSize];
#if 0
	GetObject(m_stDraw.m_hBmp, sizeof(BITMAP), &(m_stDraw.m_stBmpInfo));

	m_stDraw.m_pPic = new char[dwSize];
	CHECK_HANDLE(m_stDraw.m_pPic);

	memset(m_stDraw.m_pPic, BACKGROUND, dwSize);
#endif
	m_stDraw.m_u16Height = s32Height;
	m_stDraw.m_u16Width = s32Width;
	m_stDraw.m_u16RectSize = dwRectSize;
	m_stDraw.m_u16RealHeight = s32Height / m_stDraw.m_u16RectSize;
	m_stDraw.m_u16RealWidth = s32Width / m_stDraw.m_u16RectSize;
	m_stDraw.m_u16ValidHeight = m_s32RealWidth;
	m_stDraw.m_u16ValidWidth = m_s32RealWidth;
	

	m_stDraw.m_boIsInit = true;

	DrawClear();

	return;
err:
	DrawDestroy();
}

void CFontDlg::DrawDestroy()
{

	if(m_stDraw.m_pPic)
	{
		delete [] m_stDraw.m_pPic;
	}

	if (m_stDraw.m_pBoolPrint)
	{
		delete [] m_stDraw.m_pBoolPrint;
	}
	

	if(m_stDraw.m_hBmp)
	{
		DeleteObject(m_stDraw.m_hBmp);
	}
	if(m_stDraw.m_hOldBmp)
	{
		if(m_stDraw.m_hMemDC)
		{
			SelectObject(m_stDraw.m_hMemDC, m_stDraw.m_hOldBmp);
			DeleteObject(m_stDraw.m_hMemDC);
			m_stDraw.m_hMemDC = NULL;
		}
	}

	if(m_stDraw.m_hTransferBmp)
	{
		DeleteObject(m_stDraw.m_hTransferBmp);
	}
	if(m_stDraw.m_hTransferOldBmp)
	{
		if(m_stDraw.m_hTransferMemDC)
		{
			SelectObject(m_stDraw.m_hTransferMemDC, m_stDraw.m_hTransferOldBmp);
			DeleteObject(m_stDraw.m_hTransferMemDC);
			m_stDraw.m_hTransferMemDC = NULL;
		}
	}


	if(m_stDraw.m_hDC)
	{
		::ReleaseDC(m_stDraw.m_hWnd, m_stDraw.m_hDC);
	}

	memset(&m_stDraw, 0, sizeof(StDraw));

}
void CFontDlg::DrawClear()
{
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}
	memset(m_stDraw.m_pBoolPrint, 0, m_stDraw.m_u16RealHeight * m_stDraw.m_u16RealWidth * sizeof(bool));
#if 0
	SetDCBrushColor(m_stDraw.m_hMemDC, RGB(255,255,255));
	SetDCPenColor(m_stDraw.m_hMemDC, RGB(0,0,0));
	uint16_t u16HeightTmp = 0; 
	while (u16HeightTmp < m_stDraw.m_u16Height)
	{		
		uint16_t u16WidthTmp = 0; 
		while(u16WidthTmp < m_stDraw.m_u16Width)
		{
			Rectangle(m_stDraw.m_hMemDC, u16WidthTmp, u16HeightTmp, u16WidthTmp + 8, u16HeightTmp + 8);
			u16WidthTmp += 8;
		}
		u16HeightTmp += 8;
	}
#else
	DrawPic();
#endif
}
void CFontDlg::DrawFlushDC()
{
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}
	BitBlt(m_stDraw.m_hDC, 0, 0, m_stDraw.m_u16Width, m_stDraw.m_u16Height, 
		m_stDraw.m_hMemDC, 0, 0, SRCCOPY);

}

void CFontDlg::DrawPic()
{
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}
	for (uint16_t i = 0; i < m_stDraw.m_u16RealHeight; i++)
	{
		uint16_t u16HeightTmp = i * m_stDraw.m_u16RectSize;
		for (uint16_t j = 0; j < m_stDraw.m_u16RealWidth; j++)
		{
			uint16_t u16WidthTmp = j * m_stDraw.m_u16RectSize;
			bool boIsPrint = *(m_stDraw.m_pBoolPrint + i * m_stDraw.m_u16RealWidth + j);
			if ((i < m_stDraw.m_u16ValidHeight) && (j < m_stDraw.m_u16ValidWidth))
			{
				if (boIsPrint)
				{
					SetDCBrushColor(m_stDraw.m_hMemDC, RGB(255,0,0));
				}
				else
				{
					SetDCBrushColor(m_stDraw.m_hMemDC, RGB(255,255,255));
				}
			}
			else
			{
				if (boIsPrint)
				{
					SetDCBrushColor(m_stDraw.m_hMemDC, RGB(0,255,255));
				}
				else
				{
					SetDCBrushColor(m_stDraw.m_hMemDC, RGB(255,255,0));
				}

			}
			Rectangle(m_stDraw.m_hMemDC, u16WidthTmp, u16HeightTmp, 
				u16WidthTmp + m_stDraw.m_u16RectSize, u16HeightTmp + m_stDraw.m_u16RectSize);
		}
	}
	BitBlt(m_stDraw.m_hDC, 0, 0, m_stDraw.m_u16Width, m_stDraw.m_u16Height, 
		m_stDraw.m_hMemDC, 0, 0, SRCCOPY);
}

int BufToBmp(FILE *pFile, unsigned char *Buf, int s32Height, int s32Width, int s32BufOffset, bool boIsYP)
{

	BITMAPFILEHEADER stBmpHeader = { 0 };

	BITMAPINFOHEADER stInfoHeader = { 0 };

	stBmpHeader.bfType = 0x4D42;

	stBmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	stInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	stInfoHeader.biWidth = s32Width;
	stInfoHeader.biHeight = s32Height;
	stInfoHeader.biPlanes = 1;
	stInfoHeader.biBitCount = 24;
	stInfoHeader.biCompression = 0;
	stInfoHeader.biSizeImage = (((s32Width * 3 + 3) / 4) * 4) * s32Height + 2;/* 54 ----> 56 */

	stInfoHeader.biXPelsPerMeter = stInfoHeader.biYPelsPerMeter = 0x1E40;

	unsigned char *pBmpBuf = (unsigned char *)calloc(1, stInfoHeader.biSizeImage);

	if (pBmpBuf == NULL)
	{
		return -1;
	}

	if (!boIsYP)
	{
		int s32WidthDestTmp = ((s32Width * 3 + 3) / 4) * 4;
		int s32WidthSrcTmp = s32Width * s32BufOffset;
		unsigned char *pTmpDest = pBmpBuf + s32WidthDestTmp * (s32Height - 1);
		unsigned char *pTmpSrc = Buf;
		for (int i = 0; i < s32Height; i++)
		{
			for (int j = 0; j < s32Width; j++)
			{
				memcpy(pTmpDest + j * 3, pTmpSrc + j * s32BufOffset, 3);
			}

			pTmpDest -= s32WidthDestTmp;
			pTmpSrc += s32WidthSrcTmp;
		}
	}
	else
	{
		int s32WidthDestTmp = ((s32Width * 3 + 3) / 4) * 4;
		int s32WidthSrcTmp = s32Width * s32BufOffset;
		unsigned char *pTmpDest = pBmpBuf;
		unsigned char *pTmpSrc = Buf;
		for (int i = 0; i < s32Height; i++)
		{
			for (int j = 0; j < s32Width; j++)
			{
				memcpy(pTmpDest + j * 3, pTmpSrc + j * s32BufOffset, 3);
			}

			pTmpDest += s32WidthDestTmp;
			pTmpSrc += s32WidthSrcTmp;
		}
	}

	fseek(pFile, 0, SEEK_SET);

	fwrite(&stBmpHeader, 1, sizeof(BITMAPFILEHEADER), pFile);
	fwrite(&stInfoHeader, 1, sizeof(BITMAPINFOHEADER), pFile);
	fwrite(pBmpBuf, 1, stInfoHeader.biSizeImage, pFile);

	free(pBmpBuf);

	return 0;
}

wstring Convert(string csStrSrc)
{
	int s32SrcLen = csStrSrc.length();

	int s32UnicodeLen = MultiByteToWideChar(CP_ACP, 0, csStrSrc.c_str(), -1, NULL, 0);

	wchar_t * pUnicode;
	pUnicode = new wchar_t[s32UnicodeLen + 1];

	memset(pUnicode, 0, (s32UnicodeLen + 1) * sizeof(wchar_t));

	MultiByteToWideChar(CP_ACP, 0, csStrSrc.c_str(), -1, (LPWSTR)pUnicode,
		s32UnicodeLen);
	wstring csStrDest = pUnicode;
	delete[] pUnicode;
	return csStrDest;
}

#if 1/* left to right, up to down, MSB is left bit */
void CFontDlg::DrawString(bool boIsView, bool boIsCreatFile)
{
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}
	UpdateData();
	int s32Len = m_csEditInput.GetLength();
	if ((s32Len > 2) || (s32Len == 0))
	{
		return;
	}
	int s32Size = m_s32EditFontSize;// % 65;
	int16_t s16MechineCode = 0;
	char *pStr = m_csEditInput.GetBuffer();
	if (s32Len == 2)
	{
		s16MechineCode = ((((int16_t)pStr[0]) & 0xFF) << 8) | (((int16_t)pStr[1]) & 0xFF);
		if (s16MechineCode > 0)
		{
			m_csEditInput.ReleaseBuffer();
			return;	
		}
	}
	else
	{
		s16MechineCode = (((int16_t)pStr[0]) & 0xFF);
	}
	m_csEditInput.ReleaseBuffer();


	CString csStr;
	m_ComboFont.GetLBText(m_ComboFont.GetCurSel(), csStr);
	int s32Weight = m_boIsBold ? FW_BOLD : FW_THIN;
	HFONT hFont = CreateFont(s32Size, 0, 0, 0, 
		s32Weight, m_boIsItalic, m_boIsUnderLine, m_boIsStrikeOut, 
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, csStr.GetBuffer());
	csStr.ReleaseBuffer();

	HFONT hFontOld = (HFONT)SelectObject(m_stDraw.m_hTransferMemDC, hFont);

	int s32Width = 3 - s32Len;
	s32Width = s32Size / s32Width;
	if (0)
	{
		RECT stRect = {0, 0, s32Size, s32Width};
		Rectangle(m_stDraw.m_hTransferMemDC, 0, 0, s32Size, s32Size);
		DrawText(m_stDraw.m_hTransferMemDC, pStr, -1,&stRect, DT_NOCLIP);
		m_csEditInput.ReleaseBuffer();
		SelectObject(m_stDraw.m_hTransferMemDC, hFontOld);
	
	}
	else
	{
		Graphics graphics(m_stDraw.m_hTransferMemDC);

		graphics.Clear(Color(ARGB(0 | 0xFFFFFFFF)));
		//{
		//	SolidBrush csBrushTmp(Color(0xFFFFFFFF));
		//	GraphicsPath csPath()
		//	graphics.FillPath(&csBrushTmp, )
		//}


		Gdiplus::Font csFont(m_stDraw.m_hTransferMemDC, hFont);
		Gdiplus::PointF csPoint(0.0, 0.0);

		Gdiplus::RectF csRect(0.0, 0.0, s32Size, s32Width);

		SolidBrush csBrush(Color(0xFF000000));

		Gdiplus::StringFormat csStringFromat;
		csStringFromat.SetAlignment(StringAlignmentCenter);
		//csStringFromat.SetLineAlignment(StringAlignmentCenter);

		graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
		//graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

		wstring csWStr = Convert(pStr);

		//graphics.DrawString(csWStr.c_str(), csWStr.length(), &csFont, csPoint, &csBrush);
		graphics.DrawString(csWStr.c_str(), csWStr.length(), &csFont, csRect, &csStringFromat, &csBrush);


	}

#if 1	
#if 0
	csStr.Format("const unsigned char c_u8Font%d_%d_%02x%02x[%d] = \r\n{\r\n\t", 
		s32Size, s32Width, (s16MechineCode >> 8 & 0xFF), (s16MechineCode & 0xFF),
		s32Size * s32Width / 8);
#else
	csStr.Format("const StCHS_%d stCHS_%d_%d_%02X%02X = \r\n{\r\n\t0x%02X%02X,\r\n\t{\r\n\t\t",
		s32Size,
		s32Size, s32Width, 
		(s16MechineCode >> 8 & 0xFF), (s16MechineCode & 0xFF), 
		(s16MechineCode >> 8 & 0xFF), (s16MechineCode & 0xFF));
#endif
	CString csStrTmp;
	s32Len = (s32Width + 7) / 8;
	int *pTmp = new int [s32Len];
	int s32Count = 0;
	char *pRGBBuf = new char[s32Size * s32Width * 3];
	char *pRGBTmp = pRGBBuf;
	memset(m_stDraw.m_pBoolPrint, 0, m_stDraw.m_u16RealHeight * m_stDraw.m_u16RealWidth * sizeof(bool));
	for (int i = 0; i < s32Size; i++)
	{
		memset(pTmp, 0, sizeof(int) * s32Len);
		for (int j = 0; j < s32Width; j++)
		{
			uint32_t u32Index = j / 8;

			uint32_t u32Tmp = GetPixel(m_stDraw.m_hTransferMemDC, j, i);
			u32Tmp &= 0xFFFFFF;
			if (u32Tmp != 0xFFFFFF)
			{
				*(m_stDraw.m_pBoolPrint + i * m_stDraw.m_u16RealWidth + j) = true;
				pTmp[u32Index] |= (1 << (7 - (j % 8)));
			}
			pRGBTmp[0] = (char)(u32Tmp >> 16);
			pRGBTmp[1] = (char)(u32Tmp >> 8);
			pRGBTmp[2] = (char)(u32Tmp >> 0);
			pRGBTmp += 3;
		}
		for (int j = 0; j < s32Len; j++)
		{
			csStrTmp.Format("0x%02x, ", pTmp[j]);
			csStr += csStrTmp;
			if ((s32Count != 0) && ((s32Count & 0x0F) == 0x0F))
			{
				//csStr += "\r\n\t\t";
			}
			s32Count++;
		}
		csStr += "\r\n\t\t";
	}
	csStrTmp.Format("\r\n\t},\r\n};\t/* %s */\r\n", pStr);
	csStr += csStrTmp;
	if (boIsView)
	{
		GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(csStr);
	}
	if (boIsCreatFile)
	{
		FILE *pFile = fopen("my_font.c", "ab+");
		if (pFile != NULL)
		{
			fwrite(csStr.GetBuffer(), csStr.GetLength(), 1, pFile);
			fclose(pFile);
		}
	}
	if (1)
	{
		FILE *pFile = fopen("g:\\123.bmp", "wb+");
		if (pFile != NULL)
		{
			BufToBmp(pFile, (unsigned char *)pRGBBuf, s32Size, s32Width, 3, false);
			fclose(pFile);
		}
	}
	delete[] pRGBBuf;

	delete [] pTmp;
#endif
	DrawPic();
	DeleteObject(hFont);
}
#else	/* up to down, left to write, LSB is up bit */
void CFontDlg::DrawString(bool boIsView, bool boIsCreatFile)
{
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}
	UpdateData();
	int s32Len = m_csEditInput.GetLength();
	if ((s32Len > 2) || (s32Len == 0))
	{
		return;
	}
	int s32Size = m_s32EditFontSize % 64;
	int16_t s16MechineCode = 0;
	char *pStr = m_csEditInput.GetBuffer();
	if (s32Len == 2)
	{
		s16MechineCode = ((((int16_t)pStr[0]) & 0xFF) << 8) | (((int16_t)pStr[1]) & 0xFF);
		if (s16MechineCode > 0)
		{
			m_csEditInput.ReleaseBuffer();
			return;
		}
	}
	else
	{
		s16MechineCode = (((int16_t)pStr[0]) & 0xFF);
	}
	m_csEditInput.ReleaseBuffer();


	CString csStr;
	m_ComboFont.GetLBText(m_ComboFont.GetCurSel(), csStr);
	int s32Weight = m_boIsBold ? FW_BOLD : FW_THIN;
	HFONT hFont = CreateFont(s32Size, 0, 0, 0,
		s32Weight, m_boIsItalic, m_boIsUnderLine, m_boIsStrikeOut,
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, csStr.GetBuffer());
	csStr.ReleaseBuffer();

	HFONT hFontOld = (HFONT)SelectObject(m_stDraw.m_hTransferMemDC, hFont);

	int s32Width = 3 - s32Len;
	s32Width = s32Size / s32Width;
	RECT stRect = { 0, 0, s32Size, s32Width };
	Rectangle(m_stDraw.m_hTransferMemDC, 0, 0, s32Size, s32Size);
	DrawText(m_stDraw.m_hTransferMemDC, pStr, -1, &stRect, DT_NOCLIP);
	m_csEditInput.ReleaseBuffer();
	SelectObject(m_stDraw.m_hTransferMemDC, hFontOld);

#if 1	
#if 0
	csStr.Format("const unsigned char c_u8Font%d_%d_%02x%02x[%d] = \r\n{\r\n\t",
		s32Size, s32Width, (s16MechineCode >> 8 & 0xFF), (s16MechineCode & 0xFF),
		s32Size * s32Width / 8);
#else
	csStr.Format("const StCHS_16 stCHS_%d_%d_%02X%02X = \r\n{\r\n\t0x%02X%02X,\r\n\t{\r\n\t\t",
		s32Size, s32Width,
		(s16MechineCode >> 8 & 0xFF), (s16MechineCode & 0xFF),
		(s16MechineCode >> 8 & 0xFF), (s16MechineCode & 0xFF));
#endif
	CString csStrTmp;
	s32Len = (s32Size + 7) / 8;
	int *pTmp = new int[s32Len];
	int s32Count = 0;
	memset(m_stDraw.m_pBoolPrint, 0, m_stDraw.m_u16RealHeight * m_stDraw.m_u16RealWidth * sizeof(bool));
	for (int i = 0; i < s32Width; i++)
	{
		memset(pTmp, 0, sizeof(int) * s32Len);
		for (int j = 0; j < s32Size; j++)
		{
			uint32_t u32Index = j / 8;

			uint32_t u32Tmp = GetPixel(m_stDraw.m_hTransferMemDC, i, j);
			u32Tmp &= 0xFFFFFF;
			if (u32Tmp != 0xFFFFFF)
			{
				*(m_stDraw.m_pBoolPrint + j * m_stDraw.m_u16RealWidth + i) = true;
				pTmp[u32Index] |= (1 << (j % 8));
			}
		}
		for (int j = 0; j < s32Len; j++)
		{
			csStrTmp.Format("0x%02x, ", pTmp[j]);
			csStr += csStrTmp;
			if ((s32Count != 0) && ((s32Count & 0x0F) == 0x0F))
			{
				csStr += "\r\n\t\t";
			}
			s32Count++;
		}
	}
	csStrTmp.Format("\r\n\t},\r\n};\t/* %s */\r\n", pStr);
	csStr += csStrTmp;
	if (boIsView)
	{
		GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(csStr);
	}
	if (boIsCreatFile)
	{
		FILE *pFile = fopen("g:\\font.c", "ab+");
		if (pFile != NULL)
		{
			fwrite(csStr.GetBuffer(), csStr.GetLength(), 1, pFile);
			fclose(pFile);
		}
	}
	delete[] pTmp;
#endif
	DrawPic();
	DeleteObject(hFont);
}
#endif

void CFontDlg::DrawLoadBmp(CString csBmpName)
{
	HBITMAP hBitmap = (HBITMAP)::LoadImage(NULL, csBmpName.GetBuffer(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hBitmap = hBitmap;
	
	if (hBitmap == NULL)
	{
		return;
	}
	BITMAP bmpTmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmpTmp);
	csBmpName.ReleaseBuffer();

	HDC hMemdc = CreateCompatibleDC(m_stDraw.m_hDC);
	HGDIOBJ hOld = SelectObject(hMemdc, hBitmap);
	StretchBlt(m_stDraw.m_hTransferMemDC, 0, 0, m_stDraw.m_u16ValidWidth, m_stDraw.m_u16ValidHeight,
		hMemdc, 0, 0, bmpTmp.bmWidth, bmpTmp.bmHeight,SRCCOPY);

	memset(m_stDraw.m_pBoolPrint, 0, m_stDraw.m_u16RealHeight * m_stDraw.m_u16RealWidth * sizeof(bool));
	for (int i = 0; i < m_stDraw.m_u16ValidHeight; i++)
	{
		for (int j = 0; j < m_stDraw.m_u16ValidWidth; j++)
		{
			uint32_t u32Tmp = GetPixel(m_stDraw.m_hTransferMemDC, j, i);
			
			uint32_t R = u32Tmp >> 16;
			R &= 0xFF;
			uint32_t G = u32Tmp >> 8;
			G &= 0xFF;
			uint32_t B = u32Tmp;
			B &= 0xFF;
			if ((R > 127) && (G > 127) &&(B > 127))
			{
				*(m_stDraw.m_pBoolPrint + i * m_stDraw.m_u16RealWidth + j) = true;
			}
		}
	}

	SelectObject(hMemdc, hOld);
	DeleteDC(hMemdc);

	DrawPic();
}


// CFontDlg message handlers

BOOL CFontDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	GdiplusStartupInput stGDIplusStartupInput;

	GdiplusStartup(&s_pGDIplusToken, &stGDIplusStartupInput, NULL);


	CRect csRectTmp;
	GetDlgItem(IDC_PIC)->GetWindowRect(csRectTmp);
#if 0
	uint32_t u32Tmp = csRectTmp.bottom - csRectTmp.top;
	u32Tmp = (u32Tmp + 7) / 8 * 8;
	csRectTmp.bottom = csRectTmp.top + u32Tmp;
	u32Tmp = csRectTmp.right - csRectTmp.left;
	u32Tmp = (u32Tmp + 7) / 8 * 8;
	csRectTmp.right = csRectTmp.left + u32Tmp;
#endif
	csRectTmp.top = csRectTmp.left;
	csRectTmp.bottom = csRectTmp.top + 512;
	csRectTmp.right = csRectTmp.left + 512;

	GetDlgItem(IDC_PIC)->SetWindowPos(&wndTop, csRectTmp.left, csRectTmp.top, 
		csRectTmp.Width(), csRectTmp.Height(), SWP_SHOWWINDOW);

	DrawInit(GetDlgItem(IDC_PIC)->GetSafeHwnd());
	m_stDraw.m_stRect = csRectTmp;

	// TODO: Add extra initialization here
	s_s32TotalCnt = -1;
	CString csFontName[1024];
	CPaintDC dc(this);
	int s32CurSel = 0;
	::EnumFonts(dc.m_hDC,NULL,(FONTENUMPROC)EnumFonts,(LPARAM)csFontName);
	for(int i = 0; i < s_s32TotalCnt; i++)
	{
		m_ComboFont.AddString(csFontName[i]);
	}
	s32CurSel = m_ComboFont.FindString(-1, "宋体");

	m_objEditInput.SetLimitText(2);
#if 0
	for(int i = 0; i < s_s32TotalCnt; i++)
	{
		CString csStr;
		m_ComboFont.GetLBText(i, csStr);
		if (csStr == "宋体")
		{
			s32CurSel = i;
		}
	}
#endif
	m_ComboFont.SetCurSel(s32CurSel);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFontDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		DrawFlushDC();
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFontDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CFontDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	DrawDestroy();

	GdiplusShutdown(s_pGDIplusToken);

	return CDialogEx::DestroyWindow();
}


void CFontDlg::OnBnClickedBtnCreate()
{
	// TODO: Add your control notification handler code here
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}

	CString csStr;
	int s32Len = (m_stDraw.m_u16ValidWidth + 7) / 8;
	csStr.Format("const unsigned char Bmp2Bit[%d] = \r\n{\r\n", 
				m_stDraw.m_u16ValidHeight * s32Len);
	CString csStrTmp;
	int *pTmp = new int [s32Len];
	for (int i = 0; i < m_stDraw.m_u16ValidHeight; i++)
	{
		memset(pTmp, 0, sizeof(int) * s32Len);
		for (int j = 0; j < m_stDraw.m_u16ValidWidth; j++)
		{
			uint32_t u32Index = j / 8;
			bool boIsFlag = *(m_stDraw.m_pBoolPrint + i * m_stDraw.m_u16RealWidth + j);
			if (boIsFlag)
			{
				pTmp[u32Index] |= (1 << (7 - (j % 8)));
			}
		}
		csStr += "\t";
		for (int j = 0; j < s32Len; j++)
		{
			csStrTmp.Format("0x%02x, ", pTmp[j]);
			csStr += csStrTmp;
		}
		csStr += "\r\n";
	}
	csStrTmp.Format("};\r\n");
	csStr += csStrTmp;
	GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(csStr);
	delete [] pTmp;

}


void CFontDlg::OnBnClickedBtnNew()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_stDraw.m_u16ValidHeight = m_s32RealHeight;
	m_stDraw.m_u16ValidWidth = m_s32RealWidth;
	DrawClear();
}


void CFontDlg::OnBnClickedBtnPreview()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	DrawString(true);
}

CPoint s_stPointOld;
bool s_boIsMouseDown = false;
void CFontDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!m_stDraw.m_boIsInit)
	{
		return;
	}
	if (s_stPointOld == point)
	{
		if (PtInRect(&m_stDraw.m_stRect, point))
		{
			int s32XPos = (point.x - m_stDraw.m_stRect.left) / m_stDraw.m_u16RectSize;
			int s32YPos = (point.y - m_stDraw.m_stRect.top) / m_stDraw.m_u16RectSize;

			bool *pIsFlag = m_stDraw.m_pBoolPrint + s32YPos * m_stDraw.m_u16RealWidth + s32XPos;
			*pIsFlag = !(*pIsFlag);
		}
	}
	else
	{
		for(int j = s_stPointOld.y; j < point.y; j += 8)
		for(int i = s_stPointOld.x; i < point.x; i += 8)
		{
			CPoint csPointTmp(i, j);
			if (PtInRect(&m_stDraw.m_stRect, csPointTmp))
			{
				int s32XPos = (i - m_stDraw.m_stRect.left) / m_stDraw.m_u16RectSize;
				int s32YPos = (j - m_stDraw.m_stRect.top) / m_stDraw.m_u16RectSize;

				bool *pIsFlag = m_stDraw.m_pBoolPrint + s32YPos * m_stDraw.m_u16RealWidth + s32XPos;
				*pIsFlag = !(*pIsFlag);
			}
		}

	}
	DrawPic();
	s_boIsMouseDown = false;
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CFontDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	s_stPointOld = point;
	s_boIsMouseDown = true;
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CFontDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnMouseMove(nFlags, point);
}


void CFontDlg::OnBnClickedBtnLoadbmp()
{
	// TODO: Add your control notification handler code here
	
	CFileDialog *lpszOpenFile;
	lpszOpenFile = new CFileDialog(TRUE, "", "", OFN_FILEMUSTEXIST |OFN_HIDEREADONLY , "文件类型(*.bmp)|*.bmp||");
	CString csGetName;
	if(lpszOpenFile->DoModal() == IDOK)
	{
		csGetName = lpszOpenFile->GetPathName();
	}
	delete lpszOpenFile;
	DrawLoadBmp(csGetName);
}


void CFontDlg::OnBnClickedBtnCreateFile()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	for (int i = m_s32FontStart; i < m_s32FontEnd; i++)
	{
		uint8_t u8CharArr[2] = {i, 0};
		m_csEditInput = u8CharArr;
		UpdateData(FALSE);
		DrawString(true, true);
	}

}
