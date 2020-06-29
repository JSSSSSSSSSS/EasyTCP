#pragma once
#ifndef _WSABASE_
#define _WSABASE_
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//引入关键头文件与库,提供跨平台支持
#ifdef _WIN32
	#define FD_SETSIZE		2506	//突破select最大数量限制限制
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
	#define socklen_t		int
#else
	#include<unistd.h> //uni(x) std
	#include<sys/types.h>
	#include<sys/socket.h>
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET			int
	#define INVALID_SOCKET	(SOCKET)(~0)
	#define SOCKET_ERROR	(-1) 
	#define closesocket		close
	#define sprintf_s		snprintf
	#define sscanf_s		sscanf	

	#define SD_RECEIVE      0x00
	#define SD_SEND         0x01
	#define SD_BOTH         0x02
#endif

#define RECV_BUF_SIZE	10240
#define WIN32_LEAN_AND_MEAN
#define GetLastError	WSAGetLastError
//定义一个打印日志的宏
#define LOG(info) std::cout<<"log:"<<info<<std::endl;

class wsabase
{
public:
	wsabase(unsigned int ver1 = 2, unsigned int ver2 = 2);
	virtual ~wsabase();
};

class Socket
{
public:
	Socket();
	Socket(SOCKET socketfd, sockaddr_in addr = {0});
	virtual ~Socket();
	bool InitSocket(int af = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
	SOCKET GetSocketFd()const;
	sockaddr_in GetAddr()const;
	void SetAddr(sockaddr_in addr);
protected:
	//SOCKET
	SOCKET _socketfd;
	sockaddr_in _addr;
};

class Selector
{
public:
	Selector();
	Selector(long timeout);
	~Selector();

public:
	//清空全部集合
	void ClearAll();
	//清除某个读集合中的元素
	void ClearReadSet(SOCKET s);
	//清空读集合
	void ClearReadSetAll();
	//清除某个写集合中的元素
	void ClearWriteSet(SOCKET s);
	//清空写集合
	void ClearWriteSetAll();
	//清除某个异常集合中的元素
	void ClearExceptSet(SOCKET s);
	//清空异常集合
	void ClearExceptSetAll();
	//将socket加入到集合中
	void AddReadFd(SOCKET s);
	//将socket加入到集合中
	void AddWriteFd(SOCKET s);
	//将socket加入到集合中
	void AddExceptFd(SOCKET s);
	//进行选择
	int Select();
	//是否在读集合中
	bool IsReadSet(SOCKET s);
	//是否在写集合中
	bool IsWriteSet(SOCKET s);
	//是否在异常集合中
	bool IsExceptSet(SOCKET s);
	//设置超时时间，单位ms
	void SetTimeout(long timeout);

private:
	fd_set *m_readset;
	fd_set *m_writeset;
	fd_set *m_exceptset;
	timeval *m_timeout;
	unsigned int _readmax;
	unsigned int _writemax;
	unsigned int _exceptmax;
};
#endif