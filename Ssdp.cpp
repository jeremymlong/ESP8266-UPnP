#include "Ssdp.h"

#define DEBUG_SSDP
#define SSDP_NOTIFY_SEC 1800

Ssdp::Ssdp(IPAddress ip) :
	_localIP(ip),
	_ttl(2),
	m_timer(SSDP_NOTIFY_SEC * 1000),
	_udpSendCount(2),
	m_devices()
{
}

Ssdp::~Ssdp()
{
}

void Ssdp::begin()
{
	bool result = _multicast.beginMulticast(_localIP, SSDP_ADDR, SSDP_PORT);
#if defined(CONSOLE) && defined(DEBUG_SSDP)
	if (!result)
	{
		CONSOLE.println("Ssdp: Failed to join the multicast group");
	}
#endif

	for (LinkedListItem<UpnpDevice*>* ud = m_devices.getFirst(); ud; ud = ud->getNext())
	{
		UpnpDevice* device = ud->getItem();

		device->setDateTime(m_dateTime);
		device->begin();

		if (_multicast.beginPacketMulticast(SSDP_ADDR, SSDP_PORT, _localIP, _ttl))
		{
			device->sendNotify(NT_ALIVE, &_multicast);
			_multicast.endPacket();
		}
#if defined(CONSOLE) && defined(DEBUG_SSDP)
		else
		{
			CONSOLE.printf("Ssdp::begin - beginMulticast failed for %s\n", device->getFriendlyName().c_str());
		}
#endif
	}
}

void Ssdp::loop()
{
	m_timer.loop();

	for (LinkedListItem<UpnpDevice*>* ud = m_devices.getFirst(); ud; ud = ud->getNext())
	{
		if (ud->getItem()->respondToSearch())
		{
			return;
		}
		ud->getItem()->loop();
	}

	if (m_timer.elapsed)
	{
		for (LinkedListItem<UpnpDevice*>* ud = m_devices.getFirst(); ud; ud = ud->getNext())
		{
			_multicast.beginPacketMulticast(SSDP_ADDR, SSDP_PORT, _localIP, _ttl);
			ud->getItem()->sendNotify(NT_UPDATE, &_multicast);
			_multicast.endPacket();
		}
		m_timer.reset();
	}

	if (_multicast.parsePacket())
	{
		HttpRequest* request = NULL;
		bool receivingNotifyPacket = false;
		while (_multicast.available())
		{
			String line = _multicast.readStringUntil('\r');
			line.trim();
			if (line.equals("M-SEARCH * HTTP/1.1"))
			{
				request = new HttpRequest();
				request->RemoteIP = _multicast.remoteIP();
				request->RemotePort = _multicast.remotePort();
				request->RequestLine = line;
			}
			else if (receivingNotifyPacket || line.equals("NOTIFY * HTTP/1.1"))
			{
				// Ignore notify packets
				receivingNotifyPacket = true;
			}
			else if (request != NULL)
			{
				int colonIndex = line.indexOf(':');
				if (colonIndex > -1)
				{
					String name = line.substring(0, colonIndex);
					String value = line.substring(colonIndex + 1);
					name.trim();
					value.trim();
					request->setHeader(name, value);
				}
			}
#if defined(CONSOLE) && defined(DEBUG_SSDP)
			else
			{
				CONSOLE.print("Ssdp::loop - Unexpected line: ");
				CONSOLE.println(line.c_str());
			}
#endif
		}
		if (NULL != request)
		{
			if (request->hasHeader("ST") && request->hasHeader("MX"))
			{
				HttpHeader* stHeader = request->getHeader("ST");
				HttpHeader* mxHeader = request->getHeader("MX");

#if defined(CONSOLE) && defined(DEBUG_SSDP)
				CONSOLE.println("M-SEARCH");
				CONSOLE.printf("\tST: %s\n", stHeader->Value.c_str());
				CONSOLE.printf("\tMX: %s\n", mxHeader->Value.c_str());
#endif

				for (LinkedListItem<UpnpDevice*>* ud = m_devices.getFirst(); ud; ud = ud->getNext())
				{
					UpnpDevice* upnpDevice = ud->getItem();
					if (upnpDevice->matchesSearch(stHeader->Value))
					{
						for (int i = 0; i < _udpSendCount; i++)
						{
							MSearchInfo* msearch = new MSearchInfo();
							msearch->RemoteIP = request->RemoteIP;
							msearch->RemotePort = request->RemotePort;
							msearch->SearchTarget = stHeader->Value;
							uint8 mx = mxHeader->Value.toInt();
							msearch->ProcessTime = random(mx * 1000) + millis();
							upnpDevice->addSearchRequest(msearch);
						}
					}
				}
			}

			delete request;
			request = NULL;
		}
	} // End packet process
}

void Ssdp::addDevice(UpnpDevice* device)
{
#if defined(CONSOLE) && defined(DEBUG_SSDP)
	CONSOLE.println("Ssdp: Added device " + device->getFriendlyName());
#endif
	m_devices.add(device);
}