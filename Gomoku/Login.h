#pragma once

#include <string>
#include <cstdio>

// CLogin dialog
using namespace std;
class CLogin : public CDialogEx
{
	DECLARE_DYNAMIC(CLogin)

public:
	CLogin(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogin();
	BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	HICON m_hIcon;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLogin();
	afx_msg LRESULT CLogin::On28002(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT CLogin::On28001(WPARAM wParam, LPARAM lParam);
	void AddPlayer(CString a_string);
};
