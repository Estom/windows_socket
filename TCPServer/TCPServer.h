///////////////////////////////////////////
// TCPServer.h�ļ�

#include <afxwin.h>		// 10TCPServer	
#include <afxcmn.h>
#include <winsock2.h> 

// ������������WS2_32������
#pragma comment(lib,"WS2_32.lib")

#define MAX_SOCKET 56	// ����˷��������ܽ��ܵ����ͻ���

class CMyApp : public CWinApp
{
public:
	BOOL InitInstance();
};
//������һ�����ӽṹ��
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
	//��Ҫ�����˶Կͻ���socket�Ĳ���,�����Ǳ�����Ƶķ��¼��߼�����Ҫ�Լ�����
	// �����׽��֣�������Ϊ����״̬��׼�����ܿͻ�������
	BOOL CreateAndListen(int nPort);
	// �ر������׽��֣����������׽��ֺ�����accept�������ص��׽���
	void CloseAllSocket();
	// �ر�����UDP �׽��֣�����ֹUDP���߳�
	void CloseUDPSocket();

	//������Ҫ����˶��û��б�Ĳ�������ӣ�ɾ���������û��б�
	// ��ͻ������б������һ���ͻ��������б���ʾ���û��б���
	BOOL AddClient(SocketAddr sockAddr);
	// �ӿͻ������б����ƴ�һ���ͻ�������User���û��б���ɾ��
	void RemoveClient(SocketAddr sockAddr);
	//�����û��б������
	void UpdateClientList();

	//ʵ�ַ�����Ϣ��һϵ�к���
	void AddStringToList(LPCTSTR pszText, BOOL bRecv = TRUE);

	//UDP �ȴ��������ݵĺ���
	static DWORD WINAPI udpRec(LPVOID lpParam);
	//UDP �����������ݵĺ���
	void udpSend();
	//���UDP�û�
	BOOL AddUDPClient(SocketAddr sockAddr);
	//�Ƴ�UDP�û�
	void RemoveUDPClient(SocketAddr sockAddr);


protected:
	//���ڵĹ��ɾ��
	// �����Ӵ��ڿؼ���һ����״̬����һ�����б��
	CStatusBarCtrl m_bar;
	CListBox m_listInfo;
	
	//�����û��б�Ĵ��ڿռ�
	CListBox m_listUser;
	//���ñ�ѡ�����û��б���
	CListBox m_listUserSelect;
	// �����׽��־��
	SOCKET m_socket;
	// ����UDP�׽��ֵľ��
	SOCKET m_udpSocket;

	// �ͻ������б�
	SOCKET m_arClient[MAX_SOCKET];	// �׽�������
	//������¼���ӵ�socket�����ַ
	SocketAddr m_saList[MAX_SOCKET];
	// ��������Ĵ�С
	int m_nClient;
	//��ѡ�е��б���
	SocketAddr m_saListSelect[MAX_SOCKET];
	//UDP��ַ�б�
	SocketAddr m_saListUDP[MAX_SOCKET];
	//��������Ĵ�С
	int m_nUDPClient;
	//UDP�߳�
	HANDLE h_udpThread;
	
protected:
	//��Ϣ��Ӧ��������Ϣ��������
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	// ������ֹͣ����
	afx_msg void OnStart();
	// �����Ϣ
	afx_msg void OnClear();
	// �׽���֪ͨ�¼�
	afx_msg long OnSocket(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonSelect();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonUdp();
	afx_msg void OnBnClickedButtonUdpSedn();
};