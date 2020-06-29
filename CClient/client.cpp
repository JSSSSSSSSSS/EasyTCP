#include<iostream>
#include<thread>
#include"CClient.h"
using namespace std;

bool g_ret= true;
void input()
{
	while (g_ret)
	{
		char inputbuf[4096] = { 0 };
		cin >> inputbuf;
		if (!strcmp(inputbuf, "exit"))
		{
			g_ret = false;
			break;
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}
	cout << "输入线程退出" << endl;
}
//客户端数量
const int cCount = 10000;
//发送线程数量
const int tCount = 4;
//客户端数组
Client *client[cCount];

void sendthread(int nid)
{
	//起始下标
	int begin = (nid -1)*(cCount / tCount);
	int end = (nid)*(cCount / tCount);
	int count = 0;
	for (int i = begin; i < end; i++)
	{
		client[i] = new Client();
		client[i]->InitSocket();
		if (client[i]->Connect("127.0.0.1", 8001))
		{
			cout << "Connect = " << ++count << endl;
		}
	}

	chrono::milliseconds t1(5000);
	std::this_thread::sleep_for(t1);

	//启动输入线程
	thread t(input);

	//构造消息
	MSG_LOGIN login;
	strcpy(login.username, "jiangsen");
	strcpy(login.password, "123456789");

	//发送消息
	while (g_ret)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->SendMsg(&login);
		}
	}
	//断开连接
	for (int i = begin; i < end; i++)
	{
		client[i]->Disconnect();
	}
}
void test_client()
{

	//启动发送线程
	thread *ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = new thread(sendthread,i+1);
	}
	
	//启动输入线程
	thread t(input);
	
	//等待输入线程
	t.join();
	
	//等待发送线程
	for (int i = 0; i < tCount; i++)
	{
		ts[i]->join();
	}
}
int main()
{
	test_client();

	system("pause");
	return 0;
}