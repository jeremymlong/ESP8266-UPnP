#ifndef HUELIGHT_H_
#define HUELIGHT_H_
#include <Arduino.h>
#include <aJSON.h>

typedef void(*StateHandler)(bool);
typedef void(*BrightnessHandler)(uint8);

class HueLight
{
public:
	HueLight(uint8 id, String friendlyName);
	~HueLight();

	aJsonObject* getJSON();
	void setState(bool state);
	void setBrightness(uint8 bri);

	inline uint8 getID() { return m_id; }
	inline bool getState() { return m_state; }
	inline uint8 getBrightness() { return m_brightness; }
	inline String getFriendlyName() { return m_friendlyName; }
	inline void setStateChangedHandler(StateHandler handler) { onStateChanged = handler; }
	inline void setBrightnessChangedHandler(BrightnessHandler handler) { onBrightnessChanged = handler; }

private:
	bool m_state;
	uint8 m_id;
	uint8 m_brightness;
	String m_friendlyName;
	StateHandler onStateChanged;
	BrightnessHandler onBrightnessChanged;
};

#endif
