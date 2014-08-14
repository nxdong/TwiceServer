
// ChildView.h : CChildView 类的接口
//


#pragma once

 class CIOCPServer;
 struct ClientContext;
// CChildView 窗口

class CChildView : public CWnd
{
// 构造
public:
	CChildView();

// 特性
public:

// 操作
public:

// 重写
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CChildView();

	int m_nCount;
	CListCtrl m_ListCtrl;
	CIOCPServer *m_iocpServer;
	// 生成的消息映射函数
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

