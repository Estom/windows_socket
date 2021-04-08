///////////////////////////////////////////
// TCPServer.h文件

#include <afxwin.h>		// 10TCPServer	
#include <afxcmn.h>
#include <winsock2.h> 

// 告诉连接器与WS2_32库连接
#pragma comment(lib,"WS2_32.lib")

#define MAX_SOCKET 56	// 定义此服务器所能接受的最大客户量

class CMyApp : public CWinApp
{
public:
	BOOL InitInstance();
};
//定义了一个链接结构体
typedef struct{
	SOCKET sock;
	sockaddr_in sockAddr;
	//char hostName[256];
} SocketAddr;

class CMainDialog : public CDialog
{
public:
	CMainDialog(CWnd* pParentWnd = NULL);
	
protected:
	//主要定义了对客户端socket的操作,这里是必须设计的非事件逻辑，需要自己处理。
	// 创建套节字，并设置为监听状态，准备接受客户的连接
	BOOL CreateAndListen(int nPort);
	// 关闭所有套节字，包括监听套节字和所有accept函数返回的套节字
	void CloseAllSocket();
	// 关闭所有UDP 套接字，并终止UDP的线程
	void CloseUDPSocket();

	//这里主要完成了对用户列表的操作，添加，删除，更新用户列表。
	// 向客户连接列表中添加一个客户，并将列表显示到用户列表当中
	BOOL AddClient(SocketAddr sockAddr);
	// 从客户连接列表中移处一个客户，并将User从用户列表中删除
	void RemoveClient(SocketAddr sockAddr);
	//更新用户列表的内容
	void UpdateClientList();

	//实现发送消息的一系列函数
	void AddStringToList(LPCTSTR pszText, BOOL bRecv = TRUE);

	//UDP 等待接收数据的函数
	static DWORD WINAPI udpRec(LPVOID lpParam);
	//UDP 用来发送数据的函数
	void udpSend();
	//添加UDP用户
	BOOL AddUDPClient(SocketAddr sockAddr);
	//移除UDP用户
	void RemoveUDPClient(SocketAddr sockAddr);


protected:
	//窗口的构成句柄
	// 两个子窗口控件，一个是状态栏，一个是列表框
	CStatusBarCtrl m_bar;
	CListBox m_listInfo;
	
	//设置用户列表的窗口空间
	CListBox m_listUser;
	//设置被选定的用户列表窗口
	CListBox m_listUserSelect;
	// 监听套节字句柄
	SOCKET m_socket;
	// 监听UDP套接字的句柄
	SOCKET m_udpSocket;

	// 客户连接列表
	SOCKET m_arClient[MAX_SOCKET];	// 套节字数组
	//用来记录连接的socket和其地址
	SocketAddr m_saList[MAX_SOCKET];
	// 上述数组的大小
	int m_nClient;
	//被选中的列表项
	SocketAddr m_saListSelect[MAX_SOCKET];
	//UDP地址列表
	SocketAddr m_saListUDP[MAX_SOCKET];
	//上述数组的大小
	int m_nUDPClient;
	//UDP线程
	HANDLE h_udpThread;
	
protected:
	//消息响应机制中消息处理函数。
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	// 开启或停止服务
	afx_msg void OnStart();
	// 清空信息
	afx_msg void OnClear();
	// 套节字通知事件
	afx_msg long OnSocket(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonSelect();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonUdp();
	afx_msg void OnBnClickedButtonUdpSedn();
};