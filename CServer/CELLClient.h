#pragma once
#ifndef __CLIENTSOCKET__
#define __CLIENTSOCKET__

#include "wsabase.h"
#include"Alloctor.h"

#define CLIENT_DEATH_TIME	5000
class MsgHeader;

//�ͻ�������
class CELLClient :public Socket
{
public:
	//MEMORY_POOL_IMPLEMENT
	CELLClient(SOCKET socketfd, sockaddr_in addr = { 0 });
	~CELLClient();
	//������Ϣ
	int SendMsg(MsgHeader* msg);
	//��ÿͻ���socket
	SOCKET GetSocketfd()const;
	//��ȡ��������С
	char* GetRecvBuf();
	//��ȡ������βָ��λ��
	int GetRecvLastPos()const;
	//���û�����βָ��
	void SetRecvLastPos(int newpos);
	//����������ʱ
	void ResetDtHeart();
	//���������ʱ
	bool CheckDtHeart(time_t dt);
private:
	char			_recvmsgbuf[RECV_BUF_SIZE];				//������Ϣ������
	int				_recvlastpos;							//���ջ�����β��λ��
	char			_sendmsgbuf[SEND_BUF_SIZE];				//������Ϣ������
	int				_sendlastpos;							//���ͻ�����β��λ��
	time_t			_dtHeart;								//������ʱ
};
//int a = sizeof(CELLClient);		//51240

#endif // !__CLIENTSOCKET__