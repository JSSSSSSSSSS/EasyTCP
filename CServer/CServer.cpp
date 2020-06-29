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
	//终止线程
	if (_pThread)
	{	
		_socket = NULL;
		_pThread->join();
		delete _pThread;
	}
	//清理缓冲区
	if (_recvbuf)
	{
		delete _recvbuf;
		_recvbuf = NULL;
	}
	//清理剩余缓冲队列
	for (auto &client : _clientbuf)
	{
		Disconnect(client);
		_pevent->OnLeave(client);
	}
	_clientbuf.clear();
	//清理剩余队列
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
		//将缓冲区中的客户端加入到正式队列
		_mtx.lock();
		for (auto client : _clientbuf)
		{
			_client.push_back(client);
		}
		_clientbuf.clear();
		_mtx.unlock();
		
		//如果没有数据就不用select
		if (_client.empty())
		{
			std::chrono::milliseconds t(1000);
			std::this_thread::sleep_for(t);
			continue;
		}
		//开始select
		setor.ClearAll();
		//加入客户端套接字
		for (auto client : _client)
		{
			setor.AddReadFd(client->GetSocket()->GetSocketFd());
			//setor.AddWriteFd(client->GetSocket()->GetSocketFd());
			//setor.AddExceptFd(client->GetSocket()->GetSocketFd());
		}
		//开始选择
		int ret = setor.Select();
		//如果有状态改变的套接字
		if (ret > 0)
		{
			//处理客户端
			auto begin = _client.begin();
			auto end = _client.end();

			for (auto client = begin; client != end;)
			{
				SOCKET socket = (*client)->GetSocket()->GetSocketFd();
				//如果客户端套接字可读
				if (setor.IsReadSet(socket))
				{
					int relen = RecvMsg(*client);
					//如果接收到EOF
					if (relen <= 0)
					{
						//移除客户端
						cout << "client<" << socket << ">已退出" << endl;
						Disconnect((*client));
						client = _client.erase(client);
						continue;
					}
				}
				//如果客户端套接字可写
				//if (setor.IsWriteSet((*client)->GetSocket()->GetSocketFd()))
				//{
				//	cout << "write" << endl;
				//	//发送数据,响应客户端
				//}
				//如果客户端套接字异常
				//if (setor.IsExceptSet((*client)->GetSocket()->GetSocketFd()))
				//{
				//	//移除客户端
				//	cout << "client<" << (*client)->GetSocket()->GetSocketFd() << ">已退出" << endl;
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
	//清理剩余缓冲队列
	for (auto &client : _clientbuf)
	{
		Disconnect(client);
		client = NULL;
	}
	_clientbuf.clear();
}

void CellServer::Start()
{
	//将成员函数转化为函数对象，通过指针绑定
	_pThread = new thread(std::mem_fun(&CellServer::OnRun), this);
}

int CellServer::RecvMsg(ClientSocket * pclient)
{
	//一次性接收足够多的数据
	int recvlen = recv(pclient->GetSocket()->GetSocketFd(), _recvbuf, RECV_BUF_SIZE, 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}
	//拷贝到缓冲区
	memcpy(pclient->GetBuf() + pclient->GetLastPos(), _recvbuf, recvlen);
	//更新缓冲区尾部位置
	pclient->SetLastPos(pclient->GetLastPos() + recvlen);
	//判断缓冲区中是否足够一个消息头的长度
	while (pclient->GetLastPos() >= sizeof(MsgHeader))
	{
		//取出消息头
		MsgHeader *header = reinterpret_cast<MsgHeader*>(pclient->GetBuf());
		//判断缓冲区长度是否大于等于这个消息体
		int len = header->length;
		if (pclient->GetLastPos() >= len)
		{
			//如果满足就处理这个消息
			OnNetMsg(pclient, header);
			//计算剩余缓冲区大小
			int nsize = pclient->GetLastPos() - len;
			//将缓冲区数据前移
			memcpy(pclient->GetBuf(), pclient->GetBuf() + len, nsize);
			//更新缓冲区尾部指针
			pclient->SetLastPos(nsize);
		}
	}
	return recvlen;
}
void CellServer::OnNetMsg(ClientSocket* pclient, MsgHeader * msg)
{
	//通知主服务线程
	_pevent->OnNetMsg(pclient, msg);
	switch (msg->type)
	{
		case MT_LOGIN:break;
	}
}

size_t CellServer::GetClientCount()
{
	//获取正式队列与缓冲队列中的所有客户端
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
		//开启子服务器线程
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
	//将客户端加入到最小缓冲队列
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
	//加入监听套接字
	if (IsRun())
	{	
		setor.AddReadFd(listensocket);
		//setor.AddExceptFd(_socket->GetSocketFd());
	}
	
	//开始选择
	int ret = setor.Select();
	//如果有状态改变的套接字
	if (ret > 0)
	{
		//如果监听套接字可读
		if (setor.IsReadSet(listensocket))
		{
			Accept();
		}
		//如果监听套接字异常
		if (setor.IsExceptSet(listensocket))
		{
			cout << "listen socket except" << endl;
			terminated();
			return;	//跳出循环结束运行
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
