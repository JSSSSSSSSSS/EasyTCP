#include "wsabase.h"
#include<iostream>
#include<assert.h>
using namespace std;

wsabase::wsabase(unsigned int ver1, unsigned int ver2)
{
#ifdef _WIN32
	//Windows下需要初始化socket库，Unix不需要
	WSADATA wsaData = { 0 };
	assert(WSAStartup(MAKEWORD(ver1, ver2), &wsaData) == NO_ERROR);
#endif
}

wsabase::~wsabase()
{
#ifdef _WIN32
	//Windows需要卸载socket库
	WSACleanup();
#endif
}

Socket::Socket()
	:_socketfd(INVALID_SOCKET), _addr({0})
{
}

Socket::Socket(SOCKET socketfd, sockaddr_in addr)
	: _socketfd(socketfd), _addr(addr)
{
}

Socket::~Socket()
{
	if (_socketfd != INVALID_SOCKET)
	{
		shutdown(_socketfd, SD_BOTH);
		closesocket(_socketfd);
		_socketfd = INVALID_SOCKET;
	}
	_addr = { 0 };
}

bool Socket::InitSocket(int af, int type, int protocol)
{
	/*创建套接字*/
	_addr.sin_family = af;
	if (_socketfd != INVALID_SOCKET)
	{
		closesocket(_socketfd);
	}
	_socketfd = socket(af, type, protocol);
	return (_socketfd != INVALID_SOCKET);
}

SOCKET Socket::GetSocketfd() const
{
	return _socketfd;
}

sockaddr_in Socket::GetAddr() const
{
	return _addr;
}

void Socket::SetAddr(sockaddr_in addr)
{
	_addr.sin_family = addr.sin_family;
	_addr.sin_addr = addr.sin_addr;
	_addr.sin_port = addr.sin_port;
}