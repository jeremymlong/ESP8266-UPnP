#include <Timer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "Ssdp.h"
#include "WeMoEmulator.h"

#define TREE_PIN 2
#define GARL_PIN 12

DateTime *dateTime;
Ssdp *ssdp;

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.println("Serial Ready");

	WiFi.mode(WIFI_STA);
	
	Serial.println("Connecting to WiFi");

	if (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		Serial.println("WiFi failed to connect.");
		delay(1000);
		Serial.println("Double check SSID and password.  Restarting...");
		delay(30000);
		ESP.restart();
	}

	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());

	dateTime = new DateTime();
	dateTime->setUpdateCallback([]()
	{
		HttpClient client;
		HttpResponse *response = client.openUrl(WiFi.gatewayIP(), 80, "/");
		String result = "";

		if (response->hasHeader("Date"))
		{
			HttpHeader* dateHeader = response->getHeader("Date");
			result = dateHeader->Value;
		}

		delete response;
		return result;
	});

	dateTime->update();
	dateTime->begin();

	ssdp = new Ssdp(WiFi.localIP());
	ssdp->setDateTime(dateTime);
	ssdp->setUdpSendCount(1);
	ssdp->addDevice(new WeMoEmulator("Christmas Tree", TREE_PIN, 8080U));
	ssdp->addDevice(new WeMoEmulator("Garland", GARL_PIN, 8081U));
	ssdp->begin();

	Serial.println("setup complete");
}

void loop()
{
	dateTime->loop();
	ssdp->loop();
	delay(1);
}

