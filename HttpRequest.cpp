#include "HttpRequest.h"

HttpRequest::HttpRequest() : 
	HttpMessage::HttpMessage()
{
}

void HttpRequest::printTo(Stream *stream)
{
	stream->printf("%s\r\n", RequestLine.c_str());
	printHeaders(stream);
}