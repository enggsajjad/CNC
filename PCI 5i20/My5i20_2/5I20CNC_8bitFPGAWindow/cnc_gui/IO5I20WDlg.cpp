// IO5I20WDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IO5I20W.h"
#include "IO5I20WDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*	SAJJAD HUSSAIN */
#include "Include\PlxApi.h" /* (From the PLX SDK.) */

/* Assorted #defines.
*/
#define PROGNAME      "IO5I20W"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "2.0"

/* PCI base register indeces.
*/

#define BRNUM_5I20IO32 3 /* 8-bit I/O window to FPGA. (Base address register 3.) Changed USING PLXmom*/

/* PCI registers.
*/
#define pcibasereg2offset(r) (0x10 + ((r) * 4))

#define R_5I20PCICFGIO32 pcibasereg2offset(BRNUM_5I20IO32) /* 32-bit I/O window to FPGA. */

/* Port offsets for IOPR24.
*/
#define PBASE_IO        0  /* Port offset to I/O block. */
#define PBASE_DIRECTION 16 /* Port offset to I/O data direction control block. */

/* Card I.D.
*/
#define VID 0x10B5 /* PCI vendor I.D. */
#define DID 0x9030 /* PCI device I.D. */

/* Exit codes.
*/
#define EC_OK     0   /* Exit OK. */
#define EC_BADCL  100 /* Bad command line. */
#define EC_USER   101 /* Invalid input. */
#define EC_PLXAPI 105 /* Some sort of error from the PLX API functions. */


PLX_DEVICE_OBJECT hDev;//Device;
static unsigned long OVal, DirectionMask ;
static int Opened;
//static unsigned PortIndex ;
static U16 P_FPGAIOBase ;

CString s;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIO5I20WDlg dialog

CIO5I20WDlg::CIO5I20WDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIO5I20WDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIO5I20WDlg)
	m_status = _T("");
	m_PortIndex = 0;
	m_CardNo = 0;
	m_Value = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIO5I20WDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIO5I20WDlg)
	DDX_Text(pDX, txt_status, m_status);
	DDX_Text(pDX, txt_PortIndex, m_PortIndex);
	DDV_MinMaxLong(pDX, m_PortIndex, 0, 15);
	DDX_Text(pDX, txt_Card, m_CardNo);
	DDV_MinMaxLong(pDX, m_CardNo, 0, 15);
	DDX_Text(pDX, txt_Value, m_Value);
	DDV_MaxChars(pDX, m_Value, 8);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIO5I20WDlg, CDialog)
	//{{AFX_MSG_MAP(CIO5I20WDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(cmdWrite, OncmdWrite)


	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIO5I20WDlg message handlers

BOOL CIO5I20WDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	MessageBox("Welcome to PCI-5i20 Card!","5i20w");
	m_CardNo = 0;
	m_PortIndex = 0;
	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIO5I20WDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIO5I20WDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIO5I20WDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/*************** SAJJAD HUSSAIN *******************************/


void CIO5I20WDlg::FindPciCard(unsigned cardnum)

{
	U32 p ;

	U32 devicenum ;

    RETURN_CODE       rc;//rc;
    PLX_DEVICE_KEY    dev;//Key;
	/* Find the card.*/
	(dev . bus) = 4;//-1 ;
	(dev . slot) = 2;//-1 ;
	(dev . function) = 0 ;
	(dev . VendorId) = VID ;// Vendor ID
	(dev . DeviceId) = DID ;// Device ID
	//(dev . SerialNumber[0]) = '\0' ;
	// Set key structure to ignore all fields
    memset( &dev, PCI_FIELD_IGNORE, sizeof(PLX_DEVICE_KEY) );
	
	devicenum = (U32)cardnum ;
	
	if(PlxPci_DeviceFind(&dev, (U8)devicenum) != ApiSuccess)
	{
		//fatalerror(EC_USER, "1Can't find 5I20 card number %u.\n", cardnum) ;
		s.Format("Can't find 5I20 card number %u.", cardnum);
		MessageBox(s,"Fatal Error EC_USER");
	}
	
	if(PlxPci_DeviceOpen(&dev, &hDev) != ApiSuccess)
	{
		s.Format("Can't gain access to 5I20 card number %u.", cardnum) ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	
	Opened = !0 ; /* The API documentation doesn't say that 0 will never be returned in hDev, so we use a flag. */
	
	s.Format ("\r\nUsing PCI device at bus %u/device %u.", (dev . bus), (dev . slot)) ;
	m_status += s;
	UpdateData(false);

	//Get our I/O base address.
	p = PlxPci_PciRegisterRead((dev . bus), (dev . slot),(dev . function), R_5I20PCICFGIO32, &rc) ;
	if(rc != ApiSuccess)
	{
		s.Format("Can't read 5I20 card number %u I/O base address.", cardnum) ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	//P_FPGAIOBase = (U16)(p & ~0x3) ;
	P_FPGAIOBase = (U16)(p & ~0xF) ;
	

	s.Format ("\r\nPLX 9030 32-bit I/O base: 0x%04X", P_FPGAIOBase) ;
	m_status += s;
	UpdateData(false);
}

void CIO5I20WDlg::OncmdWrite() 
{
	// TODO: Add your control notification handler code here
	// update form variables
	UpdateData(true);

	//unsigned p = (m_PortIndex * sizeof(U32)) ;
	unsigned p = (m_PortIndex) ;

	

	// check parameters validity


	m_status += "\r\nWrite...";
	UpdateData(false);

	// find pci cards
	FindPciCard(m_CardNo) ;

	// Set Direction mask and output value
	//OVal = _tcstoul(m_Value,0,16); 
	OVal = _tcstoul(m_Value,0,10); 
	
	//dataout32((p + PBASE_IO), OVal) ;
	//if(PlxPci_IoPortWrite(&hDev, (P_FPGAIOBase + (p + PBASE_IO)), &OVal,4,BitSize32) != ApiSuccess)
	if(PlxPci_IoPortWrite(&hDev, (P_FPGAIOBase + (p + PBASE_IO)), &OVal,1,BitSize8) != ApiSuccess)
	{
		s.Format("Can't write to 5I20 ports.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	
	s.Format ("\r\nPort 0x%04X <-- ", (U16)(P_FPGAIOBase + p)) ; 
	m_status += s;
	UpdateData(false);

	s.Format ("%06X",OVal);
	m_status += s;
	//m_status += saybits24(OVal, '0', '1') ;
	UpdateData(false);

	// Close the pci card
	//cleanup() ;
	if(Opened)
	{
		PlxPci_DeviceClose(&hDev) ;
	}
	
	s.Format("\r\nFunction complete.\r\n") ;
	m_status += s;
	UpdateData(false);
}

