#ifndef __INetEvent__
#define __INetEvent__
#include<memory>
class CELLClient;
class MsgHeader;
class CellServer;
//�����¼�
class INetEvent
{
public:
	INetEvent();
	virtual ~INetEvent();
	//�ͻ����˳��¼�
	virtual void OnJoin(CELLClient* pclient) = 0;
	//�ͻ����˳��¼�
	virtual void OnLeave(CELLClient* pclient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(CellServer* cellserver, std::shared_ptr<CELLClient> pclient, MsgHeader * msg) = 0;
};

#endif // !__INetEvent__

