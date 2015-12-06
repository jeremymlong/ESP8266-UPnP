#ifndef DATETIME_H_
#define DATETIME_H_

#include <functional>

#include "HttpClient.h"
#include "Timer.h"

typedef String (*UpdateFunc)();

class DateTime
{
public:
	DateTime();
	~DateTime();
	String toString();
	uint32 getEpoch();
	inline void setUpdateCallback(UpdateFunc uf) { this->updateFunc = uf; }
	void begin();
	void loop();
	void update();

	static const char* Jan;
	static const char* Feb;
	static const char* Mar;
	static const char* Apr;
	static const char* May;
	static const char* Jun;
	static const char* Jul;
	static const char* Aug;
	static const char* Sep;
	static const char* Oct;
	static const char* Nov;
	static const char* Dec;

	static const char* Mon;
	static const char* Tue;
	static const char* Wed;
	static const char* Thu;
	static const char* Fri;
	static const char* Sat;
	static const char* Sun;

private:
	uint8 monthToInt(String sMonth);
	String intToMonth(uint8 unMonth);
	uint8 wdayToInt(String sWDay);
	String intToWDay(uint8 unDay);
	void incrementSecond();
	void parseDateString(String dateString);

	uint8 m_dayOfWeek;
	uint8 m_dayOfMonth;
	uint8 m_month;
	uint16 m_year;
	uint8 m_hour;
	uint8 m_minute;
	uint8 m_second;
	String m_timeZone;
	Timer m_timer;
	UpdateFunc updateFunc;
};

#endif
