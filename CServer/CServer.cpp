#include"CServer.h"
#include"../Winsocket/Msg.h"
#include<iostream>
#include<assert.h>
#include<thread>
using namespace std;

ClientSocket::ClientSocket(Socket *socket)
	:_socket(socket),_msgbuf(NULL)
{
	_msgbuf = new char[RECV_BUF_SIZE * 10];
	assert(_msgbuf != NULL);
	memset(_msgbuf, 0, RECV_BUF_SIZE * 10);
	_lastpos = 0;
}
ClientSocket::~ClientSocket()
{
	if (_socket)
	{
		delete _socket;
		_socket = NULL;
	}
	if (_msgbuf)
	{
		delete _msgbuf;
		_msgbuf = NULL;
	}
}
Socket* ClientSocket::GetSocket() const
{
	return _socket;
}
char* ClientSocket::GetBuf()
{
	return _msgbuf;
}
int ClientSocket::GetLastPos()const
{
	return _lastpos;
}
void ClientSocket::SetLastPos(int newpos)
{
	_lastpos = newpos;
}


INetEvent::INetEvent()
{
}

INetEvent::~INetEvent()
{
}





CellServer::CellServer(Socket *socket,INetEvent* pevent)
	:_socket(socket), _pThread(NULL), _pevent(NULL)
{
	_pevent = pevent;
	_recvbuf = new char[RECV_BUF_SIZE];
	assert(_recvbuf != NULL);
}

CellServer::~CellServer()
{
	//��ֹ�߳�
	if (_pThread)
	{	
		_socket = NULL;
		_pThread->join();
		delete _pThread;
	}
	//��������
	if (_recvbuf)
	{
		delete _recvbuf;
		_recvbuf = NULL;
	}
	//����ʣ�໺�����
	for (auto &client : _clientbuf)
	{
		Disconnect(client);
		_pevent->OnLeave(client);
	}
	_clientbuf.clear();
	//����ʣ�����
	for (auto &client : _client)
	{
		Disconnect(client);
		_pevent->OnLeave(client);
	}
	_client.clear();
}

void CellServer::OnRun()
{
	Selector setor(0);
	while (IsRun())
	{
		//���������еĿͻ��˼��뵽��ʽ����
		_mtx.lock();
		for (auto client : _clientbuf)
		{
			_client.push_back(client);
		}
		_clientbuf.clear();
		_mtx.unlock();
		
		//���û�����ݾͲ���select
		if (_client.empty())
		{
			std::chrono::milliseconds t(1000);
			std::this_thread::sleep_for(t);
			continue;
		}
		//��ʼselect
		setor.ClearAll();
		//����ͻ����׽���
		for (auto client : _client)
		{
			setor.AddReadFd(client->GetSocket()->GetSocketFd());
			//setor.AddWriteFd(client->GetSocket()->GetSocketFd());
			//setor.AddExceptFd(client->GetSocket()->GetSocketFd());
		}
		//��ʼѡ��
		int ret = setor.Select();
		//�����״̬�ı���׽���
		if (ret > 0)
		{
			//����ͻ���
			auto begin = _client.begin();
			auto end = _client.end();

			for (auto client = begin; client != end;)
			{
				SOCKET socket = (*client)->GetSocket()->GetSocketFd();
				//����ͻ����׽��ֿɶ�
				if (setor.IsReadSet(socket))
				{
					int relen = RecvMsg(*client);
					//������յ�EOF
					if (relen <= 0)
					{
						//�Ƴ��ͻ���
						cout << "client<" << socket << ">���˳�" << endl;
						Disconnect((*client));
						client = _client.erase(client);
						continue;
					}
				}
				//����ͻ����׽��ֿ�д
				//if (setor.IsWriteSet((*client)->GetSocket()->GetSocketFd()))
				//{
				//	cout << "write" << endl;
				//	//��������,��Ӧ�ͻ���
				//}
				//����ͻ����׽����쳣
				//if (setor.IsExceptSet((*client)->GetSocket()->GetSocketFd()))
				//{
				//	//�Ƴ��ͻ���
				//	cout << "client<" << (*client)->GetSocket()->GetSocketFd() << ">���˳�" << endl;
				//	Disconnect((*client));
				//	client = _client.erase(client);
				//	continue;
				//}
				client++;
			}
		}
		else if(ret < 0)
		{
			return;
		}
	}
	cout << "cellserver exit" << endl;
}

bool CellServer::IsRun()
{
	return (_socket);
}

void CellServer::Disconnect(ClientSocket * pclient)
{
	if (pclient)
	{
		if (_pevent)
		{
			_pevent->OnLeave(pclient);
		}
	}
}

void CellServer::terminated()
{
	if (_socket)
	{
		_socket = NULL;
	}
	for (auto &client : _client)
	{
		Disconnect(client);
		client = NULL;
	}
	_client.clear();
	//����ʣ�໺�����
	for (auto &client : _clientbuf)
	{
		Disconnect(client);
		client = NULL;
	}
	_clientbuf.clear();
}

void CellServer::Start()
{
	//����Ա����ת��Ϊ��������ͨ��ָ���
	_pThread = new thread(std::mem_fun(&CellServer::OnRun), this);
}

int CellServer::RecvMsg(ClientSocket * pclient)
{
	//һ���Խ����㹻�������
	int recvlen = recv(pclient->GetSocket()->GetSocketFd(), _recvbuf, RECV_BUF_SIZE, 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}
	//������������
	memcpy(pclient->GetBuf() + pclient->GetLastPos(), _recvbuf, recvlen);
	//���»�����β��λ��
	pclient->SetLastPos(pclient->GetLastPos() + recvlen);
	//�жϻ��������Ƿ��㹻һ����Ϣͷ�ĳ���
	while (pclient->GetLastPos() >= sizeof(MsgHeader))
	{
		//ȡ����Ϣͷ
		MsgHeader *header = reinterpret_cast<MsgHeader*>(pclient->GetBuf());
		//�жϻ����������Ƿ���ڵ��������Ϣ��
		int len = header->length;
		if (pclient->GetLastPos() >= len)
		{
			//�������ʹ��������Ϣ
			OnNetMsg(pclient, header);
			//����ʣ�໺������С
			int nsize = pclient->GetLastPos() - len;
			//������������ǰ��
			memcpy(pclient->GetBuf(), pclient->GetBuf() + len, nsize);
			//���»�����β��ָ��
			pclient->SetLastPos(nsize);
		}
	}
	return recvlen;
}
void CellServer::OnNetMsg(ClientSocket* pclient, MsgHeader * msg)
{
	//֪ͨ�������߳�
	_pevent->OnNetMsg(pclient, msg);
	switch (msg->type)
	{
		case MT_LOGIN:break;
	}
}

size_t CellServer::GetClientCount()
{
	//��ȡ��ʽ�����뻺������е����пͻ���
	size_t size1 = _client.size();
	std::lock_guard<std::mutex> lock(_mtx);
	size_t size2 = _clientbuf.size();
	return size1+size2;
}

void CellServer::AddClient(ClientSocket * pclient)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_clientbuf.push_back(pclient);
}

Server::Server()
	:_socket(0),_recvcount(0)
{
	
}

Server::~Server()
{
	if (_socket)
	{
		delete _socket;
		_socket = NULL;
	}
	for (auto &client : _client)
	{
		if (client)
		{
			delete client;
		}
	}
	_client.clear();
	for (auto &cell : _CellServer)
	{
		if (cell)
		{
			delete cell;
		}
	}
	_CellServer.clear();
}

bool Server::InitSocket(int af, int type, int protocol)
{
	if (_socket)
	{
		delete _socket;
		_socket = NULL;
	}
	_socket = new Socket();
	if (!_socket)
	{
		return false;
	}
	return _socket->InitSocket(af,type,protocol);
}

bool Server::Bind(unsigned int port)
{
	//bind IP addr and port
	sockaddr_in addr;
	addr.sin_family = _socket->GetAddr().sin_family;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	_socket->SetAddr(addr);

	if (::bind(_socket->GetSocketFd(), (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		LOG("bind port failed!");
		return false;
	}
	return true;
}

bool Server::Listen()
{
	//start listen
	//SOMAXCONN means the maximum queue length specifiable by listen.
	if (listen(_socket->GetSocketFd(), SOMAXCONN) == SOCKET_ERROR)
	{
		LOG("listen failed!");
		return false;
	}
	else
	{
		LOG("listen success...");
		//�����ӷ������߳�
		StartCell();
		return true;
	}
}

void Server::Accept()
{
	//start accept
	sockaddr_in clientaddr = { 0 };
	socklen_t len = sizeof(clientaddr);
	SOCKET accSock = accept(_socket->GetSocketFd(), (sockaddr *)&clientaddr, &len);
	if (accSock == INVALID_SOCKET)
	{
		LOG("accept a socket failed!");
	}
	else
	{
		Socket *socket = new Socket(accSock, clientaddr);
		if (socket)
		{
			ClientSocket *pclient = new ClientSocket(socket);
			if (pclient)
			{
				AddClientToCellServer(pclient);
			}
			else
			{
				delete socket;
				LOG("failed to new a ClientSocket!");
			}
		}
	}
}

void Server::StartCell()
{
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		auto cell = new CellServer(_socket,this);
		_CellServer.push_back(cell);
		cell->Start();
	}
}

void Server::AddClientToCellServer(ClientSocket * pclient)
{
	//���ͻ��˼��뵽��С�������
	_mtx.lock();
	_client.push_back(pclient);
	_mtx.unlock();

	auto mincellserver = _CellServer[0];
	for (auto &cellserver: _CellServer)
	{
		if (cellserver->GetClientCount() < mincellserver->GetClientCount())
		{
			mincellserver = cellserver;
		}
	}
	mincellserver->AddClient(pclient);
}

void Server::MsgCount()
{
	auto t = _timer.GetElapsedSecond();
	if (t >= 1.0)
	{
		_mtx.lock();
		cout << "timesegment<" << t << ">,client<" << _client.size() << ">,recvcount<" << (int)(_recvcount /t) << ">" << endl;
		_mtx.unlock();
		_recvcount = 0;
		_timer.Update();
	}
}

void Server::Run()
{
	MsgCount();
	Selector setor(100);

	SOCKET listensocket = _socket->GetSocketFd();
	//��������׽���
	if (IsRun())
	{	
		setor.AddReadFd(listensocket);
		//setor.AddExceptFd(_socket->GetSocketFd());
	}
	
	//��ʼѡ��
	int ret = setor.Select();
	//�����״̬�ı���׽���
	if (ret > 0)
	{
		//��������׽��ֿɶ�
		if (setor.IsReadSet(listensocket))
		{
			Accept();
		}
		//��������׽����쳣
		if (setor.IsExceptSet(listensocket))
		{
			cout << "listen socket except" << endl;
			terminated();
			return;	//����ѭ����������
		}
	}
	else if(ret < 0)
	{
		terminated();
		return;
	}
}

bool Server::IsRun() const
{
	return (_socket);
}

void Server::terminated()
{
	if (_socket)
	{
		delete _socket;
		_socket = NULL;
	}
}

int Server::SendMsg(ClientSocket * client, MsgHeader * msg)
{
	int sendlen = send(client->GetSocket()->GetSocketFd(), (char*)msg, msg->length, 0);
	return sendlen;
}
int Server::SendMsgToAll(MsgHeader * msg)
{
	int sendlen = 0;
	lock_guard<mutex> lock(_mtx);
	for (auto client : _client)
	{
		sendlen = send(client->GetSocket()->GetSocketFd(), (char*)msg, msg->length, 0);
	}
	return sendlen;
}

SOCKET Server::GetListenSocket() const
{
	SOCKET socket = _socket->GetSocketFd();
	return socket;
}

void Server::OnLeave(ClientSocket * pclient)
{
	if (pclient)
	{
		delete pclient;
	}
	lock_guard<mutex> lock(_mtx);
	auto begin = _client.begin();
	auto end = _client.end();
	for (auto iter = begin; iter != end;iter++)
	{
		if (*iter == pclient)
		{
			_client.erase(iter);
			break;
		}
	}
}

void Server::OnNetMsg(ClientSocket * pclient, MsgHeader * msg)
{
	_recvcount++;
}
