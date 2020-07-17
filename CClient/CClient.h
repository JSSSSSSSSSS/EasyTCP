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
	//����������Ϣ
	void OnNetMsg(MsgHeader*msg);
	//��������ʱ��
	void ResetHeart();
	//�������
	bool CheckHeart(time_t dt);
private:
	char*	_msgbuf;		//������Ϣ������
	int		_lastpos;		//������Ϣ������β��λ��
	time_t	_oldTime;		//��ʱ���
	time_t	_dtHeart;		//����ʱ��
};

