#ifndef PERSE_MISSIONCTRL_DRIVESCREEN_H
#define PERSE_MISSIONCTRL_DRIVESCREEN_H

#include "LV_Interface/LVScreen.h"
#include "Services/Comm.h"
#include "Devices/Joystick.h"
#include "Services/PairService.h"

class DriveScreen : public LVScreen {
public:
	DriveScreen(Joystick* joystick);
	~DriveScreen() override;

private:
	void onStart() override;
	void onStop() override;
	void loop() override;

	Comm* comm;
	bool calibration = false;
	EventQueue queue;
	Joystick* joystick;
	PairService* pair = nullptr;

	ThreadedClosure calibThread;
	void calibLoop();

	lv_obj_t* joystickLabel;
};


#endif //PERSE_MISSIONCTRL_DRIVESCREEN_H
