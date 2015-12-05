#ifndef UPNPDEVICE_H_
#define UPNPDEVICE_H_

#include <WiFiUdp.h>

#include "MSearchInfo.h"
#include "NotifyType.h"
#include "DateTime.h"

class UpnpDevice
{
public:
	virtual void begin() = 0;
	virtual void loop() = 0;

	virtual String getSetupXml() = 0;
	virtual void sendNotify(NotifyType nt, WiFiUDP* session) = 0;
	virtual String getFriendlyName() = 0;
	virtual bool matchesSearch(String searchTerm) = 0;
	virtual void addSearchRequest(MSearchInfo* msearch) = 0;
	virtual void setDateTime(DateTime* dateTime) = 0;
	virtual bool respondToSearch() = 0;
};

#endif