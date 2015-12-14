#include "HueLight.h"

HueLight::HueLight(uint8 id, String friendlyName) :
	m_state(false),
	m_id(id),
	m_brightness(128),
	m_friendlyName(friendlyName),
	onStateChanged([](bool) {}),
	onBrightnessChanged([](uint8) {})
{
}

HueLight::~HueLight()
{
}

void HueLight::setState(bool state)
{
	if (m_state != state)
	{
		m_state = state;
		onStateChanged(m_state);
	}
}

void HueLight::setBrightness(uint8 brightness)
{
	if (m_brightness != brightness)
	{
		m_brightness = brightness;
		onBrightnessChanged(m_brightness);
	}
}

aJsonObject *HueLight::getJSON()
{
	aJsonObject *item = aJson.createObject();
	aJsonObject *state = aJson.createObject();
	aJson.addBooleanToObject(state, "on", m_state);
	aJson.addNumberToObject(state, "bri", m_brightness);
	aJson.addStringToObject(state, "alert", "none");
	aJson.addStringToObject(state, "effect", "none");
	aJson.addBooleanToObject(state, "reachable", true);
	aJson.addItemToObject(item, "state", state);
	aJson.addStringToObject(item, "type", "Dimmable light");
	aJson.addStringToObject(item, "name", m_friendlyName.c_str());
	aJson.addStringToObject(item, "modelid", "LWB004");
	aJson.addStringToObject(item, "swversion", "66012040");
	return item;
}
