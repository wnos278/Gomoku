
// GomokuServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GomokuServer.h"
#include "GomokuServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCriticalSection g_cs;

CGomokuServerDlg::CGomokuServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGomokuServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	for (int i = 0; i < MAX_CLIENT; i++) {
		m_clients[i].h_socket = INVALID_SOCKET;
	}
	m_nConnectClient = 0;
	m_nMatch = 0;
}

void CGomokuServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EVENT_LIST, m_event_list);
}

BEGIN_MESSAGE_MAP(CGomokuServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

BOOL CGomokuServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// TODO: Add extra initialization here
	mh_socket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(15003); // port

	bind(mh_socket, (LPSOCKADDR)&srv_addr, sizeof(srv_addr));
	listen(mh_socket, 1);
	AfxBeginThread(AcceptFunc, this);

	AddEvent(L"Khoi tao thanh cong");
	return TRUE;
}

void CGomokuServerDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

HCURSOR CGomokuServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT CGomokuServerDlg::AcceptFunc(LPVOID pParam)
{
	CGomokuServerDlg *pDlg = (CGomokuServerDlg*)pParam;
	while (true)
	{
		SOCKADDR_IN addr;
		// khoi tao addr tranh bi crash
		memset(&addr, 0, sizeof(addr));
		int iAddrLen = sizeof(addr), i;
		SOCKET client = accept(pDlg->mh_socket, (sockaddr*)&addr, &iAddrLen); // client duoc chap nhan ket noi

		if (client == INVALID_SOCKET)
			break;

		g_cs.Lock();
		for (i = 0; i < MAX_CLIENT; i++) {
			if (m_clients[i].h_socket == INVALID_SOCKET) break;
		}

		// neu trong mang chua client con trong thi them client do vao ket noi, con khong thi thoat
		if (i < MAX_CLIENT) 
		{
			// luu thong tin can thiet cua client
			m_clients[i].h_socket = client;
			strcpy(m_clients[i].ip_address, inet_ntoa(addr.sin_addr));
			m_clients[i].isLogin = false;

			// Thong bao len hop thoai server
			CString str = L"Dia chi ip vua ket noi : ";
			str += m_clients[i].ip_address;
			pDlg->AddEvent(str);

			// Gui thong bao da luu thong tin client vua ket noi, yeu cau dang nhap
			// Tao Cau truc Thong diep

			string cmd = "[CONNECT]";
			string id = "DONE";
			string data = "";
			pDlg->SendFrameData(m_clients[i].h_socket, cmd, id, data);

			// Bat dau lang nghe
			// luong xu ly event se thuc hien tru ctiep trong RecvFunc
			AfxBeginThread(RecvFunc, new CRecvParam(pDlg, client));

		}
		else {
			// Thong  bao khong ket noi duoc va closesocket
			string cmd = "[CONNECT]";
			string id = "FAIL";
			string data = "";
			pDlg->SendFrameData(m_clients[i].h_socket, cmd, id, data);
			closesocket(client);
		}
		g_cs.Unlock();
	}
	return 0;
}

UINT CGomokuServerDlg::RecvFunc(LPVOID pParam)
{
	// Dau vao la socket cua clieint vua thuc hien dang nhap thah cong
	CRecvParam *p = (CRecvParam*)pParam;
	CGomokuServerDlg *pDlg = p->m_pDlg;
	SOCKET client = p->m_ClientSocket;;
	
	while (true)
	{
		char message[MAX_MESSAGE] = { 0 };

		// nhan thong tin guii ve tu client theo socket tao ra
		recv(client, message, MAX_MESSAGE, 0);
		char cmd[20] = { 0 }; char id[20] = { 0 }; char data[20] = { 0 };
		sscanf(message, "%s %s %s\0", cmd, id, data);
		// Lang nghe thong diep gui den client, neu la lenh login thi
		if (strcmp(cmd, "[LOGIN]") == 0)
		{
			// khac voi id "done"
			if (strcmp(id, "DONE") == 0)
			{
				// trung voi tu khoa, gui thong diep dang nhap khong hop le
				const char* errorMsg = "[LOGIN] FAIL";
				send(client, errorMsg, strlen(errorMsg), 0);
				printf("trung voi tu khoa done");
				goto _COMPLETE;
			}
			// Doc du lieu tu file va so sanh 
			int found = 0;
			char fbuf[64] = { 0 }, fuser[32] = { 0 }, fpass[32] = { 0 };
			int ret = 0;

			// lay thong tin trong file passsword va check
			FILE* f = fopen("passwords.txt", "r");
			if (!f)
			{
				printf("khong ton tai password file");
				goto _COMPLETE;
			}
			while (!feof(f))
			{
				fgets(fbuf, sizeof(fbuf), f);
				ret = sscanf(fbuf, "%s %s", fuser, fpass);
				if (ret == 2)
				{
					if (strcmp(id, fuser) == 0 && strcmp(data, fpass) == 0)
					{
						found = 1;
						break;
					}
				}
			}
			fclose(f);

			if (found == 1)
			{
				for (int i = 0; i < MAX_CLIENT; i++)
				{
					if (m_players[i].socket == 0)
					{
						m_players[i].socket = client;
						m_players[i].id = string(id);
						m_players[i].state = "online";
						m_nConnectClient += 1;
						break;
					}
				}
				const char* okMsg = "[LOGIN] DONE\0";
				send(client, okMsg, strlen(okMsg), 0);
				goto _COMPLETE;
			}
			else
			{
				const char* errorMsg = "[LOGIN] FAIL\0";
				send(client, errorMsg, strlen(errorMsg), 0);
				goto _COMPLETE;
			}
		}
		else if (strcmp(cmd, "[LOGOUT]") == 0)
		{
			// Chck exist in connected client
			for (int i = 0; i < MAX_CLIENT; i++)
			{
				if (m_players[i].socket == client)
				{
					m_players[i].id = "";
					m_players[i].socket = 0;
					m_players[i].state = "";
					const char* okMsg = "[LOGOUT] DONE";
					send(client, okMsg, strlen(okMsg), 0);

					// Thong bao len hop thoai server
					CString str = L"Dia chi ip vua ngat ket noi : ";
					str += m_clients[i].ip_address;
					pDlg->AddEvent(str);

					goto _COMPLETE;
				}
				const char* errorMsg = "[LOGOUT] FAIL loi logout";
				send(client, errorMsg, strlen(errorMsg), 0);
				goto _COMPLETE;
			}
		}
		else if (strcmp(cmd, "[PLAY]") == 0)
		{
			// thay doi trang thai cua user thanh dang tim tran
			Player firstPlayer;
			for (int i = 0; i < m_nConnectClient; i++)
			{

				if (m_players[i].socket == client)
				{
					m_players[i].state = "finding match";
					firstPlayer = m_players[i];
				}
			}
			
			// tien hanh tim client khac de ghep doi
			for (int i = 0; i < m_nConnectClient; i++)
			{

				//  LAY THONG TIN CAC CLIENT DANG O TRANG THAI FINDING MATCH GUI VE CHO CLIENT
				if (strcmp(m_players[i].state.c_str(), "finding match") == 0 && m_players[i].socket != client)
				{
					// Khoi tao tran dau
					char idMatch[6];
					// nguoi choi nay se la nguoi choi thu nhat

					gen_random(idMatch, 6);
					Game(idMatch, firstPlayer, m_players[i]);
					char msg[64] = { 0 };
					// Tiep tuc gui thong diep thong tin ai di truoc ai di sau
					sprintf(msg, "[PLAY] FIRST %s", firstPlayer.id.c_str());
					send(client, msg, strlen(msg), NULL);
					send(m_players[i].socket, msg, strlen(msg), NULL);

					Sleep(1000);
					sprintf(msg, "[PLAY] %s %s", idMatch, m_players[i].id.c_str());
					send(client, msg, strlen(msg), 0);

					sprintf(msg, "[PLAY] %s %s", idMatch, firstPlayer.id.c_str());
					send(m_players[i].socket, msg, strlen(msg), 0);
					goto _COMPLETE;
				}

			}
			send(client, "[PLAY] NULL NULL", strlen("[PLAY] NULL NULL"), 0);
			goto _COMPLETE;
		}
		//else if (strcmp(cmd, "[EXITMATCH]") == 0)
		//{
		//	// Check match da co ket qua chua, neu chua thi bao loi va hoi xem co muon thoat tran khong
		//	// first check socket xem player co choi game khong
		//	for (int i = 0; i < m_nMatch; i++)
		//	{
		//		if (strcmp(id, matchInfo[i].id.c_str()) == 0)
		//		{
		//			int requestPlayerId = 0;
		//			if (matchInfo[i].player1.socket == client)
		//			{
		//				// player1 dang gui yeu cau
		//				requestPlayerId = 1;
		//				// Check xem tran dau ket thuc chua
		//			}
		//			else if (matchInfo[i].player2.socket == client)
		//			{
		//				// player2 dang gui yeu cau
		//				requestPlayerId = 2;
		//			}
		//			if (matchInfo[i].whoWin == "")
		//			{

		//			}
		//			else
		//			{
		//				// gui response thoat tran de client tro ve man hinh chinh
		//				send(client, "[", 1, 0);
		//			}
		//		}
		//	}
		//}
		//else if (strcmp(cmd, "[CHOOSE]") == 0)
		//{
		//	// check id dang o trang thai "finding match"
		//	for (int i = 0; i < m_nConnectClient; i++)
		//	{
		//		if (strcmp(m_players[i].id.c_str(), id) == 0)
		//		{
		//			if (strcmp(m_players[i].state.c_str(), "finding match") == 0)
		//			{
		//				// Tao tran dau
		//				Game("ma_game_id"); // phai thay doi dau vao khoi tao Match
		//				// thong bao tao tran thanh cong va gui thong tin client nay cho client yeu cau
		//				goto _COMPLETE;
		//			}
		//			else
		//			{
		//				// Thong bao loi khog ghep doi duoc
		//				goto _COMPLETE;
		//			}
		//		}
		//	}
		//	// Bao loi khong tim thay id, client da dang xuaat
		//	goto _COMPLETE;
		//}
		else if (strcmp(cmd, "[SUR]") == 0)
		{
			// id tran dau
			for (int i = 0; i < m_nMatch; i++)
			{
				if (strcmp(id, matchInfo[i].id.c_str()) == 0)
				{
					// check thong tin nguoi gui thong diep exit
					if (client == matchInfo[i].player1.socket)
					{
						// set nguoi thang nguoi thua cuoc, thong bao ket thuc tro choi
						matchInfo[i].whoWin = matchInfo[i].player2.id;
						// gui thong tin nguoi thang cho 2 client
						char msg[64] = { 0 };
						sprintf(msg, "[RESULT] %s %s", matchInfo[i].id.c_str(), matchInfo[i].player2.id.c_str());
						send(matchInfo[i].player1.socket, msg, strlen(msg), 0);
						send(matchInfo[i].player2.socket, msg, strlen(msg), 0);
						goto _COMPLETE;
					}
					else if (client == matchInfo[i].player2.socket)
					{
						//
						matchInfo[i].whoWin = matchInfo[i].player1.id;
						// gui thong tin nguoi thang cho 2 client
						char msg[64] = { 0 };
						sprintf(msg, "[RESULT] %s %s", matchInfo[i].id.c_str(), matchInfo[i].player1.id.c_str());
						send(matchInfo[i].player1.socket, msg, strlen(msg), 0);
						send(matchInfo[i].player2.socket, msg, strlen(msg), 0);
						goto _COMPLETE;
					}
					else
					{
						// tHONG BAO BAN KHONG THAM GIA VAO GAME NAY
						printf("nguoi nay khong dang tham gia tro choio");
					}
					goto _COMPLETE;
				}
			}
		}
		else if (strcmp(cmd, "[STEP]") == 0)
		{
			// Check match co ton tai khong
			for (int i = 0; i < m_nMatch; i++)
			{
				if (strcmp(matchInfo[i].id.c_str(), id) == 0)
				{
					char idMatch[20] = { 0 };
					strcpy(idMatch, id);
					char tmp1[64];
					char tmp2[64];
					sscanf(message, "%s %s %s %s", cmd, id, tmp1, tmp2);
					int ystep = atoi(tmp1); // gui thong tin vi tri cua y tuong ung voi board
					int xstep = atoi(tmp2); // ..
					// Xu ly nuoc di
					if (matchInfo[i].player1.socket == client)
					{
						if ((matchInfo[i].Board[xstep][ystep] == 1 || matchInfo[i].Board[xstep][ystep] == 2)
							&& matchInfo[i].player1.isTurn == true)
						{
							// bao loi da ton tai nuoc di nay
							send(client, "[STEP] ERROR 1", strlen("[STEP] ERROR 1"), 0);
							goto _COMPLETE;
						}
						if (matchInfo[i].player1.isTurn == false)
						{
							// bao loi chua den luot cua ban
							send(client, "[STEP] ERROR 2", strlen("[STEP] ERROR 2"), 0);
							goto _COMPLETE;
						}
						matchInfo[i].Board[xstep][ystep] = 1;
						// Kiem tra nguoi thang nguoi thua theo tung buoc di
						if (CheckEndGame(19, i) == 1)
						{
							// Nguoi thang cuoc la player1
							char msg[2048];
							sprintf(msg, "[RESULT] %s %s", matchInfo[i].id.c_str(), matchInfo[i].player1.id.c_str());
							send(client, msg, strlen(msg), NULL);
							send(matchInfo[i].player2.socket, msg, strlen(msg), NULL);
							goto _COMPLETE;
							// Thong bao nguoi thang cuoc va reset match va thoat
						}
						else if (CheckEndGame(19, i) == 2)
						{
							// Nguoi thang cuoc la player2
							char msg[2048];
							sprintf(msg, "[RESULT] %s %s", matchInfo[i].id.c_str(), matchInfo[i].player2.id.c_str());
							send(client, msg, strlen(msg), NULL);
							send(matchInfo[i].player2.socket, msg, strlen(msg), NULL);
							goto _COMPLETE;
							// Thong bao nguoi thang cuoc va reset match va thoat
						}
						matchInfo[i].play_steps.push_back(matchInfo[i].player1.id + ": " + to_string(xstep) + " " + to_string(ystep));
						matchInfo[i].player1.isTurn = false;
						matchInfo[i].player2.isTurn = true;
						// Gui vi tri vua chon cho 2 ban co tai client 
						char msg[2048];
						send(client, "[STEP] DONE", strlen("[STEP] DONE"), 0); // gui cho nguoi vua di chuyen
						sprintf(msg, "[STEP] %s %s %s", matchInfo[i].id.c_str(), tmp1, tmp2);
						send(matchInfo[i].player2.socket, msg, strlen(msg), NULL); // gui thong tin luot di cho doi phuong

						goto _COMPLETE;
					}
					else if (matchInfo[i].player2.socket == client)
					{
						if ((matchInfo[i].Board[xstep][ystep] == 2 || matchInfo[i].Board[xstep][ystep] == 1) && matchInfo[i].player2.isTurn == true)
						{
							// bao loi da ton tai nuoc di nay
							send(client, "[STEP] ERROR 1", strlen("[STEP] ERROR 1"), 0);
							goto _COMPLETE;
						}
						if (matchInfo[i].player2.isTurn == false)
						{
							// bao loi chua den luot cua ban
							send(client, "[STEP] ERROR 2", strlen("[STEP] ERROR 2"), 0);
							goto _COMPLETE;
						}
						matchInfo[i].Board[xstep][ystep] = 2;
						// Kiem tra nguoi thang nguoi thua theo tung buoc di
						if (CheckEndGame(19, i) == 1)
						{
							// Nguoi thang cuoc la player1
							char msg[2048];
							sprintf(msg, "[RESULT] %s %s", matchInfo[i].id.c_str(), matchInfo[i].player1.id.c_str());
							send(client, msg, strlen(msg), NULL);
							send(matchInfo[i].player2.socket, msg, strlen(msg), NULL);
							goto _COMPLETE;
							// Thong bao nguoi thang cuoc va reset match va thoat
						}
						else if (CheckEndGame(19,i) == 2)
						{
							char msg[2048];
							sprintf(msg, "[RESULT] %s %s", matchInfo[i].id.c_str(), matchInfo[i].player2.id.c_str());
							send(client, msg, strlen(msg), NULL);
							send(matchInfo[i].player2.socket, msg, strlen(msg), NULL);
							goto _COMPLETE;
							// Nguoi thang cuoc la player2
							// Thong bao nguoi thang cuoc va reset match va thoat
						}
						matchInfo[i].play_steps.push_back(matchInfo[i].player2.id + ": " + to_string(xstep) + " " + to_string(ystep));
						matchInfo[i].player1.isTurn = true;
						matchInfo[i].player2.isTurn = false;

						// Gui vi tri vua chon cho 2 ban co tai client 
						char msg[2048];
						send(client, "[STEP] DONE", strlen("[STEP] DONE"), 0); // gui cho nguoi vua di chuyen
						sprintf(msg, "[STEP] %s %s %s", matchInfo[i].id.c_str(), tmp1, tmp2);
						send(matchInfo[i].player1.socket, msg, strlen(msg), NULL); // gui thong tin luot di cho doi phuong

						goto _COMPLETE;
					}
					else
					{
						// Neu khong ton tai player nay, gui bao loi nuoc di khong hop le
						send(client, "[STEP] ERROR 3", strlen("[STEP] ERROR 3"), 0);
						goto _COMPLETE;
					}
				}
			}
		}
		else if (strcmp(cmd, "[LISTPLAYER]") == 0)
		{
			// luu thong tin danh sach client va gui ve cho nguoi choi yeu cau
			Player onlinePlayers[64];
			int nOnlinePlayer = 0;
			for (int i = 0; i < MAX_CLIENT; i++)
			{
				if (m_players[i].state == "finding match")
				{
					onlinePlayers[nOnlinePlayer] = m_players[i];
					nOnlinePlayer += 1;
				}
			}
			char message[2048] = { 0 };
			int len = sprintf(message, "[LISTPLAYER] NUMPLAYER %d\0", nOnlinePlayer);
			send(client, message, len, 0);
			int count = 0;
			Sleep(3000);
			while (count < nOnlinePlayer)
			{
				len = sprintf(message, "[LISTPLAYER] %s %s\0", onlinePlayers[count].id.c_str(), "");
				send(client, message, len, 0);
				count += 1;
			}
			goto _COMPLETE;
		}
		else if (strcmp(cmd, "[CHAT]") == 0)
		{
			char message[2048] = { 0 };
			int len = sprintf(message, "[CHAT] %s %s", id, data);
			for (int i = 0; i < m_nMatch; i++)
			{
				if (matchInfo[i].player1.socket == client || matchInfo[i].player2.socket == client)
				{
					// Gui thong tin chat den ca 2 client player
					if (matchInfo[i].player1.socket==client)
						send(matchInfo[i].player2.socket, message, len, 0);
					else
						send(matchInfo[i].player1.socket, message, len, 0);
					
					// Up su kien len cua so server
					pDlg->AddEvent(L"Gui thong diep chat cho 2 playr!");
					goto _COMPLETE;
				}
			}
		}
		else if (strcmp(cmd, "[RESULT]") == 0)
		{
			if (strcmp(id, "DONE") == 0)
			{
				// Reset lai trang thai cua nguoi choi
				for (int i = 0; i < MAX_CLIENT; i++)
					if (client == m_players[i].socket)
					{
						m_players[i].state = "online";
						m_players[i].isTurn = false;
					}
				// Lay gia tri id Game dau, kiem tra game dau con ton tai khong,  Reset lai ban dau
				for (int i = 0; i < 20; i++)
				{
					if (strcmp(data, matchInfo[i].id.c_str()) == 0)
					{
						// Reset lai game
						ResetGame(data);
					}
				}
				
			}
		}
		_COMPLETE:
			//gIAI PHONG CAC VUNG NHO KHOI TAO
			continue;
	}
}

// Them noi dung vao chat box
void CGomokuServerDlg::AddEvent(CString a_string)
{
	while (m_event_list.GetCount() > 1000) {
		m_event_list.DeleteString(0);
	}

	int index = m_event_list.InsertString(-1, a_string);
	m_event_list.SetCurSel(index);
}

void CGomokuServerDlg::SendFrameData(SOCKET ah_socket, string cmd, string id, string data)
{
	int send_data_size = cmd.size() + 1 + id.size() + 1 + data.size() + 1;
	char *p_send_data = new char[send_data_size];

	sprintf(p_send_data, "%s %s %s\0", cmd.c_str(), id.c_str(), data.c_str());
	/*memcpy(p_send_data + 4, p_send_data, send_data_size);*/
	send(ah_socket, p_send_data, send_data_size, 0);

	delete[] p_send_data;
}

// return true if delete done, else return false;
bool CGomokuServerDlg::Logout(Player client)
{
	return false;
}

// dau vao la id cua client de de phan biet cac client dang dang nhap voi nhau
void CGomokuServerDlg::GhepDoi(string id)
{
		// TIm trong connectedClient co client nao dang ranh roi va dang online thi gheop noi voi nha
		// rta lai loi neu khong tim thay client nao khac dang o trong thai online
		// neu tim duoc nguoi dau tien ranh roi thi ghep cap voi nguoi do
		// sau nay mo rong ra co the ghep cap voi nguoi co tran thang tuong duong hoac gan voi client dang tim kiem
}

void CGomokuServerDlg::Game(string mavanchoi, Player player1, Player player2) 
{
	// quan ly theo ma van choi
	// co truong bieu dien trang thai van choi: playing, end
	// tra ve ket qua cua van do va thong bao nguoi thang cuoc
	matchInfo[m_nMatch].id = mavanchoi;
	matchInfo[m_nMatch].Board;
	matchInfo[m_nMatch].player1 = player1;
	matchInfo[m_nMatch].player1.state = "playing";
	matchInfo[m_nMatch].player1.isTurn = true;
	matchInfo[m_nMatch].player2 = player2;
	matchInfo[m_nMatch].player1.state = "playing";
	matchInfo[m_nMatch].player2.isTurn = false ;
	matchInfo[m_nMatch].whoWin = "";
	m_nMatch += 1;
}

void CGomokuServerDlg::ResetGame(string mavanchoi)
{
	for (int i = 0; i < m_nMatch; i++)
	{
		if (matchInfo[i].id == mavanchoi)
		{
			matchInfo[i] = matchInfo[i + 1];
		}
	}
	m_nMatch -= 1;
}

void CGomokuServerDlg::ResetPlayer(Player player1)
{

}


int CGomokuServerDlg::CheckEndGame(int n, int indexBoard)
{
	for (int j = 0; j < n - 5; j++)
		for (int i = 0; i < n - 5; i++)
		{
			// Check ngang
			if (matchInfo[indexBoard].Board[j][i] == matchInfo[indexBoard].Board[j][i + 1]
				&& matchInfo[indexBoard].Board[j][i + 1] == matchInfo[indexBoard].Board[j][i + 2]
				&& matchInfo[indexBoard].Board[j][i + 2] == matchInfo[indexBoard].Board[j][i + 3]
				&& matchInfo[indexBoard].Board[j][i + 3] == matchInfo[indexBoard].Board[j][i + 4]
				&& matchInfo[indexBoard].Board[j][i] != 0)
				return matchInfo[indexBoard].Board[j][i];
			// Check doc
			if (matchInfo[indexBoard].Board[j][i] == matchInfo[indexBoard].Board[j + 1][i]
				&& matchInfo[indexBoard].Board[j + 1][i] == matchInfo[indexBoard].Board[j + 2][i]
				&& matchInfo[indexBoard].Board[j + 2][i] == matchInfo[indexBoard].Board[j + 3][i]
				&& matchInfo[indexBoard].Board[j + 3][i] == matchInfo[indexBoard].Board[j + 4][i]
				&& matchInfo[indexBoard].Board[j][i] != 0)
				return matchInfo[indexBoard].Board[j][i];
			// Check duong cheo
			if (matchInfo[indexBoard].Board[j][i] == matchInfo[indexBoard].Board[j + 1][i + 1]
				&& matchInfo[indexBoard].Board[j + 1][i + 1] == matchInfo[indexBoard].Board[j + 2][i + 2]
				&& matchInfo[indexBoard].Board[j + 2][i + 2] == matchInfo[indexBoard].Board[j + 3][i + 3]
				&& matchInfo[indexBoard].Board[j + 3][i + 3] == matchInfo[indexBoard].Board[j + 4][i + 4]
				&& matchInfo[indexBoard].Board[j][i] != 0)
				return matchInfo[indexBoard].Board[j][i];
		}
	return 0;
}

void gen_random(char *s, const int len) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	s[len] = 0;
}