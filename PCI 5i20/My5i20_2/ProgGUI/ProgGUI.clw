; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CProgGUIDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ProgGUI.h"

ClassCount=3
Class1=CProgGUIApp
Class2=CProgGUIDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_PROGGUI_DIALOG

[CLS:CProgGUIApp]
Type=0
HeaderFile=ProgGUI.h
ImplementationFile=ProgGUI.cpp
Filter=N

[CLS:CProgGUIDlg]
Type=0
HeaderFile=ProgGUIDlg.h
ImplementationFile=ProgGUIDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=txtStatus

[CLS:CAboutDlg]
Type=0
HeaderFile=ProgGUIDlg.h
ImplementationFile=ProgGUIDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_PROGGUI_DIALOG]
Type=1
Class=CProgGUIDlg
ControlCount=5
Control1=txtStatus,edit,1352728580
Control2=btn_Programming,button,1342242816
Control3=txtCard,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352

