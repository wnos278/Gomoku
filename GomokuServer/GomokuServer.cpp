
// GomokuServer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GomokuServer.h"
#include "GomokuServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGomokuServerApp

BEGIN_MESSAGE_MAP(CGomokuServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGomokuServerApp construction

CGomokuServerApp::CGomokuServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CGomokuServerApp object

CGomokuServerApp theApp;


// CGomokuServerApp initialization

BOOL CGomokuServerApp::InitInstance()
{
	CWinApp::InitInstance();


	WSADATA temp;
	WSAStartup(0x0202, &temp);

	CGomokuServerDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	WSACleanup();
	return FALSE;
}

