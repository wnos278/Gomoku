// Home.cpp : implementation file
//

#include "stdafx.h"
#include "Gomoku.h"
#include "Home.h"
#include "Login.h"
#include "afxdialogex.h"
#include "GomokuDlg.h"

#define MAX_SIZE 2048

// CHome dialog
extern SOCKET client;
string id_doithu, id_match, first_step_id;
extern string self_id;

int numPlayer = 0;
IMPLEMENT_DYNAMIC(CHome, CDialogEx)

CHome::CHome(CWnd* pParent /*=NULL*/)
: CDialogEx(CHome::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CHome::~CHome()
{
}

void CHome::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLAYER_LIST, m_events_list);
}


BEGIN_MESSAGE_MAP(CHome, CDialogEx)
	ON_MESSAGE(28001, &CHome::On28001)
	ON_MESSAGE(28002, &CHome::On28002)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CHome::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_FINDPLAYER, &CHome::OnBnClickedBtnFindplayer)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CHome::OnBnClickedBtnLogout)
END_MESSAGE_MAP()


// CHome message handlers

// Gui yeu cau lay danh sach client, 
// Dang ky AsyncSelect

BOOL CHome::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	WSAAsyncSelect(client, m_hWnd, 28001, FD_READ);
	string info = "[LISTPLAYER]";
	int ret = send(client, info.c_str(), info.size(), NULL);
	if (ret == NULL) 
	{

		return FALSE;
	};
	return TRUE;
}


void CHome::OnBnClickedBtnPlay()
{
	// An nut play de bao voi server ban da san sang choi game
	int nSel = m_events_list.GetCurSel();
	CString ItemSelected;
	if (nSel != LB_ERR)
	{
		m_events_list.GetText(m_events_list.GetCurSel(), ItemSelected);
	}
	int ret = send(client, "[PLAY]", strlen("[PLAY]"), NULL);
}


void CHome::OnBnClickedBtnFindplayer()
{
	// Gui thong tin yeu cau lay tat ca nguoi dung dang online cho vao list
	string info = "[LISTPLAYER]";
	int ret = send(client, info.c_str(), info.size(), NULL);
	if (ret == NULL) 
	{
	};

}


void CHome::OnBnClickedBtnLogout()
{
	// Chi can gui di thong diep muon logout
	string info = "[LOGOUT]";
	int ret = send(client, info.c_str(), info.size(), NULL);
	if (ret == NULL) {};
}

afx_msg LRESULT CHome::On28001(WPARAM wParam, LPARAM lParam)
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

void CHome::AddPlayer(CString a_string)
{
	// tan dung ham nay de luu thong tin ra list
	while (m_events_list.GetCount() > 1000) {
		m_events_list.DeleteString(0);
	}

	int index = m_events_list.InsertString(-1, a_string);
	m_events_list.SetCurSel(index);
}

void CHome::SendFrameData(SOCKET ah_socket, string cmd, string id, string data)
{
	int send_data_size = cmd.size() + 1 + id.size() + 1 + data.size() + 1;
	char *p_send_data = new char[send_data_size];

	sprintf(p_send_data, "%s %s %s\0", cmd.c_str(), id.c_str(), data.c_str());
	send(ah_socket, p_send_data, send_data_size, 0);

	delete[] p_send_data;
}


afx_msg LRESULT CHome::On28002(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTEVENT(lParam) == FD_READ) 
	{
		WSAAsyncSelect(client, m_hWnd, 28002, FD_CLOSE);

		char buff[MAX_SIZE] = { 0 };
		strcpy(buff, ""); // set null to recv message server to client
		int ret = recv(client, buff, MAX_SIZE, NULL);
		buff[ret] = '\0';
		char cmd[64] = { 0 };
		char id[64] = { 0 };
		char data[64] = { 0 };
		// set null string
		strcpy(cmd, ""); strcpy(id, ""); strcpy(data, "");

		sscanf(buff, "%s %s %s\0", cmd, id, data);
		
		if (strcmp(cmd, "[LISTPLAYER]") == 0)
		{
			
			if (strcmp(id, "NUMPLAYER") == 0)
			{
				// LAY GIA TRI SO LUONG PLAYER
				numPlayer = atoi(data);
			}
			else
			{
				if (numPlayer != 0)
				{
					int count = 0;
					while (count < numPlayer)
					{
						ret = recv(client, buff, MAX_SIZE, NULL);
						// set null string
						strcpy(cmd, ""); strcpy(id, ""); strcpy(data, "");

						sscanf(buff, "%s %s %s\0", cmd, id, data);
						if (strcmp(cmd, "[LISTPLAYER]") == 0)
						{
							AddPlayer(CString(id));
						}

						count += 1;
					}
				}
			}
		}
		else if (strcmp(cmd, "[PLAY]") == 0)
		{
			
			// Nhan thong diep khoi tao ban dau
			if (strcmp(id, "NULL") == 0)
			{
					MessageBox(L"Khong tim duoc nguoi choi phu hop", L"Thong bao", NULL);
			}
			else if (strcmp(id, "FIRST") == 0)
			{
				
				if (strcmp(data, id_doithu.c_str()) == 0)
				{
					// neu la doi thu
					first_step_id = string(id_doithu);
				}
				else
					first_step_id = string(self_id);
			
			}
			else
			{
				// B1: Luu thong tin id nguoi choi
				id_doithu = string(data);
				id_match = string(id);

				// B2: Tao ban dau
				CHome::OnOK();
				CGomokuDlg dlg;
				dlg.DoModal();
			}
		}
		else if (strcmp(cmd, "[LOGOUT]") == 0)
		{
			if (strcmp(id, "DONE") == 0)
			{
				CHome::OnOK();
				CLogin login;
				login.DoModal();
			}
			else if (strcmp(id, "FAIL") == 0)
			{
				MessageBox(L"Dang xuat that bai", L"Bao loi", NULL);
			}
		}
		else
		{
			MessageBox(L"Thong diep gui toi khong dung logic", L"Bao loi", NULL);
		}
		

		WSAAsyncSelect(client, m_hWnd, 28002, FD_READ | FD_CLOSE);
	}
	else
	{
		// 
		AddPlayer(L"Loi!");
		closesocket(client);
		client = INVALID_SOCKET;
	}
	return 0;
}

