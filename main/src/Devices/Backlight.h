#ifndef BIT_FIRMWARE_BACKLIGHTBRIGHTNESS_H
#define BIT_FIRMWARE_BACKLIGHTBRIGHTNESS_H

#include "Periph/PWM.h"
#include "Services/Settings.h"

class Backlight {
public:
	explicit Backlight(ledc_channel_t channel);
	void setBrightness(uint8_t level); //0 - 100%

	void fadeIn();
	void fadeOut();

	bool isOn();

private:
	PWM pwm;

	bool state = false;

	static constexpr uint8_t mapDuty(uint8_t level);
	static constexpr uint8_t FadeDelay = 3;
	static constexpr uint8_t MinDuty = 10;

};


#endif //BIT_FIRMWARE_BACKLIGHTBRIGHTNESS_H
