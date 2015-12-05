#ifndef HTTPHEADER_H_
#define HTTPHEADER_H_
#include <Arduino.h>

struct HttpHeader
{
public:
	HttpHeader() { }

	String Name;
	String Value;
};

#endif
