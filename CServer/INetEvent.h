#ifndef __INetEvent__
#define __INetEvent__
#include<memory>
class CELLClient;
class MsgHeader;
class CellServer;
//网络事件
class INetEvent
{
public:
	INetEvent();
	virtual ~INetEvent();
	//客户端退出事件
	virtual void OnJoin(CELLClient* pclient) = 0;
	//客户端退出事件
	virtual void OnLeave(CELLClient* pclient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(CellServer* cellserver, std::shared_ptr<CELLClient> pclient, MsgHeader * msg) = 0;
};

#endif // !__INetEvent__

