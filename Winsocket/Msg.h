#pragma once
#ifndef _MSG_
#define _MSG_
#define MSG_SEG_SIZE_MAX	4096
enum MsgType
{
	MT_LOGIN,
	MT_LOGINRE,
};
struct MsgHeader
{
	MsgType type;
	int length;
};

struct MSG_LOGIN :public MsgHeader
{
	MSG_LOGIN()
	{
		type = MT_LOGIN;
		length = sizeof(MSG_LOGIN);
	}
	char username[15];
	char password[17];
	char data[88];
};

struct MSG_LOGINRE :public MsgHeader
{
	MSG_LOGINRE()
	{
		type = MT_LOGINRE;
		length = sizeof(MSG_LOGINRE);
	}
	int isok;
	char data[116];
};

#endif