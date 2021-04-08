/////////////////////////////////////////////////////
// TCPServer.cpp�ļ�

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
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_MESSAGE(WM_SOCKET, OnSocket)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CMainDialog::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CMainDialog::OnBnClickedButtonSelect)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CMainDialog::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_UDP, &CMainDialog::OnBnClickedButtonUdp)
	ON_BN_CLICKED(IDC_BUTTON_UDP_SEDN, &CMainDialog::OnBnClickedButtonUdpSedn)
END_MESSAGE_MAP()

BOOL CMainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	// ����ͼ��
	SetIcon(theApp.LoadIcon(IDI_MAIN), FALSE);
	// ����״̬����������������
	m_bar.Create(WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP, CRect(0, 0, 0, 0), this, 101);
	m_bar.SetBkColor(RGB(0x6, 0xca, 0xf0));		// ����ɫ
	int arWidth[] = { 200, -1 };
	m_bar.SetParts(2, arWidth);				// ����
	m_bar.SetText(" by yinkanglong and zhangmeng ", 1, 0);	// ��һ�������ı�
	m_bar.SetText(" ����", 0, 0);				// �ڶ��������ı�
	// �����б��ؼ���m_listInfo����Ĺ���
	m_listInfo.SubclassDlgItem(IDC_INFO, this);
	
	//�����û��б��ڿռ�m_listUser����Ĺ���
	m_listUser.SubclassDlgItem(IDC_LIST_USER,this);

	//����ѡ�е��û��б�ؼ���������
	m_listUserSelect.SubclassDlgItem(IDC_LIST_USER_SELECT,this);
	// ��ʼ�������׽��ֺ������б�
	m_socket = INVALID_SOCKET;
	m_udpSocket = INVALID_SOCKET;
	// ��ʼ�������׽��ֵ��б�������
	m_nClient = 0;
	m_nUDPClient =0;
	
	// ������ȡ�ñ���IP��ַ�Ĺ��̣�������ʾ��״̬���ĵ�һ��������
	// ȡ�ñ�������	
	char szHost[256];
	::gethostname(szHost, 256);
	// ͨ����������ȡ�õ�ַ��Ϣ
	HOSTENT* pHost = gethostbyname(szHost);
	if(pHost != NULL)
	{  	
		CString sIP;
		
		// �õ���һ��IP��ַ
		in_addr *addr =(in_addr*) *(pHost->h_addr_list);
		
		// ��ʾ���û�
		sIP.Format(" ����IP��%s", inet_ntoa(addr[0]));
		m_bar.SetText(sIP, 0, 0);
	}
	
	return TRUE;
}

void CMainDialog::OnStart()
{
	if(m_socket == INVALID_SOCKET)  // ��������
	{
		// ȡ�ö˿ں�
		CString sPort;
		GetDlgItem(IDC_PORT)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("�˿ںŴ���");
			return;
		}

		// ���������׽��֣�ʹ���������״̬
		if(!CreateAndListen(nPort))
		{
			MessageBox("�����������");
			return;
		}
		
		// ��������Ӵ��ڿؼ�״̬
		GetDlgItem(IDC_START)->SetWindowText("ֹͣ����");
		m_bar.SetText(" ���ڼ�������", 0, 0);
		GetDlgItem(IDC_PORT)->EnableWindow(FALSE);
	}
	else				// ֹͣ����
	{
		// �ر���������
		CloseAllSocket();

		// ��������Ӵ��ڿؼ�״̬
		GetDlgItem(IDC_START)->SetWindowText("��������");
		m_bar.SetText(" ����", 0, 0);
		GetDlgItem(IDC_PORT)->EnableWindow(TRUE);
	}
}

void CMainDialog::OnCancel()
{
	CloseAllSocket();
	CloseUDPSocket();
	CDialog::OnCancel();
}

void CMainDialog::OnClear()
{
	m_listInfo.ResetContent();
}

long CMainDialog::OnSocket(WPARAM wParam, LPARAM lParam)
{
	// ȡ�����¼��������׽��־��
	SOCKET s = wParam;
	// �鿴�Ƿ����
	if(WSAGETSELECTERROR(lParam))
	{
		SocketAddr sa ={};
		sa.sock=s;
		RemoveClient(sa);
		::closesocket(s);
		return 0;
	}
	// ���������¼�
	switch(WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:		// �����е��׽��ּ�⵽�����ӽ���
		{
			if(m_nClient < MAX_SOCKET)
			{
				//���������
				sockaddr_in sockAddr;
				memset(&sockAddr, 0, sizeof(sockAddr));
				int nSockAddrLen = sizeof(sockAddr);
				// �������������µ��׽���client�������ӵ��׽���,�����ͻ��˵ĵ�ַ�Ͷ˿ں�
				SOCKET client = ::accept(s,(SOCKADDR*)&sockAddr,&nSockAddrLen);
				// �����µ��׽���Ϊ����֪ͨ��Ϣ����
				int i = ::WSAAsyncSelect(client, 
					m_hWnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE);
				SocketAddr sa={};
				sa.sock=client;
				sa.sockAddr=sockAddr;
				AddClient(sa);

	
			}
			else
			{
				MessageBox("���ӿͻ�̫�࣡");
			}
		}
		break;

	case FD_CLOSE:		// ��⵽�׽��ֶ�Ӧ�����ӱ��رա�
		{				//���������
			// �����µ��׽���Ϊ����֪ͨ��Ϣ����
			SocketAddr sa ={};
			sa.sock=s;
			RemoveClient(sa);
			::closesocket(s);
		}
		break;

	case FD_READ:		// �׽��ֽ��ܵ��Է����͹��������ݰ�
		{

			// ȡ�öԷ���IP��ַ�Ͷ˿ںţ�ʹ��getpeername������
			// Peer�Է��ĵ�ַ��Ϣ
			sockaddr_in sockAddr;
			memset(&sockAddr, 0, sizeof(sockAddr));
			int nSockAddrLen = sizeof(sockAddr);
			//ͨ��peer�ķ�ʽ�õ��Է���ip��ַ�Ͷ˿ں���Ϣ
			::getpeername(s, (SOCKADDR*)&sockAddr, &nSockAddrLen);
			//�õ��˿ں�,�����˿ں�ת��Ϊ�ַ���
			u_short port = sockAddr.sin_port;
			char cs_port[10];
			sprintf_s(cs_port,10,"%d",int(port));
			// ת��Ϊ�����ֽ�˳��
			int nPeerPort = ::ntohs(sockAddr.sin_port);
			// ת��Ϊ�ַ���IP
			CString sPeerIP = ::inet_ntoa(sockAddr.sin_addr);
			
			// ȡ�öԷ�����������
			// ȡ�������ֽ�˳���IPֵ
			DWORD dwIP = ::inet_addr(sPeerIP);
			// ��ȡ�������ƣ�ע�����е�һ��������ת��
			hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
			char szHostName[256];
			strncpy_s(szHostName,256, pHost->h_name, 256);		 

			// ������������������
			char szText[1024] = { 0 };
			::recv(s, szText, 1024, 0);

			// ��ʾ���û�
			CString strItem = CString(szHostName) + "["+ sPeerIP+ "/"+CString(cs_port)+ "]: " + CString(szText);
			AddStringToList(strItem,TRUE);
			//m_listInfo.InsertString(-1,strItem);

		}
		break;
	}
	return 0;
}


BOOL CMainDialog::CreateAndListen(int nPort)
{
	if(m_socket == INVALID_SOCKET)
		::closesocket(m_socket);

	// �����׽���
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket == INVALID_SOCKET)
		return FALSE;
	
	// ��дҪ�����ı��ص�ַ
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.s_addr = INADDR_ANY;
	// �󶨶˿�
	if(::bind(m_socket, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		return FALSE;
	}

	// ����socketΪ����֪ͨ��Ϣ����,�첽������Ϣ
	::WSAAsyncSelect(m_socket, m_hWnd, WM_SOCKET, FD_ACCEPT|FD_CLOSE);
	// �������ģʽ
	::listen(m_socket, 5);

	return TRUE;
}


BOOL CMainDialog::AddClient(SocketAddr socketAddr)
{


	if(m_nClient < MAX_SOCKET)
	{
		// ����µĳ�Ա
		m_saList[m_nClient++] = socketAddr;
		UpdateClientList();
		return TRUE;
	}
	return FALSE;
}

void CMainDialog::RemoveClient(SocketAddr socketAddr)
{


	BOOL bFind = FALSE;
	int i=0;
	for(i=0; i<m_nClient; i++)
	{
		if(m_saList[i].sock == socketAddr.sock)
		{
			bFind = TRUE;
			break;
		}
	}

	// ����ҵ��ͽ��˳�Ա���б����Ƴ�
	if(bFind)
	{
		m_nClient--;
		// ���˳�Ա����ĳ�Ա����ǰ�ƶ�һ����λ
		for(int j=i; j<m_nClient; j++)
		{
			m_arClient[j] = m_arClient[j+1];
		}
	}
	UpdateClientList();
}

void CMainDialog::UpdateClientList(){

	//���ԭ�����е��б���
	m_listUser.ResetContent();
	int i=0;
	char szHostName[256];
	memset(szHostName,0,256);
	for (i=0;i<m_nClient;i++)
	{
		//�����û��б������
		//�õ��˿ں�,�����˿ں�ת��Ϊ�ַ���
		u_short port = m_saList[i].sockAddr.sin_port;
		char cs_port[10];
		sprintf_s(cs_port,10,"%d",int(port));
		// ת��Ϊ�����ֽ�˳��
		int nPeerPort = ::ntohs(m_saList[i].sockAddr.sin_port);
		// ת��Ϊ�ַ���IP
		CString sPeerIP = ::inet_ntoa(m_saList[i].sockAddr.sin_addr);

		// ȡ�öԷ�����������
		// ȡ�������ֽ�˳���IPֵ
		DWORD dwIP = ::inet_addr(sPeerIP);
		// ��ȡ�������ƣ�ע�����е�һ��������ת��
		hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
		
		strncpy_s(szHostName,256, pHost->h_name, 256);
		// ��ʾ�û��б�
		CString strItem = CString(szHostName) + "|"+ sPeerIP+ "|"+CString(cs_port);
		AddStringToList(strItem, TRUE);
		m_listUser.AddString(strItem);
	}
}
void CMainDialog::CloseAllSocket()
{
	// �رռ����׽���
	if(m_socket != INVALID_SOCKET)
	{
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	
	// �ر����пͻ�������
	for(int i=0; i<m_nClient; i++)
	{
		::closesocket(m_arClient[i]);
	}
	m_nClient = 0;
}

void CMainDialog::CloseUDPSocket()
{
	// �رռ����׽���
	if(m_udpSocket != INVALID_SOCKET)
	{
		::closesocket(m_udpSocket);
		m_udpSocket = INVALID_SOCKET;
		m_udpSocket = INVALID_SOCKET;
	}

	m_udpSocket = INVALID_SOCKET;
	//��ֹUDP�߳�
	if (h_udpThread !=NULL)
	{
		TerminateThread(h_udpThread,0);
		CloseHandle(h_udpThread);
	}
}

void CMainDialog::AddStringToList(LPCTSTR pszText, BOOL bRecv)
{
	CString strEdit="";

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
	m_listInfo.InsertString(-1,strEdit);
}

void CMainDialog::OnBnClickedButtonSend()
{
	//��ȡĿ���û��б���
	int sum = m_listUserSelect.GetCount();
	int i=0;
	SOCKET m_socketSelect;

	// ȡ��Ҫ���͵��ַ���
	CString sText;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sText);

	// ���һ�����س����С�
	// ע�⣬����������Ǳ���ģ��������ʹ�ñ������Ϊ�ͻ��˵�������Э�飬
	// ����SMTP��FTP�ȣ���Ҫ������ˡ���Ϊ��ЩЭ�鶼Ҫ��ʹ�á��س����С���Ϊһ������Ľ������
	sText += "\r\n";
	for(i=0;i<sum;i++)
	{
		m_socketSelect=m_saListSelect[i].sock;

		if(m_socketSelect == INVALID_SOCKET)
		{
			return;
		}

		// �������ݵ�������
		if(::send(m_socketSelect, sText, sText.GetLength(), 0) != -1)
		{
			//���ͳɹ�
		}

	}
	AddStringToList(sText, FALSE);
	GetDlgItem(IDC_EDIT1)->SetWindowText("");

}

void CMainDialog::OnBnClickedButtonSelect()
{
	int j=0,z=0;
	int num = m_listUser.GetCount();
	for(j=0;j<num;j++)
	{
		if (m_listUser.GetSel(j)>0)
		{
			//��¼��ѡ�е��û�
			m_saListSelect[z++]=m_saList[j];
		}
	}


	//���ԭ�����е��б���
	m_listUserSelect.ResetContent();
	int i=0;
	for (i=0;i<z;i++)
	{
		//�����û��б������
		//�õ��˿ں�,�����˿ں�ת��Ϊ�ַ���
		u_short port = m_saListSelect[i].sockAddr.sin_port;
		char cs_port[10];
		sprintf_s(cs_port,10,"%d",int(port));
		// ת��Ϊ�����ֽ�˳��
		int nPeerPort = ::ntohs(m_saListSelect[i].sockAddr.sin_port);
		// ת��Ϊ�ַ���IP
		CString sPeerIP = ::inet_ntoa(m_saListSelect[i].sockAddr.sin_addr);

		// ȡ�öԷ�����������
		// ȡ�������ֽ�˳���IPֵ
		DWORD dwIP = ::inet_addr(sPeerIP);
		// ��ȡ�������ƣ�ע�����е�һ��������ת��
		hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
		char szHostName[256];
		strncpy_s(szHostName,256, pHost->h_name, 256);
		// ��ʾ�û��б�
		CString strItem = CString(szHostName) + "|"+ sPeerIP+ "|"+CString(cs_port);
		m_listUserSelect.AddString(strItem);
	}

	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void CMainDialog::OnBnClickedButtonUpdate()
{
	UpdateClientList();
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}
/***************************UDP��ʼʵ��****************************/
//DWORD WINAPI CMainDialog::udpRec()
//{
//	char revBuf[MaxSize];  
//	int byte = 0;  
//
//	while(1)  
//	{  
//		WaitForSingleObject(hMutex, INFINITE);  
//
//		byte= recv(sockClient,revBuf,strlen(revBuf)+1,0);//�������ӿͻ��˽�������  
//		if (byte<=0)  
//		{  
//			break;  
//		}     
//
//		printf("%s\n",revBuf);  
//
//		Sleep(1000);  
//		ReleaseMutex(hMutex);  
//
//	}  
//	closesocket(sockClient);//�ر�socket,һ��ͨ�����  
//}

void CMainDialog::OnBnClickedButtonUdp()
{
	if(m_udpSocket == INVALID_SOCKET)  // ��������
	{
		// ȡ�ö˿ں�
		CString sPort;
		GetDlgItem(IDC_EDIT_UDP)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("�˿ںŴ���");
			return;
		}


		// ���������׽���
		m_udpSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (m_udpSocket == SOCKET_ERROR)
		{
			MessageBox("�����������");
			return;
		}
	
		//�󶨵�ַ��Ϣ
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(nPort);
		sin.sin_addr.s_addr = INADDR_ANY;
		// �󶨶˿�
		if(::bind(m_udpSocket,(sockaddr*)&sin,sizeof(sin)) == SOCKET_ERROR)
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
		GetDlgItem(IDC_BUTTON_UDP)->SetWindowText("ֹͣ����");
		//m_bar.SetText(" ���ڼ�������", 0, 0);
		GetDlgItem(IDC_EDIT_UDP)->EnableWindow(FALSE);
	}
	else				// ֹͣ����
	{
		MessageBox("UDP������ֹ");
		// �ر�����UDP����
		CloseUDPSocket();
		// ��������Ӵ��ڿؼ�״̬
		GetDlgItem(IDC_BUTTON_UDP)->SetWindowText("��������");
		m_bar.SetText(" ����", 0, 0);
		GetDlgItem(IDC_EDIT_UDP)->EnableWindow(TRUE);
	}
	return;
}

DWORD WINAPI CMainDialog::udpRec(LPVOID lpParam)
{
	CMainDialog* lp_dialog = (CMainDialog*)lpParam;
	if (lp_dialog->m_udpSocket == INVALID_SOCKET)
	{
		::MessageBox(lp_dialog->m_hWnd,"UDP����ʧ���޷�������Ϣ","udp",0);
	}
	::MessageBox(lp_dialog->m_hWnd,"UDP���ڽ�����Ϣ","udp",0);

	//���û�������С
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
	SocketAddr temp;
	while (1)
	{
		if (lp_dialog->m_udpSocket == INVALID_SOCKET)
		{
			::MessageBox(lp_dialog->m_hWnd,"UDP ���ܶ��Ѿ��˳�","udp",0);
			return 0;

		}
		//::MessageBox(lp_dialog->m_hWnd,"receive qian","udp",0);
		//���ܿͻ��˷���������
		ret = recvfrom(lp_dialog->m_udpSocket,buf,255,0,(sockaddr*)&clientAddr,&clentAddrLen);
		//::MessageBox(lp_dialog->m_hWnd,"receive hou","udp",0);
		//���ͻ�����ӵ�UDP�ͻ����б���
		temp.sock=lp_dialog->m_udpSocket;
		temp.sockAddr=clientAddr;
		lp_dialog->AddUDPClient(temp);

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
		strncpy_s(bufHome,256, pHost->h_name, 255);

		// ��ʾ���û�
		strItem = CString("��RECV_udp��:")+CString(bufHome) + "["+ sPeerIP+ "/"+CString(cs_port)+ "]: " + CString(buf);
		lp_dialog->m_listInfo.InsertString(-1,strItem);
	}
	return 1;
}

//UDP �����������ݵĺ���
void CMainDialog::udpSend()
{
	if (m_udpSocket == INVALID_SOCKET)
	{
		MessageBox("UDP����ʧ���޷�������Ϣ");
	}
	MessageBox("UDP���ڷ�����Ϣ");
	//��ȡĿ���û��б���
	int i=0;
	sockaddr_in m_socketAddr;
	//char buf[255];
	// ȡ��Ҫ���͵��ַ���
	CString sText;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sText);

	// ���һ�����س����С�
	// ע�⣬����������Ǳ���ģ��������ʹ�ñ������Ϊ�ͻ��˵�������Э�飬
	// ����SMTP��FTP�ȣ���Ҫ������ˡ���Ϊ��ЩЭ�鶼Ҫ��ʹ�á��س����С���Ϊһ������Ľ������
	sText += "udp\r\n";
	for(i=0;i<m_nUDPClient;i++)
	{

		m_socketAddr=m_saListUDP[i].sockAddr;

		// �������ݵ�������
		if(::sendto(m_udpSocket,sText,strlen(sText),0,(SOCKADDR *)&m_socketAddr,sizeof(SOCKADDR)) != -1)
		{
			//���ͳɹ�
		}

	}
	AddStringToList(sText, FALSE);
	GetDlgItem(IDC_EDIT1)->SetWindowText("");
}

BOOL CMainDialog::AddUDPClient(SocketAddr socketAddr)
{
	BOOL bFind = FALSE;
	int i=0;
	for(i=0; i<m_nUDPClient; i++)
	{
		if(m_saListUDP[i].sockAddr.sin_addr.S_un.S_addr == socketAddr.sockAddr.sin_addr.S_un.S_addr)
		{
			bFind = TRUE;
			return TRUE;
		}
	}

	if(m_nUDPClient < MAX_SOCKET)
	{
		// ����µĳ�Ա
		m_saListUDP[m_nUDPClient++] = socketAddr;
		return TRUE;
	}
	return FALSE;
}

void CMainDialog::RemoveUDPClient(SocketAddr socketAddr)
{


	BOOL bFind = FALSE;
	int i=0;
	for(i=0; i<m_nUDPClient; i++)
	{
		if(m_saListUDP[i].sockAddr.sin_addr.S_un.S_addr == socketAddr.sockAddr.sin_addr.S_un.S_addr)
		{
			bFind = TRUE;
			break;
		}
	}

	// ����ҵ��ͽ��˳�Ա���б����Ƴ�
	if(bFind)
	{
		m_nUDPClient--;
		// ���˳�Ա����ĳ�Ա����ǰ�ƶ�һ����λ
		for(int j=i; j<m_nUDPClient; j++)
		{
			m_saListUDP[j] = m_saListUDP[j+1];
		}
	}
}
void CMainDialog::OnBnClickedButtonUdpSedn()
{
	//ȡ��Ҫ���͵��ַ���
	// ȡ��Ҫ���͵��ַ���
	CString sText;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sText);

	// ���һ�����س����С�
	sText += "\r\n";
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int i=0;
	for(i=0;i<m_nUDPClient;i++)
	{
		//Ŀ�ĵ�ַ��Ϣ
		sockaddr_in sin=m_saListUDP[i].sockAddr;


		// �������ݵ�������
		if(::sendto(m_udpSocket,sText,strlen(sText),0,(SOCKADDR *)&sin,sizeof(SOCKADDR))!= -1)
		{
			//���ͳɹ�
		}

	}
	//AddStringToList(sText, FALSE);
	sText = "��SEND_udp��:"+sText;
	m_listInfo.InsertString(-1,sText);
	GetDlgItem(IDC_EDIT1)->SetWindowText("");

}
