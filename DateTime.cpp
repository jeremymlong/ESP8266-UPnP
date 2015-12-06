#include "DateTime.h"

const char* DateTime::Jan = "Jan";
const char* DateTime::Feb = "Feb";
const char* DateTime::Mar = "Mar";
const char* DateTime::Apr = "Apr";
const char* DateTime::May = "May";
const char* DateTime::Jun = "Jun";
const char* DateTime::Jul = "Jul";
const char* DateTime::Aug = "Aug";
const char* DateTime::Sep = "Sep";
const char* DateTime::Oct = "Oct";
const char* DateTime::Nov = "Nov";
const char* DateTime::Dec = "Dec";

const char* DateTime::Sun = "Sun";
const char* DateTime::Mon = "Mon";
const char* DateTime::Tue = "Tue";
const char* DateTime::Wed = "Wed";
const char* DateTime::Thu = "Thu";
const char* DateTime::Fri = "Fri";
const char* DateTime::Sat = "Sat";

DateTime::DateTime() : 
	m_dayOfWeek(0),
	m_dayOfMonth(1),
	m_month(0),
	m_year(0),
	m_hour(0),
	m_minute(0),
	m_second(0),
	m_timeZone("GMT"),
	m_timer(1000)
{
}

DateTime::~DateTime()
{
}

void DateTime::begin()
{
}

void DateTime::loop()
{
	m_timer.loop();
	if (m_timer.elapsed)
	{
		incrementSecond();
		m_timer.reset();
	}
}

void DateTime::parseDateString(String date)
{
#if defined(CONSOLE) && defined(DEBUG_DATETIME)
	CONSOLE.print("Parsing date: ");
	CONSOLE.println(date);
#endif
	// Sat, 28 Nov 2015 16:43:19 GMT
	int startIndex = 0, endIndex = 0;
	if (startIndex > -1)
	{
		char split[7] = { ',', ' ', ' ', ' ', ':', ':', ' ' };
		String results[7];
		for (byte i = 0; i < 7; i++)
		{
			int index = date.indexOf(split[i]);
			results[i] = date.substring(0, index);
			date = date.substring(index + 1);
			date.trim();
		}
		m_dayOfWeek = wdayToInt(results[0]);
		m_dayOfMonth = results[1].toInt();
		m_month = monthToInt(results[2]);
		m_year = results[3].toInt();
		m_hour = results[4].toInt();
		m_minute = results[5].toInt();
		m_second = results[6].toInt();
		m_timeZone = date; // whatever is left.  Should be GMT
	}

#if defined(CONSOLE) && defined(DEBUG_DATETIME)
	CONSOLE.print("Parsed date:  ");
	CONSOLE.println(toString());
#endif
}

void DateTime::update()
{
	parseDateString(updateFunc());
}

String DateTime::toString()
{	
	char buffer[32];
	snprintf(buffer, 32, "%s, %02u %s %u %02u:%02u:%02u %s", 
		intToWDay(m_dayOfWeek).c_str(),
		m_dayOfMonth,
		intToMonth(m_month).c_str(),
		m_year,
		m_hour,
		m_minute,
		m_second,
		m_timeZone.c_str());
	return String(buffer);
}

uint8 DateTime::wdayToInt(String sWDay)
{
	const char* pszDay = sWDay.c_str();
	if (strcmp(pszDay, Sun) == 0)
	{
		return 0;
	}
	else if (strcmp(pszDay, Mon) == 0)
	{
		return 1;
	}
	else if (strcmp(pszDay, Tue) == 0)
	{
		return 2;
	}
	else if (strcmp(pszDay, Wed) == 0)
	{
		return 3;
	}
	else if (strcmp(pszDay, Thu) == 0)
	{
		return 4;
	}
	else if (strcmp(pszDay, Fri) == 0)
	{
		return 5;
	}
	else if (strcmp(pszDay, Sat) == 0)
	{
		return 6;
	}

	return 0;
}

String DateTime::intToWDay(uint8 unDay)
{
	switch (unDay)
	{
	default:
	case 0: return String(Sun);
	case 1: return String(Mon);
	case 2: return String(Tue);
	case 3: return String(Wed);
	case 4: return String(Thu);
	case 5: return String(Fri);
	case 6: return String(Sat);
	}
}

uint8 DateTime::monthToInt(String sMonth) 
{
	const char* pszMonth = sMonth.c_str();
	if (strcmp(pszMonth, Jan) == 0) 
	{
		return 0;
	}
	else if (strcmp(pszMonth, Feb) == 0) 
	{
		return 1;
	}
	else if (strcmp(pszMonth, Mar) == 0)
	{
		return 2;
	}
	else if (strcmp(pszMonth, Apr) == 0)
	{
		return 3;
	}
	else if (strcmp(pszMonth, May) == 0)
	{
		return 4;
	}
	else if (strcmp(pszMonth, Jun) == 0)
	{
		return 5;
	}
	else if (strcmp(pszMonth, Jul) == 0)
	{
		return 6;
	}
	else if (strcmp(pszMonth, Aug) == 0)
	{
		return 7;
	}
	else if (strcmp(pszMonth, Sep) == 0)
	{
		return 8;
	}
	else if (strcmp(pszMonth, Oct) == 0)
	{
		return 9;
	}
	else if (strcmp(pszMonth, Nov) == 0)
	{
		return 10;
	}
	else if (strcmp(pszMonth, Dec) == 0)
	{
		return 11;
	}
	return 0;
}

String DateTime::intToMonth(uint8 unMonth)
{
	switch (unMonth)
	{
	default:
	case 0: return String(Jan);
	case 1: return String(Feb);
	case 2: return String(Mar);
	case 3: return String(Apr);
	case 4: return String(May);
	case 5: return String(Jun);
	case 6: return String(Jul);
	case 7: return String(Aug);
	case 8: return String(Sep);
	case 9: return String(Oct);
	case 10: return String(Nov);
	case 11: return String(Dec);
	}
}

void DateTime::incrementSecond()
{
	m_second++;
	if (m_second > 59)
	{
		m_second = 0;
		m_minute++;
		if (m_minute > 59)
		{
			m_minute = 0;
			m_hour++;
			if (m_hour > 23)
			{
				m_hour = 0;
			}
			update();
		}
	}

#if defined(CONSOLE) && defined(DEBUG_DATETIME)
	CONSOLE.print("Updated date:  ");
	CONSOLE.println(toString());
#endif
}

uint32 DateTime::getEpoch()
{
	/*
	1 hour	3600 seconds
	1 day	86400 seconds
	1 week	604800 seconds
	1 month (30.44 days) 	2629743 seconds
	1 year (365.24 days) 	 31556926 seconds
	*/
	return (m_year - 2000) * 31556926U + 
		m_month * 2629743U + 
		m_dayOfMonth * 86400U +
		m_hour * 3600 +
		m_second;
}