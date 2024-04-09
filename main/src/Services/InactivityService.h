#ifndef PERSE_MISSIONCTRL_INACTIVITYSERVICE_H
#define PERSE_MISSIONCTRL_INACTIVITYSERVICE_H

#include <atomic>
#include "Util/Threaded.h"
#include "Util/Events.h"

#ifdef CTRL_TYPE_MISSIONCTRL
#include "Devices/Joystick.h"
#endif

class InactivityService : public Threaded {
public:
	InactivityService();

protected:
	void loop() override;

private:
	bool checkActions();

	uint32_t timer = 0;
	EventQueue queue;
	bool paired = false;

#ifdef CTRL_TYPE_MISSIONCTRL
	Joystick* joystick;
	static constexpr float JoystickStep = 0.1; //step that joystick value needs to shift between readings, as to indicate a user input
#endif

	static constexpr uint32_t UnpairedTimeout = 300000; //5 mins
	static constexpr uint32_t PairedTimeout = 600000; //10 mins
};


#endif //PERSE_MISSIONCTRL_INACTIVITYSERVICE_H
