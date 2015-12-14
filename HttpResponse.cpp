#include "HttpResponse.h"

HttpResponse::HttpResponse() :
	HttpMessage::HttpMessage()
{
}

void HttpResponse::printTo(Stream *stream)
{
	stream->printf("HTTP/1.1 %u %s\r\n", ResponseCode, getResponseCodeString());
	printHeaders(stream);
}

const char* HttpResponse::getResponseCodeString()
{
	switch (ResponseCode)
	{
	case 200: return "OK";
	default: return "";
	}
}
