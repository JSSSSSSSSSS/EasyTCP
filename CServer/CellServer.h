#ifndef __CellServer__
#define __CellServer__

#include<map>
#include<list>
#include<mutex>
#include<thread>
#include"Alloctor.h"
#include"wsabase.h"

class INetEvent;
class CELLClient;
class MsgHeader;
class CellTask;
class CellTaskServer;

//�ӷ����������տͻ��˷��͵�����
class CellServer
{
public:
	CellServer(INetEvent* pevent = NULL);
	~CellServer();

	//�����������߳�
	void Start();
	//��ȡcellserverʵ�ʷ���Ŀͻ�������
	size_t GetClientCount();
	//������������ӿͻ���
	void AddClient(std::shared_ptr<CELLClient> client);
	//��ӷ�������
	void addSendTask(std::shared_ptr<CELLClient> client, MsgHeader * msg);
	//�Ƿ�����
	bool IsRun();
private:
	//�����к���������select�����ݽ���
	void OnRun();
	//�Ͽ�ָ���ͻ��˵�����
	void Disconnect(CELLClient * pclient);
	//��ȡ�ͻ��˵�����
	void ReadClients(fd_set &cloneReadSet,fd_set &readset);
	//���ͻ��˵�����
	void CheckClientHeart(fd_set &readset);
	//��������
	int RecvMsg(std::shared_ptr<CELLClient> pclient);
	//��Ӧ������Ϣ
	virtual void OnNetMsg(std::shared_ptr<CELLClient> pclient, MsgHeader*msg);
private:
	std::map<SOCKET, std::shared_ptr<CELLClient>>	_client;		//��ʽ�ͻ��˶���
	std::list<std::shared_ptr<CELLClient>>		_clientbuf;		//����ͻ��˶���
	std::mutex										_mtx;			//���������
	std::thread*									_pThread;		//�߳�
	INetEvent*										_pevent;		//�¼�
	CellTaskServer*									_taskServer;	//���������
	time_t											_oldTime;		//��ʱ���
};

//int a = sizeof(CellServer);	//144
#endif // !__CellServer__


