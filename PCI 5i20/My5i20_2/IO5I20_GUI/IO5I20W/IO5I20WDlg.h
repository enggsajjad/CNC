// IO5I20WDlg.h : header file
//

#if !defined(AFX_IO5I20WDLG_H__39CE52B5_321A_4177_AB38_D1905CA50F3A__INCLUDED_)
#define AFX_IO5I20WDLG_H__39CE52B5_321A_4177_AB38_D1905CA50F3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CIO5I20WDlg dialog

class CIO5I20WDlg : public CDialog
{
// Construction
public:
	CIO5I20WDlg(CWnd* pParent = NULL);	// standard constructor
	void FindPciCard(unsigned cardnum);
// Dialog Data
	//{{AFX_DATA(CIO5I20WDlg)
	enum { IDD = IDD_IO5I20W_DIALOG };
	CString	m_status;
	long	m_PortIndex;
	long	m_CardNo;
	CString	m_Value;
	CString	m_DirectionMask;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIO5I20WDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
	
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIO5I20WDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OncmdWrite();
	afx_msg void OncmdRead();
	afx_msg void OncmdTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IO5I20WDLG_H__39CE52B5_321A_4177_AB38_D1905CA50F3A__INCLUDED_)
