#include "DriveScreen.h"
#include "Services/TCPClient.h"
#include "Screens/PairScreen.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Pins.hpp"
#include "glm.hpp"
#include "gtx/vector_angle.hpp"
#include "Modules/AltPressModule.h"
#include "Modules/CO2Module.h"
#include "Modules/GyroModule.h"
#include "Modules/LEDModule.h"
#include "Modules/MotionModule.h"
#include "Modules/PhotoresModule.h"
#include "Modules/RGBModule.h"
#include "Modules/TempHumModule.h"
#include "Modules/UnkownModule.h"

DriveScreen::DriveScreen(Sprite& canvas) : Screen(canvas), dcEvts(6), evts(12), comm(*((Comm*) Services.get(Service::Comm))),
										   led(*((LED*) Services.get(Service::LED))), roverState(*(RoverState*) Services.get(Service::RoverState)),
										   joy(*((Joystick*) Services.get(Service::Joystick))){
	lastFrame.resize(160*120, 0);

	gpio_set_level((gpio_num_t) LED_PAIR, 1);

	comm.sendFeedQuality(1);

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

	gpio_set_level((gpio_num_t) LED_PAIR, 0);

	led.off(EXTLED_ARM);
	led.off(EXTLED_LIGHT);

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
	if(roverState.getLeftModuleInsert()){
		createModule(ModuleBus::Left, roverState.getLeftModuleType());
	}

	if(roverState.getRightModuleInsert()){
		createModule(ModuleBus::Right, roverState.getRightModuleType());
	}
}

void DriveScreen::setupControl(){
	auto input = (Input*) Services.get(Service::Input);

	Events::listen(Facility::Input, &evts);
	Events::listen(Facility::Encoders, &evts);
	Events::listen(Facility::RoverState, &evts);

	comm.sendModulesEnable(true);

	if(input->getState(Input::SwLight)){
		comm.sendHeadlights(HeadlightsMode::On);
		led.on(EXTLED_LIGHT);
	}

	armUnlocked = input->getState(Input::SwArm);
	if(armUnlocked){
		led.on(EXTLED_ARM);
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
	}

	free(evt.data);
}

void DriveScreen::processInput(const Input::Data& evt){
	if(evt.btn == Input::SwArm){
		armUnlocked = evt.action == Input::Data::Press;
		if(armUnlocked){
			led.on(EXTLED_ARM);
		}else{
			led.off(EXTLED_ARM);
		}
	}else if(evt.btn == Input::SwLight){
		if(evt.action == Input::Data::Press){
			comm.sendHeadlights(HeadlightsMode::On);
			led.on(EXTLED_LIGHT);
		}else{
			comm.sendHeadlights(HeadlightsMode::Off);
			led.off(EXTLED_LIGHT);
		}
	}
}

void DriveScreen::processEncoders(const Encoders::Data& evt){
	if((evt.enc == Encoders::Pinch || evt.enc == Encoders::Arm) && !armUnlocked) return;

	if(evt.enc == Encoders::Arm){
		armPos = std::clamp(armPos + ArmDirectionMultiplier * evt.dir, 0, 100);
		comm.sendArmPos(armPos);
		led.blink(evt.dir > 0 ? EXTLED_ARM_DOWN : EXTLED_ARM_UP);
	}else if(evt.enc == Encoders::Pinch){
		pinchPos = std::clamp(pinchPos + PinchDirectionMultiplier * evt.dir, 0, 100);
		comm.sendArmPinch(pinchPos);
		led.blink(evt.dir > 0 ? EXTLED_PINCH_OPEN : EXTLED_PINCH_CLOSE);
	}else if(evt.enc == Encoders::Cam){
		camPos = std::clamp(camPos + CameraDirectionMultiplier * evt.dir, 0, 100);
		comm.sendCameraRotation(camPos);
	}
}

void DriveScreen::processRoverState(const RoverState::Event& evt){
	if(evt.type != RoverState::StateType::Modules) return;

	if(evt.modulePlug.bus == ModuleBus::Left){
		if(!evt.modulePlug.insert){
			if(leftModule){
				delete leftModule;
				leftModule = nullptr;
			}
		}else if(leftModule == nullptr){
			createModule(ModuleBus::Left, evt.modulePlug.type);
		}
	}

	if(evt.modulePlug.bus == ModuleBus::Right){
		if(!evt.modulePlug.insert){
			if(rightModule){
				delete rightModule;
				rightModule = nullptr;
			}
		}else if(rightModule == nullptr){
			createModule(ModuleBus::Right, evt.modulePlug.type);
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

	if(bus == ModuleBus::Left){
		leftModule = module;
		leftModule->setPos(2, 30);
	}else{
		rightModule = module;
		rightModule->setPos(126, 30);
	}
}
