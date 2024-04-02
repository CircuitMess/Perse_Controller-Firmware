#include "DriveState.h"
#include "Util/Services.h"
#include "Util/stdafx.h"
#include "Services/LEDService.h"
#include "PairState.h"
#include "Periph/WiFiSTA.h"
#include "vec2.hpp"
#include "geometric.hpp"
#include "gtx/vector_angle.hpp"

DriveState::DriveState() : comm(*((Comm*) Services.get(Service::Comm))), evts(12){

	if(auto led = (LEDService*) Services.get(Service::LED)){
		led->on(LED::Pair);
		led->on(LED::Navigation);
	}

	Events::listen(Facility::TCP, &evts);
	Events::listen(Facility::Input, &evts);

	// Turn off feed for basic controller, this way there will be no unnecessary network clutter
	comm.sendFeedQuality(0);
	comm.sendScanningEnable(false);
}

DriveState::~DriveState(){
	Events::unlisten(&evts);

	if(auto led = (LEDService*) Services.get(Service::LED)){
		led->off(LED::Navigation);
		led->off(LED::SoundLight);
		led->off(LED::ArmPinch);
	}
}

void DriveState::loop(){
	Event evt{};
	if(evts.get(evt, armMovement ? 0 : portMAX_DELAY)){
		if(evt.facility == Facility::TCP){
			if(const auto data = (TCPClient::Event*) evt.data){
				if(data->status == TCPClient::Event::Status::Disconnected){
					if(auto led = (LEDService*) Services.get(Service::LED)){
						led->blink(LED::Pair, 4, 500);
						led->blink(LED::Warning, 4, 500);
					}

					if(auto stateMachine = (StateMachine*) Services.get(Service::StateMachine)){
						stateMachine->transition<PairState>(false);
					}

					free(evt.data);
					return;
				}
			}
		}else if(evt.facility == Facility::Input){
			auto data = (Input::Data*) evt.data;
			processInput(*data);
		}

		free(evt.data);
	}

	if(armMovement){
		if(millis() - armControlStartTime >= ArmControlInterval){
			armControlStartTime = millis();
			sendArmControl();
		}
	}

	if(WiFiSTA* wifi = (WiFiSTA*) Services.get(Service::WiFi)){
		const ConnectionStrength str = wifi->getConnectionStrength();

		if(str != lastSentStr){
			comm.sendConnectionStrength(str);
			lastSentStr = str;
		}
	}
}

void DriveState::processInput(const Input::Data& evt){
	if(evt.btn == Input::Pair) return;

	if(evt.btn == Input::Mode && evt.action == Input::Data::Press){
		auto nextMode = (ControlMode) (((uint8_t) controlMode + 1) % (uint8_t) ControlMode::COUNT);
		changeMode(nextMode);
	}else if(evt.btn == Input::Up || evt.btn == Input::Down || evt.btn == Input::Left || evt.btn == Input::Right){
		switch(controlMode){
			case ControlMode::Navigation: {
				sendDriveDir();
				break;
			}
			case ControlMode::ArmPinch: {
				processArmInput(evt);
				break;
			}
			case ControlMode::SoundLight: {
				processSoundInput(evt);
				break;
			}
			default: {
				break;
			}
		}
	}
}

void DriveState::changeMode(DriveState::ControlMode nextMode){
	if(controlMode == ControlMode::ArmPinch){
		comm.sendArmEnabled(false);
	}
	controlMode = nextMode;

	auto led = (LEDService*) Services.get(Service::LED);
	if(!led) return;

	led->off(LED::Navigation);
	led->off(LED::ArmPinch);
	led->off(LED::SoundLight);

	switch(controlMode){
		case ControlMode::Navigation:
			led->on(LED::Navigation);
			break;
		case ControlMode::SoundLight:
			led->on(LED::SoundLight);
			break;
		case ControlMode::ArmPinch:
			led->on(LED::ArmPinch);
			comm.sendArmEnabled(true);
			break;
		default:
			break;
	}

	comm.sendDriveDir({ 0, 0 });
	armMovement = false;
}


void DriveState::processArmInput(const Input::Data& evt){
	auto input = (Input*) Services.get(Service::Input);
	if(!input) return;

	armDir = input->getState(Input::Up) * (int8_t) (1) + input->getState(Input::Down) * (int8_t) (-1);
	pinchDir = input->getState(Input::Left) * (int8_t) (1) + input->getState(Input::Right) * (int8_t) (-1);

	if(armDir != 0 || pinchDir != 0){
		armMovement = true;
		armControlStartTime = millis();
		sendArmControl();
	}else{
		armMovement = false;
	}
}

void DriveState::processSoundInput(const Input::Data& evt){
	if(evt.action != Input::Data::Press) return;

	switch(evt.btn){
		case Input::Up:
			comm.sendAudio(true);
			break;
		case Input::Down:
			comm.sendAudio(false);
			break;
		case Input::Left:
			comm.sendHeadlights(HeadlightsMode::Off);
			break;
		case Input::Right:
			comm.sendHeadlights(HeadlightsMode::On);
			break;
		default:
			break;
	}
}

void DriveState::sendDriveDir(){
	auto input = (Input*) Services.get(Service::Input);
	if(!input) return;

	glm::vec2 dir{};
	dir.y = (input->getState(Input::Up) * 1.0f + input->getState(Input::Down) * -1.0f);
	dir.x = (input->getState(Input::Right) * 1.0f + input->getState(Input::Left) * -1.0f);

	if(dir.x == 0 && dir.y == 0){
		if(shouldSendZeroDrive){
			comm.sendDriveDir({ 0, 0 });
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

	comm.sendDriveDir({ number, 1.0f });
}

void DriveState::sendArmControl(){
	if(!armMovement || (armDir == 0 && pinchDir == 0)) return;

	if(armDir != 0){
		armPos = std::clamp(armPos + ArmDirectionMultiplier * armDir, 0, 100);
		comm.sendArmPos(armPos);
	}
	if(pinchDir != 0){
		pinchPos = std::clamp(pinchPos + PinchDirectionMultiplier * pinchDir, 0, 100);
		comm.sendArmPinch(pinchPos);
	}
}

void DriveState::unblock(){
	evts.unblock();
}
