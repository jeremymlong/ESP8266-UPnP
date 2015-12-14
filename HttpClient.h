#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include "HttpResponse.h"
#include "HttpRequest.h"

class HttpClient : public WiFiClient
{
public:
	HttpClient();
	~HttpClient();

	void openUrl(HttpRequest *request, HttpResponse *response);

private:

	void parseHeaderLine(HttpResponse* response, String* line);
};

#endif
