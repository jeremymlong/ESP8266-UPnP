#ifndef WEMOEMULATOR_H_
#define WEMOEMULATOR_H_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "UpnpDevice.h"
#include "Ssdp.h"

class WeMoEmulator : public UpnpDevice
{
public:
	WeMoEmulator(String friendlyName, uint8 pin, uint16 port);

	void begin();
	void loop();

	inline String getFriendlyName() { return m_friendlyName; }
	inline uint16 getPort() { return m_port; }
	inline void setDateTime(DateTime* dateTime) { m_dateTime = dateTime; }

	String getSetupXml();
	void sendNotify(NotifyType nt, WiFiUDP* session);
	bool matchesSearch(String searchTerm);
	void addSearchRequest(MSearchInfo* msearch);
	bool respondToSearch();

private:
	void respondToSearch(MSearchInfo* msearch);
	String getNLS();
	inline DateTime* getDateTime() { return m_dateTime; }

	String m_friendlyName;
	uint8 m_pin;
	uint16 m_port;
	uint32 m_serial;
	ESP8266WebServer m_webServer;
	DateTime *m_dateTime;
	LinkedList<MSearchInfo*> m_searchRequests;
};

#endif