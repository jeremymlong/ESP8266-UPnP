#include "HttpClient.h"

HttpClient::HttpClient() :
	WiFiClient()
{
}

HttpClient::~HttpClient()
{
}

HttpResponse* HttpClient::openUrl(IPAddress address, uint16 port, String url)
{
#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
	CONSOLE.println("Opening connection");
#endif
	if (!connect(address, port))
	{
#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
		CONSOLE.println("connection failed");
#endif
		return NULL;
	}

	HttpResponse* response = new HttpResponse();

	// This will send the request to the server
	print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + address + "\r\n" +
		"Connection: close\r\n\r\n");

	bool parsingHeaders = true;
	// Read all the lines of the reply from server and print them to Serial
	while (connected())
	{
		if (available())
		{
			String line = readStringUntil('\r');
			line.trim();
			if (line.equals(""))
			{
				// End of HTTP header
				parsingHeaders = false;
				continue;
			}

			if (parsingHeaders)
			{
				parseHeaderLine(response, &line);
			}
		}
	}
#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
	CONSOLE.println("Connection closed");
#endif
	return response;
}

HttpResponse* HttpClient::openUrl(String host, uint16_t port, String url)
{
	IPAddress remote_addr;
	if (WiFi.hostByName(host.c_str(), remote_addr))
	{
		return openUrl(remote_addr, port, url);
	}
	return NULL;
}

void HttpClient::parseHeaderLine(HttpResponse* response, String* line)
{
	if (line->startsWith("HTTP"))
	{
		int spaceIndex = line->indexOf(' ');
		String codeString = line->substring(line->indexOf(' '), line->lastIndexOf(' '));
		codeString.trim();
		response->ResponseCode = codeString.toInt();

#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
		CONSOLE.print("Response Code: ");
		CONSOLE.println(response->ResponseCode);
#endif
	}
	else 
	{
		int colonIndex = line->indexOf(':');
		if (colonIndex > -1)
		{
			String name = line->substring(0, colonIndex);
			String value = line->substring(colonIndex + 1);
			name.trim();
			value.trim();
#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
			CONSOLE.print(name);
			CONSOLE.print(":");
			CONSOLE.println(value);
#endif
			response->setHeader(name, value);
		}
	}
}