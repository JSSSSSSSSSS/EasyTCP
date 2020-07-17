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
	//m_addr.sin_family�Ѿ���InitSocket()�и�ֵ
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
				//������յ�EOF
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
		//�Է����������������
		time_t dt = CELLTime::GetNowTimeMiliSec() - _oldTime;
		if (!CheckHeart(dt))
		{
			MSG_HEART heart;
			SendMsg(&heart, heart.length);
		}
		//���¾�ʱ���
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
	//ֱ��ʹ�ô󻺳���
	char* recvbuf = _msgbuf + _lastpos;
	//һ���Խ����㹻�������
	int recvlen = recv(_socketfd, recvbuf, (RECV_BUF_SIZE)- _lastpos, 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}
	//������������
	//memcpy(_msgbuf+ _lastpos, _recvbuf, recvlen);
	//���»�����β��λ��
	_lastpos += recvlen;
	//�жϻ��������Ƿ��㹻һ����Ϣͷ�ĳ���
	while (_lastpos >= sizeof(MsgHeader))
	{
		//ȡ����Ϣͷ
		MsgHeader *header = reinterpret_cast<MsgHeader*>(_msgbuf);
		//�жϻ����������Ƿ���ڵ��������Ϣ��
		int len = header->length;
		if (_lastpos >= len)
		{
			//�������ʹ��������Ϣ
			OnNetMsg(header);
			//����ʣ�໺������С
			int nsize = _lastpos - len;
			//������������ǰ��
			memcpy(_msgbuf, _msgbuf + len, nsize);
			//���»�����β��ָ��
			_lastpos = nsize;
		}
		else
		{
			//��Ϣ���㣬�˳�ѭ�����������Ҫ��
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
	//�ر��׽���
	if (_socketfd != INVALID_SOCKET)
	{
		shutdown(_socketfd, SD_BOTH);
		closesocket(_socketfd);
		_socketfd = INVALID_SOCKET;
	}
	//��շ�������ַ�ṹ��
	_addr = { 0 };
	//�ͷŻ�����
	if (_msgbuf)
	{
		delete _msgbuf;
		_msgbuf = NULL;
	}
}

