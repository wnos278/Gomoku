#pragma once
#include <string>
#include "afxwin.h"

using namespace std;

// CHome dialog

class CHome : public CDialogEx
{
	DECLARE_DYNAMIC(CHome)
private:
public:
	CHome(CWnd* pParent = NULL);   // standard constructor
	BOOL OnInitDialog();
	virtual ~CHome();

// Dialog Data
	enum { IDD = IDD_HOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HICON m_hIcon;

	afx_msg LRESULT On28002(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On28001(WPARAM wParam, LPARAM lParam);
	void SendFrameData(SOCKET ah_socket, string cmd, string id, string data);
	void AddPlayer(CString a_string);

public:
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnFindplayer();
	afx_msg void OnBnClickedBtnLogout();
	CListBox m_events_list;
};
