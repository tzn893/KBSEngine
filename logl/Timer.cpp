#include "Timer.h"
#include <time.h>
#pragma warning(disable : 4996) 
#ifdef _WIN64
#include <Windows.h>

#define QUERY_CLOCK_TICK(out) QueryPerformanceCounter((LARGE_INTEGER*)&out)
#define QUERY_CLOCK_FREQUENCY(out) QueryPerformanceFrequency((LARGE_INTEGER*)&out)
#endif

bool Timer::initialize() {
	QUERY_CLOCK_FREQUENCY(clockFrequency);
	Reset();

	return true;
}

void Timer::finalize() {}

void Timer::Reset() {
	pauseClockTick = 0;

	paused = false;

	QUERY_CLOCK_TICK(startClockTick);

	currClockTick = prevClockTick = startClockTick;
}

void Timer::Pause() {
	prevClockTick = currClockTick;

	paused = true;
}

void Timer::Start() {
	paused = false;
}

void Timer::tick() {
	if (!paused) {
		prevClockTick = currClockTick;

		QUERY_CLOCK_TICK(currClockTick);
	}
	else {
		uint64_t temp;

		QUERY_CLOCK_TICK(temp);

		pauseClockTick += temp - prevClockTick;
		currClockTick = prevClockTick = temp;
	}
}

float Timer::DeltaTime() {
	return (currClockTick - prevClockTick) / static_cast<float>(clockFrequency);
}

float Timer::TotalTime() {
	return (currClockTick - pauseClockTick - startClockTick) / static_cast<float>(clockFrequency);
}

CalendarTime Timer::Calendar() {
	CalendarTime result;
	time_t raw_time;


	time(&raw_time);
	tm* local_time = localtime(&raw_time);

	result.year = local_time->tm_year + 1900;
	result.month = local_time->tm_mon + 1;
	result.date = local_time->tm_mday;

	result.hour = local_time->tm_hour;
	result.mintue = local_time->tm_min;
	result.second = local_time->tm_sec;

	return result;
}

std::string CalendarTime::ymd() {
	std::string format = "0000/00/00";
	format[0] = year / 1000 + '0';
	format[1] = year / 100 % 10 + '0';
	format[2] = year / 10 % 10 + '0';
	format[3] = year % 10 + '0';

	format[5] = month / 10 + '0';
	format[6] = month % 10 + '0';

	format[8] = date / 10 + '0';
	format[9] = date % 10 + '0';

	return std::move(format);
}

std::string CalendarTime::hms() {
	std::string format = "00:00:00";
	format[0] = hour / 10 + '0';
	format[1] = hour % 10 + '0';

	format[3] = mintue / 10 + '0';
	format[4] = mintue % 10 + '0';

	format[6] = second / 10 + '0';
	format[7] = second % 10 + '0';

	return std::move(format);
}