
// FontDlg.h : header file
//

#pragma once
#include "afxwin.h"

#define BACKGROUND	0xBF

typedef struct _STDRAW_
{
	HWND m_hWnd;
	HDC m_hDC;

	HDC m_hMemDC;
	HBITMAP m_hBmp;
	HBITMAP m_hOldBmp;

	HDC m_hTransferMemDC;
	HBITMAP m_hTransferBmp;
	HBITMAP m_hTransferOldBmp;

	BITMAP m_stBmpInfo;
	bool m_boIsInit;
	char *m_pPic;
	bool *m_pBoolPrint;
	unsigned short m_u16Width;
	unsigned short m_u16Height;
	unsigned short m_u16RealWidth;
	unsigned short m_u16RealHeight;
	unsigned short m_u16ValidWidth;
	unsigned short m_u16ValidHeight;


	unsigned short m_u16RectSize;
	RECT m_stRect;
}StDraw;


// CFontDlg dialog
class CFontDlg : public CDialogEx
{
// Construction
public:
	CFontDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FONT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_ComboFont;
	afx_msg void OnBnClickedBtnCreate();
	int m_s32EditFontSize;
	CString m_csEditInput;
	CEdit m_objEditInput;

public:
	virtual BOOL DestroyWindow();

public:
	StDraw m_stDraw;
	void DrawInit(HWND hWnd);
	void DrawDestroy();
	void DrawClear();
	void DrawFlushDC();
	void DrawPic();
	void DrawString(bool boIsView = false, bool boIsCreateFile = false);
	void DrawLoadBmp(CString csBmpName);
	int m_s32RealWidth;
	int m_s32RealHeight;
	afx_msg void OnBnClickedBtnNew();
	afx_msg void OnBnClickedBtnPreview();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnLoadbmp();
	int m_s32FontStart;
	int m_s32FontEnd;
	afx_msg void OnBnClickedBtnCreateFile();
	BOOL m_boIsBold;
	BOOL m_boIsItalic;
	BOOL m_boIsUnderLine;
	BOOL m_boIsStrikeOut;
};
