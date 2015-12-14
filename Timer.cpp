#include "Timer.h"

Timer::Timer(uint32 interval) :
	elapsed(false),
	m_lastReadTime(0),
	m_interval(interval)
{
}

void Timer::reset()
{
	elapsed = false;
	m_lastReadTime = millis();
}

void Timer::loop()
{
	uint64 currentMillis = millis();
	if ((currentMillis - m_lastReadTime) > m_interval)
	{
		elapsed = true;
		m_lastReadTime = currentMillis;
	}
}
