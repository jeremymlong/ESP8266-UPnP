#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <IPAddress.h>
#include "HttpMessage.h"

class HttpRequest :
	public HttpMessage
{
public:
	HttpRequest();
	HttpRequest(IPAddress address, uint16 port, String url);
	String RequestLine;
	IPAddress RemoteIP;
	uint16 RemotePort;
	String Url;

	void printTo(Stream* stream);
};

#endif
