#include "DriveScreen.h"
#include "Services/TCPClient.h"
#include "Screens/PairScreen.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Services/LEDService.h"
#include "glm.hpp"
#include "gtx/vector_angle.hpp"
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

	connectedLabel = new LabelElement(this, "Connected");
	connectedLabel->setStyle({
									 .color = TFT_GREEN,
									 .datum = CC_DATUM
							 });
	connectedLabel->setPos(64, 64);

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
		led->off(LED::PanicRight);
		led->off(LED::PanicLeft);
	}

	joy.end();
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
				transition([](Sprite& canvas){ return std::make_unique<PairScreen>(canvas); });
				return;
			}
		}

		free(evt.data);
	}

	if(!holdDone){
		if(millis() - startTime < StartHoldTime) return;

		delete connectedLabel;
		connectedLabel = nullptr;

		holdDone = true;
		buildUI();
		setupControl();

		return;
	}

	sendDriveDir();
	checkEvents();
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

}

void DriveScreen::setupControl(){
	auto input = (Input*) Services.get(Service::Input);
	LEDService* led = (LEDService*) Services.get(Service::LED);

	Events::listen(Facility::Input, &evts);
	Events::listen(Facility::Encoders, &evts);
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
	}else if(evt.facility == Facility::Potentiometers){
		auto data = (Potentiometers::Data*) evt.data;
		processPotentiometers(*data);
	}

	free(evt.data);
}

void DriveScreen::processInput(const Input::Data& evt){
	LEDService* led = (LEDService*) Services.get(Service::LED);

	if(evt.btn == Input::Panic){
		if(evt.action == Input::Data::Press){
			if(isInPanicMode){
				comm.sendEmergencyMode(false);
				isInPanicMode = false;
				panicHoldStart = 0;

				LEDService* led = (LEDService*) Services.get(Service::LED);
				if(led != nullptr){
					led->off(LED::PanicLeft);
					led->off(LED::PanicRight);
				}
			}else{
				panicHoldStart = millis();

				LEDService* led = (LEDService*) Services.get(Service::LED);
				if(led != nullptr){
					led->breathe(LED::PanicLeft, 2 * PanicHoldDuration);
					led->breathe(LED::PanicRight, 2 * PanicHoldDuration);
				}
			}
		}else{
			if(panicHoldStart != 0 && millis() - panicHoldStart >= PanicHoldDuration){
				comm.sendEmergencyMode(true);
				isInPanicMode = true;

				LEDService* led = (LEDService*) Services.get(Service::LED);
				if(led != nullptr){
					led->blink(LED::PanicLeft, 0);
					led->blink(LED::PanicRight, 0);
				}
			}else{
				isInPanicMode = false;

				LEDService* led = (LEDService*) Services.get(Service::LED);
				if(led != nullptr){
					led->off(LED::PanicLeft);
					led->off(LED::PanicRight);
				}
			}

			panicHoldStart = 0;
		}
	}else if(evt.btn == Input::SwArm){
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
			if(!isInPanicMode){
				comm.sendHeadlights(HeadlightsMode::On);
			}

			if(led != nullptr){
				led->on(LED::Light);
			}
		}else{
			if(!isInPanicMode){
				comm.sendHeadlights(HeadlightsMode::Off);
			}

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

		if(!isInPanicMode){
			comm.sendArmPos(armPos);
		}

		if(led != nullptr){
			led->blink(evt.dir > 0 ? LED::ArmDown : LED::ArmUp);
		}
	}else if(evt.enc == Encoders::Pinch){
		pinchPos = std::clamp(pinchPos + PinchDirectionMultiplier * evt.dir, 0, 100);

		if(!isInPanicMode){
			comm.sendArmPinch(pinchPos);
		}

		if(led != nullptr){
			led->blink(evt.dir > 0 ? LED::PinchOpen : LED::PinchClose);
		}
	}else if(evt.enc == Encoders::Cam){
		camPos = std::clamp(camPos + CameraDirectionMultiplier * evt.dir, 0, 100);

		if(!isInPanicMode){
			comm.sendCameraRotation(camPos);
		}
	}
}

void DriveScreen::processPotentiometers(const Potentiometers::Data& evt){
	if(evt.potentiometer != Potentiometers::FeedQuality){
		return;
	}

	const uint8_t value = std::clamp(100 - evt.value, 0, 100);
	const uint8_t quality = map(value, 0, 100, 0, 30);

	comm.sendFeedQuality(quality);
}
