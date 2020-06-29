#pragma once
#ifndef _CELLTimestamp_
#define _CELLTimestamp_
#include<chrono>	//高精度时钟
using namespace std::chrono;
class CELLTimestamp
{
public:
	CELLTimestamp();
	~CELLTimestamp();
	/**
	*	更新时间
	*/
	void Update();
	/**
	*	获取当前秒
	*/
	double GetElapsedSecond();
	/**
	*	获取毫秒
	*/
	double GetElapesdMilliSec();
	/**
	*	获取微秒
	*/
	double GetElapsedMicroSec();
private:
	time_point<high_resolution_clock> _begin;
};
#endif