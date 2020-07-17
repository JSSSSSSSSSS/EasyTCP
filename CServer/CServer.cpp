#include<iostream>
#include<assert.h>
#include<thread>
using namespace std;

#include"Msg.h"
#include"CServer.h"
#include"CELLClient.h"
#include"CellServer.h"

Server::Server()
	:_recvcount(0), _clientcount(0)
{
	
}

Server::~Server()
{
	for (auto cellserver : _CellServer)
	{
		delete cellserver;
	}
	_CellServer.clear();
}


bool Server::Bind(unsigned int port)
{
	//bind IP addr and port
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(port);

	if (::bind(_socketfd, (sockaddr *)&_addr, sizeof(_addr)) == SOCKET_ERROR)
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
	if (listen(_socketfd, SOMAXCONN) == SOCKET_ERROR)
	{
		LOG("listen failed!");
		return false;
	}
	else
	{
		LOG("listen success...");
		return true;
	}
}

void Server::Accept()
{
	//start accept
	sockaddr_in clientaddr = { 0 };
	socklen_t len = sizeof(clientaddr);
	SOCKET accSock = accept(_socketfd, (sockaddr *)&clientaddr, &len);
	if (accSock == INVALID_SOCKET)
	{
		LOG("accept a socket failed!");
	}
	else
	{
		//新建一个clientsocket对象
		auto pclient = make_shared<CELLClient>(accSock, clientaddr);
		if (pclient)
		{
			//将新加入的客户端添加到子服务器缓冲
			AddClientToCellServer(pclient);
		}
		else
		{
			LOG("failed to new a CELLClient!");
		}
	}
}

void Server::StartCell(int ThreadCount)
{
	//开启子服务器
	for (int i = 0; i < ThreadCount; i++)
	{
		//创建子服务器
		auto cell = new CellServer(this);
		//加入vector
		_CellServer.push_back(cell);
		//开启线程
		cell->Start();
	}
}

void Server::AddClientToCellServer(std::shared_ptr<CELLClient> client)
{
	//将客户端加入到最小缓冲队列
	auto mincellserver = _CellServer[0];
	for (auto &cellserver: _CellServer)
	{
		if (cellserver->GetClientCount() < mincellserver->GetClientCount())
		{
			mincellserver = cellserver;
		}
	}
	mincellserver->AddClient(client);
	OnJoin(client.get());
}

void Server::MsgCount()
{
	auto t = _timer.GetElapsedSecond();
	if (t >= 1.0)
	{
		printf("time<%lf>,client<%d>,recvcount<%d>\n", t, _clientcount.load(), (int)(_recvcount / t));
		_recvcount = 0;
		_timer.Update();
	}
}

void Server::Run()
{
	MsgCount();
	fd_set readset;
	FD_ZERO(&readset);
	timeval timeout = { 0,100 };
	//加入监听套接字
	if (IsRun())
	{
		//加入监听套接字
		FD_SET(_socketfd,&readset);
		//开始选择
		int ret = select(_socketfd + 1, &readset, 0, 0, &timeout);
		//如果有状态改变的套接字
		if (ret > 0)
		{
			//如果监听套接字可读
			if (FD_ISSET(_socketfd,&readset))
			{
				Accept();
			}
		}
		else if (ret < 0)
		{
			Close();
			return;
		}
	}
}

bool Server::IsRun() const
{
	return (_socketfd !=INVALID_SOCKET);
}

void Server::Close()
{
	if (_socketfd != INVALID_SOCKET)
	{
		shutdown(_socketfd, SD_BOTH);
		closesocket(_socketfd);
		_socketfd = INVALID_SOCKET;
	}
	_addr = { 0 };
}


SOCKET Server::GetListenSocket() const
{
	return _socketfd;
}

void Server::OnJoin(CELLClient * pclient)
{
	_clientcount++;
}

void Server::OnLeave(CELLClient * pclient)
{
	_clientcount--;
}

void Server::OnNetMsg(CellServer* cellserver, std::shared_ptr<CELLClient> pclient, MsgHeader * msg)
{
	_recvcount++;
	switch (msg->type)
	{
		case MT_LOGIN:
			{
				MSG_LOGINRE *loginre = new MSG_LOGINRE();
				cellserver->addSendTask(pclient,loginre);
				break;
			}
		case MT_HEART:
			{
				MSG_HEARTRE *heartre = new MSG_HEARTRE();
				cellserver->addSendTask(pclient, heartre);
				break;
			}
	}
}
