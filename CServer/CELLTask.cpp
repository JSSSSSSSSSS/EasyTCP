#include "CELLTask.h"
#include"CellServer.h"
#include<iostream>
using namespace std;

CellTaskServer::CellTaskServer(CellServer* cellserver)
	:_cellserver(cellserver)
{
}

CellTaskServer::~CellTaskServer()
{
	if (_thread)
	{
		_thread->join();
		delete _thread;
		_thread = nullptr;
	}
}

void CellTaskServer::addTask(CellTask task)
{
	std::lock_guard<std::mutex> guard(_mtx);
	_tasklistbuf.push_back(task);
}

void CellTaskServer::start()
{
	_thread = new std::thread(&CellTaskServer::onRun, this);
}

void CellTaskServer::onRun()
{
	while(_cellserver->IsRun())
	{
		//�ӻ��������ȡ������
		{
			std::lock_guard<std::mutex> guard(_mtx);
			if (!_tasklistbuf.empty())
			{
				for (auto task : _tasklistbuf)
				{
					_tasklist.push_back(task);
				}
				_tasklistbuf.clear();
			}
		}
		//���û������
		if (_tasklist.empty())
		{
			std::chrono::milliseconds t(100);
			std::this_thread::sleep_for(t);
			continue;
		}

		//��������
		for (auto task : _tasklist)
		{
			task();
		}
		//����������
		_tasklist.clear();
	}
}
