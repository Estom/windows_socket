/////////////////////////////////////////////////
// TCPClient.cpp文件

#include "TCPClient.h"
#include "resource.h"

// 定义网络事件通知消息
#define WM_SOCKET WM_USER + 1	

CMyApp theApp;

BOOL CMyApp::InitInstance()
{
	// 初始化Winsock库
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	::WSAStartup(sockVersion, &wsaData);
	// 弹出主窗口对话框
	CMainDialog dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	// 释放Winsock库
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
	// 设置图标
	SetIcon(theApp.LoadIcon(IDI_MAIN), FALSE);
	// 创建状态栏，设置它的属性
	m_bar.Create(WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP, CRect(0, 0, 0, 0), this, 101);
	m_bar.SetBkColor(RGB(0xa6, 0xca, 0xf0));		// 背景色
	int arWidth[] = { 200, -1 };
	m_bar.SetParts(2, arWidth);				// 分栏
	m_bar.SetText(" TCP and UDP Client by yinkanglong and zhangmeng ", 1, 0);	// 第一个栏的文本
	m_bar.SetText(" 空闲", 0, 0);				// 第二个栏的文本
	// 初始化发送按钮和发送编辑框的状态
	GetDlgItem(IDC_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);

	// 初始化连接套节字
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
	if(m_socket == INVALID_SOCKET)  // 连接服务器
	{
		// 取得服务器地址
		CString sAddr;
		GetDlgItem(IDC_ADDR)->GetWindowText(sAddr);
		if(sAddr.IsEmpty())
		{
			MessageBox("请输入服务器地址！");
			return;
		}

		// 取得端口号
		CString sPort;
		GetDlgItem(IDC_PORT)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("端口号错误！");
			return;
		}

		// 试图连接服务器
		if(!Connect(sAddr, nPort))
		{
			MessageBox("连接服务器出错！");
			return;
		}
		
		// 设置用户界面
		GetDlgItem(IDC_CONNECT)->SetWindowText("取消");
		m_bar.SetText(" 正在连接……", 0, 0);
	}
	else				// 断开服务器
	{
		// 关闭套节字
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;

		// 设置用户界面
		GetDlgItem(IDC_CONNECT)->SetWindowText("连接服务器");
		m_bar.SetText(" 空闲", 0, 0);	
		GetDlgItem(IDC_ADDR)->EnableWindow(TRUE);
		GetDlgItem(IDC_PORT)->EnableWindow(TRUE);
		//GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_SEND)->EnableWindow(FALSE);
	}
}

long CMainDialog::OnSocket(WPARAM wParam, LPARAM lParam)
{
	// 取得有事件发生的套节字句柄
	SOCKET s = wParam;
	// 查看是否出错
	if(WSAGETSELECTERROR(lParam))
	{
		if(m_socket != SOCKET_ERROR)
			OnButtonConnect();
		m_bar.SetText(" 连接出错！", 0, 0);
		return 0;
	}
	// 处理发生的事件
	switch(WSAGETSELECTEVENT(lParam))
	{	
	case FD_CONNECT:	// 套节字正确的连接到服务器
		{
			// 设置用户界面
			GetDlgItem(IDC_CONNECT)->SetWindowText("断开连接");

			GetDlgItem(IDC_ADDR)->EnableWindow(FALSE);
			GetDlgItem(IDC_PORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
			GetDlgItem(IDC_SEND)->EnableWindow(TRUE);
			m_bar.SetText(" 已经连接到服务器", 0, 0);
		}
		break;

	case FD_READ:		// 套接字接受到对方发送过来的数据包
		{
			// 从服务器接受数据
			char szText[1024] = { 0 };
			::recv(s, szText, 1024, 0);
			// 显示给用户
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

	// 取得要发送的字符串
	CString sText;
	GetDlgItem(IDC_TEXT)->GetWindowText(sText);

	// 添加一个“回车换行”
	// 注意，添加它并不是必须的，但是如果使用本软件作为客户端调试网络协议，
	// 比如SMTP、FTP等，就要添加它了。因为这些协议都要求使用“回车换行”作为一个命令的结束标记
	sText += "\r\n";

	// 发送数据到服务器
	if(::send(m_socket, sText, sText.GetLength(), 0) != -1)
	{
		AddStringToList(sText, FALSE);
		GetDlgItem(IDC_TEXT)->SetWindowText("");
	}
}

BOOL CMainDialog::Connect(LPCTSTR pszRemoteAddr, u_short nPort)
{
	// 创建套节字
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket == INVALID_SOCKET)
	{
		return FALSE;
	}
	
	// 设置socket为窗口通知消息类型
	::WSAAsyncSelect(m_socket, m_hWnd,
		WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_WRITE | FD_READ);
	
	// 假定szAddr是IP地址
	ULONG uAddr = ::inet_addr(pszRemoteAddr);
        if(uAddr == INADDR_NONE)
	{
        // 不是IP地址，就认为这是主机名称
		// 从主机名取得IP地址
		hostent* pHost = ::gethostbyname(pszRemoteAddr);
		if(pHost == NULL)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return FALSE;
		}
		// 得到以网络字节顺序排列的IP地址
                uAddr = ((struct in_addr*)*(pHost->h_addr_list))->s_addr;
        }
	
        // 填写服务器地址信息
	sockaddr_in remote;
        remote.sin_addr.S_un.S_addr = uAddr;
        remote.sin_family = AF_INET;
        remote.sin_port = htons(nPort);
	
        // 连接到远程机
        ::connect(m_socket, (sockaddr*)&remote, sizeof(sockaddr));
	
	return TRUE;
}

void CMainDialog::AddStringToList(LPCTSTR pszText, BOOL bRecv)
{
	CString strEdit;
	GetDlgItem(IDC_INFO)->GetWindowText(strEdit);

	if(bRecv)
	{
		strEdit += "【Recv_tcp】：";
		strEdit += pszText;
	}
	else
	{
		strEdit += "【Send_tcp】：";
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
		strEdit += "【Recv_udp】：";
		strEdit += pszText;
	}
	else
	{
		strEdit += "【Send_udp】：";
		strEdit += pszText;
	}
	GetDlgItem(IDC_INFO)->SetWindowText(strEdit);	
}
/**********************************UDP部分*************/
void CMainDialog::OnBnClickedButtonUdp2()
{

	//获得目的端
	// 取得服务器地址
	CString sAddr;
	GetDlgItem(IDC_ADDR)->GetWindowText(sAddr);
	if(sAddr.IsEmpty())
	{
		MessageBox("请输入服务器地址！");
		return;
	}

	// 取得UDP端口号
	CString sPort;
	GetDlgItem(IDC_EDIT_UDP_PORT)->GetWindowText(sPort);
	int nPort = atoi(sPort);
	if(nPort < 1 || nPort > 65535)
	{
		MessageBox("端口号错误！");
		return;
	}


	// 创建监听套节字
	//m_UDPsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (m_UDPsocket == SOCKET_ERROR)
	{
		MessageBox("启动服务出错！");
		return;
	}

	//目的地址信息
	sockaddr_in sin={0};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.S_un.S_addr = inet_addr(sAddr);
	
	// 取得要发送的字符串
	CString sText;
	GetDlgItem(IDC_TEXT)->GetWindowText(sText);

	// 添加一个“回车换行”
	sText += "\r\n";

	// 发送数据到服务器
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
		::MessageBox(lp_dialog->m_hWnd,"UDP创建失败无法接受消息","udp",0);
	}
	::MessageBox(lp_dialog->m_hWnd,"UDP正在接受消息","udp",0);

	//设置缓冲区大小
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
			::MessageBox(lp_dialog->m_hWnd,"UDP 接受端已经退出","udp",0);
			return 0;

		}
		//::MessageBox(lp_dialog->m_hWnd,"receive qian","udp",0);
		//接受客户端发来的数据
		ret = recvfrom(lp_dialog->m_UDPsocket,buf,256,0,(sockaddr*)&clientAddr,&clentAddrLen);
		//::MessageBox(lp_dialog->m_hWnd,"receive hou","udp",0);


		//得到端口号,并将端口号转化为字符串
		port = clientAddr.sin_port;
		
		sprintf_s(cs_port,10,"%d",int(port));
		// 转化为主机字节顺序
		nPeerPort = ::ntohs(clientAddr.sin_port);
		// 转化为字符串IP
		sPeerIP = ::inet_ntoa(clientAddr.sin_addr);

		// 取得对方的主机名称
		// 取得网络字节顺序的IP值
		dwIP = ::inet_addr(sPeerIP);
		// 获取主机名称，注意其中第一个参数的转化
		pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
		
		strncpy_s(bufHome,256, pHost->h_name, 256);

		// 显示给用户
		strItem = CString("udp")+CString(bufHome) + "["+ sPeerIP+ "/"+CString(cs_port)+ "]: " + CString(buf);
		lp_dialog->AddStringToList_UDP(strItem, TRUE);
		//GetDlgItem(IDC_TEXT)->SetWindowText("");
		//lp_dialog->m_listInfo.InsertString(-1,strItem);
	}
	return 1;
}

void CMainDialog::OnBnClickedButtonUdpStart()
{
	if(m_UDPsocket == INVALID_SOCKET)  // 开启服务
	{
		// 取得目标地址
		CString sAddr;
		GetDlgItem(IDC_ADDR)->GetWindowText(sAddr);
		if(sAddr.IsEmpty())
		{
			MessageBox("请输入服务器地址！");
			return;
		}

		// 取得UDP端口号
		CString sPort;
		GetDlgItem(IDC_EDIT_UDP_PORT)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("端口号错误！");
			return;
		}


		remoteUDPAddress.sin_family=AF_INET;
		remoteUDPAddress.sin_port = htons(nPort);
		remoteUDPAddress.sin_addr.S_un.S_addr = inet_addr(sAddr);

		// 创建监听套节字
		m_UDPsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (m_UDPsocket == SOCKET_ERROR)
		{
			MessageBox("启动服务出错！");
			return;
		}

		//绑定随机地址信息
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = 0;
		sin.sin_addr.s_addr = INADDR_ANY;
		// 绑定端口
		if(::bind(m_UDPsocket,(sockaddr*)&sin,sizeof(sin)) == SOCKET_ERROR)
		{
			MessageBox("端口号绑定失败");
			return;
		}

		//开启多线程，用来接收数据包
		//MessageBox("UDP线程启动前");
		h_udpThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)udpRec,this,0,NULL);

		//udpRec();
		//MessageBox("UDP线程启动后");

		//if(!CreateAndListen(nPort))
		//{
		//	MessageBox("启动服务出错！");
		//	return;
		//}

		// 设置相关子窗口控件状态
		GetDlgItem(IDC_BUTTON_UDP_START)->SetWindowText("停止UDP客户端");
		//m_bar.SetText(" 正在监听……", 0, 0);
		//GetDlgItem(IDC_EDIT_UDP)->EnableWindow(FALSE);
	}
	else				// 停止服务
	{
		MessageBox("UDP服务终止");
		// 关闭所有UDP连接
		CloseUDPSocket();
		// 设置相关子窗口控件状态
		GetDlgItem(IDC_BUTTON_UDP_START)->SetWindowText("开启UDP客户端");
		//m_bar.SetText(" 空闲", 0, 0);
		//GetDlgItem(IDC_EDIT_UDP)->EnableWindow(TRUE);
	}
	// TODO: 在此添加控件通知处理程序代码
}


void CMainDialog::CloseUDPSocket()
{
	// 关闭监听套节字
	if(m_UDPsocket != INVALID_SOCKET)
	{
		::closesocket(m_UDPsocket);
		m_UDPsocket = INVALID_SOCKET;
	}

	//终止UDP线程
	if (h_udpThread!=NULL)
	{
		TerminateThread(h_udpThread,0);
		CloseHandle(h_udpThread);
	}

}

