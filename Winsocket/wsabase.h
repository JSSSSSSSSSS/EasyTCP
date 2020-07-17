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

	#define SD_RECEIVE      0x00
	#define SD_SEND         0x01
	#define SD_BOTH         0x02
#endif

#define RECV_BUF_SIZE	10240
#define SEND_BUF_SIZE	10240
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
	SOCKET GetSocketfd()const;
	sockaddr_in GetAddr()const;
	void SetAddr(sockaddr_in addr);
protected:
	//SOCKET
	SOCKET _socketfd;
	sockaddr_in _addr;
};

//int a = sizeof(Socket);	32

#endif