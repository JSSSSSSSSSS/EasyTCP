#include "CELLTimestamp.h"


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
