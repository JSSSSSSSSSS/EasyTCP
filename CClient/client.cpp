#include<iostream>
#include<thread>
#include"CClient.h"
#include"CELLTimestamp.h"
#include"Msg.h"
#include<atomic>
using namespace std;

bool g_ret= true;
void input()
{
	while (g_ret)
	{
		char inputbuf[32] = { 0 };
		cin >> inputbuf;
		if (!strcmp(inputbuf, "exit"))
		{
			g_ret = false;
			break;
		}
		else
		{
			cout << "command not found!" << endl;
		}
	}
	cout << "input thread exit!" << endl;
}
//�ͻ�������
const int cCount = 8;
//�����߳�����
const int tCount = 4;
//�ͻ�������
Client *client[cCount];
//���ͼ���
atomic_int sendCount;
//׼�������̼߳���
atomic_int readyCount;
void recvthread( int begin, int end)
{
	//������Ϣ
	while (g_ret)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->Run();
		}
	}
}
void sendthread(int nid)
{
	//�����±꣬Ϊ�̷߳���ͻ���
	int begin = (nid -1)*(cCount / tCount);
	int end = (nid)*(cCount / tCount);
	for (int i = begin; i < end; i++)
	{
		client[i] = new Client();
		client[i]->InitSocket();
	}
	//���ӿͻ���
	for (int i = begin; i < end; i++)
	{
		//aliyun 39.97.254.36
		//localhost 127.0.0.1
		client[i]->Connect("127.0.0.1", 8001);
	}
	cout << "thread<" << nid << ">,Connect =< " << begin <<","<<end<<">"<< endl;
	
	readyCount++;
	while(readyCount < tCount)
	{
		chrono::milliseconds t1(1000);
		std::this_thread::sleep_for(t1);
	}

	thread tr(recvthread, begin,end);
	tr.detach();

	//������Ϣ
	MSG_LOGIN login[10];
	int nlen = sizeof(login);
	
	//������Ϣ
	while (g_ret)
	{
		for (int i = begin; i < end; i++)
		{
			if (-1 != client[i]->SendMsg(login,nlen))
			{
				sendCount++;
			}
		}
	}
	//�Ͽ�����
	for (int i = begin; i < end; i++)
	{
		client[i]->Disconnect();
	}
	//�Ͽ�����
	for (int i = begin; i < end; i++)
	{
		delete client[i];
	}
	cout << "thread<" << nid << "> exit!" << endl;
}

void test_client()
{
	//���������߳�
	thread t(input);
	t.detach();

	thread ts[tCount];
	//���������߳�
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(sendthread, i + 1);
		ts[i].detach();
	}
}

int main()
{
	sendCount.store(0);
	readyCount.store(0);

	test_client();

	CELLTimestamp timer;
	chrono::milliseconds t1(100);

	while (g_ret)
	{
		double t = timer.GetElapsedSecond();
		if (t >= 1)
		{
			printf("time<%lf>,sendCount<%d>\n",t,sendCount.load());
			sendCount = 0;
			timer.Update();
		}
		this_thread::sleep_for(t1);
	}
	cout << "input a char to exit!" << endl;
	getchar();
	return 0;
}