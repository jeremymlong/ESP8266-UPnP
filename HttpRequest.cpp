#include "HttpRequest.h"

HttpRequest::HttpRequest() : 
	HttpMessage::HttpMessage()
{
}

HttpRequest::HttpRequest(IPAddress address, uint16 port, String url) :
	HttpMessage::HttpMessage(),
	RemoteIP(address),
	RemotePort(port),
	Url(url)
{
	RequestLine = "GET " + url + "HTTP/1.1";
	setHeader("Host", RemoteIP.toString());
	setHeader("Connection", "close");
}

void HttpRequest::printTo(Stream *stream)
{
	stream->printf("%s\r\n", RequestLine.c_str());
	printHeaders(stream);
}
