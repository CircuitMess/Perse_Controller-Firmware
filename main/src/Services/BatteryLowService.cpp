#include "BatteryLowService.h"
#include "RoverState.h"
#include "LEDService.h"
#include "Util/Services.h"
#include "Devices/Battery.h"
#include "Util/stdafx.h"

BatteryLowService::BatteryLowService() : Threaded("BattLowService", 2 * 1024), queue(6){
	Events::listen(Facility::Battery, &queue);
	Events::listen(Facility::RoverState, &queue);
	start();
}

BatteryLowService::~BatteryLowService(){
	stop(0);
	queue.unblock();

	while(running()){
		delayMillis(1);
	}
	Events::unlisten(&queue);
}

void BatteryLowService::loop(){
	Event event{};
	if(!queue.get(event, portMAX_DELAY)) return;

	if(event.facility == Facility::RoverState && event.data != nullptr){
		auto* data = (RoverState::Event*) event.data;
		if(data->type == RoverState::StateType::BatteryPercent){
			if(data->batteryPercent < CriticalPercent){
				if(warningState != WarningLEDState::Critical){
					auto led = (LEDService*) Services.get(Service::LED);
					led->blink(LED::Warning, 0, CriticalBlinkPeriod);
					warningState = WarningLEDState::Critical;
				}
			}else if(data->batteryPercent < LowPercent){
				if(warningState != WarningLEDState::Low){
					auto led = (LEDService*) Services.get(Service::LED);
					led->breathe(LED::Warning);
					warningState = WarningLEDState::Low;
				}
			}
		}
	}else if(event.facility == Facility::Battery && event.data != nullptr){
		auto* data = (Battery::Event*) event.data;
		if(data->action == Battery::Event::LevelChange){
			if(data->level == Battery::Low){
				auto led = (LEDService*) Services.get(Service::LED);
				led->breathe(LED::Power);
			}else if(data->level == Battery::VeryLow){
				auto led = (LEDService*) Services.get(Service::LED);
				led->blink(LED::Power, 0, CriticalBlinkPeriod);
			}
		}
	}

	free(event.data);
}
