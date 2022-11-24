// IO5I20W.h : main header file for the IO5I20W application
//

#if !defined(AFX_IO5I20W_H__5519C798_76CF_4659_A878_29D03BCFFAEF__INCLUDED_)
#define AFX_IO5I20W_H__5519C798_76CF_4659_A878_29D03BCFFAEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIO5I20WApp:
// See IO5I20W.cpp for the implementation of this class
//

class CIO5I20WApp : public CWinApp
{
public:
	CIO5I20WApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIO5I20WApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIO5I20WApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IO5I20W_H__5519C798_76CF_4659_A878_29D03BCFFAEF__INCLUDED_)
