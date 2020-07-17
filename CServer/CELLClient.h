#pragma once
#ifndef __CLIENTSOCKET__
#define __CLIENTSOCKET__

#include "wsabase.h"
#include"Alloctor.h"

#define CLIENT_DEATH_TIME	5000
class MsgHeader;

//客户端类型
class CELLClient :public Socket
{
public:
	//MEMORY_POOL_IMPLEMENT
	CELLClient(SOCKET socketfd, sockaddr_in addr = { 0 });
	~CELLClient();
	//发送消息
	int SendMsg(MsgHeader* msg);
	//获得客户端socket
	SOCKET GetSocketfd()const;
	//获取缓冲区大小
	char* GetRecvBuf();
	//获取缓冲区尾指针位置
	int GetRecvLastPos()const;
	//设置缓冲区尾指针
	void SetRecvLastPos(int newpos);
	//重置心跳计时
	void ResetDtHeart();
	//检查心跳计时
	bool CheckDtHeart(time_t dt);
private:
	char			_recvmsgbuf[RECV_BUF_SIZE];				//接收消息缓冲区
	int				_recvlastpos;							//接收缓冲区尾部位置
	char			_sendmsgbuf[SEND_BUF_SIZE];				//发送消息缓冲区
	int				_sendlastpos;							//发送缓冲区尾部位置
	time_t			_dtHeart;								//心跳计时
};
//int a = sizeof(CELLClient);		//51240

#endif // !__CLIENTSOCKET__