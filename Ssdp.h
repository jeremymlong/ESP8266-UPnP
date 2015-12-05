#ifndef SSDP_H__
#define SSDP_H__

#include <Arduino.h>
#include <WiFiUdp.h>
#include <Timer.h>

#include "UpnpDevice.h"
#include "DateTime.h"
#include "HttpRequest.h"
#include "MSearchInfo.h"
#include "NotifyType.h"
#include "LinkedList.h"

#define SSDP_ADDR IPAddress(239,255,255,250)
#define SSDP_PORT 1900
#define SSDP_UNICAST_PORT 2869

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

protected:
	inline IPAddress getIPAddress() { return _localIP; }
	inline uint8 getTTL() { return _ttl; }
	WiFiUDP* getMulticastSession() { return &_multicast; }

private:
	uint32 getNextBootID();
	uint16 getConfigID();

	WiFiUDP _multicast;
	IPAddress _localIP;
	uint8 _ttl;
	DateTime* m_dateTime;
	Arduino::Timer m_timer;
	uint8 _udpSendCount;
	LinkedList<UpnpDevice*> m_devices;
};

#endif