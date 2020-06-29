#include<iostream>
#include<fstream>
#include<time.h>
#include<thread>
#include"../Winsocket/Msg.h"
#include"../Winsocket/wsabase.h"
#include"CServer.h"

using namespace std;
#define MAX_BUF_LEN 1024
//void make_header(char* header,const char* state,const char* content_type)
//{
//	time_t it = time(NULL);
//	tm *t = localtime(&it);
//	char date[128] = { 0 };
//	sprintf_s(date, 127, "%d-%d-%d %d:%d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
//
//	sprintf_s(header, 127, "HTTP/1.1 %s\r\nDate:%s\r\nContent-Type:%s\r\n\r\n",state,date,content_type);
//}
//void parse_file(const string& file,string &filename,string &suffix)
//{
//	size_t npos = file.find_last_of('.', file.size());
//	filename = file.substr(0, npos);
//	suffix = file.substr(npos + 1, file.size());
//}
//void handle_request(ConnectedSocket *s,string file)
//{
//	if (file == "HTTP/1.1")
//	{
//		file = "base.html";
//	}
//	string filename, suffix;
//	parse_file(file, filename, suffix);
//	if (suffix == "html")
//	{
//		ifstream infile(file, ios::in | ios::binary);
//		if (infile.good())
//		{
//			char rep[128] = { 0 };
//			make_header(rep, "200 OK", "text/html");
//			s->SendMsg(rep, strlen(rep));
//		}
//		else
//		{
//			char header[128] = { 0 };
//			make_header(header, "404 NOT FOUND", "text/html");
//			string rep = header;
//			rep.append("<h1 style = \"text-align:center\">404 not found</h1>");
//			s->SendMsg((char*)rep.c_str(), rep.length());
//			return;
//		}
//		while (!infile.eof())
//		{
//			char content[1024*10];
//			streamsize realen = infile.read(content, 1024*10-1).gcount();	//返回实际读取的字节数
//
//			s->SendMsg(content, realen);
//		}
//	}
//	else
//	{
//		char header[128] = { 0 };
//		make_header(header, "404 NOT FOUND", "text/html");
//		string rep = header;
//		rep.append("<h1 style = \"color:red\">error file type</h1>");
//		s->SendMsg((char*)rep.c_str(), rep.length());
//	}
//}
//void HTTP()
//{
//	wsabase startup;
//	ListenSocket server;
//	server.InitSocket();
//	if (!server.Listen(80))
//	{
//		getchar();
//		return;
//	}
//	ConnectedSocket *client = NULL;
//	char msg[MAX_BUF_LEN + 1] = { 0 };
//	while (1)
//	{
//		server.Accept(&client);
//		if(!client || client->GetSocket() == INVALID_SOCKET)
//		{
//			continue;
//		}
//
//		memset(msg, 0, MAX_BUF_LEN);
//		int retlen = client->RecvMsg(msg, MAX_BUF_LEN);
//		if (retlen > 0)
//		{
//			msg[retlen] = 0;
//			cout << msg << endl;
//			char filename[128] = { 0 };
//			sscanf_s(msg, "GET /%s HTTP/1.1", filename,127); //解析文本、空格结束
//			
//			/*string file = filename;
//			size_t pos = file.find_last_of('.', file.length());
//			cout << file.substr(pos, file.length() - pos).c_str() << endl;*/
//
//			handle_request(client, filename);	//处理请求
//		}
//		client->Shutdown(SD_SEND);
//		client->Close();
//	}
//}

bool g_ret = true;
void input(Server*server)
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
void test_server()
{
	Server server;
	if (!server.InitSocket())
	{
		cout << "InitSocket failed!" << endl;
		return;
	}
	if (!server.Bind(8001))
	{
		cout << "bind failed!" << endl;
		return;
	}
	if (!server.Listen())
	{
		cout << "Listen failed!"<< endl;
		return;
	}
	thread t(input, &server);
	while (g_ret)
	{
		server.Run();
	}
	t.join();
}
int main()
{
	//HTTP();
	test_server();
	//test_for();
	
	system("pause");
	return 0;
}

