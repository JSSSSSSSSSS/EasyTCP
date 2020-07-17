#pragma once
#include<mutex>
#include"wsabase.h"

#define SERVER_HEART_TIME	5000
struct MsgHeader;

class Client:public wsabase,public Socket
{
public:
	Client(unsigned int ver1 = 2, unsigned int ver2 = 2);
	~Client();
	bool Connect(const char * ipaddr, unsigned int port);
	void Run();
	bool IsRun();
	int SendMsg(MsgHeader *msg, int nlen);
	int RecvMsg();
	void Disconnect();
private:
	//处理网络消息
	void OnNetMsg(MsgHeader*msg);
	//重置心跳时间
	void ResetHeart();
	//心跳检测
	bool CheckHeart(time_t dt);
private:
	char*	_msgbuf;		//接收消息缓冲区
	int		_lastpos;		//接收消息缓冲区尾部位置
	time_t	_oldTime;		//旧时间戳
	time_t	_dtHeart;		//心跳时间
};

