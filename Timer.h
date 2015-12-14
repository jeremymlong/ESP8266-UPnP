#ifndef TIMER_H_
#define TIMER_H_

#include <Arduino.h>

class Timer
{
public:
	Timer(uint32 interval);
	void loop();
	void reset();
	bool elapsed;

private:
	uint64 m_lastReadTime;
	uint32 m_interval;
};

#endif
