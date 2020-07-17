#pragma once
#ifndef _CELLTimestamp_
#define _CELLTimestamp_
#include<chrono>	//�߾���ʱ��
using namespace std::chrono;

//ʱ����
class CELLTime
{
public:
	//��ȡ��ǰʱ���
	static time_t GetNowTimeMiliSec();
};


//��ʱ����
class CELLTimestamp
{
public:
	CELLTimestamp();
	~CELLTimestamp();
	/**
	*	����ʱ��
	*/
	void Update();
	/**
	*	��ȡ��ǰ��
	*/
	double GetElapsedSecond();
	/**
	*	��ȡ����
	*/
	double GetElapesdMilliSec();
	/**
	*	��ȡ΢��
	*/
	double GetElapsedMicroSec();
private:
	time_point<high_resolution_clock> _begin;
};
#endif