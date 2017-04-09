#include "TimerClass.h"

TimerClass::TimerClass()
{
}

TimerClass::TimerClass(const TimerClass&)
{
}

TimerClass::~TimerClass()
{
}

bool TimerClass::Initialize()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);

	ticksPerMs = float(double(li.QuadPart)/1000.0);

	if(ticksPerMs == 0)
	{
		return false;
	}

	QueryPerformanceFrequency(&li);
	startTime = li.QuadPart;

	return true;
}

void TimerClass::Frame()
{
	INT64 currentTime = 0;
	LARGE_INTEGER li;
	float timeDifference;

	QueryPerformanceCounter(&li);
	currentTime = li.QuadPart;

	timeDifference = static_cast<float>(currentTime - startTime);
	frameTime = timeDifference / ticksPerMs;
	allTime = currentTime / ticksPerMs;

	startTime = currentTime;
}

float TimerClass::GetTime()
{
	return allTime;
}

float TimerClass::GetFrameTime()
{
	return frameTime;
}
