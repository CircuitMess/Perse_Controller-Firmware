#include "PairState.h"
#include "Util/Services.h"
#include "DriveState.h"
#include "Services/LEDService.h"

PairState::PairState(bool connectionError) : State(), evts(10){
	Events::listen(Facility::Input, &evts);
	Events::listen(Facility::Pair, &evts);

	if(!connectionError) return;
	if(auto* led = (LEDService*) Services.get(Service::LED)){
		led->off(LED::Pair);
	}
}

PairState::~PairState(){
	Events::unlisten(&evts);
}

void PairState::loop(){
	Event evt{};
	if(evts.get(evt, portMAX_DELAY)){
		if(evt.facility == Facility::Input && evt.data != nullptr){
			auto data = (Input::Data*) evt.data;
			processInput(*data);
		}else if(evt.facility == Facility::Pair && pair){
			auto success = ((PairService::Event*) evt.data)->success;
			pair.reset();

			if(success){
				if(auto* stateMachine = (StateMachine*) Services.get(Service::StateMachine)){
					stateMachine->transition<DriveState>();
				}
			}else{
				if(auto* led = (LEDService*) Services.get(Service::LED)){
					led->blink(LED::Pair, 4, 500);
					led->blink(LED::Warning, 4, 500);
				}
			}

			return;
		}
		free(evt.data);
	}
}

void PairState::processInput(const Input::Data& evt){
	if(evt.btn != Input::Pair) return;

	auto* led = (LEDService*) Services.get(Service::LED);

	if(evt.action == Input::Data::Press && !pair){
		pair = std::make_unique<PairService>();

		if(led != nullptr){
			led->blink(LED::Pair, -1, PairBlinkInterval);
		}
	}else if(evt.action == Input::Data::Release && pair){
		pair.reset();

		if(led != nullptr){
			led->off(LED::Pair);
		}
	}
}

void PairState::unblock(){
	evts.unblock();
}
