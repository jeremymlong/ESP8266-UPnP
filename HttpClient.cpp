#include "HttpClient.h"

HttpClient::HttpClient() :
	WiFiClient()
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::openUrl(HttpRequest *request, HttpResponse *response)
{
#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
	CONSOLE.println("Opening connection");
#endif
	if (!connect(request->RemoteIP, request->RemotePort))
	{
#if defined(CONSOLE) && defined(DEBUG_HTTPCLIENT)
		CONSOLE.println("connection failed");
#endif
		return;
	}

	request->printTo(this);

	bool parsingHeaders = true;

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
	return;
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
