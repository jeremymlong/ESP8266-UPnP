#ifndef SSDP_H__
#define SSDP_H__

#include <Arduino.h>
#include <WiFiUdp.h>

#include "Timer.h"
#include "UpnpDevice.h"
#include "DateTime.h"
#include "HttpRequest.h"
#include "MSearchInfo.h"
#include "NotifyType.h"
#include "LinkedList.h"

class Ssdp
{
public:
	Ssdp(IPAddress ip);
	~Ssdp();
	void begin();
	void loop();
	
	inline DateTime* getDateTime() { return m_dateTime; }
	inline void setDateTime(DateTime* dateTime) { m_dateTime = dateTime; }
	inline void setUdpSendCount(uint8 count) { _udpSendCount = count; }
	inline void setIPAddress(IPAddress ip) { _localIP = ip; }
	void addDevice(UpnpDevice* device);

private:
	WiFiUDP _multicast;
	IPAddress _localIP;
	uint8 _ttl;
	DateTime* m_dateTime;
	Timer m_timer;
	uint8 _udpSendCount;
	LinkedList<UpnpDevice*> m_devices;
};

#endif
