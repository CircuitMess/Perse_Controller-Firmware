#ifndef PERSE_MISSIONCTRL_DRIVESCREEN_H
#define PERSE_MISSIONCTRL_DRIVESCREEN_H

#include "UISystem/Screen.h"
#include "Services/Feed.h"
#include "Devices/Joystick.h"
#include "Devices/Encoders.h"
#include "Devices/Input.h"
#include "Util/Events.h"
#include "Services/Comm.h"
#include "UISystem/LabelElement.h"

class DriveScreen : public Screen {
public:
	explicit DriveScreen(Sprite& canvas);
	virtual ~DriveScreen();

private:
	Feed feed;
	std::vector<Color> lastFrame;

	EventQueue dcEvts;
	EventQueue evts;

	Comm& comm;

	Joystick& joy;
	uint64_t lastDirSend = 0;
	static constexpr uint64_t DirSendInterval = 50; // [ms]
	static constexpr int8_t ArmDirectionMultiplier = -2;
	static constexpr int8_t PinchDirectionMultiplier = 5;
	static constexpr int8_t CameraDirectionMultiplier = -4;
	static constexpr uint64_t PanicHoldDuration = 1000;
	void sendDriveDir();

	void extractInfo(const DriveInfo& info);

	void preDraw() override;
	void onLoop() override;

	bool shouldSendZeroDrive = true;
	uint64_t startTime;
	static constexpr uint64_t StartHoldTime = 1000; // [ms]
	bool holdDone = false;

	uint64_t panicHoldStart = 0;
	bool isInPanicMode = false;

	LabelElement* connectedLabel;

	void buildUI();
	void setupControl();

	bool armUnlocked;
	uint8_t pinchPos = 50;
	uint8_t armPos = 50;
	uint8_t camPos = 50;

	void checkEvents();
	void processInput(const Input::Data& evt);
	void processEncoders(const Encoders::Data& evt);

};


#endif //PERSE_MISSIONCTRL_DRIVESCREEN_H
