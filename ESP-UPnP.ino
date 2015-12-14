#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <SPI.h>
#include <APA.h>

#include <aJSON.h>
#if PRINT_BUFFER_LEN < 2048
#error "Increase PRINT_BUFFER_LEN in aJSON.h"
#endif

#include "Ssdp.h"
#include "WeMoEmulator.h"
#include "HueBridge.h"

#define TREE_PIN 2
#define GARL_PIN 12
#define LED_COUNT 72

DateTime *dateTime;
Ssdp *ssdp;
APA apa(LED_COUNT);

bool state;
uint8 h = 255, s = 255, l = 255;

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
		HttpRequest request(WiFi.gatewayIP(), 80, "/");
		HttpResponse response;
		HttpClient().openUrl(&request, &response);

		String result;
		if (response.hasHeader("Date"))
		{
			HttpHeader* dateHeader = response.getHeader("Date");
			result = dateHeader->Value;
			Serial.printf("Date: %s\n", result.c_str());
		}
		return result;
	});

	dateTime->update();
	dateTime->begin();
	randomSeed(dateTime->getEpoch());

	ssdp = new Ssdp(WiFi.localIP());
	ssdp->setDateTime(dateTime);
	ssdp->setUdpSendCount(1);
	ssdp->addDevice(new WeMoEmulator("Christmas Tree", TREE_PIN, 8080U));
	ssdp->addDevice(new WeMoEmulator("Garland", GARL_PIN, 8081U));
	
	HueBridge *bridge = new HueBridge(8082);
	HueLight *deskLamp = new HueLight(1, "Desk Lamp");
	deskLamp->setBrightnessChangedHandler([](uint8 b) 
	{
		Serial.printf("Set desk lamp brightness to: %u\n", b);
		l = b;
		updateDeskLamp();
	});
	deskLamp->setStateChangedHandler([](bool s) 
	{
		Serial.printf("Set desk lamp state to: %s\n", s ? "on" : "off");
		state = s;
		updateDeskLamp();
	});

	HueLight *deskLampColor = new HueLight(2, "Lamp Color");
	deskLampColor->setStateChangedHandler([](bool value)
	{
		Serial.printf("Set lamp color to: %s\n", value ? "on" : "off");
		s = value ? 255 : 0;
		updateDeskLamp();
	});
	deskLampColor->setBrightnessChangedHandler([](uint8 b)
	{
		Serial.printf("Set lamp color to: %u\n", b);
		h = b;
		updateDeskLamp();
	});

	bridge->addLight(deskLamp);
	bridge->addLight(deskLampColor);
	ssdp->addDevice(bridge);
	ssdp->begin();
	
	apa.setup();
	apa.setMaxBrightness(31U);
	updateDeskLamp();

	Serial.println("setup complete");
}

void setHslColor(uint32 h, uint32 s, uint32 l)
{
	uint32 r, g, b;
	HSL_to_RGB(h, s, l, &r, &g, &b);
	setRgbColor(r, g, b);
}

void setRgbColor(uint8 r, uint8 g, uint8 b)
{
	apa.beginFrame();
	for (int i = 0; i < LED_COUNT; i++)
	{
		apa.setPixelColor(r, g, b);
	}
	apa.endFrame();
}

void updateDeskLamp()
{
	if (state)
	{
		setHslColor(h, s, l);
	}
	else 
	{
		setRgbColor(0, 0, 0);
	}
	Serial.printf("Free Heap: %u\n", ESP.getFreeHeap());
}

void loop()
{
	dateTime->loop();
	ssdp->loop();
	delay(1);
}


void HSL_to_RGB(uint32_t hue, uint32_t sat, uint32_t lum, uint32_t* r, uint32_t* g, uint32_t* b)
{
	uint32_t v;

	v = (lum < 128) ? (lum * (255 + sat)) >> 8 :
		(((lum + sat) << 8) - lum * sat) >> 8;
	if (v <= 0) {
		*r = *g = *b = 0;
	}
	else {
		uint32_t m;
		uint32_t sextant;
		uint32_t fract, vsf, mid1, mid2;

		m = lum + lum - v;
		hue *= 6;
		sextant = hue >> 8;
		fract = hue - (sextant << 8);
		vsf = v * fract * (v - m) / v >> 8;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant) {
		case 0: *r = v; *g = mid1; *b = m; break;
		case 1: *r = mid2; *g = v; *b = m; break;
		case 2: *r = m; *g = v; *b = mid1; break;
		case 3: *r = m; *g = mid2; *b = v; break;
		case 4: *r = mid1; *g = m; *b = v; break;
		case 5: *r = v; *g = m; *b = mid2; break;
		}
	}
}
