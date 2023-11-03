#include "DriveScreen.h"
#include "Services/TCPClient.h"
#include "Screens/PairScreen.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Pins.hpp"
#include "Services/LEDService.h"
#include "glm.hpp"
#include "gtx/vector_angle.hpp"

DriveScreen::DriveScreen(Sprite& canvas) : Screen(canvas), dcEvts(6), evts(12), comm(*((Comm*)Services.get(Service::Comm))), joy(*((Joystick*)Services.get(Service::Joystick))){
	lastFrame.resize(160*120, 0);

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->on(LED::Pair);
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

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->off(LED::Pair);
		led->off(LED::Arm);
		led->off(LED::Light);
	}

	joy.end();
}

void DriveScreen::preDraw(){
	bool gotFrame = feed.nextFrame([this](const DriveInfo& info, const Color* frame){
		extractInfo(info);
		memcpy(lastFrame.data(), frame, 160*120*2);
	});

	canvas.pushImage(0, 0, 160, 120, lastFrame.data());
}

void DriveScreen::extractInfo(const DriveInfo& info){

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
	const auto now = millis();
	if(lastDirSend != 0 && now - lastDirSend < DirSendInterval) return;
	lastDirSend = now;

	auto dir = joy.getPos();
	dir.x *= -1;

	const auto len = std::clamp(glm::length(dir), 0.0f, 1.0f);
	if(len < 0.1){
		if (shouldSendZeroDrive) {
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

	static constexpr float circParts = 360.0/8.0;

	float calcAngle = angle + circParts/2.0;
	if(calcAngle >= 360){
		calcAngle -= 360.0f;
	}
	const uint8_t numer = std::floor(calcAngle / circParts);

	comm.sendDriveDir({ numer, len });
}

void DriveScreen::buildUI(){

}

void DriveScreen::setupControl(){
	auto input = (Input*) Services.get(Service::Input);
	LEDService* led = (LEDService*)Services.get(Service::LED);

	Events::listen(Facility::Input, &evts);
	Events::listen(Facility::Encoders, &evts);

	if(input->getState(Input::SwLight)){
		comm.sendHeadlights(HeadlightsMode::On);

		if (led != nullptr) {
			led->on(LED::Light);
		}
	}

	armUnlocked = input->getState(Input::SwArm);
	if(armUnlocked){
		if (led!= nullptr) {
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
	}

	free(evt.data);
}

void DriveScreen::processInput(const Input::Data& evt){
	LEDService* led = (LEDService*)Services.get(Service::LED);

	if(evt.btn == Input::SwArm){
		armUnlocked = evt.action == Input::Data::Press;
		if(armUnlocked){
			led->on(LED::Arm);
		}else{
			led->off(LED::Arm);
		}
	}else if(evt.btn == Input::SwLight){
		if(evt.action == Input::Data::Press){
			comm.sendHeadlights(HeadlightsMode::On);
			led->on(LED::Light);
		}else{
			comm.sendHeadlights(HeadlightsMode::Off);
			led->off(LED::Light);
		}
	}
}

void DriveScreen::processEncoders(const Encoders::Data& evt){
	LEDService* led = (LEDService*)Services.get(Service::LED);

	if((evt.enc == Encoders::Pinch || evt.enc == Encoders::Arm) && !armUnlocked) return;

	if(evt.enc == Encoders::Arm){
		armPos = std::clamp(armPos + ArmDirectionMultiplier * evt.dir, 0, 100);
		comm.sendArmPos(armPos);

		if (led != nullptr) {
			led->blink(evt.dir > 0 ? LED::ArmDown : LED::ArmUp);
		}
	}else if(evt.enc == Encoders::Pinch){
		pinchPos = std::clamp(pinchPos + PinchDirectionMultiplier * evt.dir, 0, 100);
		comm.sendArmPinch(pinchPos);

		if (led != nullptr) {
			led->blink(evt.dir > 0 ? LED::PinchOpen : LED::PinchClose);
		}
	}else if(evt.enc == Encoders::Cam){
		camPos = std::clamp(camPos + CameraDirectionMultiplier * evt.dir, 0, 100);
		comm.sendCameraRotation(camPos);
	}
}
