
// GomokuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Gomoku.h"
#include "GomokuDlg.h"
#include "afxdialogex.h"
#include "Home.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGomokuDlg dialog

extern SOCKET client;
extern string id_doithu; 
extern string id_match;
extern string first_step_id;
extern string self_id;
int x;
int y;
int countStep = 0;
CGomokuDlg::CGomokuDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GOMOKU_DIALOG, pParent), m_white_brush(RGB(255, 255, 255)), m_black_brush(RGB(0, 0, 0))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CGomokuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EVENT_LIST, m_event_list);
}

BEGIN_MESSAGE_MAP(CGomokuDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(28001, &CGomokuDlg::On28001)
	ON_BN_CLICKED(IDC_SEND_BTN, &CGomokuDlg::OnBnClickedSendBtn)
	ON_MESSAGE(28002, &CGomokuDlg::On28002)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDCANCEL, &CGomokuDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CGomokuDlg message handlers

// {INIT}
BOOL CGomokuDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//CDialogEx::OnInitDialog();
	WSAAsyncSelect(client, m_hWnd, 28001, FD_READ);
	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		
	return TRUE;
}

// {INIT}
void CGomokuDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		// Ve ban co
		CPaintDC dc(this);

		for (size_t y = 1; y < MAX_Y_COUNT; y++) {
			for (size_t x = 1; x < MAX_X_COUNT; x++) {
				dc.Rectangle((x + MOVX)*LINE_INTERVAL, (y + MOVY)*LINE_INTERVAL,
					(x + MOVX)*LINE_INTERVAL + LINE_INTERVAL + 1, (y + MOVY)*LINE_INTERVAL + LINE_INTERVAL + 1);
			}
		}
		CBrush *p_old_brush;

		for (size_t y = 1; y <= MAX_Y_COUNT; y++) {
			for (size_t x = 1; x <= MAX_X_COUNT; x++) {
				if (m_dol_pos[y - 1][x - 1]) {
					if (m_dol_pos[y - 1][x - 1] == 1) {
						p_old_brush = dc.SelectObject(&m_black_brush);
					}
					else {
						p_old_brush = dc.SelectObject(&m_white_brush);
					}
					dc.Rectangle((x + MOVX)*LINE_INTERVAL - LINE_INTERVAL / 2, (y + MOVY)*LINE_INTERVAL - LINE_INTERVAL / 2,
						(x + MOVX)*LINE_INTERVAL + LINE_INTERVAL + 1 - LINE_INTERVAL / 2,
						(y + MOVY)*LINE_INTERVAL + LINE_INTERVAL + 1 - LINE_INTERVAL / 2);

					dc.SelectObject(p_old_brush);
				}
			}
		}
	}
}

HCURSOR CGomokuDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// {COMMON}
void CGomokuDlg::AddEvent(CString a_string)
{
	while (m_event_list.GetCount() > 1000) {
		m_event_list.DeleteString(0);
	}

	int index = m_event_list.InsertString(-1, a_string);
	m_event_list.SetCurSel(index);
}

// {COMMON}
void CGomokuDlg::SendFrameData(SOCKET ah_socket, string cmd, string id, string data)
{
	int send_data_size = cmd.size() + 1 + id.size() + 1 + data.size() + 1;
	char *p_send_data = new char[send_data_size];

	sprintf(p_send_data, "%s %s %s\0", cmd.c_str(), id.c_str(), data.c_str());
	send(ah_socket, p_send_data, send_data_size, 0);

	delete[] p_send_data;
}

// {CHAT}
void CGomokuDlg::OnBnClickedSendBtn()
{
	// Tao va gui thong diep chat
	CString text, str;
	GetDlgItemText(IDC_CHAT_EDIT, text);
	string cmd = "[CHAT]";
	string data(CW2A(text.GetString()));
	AddEvent(L"You: " + text); // Ghi len giao dien de theo doi
	// Gui thong diep chat cho server
	SendFrameData(client, cmd, id_doithu, data);
	
	SetDlgItemText(IDC_CHAT_EDIT, L"");
}

// {EVENT}
void CGomokuDlg::OnBnClickedCancel()
{
	// Gui thong tin cancel: dau hang
	SendFrameData(client, "[SUR]", id_match, "");
}

// {EVENT}
void CGomokuDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CClientDC dc(this);
	bool flag = true;
	if (point.x >= 95 && point.x <= 680)
	{
		x = (point.x-1) / LINE_INTERVAL - 3;
		if ((point.x-1) % LINE_INTERVAL > LINE_INTERVAL / 2) x++;
	}
	else flag = false;
	
	if (point.y >= 230 && point.y <= 810)
	{
		y = (point.y) / LINE_INTERVAL - 8;
		if ((point.y) % LINE_INTERVAL > LINE_INTERVAL / 2) y++;
	}
	else flag = false;
	
	// Lay thong tin diem click chuot va gui di cho server
	if (flag)
	{
		string cmd = "[STEP]";
		char data[256];
		string xPos = to_string(x);
		string yPos = to_string(y);
		sprintf(data, "%s %s", xPos.c_str(), yPos.c_str());

		SendFrameData(client, cmd, id_match, string(data));
	}
	
	CDialogEx::OnLButtonDown(nFlags, point);
}

// {EVENT}
afx_msg LRESULT CGomokuDlg::On28001(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam)) {
		AddEvent(L"Loi!");
		closesocket(client);
		client = INVALID_SOCKET;
	}
	else {
		//AddEvent(L"Client vua tham gia game");
		WSAAsyncSelect(client, m_hWnd, 28002, FD_READ | FD_CLOSE);
	}
	return 0;
}

// {EVENT}
afx_msg LRESULT CGomokuDlg::On28002(WPARAM wParam, LPARAM lParam)
{

	if (WSAGETSELECTEVENT(lParam) == FD_READ) {
		//WSAAsyncSelect(client, m_hWnd, 28002, FD_CLOSE);

		char message[2048] = { 0 };

		// nhan thong tin guii ve tu client theo socket tao ra
		recv(client, message, 2048, 0);
		char cmd[20] = { 0 }; char id[20] = { 0 }; char data[20] = { 0 };
		sscanf(message, "%s %s %s\0", cmd, id, data);
		// Lang nghe thong diep gui den client, neu la lenh login thi
		if (strcmp(cmd, "[STEP]") == 0)
		{
			if (strcmp(id, "ERROR") == 0)
			{
				if (strcmp(data, "1") == 0)
				{
					// Loi da ton tai nuoc di
					MessageBox(L"Da ton tai nuoc di nay", L"Thong bao", NULL);
				}
				else if (strcmp(data, "2") == 0)
				{
					// Loi chua den luot di
					MessageBox(L"Chua den luot di cua ban", L"Thong bao", NULL);
				}
				else if (strcmp(data, "3") == 0)
				{
					MessageBox(L"Nuoc di khong hop le", L"Thong bao", NULL);
				}
			}

			else if (strcmp(id, "DONE") == 0)
			{
				countStep += 1;
				// doc vi tri cua x va y sau do ghi ra bang dau
				CClientDC dc(this);

				CBrush *p_old_brush;
				// neu la doi phuong thi color la gi
				// neu la minh thi color la gi
				if (first_step_id == id_doithu)
				{
					if (countStep % 2 == 1)
						p_old_brush = dc.SelectObject(&m_black_brush);
					else
						p_old_brush = dc.SelectObject(&m_white_brush);
				}
				else
				{
					if (countStep % 2 == 1)
						p_old_brush = dc.SelectObject(&m_white_brush);
					else
						p_old_brush = dc.SelectObject(&m_black_brush);
				}
					
				dc.Ellipse((x + MOVX)*LINE_INTERVAL - LINE_INTERVAL / 2 + LINE_INTERVAL / 2, (y + MOVY)*LINE_INTERVAL - LINE_INTERVAL / 2 + LINE_INTERVAL / 2,
					(x + MOVX)*LINE_INTERVAL + LINE_INTERVAL / 2 + LINE_INTERVAL / 2, (y + MOVY)*LINE_INTERVAL + LINE_INTERVAL / 2 + LINE_INTERVAL / 2);
				dc.SelectObject(p_old_brush);
			}
			else if (strcmp(id, id_match.c_str()) == 0)
			{
				countStep += 1;
				// Cac nuoc di cua nguoi choi 2
				char tmp1[64];
				char tmp2[64];
				sscanf(message, "%s %s %s %s", cmd, id, tmp1, tmp2);
				int x = atoi(tmp1); // gui thong tin vi tri cua y tuong ung voi board
				int y = atoi(tmp2); // ..
				CClientDC dc(this);

				CBrush *p_old_brush;
				if (first_step_id == id_doithu)
				{
					if (countStep % 2 == 1)
						p_old_brush = dc.SelectObject(&m_black_brush);
					else
						p_old_brush = dc.SelectObject(&m_white_brush);
				}
				else
				{
					if (countStep % 2 == 1)
						p_old_brush = dc.SelectObject(&m_white_brush);
					else
						p_old_brush = dc.SelectObject(&m_black_brush);
				}

				dc.Ellipse((x + MOVX)*LINE_INTERVAL - LINE_INTERVAL / 2 + LINE_INTERVAL / 2, (y + MOVY)*LINE_INTERVAL - LINE_INTERVAL / 2 + LINE_INTERVAL / 2,
					(x + MOVX)*LINE_INTERVAL + LINE_INTERVAL / 2 + LINE_INTERVAL / 2, (y + MOVY)*LINE_INTERVAL + LINE_INTERVAL / 2 + LINE_INTERVAL / 2);
				dc.SelectObject(p_old_brush);

			}
		}
		else if (strcmp(cmd, "[RESULT]") == 0)
		{
			if (strcmp(data, id_doithu.c_str()) == 0)
			{
				// Doi thu thang, thong bao kt thuc tro choi
				MessageBox(L"Doi thu thang", L"Thong bao", NULL);
				CGomokuDlg::OnOK();
				CHome dlg;
				dlg.DoModal();
			}
			else 
			{
				MessageBox(L"Ban thang", L"Thong bao", NULL);
				CGomokuDlg::OnOK();
				CHome dlg;
				dlg.DoModal();
			}
			//return 0;
		}
		else if (strcmp(cmd, "[CHAT]") == 0)
		{
			// ID: TEN NGUOI GUI, DATA: NOI DUNG GUI DI
			CString name;
			CString text;
			name = CString(id);
			text = CString(data);
			AddEvent(name + L" : " + text);
		}
		

		WSAAsyncSelect(client, m_hWnd, 28002, FD_READ | FD_CLOSE);
	
	}
	else
	{
		AddEvent(L"Loi!");
		closesocket(client);
		client = INVALID_SOCKET;
	}
	return 0;
}

