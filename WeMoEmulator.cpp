#include "WeMoEmulator.h"

//#define DEBUG_WEMOEMULATOR

const char* _setupTemplate =
"<?xml version=\"1.0\"?>"
"<root>"
"<device>"
"<deviceType>urn:ArduinoESP:device:controllee:1</deviceType>"
"<friendlyName>%s</friendlyName>" // friendlyName
"<manufacturer>Belkin International Inc.</manufacturer>"
"<modelName>Emulated Socket</modelName>"
"<modelNumber>ESP8266</modelNumber>"
"<UDN>uuid:Socket-1_0-%u</UDN>"	// Serial Number
"</device>"
"</root>\r\n\r\n";

const char* _notifyTemplate =
"NOTIFY * HTTP/1.1\r\n"
"HOST: 239.255.255.250:1900\r\n"
"CACHE-CONTROL: max-age=86400\r\n"
"LOCATION: http://%u.%u.%u.%u:%u/setup.xml\r\n" // ip, port
"OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
"01-NLS: %s\r\n" // nls - ?this is kind of like bootid, but it's a guid?
"NT: urn:Belkin:service:manufacture:1\r\n"
"NTS: ssdp:%s\r\n" // alive or update
"SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
"X-User-Agent: redsonic\r\n"
"USN: uuid:Socket-1_0-%u::urn:Belkin:service:manufacture:1\r\n"// chip ID
"\r\n";

WeMoEmulator::WeMoEmulator(String friendlyName, uint8 pin, uint16 port) :
	m_friendlyName(friendlyName),
	m_pin(pin),
	m_port(port),
	m_serial(0),
	m_webServer(port)
{
	pinMode(m_pin, OUTPUT);

	m_serial = ESP.getChipId();
	for (int i = 0; i < m_friendlyName.length(); i++)
	{
		m_serial *= m_friendlyName[i];
	}
}

void WeMoEmulator::begin()
{
	m_webServer.on("/setup.xml", HTTP_GET, [this]()
	{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
		CONSOLE.println("webServer: /setup.xml");
#endif
		this->m_webServer.send(200, "text/xml", this->getSetupXml());
	});

	m_webServer.on("/upnp/control/basicevent1", HTTP_POST, [this]()
	{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
		CONSOLE.println("webServer: /upnp/control/basicevent1");
#endif
		// The web server class has a bug with POST. The content ends up as an arg, but we can still get it.
		if (this->m_webServer.args() == 1 && this->m_webServer.argName(0).startsWith("<?xml"))
		{
			// Need to get BinaryState out of this SOAP response
			/*
			"1.0" encoding="utf-8"?><s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><u:SetBinaryState xmlns:u="urn:Belkin:service:basicevent:1"><BinaryState>0</BinaryState></u:SetBinaryState></s:Body></s:Envelope>
			*/
			const char* input = this->m_webServer.arg(0).c_str();
			const char* pattern = "<BinaryState>";
			int inputLen = strlen(input);
			int patternLen = strlen(pattern);
			int index = -1;
			for (uint16 i = 0; i < inputLen; i++)
			{
				bool allMatch = true;
				for (uint8 j = 0; j < patternLen; j++)
				{
					if (input[i + j] != pattern[j])
					{
						allMatch = false;
						break;
					}
				}
				if (allMatch)
				{
					index = i + patternLen;
					break;
				}
			}
			if (index > -1)
			{
				char state = input[index];
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
				CONSOLE.printf("State: %c\n", state);
#endif
				if (state == '0')
				{
					digitalWrite(m_pin, LOW);
				}
				else if (state == '1')
				{
					digitalWrite(m_pin, HIGH);
				}
			}
		}

		this->m_webServer.send(200, "text/xml", "");
	});

	m_webServer.begin();


#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	IPAddress ip = WiFi.localIP();
	CONSOLE.printf("%s ready at %u:%u:%u:%u on port %u\n",
		m_friendlyName.c_str(), ip[0], ip[1], ip[2], ip[3], m_port);
#endif
}

bool WeMoEmulator::respondToSearch()
{
	for (LinkedListItem<MSearchInfo*>* i = m_searchRequests.getFirst(); i; i = i->getNext())
	{
		MSearchInfo *msearch = i->getItem();
		if (msearch->ProcessTime < millis())
		{
			respondToSearch(msearch);
			m_searchRequests.remove(msearch);
			delete msearch;
			return true;
		}
	}
	return false;
}

void WeMoEmulator::loop()
{
	m_webServer.handleClient();
}

void WeMoEmulator::sendNotify(NotifyType nt, WiFiUDP* session)
{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	CONSOLE.printf("WeMoEmulator::sendNotify called for %s\n", m_friendlyName.c_str());
#endif

	const char* notifyType;
	switch (nt)
	{
	default:
	case NT_ALIVE:
		notifyType = "alive";
		break;
	case NT_BYEBYE:
		notifyType = "byebye";
		break;
	case NT_UPDATE:
		notifyType = "update";
		break;
	}

	IPAddress ip = WiFi.localIP();

	char buffer[512];
	snprintf(buffer, 512, _notifyTemplate,
		ip[0], ip[1], ip[2], ip[3],
		getPort(),
		getNLS().c_str(),
		notifyType,
		m_serial);

	session->write(buffer);
}

void WeMoEmulator::respondToSearch(MSearchInfo* msearch)
{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	CONSOLE.printf("WeMoEmulator::respondToSearch start ( %s )\n", m_friendlyName.c_str());
#endif
	WiFiUDP unicast;
	bool result;

	result = unicast.beginPacket(msearch->RemoteIP, msearch->RemotePort);
	if (!result)
	{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
		CONSOLE.printf("WeMoEmulator::respondToSearch WiFIUDP::beginPacket failed for %s\n", m_friendlyName.c_str());
#endif
		goto onError;
	}

	unicast.write("HTTP/1.1 200 OK\r\n");
	unicast.write("CACHE-CONTROL: max-age=86400\r\n");
	unicast.printf("DATE: %s\r\n", getDateTime()->toString().c_str());
	unicast.write("EXT: \r\n");
	unicast.write("LOCATION: http://");
	WiFi.localIP().printTo(unicast);
	unicast.printf(":%u/setup.xml\r\n", getPort());
	unicast.write("OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n");
	unicast.printf("01-NLS: %s\r\n", getNLS().c_str());
	unicast.write("SERVER: Unspecified, UPnP/1.0, Unspecified\r\n");
	unicast.write("X-User-Agent: redsonic\r\n");
	unicast.printf("ST: %s\r\n", msearch->SearchTarget.c_str());
	unicast.printf("USN: uuid:Socket-1_0-%u::urn:Belkin:device:**\r\n", m_serial);
	unicast.write("\r\n");
	result = unicast.endPacket();
	if (!result)
	{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
		CONSOLE.printf("WeMoEmulator::respondToSearch WiFIUDP::endPacket failed for %s\n", m_friendlyName.c_str());
#endif
		goto onError;
	}

onError:
	unicast.stop();

#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	CONSOLE.printf("WeMoEmulator::respondToSearch finish ( %s )\n", m_friendlyName.c_str());
#endif
}

bool WeMoEmulator::matchesSearch(String searchTerm)
{
	return 
		searchTerm.equalsIgnoreCase("urn:Belkin:device:**") || 
		searchTerm.equalsIgnoreCase("ssdp:all") || 
		searchTerm.equalsIgnoreCase("upnp:rootdevice");
}

String WeMoEmulator::getSetupXml()
{
	char buffer[512];
	snprintf(buffer, 512, _setupTemplate, m_friendlyName.c_str(), m_serial);
	return String(buffer);
}

String WeMoEmulator::getNLS()
{
	uint32 flashID = ESP.getFlashChipId();
	uint32 epoch = getDateTime()->getEpoch();

	char buffer[64];
	snprintf(buffer, 64, "%06x-E512-8266-%04x-%012x",
		m_serial & 0xFFFFFF,
		flashID >> 16 & 0xFFFF,
		epoch);
	return String(buffer);
}

void WeMoEmulator::addSearchRequest(MSearchInfo* msearch)
{
	m_searchRequests.add(msearch);
}