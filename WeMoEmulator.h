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
	
	inline uint16 getPort() { return m_port; }

	String getDescriptionXml();
	void sendNotify(NotifyType nt, WiFiUDP* session);
	bool matchesSearch(String searchTerm);

protected:
	void respondToSearch(MSearchInfo* msearch);
	void setup();

private:
	String getNLS();

	uint8 m_pin;
};

#endif
