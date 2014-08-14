
// ChildView.h : CChildView ��Ľӿ�
//


#pragma once

 class CIOCPServer;
 struct ClientContext;
// CChildView ����

class CChildView : public CWnd
{
// ����
public:
	CChildView();

// ����
public:

// ����
public:

// ��д
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CChildView();

	int m_nCount;
	CListCtrl m_ListCtrl;
	CIOCPServer *m_iocpServer;
	// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnPaint();
	afx_msg LRESULT OnCreateListCtrl(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAddToList(WPARAM, LPARAM);
	afx_msg LRESULT OnRemoveFromList(WPARAM, LPARAM);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnOpenShellDialog(WPARAM, LPARAM);
};

