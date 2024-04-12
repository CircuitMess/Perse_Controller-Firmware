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
#include "UISystem/AnimElement.h"
#include "Services/RoverState.h"
#include "Devices/Potentiometers.h"
#include "Modules/ModuleElement.h"

class DriveScreen : public Screen {
public:
	explicit DriveScreen(Sprite& canvas);
	virtual ~DriveScreen();

protected:
	virtual void preDraw() override;
	virtual void onLoop() override;

	void setCamPosValue(uint8_t pos);

private:
	static constexpr uint64_t DirSendInterval = 50; // [ms]
	static constexpr int8_t ArmDirectionMultiplier = -2;
	static constexpr int8_t PinchDirectionMultiplier = 5;
	static constexpr int8_t CameraDirectionMultiplier = -4;
	static constexpr Color MarkerVisualizationColor = C_RGB(255, 0, 0);
	static constexpr uint64_t MarkerVisualizingInterval = 50; /// [ms]
	static constexpr uint64_t StartHoldTime = 2000; // [ms]
	static constexpr uint64_t PanicHoldDuration = 2000; // [ms]
	static constexpr float JoystickDriveDeadzone = 0.15f;

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
	LabelElement busA = LabelElement(this, "BUS_A");
	LabelElement busB = LabelElement(this, "BUS_B");
	LabelElement busAStatus = LabelElement(this, "OFF");
	LabelElement busBStatus = LabelElement(this, "OFF");

	glm::vec<2, int8_t> cachedMotorSpeeds = { 0, 0 };
	uint8_t cachedDriveDir = 0;

	bool armEnabled;
	uint8_t pinchPos = 50;
	uint8_t armPos = 50;
	uint8_t camPos = 50;

	bool audio = true;

	int64_t lastFeedQualityUpdate = -1;

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

	LabelElement noFeedElement = LabelElement(this, "");

	uint64_t lastMarkerVisualizationTime = 0;
	std::vector<std::pair<int16_t, int16_t>> markerVisualizationData;

	ImageElement panicText = ImageElement(this, "/spiffs/drive/panicText.raw", 74, 13);
	AnimElement panicBar = AnimElement(this, "/spiffs/drive/panicBar.gif");
	LabelElement panicDescription1 = LabelElement(this, "Press EMERGENCY");
	LabelElement panicDescription2 = LabelElement(this, "to resume operation");
	static constexpr uint32_t PanicTextBlinkDuration = 500; //[ms]
	bool panicTextBlink = true;
	uint32_t panicTextMillis = 0;

	LabelElement scanningLabel = LabelElement(this, ScanText);
	static constexpr const char* ScanText = "SCN";
	static constexpr uint32_t ScanBlinkTime = 500; //[ms]
	uint32_t scanBlinkMillis = 0;
	bool scanBlink = true;

	ConnectionStrength lastSentStr = ConnectionStrength::None;

	static constexpr const char* IconPath = "/spiffs/battery/shutdown.raw";
	ImageElement shutdownIcon = ImageElement(this, IconPath, 91, 48);

	ImageElement muteIcon = ImageElement(this, "/spiffs/drive/mute.raw", 15, 15);

	ImageElement qualityBar = ImageElement(this, "/spiffs/drive/quality-bar.raw", 119, 15);
	ImageElement qualityLine = ImageElement(this, "/spiffs/drive/quality-line.raw", 3, 11);
	ImageElement qualityText = ImageElement(this, "/spiffs/drive/camera-feed-text.raw", 86, 13);

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
	void deleteModule(ModuleBus bus);
	void sendCurrentStates();

	void startHoldingPanic();
	void stopHoldingPanic();
	void startPanic();
	void stopPanic();

	void shutdown();
};


#endif //PERSE_MISSIONCTRL_DRIVESCREEN_H
