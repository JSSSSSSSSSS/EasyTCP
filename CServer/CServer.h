#pragma once
#ifndef __SERVER__
#define __SERVER__


#include<vector>
#include<atomic>
#include<memory>

#include"wsabase.h"
#include"CELLTimestamp.h"
#include"INetEvent.h"
class CellServer;

/*
����Ҫ�㣺
	1.����Ӧ�����û�������
		��Ϊ�������������ӵ�����£����ܳ����ٰ���ճ���������

	2.���պͷ���Ӧ���ڲ�ͬ�̡߳��������ߡ���������ģʽ��
		��Ϊ����ͬʱ���ڣ�send�ĵ��ô���ԶԶ����recv�Ĵ�����
		��ͬһ���߳���ͬʱ����send��recv�Ļ���send��Ӱ��recv�����ܡ�
		sendͨ��������ƿ����

	3.���տͻ�����ͻ��˵ľ���ҵ��Ӧ�÷ֿ����������ߡ���������ģʽ��
		�ͻ��˵�����Ӧ�õ���ʹ��һ���̣߳�ͨ���������������Ѿ����ӵĿͻ��ˡ��������ߡ���������ģʽ��
		ͨ���ж���������̣߳�����ÿ��������ӵ���Լ��Ļ��������������Լ��ټ������ͷ����Ĵ�����
		���������ڴ������пͻ��˵�ͬʱ�����Ӽ�ʱ�ؽ����¿ͻ��˵����ӡ�

	4.��ʱ�������������ݡ�
		����send�ĵ��ô�����
*/


/*
�µ����⣺pclient�����Ѿ����ͷţ�����send����û����ɡ�
	����ָ����Խ��
*/

//������������������������ҵ��
class Server:public wsabase,public INetEvent,public Socket
{
public:
	Server();
	virtual ~Server();
	//�󶨶˿�
	bool Bind(unsigned int port);
	//�����˿�
	bool Listen();
	//���ܿͻ�������,Ϊ�ͻ��˷���ռ�
	void Accept();
	//�����ӷ�������������
	void StartCell(int ThreadCount);
	//�����յĿͻ�����ӵ��ӷ�����������
	void AddClientToCellServer(std::shared_ptr<CELLClient> client);
	//��Ϣ����
	void MsgCount();
	//�����к���
	void Run();
	//�Ƿ�����
	bool IsRun()const;
	//��������
	void Close();
	//��ȡ�����׽���
	SOCKET GetListenSocket()const;

	//�ͻ��˼�����Ϣ
	virtual void OnJoin(CELLClient* pclient);
	//�ӷ������пͻ��˶Ͽ��¼���ͳһ�ͷſͻ�����Դ
	virtual void OnLeave(CELLClient* pclient);
	//�ӷ������пͻ�����Ϣ��Ӧ�¼�
	virtual void OnNetMsg(CellServer* cellserver, std::shared_ptr<CELLClient> pclient, MsgHeader * msg);
private:
	std::vector<CellServer*>	_CellServer;	//�ӷ���������
	CELLTimestamp				_timer;			//�߾��ȼ�ʱ��
	std::atomic_int				_recvcount;		//�հ�����
	std::atomic_int				_clientcount;	//�ͻ��˼���
};

//int a = sizeof(Server);	//88
#endif // !__SERVER__