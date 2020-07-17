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
	//终止线程
	if (_pThread)
	{
		_pThread->join();
		delete _pThread;
	}
	//清理剩余缓冲队列
	for (auto &client : _clientbuf)
	{
		cout << "in ~cellserver" << endl;
		Disconnect(client.get());
		client = NULL;
	}
	_clientbuf.clear();
	//清理剩余队列
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
		//将缓冲区中的客户端加入到正式队列
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

		//如果没有数据就不用select
		if (_client.empty())
		{
			std::chrono::milliseconds t(1000);
			std::this_thread::sleep_for(t);
			//更新旧时间戳
			_oldTime = CELLTime::GetNowTimeMiliSec();
			continue;
		}
		//开始select
		fd_set cloneReadSet;
		//fd_set cloneExcptSet;
		memcpy(&cloneReadSet, &readset, sizeof(fd_set));
		//memcpy(&cloneExcptSet, &readset, sizeof(fd_set));

		//开始选择
		ret = select(maxsocketfd + 1, &cloneReadSet, 0, 0, &timeout);
		//如果有状态改变的套接字
		if (ret > 0)
		{
			//读取客户端的数据，并且更新readset
			ReadClients(cloneReadSet, readset);
			//检查客户端的心跳
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
		//如果接收到EOF
		//if (relen <= 0)
		//{
		//	//更新fd_set
		//	FD_CLR(cloneReadSet.fd_array[i], &readset);
		//	//断开客户端连接
		//	Disconnect(pclient.get());
		//	//移除客户端
		//	_client.erase(cloneReadSet.fd_array[i]);
		//	continue;
		//}
	}
#else

	//处理客户端
	auto begin = _client.begin();
	auto end = _client.end();

	for (auto client = begin; client != end;)
	{
		SOCKET socket = (*client).first;
		//如果客户端套接字可读
		if (FD_ISSET(socket, &cloneReadSet))
		{
			int relen = RecvMsg((*client).second);
			//如果接收到EOF
			if (relen <= 0)
			{
				//更新fd_set
				FD_CLR(socket, &readset);
				//断开客户端连接
				Disconnect((*client).second.get());
				//移除客户端
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
	//获取当前时间
	time_t nowTime = CELLTime::GetNowTimeMiliSec();
	//获取时间间隔
	time_t dt = nowTime - _oldTime;

	for (auto iter = begin; iter != end; )
	{
		//心跳检测
		if (!iter->second->CheckDtHeart(dt))
		{
			//更新fd_set
			FD_CLR(iter->second->GetSocketfd(), &readset);
			//断开客户端连接
			Disconnect((*iter).second.get());
			//移除客户端
			iter = _client.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	//更新旧时间戳
	_oldTime = CELLTime::GetNowTimeMiliSec();
}

void CellServer::Start()
{
	//将成员函数转化为函数对象，通过指针绑定
	_pThread = new thread(&CellServer::OnRun, this);
	//启动任务服务器
	_taskServer->start();
}

int CellServer::RecvMsg(std::shared_ptr<CELLClient> pclient)
{
	//直接使用客户端的缓冲区
	char* recvbuf = pclient->GetRecvBuf() + pclient->GetRecvLastPos();
	//一次性接收足够多的数据
	int recvlen = recv(pclient->GetSocketfd(), recvbuf, (RECV_BUF_SIZE)-pclient->GetRecvLastPos(), 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}

	//收到任何数据，重置心跳
	pclient->ResetDtHeart();

	//拷贝到缓冲区
	//memcpy(pclient->GetBuf() + pclient->GetLastPos(), _recvbuf, recvlen);
	//更新缓冲区尾部位置
	pclient->SetRecvLastPos(pclient->GetRecvLastPos() + recvlen);
	//判断缓冲区中是否足够一个消息头的长度
	while (pclient->GetRecvLastPos() >= sizeof(MsgHeader))
	{
		//取出消息头
		MsgHeader *header = (MsgHeader*)(pclient->GetRecvBuf());
		//判断缓冲区长度是否大于等于这个消息体
		int len = header->length;
		if (pclient->GetRecvLastPos() >= len)
		{
			//如果满足就处理这个消息
			OnNetMsg(pclient, header);
			//计算剩余缓冲区大小
			int nsize = pclient->GetRecvLastPos() - len;
			//将缓冲区数据前移
			memcpy(pclient->GetRecvBuf(), pclient->GetRecvBuf() + len, nsize);
			//更新缓冲区尾部指针
			pclient->SetRecvLastPos(nsize);
		}
		else
		{
			//消息不足
			break;
		}
	}
	return recvlen;
}
void CellServer::OnNetMsg(std::shared_ptr<CELLClient> pclient, MsgHeader * msg)
{
	//通知主服务线程
	_pevent->OnNetMsg(this,pclient, msg);
}

void CellServer::addSendTask(std::shared_ptr<CELLClient> client, MsgHeader * msg)
{
	//构造发送任务，并加入到列表
	_taskServer->addTask([client, msg]() {
		client->SendMsg(msg);
		delete msg; });
}

size_t CellServer::GetClientCount()
{
	//获取正式队列与缓冲队列中的所有客户端
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

