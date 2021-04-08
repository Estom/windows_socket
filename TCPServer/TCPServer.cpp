/////////////////////////////////////////////////////
// TCPServer.cpp文件

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
	// 设置图标
	SetIcon(theApp.LoadIcon(IDI_MAIN), FALSE);
	// 创建状态栏，设置它的属性
	m_bar.Create(WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP, CRect(0, 0, 0, 0), this, 101);
	m_bar.SetBkColor(RGB(0x6, 0xca, 0xf0));		// 背景色
	int arWidth[] = { 200, -1 };
	m_bar.SetParts(2, arWidth);				// 分栏
	m_bar.SetText(" by yinkanglong and zhangmeng ", 1, 0);	// 第一个栏的文本
	m_bar.SetText(" 空闲", 0, 0);				// 第二个栏的文本
	// 设置列表框控件到m_listInfo对象的关联
	m_listInfo.SubclassDlgItem(IDC_INFO, this);
	
	//设置用户列表窗口空间m_listUser对象的关联
	m_listUser.SubclassDlgItem(IDC_LIST_USER,this);

	//设置选中的用户列表控件关联对象
	m_listUserSelect.SubclassDlgItem(IDC_LIST_USER_SELECT,this);
	// 初始化监听套节字和连接列表
	m_socket = INVALID_SOCKET;
	m_udpSocket = INVALID_SOCKET;
	// 初始化监听套接字的列表项数量
	m_nClient = 0;
	m_nUDPClient =0;
	
	// 下面是取得本地IP地址的过程，将它显示在状态栏的第一个分栏中
	// 取得本机名称	
	char szHost[256];
	::gethostname(szHost, 256);
	// 通过本机名称取得地址信息
	HOSTENT* pHost = gethostbyname(szHost);
	if(pHost != NULL)
	{  	
		CString sIP;
		
		// 得到第一个IP地址
		in_addr *addr =(in_addr*) *(pHost->h_addr_list);
		
		// 显示给用户
		sIP.Format(" 本机IP：%s", inet_ntoa(addr[0]));
		m_bar.SetText(sIP, 0, 0);
	}
	
	return TRUE;
}

void CMainDialog::OnStart()
{
	if(m_socket == INVALID_SOCKET)  // 开启服务
	{
		// 取得端口号
		CString sPort;
		GetDlgItem(IDC_PORT)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("端口号错误！");
			return;
		}

		// 创建监听套节字，使它进入监听状态
		if(!CreateAndListen(nPort))
		{
			MessageBox("启动服务出错！");
			return;
		}
		
		// 设置相关子窗口控件状态
		GetDlgItem(IDC_START)->SetWindowText("停止服务");
		m_bar.SetText(" 正在监听……", 0, 0);
		GetDlgItem(IDC_PORT)->EnableWindow(FALSE);
	}
	else				// 停止服务
	{
		// 关闭所有连接
		CloseAllSocket();

		// 设置相关子窗口控件状态
		GetDlgItem(IDC_START)->SetWindowText("开启服务");
		m_bar.SetText(" 空闲", 0, 0);
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
	// 取得有事件发生的套节字句柄
	SOCKET s = wParam;
	// 查看是否出错
	if(WSAGETSELECTERROR(lParam))
	{
		SocketAddr sa ={};
		sa.sock=s;
		RemoveClient(sa);
		::closesocket(s);
		return 0;
	}
	// 处理发生的事件
	switch(WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:		// 监听中的套接字检测到有连接进入
		{
			if(m_nClient < MAX_SOCKET)
			{
				//定义接受者
				sockaddr_in sockAddr;
				memset(&sockAddr, 0, sizeof(sockAddr));
				int nSockAddrLen = sizeof(sockAddr);
				// 接受连接请求，新的套节字client是新连接的套节字,包括客户端的地址和端口号
				SOCKET client = ::accept(s,(SOCKADDR*)&sockAddr,&nSockAddrLen);
				// 设置新的套节字为窗口通知消息类型
				int i = ::WSAAsyncSelect(client, 
					m_hWnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE);
				SocketAddr sa={};
				sa.sock=client;
				sa.sockAddr=sockAddr;
				AddClient(sa);

	
			}
			else
			{
				MessageBox("连接客户太多！");
			}
		}
		break;

	case FD_CLOSE:		// 检测到套接字对应的连接被关闭。
		{				//定义接受者
			// 设置新的套节字为窗口通知消息类型
			SocketAddr sa ={};
			sa.sock=s;
			RemoveClient(sa);
			::closesocket(s);
		}
		break;

	case FD_READ:		// 套接字接受到对方发送过来的数据包
		{

			// 取得对方的IP地址和端口号（使用getpeername函数）
			// Peer对方的地址信息
			sockaddr_in sockAddr;
			memset(&sockAddr, 0, sizeof(sockAddr));
			int nSockAddrLen = sizeof(sockAddr);
			//通过peer的方式得到对方的ip地址和端口号信息
			::getpeername(s, (SOCKADDR*)&sockAddr, &nSockAddrLen);
			//得到端口号,并将端口号转化为字符串
			u_short port = sockAddr.sin_port;
			char cs_port[10];
			sprintf_s(cs_port,10,"%d",int(port));
			// 转化为主机字节顺序
			int nPeerPort = ::ntohs(sockAddr.sin_port);
			// 转化为字符串IP
			CString sPeerIP = ::inet_ntoa(sockAddr.sin_addr);
			
			// 取得对方的主机名称
			// 取得网络字节顺序的IP值
			DWORD dwIP = ::inet_addr(sPeerIP);
			// 获取主机名称，注意其中第一个参数的转化
			hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
			char szHostName[256];
			strncpy_s(szHostName,256, pHost->h_name, 256);		 

			// 接受真正的网络数据
			char szText[1024] = { 0 };
			::recv(s, szText, 1024, 0);

			// 显示给用户
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

	// 创建套节字
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket == INVALID_SOCKET)
		return FALSE;
	
	// 填写要关联的本地地址
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.s_addr = INADDR_ANY;
	// 绑定端口
	if(::bind(m_socket, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		return FALSE;
	}

	// 设置socket为窗口通知消息类型,异步接受消息
	::WSAAsyncSelect(m_socket, m_hWnd, WM_SOCKET, FD_ACCEPT|FD_CLOSE);
	// 进入监听模式
	::listen(m_socket, 5);

	return TRUE;
}


BOOL CMainDialog::AddClient(SocketAddr socketAddr)
{


	if(m_nClient < MAX_SOCKET)
	{
		// 添加新的成员
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

	// 如果找到就将此成员从列表中移除
	if(bFind)
	{
		m_nClient--;
		// 将此成员后面的成员都向前移动一个单位
		for(int j=i; j<m_nClient; j++)
		{
			m_arClient[j] = m_arClient[j+1];
		}
	}
	UpdateClientList();
}

void CMainDialog::UpdateClientList(){

	//清除原来所有的列表项
	m_listUser.ResetContent();
	int i=0;
	char szHostName[256];
	memset(szHostName,0,256);
	for (i=0;i<m_nClient;i++)
	{
		//设置用户列表的内容
		//得到端口号,并将端口号转化为字符串
		u_short port = m_saList[i].sockAddr.sin_port;
		char cs_port[10];
		sprintf_s(cs_port,10,"%d",int(port));
		// 转化为主机字节顺序
		int nPeerPort = ::ntohs(m_saList[i].sockAddr.sin_port);
		// 转化为字符串IP
		CString sPeerIP = ::inet_ntoa(m_saList[i].sockAddr.sin_addr);

		// 取得对方的主机名称
		// 取得网络字节顺序的IP值
		DWORD dwIP = ::inet_addr(sPeerIP);
		// 获取主机名称，注意其中第一个参数的转化
		hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
		
		strncpy_s(szHostName,256, pHost->h_name, 256);
		// 显示用户列表
		CString strItem = CString(szHostName) + "|"+ sPeerIP+ "|"+CString(cs_port);
		AddStringToList(strItem, TRUE);
		m_listUser.AddString(strItem);
	}
}
void CMainDialog::CloseAllSocket()
{
	// 关闭监听套节字
	if(m_socket != INVALID_SOCKET)
	{
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	
	// 关闭所有客户的连接
	for(int i=0; i<m_nClient; i++)
	{
		::closesocket(m_arClient[i]);
	}
	m_nClient = 0;
}

void CMainDialog::CloseUDPSocket()
{
	// 关闭监听套节字
	if(m_udpSocket != INVALID_SOCKET)
	{
		::closesocket(m_udpSocket);
		m_udpSocket = INVALID_SOCKET;
		m_udpSocket = INVALID_SOCKET;
	}

	m_udpSocket = INVALID_SOCKET;
	//终止UDP线程
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
		strEdit += "【Recv_tcp】：";
		strEdit += pszText;
	}
	else
	{
		strEdit += "【Send_tcp】：";
		strEdit += pszText;
	}
	m_listInfo.InsertString(-1,strEdit);
}

void CMainDialog::OnBnClickedButtonSend()
{
	//获取目标用户列表长度
	int sum = m_listUserSelect.GetCount();
	int i=0;
	SOCKET m_socketSelect;

	// 取得要发送的字符串
	CString sText;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sText);

	// 添加一个“回车换行”
	// 注意，添加它并不是必须的，但是如果使用本软件作为客户端调试网络协议，
	// 比如SMTP、FTP等，就要添加它了。因为这些协议都要求使用“回车换行”作为一个命令的结束标记
	sText += "\r\n";
	for(i=0;i<sum;i++)
	{
		m_socketSelect=m_saListSelect[i].sock;

		if(m_socketSelect == INVALID_SOCKET)
		{
			return;
		}

		// 发送数据到服务器
		if(::send(m_socketSelect, sText, sText.GetLength(), 0) != -1)
		{
			//发送成功
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
			//记录被选中的用户
			m_saListSelect[z++]=m_saList[j];
		}
	}


	//清除原来所有的列表项
	m_listUserSelect.ResetContent();
	int i=0;
	for (i=0;i<z;i++)
	{
		//设置用户列表的内容
		//得到端口号,并将端口号转化为字符串
		u_short port = m_saListSelect[i].sockAddr.sin_port;
		char cs_port[10];
		sprintf_s(cs_port,10,"%d",int(port));
		// 转化为主机字节顺序
		int nPeerPort = ::ntohs(m_saListSelect[i].sockAddr.sin_port);
		// 转化为字符串IP
		CString sPeerIP = ::inet_ntoa(m_saListSelect[i].sockAddr.sin_addr);

		// 取得对方的主机名称
		// 取得网络字节顺序的IP值
		DWORD dwIP = ::inet_addr(sPeerIP);
		// 获取主机名称，注意其中第一个参数的转化
		hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
		char szHostName[256];
		strncpy_s(szHostName,256, pHost->h_name, 256);
		// 显示用户列表
		CString strItem = CString(szHostName) + "|"+ sPeerIP+ "|"+CString(cs_port);
		m_listUserSelect.AddString(strItem);
	}

	// TODO: 在此添加控件通知处理程序代码
}

void CMainDialog::OnBnClickedButtonUpdate()
{
	UpdateClientList();
	// TODO: 在此添加控件通知处理程序代码
}
/***************************UDP开始实现****************************/
//DWORD WINAPI CMainDialog::udpRec()
//{
//	char revBuf[MaxSize];  
//	int byte = 0;  
//
//	while(1)  
//	{  
//		WaitForSingleObject(hMutex, INFINITE);  
//
//		byte= recv(sockClient,revBuf,strlen(revBuf)+1,0);//服务器从客户端接受数据  
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
//	closesocket(sockClient);//关闭socket,一次通信完毕  
//}

void CMainDialog::OnBnClickedButtonUdp()
{
	if(m_udpSocket == INVALID_SOCKET)  // 开启服务
	{
		// 取得端口号
		CString sPort;
		GetDlgItem(IDC_EDIT_UDP)->GetWindowText(sPort);
		int nPort = atoi(sPort);
		if(nPort < 1 || nPort > 65535)
		{
			MessageBox("端口号错误！");
			return;
		}


		// 创建监听套节字
		m_udpSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (m_udpSocket == SOCKET_ERROR)
		{
			MessageBox("启动服务出错！");
			return;
		}
	
		//绑定地址信息
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(nPort);
		sin.sin_addr.s_addr = INADDR_ANY;
		// 绑定端口
		if(::bind(m_udpSocket,(sockaddr*)&sin,sizeof(sin)) == SOCKET_ERROR)
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
		GetDlgItem(IDC_BUTTON_UDP)->SetWindowText("停止服务");
		//m_bar.SetText(" 正在监听……", 0, 0);
		GetDlgItem(IDC_EDIT_UDP)->EnableWindow(FALSE);
	}
	else				// 停止服务
	{
		MessageBox("UDP服务终止");
		// 关闭所有UDP连接
		CloseUDPSocket();
		// 设置相关子窗口控件状态
		GetDlgItem(IDC_BUTTON_UDP)->SetWindowText("开启服务");
		m_bar.SetText(" 空闲", 0, 0);
		GetDlgItem(IDC_EDIT_UDP)->EnableWindow(TRUE);
	}
	return;
}

DWORD WINAPI CMainDialog::udpRec(LPVOID lpParam)
{
	CMainDialog* lp_dialog = (CMainDialog*)lpParam;
	if (lp_dialog->m_udpSocket == INVALID_SOCKET)
	{
		::MessageBox(lp_dialog->m_hWnd,"UDP创建失败无法接受消息","udp",0);
	}
	::MessageBox(lp_dialog->m_hWnd,"UDP正在接受消息","udp",0);

	//设置缓冲区大小
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
	SocketAddr temp;
	while (1)
	{
		if (lp_dialog->m_udpSocket == INVALID_SOCKET)
		{
			::MessageBox(lp_dialog->m_hWnd,"UDP 接受端已经退出","udp",0);
			return 0;

		}
		//::MessageBox(lp_dialog->m_hWnd,"receive qian","udp",0);
		//接受客户端发来的数据
		ret = recvfrom(lp_dialog->m_udpSocket,buf,255,0,(sockaddr*)&clientAddr,&clentAddrLen);
		//::MessageBox(lp_dialog->m_hWnd,"receive hou","udp",0);
		//将客户端添加到UDP客户端列表当中
		temp.sock=lp_dialog->m_udpSocket;
		temp.sockAddr=clientAddr;
		lp_dialog->AddUDPClient(temp);

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
		strncpy_s(bufHome,256, pHost->h_name, 255);

		// 显示给用户
		strItem = CString("【RECV_udp】:")+CString(bufHome) + "["+ sPeerIP+ "/"+CString(cs_port)+ "]: " + CString(buf);
		lp_dialog->m_listInfo.InsertString(-1,strItem);
	}
	return 1;
}

//UDP 用来发送数据的函数
void CMainDialog::udpSend()
{
	if (m_udpSocket == INVALID_SOCKET)
	{
		MessageBox("UDP创建失败无法发送消息");
	}
	MessageBox("UDP正在发送消息");
	//获取目标用户列表长度
	int i=0;
	sockaddr_in m_socketAddr;
	//char buf[255];
	// 取得要发送的字符串
	CString sText;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sText);

	// 添加一个“回车换行”
	// 注意，添加它并不是必须的，但是如果使用本软件作为客户端调试网络协议，
	// 比如SMTP、FTP等，就要添加它了。因为这些协议都要求使用“回车换行”作为一个命令的结束标记
	sText += "udp\r\n";
	for(i=0;i<m_nUDPClient;i++)
	{

		m_socketAddr=m_saListUDP[i].sockAddr;

		// 发送数据到服务器
		if(::sendto(m_udpSocket,sText,strlen(sText),0,(SOCKADDR *)&m_socketAddr,sizeof(SOCKADDR)) != -1)
		{
			//发送成功
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
		// 添加新的成员
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

	// 如果找到就将此成员从列表中移除
	if(bFind)
	{
		m_nUDPClient--;
		// 将此成员后面的成员都向前移动一个单位
		for(int j=i; j<m_nUDPClient; j++)
		{
			m_saListUDP[j] = m_saListUDP[j+1];
		}
	}
}
void CMainDialog::OnBnClickedButtonUdpSedn()
{
	//取出要发送的字符串
	// 取得要发送的字符串
	CString sText;
	GetDlgItem(IDC_EDIT1)->GetWindowText(sText);

	// 添加一个“回车换行”
	sText += "\r\n";
	// TODO: 在此添加控件通知处理程序代码
	int i=0;
	for(i=0;i<m_nUDPClient;i++)
	{
		//目的地址信息
		sockaddr_in sin=m_saListUDP[i].sockAddr;


		// 发送数据到服务器
		if(::sendto(m_udpSocket,sText,strlen(sText),0,(SOCKADDR *)&sin,sizeof(SOCKADDR))!= -1)
		{
			//发送成功
		}

	}
	//AddStringToList(sText, FALSE);
	sText = "【SEND_udp】:"+sText;
	m_listInfo.InsertString(-1,sText);
	GetDlgItem(IDC_EDIT1)->SetWindowText("");

}
