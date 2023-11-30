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
#include "UISystem/ImageElement.h"
#include "Services/RoverState.h"
#include "Devices/Potentiometers.h"
#include "Modules/ModuleElement.h"

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
	static constexpr uint64_t StartHoldTime = 2000; // [ms]
	static constexpr uint64_t PanicHoldDuration = 1000; // [ms]

	Comm& comm;
	Joystick& joy;
	Feed feed;
	std::vector<Color> lastFrame;

	EventQueue dcEvts;
	EventQueue evts;

	RoverState& roverState;

	bool shouldSendZeroDrive = true;
	uint64_t lastDirSend = 0;
	uint64_t panicHoldStart = 0;
	bool isInPanicMode = false;

	uint64_t startTime;
	bool holdDone = false;
	bool isScanningEnabled = false;

	std::unique_ptr<ModuleElement> leftModule = nullptr;
	std::unique_ptr<ModuleElement> rightModule = nullptr;

	glm::vec<2, int8_t> cachedMotorSpeeds = {0, 0};
	uint8_t cachedDriveDir = 0;

	bool armUnlocked;
	uint8_t pinchPos = 50;
	uint8_t armPos = 50;
	uint8_t camPos = 50;

	ImageElement* connectedSign = new ImageElement(this, "/spiffs/drive/connected.raw", 103, 51);

	ImageElement arrowUp = ImageElement(this, "/spiffs/drive/arrow-up.raw", 13, 8);
	ImageElement arrowDown = ImageElement(this, "/spiffs/drive/arrow-down.raw", 13, 8);
	ImageElement arrowLeft = ImageElement(this, "/spiffs/drive/arrow-left.raw", 8, 13);
	ImageElement arrowRight = ImageElement(this, "/spiffs/drive/arrow-right.raw", 8, 13);

	std::array<ImageElement, 4> crosses = {
			ImageElement(this, "/spiffs/intro/cross.raw", 9, 9),
			ImageElement(this, "/spiffs/intro/cross.raw", 9, 9),
			ImageElement(this, "/spiffs/intro/cross.raw", 9, 9),
			ImageElement(this, "/spiffs/intro/cross.raw", 9, 9)
	};

	ImageElement wifiSignalIcon = ImageElement(this, "/spiffs/drive/signal4.raw", 22, 17);

	LabelElement roverBatteryLabel = LabelElement(this, "0");
	LabelElement controllerBatteryLabel = LabelElement(this, "0");

	LabelElement leftMotorSpeedLabel = LabelElement(this, "100");
	LabelElement rightMotorSpeedLabel = LabelElement(this, "100");

	LabelElement rvrElement = LabelElement(this, "RVR");
	LabelElement rssiElement = LabelElement(this, "RSSI");
	LabelElement ctrlElement = LabelElement(this, "CTRL");

	uint64_t lastMarkerVisualizationTime = 0;
	std::vector<std::pair<int16_t, int16_t>> markerVisualizationData;

private:
	void sendDriveDir();
	void buildUI();
	void setupControl();
	void checkEvents();
	void processInput(const Input::Data& evt);
	void processEncoders(const Encoders::Data& evt);
	void processRoverState(const RoverState::Event& evt);
	void processPotentiometers(const Potentiometers::Data& evt);
	void createModule(ModuleBus bus, ModuleType type);
	void sendCurrentStates();
};


#endif //PERSE_MISSIONCTRL_DRIVESCREEN_H