#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std::chrono;

class PerformanceTimer
{
public:
	PerformanceTimer();
	PerformanceTimer(const PerformanceTimer&);
	~PerformanceTimer();

	void StartTimer();

	double GetTime() const;
	std::string GetFormattedDuration(std::string text) const;
	std::string FormatTime(double time) const;

private:
	std::chrono::high_resolution_clock::time_point startTime;

};