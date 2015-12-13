#include "HueBridge.h"

/*
SSDP.begin();
SSDP.setSchemaURL((char*)"description.xml");
SSDP.setHTTPPort(80);
SSDP.setName((char*)"Philips hue clone");
SSDP.setSerialNumber((char*)"001788102201");
SSDP.setURL((char*)"index.html");
SSDP.setModelName((char*)"Philips hue bridge 2012");
SSDP.setModelNumber((char*)"929000226503");
SSDP.setModelURL((char*)"http://www.meethue.com");
SSDP.setManufacturer((char*)"Royal Philips Electronics");
SSDP.setManufacturerURL((char*)"http://www.philips.com");
*/

#define DEBUG_HUEBRIDGE

const char* setupTemplate =
"<root>"
"<specVersion><major>1</major><minor>0</minor></specVersion>"
"<URLBase>http://%s:%u/</URLBase>" //ip, port
"<device>"
"<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
"<friendlyName>Philips Hue Bridge</friendlyName>" // ip
"<manufacturer>Royal Philips Electronics</manufacturer>"
"<manufacturerURL>http://www.philips.com</manufacturerURL>"
"<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
"<modelName>Philips hue bridge 2012</modelName>"
"<modelNumber>929000226503</modelNumber>"
"<modelURL>http://www.meethue.com</modelURL>"
"<serialNumber>%012x</serialNumber>" // serial
"<UDN>uuid:%s</UDN>" // uuid
//"<presentationURL>index.html</presentationURL>"
//"<iconList><icon><mimetype>image/png</mimetype><height>48</height><width>48</width><depth>24</depth><url>hue_logo_0.png</url></icon><icon><mimetype>image/png</mimetype><height>120</height><width>120</width><depth>24</depth><url>hue_logo_3.png</url></icon></iconList>"
"</device>"
"</root>"
"\r\n\r\n";

HueBridge::HueBridge(uint16 port) :
	UpnpDevice("Philips Hue Bridge", port)
{
}

HueBridge::~HueBridge()
{
}

void HueBridge::setup()
{
	m_webServer.on("/description.xml", HTTP_GET, [this]()
	{
		this->m_webServer.send(200, "text/plain", this->getDescriptionXml());
	});
	m_webServer.onNotFound([this]()
	{
		auto webServer = &this->m_webServer;
		String uri = webServer->uri();

		if (uri.startsWith("/api/"))
		{
			// Split the uri into parts
			// The api uri has 5 or so parts to it.
			/* /api/[CLIENT_ID]/[API_NAME]/[DEVICE_ID]/[ACTION_NAME] */
			String clientID, apiName, deviceID, actionName;
			String* parts[] = { &clientID, &apiName, &deviceID, &actionName };

			// strip /api/
			uri = uri.substring(5, uri.length());

			// parse the rest
			for (int i = 0; i < 4; i++)
			{
#if defined(CONSOLE) && defined(DEBUG_HUEBRIDGE)
				CONSOLE.printf("HueBridge::parsingURI: %s\n", uri.c_str());
#endif
				int index = uri.indexOf('/');
				if (index > -1)
				{
					*parts[i] = uri.substring(0, index);
					uri = uri.substring(index + 1, uri.length());
				}
				else
				{
					*parts[i] = uri;
					break;
				}
			}

#if defined(CONSOLE) && defined(DEBUG_HUEBRIDGE)
			CONSOLE.println("HueBridge api call:");
			CONSOLE.printf("\tClient ID:   %s\n", clientID.c_str());
			CONSOLE.printf("\tAPI Name:    %s\n", apiName.c_str());
			CONSOLE.printf("\tDevice ID:   %s\n", deviceID.c_str());
			CONSOLE.printf("\tAction Name: %s\n", actionName.c_str());
#endif

			if (apiName.equalsIgnoreCase("lights"))
			{
				if (deviceID.length() > 0)
				{
					HueLight *light = GetLightByID(deviceID.toInt());
					if (light != NULL)
					{
						if (actionName.equalsIgnoreCase("state"))
						{
							if (webServer->hasArg("plain"))
							{
								// Handle state query for light
								// [ {"success":{"/lights/1/state/bri":200}}, {"success":{"/lights/1/state/on":true}}, ]

								aJsonObject *plainArg = aJson.parse((char*)webServer->arg("plain").c_str());
								aJsonObject *response = aJson.createArray();

								aJsonObject *onState = aJson.getObjectItem(plainArg, "on");
								if (onState != NULL)
								{
									light->setState(onState->valuebool);

									//{"success":{"/lights/1/state/on":true}}
									aJsonObject *onContent = aJson.createObject();
									aJsonObject *success = aJson.createObject();
									String name = "/lights/" + deviceID + "/state/on";
									aJson.addBooleanToObject(success, name.c_str(), light->getState());
									aJson.addItemToObject(onContent, "success", success);
									aJson.addItemToArray(response, onContent);
								}

								aJsonObject *bri = aJson.getObjectItem(plainArg, "bri");
								if (bri != NULL)
								{
									light->setBrightness(bri->valueint);

									//{"success":{"/lights/1/state/bri":200}}
									aJsonObject *briContent = aJson.createObject();
									aJsonObject *success = aJson.createObject();
									String name = "/lights/" + deviceID + "/state/bri";
									aJson.addNumberToObject(success, name.c_str(), light->getBrightness());
									aJson.addItemToObject(briContent, "success", success);
									aJson.addItemToArray(response, briContent);
								}
								webServer->send(200, "application/json", aJson.print(response));
							}
							else
							{
								webServer->send(400);
							}
							return;
						}
						// Return info for light
						aJsonObject *item = light->getJSON();
						webServer->send(200, "application/json", aJson.print(item));
						aJson.deleteItem(item);
						return;
					}
					else
					{
						webServer->send(400);
					}
				}

				// Return info for all lights
				aJsonObject *response = aJson.createObject();
				for (auto i = this->m_lights.getFirst(); i; i = i->getNext())
				{
					HueLight *light = i->getItem();
					aJsonObject *item = light->getJSON();
					aJson.addItemToObject(response, String(light->getID()).c_str(), item);
				}
				webServer->send(200, "application/json", aJson.print(response));
				aJson.deleteItem(response);
				return;
			}
		}

		webServer->send(404);
	});
}

HueLight *HueBridge::GetLightByID(uint8 id)
{
	for (auto i = m_lights.getFirst(); i; i = i->getNext())
	{
		auto light = i->getItem();
		if (light->getID() == id)
		{
			return light;
		}
	}
	return NULL;
}

String HueBridge::getDescriptionXml()
{
	String ip = WiFi.localIP().toString();
	char buffer[1536];
	snprintf(buffer, 1536, setupTemplate,
		ip.c_str(), m_port,
		ip.c_str(),
		m_serial,
		getUUID().c_str()
	);
	return String(buffer);
}

void HueBridge::sendNotify(NotifyType nt, WiFiUDP* session)
{
	HttpRequest request;
	request.RequestLine = "NOTIFY * HTTP/1.1";
	request.setHeader("HOST", SSDP_ADDR.toString() + ":" + String(SSDP_PORT));
	request.setHeader("CACHE_CONTROL", "max-age=86400");
	request.setHeader("LOCATION", String("http://") + WiFi.localIP().toString() + ":" + String(m_port) + "/description.xml");
	request.setHeader("NTS", "ssdp:alive");
	request.setHeader("SERVER", "FreeRTOS/6.0.5, UPnP/1.0, IpBridge/0.1");
	request.setHeader("USN", String("uuid:") + getUUID() + "::upnp:rootdevice");
	request.printTo(session);
}

void HueBridge::respondToSearch(MSearchInfo* msearch)
{
	HttpResponse response;
	response.ResponseCode = 200;
	response.setHeader("CACHE-CONTROL", "max-age=86400");
	//response.setHeader("DATE", m_dateTime->toString());
	response.setHeader("EXT", "");
	response.setHeader("LOCATION", String("http://") + WiFi.localIP().toString() + ":" + String(m_port) + "/description.xml");
	response.setHeader("SERVER", "FreeRTOS/6.0.5, UPnP/1.0, IpBridge/0.1");
	response.setHeader("ST", msearch->SearchTarget);
	response.setHeader("USN", String("uuid:Socket-1_0-") + getUUID() + "::urn:schemas-upnp-org:device:basic:1");

#if defined(CONSOLE) && defined(DEBUG_HUEBRIDGE)
	CONSOLE.printf("HueBridge::respondToSearch start ( %s )\n", m_friendlyName.c_str());
#endif

	WiFiUDP unicast;
	bool result;

	result = unicast.beginPacket(msearch->RemoteIP, msearch->RemotePort);
	if (!result)
	{
#if defined(CONSOLE) && defined(DEBUG_HUEBRIDGE)
		CONSOLE.printf("HueBridge::respondToSearch WiFIUDP::beginPacket failed for %s\n", m_friendlyName.c_str());
#endif
		goto done;
	}

	response.printTo(&unicast);

	result = unicast.endPacket();
	if (!result)
	{
#if defined(CONSOLE) && defined(DEBUG_HUEBRIDGE)
		CONSOLE.printf("HueBridge::respondToSearch WiFIUDP::endPacket failed for %s\n", m_friendlyName.c_str());
#endif
		goto done;
	}

done:
	unicast.stop();

#if defined(CONSOLE) && defined(DEBUG_HUEBRIDGE)
	CONSOLE.printf("HueBridge::respondToSearch finish ( %s )\n", m_friendlyName.c_str());
#endif
}

bool HueBridge::matchesSearch(String searchTerm)
{
	return UpnpDevice::matchesSearch(searchTerm) || searchTerm.equalsIgnoreCase("urn:schemas-upnp-org:device:basic:1");
}

String HueBridge::getUUID()
{
	uint32 flashID = ESP.getFlashChipId();
	char buffer[64];
	snprintf(buffer, 64, "%06x-E512-8266-%04x-%012x",
		m_serial & 0xFFFFFF,
		flashID >> 16 & 0xFFFF,
		m_serial
	);
	return String(buffer);
}