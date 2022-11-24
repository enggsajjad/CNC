// ProgGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProgGUI.h"
#include "ProgGUIDlg.h"
#include <stdio.h>
#include <time.h>
#include "Include\PlxApi.h" /* (From the PLX SDK.) */

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////// SAJJAD HUSSAIN
/* Assorted #defines.
*/
#define PROGNAME      "SC9030W"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "1.0"

/* I/O register indices.
*/
#define R_9030CONTROLNSTATUS 0x0054 /* 9030 32-bit control/status register. */

/* PCI base register indeces.
*/
#define BRNUM_9030LCLCFGIO 1 /* Local configuration I/O base address register number. */
#define BRNUM_9030IO16     2 /* 16-bit I/O window to FPGA. (Base address register 2.) */
#define BRNUM_9030IO32     3 /* 32-bit I/O window to FPGA. (Base address register 3.) */
#define BRNUM_9030CFGMEM   4 /* Configuration memory window. */

/* PCI registers.
*/
#define pcibasereg2offset(r) (0x10 + ((r) * 4))

#define R_9030PCILCLCFGIO pcibasereg2offset(BRNUM_9030LCLCFGIO) /* Local configuration registers. */
#define R_9030PCICFGIO16  pcibasereg2offset(BRNUM_9030IO16)     /* 16-bit I/O window to FPGA. */
#define R_9030PCICFGIO32  pcibasereg2offset(BRNUM_9030IO32)     /* 32-bit I/O window to FPGA. */
#define R_9030PCICFGMEM   pcibasereg2offset(BRNUM_9030CFGMEM)   /* Configuration memory/32-bit memory window to FPGA. */

/* Masks for R_9030CONTROLNSTATUS.
*/
#define P_9030CFGPRGM         P_FPGAIOControlnStatus32
#define B_9030CFGPRGM         26 /* Program enable. (Output.) */
#define M_9030CFGPRGM         (0x1 << B_9030CFGPRGM)
#define M_9030CFGPRGMDEASSERT (0x0 << B_9030CFGPRGM) /* Reset the FPGA. */
#define M_9030CFGPRGMASSERT   (0x1 << B_9030CFGPRGM) /* Begin programming. */

#define P_9030CFGRW           P_FPGAIOControlnStatus32
#define B_9030CFGRW           23 /* Data direction control. (Output.) */
#define M_9030CFGRW           (0x1 << B_9030CFGRW)
#define M_9030CFGWRITEENABLE  (0x0 << B_9030CFGRW) /* From CPU. */
#define M_9030CFGWRITEDISABLE (0x1 << B_9030CFGRW) /* To CPU. */

#define P_9030LEDONOFF        P_FPGAIOControlnStatus32
#define B_9030LEDONOFF        17 /* Red LED control. (Output.) */
#define M_9030LEDONOFF        (0x1 << B_9030LEDONOFF)
#define M_9030LEDON           (0x0 << B_9030LEDONOFF)
#define M_9030LEDOFF          (0x1 << B_9030LEDONOFF)

#define P_9030PRGMDUN         P_FPGAIOControlnStatus32
#define B_9030PRGMDUN         11 /* Programming-done flag. (Input.) */
#define M_9030PRGMDUN         (0x1 << B_9030PRGMDUN)
#define M_9030PRGMDUNNOW      (0x1 << B_9030PRGMDUN)

/* Card I.D.
*/
#define VID 0x10B5 /* PCI vendor I.D. */
#define DID 0x9030 /* PCI device I.D. */

/* Exit codes.
*/
#define EC_OK     0   /* Exit OK. */
#define EC_BADCL  100 /* Bad command line. */
#define EC_USER   101 /* Invalid input. */
#define EC_HDW    102 /* Some sort of hardware failure on the 9030. */
#define EC_FILE   103 /* File error of some sort. */
#define EC_SYS    104 /* Beyond our scope. */
#define EC_PLXAPI 105 /* Some sort of error from the PLX API functions. */

/* Other constants.
*/
#define DOWNLOAD2MEMORY /* Download using mapped memory. Uncomment for fast download. */

/*---------------------------------------------------------------------------*/

/* Assorted typedefs.
*/
//typedef U8  unsigned8,  ubyte ;
//typedef U16 unsigned16, uword ;
//typedef U32 unsigned32, udword ;

//typedef RETURN_CODE     plxapiretcode ;
//typedef PLX_DEVICE_KEY plxdevloc ;

/*---------------------------------------------------------------------------*/

/* Local external variables.
*/
static void (*DataOutFuncPtr)(unsigned char thebyte) ;

static signed long ImageLen ; /* Number of bytes in device image portion of file. */

static int Opened /* = !!0 */ ;
#if defined DOWNLOAD2MEMORY
static int Mapped /* = !!0 */ ;
#endif

static FILE *ConfigFile ;

#if defined DOWNLOAD2MEMORY
static unsigned char *A_FPGAData;

#else
static U32 P_FPGAData ;
#endif

static PLX_DEVICE_OBJECT hDev ;

static U32 P_FPGAIOControlnStatus32 ;

static const char ProgNameMsg[] = { PROGNAME } ;
CString s;
void outdata8wswap(unsigned char thebyte);
void outdata8noswap(unsigned char thebyte);
void writedata8(unsigned char b);

///////// SAJJAD HUSSAIN
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
// CProgGUIDlg dialog

CProgGUIDlg::CProgGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgGUIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgGUIDlg)
	m_CardNo = 0;
	m_status = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProgGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgGUIDlg)
	DDX_Text(pDX, txtCard, m_CardNo);
	DDV_MinMaxLong(pDX, m_CardNo, 0, 15);
	DDX_Text(pDX, txtStatus, m_status);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgGUIDlg, CDialog)
	//{{AFX_MSG_MAP(CProgGUIDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(btn_Programming, OnProgramming)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgGUIDlg message handlers

BOOL CProgGUIDlg::OnInitDialog()
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
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProgGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CProgGUIDlg::OnPaint() 
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
HCURSOR CProgGUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CProgGUIDlg::OnProgramming() 
{
	// TODO: Add your control notification handler code here
	CString fname;
	// update form variables
	UpdateData(true);
	// Opne the file
	
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Bit Files(*.bit)|*.bit||",this);
	// Initializes m_ofn structure
	fileDlg.m_ofn.lpstrTitle = "Select Configuration File";
	// Call DoModal
	if ( fileDlg.DoModal() == IDOK)
	{
		fname = fileDlg.GetPathName(); // This is your selected file name with path
		s.Format("Your file name is :" +fname );
		MessageBox(s,"Fatal Error EC_HDW");
	}

	//if((ConfigFile = fopen("F:\\My5i20\\ProgGUI\\Bits\\IOPR24.BIT", "rb")) == 0) /* (Binary mode.) */
	if((ConfigFile = fopen(fname, "rb")) == 0) 
	{
		MessageBox("File not found: ","Fatal Error EC_USER");
		return;
	}

	// check parameters validity
	if(m_CardNo<0 && m_CardNo>15)
	{
		MessageBox("Invalid card number.","Fatal Error EC_USER");
		return;
	}
	FindPciCard(m_CardNo);
	
	selectfiletype() ;
	
	programfpga() ;
	
	cleanup() ;
	
	m_status += "\r\nFunction complete.";
	UpdateData(false);
	
}
void CProgGUIDlg::FindPciCard(unsigned cardnum)

{
	U32 p ;

	U32 devicenum ;

    RETURN_CODE       rc;//rcode;
    PLX_DEVICE_KEY    dev;//devicelocinfo;
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
	//flushing 
	fflush(stdout) ;
	
	switch (dev.SubDeviceId)
	{
	case 0x3131: 
		s.Format ("\r\nFound Mesa Electronics 5I20 card.") ;
		m_status += s;
		UpdateData(false);
		break;
	case 0x3132:
		s.Format ("\r\nFound Mesa Electronics 4I65 card.") ;
		m_status += s;
		UpdateData(false);
		break;
	default: 
		s.Format ("\r\nUnknown PLX PCI9030 based card!") ;
		m_status += s;
		UpdateData(false);
		
	}
	//Get our I/O base address.
	p = PlxPci_PciRegisterRead((dev . bus), (dev . slot),(dev . function), R_9030PCILCLCFGIO, &rc) ;
	if(rc != ApiSuccess)
	{
		#if !defined DOWNLOAD2MEMORY /* (Frivolous conditional compilation supresses annoying warning message.) */
			nocfgaccess:
		#endif
		s.Format("Can't read 9030 card number %u configuration registers.", cardnum) ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	P_FPGAIOControlnStatus32 = (p & ~0x3) ;/* FPGA base port. (We always assume that this base register maps I/O.) */

	s.Format ("\r\nPLX 9030 configuration I/O base port: 0x%04lX", P_FPGAIOControlnStatus32) ;
	m_status += s;
	UpdateData(false);

	P_FPGAIOControlnStatus32 += R_9030CONTROLNSTATUS ;
	s.Format ("\r\nPLX 9030 control/status port:         0x%04lX", P_FPGAIOControlnStatus32) ;
	m_status += s;
	UpdateData(false);

	#if defined DOWNLOAD2MEMORY
		rc = PlxPci_PciBarMap(&hDev, BRNUM_9030CFGMEM, (void**)&A_FPGAData) ;
		if(rc != ApiSuccess)
		{
			s.Format("Can't map 9030 card number %u base registers.", cardnum) ;
			MessageBox(s,"Fatal Error EC_PLXAPI");
		}
		Mapped = !0 ;

		if((A_FPGAData == (U8 *)-1) || (A_FPGAData == 0)) /* PLX example code performs this check, so we do too. */
		{
			s.Format("Invalid 9030 card number %u base register mapping.", cardnum) ;
			MessageBox(s,"Fatal Error EC_PLXAPI");
		}
		// From this point on, cleanup() *must* be called before we exit in order to prevent system resource leaks

		s.Format ("\r\nPLX 9030 memory base:                 0x%p",  (void *)A_FPGAData) ;
		m_status += s;
		UpdateData(false);
	#else
		p = PlxPci_PciRegisterRead((dev . bus), (dev . slot), (dev . function), R_9030PCICFGIO32, &rc) ;
		if(rc != ApiSuccess)
		{
			goto nocfgaccess ;
		}
		P_FPGAData = (U16)(p & ~0x3) ;

		s.Format ("\r\nPLX 9030 32-bit I/O base:             0x%04lX",P_FPGAData) ;
		m_status += s;
		UpdateData(false);
	#endif
}

/*---------------------------------------------------------------------------*/

/* Function: cleanup
   Purpose: Release external resources.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
     -Failure to call this function before program exit will cause system
		resource leaks.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::cleanup(void)

{
	#if defined DOWNLOAD2MEMORY
		if(Mapped)
		{
			PlxPci_PciBarUnmap(&hDev, (void**)&A_FPGAData) ;
		}
	#endif
	if(Opened)
	{
		PlxPci_DeviceClose(&hDev) ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: ledon
   Purpose: Turn on the 9030's programming LED.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::ledon(void)

{
	U32 r ;


	if(PlxPci_IoPortRead(&hDev, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		s.Format("Can't access 9030 card control register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");

	}
	r = ((r & ~M_9030LEDONOFF) | M_9030LEDON) ;
	if(PlxPci_IoPortWrite(&hDev, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: ledoff
   Purpose: Turn off the 9030's programming LED.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::ledoff(void)

{
	U32 r ;


	if(PlxPci_IoPortRead(&hDev, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		s.Format("Can't access 9030 card control register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	r = ((r & ~M_9030LEDONOFF) | M_9030LEDOFF) ;
	if(PlxPci_IoPortWrite(&hDev, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: writeenable
   Purpose: Enable configuration data transfer from the CPU to the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::writeenable(void)

{
	U32 r ;


	if(PlxPci_IoPortRead(&hDev, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		s.Format("Can't access 9030 card control register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	r = ((r & ~M_9030CFGRW) | M_9030CFGWRITEENABLE) ;
	if(PlxPci_IoPortWrite(&hDev, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
	ledon() ;
}

/*---------------------------------------------------------------------------*/

/* Function: writedisable
   Purpose: Disable configuration data transfer from the CPU to the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::writedisable(void)

{
	U32 r ;


	if(PlxPci_IoPortRead(&hDev, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		s.Format("Can't access 9030 card control register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	r = ((r & ~M_9030CFGRW) | M_9030CFGWRITEDISABLE) ;
	if(PlxPci_IoPortWrite(&hDev, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
	ledoff() ;
}

/*---------------------------------------------------------------------------*/

/* Function: programenable
   Purpose: Enable FPGA programming.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::programenable(void)

{
	U32 r ;


	writeenable() ;

	if(PlxPci_IoPortRead(&hDev, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		s.Format("Can't access 9030 card control register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	r = ((r & ~M_9030CFGPRGM) | M_9030CFGPRGMASSERT) ;
	if(PlxPci_IoPortWrite(&hDev, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: programdisable
   Purpose: Disable FPGA programming/reset the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::programdisable(void)

{
	U32 r ;


	if(PlxPci_IoPortRead(&hDev, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		s.Format("Can't access 9030 card control register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
	r = ((r & ~M_9030CFGPRGM) | M_9030CFGPRGMDEASSERT) ;
	if(PlxPci_IoPortWrite(&hDev, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
	writedisable() ;
}

/*---------------------------------------------------------------------------*/

/* Function: writedata8
   Purpose: Send a byte to the FPGA data port.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void writedata8(U8 b)

{
#if defined DOWNLOAD2MEMORY
	*A_FPGAData = b ;
#else
	if(PlxPci_IoPortWrite(hDev, P_FPGAData, &b, 4, BitSize32) != ApiSuccess)
	{
		s.Format("Can't access 9030 card data register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");
	}
#endif
}

/*---------------------------------------------------------------------------*/

/* Function: outdata8wswap
   Purpose: Send a byte to the FPGA data port with bits swapped.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void outdata8wswap(U8 thebyte)

{
	static const U8 swaptab[256] =
	{
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
	} ;

	
	writedata8(swaptab[thebyte]) ;
}

/*---------------------------------------------------------------------------*/

/* Function: outdata8noswap
   Purpose: Send a byte to the FPGA data port with no bit swapping.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void outdata8noswap(U8 thebyte)

{
	writedata8(thebyte) ;
}


/*---------------------------------------------------------------------------*/

/* Function: sendcompletionclocks
   Purpose: Send completion clocks to finish up programming.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::sendcompletionclocks(void)

{
	unsigned count ;


	writedisable() ;

	/* Send configuration completion clocks.  (6 should be enough, but we send
		 lots for good measure.)
	*/
	for(count = 24 ; count != 0 ; --count)
	{
		writedata8(-1) ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: programdunq
   Purpose: Return whether or not the FPGA is done programming.
   Used by: Local functions.
   Returns: Yes/no status.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

int CProgGUIDlg::programdunq(void)

{
	U32 r ;


	if(PlxPci_IoPortRead(&hDev, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		s.Format("Can't access 9030 card status register.") ;
		MessageBox(s,"Fatal Error EC_PLXAPI");

	}
	return (r & M_9030PRGMDUN) == M_9030PRGMDUNNOW ;
}

/*---------------------------------------------------------------------------*/

/* Function: wait4dun
   Purpose: Wait for the FPGA to finish programming.
   Used by: Local functions.
   Returns: Done/timeout status.
   Notes:
	 -Failure to become DONE may be caused by an attempt to load an invalid
		FPGA code image, or an I/O address conflict.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

signed CProgGUIDlg::wait4dun(void)

{
	clock_t then ;

	unsigned waitcount ;


	/* Wait at least long enough for the FPGA to finish its post-programming
		 tomfoolery.  This time is generally masked by software overhead, but the
		 programmer needs to be aware of it.
	*/
	then = (clock() + ((CLOCKS_PER_SEC / 10000) + 1)) ; /* 100 uS. should be enough. */
	while(clock() < then)
	{
		;
	}

	/* The DONE bit should now indicate end of programming.
	*/
	if(programdunq())
	{	/* Programming seems to be complete; make sure the state sticks and we're not
			 seeing some sort of I/O conflict-related foo.
		*/
		for(waitcount = 100 ; waitcount != 0 ; --waitcount)
		{
			if(!programdunq())
			{
				return -1 ; /* (Shouldn't.) */
			}
		}

		return 0 ; /* Success. */
	}
	else
	{
		return -1 ; /* Failure -- didn't become DONE. */
	}
}

/*---------------------------------------------------------------------------*/

/* Function: delay100us
   Purpose: Wait for 100 uS.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::delay100us(void)

{
	clock_t then ;

	then = (clock() + ((CLOCKS_PER_SEC / 10000) + 1)) ;
	while(clock() < then)
	{
		;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: filelengthq
   Purpose: Determine the length of the specified file.
   Used by: Local functions.
   Returns: The file length, or -1 on failure.
   Notes:
	 -The file's location pointer is preserved.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

long CProgGUIDlg::filelengthq(FILE *f)

{
	long curpos, len ;


	curpos = ftell(f) ;
	if(curpos < 0)
	{
		return -1 ;
	}
	if(fseek(f, 0, SEEK_END) != 0)
	{
		return -1 ;
	}
	len = ftell(f) ;
	if(len < 0)
	{
		return -1 ;
	}
	if(fseek(f, curpos, SEEK_SET) != 0)
	{
		return -1 ;
	}
	return len ;
}

/*---------------------------------------------------------------------------*/

/* Function: saybytesfromfile
   Purpose: Send the bytes from the specified offset in the specified open
     file to standard out.
   Used by: Local functions.
   Returns: The number of bytes read.
   Notes:
     -It is assumed that numbytes includes the trailing '\0'.
     -The file position is not restored to its original value.
     -Don't try to output 0 bytes.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

unsigned int CProgGUIDlg::saybytesfromfile(FILE *f, long fromoffset, size_t numbytes)

{
	int c ;

	size_t len = 0 ;

	CString s2,s3;// Sajjad

	if(fseek(f, fromoffset, SEEK_SET) != 0)
	{
		return 0 ;
	}
	while(numbytes-- != 0)
	{
		if((c = getc(f)) == EOF)
		{
			break ;
		}
		++len ;
		if(c == '\0')
		{
			break ;
		}
		//putchar(c) ;
		s3.Format ("%c",c);
		s2 += s3;
	}
	m_status += s2;
	UpdateData(false);

	return len ;
}

/*---------------------------------------------------------------------------*/

/* Function: readbytesfromfile
   Purpose: Read the specified number of bytes from the specified offset in
	 the specified open file.
   Used by: Local functions.
   Returns: The number of bytes read.
   Notes:
     -The file position is not restored to its original value.
     -Don't try to read 0 bytes.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

unsigned int CProgGUIDlg::readbytesfromfile(FILE *f, long fromoffset, size_t numbytes, U8 *bufptr)

{
	if(fseek(f, fromoffset, SEEK_SET) != 0)
	{
		return 0 ;
	}
	return fread(bufptr, 1, numbytes, f) ;
}

/*---------------------------------------------------------------------------*/

/* Function: sayfileinfo
   Purpose: Display some bytes from our configuration file on standard out.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
     -It is assumed that numbytes includes the trailing '\0'.
     -The file position is not restored to its original value.
     -Don't try to output 0 bytes.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::sayfileinfo(signed long *infobaseptr)

{
	size_t len ;

	U8 b[2] ;


	if(readbytesfromfile(ConfigFile, *infobaseptr, sizeof(b), b) != sizeof(b))
	{
		ioerror:
		s.Format("File I/O error.  Unexpected end of file?") ;
		MessageBox(s,"Fatal Error EC_FILE");
	}
	len = ((b[0] << 8) | b[1]) ; /* (Bigendian.) */
	if(saybytesfromfile(ConfigFile, (*infobaseptr + sizeof(b)), len) != len)
	{
		goto ioerror ;
	}
	*infobaseptr += (sizeof(b) + (signed long)len + 1) ; /* Skip over length field, text field, and tag. */
}

/*---------------------------------------------------------------------------*/

/* Function: selectfiletype
   Purpose: Set up for processing the input file.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

void CProgGUIDlg::selectfiletype(void)

{
	U8 b[14] ;


	/* Read in the interesting parts of the file header.
	*/
	if(readbytesfromfile(ConfigFile, 0, sizeof(b), b) != sizeof(b))
	{
		ioerror:
		s.Format("File I/O error.  Unexpected end of file?") ;
		MessageBox(s,"Fatal Error EC_FILE");
	}

	/* Figure out what kind of file we have.
	*/
	if((b[0] == 0x00) && (b[1] == 0x09) && (b[11] == 0x00) && (b[12] == 0x01) && (b[13] == 'a'))
	{	/* Looks like a .BIT file.
		*/
		signed long base ;


		m_status += "\r\nLooks like a .BIT file:";
		UpdateData(false);
		base = 14 ; /* Offset of design name length field. */

		/* Display file particulars.
		*/
		m_status += "\r\n  Design name:          ";
		sayfileinfo(&base) ;
		UpdateData(false);
		

		m_status += "\r\n  Part I.D.:            ";
		sayfileinfo(&base) ;
		UpdateData(false);

		
		m_status += "\r\n  Design date:          ";
		sayfileinfo(&base) ;
		UpdateData(false);

		
		m_status += "\r\n  Design time:          ";
		sayfileinfo(&base) ;
		UpdateData(false);
		

		if(readbytesfromfile(ConfigFile, base, 4, b) != 4)
		{
			goto ioerror ;
		}
		ImageLen = (((unsigned long)b[0] << 24) |
					((unsigned long)b[1] << 16) |
					((unsigned long)b[2] << 8) |
					(unsigned long)b[3]) ;
		
		s.Format ("\r\n  Configuration length: %lu bytes", ImageLen);
		m_status += s;
		UpdateData(false);

		DataOutFuncPtr = outdata8wswap ; /* We have to swap bits in .BIT files. */

		/* We leave the file position set to the next byte in the file, which should
		     be the first byte of the body of the data image.
		*/
	}
	else if((b[0] == 0xFF) && (b[4] == 0x55) && (b[5] == 0x99) && (b[6] == 0xAA) && (b[7] == 0x66))
	{	/* Looks like a PROM file.
		*/
		m_status += "\r\nLooks like a PROM file:";
		UpdateData(false);

		DataOutFuncPtr = outdata8noswap ; /* No bit swap in PROM files. */

		ImageLen = filelengthq(ConfigFile); ;
		if(ImageLen < 0)
		{
			seekerror:
			s.Format("File seek error.") ;
			MessageBox(s,"Fatal Error EC_SYS");
		}
		s.Format ("\r\n  Configuration length: %lu bytes", ImageLen) ;
		m_status += s;
		UpdateData(false);

		/* PROM file data starts at offset 0, so we have to back up.
		*/
		if(fseek(ConfigFile, 0, SEEK_SET) != 0)
		{
			goto seekerror ;
		}
	}
	else
	{	/* It isn't something we know about.
		*/
		s.Format("Unknown file type.") ;
		MessageBox(s,"Fatal Error EC_FILE");
	}

	fflush(stdout) ; /* (Make sure the user can see all output to date.) */
}



/*---------------------------------------------------------------------------*/

/* Function: programfpga
   Purpose: Initialize the FPGA with the contents of file ConfigFile.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Thursday December 9, 2004 -- 13:40:27.
*/

//#define FORMATSTRING 

void CProgGUIDlg::programfpga(void)

{
	signed long bytesleft ;
	unsigned long numbytes ;

	size_t count ;

	static U8 filebuffer[16384] ;
	U8 *bptr ;


	/* Enable programming.
	*/
	m_status += "\r\nProgramming...\r\n";
	UpdateData(false);

	fflush(stdout) ;

	programdisable() ; /* Reset the FPGA. */
	if(programdunq())
	{	/* Note that if we see DONE at the start of programming, it's most likely due
			 to an attempt to access the FPGA at the wrong I/O location.
		*/
		s.Format("<DONE> status bit indicates busy at start of programming.") ;
		MessageBox(s,"Fatal Error EC_HDW");
	}
	programenable() ;

	/* Delay for at least 100 uS. to allow the FPGA to finish its reset
		 sequencing.  (In reality, the time taken for the initial file access
		 makes this this delay redundant.)
	*/
	delay100us() ;

	/* Program the card.
	*/
	numbytes = 0 ;
	bytesleft = ImageLen ;
	for( ; ; )
	{	/* Write the file to the FPGA.
		*/
		errno = 0 ;
		count = fread(filebuffer, 1, sizeof(filebuffer), ConfigFile) ;
		if(count == 0)
		{	/* 0 bytes could be end of file or error; must check.
			*/
			if(errno != 0)
			{
				programdisable() ;

				s.Format("File I/O error.") ;
				MessageBox(s,"Fatal Error EC_SYS");
			}
			else
			{
				//printf(FORMATSTRING, 0L) ;
				s.Format ("%-6lu", 0L) ;
				m_status += s;
				UpdateData(false);
				break ; /* Done. */
			}
		}
		else
		{	/* Write the block to the FPGA data port.
			*/
			//printf(FORMATSTRING "\r", bytesleft) ;
			//s.Format ("%-6lu" "\r\n", bytesleft) ;
			//m_status += s;
			//UpdateData(false);

			fflush(stdout) ;
			numbytes += (unsigned long)count ;
			bytesleft -= (signed long)count ;

			bptr = filebuffer ;
			do
			{
				DataOutFuncPtr(*bptr++) ;
			} while(--count != 0) ;
		}
	}

	fclose(ConfigFile) ;

	/* Wait for completion of programming.
	*/
	if(wait4dun() < 0)
	{
		s.Format("Error: Not <DONE>; programming not completed.") ;
		MessageBox(s,"Fatal Error EC_HDW");
	}

	/* Send configuration completion clocks.
	*/
	sendcompletionclocks() ;

	//printf("\nSuccessfully programmed %lu bytes.", numbytes) ;
	s.Format ("\r\nSuccessfully programmed %lu bytes.", numbytes) ;
	m_status += s;
	UpdateData(false);

}

void CProgGUIDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here

	// Create an instance
CFileDialog fileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Bit Files(*.bit)|*.bit||",this);
// Initializes m_ofn structure
fileDlg.m_ofn.lpstrTitle = "Select Configuration File";
// Call DoModal
if ( fileDlg.DoModal() == IDOK)
{
	CString szlstfile = fileDlg.GetPathName(); // This is your selected file name with path
	s.Format("Your file name is :" +szlstfile );
	MessageBox(s,"Fatal Error EC_HDW");
}

}
