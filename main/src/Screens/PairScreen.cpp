#include "PairScreen.h"
#include "Devices/Input.h"
#include "DriveScreen/DriveScreen.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

PairScreen::PairScreen(Sprite& canvas, bool disconnectOccurred) : Screen(canvas), evts(6), disconnectMessageActive(disconnectOccurred),
																  frame(this, "/spiffs/pair/frame.raw", 122, 56),
																  controllerRover(this, "/spiffs/pair/controller_rover.raw", 122, 41),
																  signalAnim(this, "/spiffs/pair/signal.gif"),
																  buttonAnim(this, "/spiffs/pair/button.gif"),
																  error(this, "/spiffs/pair/error.raw", 40, 40),
																  description(this, TextPaths[0], 70, 54){

	Events::listen(Facility::Input, &evts);

	setBgColor(BackgroundColor);
	frame.setPos(3, 63);
	controllerRover.setPos(3, 12);
	signalAnim.setPos(-100, -100);
	description.setPos(54, 64);
	buttonAnim.setLoopMode(GIF::Single);
	signalAnim.setLoopMode(GIF::Infinite);

	//shifting buttonAnim and error in and out of view
	if(disconnectOccurred){
		buttonAnim.setPos(-100, -100);
		error.setPos(ErrorPos.x, ErrorPos.y);
		description.setPath(TextPaths[2]);
		startTime = millis();
	}else{
		buttonAnim.setPos(ButtonPos.x, ButtonPos.y);
		error.setPos(-100, -100);
		description.setPath(TextPaths[0]);
	}

	signalAnim.start();
	signalAnim.stop();
	signalAnim.reset();
	buttonAnim.start();
	buttonAnim.stop();
	buttonAnim.reset();

	Input* input = (Input*) Services.get(Service::Input);
	if(input == nullptr){
		return;
	}

	if(input->getState(Input::Button::Pair)){
		pair = std::make_unique<PairService>();
		description.setPath(TextPaths[0]);
		this->disconnectMessageActive = false;
		buttonAnim.start();
		signalAnim.start();
		buttonAnim.setPos(ButtonPos.x, ButtonPos.y);
		signalAnim.setPos(SignalPos.x, SignalPos.y);
		error.setPos(-100, -100);
	}
}

PairScreen::~PairScreen(){
	Events::unlisten(&evts);
}

void PairScreen::onLoop(){
	if(disconnectMessageActive && millis() - startTime >= DisconnectMessageDuration){
		description.setPath(TextPaths[0]);
		buttonAnim.setPos(ButtonPos.x, ButtonPos.y);
		error.setPos(-100, -100);
		disconnectMessageActive = false;
	}

	{
		Event evt{};
		if(evts.get(evt, 0)){
			if(evt.facility == Facility::Input && evt.data != nullptr){
				auto data = (Input::Data*) evt.data;

				if(data != nullptr){
					processInput(*data);
				}
			}

			free(evt.data);
		}
	}

	if(pair && pair->getState() != PairService::State::Pairing){
		volatile const auto state = pair->getState();
		pair.reset();

		if(state == PairService::State::Success){
			transition([](Sprite& canvas){ return std::make_unique<DriveScreen>(canvas); });
		}else if(state == PairService::State::Fail){
			error.setPos(ErrorPos.x, ErrorPos.y);
			buttonAnim.setPos(-100, -100);
			signalAnim.setPos(-100, -100);
			buttonAnim.stop();
			buttonAnim.reset();
			signalAnim.stop();
			signalAnim.reset();
			description.setPath(TextPaths[1]);
		}

		return;
	}
}

void PairScreen::processInput(const Input::Data& evt){
	if(evt.btn != Input::Pair) return;

	if(evt.action == Input::Data::Press && !pair){
		pair = std::make_unique<PairService>();
		description.setPath(TextPaths[0]);
		disconnectMessageActive = false;
		buttonAnim.start();
		signalAnim.start();
		buttonAnim.setPos(ButtonPos.x, ButtonPos.y);
		signalAnim.setPos(SignalPos.x, SignalPos.y);
		error.setPos(-100, -100);
	}else if(evt.action == Input::Data::Release && pair){
		pair.reset();
		signalAnim.setPos(-100, -100);
		buttonAnim.reset();
		buttonAnim.stop();
		signalAnim.reset();
		signalAnim.stop();
	}
}
