#pragma once
#ifndef __SERVER__
#define __SERVER__


#include<vector>
#include<atomic>
#include<memory>

#include"wsabase.h"
#include"CELLTimestamp.h"
#include"INetEvent.h"
class CellServer;

/*
性能要点：
	1.接收应该设置缓冲区。
		因为在网络条件复杂的情况下，可能出现少包、粘包的情况。

	2.接收和发送应该在不同线程。（生产者——消费者模式）
		因为，相同时间内，send的调用次数远远少于recv的次数。
		在同一个线程中同时存在send和recv的话，send会影响recv的性能。
		send通常是性能瓶颈。

	3.接收客户端与客户端的具体业务应该分开。（生产者——消费者模式）
		客户端的连接应该单独使用一个线程，通过缓冲区来传递已经连接的客户端。（生产者——消费者模式）
		通常有多个消费者线程，并且每个消费者拥有自己的缓冲区。这样可以减少加锁和释放锁的次数。
		这样可以在处理已有客户端的同时，更加及时地接收新客户端的连接。

	4.定时、定量发送数据。
		减少send的调用次数。
*/


/*
新的问题：pclient可能已经被释放，但是send任务没有完成。
	智能指针可以解决
*/

//主服务器，处理连接与其他业务
class Server:public wsabase,public INetEvent,public Socket
{
public:
	Server();
	virtual ~Server();
	//绑定端口
	bool Bind(unsigned int port);
	//监听端口
	bool Listen();
	//接受客户端连接,为客户端分配空间
	void Accept();
	//启动子服务器接收数据
	void StartCell(int ThreadCount);
	//将接收的客户端添加到子服务器缓冲区
	void AddClientToCellServer(std::shared_ptr<CELLClient> client);
	//消息计数
	void MsgCount();
	//主运行函数
	void Run();
	//是否运行
	bool IsRun()const;
	//结束运行
	void Close();
	//获取监听套接字
	SOCKET GetListenSocket()const;

	//客户端加入消息
	virtual void OnJoin(CELLClient* pclient);
	//子服务器中客户端断开事件，统一释放客户端资源
	virtual void OnLeave(CELLClient* pclient);
	//子服务器中客户端消息响应事件
	virtual void OnNetMsg(CellServer* cellserver, std::shared_ptr<CELLClient> pclient, MsgHeader * msg);
private:
	std::vector<CellServer*>	_CellServer;	//子服务器队列
	CELLTimestamp				_timer;			//高精度计时器
	std::atomic_int				_recvcount;		//收包计数
	std::atomic_int				_clientcount;	//客户端计数
};

//int a = sizeof(Server);	//88
#endif // !__SERVER__