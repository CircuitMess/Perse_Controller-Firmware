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
#include "Devices/Potentiometers.h"

class DriveScreen : public Screen {
public:
	explicit DriveScreen(Sprite& canvas);
	virtual ~DriveScreen();

protected:
	void preDraw() override;
	void onLoop() override;

private:
	static constexpr uint64_t DirSendInterval = 50; // [ms]
	static constexpr int8_t ArmDirectionMultiplier = -2;
	static constexpr int8_t PinchDirectionMultiplier = 5;
	static constexpr int8_t CameraDirectionMultiplier = -4;
	static constexpr Color MarkerVisualizationColor = C_RGB(255, 0, 0);
	static constexpr uint64_t MarkerVisualizingInterval = 50; /// [ms]
	static constexpr uint64_t StartHoldTime = 1000; // [ms]

	Comm& comm;
	Joystick& joy;
	Feed feed;
	std::vector<Color> lastFrame;

	EventQueue dcEvts;
	EventQueue evts;

	LabelElement* connectedLabel;

	uint64_t startTime;
	bool holdDone = false;
	bool isScanningEnabled = false;

	bool shouldSendZeroDrive = true;
	uint64_t lastDirSend = 0;

	bool armUnlocked;
	uint8_t pinchPos = 50;
	uint8_t armPos = 50;
	uint8_t camPos = 50;

	uint64_t lastMarkerVisualizationTime = 0;
	std::vector<std::pair<int16_t, int16_t>> markerVisualizationData;

private:
	void sendDriveDir();
	void buildUI();
	void setupControl();
	void checkEvents();
	void processInput(const Input::Data& evt);
	void processEncoders(const Encoders::Data& evt);
	void processPotentiometers(const Potentiometers::Data& evt);
};


#endif //PERSE_MISSIONCTRL_DRIVESCREEN_H
