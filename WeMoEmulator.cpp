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

WeMoEmulator::WeMoEmulator(String friendlyName, uint8 pin, uint16 port) :
	UpnpDevice(friendlyName, port),
	m_pin(pin)
{
	pinMode(m_pin, OUTPUT);
}

void WeMoEmulator::setup()
{
	m_webServer.on("/setup.xml", HTTP_GET, [this]() 
	{
		this->m_webServer.send(200, "text/xml", this->getDescriptionXml());
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

#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	CONSOLE.printf("%s ready at %s on port %u\n",
		m_friendlyName.c_str(), WiFi.localIP().toString().c_str(), m_port);
#endif
}

void WeMoEmulator::sendNotify(NotifyType nt, WiFiUDP* session)
{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	CONSOLE.printf("WeMoEmulator::sendNotify called for %s\n", m_friendlyName.c_str());
#endif
	HttpRequest request;
	request.RequestLine = "NOTIFY * HTTP/1.1";
	request.setHeader("HOST", SSDP_ADDR.toString() + ":" + String(SSDP_PORT));
	request.setHeader("CACHE_CONTROL", "max-age=86400");
	request.setHeader("LOCATION", String("http://") + WiFi.localIP().toString() + ":" + String(m_port) + "/setup.xml");
	request.setHeader("OPT", "\"http://schemas.upnp.org/upnp/1/0/\"; ns=01");
	request.setHeader("01-NLS", getNLS());
	request.setHeader("NT", "urn:Belkin:service:manufacture:1");
	request.setHeader("NTS", "ssdp:alive");
	request.setHeader("SERVER", "Unspecified, UPnP/1.0, Unspecified");
	request.setHeader("X-User-Agent", "redsonic");
	request.setHeader("USN", "uuid:Socket-1_0-" + String(m_serial) + "::urn:Belkin:service:manufacture:1");

	request.printTo(session);
}

void WeMoEmulator::respondToSearch(MSearchInfo* msearch)
{
	HttpResponse response;
	response.ResponseCode = 200;
	response.setHeader("CACHE-CONTROL", "max-age=86400");
	response.setHeader("DATE", m_dateTime->toString());
	response.setHeader("EXT", "");
	response.setHeader("LOCATION", String("http://") + WiFi.localIP().toString() + ":" + String(m_port) + "/setup.xml");
	response.setHeader("OPT", "\"http://schemas.upnp.org/upnp/1/0/\"; ns=01");
	response.setHeader("01-NLS", getNLS());
	response.setHeader("SERVER", "Unspecified, UPnP/1.0, Unspecified");
	response.setHeader("X-User-Agent", "redsonic");
	response.setHeader("ST", msearch->SearchTarget);
	response.setHeader("USN", String("uuid:Socket-1_0-") + m_serial + "::urn:Belkin:device:**");

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
		goto done;
	}

	response.printTo(&unicast);

	result = unicast.endPacket();
	if (!result)
	{
#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
		CONSOLE.printf("WeMoEmulator::respondToSearch WiFIUDP::endPacket failed for %s\n", m_friendlyName.c_str());
#endif
		goto done;
	}

done:
	unicast.stop();

#if defined(CONSOLE) && defined(DEBUG_WEMOEMULATOR)
	CONSOLE.printf("WeMoEmulator::respondToSearch finish ( %s )\n", m_friendlyName.c_str());
#endif
}

bool WeMoEmulator::matchesSearch(String searchTerm)
{
	return UpnpDevice::matchesSearch(searchTerm) || searchTerm.equalsIgnoreCase("urn:Belkin:device:**");
}

String WeMoEmulator::getDescriptionXml()
{
	char buffer[512];
	snprintf(buffer, 512, _setupTemplate, m_friendlyName.c_str(), m_serial);
	return String(buffer);
}

String WeMoEmulator::getNLS()
{
	uint32 flashID = ESP.getFlashChipId();
	uint32 epoch = m_dateTime->getEpoch();

	char buffer[64];
	snprintf(buffer, 64, "%06x-E512-8266-%04x-%012x",
		m_serial & 0xFFFFFF,
		flashID >> 16 & 0xFFFF,
		epoch);
	return String(buffer);
}
