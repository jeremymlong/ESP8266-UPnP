#include "UpnpDevice.h"

UpnpDevice::UpnpDevice(String friendlyName, uint16 port) :
	m_friendlyName(friendlyName),
	m_serial(0),
	m_port(port),
	m_webServer(port)
{
	m_serial = ESP.getChipId();
	uint8 serial[4] = { 0 };
	serial[0] = m_serial >> 24 & 0xFF;
	serial[1] = m_serial >> 16 & 0xFF;
	serial[2] = m_serial >> 8 & 0xFF;
	serial[3] = m_serial  & 0xFF;

	for (int i = 0; i < m_friendlyName.length(); i++)
	{
		serial[i % 4] ^= m_friendlyName[i];
	}
	m_serial = serial[0] << 24 | serial[1] << 16 | serial[2] << 8 | serial[3];
}

void UpnpDevice::loop()
{
	m_webServer.handleClient();
}

bool UpnpDevice::matchesSearch(String searchTerm)
{
	return searchTerm.equalsIgnoreCase("ssdp:all") ||
		searchTerm.equalsIgnoreCase("upnp:rootdevice");
}

bool UpnpDevice::respondToSearch()
{
	for (LinkedListItem<MSearchInfo*>* i = m_searchRequests.getFirst(); i; i = i->getNext())
	{
		MSearchInfo *msearch = i->getItem();
		if (msearch->ProcessTime < millis())
		{
			respondToSearch(msearch);
			m_searchRequests.remove(msearch);
			delete msearch;
			return true;
		}
	}
	return false;
}

void UpnpDevice::addSearchRequest(MSearchInfo *msearch)
{
	m_searchRequests.add(msearch);
}

void UpnpDevice::begin()
{
	setup();
	m_webServer.begin();
}