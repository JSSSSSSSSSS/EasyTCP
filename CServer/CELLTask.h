#ifndef __CELLTASK__
#define __CELLTASK__
#include<thread>
#include<mutex>
#include<list>
#include"Alloctor.h"
#include<memory>
#include<functional>

class CellServer;

//ִ���������
class CellTaskServer
{
	using CellTask = std::function<void()>;
public:
	CellTaskServer(CellServer* cellserver);
	virtual ~CellTaskServer();
	//������񵽻������
	void addTask(CellTask task);
	//���������߳�
	void start();

protected:
	//ִ������
	void onRun();

private:
	std::list<CellTask>			_tasklist;		//�������
	std::list<CellTask>			_tasklistbuf;	//������л�����
	std::mutex						_mtx;			//������л�������
	std::thread*					_thread;		//�����߳�
	CellServer*						_cellserver;	//������server
};

//int a = sizeof(CellTaskServer);	//136


#endif // !__CELLTASK__



