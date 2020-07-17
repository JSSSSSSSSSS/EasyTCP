#include<assert.h>

#include "CELLClient.h"
#include"Msg.h"


CELLClient::CELLClient(SOCKET socketfd, sockaddr_in addr)
	: _recvlastpos(0), _sendlastpos(0), Socket(socketfd, addr),_dtHeart(0)
{
	memset(_recvmsgbuf, 0, RECV_BUF_SIZE);
	memset(_sendmsgbuf, 0, SEND_BUF_SIZE);
}
CELLClient::~CELLClient()
{
	
}
int CELLClient::SendMsg(MsgHeader * msg)
{
	int len = 0;
	//�����Ϣ��Ϊ��
	if (msg)
	{
		//�����͵����ݳ���
		int nSendLen = msg->length;
		//�����͵�����
		const char* pSendData = (char*)msg;
		while (true)
		{//�����������+�����͵����� ���ڻ�������С��˵�����������������������Ҫ����send����
			if (nSendLen + _sendlastpos >= SEND_BUF_SIZE)
			{
				//����ɿ����ֽ���
				int nCopyLen = SEND_BUF_SIZE - _sendlastpos;
				//�������ݣ�������������
				memcpy(_sendmsgbuf + _sendlastpos, pSendData, nCopyLen);
				//���´��������ݣ�������һ����
				pSendData += nCopyLen;
				//���´��������ݳ���
				nSendLen -= nCopyLen;
				//��������,������������ȫ������
				len = send(_socketfd, _sendmsgbuf, SEND_BUF_SIZE, 0);
				//���»�����ָ��
				_sendlastpos = 0;

				if (len == -1)
				{
					return len;
				}
			}
			else
			{
				//��������
				memcpy(_sendmsgbuf + _sendlastpos, pSendData, nSendLen);
				//����β��λ��
				_sendlastpos += nSendLen;
				break;
			}
		}
	}
	return len;
}
SOCKET CELLClient::GetSocketfd() const
{
	return _socketfd;
}
char* CELLClient::GetRecvBuf()
{
	return _recvmsgbuf;
}
int CELLClient::GetRecvLastPos()const
{
	return _recvlastpos;
}
void CELLClient::SetRecvLastPos(int newpos)
{
	_recvlastpos = newpos;
}

void CELLClient::ResetDtHeart()
{
	_dtHeart = 0;
}

bool CELLClient::CheckDtHeart(time_t dt)
{
	_dtHeart += dt;
	if (_dtHeart > CLIENT_DEATH_TIME)
	{
		printf("_socketfd<%d> is dead in %dms\n",_socketfd, _dtHeart);
		return false;
	}
	else
		return true;
}
