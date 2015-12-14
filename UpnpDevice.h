#ifndef UPNPDEVICE_H_
#define UPNPDEVICE_H_

#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "MSearchInfo.h"
#include "NotifyType.h"
#include "DateTime.h"

#define SSDP_ADDR IPAddress(239,255,255,250)
#define SSDP_PORT 1900
#define SSDP_UNICAST_PORT 2869

class UpnpDevice
{
public:
	UpnpDevice(String friendlyName, uint16 port);
	virtual ~UpnpDevice() {}

	void begin();
	void loop();

	virtual String getDescriptionXml() = 0;
	virtual void sendNotify(NotifyType nt, WiFiUDP *session) = 0;
	virtual bool matchesSearch(String searchTerm);
	virtual void addSearchRequest(MSearchInfo *msearch);

	bool respondToSearch();
	inline void setDateTime(DateTime* dateTime) { m_dateTime = dateTime; }
	inline String getFriendlyName() { return m_friendlyName; }

protected:
	virtual void respondToSearch(MSearchInfo* msearch) = 0;
	virtual void setup() = 0;

	String m_friendlyName;
	uint32 m_serial;
	uint16 m_port;
	DateTime *m_dateTime;
	ESP8266WebServer m_webServer;

private:
	LinkedList<MSearchInfo*> m_searchRequests;
};

#endif
