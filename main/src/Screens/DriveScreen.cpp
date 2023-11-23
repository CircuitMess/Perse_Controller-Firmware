#include "DriveScreen.h"
#include "Services/TCPClient.h"
#include "Screens/PairScreen.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Services/LEDService.h"
#include "glm.hpp"
#include "gtx/vector_angle.hpp"
#include "Devices/Battery.h"
#include "Devices/Potentiometers.h"

DriveScreen::DriveScreen(Sprite& canvas) : Screen(canvas), comm(*((Comm*) Services.get(Service::Comm))), joy(*((Joystick*) Services.get(Service::Joystick))),
										   dcEvts(6), evts(12){
	lastFrame.resize(160 * 120, 0);

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->on(LED::Pair);
	}

	if(Potentiometers* potentiometers = (Potentiometers*) Services.get(Service::Potentiometers)){
		const uint8_t value = std::clamp(100 - potentiometers->scanCurrentValue(Potentiometers::FeedQuality), 0, 100);
		const uint8_t quality = map(value, 0, 100, 0, 30);
		comm.sendFeedQuality(quality);
	}else{
		comm.sendFeedQuality(30);
	}

	if(connectedSign != nullptr){
		connectedSign->setPos(getWidth() / 2 - connectedSign->getWidth() / 2, getHeight() / 2 - connectedSign->getHeight() / 2);
	}

	for(ImageElement& cross: crosses){
		cross.setPos(-getWidth(), -getHeight());
	}

	arrowUp.setPos(-getWidth(), -getHeight());
	arrowDown.setPos(-getWidth(), -getHeight());
	arrowLeft.setPos(-getWidth(), -getHeight());
	arrowRight.setPos(-getWidth(), -getHeight());
	wifiSignalIcon.setPos(-getWidth(), -getHeight());

	roverBatteryLabel.setPos(-getWidth(), -getHeight());
	roverBatteryLabel.setStyle({ .color = TFT_WHITE, .datum = BL_DATUM });

	if(const RoverState* roverState = (RoverState*) Services.get(Service::RoverState)){
		roverBatteryLabel.setText(std::to_string(roverState->getBatteryPercent()));
	}

	controllerBatteryLabel.setPos(-getWidth(), -getHeight());
	controllerBatteryLabel.setStyle({ .color = TFT_WHITE, .datum = BR_DATUM });

	if(const Battery* battery = (Battery*) Services.get(Service::Battery)){
		controllerBatteryLabel.setText(std::to_string(battery->getPerc()));
	}

	leftMotorSpeedLabel.setPos(-getWidth(), -getHeight());
	leftMotorSpeedLabel.setStyle({ .color = TFT_PURPLE, .datum = TL_DATUM });

	rightMotorSpeedLabel.setPos(-getWidth(), -getHeight());
	rightMotorSpeedLabel.setStyle({ .color = TFT_YELLOW, .datum = TR_DATUM });

	rvrElement.setPos(-getWidth(), -getHeight());
	rvrElement.setStyle({ .color = TFT_WHITE, .datum = BL_DATUM });

	rssiElement.setPos(-getWidth(), -getHeight());
	rssiElement.setStyle({ .color = TFT_WHITE, .datum = BC_DATUM });

	ctrlElement.setPos(-getWidth(), -getHeight());
	ctrlElement.setStyle({ .color = TFT_WHITE, .datum = BR_DATUM });

	startTime = millis();

	Events::listen(Facility::TCP, &dcEvts);

	joy.begin();

	// TODO: setup Pinch, Arm, Cam LEDs
	// TODO: setup headlight LED and send state
}

DriveScreen::~DriveScreen(){
	Events::unlisten(&dcEvts);
	Events::unlisten(&evts);

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->off(LED::Pair);
		led->off(LED::Arm);
		led->off(LED::Light);
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

	// TODO cache this when sending it to the rover, use that to calculate motor speeds and which arrows to light up
	auto dir = joy.getPos();
	dir.x *= -1;

	const auto len = std::clamp(glm::length(dir), 0.0f, 1.0f);
	if(len < 0.1){
		arrowUp.setPath("/spiffs/drive/arrow-up.raw");
		arrowRight.setPath("/spiffs/drive/arrow-right.raw");
		arrowDown.setPath("/spiffs/drive/arrow-down.raw");
		arrowLeft.setPath("/spiffs/drive/arrow-left.raw");
	}else{
		auto angle = glm::degrees(glm::angle(glm::normalize(dir), { 0.0, 1.0 }));
		if(dir.x < 0){
			angle = 360.0f - angle;
		}

		static constexpr float circParts = 360.0 / 8.0;

		float calcAngle = angle + circParts / 2.0;
		if(calcAngle >= 360){
			calcAngle -= 360.0f;
		}

		const uint8_t number = std::floor(calcAngle / circParts);

		if(number == 7 || number == 0 || number == 1){
			arrowUp.setPath("/spiffs/drive/arrow-up-active.raw");
		}else{
			arrowUp.setPath("/spiffs/drive/arrow-up.raw");
		}

		if(number == 1 || number == 2 || number == 3){
			arrowRight.setPath("/spiffs/drive/arrow-right-active.raw");
		}else{
			arrowRight.setPath("/spiffs/drive/arrow-right.raw");
		}

		if(number == 3 || number == 4 || number == 5){
			arrowDown.setPath("/spiffs/drive/arrow-down-active.raw");
		}else{
			arrowDown.setPath("/spiffs/drive/arrow-down.raw");
		}

		if(number == 5 || number == 6 || number == 7){
			arrowLeft.setPath("/spiffs/drive/arrow-left-active.raw");
		}else{
			arrowLeft.setPath("/spiffs/drive/arrow-left.raw");
		}
	}

	if(const Battery* battery = (Battery*) Services.get(Service::Battery)){
		controllerBatteryLabel.setText(std::to_string(battery->getPerc()));
	}

	if(WiFiSTA* wifi = (WiFiSTA*) Services.get(Service::WiFi)){
		WiFiSTA::ConnectionStrength str = wifi->getConnectionStrength();

		if(str == WiFiSTA::None){
			wifiSignalIcon.setPath("/spiffs/drive/signal5.raw");
		}else if(str == WiFiSTA::VeryLow){
			wifiSignalIcon.setPath("/spiffs/drive/signal1.raw");
		}else if(str == WiFiSTA::Low){
			wifiSignalIcon.setPath("/spiffs/drive/signal2.raw");
		}else if(str == WiFiSTA::Medium){
			wifiSignalIcon.setPath("/spiffs/drive/signal3.raw");
		}else if(str == WiFiSTA::High){
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
	Event evt{};
	if(dcEvts.get(evt, 0)){
		if(evt.facility == Facility::TCP){
			const auto data = (TCPClient::Event*) evt.data;
			if(data->status == TCPClient::Event::Status::Disconnected){
				free(evt.data);
				transition([](Sprite& canvas){ return std::make_unique<PairScreen>(canvas, true); });
				return;
			}
		}

		free(evt.data);
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
}

void DriveScreen::sendDriveDir(){
	const auto now = millis();
	if(lastDirSend != 0 && now - lastDirSend < DirSendInterval) return;
	lastDirSend = now;

	auto dir = joy.getPos();
	dir.x *= -1;

	const auto len = std::clamp(glm::length(dir), 0.0f, 1.0f);
	if(len < 0.1){
		if(shouldSendZeroDrive){
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
}

void DriveScreen::buildUI(){
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
}

void DriveScreen::setupControl(){
	auto input = (Input*) Services.get(Service::Input);
	LEDService* led = (LEDService*) Services.get(Service::LED);

	Events::listen(Facility::Input, &evts);
	Events::listen(Facility::Encoders, &evts);
	Events::listen(Facility::RoverState, &evts);
	Events::listen(Facility::Potentiometers, &evts);

	if(input->getState(Input::SwLight)){
		comm.sendHeadlights(HeadlightsMode::On);

		if(led != nullptr){
			led->on(LED::Light);
		}
	}

	armUnlocked = input->getState(Input::SwArm);
	if(armUnlocked){
		if(led != nullptr){
			led->on(LED::Arm);
		}
	}
}

void DriveScreen::checkEvents(){
	Event evt{};
	if(!evts.get(evt, 0)) return;

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

void DriveScreen::processInput(const Input::Data& evt){
	LEDService* led = (LEDService*) Services.get(Service::LED);

	if(evt.btn == Input::SwArm){
		armUnlocked = evt.action == Input::Data::Press;

		if(led != nullptr){
			if(armUnlocked){
				led->on(LED::Arm);
			}else{
				led->off(LED::Arm);
			}
		}
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
		}
	}
}

void DriveScreen::processEncoders(const Encoders::Data& evt){
	LEDService* led = (LEDService*) Services.get(Service::LED);

	if((evt.enc == Encoders::Pinch || evt.enc == Encoders::Arm) && !armUnlocked) return;

	if(evt.enc == Encoders::Arm){
		armPos = std::clamp(armPos + ArmDirectionMultiplier * evt.dir, 0, 100);
		comm.sendArmPos(armPos);

		if(led != nullptr){
			led->blink(evt.dir > 0 ? LED::ArmDown : LED::ArmUp);
		}
	}else if(evt.enc == Encoders::Pinch){
		pinchPos = std::clamp(pinchPos + PinchDirectionMultiplier * evt.dir, 0, 100);
		comm.sendArmPinch(pinchPos);

		if(led != nullptr){
			led->blink(evt.dir > 0 ? LED::PinchOpen : LED::PinchClose);
		}
	}else if(evt.enc == Encoders::Cam){
		camPos = std::clamp(camPos + CameraDirectionMultiplier * evt.dir, 0, 100);
		comm.sendCameraRotation(camPos);
	}
}

void DriveScreen::processRoverState(const RoverState::Event& evt){
	if(evt.type != RoverState::StateType::BatteryPercent){
		return;
	}

	roverBatteryLabel.setText(std::to_string(evt.batteryPercent));
}

void DriveScreen::processPotentiometers(const Potentiometers::Data& evt){
	if(evt.potentiometer != Potentiometers::FeedQuality){
		return;
	}

	const uint8_t value = std::clamp(100 - evt.value, 0, 100);
	const uint8_t quality = map(value, 0, 100, 0, 30);

	comm.sendFeedQuality(quality);
}
