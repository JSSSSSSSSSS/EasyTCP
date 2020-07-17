#include<iostream>
#include<assert.h>
using namespace std;

#include"Msg.h"
#include "CellServer.h"
#include"CELLClient.h"
#include"CServer.h"
#include"CELLTask.h"
#include"CELLTimestamp.h"

CellServer::CellServer(INetEvent* pevent)
	: _pThread(NULL), _pevent(pevent),_taskServer(NULL), _oldTime(CELLTime::GetNowTimeMiliSec())
{
	_taskServer = new CellTaskServer(this);
	assert(_taskServer != NULL);
}

CellServer::~CellServer()
{
	//��ֹ�߳�
	if (_pThread)
	{
		_pThread->join();
		delete _pThread;
	}
	//����ʣ�໺�����
	for (auto &client : _clientbuf)
	{
		cout << "in ~cellserver" << endl;
		Disconnect(client.get());
		client = NULL;
	}
	_clientbuf.clear();
	//����ʣ�����
	for (auto &client : _client)
	{
		cout << "in ~cellserver" << endl;
		Disconnect(client.second.get());
	}
	_client.clear();
	if (_taskServer)
	{
		delete _taskServer;
	}
}

void CellServer::OnRun()
{
	fd_set readset;
	FD_ZERO(&readset);
	bool ischange = true;
	int ret = 0;
	int maxsocketfd = 0;
	timeval timeout = { 0,1 };
	while (IsRun())
	{
		//���������еĿͻ��˼��뵽��ʽ����
		_mtx.lock();
		for (auto client : _clientbuf)
		{
			_client[client->GetSocketfd()] = client;
			if (client->GetSocketfd() > maxsocketfd)
			{
				maxsocketfd = client->GetSocketfd();
			}
			FD_SET(client->GetSocketfd(), &readset);
		}
		_clientbuf.clear();
		_mtx.unlock();

		//���û�����ݾͲ���select
		if (_client.empty())
		{
			std::chrono::milliseconds t(1000);
			std::this_thread::sleep_for(t);
			//���¾�ʱ���
			_oldTime = CELLTime::GetNowTimeMiliSec();
			continue;
		}
		//��ʼselect
		fd_set cloneReadSet;
		//fd_set cloneExcptSet;
		memcpy(&cloneReadSet, &readset, sizeof(fd_set));
		//memcpy(&cloneExcptSet, &readset, sizeof(fd_set));

		//��ʼѡ��
		ret = select(maxsocketfd + 1, &cloneReadSet, 0, 0, &timeout);
		//�����״̬�ı���׽���
		if (ret > 0)
		{
			//��ȡ�ͻ��˵����ݣ����Ҹ���readset
			ReadClients(cloneReadSet, readset);
			//���ͻ��˵�����
			CheckClientHeart(readset);
		}
		else if (ret < 0)
		{
			break;
		}
	}
	cout << "cellserver exit" << endl;
}

bool CellServer::IsRun()
{
	return (((Server*)_pevent)->GetSocketfd() != INVALID_SOCKET);
}

void CellServer::Disconnect(CELLClient * pclient)
{
	if (pclient)
	{
		_pevent->OnLeave(pclient);
	}
}

void CellServer::ReadClients(fd_set &cloneReadSet,fd_set & readset)
{
#ifdef _WIN32
	for (int i = 0; i < cloneReadSet.fd_count; i++)
	{
		std::shared_ptr<CELLClient> pclient = _client[cloneReadSet.fd_array[i]];
		int relen = RecvMsg(pclient);
		//������յ�EOF
		//if (relen <= 0)
		//{
		//	//����fd_set
		//	FD_CLR(cloneReadSet.fd_array[i], &readset);
		//	//�Ͽ��ͻ�������
		//	Disconnect(pclient.get());
		//	//�Ƴ��ͻ���
		//	_client.erase(cloneReadSet.fd_array[i]);
		//	continue;
		//}
	}
#else

	//����ͻ���
	auto begin = _client.begin();
	auto end = _client.end();

	for (auto client = begin; client != end;)
	{
		SOCKET socket = (*client).first;
		//����ͻ����׽��ֿɶ�
		if (FD_ISSET(socket, &cloneReadSet))
		{
			int relen = RecvMsg((*client).second);
			//������յ�EOF
			if (relen <= 0)
			{
				//����fd_set
				FD_CLR(socket, &readset);
				//�Ͽ��ͻ�������
				Disconnect((*client).second.get());
				//�Ƴ��ͻ���
				client = _client.erase(client);
				continue;
			}
		}
		client++;
	}
#endif // _WIN32
}

void CellServer::CheckClientHeart(fd_set &readset)
{
	std::map<SOCKET, shared_ptr<CELLClient>>::iterator begin = _client.begin();
	std::map<SOCKET, shared_ptr<CELLClient>>::iterator end = _client.end();
	//��ȡ��ǰʱ��
	time_t nowTime = CELLTime::GetNowTimeMiliSec();
	//��ȡʱ����
	time_t dt = nowTime - _oldTime;

	for (auto iter = begin; iter != end; )
	{
		//�������
		if (!iter->second->CheckDtHeart(dt))
		{
			//����fd_set
			FD_CLR(iter->second->GetSocketfd(), &readset);
			//�Ͽ��ͻ�������
			Disconnect((*iter).second.get());
			//�Ƴ��ͻ���
			iter = _client.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	//���¾�ʱ���
	_oldTime = CELLTime::GetNowTimeMiliSec();
}

void CellServer::Start()
{
	//����Ա����ת��Ϊ��������ͨ��ָ���
	_pThread = new thread(&CellServer::OnRun, this);
	//�������������
	_taskServer->start();
}

int CellServer::RecvMsg(std::shared_ptr<CELLClient> pclient)
{
	//ֱ��ʹ�ÿͻ��˵Ļ�����
	char* recvbuf = pclient->GetRecvBuf() + pclient->GetRecvLastPos();
	//һ���Խ����㹻�������
	int recvlen = recv(pclient->GetSocketfd(), recvbuf, (RECV_BUF_SIZE)-pclient->GetRecvLastPos(), 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}

	//�յ��κ����ݣ���������
	pclient->ResetDtHeart();

	//������������
	//memcpy(pclient->GetBuf() + pclient->GetLastPos(), _recvbuf, recvlen);
	//���»�����β��λ��
	pclient->SetRecvLastPos(pclient->GetRecvLastPos() + recvlen);
	//�жϻ��������Ƿ��㹻һ����Ϣͷ�ĳ���
	while (pclient->GetRecvLastPos() >= sizeof(MsgHeader))
	{
		//ȡ����Ϣͷ
		MsgHeader *header = (MsgHeader*)(pclient->GetRecvBuf());
		//�жϻ����������Ƿ���ڵ��������Ϣ��
		int len = header->length;
		if (pclient->GetRecvLastPos() >= len)
		{
			//�������ʹ��������Ϣ
			OnNetMsg(pclient, header);
			//����ʣ�໺������С
			int nsize = pclient->GetRecvLastPos() - len;
			//������������ǰ��
			memcpy(pclient->GetRecvBuf(), pclient->GetRecvBuf() + len, nsize);
			//���»�����β��ָ��
			pclient->SetRecvLastPos(nsize);
		}
		else
		{
			//��Ϣ����
			break;
		}
	}
	return recvlen;
}
void CellServer::OnNetMsg(std::shared_ptr<CELLClient> pclient, MsgHeader * msg)
{
	//֪ͨ�������߳�
	_pevent->OnNetMsg(this,pclient, msg);
}

void CellServer::addSendTask(std::shared_ptr<CELLClient> client, MsgHeader * msg)
{
	//���췢�����񣬲����뵽�б�
	_taskServer->addTask([client, msg]() {
		client->SendMsg(msg);
		delete msg; });
}

size_t CellServer::GetClientCount()
{
	//��ȡ��ʽ�����뻺������е����пͻ���
	size_t size1 = _client.size();
	std::lock_guard<std::mutex> lock(_mtx);
	size_t size2 = _clientbuf.size();
	return size1 + size2;
}

void CellServer::AddClient(std::shared_ptr<CELLClient> client)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_clientbuf.push_back(client);
}

