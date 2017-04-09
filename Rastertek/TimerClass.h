#pragma once

#include <Windows.h>

class TimerClass
{
public:
	TimerClass();
	TimerClass(const TimerClass&);
	~TimerClass();

	bool Initialize();
	void Frame();

	float GetTime();
	float GetFrameTime();

private:
	INT64 startTime;
	float ticksPerMs, frameTime, allTime;
	double frequency;
};