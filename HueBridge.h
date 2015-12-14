#ifndef HUEBRIDGE_H_
#define HUEBRIDGE_H_

#include <aJSON.h>

#include "UpnpDevice.h"
#include "HttpRequest.h"
#include "LinkedList.h"
#include "HueLight.h"

class HueBridge : public UpnpDevice
{
public:
	HueBridge(uint16 port);
	~HueBridge();

	String getDescriptionXml();
	void sendNotify(NotifyType nt, WiFiUDP* session);
	String getFriendlyName();
	bool matchesSearch(String searchTerm);
	void addLight(HueLight *light) { m_lights.add(light); }
	
protected:
	void setup();
	void respondToSearch(MSearchInfo* msearch);

private:
	String getUUID();
	HueLight *GetLightByID(uint8 id);
	LinkedList<HueLight*> m_lights;
};

#endif
