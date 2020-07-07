
// GomokuServer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CGomokuServerApp:
// See GomokuServer.cpp for the implementation of this class
//

class CGomokuServerApp : public CWinApp
{
public:
	CGomokuServerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGomokuServerApp theApp;

void gen_random(char *s, const int len);