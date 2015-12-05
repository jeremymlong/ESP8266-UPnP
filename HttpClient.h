#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include "HttpResponse.h"

class HttpClient : public WiFiClient
{
public:
	HttpClient();
	~HttpClient();

	HttpResponse* openUrl(IPAddress address, uint16 port, String url);
	HttpResponse* openUrl(String host, uint16_t port, String url);

private:

	void parseHeaderLine(HttpResponse* response, String* line);
};

#endif
