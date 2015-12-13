#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <Stream.h>

#include "HttpMessage.h"

class HttpResponse : 
	public HttpMessage
{
public:
	HttpResponse();
	void printTo(Stream* stream);

	uint16 ResponseCode;

private:
	const char* getResponseCodeString();
};

#endif
