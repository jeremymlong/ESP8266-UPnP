#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include "HttpMessage.h"

class HttpResponse : 
	public HttpMessage
{
public:
	HttpResponse();
	uint16 ResponseCode;
};

#endif
