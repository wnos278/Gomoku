
// Gomoku.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Gomoku.h"
#include "GomokuDlg.h"
#include "Login.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGomokuApp

BEGIN_MESSAGE_MAP(CGomokuApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGomokuApp construction

CGomokuApp::CGomokuApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CGomokuApp object

CGomokuApp theApp;


// CGomokuApp initialization

BOOL CGomokuApp::InitInstance()
{
	CWinApp::InitInstance();
	WSADATA temp;
	WSAStartup(0x0202, &temp);

	CLogin dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	WSACleanup();

	return FALSE;
}

