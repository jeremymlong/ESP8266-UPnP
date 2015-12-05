#include "HttpMessage.h"

//#define DEBUG_HTTPMESSAGE

HttpMessage::HttpMessage() :
	m_headers()
{
}

HttpMessage::~HttpMessage()
{
	LinkedListItem<HttpHeader*>* item = NULL;
	while (item = m_headers.getFirst())
	{
		HttpHeader* header = m_headers.remove(item->getItem());
		delete header;
	}
}

bool HttpMessage::hasHeader(String name)
{
	for (LinkedListItem<HttpHeader*>* i = m_headers.getFirst(); i; i = i->getNext())
	{
#if defined(CONSOLE) && defined(DEBUG_HTTPMESSAGE)
		CONSOLE.print("Searching Header: ");
		CONSOLE.println(i->getItem()->Name);
#endif
		if (i->getItem()->Name.equalsIgnoreCase(name))
		{
			return true;
		}
	}
	return false;
}

HttpHeader* HttpMessage::getHeader(String name)
{
	for (LinkedListItem<HttpHeader*>* i = m_headers.getFirst(); i; i = i->getNext())
	{
		if (i->getItem()->Name == name)
		{
			return i->getItem();
		}
	}
	return NULL;
}

void HttpMessage::setHeader(String name, String value)
{
	HttpHeader * header;
	if (hasHeader(name))
	{
		header = getHeader(name);
#if defined(CONSOLE) && defined(DEBUG_HTTPMESSAGE)
		CONSOLE.print("Found existing header: ");
		CONSOLE.println(header->Name);
#endif
	}
	else
	{
		header = new HttpHeader();
		header->Name = name;
		header->Value = value;
		m_headers.add(header);
	}
}