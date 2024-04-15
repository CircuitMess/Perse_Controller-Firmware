#include "DriveScreen.h"
#include "Services/TCPClient.h"
#include "Screens/Pair/PairScreen.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Services/LEDService.h"
#include "glm.hpp"
#include "gtx/vector_angle.hpp"
#include "Devices/Battery.h"
#include "Devices/Potentiometers.h"
#include "Modules/AltPressModule.h"
#include "Modules/CO2Module.h"
#include "Modules/GyroModule.h"
#include "Modules/LEDModule.h"
#include "Modules/MotionModule.h"
#include "Modules/PhotoresModule.h"
#include "Modules/RGBModule.h"
#include "Modules/TempHumModule.h"
#include "Modules/UnkownModule.h"

DriveScreen::DriveScreen(Sprite& canvas) : Screen(canvas), comm(*((Comm*) Services.get(Service::Comm))), joy(*((Joystick*) Services.get(Service::Joystick))),
										   dcEvts(6), evts(32), roverState(*(RoverState*) Services.get(Service::RoverState)){
	if(const TCPClient* tcp = (TCPClient*) Services.get(Service::TCP)){
		if(!tcp->isConnected()){
			transition([](Sprite& canvas){ return std::make_unique<PairScreen>(canvas, true); });
			return;
		}
	}

	lastFrame.resize(160 * 120, 0);

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->breathe(LED::Pair);
		led->on(LED::CamCenter);
	}

	if(connectedSign != nullptr){
		connectedSign->setPos(getWidth() / 2 - connectedSign->getWidth() / 2, getHeight() / 2 - connectedSign->getHeight() / 2);
	}

	for(ImageElement& cross: crosses){
		cross.setPos(-getWidth(), -getHeight());
	}

	TextStyle busStyle = { &lgfx::fonts::Font0, TFT_CYAN, 1, TL_DATUM, { TFT_BLACK, 1 }};
	busA.setPos(-getWidth(), -getHeight());
	busA.setStyle(busStyle);
	busB.setPos(-getWidth(), -getHeight());
	busStyle.datum = TR_DATUM;
	busB.setStyle(busStyle);

	scanningLabel.setStyle({ .color = TFT_WHITE, .datum = TC_DATUM, .shadingStyle = { TFT_BLACK, 1 }});
	scanningLabel.setPos(-getWidth(), -getHeight());

	TextStyle busStatusStyle = { &lgfx::fonts::Font0, TFT_PINK, 1, TL_DATUM, { TFT_BLACK, 1 } };
	busAStatus.setPos(-getWidth(), -getHeight());
	busAStatus.setStyle(busStatusStyle);
	busBStatus.setPos(-getWidth(), -getHeight());
	busStatusStyle.datum = TR_DATUM;
	busBStatus.setStyle(busStatusStyle);

	TextStyle noFeedStyle = {&lgfx::fonts::Font0, TFT_RED, 1, CC_DATUM};
	noFeedElement.setStyle(noFeedStyle);
	noFeedElement.setPos(getWidth() / 2, getHeight() / 2 - 9);

	arrowUp.setPos(-getWidth(), -getHeight());
	arrowDown.setPos(-getWidth(), -getHeight());
	arrowLeft.setPos(-getWidth(), -getHeight());
	arrowRight.setPos(-getWidth(), -getHeight());
	wifiSignalIcon.setPos(-getWidth(), -getHeight());

	roverBatteryLabel.setPos(-getWidth(), -getHeight());
	roverBatteryLabel.setStyle({ .color = TFT_WHITE, .datum = BL_DATUM, .shadingStyle = { TFT_BLACK, 1 }});

	roverBatteryLabel.setText(std::to_string(roverState.getBatteryPercent()));

	controllerBatteryLabel.setPos(-getWidth(), -getHeight());
	controllerBatteryLabel.setStyle({ .color = TFT_WHITE, .datum = BR_DATUM, .shadingStyle={ TFT_BLACK, 1 }});

	if(const Battery* battery = (Battery*) Services.get(Service::Battery)){
		controllerBatteryLabel.setText(std::to_string(battery->getPerc()));
	}

	leftMotorSpeedLabel.setPos(-getWidth(), -getHeight());
	leftMotorSpeedLabel.setStyle({ .color = lgfx::color565(200, 127, 255), .datum = TL_DATUM,  .shadingStyle={ TFT_BLACK, 1 } });

	rightMotorSpeedLabel.setPos(-getWidth(), -getHeight());
	rightMotorSpeedLabel.setStyle({ .color = TFT_YELLOW, .datum = TR_DATUM,  .shadingStyle={ TFT_BLACK, 1 }});

	rvrElement.setPos(-getWidth(), -getHeight());
	rvrElement.setStyle({ .color = TFT_WHITE, .datum = BL_DATUM });

	rssiElement.setPos(-getWidth(), -getHeight());
	rssiElement.setStyle({ .color = TFT_WHITE, .datum = BC_DATUM });

	ctrlElement.setPos(-getWidth(), -getHeight());
	ctrlElement.setStyle({ .color = TFT_WHITE, .datum = BR_DATUM });

	panicBar.setPos(-getWidth(), -getHeight());
	panicBar.setLoopMode(GIF::Single);
	panicBar.start();
	panicBar.stop();
	panicBar.reset();
	panicText.setPos(-getWidth(), -getHeight());
	const TextStyle panicStyle = { .color = lgfx::color565(250, 180, 11), .datum = TC_DATUM, .shadingStyle = { TFT_BLACK, 1 }};
	panicDescription1.setPos(-getWidth(), -getHeight());
	panicDescription1.setStyle(panicStyle);
	panicDescription2.setPos(-getWidth(), -getHeight());
	panicDescription2.setStyle(panicStyle);

	shutdownIcon.setPos(-getWidth(), -getHeight());

	muteIcon.setPos(-getWidth(), -getHeight());

	qualityLine.setPos(-getWidth(), -getHeight());
	qualityBar.setPos(-getWidth(), -getHeight());
	qualityText.setPos(-getWidth(), -getHeight());

	startTime = millis();

	Events::listen(Facility::TCP, &dcEvts);
	Events::listen(Facility::Battery, &dcEvts);

	joy.begin();

	Input* input = (Input*) Services.get(Service::Input);
	if(input == nullptr){
		return;
	}

	armEnabled = input->getState(Input::SwArm);

	sendCurrentStates();
}

DriveScreen::~DriveScreen(){
	Events::unlisten(&dcEvts);
	Events::unlisten(&evts);

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->off(LED::Pair);
		led->off(LED::Arm);
		led->off(LED::Light);
		led->off(LED::PanicRight);
		led->off(LED::PanicLeft);
		for(LED camLed = LED::CamL4; camLed <= LED::CamR4; camLed = (LED) ((uint8_t) camLed + 1)){
			led->off(camLed);
		}
	}

	joy.end();

	delete connectedSign;
}

void DriveScreen::preDraw(){
	DriveInfo driveInfo;

	const bool gotFrame = feed.nextFrame([this, &driveInfo](const DriveInfo& info, const Color* frame){
		driveInfo = info;

		if(frame == nullptr){
			return;
		}

		memcpy(lastFrame.data(), frame, 160 * 120 * 2);
	});

	canvas.pushImage(0, 0, 160, 120, lastFrame.data());

	leftMotorSpeedLabel.setText(std::to_string(cachedMotorSpeeds.x));
	rightMotorSpeedLabel.setText(std::to_string(cachedMotorSpeeds.y));

	if(cachedMotorSpeeds.x == 0 && cachedMotorSpeeds.y == 0){
		arrowUp.setPath("/spiffs/drive/arrow-up.raw");
		arrowRight.setPath("/spiffs/drive/arrow-right.raw");
		arrowDown.setPath("/spiffs/drive/arrow-down.raw");
		arrowLeft.setPath("/spiffs/drive/arrow-left.raw");
	}else{
		if(cachedDriveDir == 7 || cachedDriveDir == 0 || cachedDriveDir == 1){
			arrowUp.setPath("/spiffs/drive/arrow-up-active.raw");
		}else{
			arrowUp.setPath("/spiffs/drive/arrow-up.raw");
		}

		if(cachedDriveDir == 1 || cachedDriveDir == 2 || cachedDriveDir == 3){
			arrowRight.setPath("/spiffs/drive/arrow-right-active.raw");
		}else{
			arrowRight.setPath("/spiffs/drive/arrow-right.raw");
		}

		if(cachedDriveDir == 3 || cachedDriveDir == 4 || cachedDriveDir == 5){
			arrowDown.setPath("/spiffs/drive/arrow-down-active.raw");
		}else{
			arrowDown.setPath("/spiffs/drive/arrow-down.raw");
		}

		if(cachedDriveDir == 5 || cachedDriveDir == 6 || cachedDriveDir == 7){
			arrowLeft.setPath("/spiffs/drive/arrow-left-active.raw");
		}else{
			arrowLeft.setPath("/spiffs/drive/arrow-left.raw");
		}
	}

	if(const Battery* battery = (Battery*) Services.get(Service::Battery)){
		controllerBatteryLabel.setText(std::to_string(battery->getPerc()));
	}

	if(WiFiSTA* wifi = (WiFiSTA*) Services.get(Service::WiFi)){
		const ConnectionStrength str = wifi->getConnectionStrength();

		if(str != lastSentStr){
			comm.sendConnectionStrength(str);
			lastSentStr = str;
		}

		if(str == ConnectionStrength::None){
			wifiSignalIcon.setPath("/spiffs/drive/signal5.raw");
		}else if(str == ConnectionStrength::VeryLow){
			wifiSignalIcon.setPath("/spiffs/drive/signal1.raw");
		}else if(str == ConnectionStrength::Low){
			wifiSignalIcon.setPath("/spiffs/drive/signal2.raw");
		}else if(str == ConnectionStrength::Medium){
			wifiSignalIcon.setPath("/spiffs/drive/signal3.raw");
		}else if(str == ConnectionStrength::High){
			wifiSignalIcon.setPath("/spiffs/drive/signal4.raw");
		}
	}

	if(!isScanningEnabled){
		return;
	}

	if(gotFrame && (lastMarkerVisualizationTime == 0 || (millis() - lastMarkerVisualizationTime) >= MarkerVisualizingInterval)){
		markerVisualizationData.clear();

		if(!driveInfo.markerInfo.markers.empty()){
			for(const std::pair<int16_t, int16_t>& point: driveInfo.markerInfo.markers.front().projected){
				markerVisualizationData.emplace_back(point);
			}

			lastMarkerVisualizationTime = millis();
		}
	}

	for(size_t j = 0; j < markerVisualizationData.size(); ++j){
		const size_t nextIndex = (j + 1) % markerVisualizationData.size();

		const std::pair<int16_t, int16_t>& currentProjected = markerVisualizationData[j];
		const std::pair<int16_t, int16_t>& nextProjected = markerVisualizationData[nextIndex];

		canvas.drawLine(currentProjected.first, currentProjected.second, nextProjected.first, nextProjected.second, MarkerVisualizationColor);
	}
}

void DriveScreen::onLoop(){
	{
		Event evt{};
		if(dcEvts.get(evt, 0)){
			if(evt.facility == Facility::TCP){
				if(const auto data = (TCPClient::Event*) evt.data){
					if(data->status == TCPClient::Event::Status::Disconnected){
						free(evt.data);
						transition([](Sprite& canvas){ return std::make_unique<PairScreen>(canvas, true); });
						return;
					}
				}
			}
			if(evt.facility == Facility::Battery && evt.data != nullptr){
				auto data = (Battery::Event*) evt.data;
				if(data->level == Battery::Critical){
					shutdown();
					free(evt.data);
					return;
				}
			}

			free(evt.data);
		}
	}

	if(!holdDone){
		if(millis() - startTime < StartHoldTime) return;

		delete connectedSign;
		connectedSign = nullptr;

		holdDone = true;
		buildUI();
		setupControl();

		return;
	}

	sendDriveDir();
	checkEvents();

	if(panicHoldStart != 0 && millis() - panicHoldStart >= PanicHoldDuration){
		startPanic();
	}

	if(isInPanicMode && millis() - panicTextMillis >= PanicTextBlinkDuration){
		panicTextBlink = !panicTextBlink;
		panicTextMillis = millis();

		if(panicTextBlink){
			panicText.setPos((int16_t) ((getWidth() - panicText.getWidth()) / 2.0), 38);
		}else{
			panicText.setPos(-getWidth(), -getHeight());
		}
	}

	if(isScanningEnabled && millis() - scanBlinkMillis >= ScanBlinkTime){
		scanBlinkMillis = millis();
		scanBlink = !scanBlink;
		if(scanBlink){
			scanningLabel.setText(ScanText);
		}else{
			scanningLabel.setText("");
		}
	}

	if(lastFeedQualityUpdate >= 0 && millis() - lastFeedQualityUpdate >= 1000){
		buildUI();
		lastFeedQualityUpdate = -1;
	}
}

void DriveScreen::setCamPosValue(uint8_t pos){
	static const std::map<LED, int> ledOnLimits = {
			{LED::CamL4, 40},
			{LED::CamL3, 30},
			{LED::CamL2, 20},
			{LED::CamL1, 10},
			{LED::CamCenter, 0},
			{LED::CamR1, -10},
			{LED::CamR2, -20},
			{LED::CamR3, -30},
			{LED::CamR4, -40},
	};

	camPos = pos;

	LEDService* led = (LEDService*) Services.get(Service::LED);
	if(led == nullptr){
		return;
	}

	const int deltaFromCenter = camPos - 50;

	for(LED camLed = LED::CamL4; camLed <= LED::CamR4; camLed = (LED) ((uint8_t) camLed + 1)){
		if(!ledOnLimits.contains(camLed)){
			led->off(camLed);
			continue;
		}

		const int limit = ledOnLimits.at(camLed);

		if((SIGN(limit) == SIGN(deltaFromCenter) || limit == 0) && std::abs(deltaFromCenter) >= std::abs(limit)){
			led->on(camLed);
		}else{
			led->off(camLed);
		}
	}
}

void DriveScreen::sendDriveDir(){
	if(isInPanicMode){
		return;
	}

	const auto now = millis();
	if(lastDirSend != 0 && now - lastDirSend < DirSendInterval) return;
	lastDirSend = now;

	auto dir = joy.getPos();
	dir.x *= -1;

	const auto len = std::clamp(glm::length(dir), 0.0f, 1.0f);
	if(len < JoystickDriveDeadzone){
		if(shouldSendZeroDrive){
			cachedMotorSpeeds = {0, 0};
			cachedDriveDir = 0;
			comm.sendDriveDir({ 0, 0.0f });
			shouldSendZeroDrive = false;
		}

		return;
	}

	shouldSendZeroDrive = true;

	auto angle = glm::degrees(glm::angle(glm::normalize(dir), { 0.0, 1.0 }));
	if(dir.x < 0){
		angle = 360.0f - angle;
	}

	static constexpr float circParts = 360.0f / 8.0f;

	float calcAngle = angle + circParts / 2.0f;
	if(calcAngle >= 360){
		calcAngle -= 360.0f;
	}
	const uint8_t number = std::floor(calcAngle / circParts);

	comm.sendDriveDir({ number, len });

	float leftSpeed = 0.0f;
	float rightSpeed = 0.0f;

	if(number == 0){
		leftSpeed = rightSpeed = 100;
	}
	else if(number == 1){
		leftSpeed = 100;
		rightSpeed = 30;
	}
	else if(number == 2){
		leftSpeed = 100;
		rightSpeed = -100;
	}
	else if(number == 3){
		leftSpeed = -100;
		rightSpeed = -30;
	}
	else if(number == 4){
		leftSpeed = rightSpeed = -100;
	}
	else if(number == 5){
		leftSpeed = -30;
		rightSpeed = -100;
	}
	else if(number == 6){
		leftSpeed = -100;
		rightSpeed = 100;
	}
	else if(number == 7){
		leftSpeed = 30;
		rightSpeed = 100;
	}

	leftSpeed = std::clamp(leftSpeed * len, -100.0f, 100.0f);
	rightSpeed = std::clamp(rightSpeed * len, -100.0f, 100.0f);

	cachedMotorSpeeds = {leftSpeed, rightSpeed};
	cachedDriveDir = number;
}

void DriveScreen::buildUI(){
	busA.setPos(2, 2);
	busB.setPos(getWidth() - 2, 2);
	busAStatus.setPos(2, 13);
	busBStatus.setPos(getWidth() - 2, 13);
	if(roverState.getLeftModuleInsert()){
		createModule(ModuleBus::Left, roverState.getLeftModuleType());
	}

	if(roverState.getRightModuleInsert()){
		createModule(ModuleBus::Right, roverState.getRightModuleType());
	}

	if(roverState.getNoFeed()){
		noFeedElement.setText("NO FEED");
	}else{
		noFeedElement.setText("");
	}

	static constexpr int16_t CrossMargin = 20;
	static constexpr int16_t ArrowMargin = 30;
	static constexpr int16_t BottomOffset = 9;

	for(int x = 0; x < 2; ++x){
		for(int y = 0; y < 2; ++y){
			ImageElement& cross = crosses[y * 2 + x];
			cross.setPos(CrossMargin + x * (getWidth() - cross.getWidth() - 2 * CrossMargin),
						 CrossMargin + y * (getHeight() - cross.getHeight() - 2 * CrossMargin));
			cross.setY(cross.getY() - BottomOffset);
		}
	}

	arrowUp.setPos(getWidth() / 2 - arrowUp.getWidth() / 2, ArrowMargin);
	arrowDown.setPos(getWidth() / 2 - arrowDown.getWidth() / 2, getHeight() - ArrowMargin - arrowDown.getHeight());
	arrowLeft.setPos(ArrowMargin, getHeight() / 2 - arrowLeft.getHeight() / 2);
	arrowRight.setPos(getWidth() - ArrowMargin - arrowRight.getWidth(), getHeight() / 2 - arrowRight.getHeight() / 2);
	wifiSignalIcon.setPos(getWidth() / 2 - wifiSignalIcon.getWidth() / 2, getHeight() - wifiSignalIcon.getHeight());

	arrowUp.setY(arrowUp.getY() - BottomOffset);
	arrowDown.setY(arrowDown.getY() - BottomOffset);
	arrowLeft.setY(arrowLeft.getY() - BottomOffset);
	arrowRight.setY(arrowRight.getY() - BottomOffset);
	wifiSignalIcon.setY(wifiSignalIcon.getY() - BottomOffset);

	roverBatteryLabel.setPos(2, getHeight() - BottomOffset);
	controllerBatteryLabel.setPos(getWidth() - 2, getHeight() - BottomOffset);

	leftMotorSpeedLabel.setPos(2, 70);
	rightMotorSpeedLabel.setPos(getWidth() - 2, 70);

	rvrElement.setPos(0, getHeight());
	rssiElement.setPos(getWidth() / 2, getHeight());
	ctrlElement.setPos(getWidth(), getHeight());

	qualityBar.setPos(-getWidth(), -getHeight());
	qualityLine.setPos(-getWidth(), -getHeight());
	qualityText.setPos(-getWidth(), -getHeight());
}

void DriveScreen::setupControl(){
	auto input = (Input*) Services.get(Service::Input);
	if(input == nullptr){
		return;
	}

	LEDService* led = (LEDService*) Services.get(Service::LED);

	Events::listen(Facility::Input, &evts);
	Events::listen(Facility::Encoders, &evts);
	Events::listen(Facility::RoverState, &evts);
	Events::listen(Facility::Potentiometers, &evts);
	Events::listen(Facility::RoverState, &evts);

	comm.sendModulesEnable(true);

	if(input->getState(Input::SwLight)){
		comm.sendHeadlights(HeadlightsMode::On);

		if(led != nullptr){
			led->on(LED::Light);
		}
	}

	armEnabled = input->getState(Input::SwArm);
	if(armEnabled){
		if(led != nullptr){
			led->on(LED::Arm);
		}
	}

	roverBatteryLabel.setText(std::to_string(roverState.getBatteryPercent()));
}

void DriveScreen::checkEvents(){
	for(Event evt{}; evts.get(evt, 0); ){
		if(evt.data == nullptr){
			return;
		}

		if(evt.facility == Facility::Input){
			auto data = (Input::Data*) evt.data;
			processInput(*data);
		}else if(evt.facility == Facility::Encoders){
			auto data = (Encoders::Data*) evt.data;
			processEncoders(*data);
		}else if(evt.facility == Facility::RoverState){
			auto data = (RoverState::Event*) evt.data;
			processRoverState(*data);
		}else if(evt.facility == Facility::Potentiometers){
			auto data = (Potentiometers::Data*) evt.data;
			processPotentiometers(*data);
		}

		free(evt.data);
	}
}

void DriveScreen::processInput(const Input::Data& evt){
	LEDService* led = (LEDService*) Services.get(Service::LED);

	if(evt.btn == Input::Panic){
		if(evt.action == Input::Data::Press){
			if(isInPanicMode){
				stopPanic();
			}else{
				startHoldingPanic();
			}
		}else{
			stopHoldingPanic();
		}
	}else if(evt.btn == Input::SwArm){
		armEnabled = evt.action == Input::Data::Press;
		comm.sendArmEnabled(armEnabled);

		if(led != nullptr){
			if(armEnabled){
				led->on(LED::Arm);
			}else{
				led->off(LED::Arm);
			}
		}
	}else if(isInPanicMode){
		return;
	}else if(evt.btn == Input::SwLight){
		if(evt.action == Input::Data::Press){
			comm.sendHeadlights(HeadlightsMode::On);

			if(led != nullptr){
				led->on(LED::Light);
			}
		}else{
			comm.sendHeadlights(HeadlightsMode::Off);

			if(led != nullptr){
				led->off(LED::Light);
			}
		}
	}else if(evt.btn == Input::EncCam){
		if(evt.action == Input::Data::Press){
			isScanningEnabled = !isScanningEnabled;
			comm.sendScanningEnable(isScanningEnabled);
			if(isScanningEnabled){
				scanningLabel.setPos((int16_t) (getWidth() / 2), 2);
				scanningLabel.setText(ScanText);
				scanBlinkMillis = millis();
				scanBlink = true;
			}else{
				scanningLabel.setPos(-getWidth(), -getHeight());
			}
		}
	}else if(evt.btn == Input::EncArm){
		armPos = 50;
		comm.sendArmPos(armPos);
	}else if(evt.btn == Input::EncPinch){
		pinchPos = 50;
		comm.sendArmPinch(pinchPos);
	}else if(evt.btn == Input::Joy && evt.action == Input::Data::Press){
		audio = !audio;
		comm.sendAudio(audio);
		if(audio){
			muteIcon.setPos(-getWidth(), -getHeight());
		}else{
			muteIcon.setPos(30, getHeight() - 9 - 15);
		}
	}
}

void DriveScreen::processEncoders(const Encoders::Data& evt){
	LEDService* led = (LEDService*) Services.get(Service::LED);

	if((evt.enc == Encoders::Pinch || evt.enc == Encoders::Arm) && !armEnabled) return;

	if(isInPanicMode){
		return;
	}

	if(evt.enc == Encoders::Arm){
		armPos = std::clamp(armPos + ArmDirectionMultiplier * evt.dir, 0, 100);

		comm.sendArmPos(armPos);

		if(led != nullptr){
			led->blink(evt.dir > 0 ? LED::ArmDown : LED::ArmUp, 1, 200);
		}
	}else if(evt.enc == Encoders::Pinch){
		pinchPos = std::clamp(pinchPos + PinchDirectionMultiplier * evt.dir, 0, 100);

		comm.sendArmPinch(pinchPos);

		if(led != nullptr){
			led->blink(evt.dir > 0 ? LED::PinchOpen : LED::PinchClose, 1, 200);
		}
	}else if(evt.enc == Encoders::Cam){
		setCamPosValue(std::clamp(camPos + CameraDirectionMultiplier * evt.dir, 0, 100));

		comm.sendCameraRotation(camPos);
	}
}

void DriveScreen::processRoverState(const RoverState::Event& evt){
	if(evt.type == RoverState::StateType::BatteryPercent){
		roverBatteryLabel.setText(std::to_string(evt.batteryPercent));
	}else if(evt.type == RoverState::StateType::Modules){
		if(evt.modulePlug.bus == ModuleBus::Left){
			if(!evt.modulePlug.insert){
				if(leftModule){
					deleteModule(ModuleBus::Left);
				}
			}else if(leftModule == nullptr){
				createModule(ModuleBus::Left, evt.modulePlug.type);
			}
		}

		if(evt.modulePlug.bus == ModuleBus::Right){
			if(!evt.modulePlug.insert){
				if(rightModule){
					deleteModule(ModuleBus::Right);
				}
			}else if(rightModule == nullptr){
				createModule(ModuleBus::Right, evt.modulePlug.type);
			}
		}
	}else if(evt.type == RoverState::StateType::Feed){
		if(evt.noFeed){
			noFeedElement.setText("NO FEED");
		}else{
			noFeedElement.setText("");
		}
	}

    if(!evt.changedOnRover){
        return;
    }

    if(evt.type == RoverState::StateType::ArmPos){
        armPos = roverState.getArmPositionState();
    }else if(evt.type == RoverState::StateType::ArmPinch){
        pinchPos = roverState.getArmPinchState();
    }else if(evt.type == RoverState::StateType::CameraRotation){
        setCamPosValue(roverState.getCameraRotationState());
    }
}

void DriveScreen::processPotentiometers(const Potentiometers::Data& evt){
	if(evt.potentiometer != Potentiometers::FeedQuality){
		return;
	}

	if(isInPanicMode){
		return;
	}

	const uint8_t value = std::clamp(100 - evt.value, 0, 100);
	const uint8_t quality = map(value, 0, 100, 1, 30);

	comm.sendFeedQuality(quality);

	if(panicHoldStart == 0 && !roverState.getNoFeed() && (shutdownIcon.getX() <= -shutdownIcon.getWidth() || shutdownIcon.getY() <= -shutdownIcon.getHeight()) && !isInShutdown){
		lastFeedQualityUpdate = millis();

		qualityBar.setPos(6, 52);

		qualityText.setPos((getWidth() - qualityText.getWidth()) / 2, 36);

		const float percent = value / 100.0f;
		qualityLine.setPos(6 + (qualityBar.getWidth() - 4) * (1.0f - percent), 50);

		arrowUp.setPos(-getWidth(), -getHeight());
		arrowDown.setPos(-getWidth(), -getHeight());
		arrowLeft.setPos(-getWidth(), -getHeight());
		arrowRight.setPos(-getWidth(), -getHeight());
		leftMotorSpeedLabel.setPos(-getWidth(), -getHeight());
		rightMotorSpeedLabel.setPos(-getWidth(), -getHeight());

		if(leftModule){
			leftModule->setPos(-getWidth(), -getHeight());
		}

		if(rightModule){
			rightModule->setPos(-getWidth(), -getHeight());
		}
	}
}

void DriveScreen::createModule(ModuleBus bus, ModuleType type){
	ModuleElement* module = nullptr;
	switch(type){
		case ModuleType::Gyro:
			module = new GyroModule(this, bus, type);
			break;
		case ModuleType::TempHum:
			module = new TempHumModule(this, bus, type);
			break;
		case ModuleType::AltPress:
			module = new AltPressModule(this, bus, type);
			break;
		case ModuleType::LED:
			module = new LEDModule(this, bus, type);
			break;
		case ModuleType::RGB:
			module = new RGBModule(this, bus, type);
			break;
		case ModuleType::PhotoRes:
			module = new PhotoresModule(this, bus, type);
			break;
		case ModuleType::Motion:
			module = new MotionModule(this, bus, type);
			break;
		case ModuleType::CO2:
			module = new CO2Module(this, bus, type);
			break;
		default:
			module = new UnkownModule(this, bus, type);
			break;
	}

	if(module == nullptr){
		return;
	}

	if(bus == ModuleBus::Left){
		leftModule.reset(module);
		leftModule->setPos(2, 30);
		busAStatus.setText("ON");
		busAStatus.setColor(TFT_GREENYELLOW);
	}else{
		rightModule.reset(module);
		rightModule->setPos(126, 30);
		busBStatus.setText("ON");
		busBStatus.setColor(TFT_GREENYELLOW);
	}
}

void DriveScreen::deleteModule(ModuleBus bus){
	if(bus == ModuleBus::Left){
		leftModule.reset();
		busAStatus.setText("OFF");
		busAStatus.setColor(TFT_PINK);
	}else{
		rightModule.reset();
		busBStatus.setText("OFF");
		busBStatus.setColor(TFT_PINK);
	}
}

void DriveScreen::sendCurrentStates(){
	Input* input = (Input*) Services.get(Service::Input);
	if(input == nullptr){
		return;
	}

	LEDService* led = (LEDService*) Services.get(Service::LED);

	if(input->getState(Input::Button::SwLight)){
		comm.sendHeadlights(HeadlightsMode::On);

		if(led != nullptr){
			led->on(LED::Light);
		}
	}else{
		comm.sendHeadlights(HeadlightsMode::Off);

		if(led != nullptr){
			led->off(LED::Light);
		}
	}

	if(Potentiometers* potentiometers = (Potentiometers*) Services.get(Service::Potentiometers)){
		const uint8_t value = std::clamp(100 - potentiometers->scanCurrentValue(Potentiometers::FeedQuality), 0, 100);
		const uint8_t quality = map(value, 0, 100, 1, 30);
		comm.sendFeedQuality(quality);
	}else{
		comm.sendFeedQuality(15);
	}
}

void DriveScreen::startHoldingPanic(){
	panicHoldStart = millis();

	auto led = (LEDService*) Services.get(Service::LED);
	if(led != nullptr){
		led->breathe(LED::PanicLeft, 2 * PanicHoldDuration);
		led->breathe(LED::PanicRight, 2 * PanicHoldDuration);
	}

	panicBar.setPos(9, 54);
	panicBar.start();

	arrowUp.setPos(-getWidth(), -getHeight());
	arrowDown.setPos(-getWidth(), -getHeight());
	arrowLeft.setPos(-getWidth(), -getHeight());
	arrowRight.setPos(-getWidth(), -getHeight());
	leftMotorSpeedLabel.setPos(-getWidth(), -getHeight());
	rightMotorSpeedLabel.setPos(-getWidth(), -getHeight());
	qualityBar.setPos(-getWidth(), -getHeight());
	qualityLine.setPos(-getWidth(), -getHeight());
	qualityText.setPos(-getWidth(), -getHeight());
}

void DriveScreen::stopHoldingPanic(){
	panicHoldStart = 0;

	if(isInPanicMode) return;

	auto led = (LEDService*) Services.get(Service::LED);
	if(led != nullptr){
		led->off(LED::PanicLeft);
		led->off(LED::PanicRight);
	}

	panicBar.setPos(-getWidth(), -getHeight());
	panicBar.reset();
	panicBar.stop();
	buildUI();
}

void DriveScreen::startPanic(){
	comm.sendScanningEnable(false);
	comm.sendEmergencyMode(true);
	isInPanicMode = true;
	isScanningEnabled = false;
	panicHoldStart = 0;
	scanningLabel.setPos(-getWidth(), -getHeight());

	auto led = (LEDService*) Services.get(Service::LED);
	if(led != nullptr){
		led->blink(LED::PanicLeft, 0);
		led->blink(LED::PanicRight, 0);
	}

	panicText.setPos((int16_t) ((getWidth() - panicText.getWidth()) / 2.0), 38);
	panicDescription1.setPos((int16_t) (getWidth() / 2), 69);
	panicDescription2.setPos((int16_t) (getWidth() / 2), 78);
	panicTextMillis = millis();
	panicTextBlink = true;
}

void DriveScreen::stopPanic(){
	sendCurrentStates();
	comm.sendEmergencyMode(false);
	isInPanicMode = false;
	panicHoldStart = 0;

	auto led = (LEDService*) Services.get(Service::LED);
	if(led != nullptr){
		led->off(LED::PanicLeft);
		led->off(LED::PanicRight);
	}

	panicBar.reset();
	panicBar.stop();
	panicBar.setPos(-getWidth(), -getHeight());
	panicText.setPos(-getWidth(), -getHeight());
	panicDescription1.setPos(-getWidth(), -getHeight());
	panicDescription2.setPos(-getWidth(), -getHeight());

	buildUI();
}

void DriveScreen::shutdown(){
	isInShutdown = true;

	if(isInPanicMode){
		stopPanic();
	}

	arrowUp.setPos(-getWidth(), -getHeight());
	arrowDown.setPos(-getWidth(), -getHeight());
	arrowLeft.setPos(-getWidth(), -getHeight());
	arrowRight.setPos(-getWidth(), -getHeight());
	qualityBar.setPos(-getWidth(), -getHeight());
	qualityLine.setPos(-getWidth(), -getHeight());
	qualityText.setPos(-getWidth(), -getHeight());

	shutdownIcon.setPos((int16_t) ((this->getWidth() - shutdownIcon.getWidth()) / 2.0),
						(int16_t) ((this->getHeight() - shutdownIcon.getHeight()) / 2.0));
	Events::unlisten(&dcEvts);
	Events::unlisten(&evts);
}
