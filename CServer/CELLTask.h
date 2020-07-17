#ifndef __CELLTASK__
#define __CELLTASK__
#include<thread>
#include<mutex>
#include<list>
#include"Alloctor.h"
#include<memory>
#include<functional>

class CellServer;

//执行任务的类
class CellTaskServer
{
	using CellTask = std::function<void()>;
public:
	CellTaskServer(CellServer* cellserver);
	virtual ~CellTaskServer();
	//添加任务到缓冲队列
	void addTask(CellTask task);
	//开启工作线程
	void start();

protected:
	//执行任务
	void onRun();

private:
	std::list<CellTask>			_tasklist;		//任务队列
	std::list<CellTask>			_tasklistbuf;	//任务队列缓冲区
	std::mutex						_mtx;			//任务队列缓冲区锁
	std::thread*					_thread;		//任务线程
	CellServer*						_cellserver;	//归属的server
};

//int a = sizeof(CellTaskServer);	//136


#endif // !__CELLTASK__



