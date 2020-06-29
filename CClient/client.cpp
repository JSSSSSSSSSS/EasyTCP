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
			cout << "��֧�ֵ�����" << endl;
		}
	}
	cout << "�����߳��˳�" << endl;
}
//�ͻ�������
const int cCount = 10000;
//�����߳�����
const int tCount = 4;
//�ͻ�������
Client *client[cCount];

void sendthread(int nid)
{
	//��ʼ�±�
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

	//���������߳�
	thread t(input);

	//������Ϣ
	MSG_LOGIN login;
	strcpy(login.username, "jiangsen");
	strcpy(login.password, "123456789");

	//������Ϣ
	while (g_ret)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->SendMsg(&login);
		}
	}
	//�Ͽ�����
	for (int i = begin; i < end; i++)
	{
		client[i]->Disconnect();
	}
}
void test_client()
{

	//���������߳�
	thread *ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = new thread(sendthread,i+1);
	}
	
	//���������߳�
	thread t(input);
	
	//�ȴ������߳�
	t.join();
	
	//�ȴ������߳�
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