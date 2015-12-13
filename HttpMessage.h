#ifndef HTTPMESSAGE_H_
#define HTTPMESSAGE_H_

#include <Arduino.h>

#include "LinkedList.h"
#include "HttpHeader.h"

class HttpMessage
{
public:
	HttpMessage();
	virtual ~HttpMessage();
	HttpHeader * getHeader(String name);
	bool hasHeader(String name);
	void setHeader(String name, String value);

protected:
	LinkedList<HttpHeader*> m_headers;
	void printHeaders(Stream *stream);
};

#endif
