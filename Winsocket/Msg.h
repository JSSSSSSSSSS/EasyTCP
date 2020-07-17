#pragma once
#ifndef _MSG_
#define _MSG_

#define MSG_SEG_SIZE_MAX	4096
enum MsgType
{
	MT_LOGIN,
	MT_LOGINRE,
	MT_HEART,
	MT_HEARTRE
};
class MsgHeader
{
public:
	MsgType type;
	int length;
};

class MSG_LOGIN :public MsgHeader
{
public:
	MSG_LOGIN()
	{
		type = MT_LOGIN;
		length = sizeof(MSG_LOGIN);
	}
	char username[15];
	char password[17];
	char data[60];
};

class MSG_LOGINRE :public MsgHeader
{
public:
	MSG_LOGINRE()
	{
		type = MT_LOGINRE;
		length = sizeof(MSG_LOGINRE);
	}
	int isok;
	char data[88];
};

class MSG_HEART :public MsgHeader
{
public:
	MSG_HEART()
	{
		type = MT_HEART;
		length = sizeof(MSG_HEART);
		isok = 1;
	}
	int isok;
};
class MSG_HEARTRE :public MsgHeader
{
public:
	MSG_HEARTRE()
	{
		type = MT_HEARTRE;
		length = sizeof(MSG_HEARTRE);
		isok = 2;
	}
	int isok;
};
#endif