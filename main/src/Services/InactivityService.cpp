#include "InactivityService.h"
#include <esp_log.h>
#include "Util/stdafx.h"
#include "Comm.h"
#include "PairService.h"
#include "Devices/Encoders.h"
#include "Util/Services.h"
#include "geometric.hpp"

static const char* TAG = "InactivityService";

InactivityService::InactivityService() : Threaded("Inactivity", 2 * 1024), queue(24){
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Encoders, &queue);
	Events::listen(Facility::Potentiometers, &queue);
	Events::listen(Facility::Pair, &queue);
	Events::listen(Facility::TCP, &queue);

#ifdef CTRL_TYPE_MISSIONCTRL
	joystick = (Joystick*) Services.get(Service::Joystick);
#endif

	timer = millis();
	start();
}

void InactivityService::loop(){
	if(checkActions()){
		timer = millis();
		return;
	}

	if((paired && millis() - timer >= PairedTimeout) || (!paired && millis() - timer >= UnpairedTimeout)){
		Events::unlisten(&queue);
		ESP_LOGI(TAG, "Inactivity shutdown!");

		extern void shutdown();
		extern void preShutdown();

		preShutdown();
		shutdown();
	}
}

bool InactivityService::checkActions(){
	bool action = false;

#ifdef CTRL_TYPE_MISSIONCTRL
	const auto currPos = joystick->readAndGetPos();
	if(glm::length(currPos) >= JoystickStep){
		action = true;
		ESP_LOGD(TAG, "Joystick action reset");
	}
#endif

	Event e{};
	if(!queue.get(e, 1000)) return false;

	if(e.data == nullptr){
		return false;
	}

	switch(e.facility){
		case Facility::Encoders:{
			action = true;
			ESP_LOGD(TAG, "Encoders action reset");
			break;
		}
		case Facility::Potentiometers:{
			action = true;
			ESP_LOGD(TAG, "Potentiometers action reset");
			break;
		}
		case Facility::Pair:{
			auto* pairData = (PairService::Event*) e.data;
			action = true;
			paired = pairData->success;
			ESP_LOGD(TAG, "Pair action reset");
			break;
		}
		case Facility::Input:{
			action = true;
			ESP_LOGD(TAG, "Input action reset");
			break;
		}
		case Facility::TCP:{
			auto* tcpData = (TCPClient::Event*) e.data;
			action = true;
			paired = tcpData->status == TCPClient::Event::Status::Connected;
			ESP_LOGD(TAG, "TCP action reset");
			break;
		}

		default:
			break;
	}
	free(e.data);


	return action;
}
