#include<iostream>
#include<assert.h>
#include "CClient.h"
#include"Msg.h"
#include"CELLTimestamp.h"

using namespace std;

Client::Client(unsigned int ver1, unsigned int ver2)
	:_msgbuf(NULL), _lastpos(0),_dtHeart(0),_oldTime(0)
{
	_msgbuf = new char[RECV_BUF_SIZE];
	assert(_msgbuf != NULL);
}

Client::~Client()
{
	if (_msgbuf)
	{
		delete _msgbuf;
		_msgbuf = NULL;
	}
}

bool Client::Connect(const char * ipaddr, unsigned int port)
{
	//m_addr.sin_family已经在InitSocket()中赋值
	_addr.sin_addr.s_addr = inet_addr(ipaddr);
	_addr.sin_port = htons(port);
	if (_socketfd == INVALID_SOCKET)
	{
		cout << "you should call InitSocket() before Connect" << endl;
		return false;
	}
	
	if (connect(_socketfd, (sockaddr *)&_addr, sizeof(_addr)) == SOCKET_ERROR)
	{
		//LOG("Connect failed! ErrorCode = "<<GetLastError());
		LOG(_socketfd << "Connect failed!");
		return false;
	}
	{
		//LOG(_socketfd << "connected!");
		_oldTime = CELLTime::GetNowTimeMiliSec();
		return true;
	}
}

void Client::Run()
{
	if (IsRun())
	{
		fd_set readset;
		FD_ZERO(&readset);
		FD_SET(_socketfd, &readset);
		timeval timeout = { 0,0 };
		int ret = select(_socketfd + 1, &readset, 0, 0, &timeout);
		if (ret > 0)
		{
			if (FD_ISSET(_socketfd, &readset))
			{
				int relen = RecvMsg();
				//如果接收到EOF
				if (relen <= 0)
				{
					//cout << "recv error! errorcode = "<<GetLastError() << endl;
					cout << "disconnect from server!" << endl;
					Disconnect();
					return;
				}
				else
				{
					ResetHeart();
				}
			}
		}
		//对服务器进行心跳检测
		time_t dt = CELLTime::GetNowTimeMiliSec() - _oldTime;
		if (!CheckHeart(dt))
		{
			MSG_HEART heart;
			SendMsg(&heart, heart.length);
		}
		//更新旧时间戳
		_oldTime = CELLTime::GetNowTimeMiliSec();
	}
}

bool Client::IsRun()
{
	return (_socketfd !=INVALID_SOCKET);
}

int Client::SendMsg(MsgHeader *msg, int nlen)
{
	int ret = SOCKET_ERROR;
	if (IsRun() && msg)
	{
		ret = send(_socketfd, (char*)msg, nlen, 0);
		if (ret == SOCKET_ERROR)
		{
			Disconnect();
		}
	}
	return ret;
}

int Client::RecvMsg()
{
	//直接使用大缓冲区
	char* recvbuf = _msgbuf + _lastpos;
	//一次性接收足够多的数据
	int recvlen = recv(_socketfd, recvbuf, (RECV_BUF_SIZE)- _lastpos, 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}
	//拷贝到缓冲区
	//memcpy(_msgbuf+ _lastpos, _recvbuf, recvlen);
	//更新缓冲区尾部位置
	_lastpos += recvlen;
	//判断缓冲区中是否足够一个消息头的长度
	while (_lastpos >= sizeof(MsgHeader))
	{
		//取出消息头
		MsgHeader *header = reinterpret_cast<MsgHeader*>(_msgbuf);
		//判断缓冲区长度是否大于等于这个消息体
		int len = header->length;
		if (_lastpos >= len)
		{
			//如果满足就处理这个消息
			OnNetMsg(header);
			//计算剩余缓冲区大小
			int nsize = _lastpos - len;
			//将缓冲区数据前移
			memcpy(_msgbuf, _msgbuf + len, nsize);
			//更新缓冲区尾部指针
			_lastpos = nsize;
		}
		else
		{
			//消息不足，退出循环（这个很重要）
			break;
		}
	}
	return recvlen;
}


void Client::OnNetMsg(MsgHeader*msg)
{
	switch (msg->type)
	{
		case MT_LOGINRE: break; 
	}
}

void Client::ResetHeart()
{
	_dtHeart = 0;
}

bool Client::CheckHeart(time_t dt)
{
	_dtHeart += dt;
	if (_dtHeart > SERVER_HEART_TIME)
	{
		printf("server is likely disconnected !");
		return false;
	}
	return true;
}

void Client::Disconnect()
{
	//关闭套接字
	if (_socketfd != INVALID_SOCKET)
	{
		shutdown(_socketfd, SD_BOTH);
		closesocket(_socketfd);
		_socketfd = INVALID_SOCKET;
	}
	//清空服务器地址结构体
	_addr = { 0 };
	//释放缓冲区
	if (_msgbuf)
	{
		delete _msgbuf;
		_msgbuf = NULL;
	}
}

