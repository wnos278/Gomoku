
// GomokuDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <string> 

#define MAX_X_COUNT 19
#define MAX_Y_COUNT 19

using namespace std;

#define LINE_INTERVAL 30
#define MOVX 3
#define MOVY 7.5
// CGomokuDlg dialog
class CGomokuDlg : public CDialogEx
{
private:
	//int m_step_count;

	CListBox m_event_list;
	CString client_name;
	CBrush m_white_brush, m_black_brush;

	char m_dol_pos[MAX_Y_COUNT][MAX_X_COUNT];
// Construction
public:
	CGomokuDlg(CWnd* pParent = NULL);	// standard constructor
	inline void AddEvent(CString a_string);
	void SendFrameData(SOCKET ah_socket, string cmd, string id, string data);

// Dialog Data
	enum { IDD = IDD_GOMOKU_DIALOG };

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
public:
	afx_msg void OnBnClickedSendBtn();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT On28001(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On28002(WPARAM wParam, LPARAM lParam);

};
