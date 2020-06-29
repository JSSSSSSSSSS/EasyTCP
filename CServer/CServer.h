#pragma once
#include"../Winsocket/wsabase.h"
#include"../Winsocket/CELLTimestamp.h"
#include<list>
#include<vector>
#include<mutex>
#include<atomic>
#include<thread>
#define THREAD_COUNT 4
struct MsgHeader;

//�ͻ�������
class ClientSocket
{
public:
	ClientSocket(Socket *socket = NULL);	
	~ClientSocket();
	//��ÿͻ���socket
	Socket *GetSocket()const;
	//��ȡ��������С
	char* GetBuf();
	//��ȡ������βָ��λ��
	int GetLastPos()const;
	//���û�����βָ��
	void SetLastPos(int newpos);
private:
	Socket			*_socket;					//socket��addr��װ����
	char			*_msgbuf;					//�ڶ�������
	int				_lastpos;					//������β��λ��
};

//�����¼�
class INetEvent
{
public:
	INetEvent();
	virtual ~INetEvent();
	//�ͻ����˳��¼�
	virtual void OnLeave(ClientSocket* pclient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(ClientSocket* pclient, MsgHeader *msg) = 0;
};


//�ӷ�������������տͻ��˷��͵�����
class CellServer
{
public:
	CellServer(Socket *socket = NULL, INetEvent* pevent = NULL);
	~CellServer();
	
	//�����������߳�
	void Start();
	//��ȡcellserverʵ�ʷ���Ŀͻ�������
	size_t GetClientCount();
	//������������ӿͻ���
	void AddClient(ClientSocket* pclient);

private:
	//�����к���������select�����ݽ���
	void OnRun();
	//�Ƿ�����
	bool IsRun();
	//�Ͽ�ָ���ͻ��˵�����
	void Disconnect(ClientSocket * pclient);
	//�Ͽ����пͻ��ˣ�����ֹ����������
	void terminated();
	//��������
	int RecvMsg(ClientSocket* pclient);
	//��Ӧ������Ϣ
	virtual void OnNetMsg(ClientSocket* pclient, MsgHeader*msg);

private:
	std::list<ClientSocket*>	_client;		//�ͻ��˶���
	std::list<ClientSocket*>	_clientbuf;		//����ͻ��˶���
	Socket *					_socket;		//socket��addr
	char *						_recvbuf;		//���ջ�����
	std::thread*				_pThread;		//�߳�
	INetEvent*					_pevent;		//�¼�
	std::mutex					_mtx;			//���������
};

//������������������������ҵ��
class Server:public wsabase,public INetEvent
{
public:
	Server();
	virtual ~Server();
	//��ʼ�������׽���
	bool InitSocket(int af = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
	//�󶨶˿�
	bool Bind(unsigned int port);
	//�����˿�
	bool Listen();
	//���ܿͻ�������,Ϊ�ͻ��˷���ռ�
	void Accept();
	//�����ӷ�������������
	void StartCell();
	//�����յĿͻ�����ӵ��ӷ�����������
	void AddClientToCellServer(ClientSocket*client);
	//��Ϣ����
	void MsgCount();
	//������Ϣ
	int SendMsg(ClientSocket*client,MsgHeader*msg);
	//�����к���
	void Run();
	//�Ƿ�����
	bool IsRun()const;
	//��������
	void terminated();
	//��ȡ�����׽���
	SOCKET GetListenSocket()const;
	//�ӷ������пͻ��˶Ͽ��¼���ͳһ�ͷſͻ�����Դ
	virtual void OnLeave(ClientSocket* pclient);
	//�ӷ������пͻ�����Ϣ��Ӧ�¼�
	virtual void OnNetMsg(ClientSocket* pclient, MsgHeader *msg);
private:
	Socket *					_socket;		//socket��addr
	std::list<ClientSocket*>	_client;		//�ͻ��˶���
	std::vector<CellServer*>	_CellServer;	//С����������
	std::mutex					_mtx;			//�ͻ��˶�����,��ΪOnLeave�������̲߳�����_client
	CELLTimestamp				_timer;			//�߾��ȼ�ʱ��
	std::atomic_int				_recvcount;		//�հ�������
};

