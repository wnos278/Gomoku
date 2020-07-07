// Login.cpp : implementation file
//

#include "stdafx.h"
#include "Gomoku.h"
#include "Login.h"
#include "afxdialogex.h"
#include "Home.h"

#define BUFF_SIZE 2048

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
SOCKET client;
string self_id;
// CLogin dialog

IMPLEMENT_DYNAMIC(CLogin, CDialogEx)

CLogin::CLogin(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLogin::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CLogin::~CLogin()
{
}

void CLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLogin, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CLogin::OnBnClickedBtnLogin)
	ON_MESSAGE(28001, &CLogin::On28001)
	ON_MESSAGE(28002, &CLogin::On28002)
END_MESSAGE_MAP()

BOOL CLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	return TRUE;
}

void CLogin::OnBnClickedBtnLogin()
{
	
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(15003);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	char buff[BUFF_SIZE] = { 0 };
	WSAAsyncSelect(client, m_hWnd, 28001, FD_CONNECT);

	// Step4: Request to connect server
	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr)))
		return;
	
}

afx_msg LRESULT CLogin::On28002(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTEVENT(lParam) == FD_READ)
	{
		WSAAsyncSelect(client, m_hWnd, 28002, FD_CLOSE);

		char buff[2048] = { 0 };
		unsigned short int body_size;

		recv(client, buff, 2048, 0);
		char cmd[64] = { 0 };
		char id[64] = { 0 };
		char data[64] = { 0 };
		sscanf(buff, "%s %s %s\0", cmd, id, data);

		if (strcmp(cmd, "[CONNECT]") == 0)
		{
			if (strcmp(id, "DONE") == 0)
			{
				CString csusername;
				CString cspassword;
				GetDlgItemText(IDC_USERNAME_ACCOUNT, csusername);
				GetDlgItemText(IDC_PASSWORD_ACCOUNT, cspassword);

				// CHECK ACCOUNT TO SERVER
				string username(CW2A(csusername.GetString()));
				string password(CW2A(cspassword.GetString()));

				string info = "[LOGIN] " + username +
					" " + password;
				
				int ret = send(client, info.c_str(), info.size(), NULL);
				if (ret == NULL) {};
				strcpy(buff, ""); // set null to recv message server to client
				Sleep(3000);
				ret = recv(client, buff, BUFF_SIZE, NULL);
				if (ret < 0)
					return 0;
				buff[ret] = '\0';
				// set null string
				strcpy(cmd, ""); strcpy(id, ""); strcpy(data, "");

				sscanf(buff, "%s %s %s\0", cmd, id, data);
				if (strcmp(cmd, "[LOGIN]") == 0)
				{
					if (strcmp(id, "DONE") == 0)
					{
						self_id = username;
						CLogin::OnOK();
						CHome home;
						home.DoModal();
					}
					else if (strcmp(id, "FAIL") == 0)
					{
						MessageBox(L"Ten tai khoan hoac mat khau khong hop le", L"Bao loi", NULL);
					}
				}
				else
				{
					MessageBox(L"Thong diep gui toi khong dung logic", L"Bao loi", NULL);
				}
			}

		}
		else
		{
			MessageBox(L"Loi ket noi", L"Bao loi", NULL);
		}
		WSAAsyncSelect(client, m_hWnd, 28002, FD_READ | FD_CLOSE);
	}
	else
	{
		closesocket(client);
		client = INVALID_SOCKET;
	}
	return 0;
}

afx_msg LRESULT CLogin::On28001(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam)) {
		closesocket(client);
		client = INVALID_SOCKET;
	}

	else {
		WSAAsyncSelect(client, m_hWnd, 28002, FD_READ | FD_CLOSE);
	}
	return 0;
}

