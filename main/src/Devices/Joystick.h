#ifndef PERSE_MISSIONCTRL_JOYSTICK_H
#define PERSE_MISSIONCTRL_JOYSTICK_H

#include <atomic>
#include <mutex>
#include "Services/Settings.h"
#include "Periph/ADC.h"
#include "vec2.hpp"
#include "Util/Threaded.h"

class Joystick : private Threaded {
public:
	Joystick(gpio_num_t joyX, gpio_num_t joyY);
	~Joystick() override;

	float getX() const;
	float getY() const;
	glm::vec2 getPos() const;

	void startRangeCalib();
	void stopRangeCalib();
	void centerCalib();

private:
	Settings& settings; //used for reading/writing calibration data
	ADC adcX, adcY;

	static constexpr float FilterStrength = 0.3; //1.0 - no filter, 0 - full strength
	static constexpr uint32_t DeadZoneVal = 10;
	void loop() override;

	JoystickCalibration calibration;
	std::atomic_bool inCalibration = false;
	std::mutex rangeCalibrationMut;
	std::mutex snapshotMut;

	void enableFilters();
	void disableFilters();
};


#endif //PERSE_MISSIONCTRL_JOYSTICK_H
