#pragma once
#include"../Winsocket/wsabase.h"
#include"../Winsocket/CELLTimestamp.h"
#include<list>
#include<vector>
#include<mutex>
#include<atomic>
#include<thread>
#define THREAD_COUNT 4
struct MsgHeader;

//客户端类型
class ClientSocket
{
public:
	ClientSocket(Socket *socket = NULL);	
	~ClientSocket();
	//获得客户端socket
	Socket *GetSocket()const;
	//获取缓冲区大小
	char* GetBuf();
	//获取缓冲区尾指针位置
	int GetLastPos()const;
	//设置缓冲区尾指针
	void SetLastPos(int newpos);
private:
	Socket			*_socket;					//socket与addr封装的类
	char			*_msgbuf;					//第二缓冲区
	int				_lastpos;					//缓冲区尾部位置
};

//网络事件
class INetEvent
{
public:
	INetEvent();
	virtual ~INetEvent();
	//客户端退出事件
	virtual void OnLeave(ClientSocket* pclient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(ClientSocket* pclient, MsgHeader *msg) = 0;
};


//子服务器，处理接收客户端发送的数据
class CellServer
{
public:
	CellServer(Socket *socket = NULL, INetEvent* pevent = NULL);
	~CellServer();
	
	//开启主运行线程
	void Start();
	//获取cellserver实际服务的客户端数量
	size_t GetClientCount();
	//往缓冲区中添加客户端
	void AddClient(ClientSocket* pclient);

private:
	//主运行函数，进行select与数据接收
	void OnRun();
	//是否运行
	bool IsRun();
	//断开指定客户端的连接
	void Disconnect(ClientSocket * pclient);
	//断开所有客户端，并终止服务器运行
	void terminated();
	//接收数据
	int RecvMsg(ClientSocket* pclient);
	//响应网络消息
	virtual void OnNetMsg(ClientSocket* pclient, MsgHeader*msg);

private:
	std::list<ClientSocket*>	_client;		//客户端队列
	std::list<ClientSocket*>	_clientbuf;		//缓冲客户端队列
	Socket *					_socket;		//socket与addr
	char *						_recvbuf;		//接收缓冲区
	std::thread*				_pThread;		//线程
	INetEvent*					_pevent;		//事件
	std::mutex					_mtx;			//缓冲队列锁
};

//主服务器，处理连接与其他业务
class Server:public wsabase,public INetEvent
{
public:
	Server();
	virtual ~Server();
	//初始化监听套接字
	bool InitSocket(int af = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
	//绑定端口
	bool Bind(unsigned int port);
	//监听端口
	bool Listen();
	//接受客户端连接,为客户端分配空间
	void Accept();
	//启动子服务器接收数据
	void StartCell();
	//将接收的客户端添加到子服务器缓冲区
	void AddClientToCellServer(ClientSocket*client);
	//消息计数
	void MsgCount();
	//发送消息
	int SendMsg(ClientSocket*client,MsgHeader*msg);
	//主运行函数
	void Run();
	//是否运行
	bool IsRun()const;
	//结束运行
	void terminated();
	//获取监听套接字
	SOCKET GetListenSocket()const;
	//子服务器中客户端断开事件，统一释放客户端资源
	virtual void OnLeave(ClientSocket* pclient);
	//子服务器中客户端消息响应事件
	virtual void OnNetMsg(ClientSocket* pclient, MsgHeader *msg);
private:
	Socket *					_socket;		//socket与addr
	std::list<ClientSocket*>	_client;		//客户端队列
	std::vector<CellServer*>	_CellServer;	//小服务器队列
	std::mutex					_mtx;			//客户端队列锁,因为OnLeave中其他线程操作了_client
	CELLTimestamp				_timer;			//高精度计时器
	std::atomic_int				_recvcount;		//收包计数器
};

