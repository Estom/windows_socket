/////////////////////////////////////////////////
// TCPClient.cpp�ļ�

#include "TCPClient.h"
#include "resource.h"

// ���������¼�֪ͨ��Ϣ
#define WM_SOCKET WM_USER + 1	

CMyApp theApp;

BOOL CMyApp::InitInstance()
{
	// ��ʼ��Winsock��
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	::WSAStartup(sockVersion, &wsaData);
	// ���������ڶԻ���
	CMainDialog dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	// �ͷ�Winsock��
	::WSACleanup();
	return FALSE;
}

CMainDialog::CMainDialog(CWnd* pParentWnd):CDialog(IDD_MAINDIALOG, pParentWnd)
{
}

BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
ON_BN_CLICKED(IDC_CONNECT, OnButtonConnect)
ON_BN_CLICKED(IDC_SEND, OnButtonSend)
ON_BN_CLICKED(IDC_CLEAR, OnButtonClear)
ON_MESSAGE(WM_SOCKET, OnSocket)
ON_BN_CLICKED(IDC_BUTTON_UDP2, &CMainDialog::OnBnClickedButtonUdp2)
ON_BN_CLICKED(IDC_BUTTON_UDP_START, &CMainDialog::OnBnClickedButtonUdpStart)
END_MESSAGE_MAP()

BOOL CMainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	// ����ͼ��
	SetIcon(theApp.LoadIcon(IDI_MAIN), FALSE);
	// ����״̬����������������
	m_bar.Create(WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP, CRect(0, 0, 0, 0), this, 101);
	m_bar.SetBkColor(RGB(0xa6, 0xca, 0xf0));		// ����ɫ
	int arWidth[] = { 200, -1 };
	m_bar.SetParts(2, arWidth);				// ����
	m_bar.SetText(" TCP and UDP Client by yinkanglong and zhangmeng ", 1, 0);	// ��һ�������ı�
	m_bar.SetText(" ����", 0, 0);				// �ڶ��������ı�
	// ��ʼ�����Ͱ�ť�ͷ��ͱ༭���״̬
	GetDlgItem(IDC_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);

	// ��ʼ�������׽���
	m_socket = INVALID_SOCKET;
	m_UDPsocket = INVALID_SOCKET;
	return TRUE;
}

void CMainDialog::OnCancel()
{
	if(m_socket != INVALID_SOCKET)
		::closesocket(m_socket);
	if(m_UDPsocket != INVALID_SOCKET)
		::closesocket(m_UDPsocket);

	CDialog::OnCancel();
}

void CMainDialog::OnButtonClear()
{
	GetDlgItem(IDC_INFO)->SetWindowText("");	
}

void CMainDialog::OnButtonConnect()
{
	if(m_socket == INVALID_SOCKET)  // ���ӷ�����
	{
		// ȡ�÷�������ַ
		CString sAddr;
		GetDlgItem(IDC_ADDR)->GetWindowText(sAddr);
		if(sAddr.IsEmpty())
		{
			MessageBox("�������������ַ��");
			return;
		}

		// ȡ�ö˿ں�
		CString sPort;
		GetDlgItem(IDC_PORT)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("�˿ںŴ���");
			return;
		}

		// ��ͼ���ӷ�����
		if(!Connect(sAddr, nPort))
		{
			MessageBox("���ӷ���������");
			return;
		}
		
		// �����û�����
		GetDlgItem(IDC_CONNECT)->SetWindowText("ȡ��");
		m_bar.SetText(" �������ӡ���", 0, 0);
	}
	else				// �Ͽ�������
	{
		// �ر��׽���
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;

		// �����û�����
		GetDlgItem(IDC_CONNECT)->SetWindowText("���ӷ�����");
		m_bar.SetText(" ����", 0, 0);	
		GetDlgItem(IDC_ADDR)->EnableWindow(TRUE);
		GetDlgItem(IDC_PORT)->EnableWindow(TRUE);
		//GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_SEND)->EnableWindow(FALSE);
	}
}

long CMainDialog::OnSocket(WPARAM wParam, LPARAM lParam)
{
	// ȡ�����¼��������׽��־��
	SOCKET s = wParam;
	// �鿴�Ƿ����
	if(WSAGETSELECTERROR(lParam))
	{
		if(m_socket != SOCKET_ERROR)
			OnButtonConnect();
		m_bar.SetText(" ���ӳ���", 0, 0);
		return 0;
	}
	// ���������¼�
	switch(WSAGETSELECTEVENT(lParam))
	{	
	case FD_CONNECT:	// �׽�����ȷ�����ӵ�������
		{
			// �����û�����
			GetDlgItem(IDC_CONNECT)->SetWindowText("�Ͽ�����");

			GetDlgItem(IDC_ADDR)->EnableWindow(FALSE);
			GetDlgItem(IDC_PORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
			GetDlgItem(IDC_SEND)->EnableWindow(TRUE);
			m_bar.SetText(" �Ѿ����ӵ�������", 0, 0);
		}
		break;

	case FD_READ:		// �׽��ֽ��ܵ��Է����͹��������ݰ�
		{
			// �ӷ�������������
			char szText[1024] = { 0 };
			::recv(s, szText, 1024, 0);
			// ��ʾ���û�
			AddStringToList(CString(szText) + "\r\n");
		}
		break;

	case FD_CLOSE:
		OnButtonConnect();
		break;
	}

	return 0;
}

void CMainDialog::OnButtonSend()
{
	if(m_socket == INVALID_SOCKET)
	{
		return;
	}

	// ȡ��Ҫ���͵��ַ���
	CString sText;
	GetDlgItem(IDC_TEXT)->GetWindowText(sText);

	// ���һ�����س����С�
	// ע�⣬����������Ǳ���ģ��������ʹ�ñ������Ϊ�ͻ��˵�������Э�飬
	// ����SMTP��FTP�ȣ���Ҫ������ˡ���Ϊ��ЩЭ�鶼Ҫ��ʹ�á��س����С���Ϊһ������Ľ������
	sText += "\r\n";

	// �������ݵ�������
	if(::send(m_socket, sText, sText.GetLength(), 0) != -1)
	{
		AddStringToList(sText, FALSE);
		GetDlgItem(IDC_TEXT)->SetWindowText("");
	}
}

BOOL CMainDialog::Connect(LPCTSTR pszRemoteAddr, u_short nPort)
{
	// �����׽���
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket == INVALID_SOCKET)
	{
		return FALSE;
	}
	
	// ����socketΪ����֪ͨ��Ϣ����
	::WSAAsyncSelect(m_socket, m_hWnd,
		WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_WRITE | FD_READ);
	
	// �ٶ�szAddr��IP��ַ
	ULONG uAddr = ::inet_addr(pszRemoteAddr);
        if(uAddr == INADDR_NONE)
	{
        // ����IP��ַ������Ϊ������������
		// ��������ȡ��IP��ַ
		hostent* pHost = ::gethostbyname(pszRemoteAddr);
		if(pHost == NULL)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return FALSE;
		}
		// �õ��������ֽ�˳�����е�IP��ַ
                uAddr = ((struct in_addr*)*(pHost->h_addr_list))->s_addr;
        }
	
        // ��д��������ַ��Ϣ
	sockaddr_in remote;
        remote.sin_addr.S_un.S_addr = uAddr;
        remote.sin_family = AF_INET;
        remote.sin_port = htons(nPort);
	
        // ���ӵ�Զ�̻�
        ::connect(m_socket, (sockaddr*)&remote, sizeof(sockaddr));
	
	return TRUE;
}

void CMainDialog::AddStringToList(LPCTSTR pszText, BOOL bRecv)
{
	CString strEdit;
	GetDlgItem(IDC_INFO)->GetWindowText(strEdit);

	if(bRecv)
	{
		strEdit += "��Recv_tcp����";
		strEdit += pszText;
	}
	else
	{
		strEdit += "��Send_tcp����";
		strEdit += pszText;
	}
	GetDlgItem(IDC_INFO)->SetWindowText(strEdit);	
}
void CMainDialog::AddStringToList_UDP(LPCTSTR pszText, BOOL bRecv)
{
	CString strEdit;
	GetDlgItem(IDC_INFO)->GetWindowText(strEdit);

	if(bRecv)
	{
		strEdit += "��Recv_udp����";
		strEdit += pszText;
	}
	else
	{
		strEdit += "��Send_udp����";
		strEdit += pszText;
	}
	GetDlgItem(IDC_INFO)->SetWindowText(strEdit);	
}
/**********************************UDP����*************/
void CMainDialog::OnBnClickedButtonUdp2()
{

	//���Ŀ�Ķ�
	// ȡ�÷�������ַ
	CString sAddr;
	GetDlgItem(IDC_ADDR)->GetWindowText(sAddr);
	if(sAddr.IsEmpty())
	{
		MessageBox("�������������ַ��");
		return;
	}

	// ȡ��UDP�˿ں�
	CString sPort;
	GetDlgItem(IDC_EDIT_UDP_PORT)->GetWindowText(sPort);
	int nPort = atoi(sPort);
	if(nPort < 1 || nPort > 65535)
	{
		MessageBox("�˿ںŴ���");
		return;
	}


	// ���������׽���
	//m_UDPsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (m_UDPsocket == SOCKET_ERROR)
	{
		MessageBox("�����������");
		return;
	}

	//Ŀ�ĵ�ַ��Ϣ
	sockaddr_in sin={0};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.S_un.S_addr = inet_addr(sAddr);
	
	// ȡ��Ҫ���͵��ַ���
	CString sText;
	GetDlgItem(IDC_TEXT)->GetWindowText(sText);

	// ���һ�����س����С�
	sText += "\r\n";

	// �������ݵ�������
	if(::sendto(m_UDPsocket,sText,strlen(sText),0,(SOCKADDR *)&sin,sizeof(SOCKADDR))!= -1)
	{
		AddStringToList_UDP(sText, FALSE);
		GetDlgItem(IDC_TEXT)->SetWindowText("");
	}
}

DWORD WINAPI CMainDialog::udpRec(LPVOID lpParam)
{
	CMainDialog* lp_dialog = (CMainDialog*)lpParam;
	if (lp_dialog->m_UDPsocket == INVALID_SOCKET)
	{
		::MessageBox(lp_dialog->m_hWnd,"UDP����ʧ���޷�������Ϣ","udp",0);
	}
	::MessageBox(lp_dialog->m_hWnd,"UDP���ڽ�����Ϣ","udp",0);

	//���û�������С
	char buf[256];
	memset(buf,0,256);
	char bufHome[256];
	memset(bufHome,0,256);
	sockaddr_in clientAddr;
	memset(&clientAddr,0,sizeof(clientAddr));

	char cs_port[10];
	memset(cs_port,0,10);

	int clentAddrLen;
	clentAddrLen = sizeof(sockaddr_in);
	u_short port;
	int ret;
	int nPeerPort;
	CString sPeerIP;
	DWORD dwIP;
	hostent* pHost;
	CString strItem;
	while(1)
	{
		if (lp_dialog->m_UDPsocket == INVALID_SOCKET)
		{
			::MessageBox(lp_dialog->m_hWnd,"UDP ���ܶ��Ѿ��˳�","udp",0);
			return 0;

		}
		//::MessageBox(lp_dialog->m_hWnd,"receive qian","udp",0);
		//���ܿͻ��˷���������
		ret = recvfrom(lp_dialog->m_UDPsocket,buf,256,0,(sockaddr*)&clientAddr,&clentAddrLen);
		//::MessageBox(lp_dialog->m_hWnd,"receive hou","udp",0);


		//�õ��˿ں�,�����˿ں�ת��Ϊ�ַ���
		port = clientAddr.sin_port;
		
		sprintf_s(cs_port,10,"%d",int(port));
		// ת��Ϊ�����ֽ�˳��
		nPeerPort = ::ntohs(clientAddr.sin_port);
		// ת��Ϊ�ַ���IP
		sPeerIP = ::inet_ntoa(clientAddr.sin_addr);

		// ȡ�öԷ�����������
		// ȡ�������ֽ�˳���IPֵ
		dwIP = ::inet_addr(sPeerIP);
		// ��ȡ�������ƣ�ע�����е�һ��������ת��
		pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
		
		strncpy_s(bufHome,256, pHost->h_name, 256);

		// ��ʾ���û�
		strItem = CString("udp")+CString(bufHome) + "["+ sPeerIP+ "/"+CString(cs_port)+ "]: " + CString(buf);
		lp_dialog->AddStringToList_UDP(strItem, TRUE);
		//GetDlgItem(IDC_TEXT)->SetWindowText("");
		//lp_dialog->m_listInfo.InsertString(-1,strItem);
	}
	return 1;
}

void CMainDialog::OnBnClickedButtonUdpStart()
{
	if(m_UDPsocket == INVALID_SOCKET)  // ��������
	{
		// ȡ��Ŀ���ַ
		CString sAddr;
		GetDlgItem(IDC_ADDR)->GetWindowText(sAddr);
		if(sAddr.IsEmpty())
		{
			MessageBox("�������������ַ��");
			return;
		}

		// ȡ��UDP�˿ں�
		CString sPort;
		GetDlgItem(IDC_EDIT_UDP_PORT)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("�˿ںŴ���");
			return;
		}


		remoteUDPAddress.sin_family=AF_INET;
		remoteUDPAddress.sin_port = htons(nPort);
		remoteUDPAddress.sin_addr.S_un.S_addr = inet_addr(sAddr);

		// ���������׽���
		m_UDPsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (m_UDPsocket == SOCKET_ERROR)
		{
			MessageBox("�����������");
			return;
		}

		//�������ַ��Ϣ
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = 0;
		sin.sin_addr.s_addr = INADDR_ANY;
		// �󶨶˿�
		if(::bind(m_UDPsocket,(sockaddr*)&sin,sizeof(sin)) == SOCKET_ERROR)
		{
			MessageBox("�˿ںŰ�ʧ��");
			return;
		}

		//�������̣߳������������ݰ�
		//MessageBox("UDP�߳�����ǰ");
		h_udpThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)udpRec,this,0,NULL);

		//udpRec();
		//MessageBox("UDP�߳�������");

		//if(!CreateAndListen(nPort))
		//{
		//	MessageBox("�����������");
		//	return;
		//}

		// ��������Ӵ��ڿؼ�״̬
		GetDlgItem(IDC_BUTTON_UDP_START)->SetWindowText("ֹͣUDP�ͻ���");
		//m_bar.SetText(" ���ڼ�������", 0, 0);
		//GetDlgItem(IDC_EDIT_UDP)->EnableWindow(FALSE);
	}
	else				// ֹͣ����
	{
		MessageBox("UDP������ֹ");
		// �ر�����UDP����
		CloseUDPSocket();
		// ��������Ӵ��ڿؼ�״̬
		GetDlgItem(IDC_BUTTON_UDP_START)->SetWindowText("����UDP�ͻ���");
		//m_bar.SetText(" ����", 0, 0);
		//GetDlgItem(IDC_EDIT_UDP)->EnableWindow(TRUE);
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CMainDialog::CloseUDPSocket()
{
	// �رռ����׽���
	if(m_UDPsocket != INVALID_SOCKET)
	{
		::closesocket(m_UDPsocket);
		m_UDPsocket = INVALID_SOCKET;
	}

	//��ֹUDP�߳�
	if (h_udpThread!=NULL)
	{
		TerminateThread(h_udpThread,0);
		CloseHandle(h_udpThread);
	}

}

