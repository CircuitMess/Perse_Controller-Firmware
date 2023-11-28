#include "PairState.h"
#include "Util/Services.h"
#include "DriveState.h"
#include "Services/LEDService.h"

PairState::PairState() : State(), evts(10){
	Events::listen(Facility::Input, &evts);

	if(auto* led = (LEDService*) Services.get(Service::LED)){
		led->off(LED::Pair);
	}
}

PairState::~PairState(){
	Events::unlisten(&evts);
}

void PairState::loop(){
	Event evt{};
	if(evts.get(evt, 0)){
		if(evt.facility == Facility::Input && evt.data != nullptr){
			auto data = (Input::Data*) evt.data;
			processInput(*data);
		}
		free(evt.data);
	}

	if(pair && pair->getState() != PairService::State::Pairing){
		volatile const auto state = pair->getState();
		pair.reset();

		if(state == PairService::State::Success){
			if(StateMachine* stateMachine = (StateMachine*) Services.get(Service::StateMachine)){
				if(auto* led = (LEDService*) Services.get(Service::LED)){
					led->blink(LED::Pair, 2, 500);
				}
				stateMachine->transition<DriveState>();
			}
		}else if(state == PairService::State::Fail){
			if(auto* led = (LEDService*) Services.get(Service::LED)){
				led->blink(LED::Pair, 4, 500);
				led->blink(LED::Warning, 4, 500);
			}
		}

		return;
	}
}

void PairState::processInput(const Input::Data& evt){
	if(evt.btn != Input::Pair) return;

	auto* led = (LEDService*) Services.get(Service::LED);

	if(evt.action == Input::Data::Press && !pair){
		pair = std::make_unique<PairService>();
		led->blink(LED::Pair, -1, PairBlinkInterval);
	}else if(evt.action == Input::Data::Release && pair){
		pair.reset();
		led->off(LED::Pair);
	}
}
