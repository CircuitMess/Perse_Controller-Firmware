#ifndef PERSE_MISSIONCTRL_DRIVESCREEN_H
#define PERSE_MISSIONCTRL_DRIVESCREEN_H

#include "LV_Interface/LVScreen.h"
#include "Services/Comm.h"
#include "Devices/Joystick.h"
#include "Services/PairService.h"
#include "Devices/Display.h"
#include "Services/Feed.h"

class DriveScreen : public LVScreen {
public:
	DriveScreen(Joystick* joystick, Display* display);
	~DriveScreen() override;

private:
	void onStart() override;
	void onStop() override;
	void loop() override;

	Comm* comm;
	bool calibration = false;
	EventQueue queue;
	Joystick* joystick;
	Display* display;
	PairService* pair = nullptr;

	ThreadedClosure calibThread;
	void calibLoop();

	lv_obj_t* joystickLabel;

	Feed feed;
	bool paired = false;

	ThreadedClosure joySender;
	void sendJoy();

	int head = 0;
	int arm = 50;
	int pinch = 50;
};


#endif //PERSE_MISSIONCTRL_DRIVESCREEN_H
