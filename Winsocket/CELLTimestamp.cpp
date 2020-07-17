#include "CELLTimestamp.h"

time_t CELLTime::GetNowTimeMiliSec()
{
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}



CELLTimestamp::CELLTimestamp()
{
	Update();
}


CELLTimestamp::~CELLTimestamp()
{
}

void CELLTimestamp::Update()
{
	_begin = high_resolution_clock::now();
}

double CELLTimestamp::GetElapsedSecond()
{
	return GetElapsedMicroSec()*0.000001;
}

double CELLTimestamp::GetElapesdMilliSec()
{
	return GetElapsedMicroSec()*0.001;
}

double CELLTimestamp::GetElapsedMicroSec()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
}


