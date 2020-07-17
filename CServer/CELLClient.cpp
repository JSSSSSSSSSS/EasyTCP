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
	//如果消息不为空
	if (msg)
	{
		//待发送的数据长度
		int nSendLen = msg->length;
		//待发送的数据
		const char* pSendData = (char*)msg;
		while (true)
		{//如果现有数据+待发送的数据 大于缓冲区大小，说明缓冲区即将溢出，我们需要调用send发送
			if (nSendLen + _sendlastpos >= SEND_BUF_SIZE)
			{
				//计算可拷贝字节数
				int nCopyLen = SEND_BUF_SIZE - _sendlastpos;
				//拷贝数据，将缓冲区填满
				memcpy(_sendmsgbuf + _sendlastpos, pSendData, nCopyLen);
				//更新待发送数据，发送了一部分
				pSendData += nCopyLen;
				//更新待发送数据长度
				nSendLen -= nCopyLen;
				//发送数据,将整个缓冲区全部发送
				len = send(_socketfd, _sendmsgbuf, SEND_BUF_SIZE, 0);
				//更新缓冲区指针
				_sendlastpos = 0;

				if (len == -1)
				{
					return len;
				}
			}
			else
			{
				//拷贝数据
				memcpy(_sendmsgbuf + _sendlastpos, pSendData, nSendLen);
				//更新尾部位置
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
