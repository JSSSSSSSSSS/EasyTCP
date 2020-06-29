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

SOCKET Socket::GetSocketFd() const
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

Selector::Selector()
	:m_timeout(NULL),
	m_exceptset(NULL),
	m_readset(NULL),
	m_writeset(NULL),
	_readmax(0),
	_writemax(0),
	_exceptmax(0)
{
}

Selector::Selector(long timeout)
	:m_timeout(NULL),
	m_exceptset(NULL),
	m_readset(NULL),
	m_writeset(NULL),
	_readmax(0),
	_writemax(0),
	_exceptmax(0)
{
	m_timeout = new timeval();
	//设置超时，秒和毫秒
	m_timeout->tv_sec = 0;
	m_timeout->tv_usec = timeout*1000; //结构体中定义的是微秒，这里*1000转化为毫秒
}

Selector::~Selector()
{
	if (m_timeout)
	{
		delete m_timeout;
		m_timeout = NULL;
	}
	if (m_readset)
	{
		delete m_readset;
		m_readset = NULL;
	}
	if (m_writeset)
	{
		delete m_writeset;
		m_writeset = NULL;
	}
	if (m_exceptset)
	{
		delete m_exceptset;
		m_exceptset = NULL;
	}
}

void Selector::ClearAll()
{
	ClearReadSetAll();
	ClearWriteSetAll();
	ClearExceptSetAll();
}

void Selector::ClearReadSetAll()
{
	if (m_readset)
		FD_ZERO(m_readset);
	_readmax = 0;
}

void Selector::ClearWriteSetAll()
{
	if (m_writeset)
		FD_ZERO(m_writeset);
	_writemax = 0;
}

void Selector::ClearReadSet(SOCKET s)
{
	if(m_readset && IsReadSet(s))
		FD_CLR(s, m_readset);
}

void Selector::ClearWriteSet(SOCKET s)
{
	if(m_writeset&& IsWriteSet(s))
		FD_CLR(s, m_writeset);
}

void Selector::ClearExceptSet(SOCKET s)
{
	if(m_exceptset&& IsExceptSet(s))
		FD_CLR(s,m_exceptset);
}

void Selector::ClearExceptSetAll()
{
	if (m_exceptset)
		FD_ZERO(m_exceptset);
	_exceptmax = 0;
}

void Selector::AddReadFd(SOCKET s)
{
	_readmax = _readmax > s ? _readmax : s;
	if (!m_readset)
	{
		m_readset = new fd_set();
	}
	FD_SET(s, m_readset);
}

void Selector::AddWriteFd(SOCKET s)
{
	_writemax = _writemax > s ? _writemax : s;
	if (!m_writeset)
	{
		m_writeset = new fd_set();
	}
	FD_SET(s, m_writeset);
}

void Selector::AddExceptFd(SOCKET s)
{
	_exceptmax = _exceptmax > s ? _exceptmax : s;
	if (!m_exceptset)
	{
		m_exceptset = new fd_set();
	}
	FD_SET(s, m_exceptset);
}

/*
Parameters
nfds
Ignored in windows platform. The nfds parameter is included only for compatibility with Berkeley sockets.

readfds
Optional pointer to a set of sockets to be checked for readability.

writefds
Optional pointer to a set of sockets to be checked for writability.

exceptfds
Optional pointer to a set of sockets to be checked for errors.

timeout
Maximum time for select to wait, provided in the form of a TIMEVAL structure. Set the timeout parameter to null for blocking operations.
*/
int Selector::Select()
{
	unsigned int max = _writemax > _exceptmax ? _writemax : _exceptmax;
	max = _readmax > max ? _readmax : max;
	return select((int)(max+1), m_readset, m_writeset, m_exceptset, m_timeout);
}

bool Selector::IsReadSet(SOCKET s)
{
	return m_readset == NULL ? false : FD_ISSET(s, m_readset);
}

bool Selector::IsWriteSet(SOCKET s)
{
	return m_writeset == NULL ? false : FD_ISSET(s, m_writeset);
}

bool Selector::IsExceptSet(SOCKET s)
{
	return m_exceptset == NULL? false:FD_ISSET(s, m_exceptset);
}


void Selector::SetTimeout(long timeout)
{
	if (!m_timeout)
	{
		m_timeout = new timeval();
	}
	//设置超时，秒和毫秒
	m_timeout->tv_sec = 0;
	m_timeout->tv_usec = timeout * 1000; //结构体中定义的是微秒，这里*1000转化为毫秒
}