#include "CClient.h"
#include<iostream>
#include<assert.h>
using namespace std;

Client::Client()
	:_socket(NULL),_recvbuf(NULL),_msgbuf(NULL), _lastpos(0)
{
	_recvbuf = new char[RECV_BUF_SIZE];
	assert(_recvbuf != NULL);
	_msgbuf = new char[RECV_BUF_SIZE*10];
	assert(_msgbuf != NULL);
}

Client::~Client()
{
	if (_msgbuf)
	{
		delete _msgbuf;
		_msgbuf = NULL;
	}
	if (_recvbuf)
	{
		delete _recvbuf;
		_recvbuf = NULL;
	}
	if (_socket)
	{
		delete _socket;
		_socket = NULL;
	}
}

bool Client::InitSocket(int af, int type, int protocol)
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
	return _socket->InitSocket(af, type, protocol);
}

bool Client::Connect(const char * ipaddr, unsigned int port)
{
	//m_addr.sin_family�Ѿ���InitSocket()�и�ֵ
	sockaddr_in addr;
	addr.sin_family = _socket->GetAddr().sin_family;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(port);
	if (!_socket)
	{
		cout << "you should call InitSocket() before Connect" << endl;
		return false;
	}
	_socket->SetAddr(addr);
	if (connect(_socket->GetSocketFd(), (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		LOG("Connect failed!");
		return false;
	}
	return true;
}

void Client::Run()
{
	Selector setor(1);
	setor.AddExceptFd(_socket->GetSocketFd());
	setor.AddWriteFd(_socket->GetSocketFd());
	int ret = setor.Select();
	if (ret > 0)
	{
		if (setor.IsWriteSet(_socket->GetSocketFd()))
		{
			MSG_LOGIN login;
			strcpy(login.username, "jiangsen");
			strcpy(login.password, "123456789");
			SendMsg(&login);
		}
		if (setor.IsExceptSet(_socket->GetSocketFd()))
		{
			cout << "�������Ͽ�����" << endl;
			Disconnect();
		}
	}
}

bool Client::IsRun()
{
	return _socket;
}

int Client::SendMsg(MsgHeader*msg)
{
	int sendlen = send(_socket->GetSocketFd(),(char*)msg, msg->length,0);
	return sendlen;
}

int Client::RecvMsg()
{
	//һ���Խ����㹻�������
	int recvlen = recv(_socket->GetSocketFd(), _recvbuf, RECV_BUF_SIZE, 0);
	if (recvlen <= 0)
	{
		return recvlen;
	}
	//������������
	memcpy(_msgbuf+ _lastpos, _recvbuf, recvlen);
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
			HandleMsg(header);
			//����ʣ�໺������С
			int nsize = _lastpos - len;
			//������������ǰ��
			memcpy(_msgbuf, _msgbuf + len, nsize);
			//���»�����β��ָ��
			_lastpos = nsize;
		}
	}
	return recvlen;
}


void Client::HandleMsg(MsgHeader*msg)
{
	switch (msg->type)
	{
		case MT_LOGIN:login(msg); break;
	}
}

void Client::Disconnect()
{
	if (_msgbuf)
	{
		delete _msgbuf;
		_msgbuf = NULL;
	}
	if (_recvbuf)
	{
		delete _recvbuf;
		_recvbuf = NULL;
	}
	if (_socket)
	{
		delete _socket;
		_socket = NULL;
	}
}

void Client::login(MsgHeader* msg)
{
	MSG_LOGIN *info = reinterpret_cast<MSG_LOGIN*>(msg);

	/*std::cout << info->username << std::endl;
	std::cout << info->password << std::endl;*/
}
