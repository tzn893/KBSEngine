#pragma once
#include <string>

struct CalendarTime {
	int year;
	int month;
	int date;
	int hour;
	int mintue;
	int second;

	std::string ymd();
	std::string hms();
};

class Timer{
public:
	virtual bool initialize();
	virtual void tick();
	virtual void finalize();

	float TotalTime();
	float DeltaTime();
	CalendarTime Calendar();

	void Reset();
	void Pause();
	void Start();

private:

	uint64_t prevClockTick;
	uint64_t currClockTick;
	uint64_t pauseClockTick;
	uint64_t startClockTick;

	uint64_t clockFrequency;

	bool paused;
};

inline Timer gTimer;