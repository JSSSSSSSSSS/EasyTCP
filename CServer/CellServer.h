#ifndef __CellServer__
#define __CellServer__

#include<map>
#include<list>
#include<mutex>
#include<thread>
#include"Alloctor.h"
#include"wsabase.h"

class INetEvent;
class CELLClient;
class MsgHeader;
class CellTask;
class CellTaskServer;

//子服务器，接收客户端发送的数据
class CellServer
{
public:
	CellServer(INetEvent* pevent = NULL);
	~CellServer();

	//开启主运行线程
	void Start();
	//获取cellserver实际服务的客户端数量
	size_t GetClientCount();
	//往缓冲区中添加客户端
	void AddClient(std::shared_ptr<CELLClient> client);
	//添加发送任务
	void addSendTask(std::shared_ptr<CELLClient> client, MsgHeader * msg);
	//是否运行
	bool IsRun();
private:
	//主运行函数，进行select与数据接收
	void OnRun();
	//断开指定客户端的连接
	void Disconnect(CELLClient * pclient);
	//读取客户端的数据
	void ReadClients(fd_set &cloneReadSet,fd_set &readset);
	//检测客户端的心跳
	void CheckClientHeart(fd_set &readset);
	//接收数据
	int RecvMsg(std::shared_ptr<CELLClient> pclient);
	//响应网络消息
	virtual void OnNetMsg(std::shared_ptr<CELLClient> pclient, MsgHeader*msg);
private:
	std::map<SOCKET, std::shared_ptr<CELLClient>>	_client;		//正式客户端队列
	std::list<std::shared_ptr<CELLClient>>		_clientbuf;		//缓冲客户端队列
	std::mutex										_mtx;			//缓冲队列锁
	std::thread*									_pThread;		//线程
	INetEvent*										_pevent;		//事件
	CellTaskServer*									_taskServer;	//任务服务器
	time_t											_oldTime;		//旧时间戳
};

//int a = sizeof(CellServer);	//144
#endif // !__CellServer__


