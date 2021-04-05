
// GomokuServerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include <string>
#define MAX_CLIENT 64
#define MAX_MESSAGE 2048
#define MAX_COLUMN 19
#define MAX_ROW 19 

#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS

using namespace std;

// Struct luu thong tin client ket noi
struct Client 
{ // thong tin client ket noi
	SOCKET	h_socket;
	char	ip_address[16];
	bool	isLogin = false;
};

// Struct luu thong tin client da dang nhap
struct Player
{
	string			id;
	SOCKET			socket;
	string			state;
	bool			isTurn;
};

// Struct luu thong tin van dau
struct GameStruct {
	string				id;
	string				whoWin; // id cua client win
	Player				player1;
	Player				player2;
	int					Board[MAX_ROW][MAX_COLUMN];
	vector<string>		play_steps;
};

static		Player		m_players[MAX_CLIENT];
//int color, m_step_count = 0;
static		GameStruct  matchInfo[32];
static		int			m_nConnectClient;
static		int			m_nMatch;
static		Client		m_clients[MAX_CLIENT];

// CGomokuServerDlg dialog
class CGomokuServerDlg : public CDialogEx
{
private:
	SOCKET mh_socket;

// Construction
public:
	CGomokuServerDlg(CWnd* pParent = NULL);	// standard constructor

	void AddEvent(CString a_string);
	void SendFrameData(SOCKET ah_socket, string cmd, string id, string data);
	static UINT AcceptFunc(LPVOID pParam);
	static UINT RecvFunc(LPVOID pParam);

	// Xu ly game
	static int	CheckEndGame(int n, int i);
	static void Game(string mavanchoi, Player player1, Player player2);
	static void ResetGame(string mavanchoi);
	static void ResetPlayer(Player player1);
	static void GhepDoi(string id);
	bool		Logout(Player client);
	bool		CheckPassword(char * account, char * password);

// Dialog Data
	enum { IDD = IDD_GOMOKUSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
private:
	CListBox m_event_list;
};

class CRecvParam
{
public:
	CRecvParam(CGomokuServerDlg* pDlg, SOCKET ClientSocket)
		:m_pDlg(pDlg), m_ClientSocket(ClientSocket)
	{

	}
	~CRecvParam()
	{
		closesocket(m_ClientSocket);
	}
	CGomokuServerDlg* m_pDlg;
	SOCKET m_ClientSocket;
};