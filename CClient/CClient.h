#pragma once
#include"../Winsocket/wsabase.h"
#include"../Winsocket/Msg.h"
#include<mutex>
class Client:public wsabase
{
public:
	Client();
	~Client();
	bool InitSocket(int af = 2, int type = 1, int protocol = 6);
	bool Connect(const char * ipaddr, unsigned int port);
	void Run();

	bool IsRun();
	int SendMsg(MsgHeader*msg);
	int RecvMsg();
	void Disconnect();
private:
	void HandleMsg(MsgHeader*msg);
	void login(MsgHeader*msg);

private:
	Socket *_socket;
	char *_recvbuf;
	char *_msgbuf;
	int _lastpos;
};

