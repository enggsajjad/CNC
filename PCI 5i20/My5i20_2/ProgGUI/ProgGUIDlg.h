// ProgGUIDlg.h : header file
//

#if !defined(AFX_PROGGUIDLG_H__25094C23_2A8D_4897_A3BC_25E4BC9201FD__INCLUDED_)
#define AFX_PROGGUIDLG_H__25094C23_2A8D_4897_A3BC_25E4BC9201FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CProgGUIDlg dialog

class CProgGUIDlg : public CDialog
{
// Construction
public:
	CProgGUIDlg(CWnd* pParent = NULL);	// standard constructor
	void ledon(void);
	void ledoff(void);
	void writeenable(void);
	void writedisable(void);
	void programenable(void);
	void programdisable(void);
	//void writedata8(unsigned char b);
	//void outdata8wswap(unsigned char thebyte);
	//void outdata8noswap(unsigned char thebyte);
	void sendcompletionclocks(void);
	int programdunq(void);
	signed wait4dun(void);
	void delay100us(void);
	long filelengthq(FILE *f);
	unsigned int saybytesfromfile(FILE *f, long fromoffset, unsigned int numbytes);
	unsigned int readbytesfromfile(FILE *f, long fromoffset, unsigned int numbytes, unsigned char *bufptr);
	void sayfileinfo(signed long *infobaseptr);
	void selectfiletype(void);
	void programfpga(void);
	void FindPciCard(unsigned cardnum);
	void cleanup(void);

// Dialog Data
	//{{AFX_DATA(CProgGUIDlg)
	enum { IDD = IDD_PROGGUI_DIALOG };
	long	m_CardNo;
	CString	m_status;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgGUIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CProgGUIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProgramming();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGGUIDLG_H__25094C23_2A8D_4897_A3BC_25E4BC9201FD__INCLUDED_)
