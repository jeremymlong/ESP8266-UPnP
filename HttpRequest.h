#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <IPAddress.h>
#include "HttpMessage.h"

class HttpRequest :
	public HttpMessage
{
public:
	HttpRequest();
	String RequestLine;
	IPAddress RemoteIP;
	uint16 RemotePort;
};

#endif
