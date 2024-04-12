#ifndef PERSE_MISSIONCTRL_JOYSTICK_H
#define PERSE_MISSIONCTRL_JOYSTICK_H

#include "Periph/ADC.h"
#include "Services/Settings.h"
#include "Services/ADCReader.h"
#include "Util/Threaded.h"
#include <vec2.hpp>
#include <atomic>
#include <mutex>

class Joystick : private Threaded {
public:
	Joystick(ADC& adc);
	~Joystick() override;

	float getX() const;
	float getY() const;
	glm::vec2 getPos() const;
	glm::vec2 readAndGetPos();

	void startRangeCalib();
	void stopRangeCalib();
	void centerCalib();

	void begin();
	void end();

private:
	Settings& settings; //used for reading/writing calibration data
	ADCReader adcX;
	ADCReader adcY;

	static constexpr float FilterStrength = 0.3; //1.0 - no filter, 0 - full strength
	static constexpr uint32_t DeadZoneVal = 5;
	void loop() override;

	JoyCalib calibration;
	std::atomic_bool inCalibration = false;

	void enableFilters();
	void disableFilters();

	ThreadedClosure calibThread;
	void calibLoop();
};


#endif //PERSE_MISSIONCTRL_JOYSTICK_H
