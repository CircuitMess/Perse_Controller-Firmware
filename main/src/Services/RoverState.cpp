#include "RoverState.h"
#include "Comm.h"

RoverState::RoverState() : Threaded("RoverState", 2048), evts(12),
						   headlightsState(HeadlightsMode::Off), armPinch(50), armPos(50), cameraRotation(50), batteryPercent(0) {
	Events::listen(Facility::Comm, &evts);
	start();
}

void RoverState::loop(){
	::Event evt{};
	if(!evts.get(evt, portMAX_DELAY)) return;

	Event e{};

	if (Comm::Event* event = (Comm::Event*)evt.data) {
		switch (event->type) {
			case (CommType::Headlights): {
				headlightsState = event->headlights;
				e.type = StateType::Headlights;
				e.headlights = headlightsState;
				break;
			}
			case (CommType::ArmPinch): {
				armPinch = event->armPinch;
				e.type = StateType::ArmPinch;
				e.armPinch = armPinch;
				break;
			}
			case (CommType::ArmPosition): {
				armPos = event->armPos;
				e.type = StateType::ArmPos;
				e.armPos = armPos;
				break;
			}
			case (CommType::CameraRotation): {
				cameraRotation = event->cameraRotation;
				e.type = StateType::CameraRotation;
				e.cameraRotation = cameraRotation;
				break;
			}
			case (CommType::Battery): {
				batteryPercent = event->batteryPercent;
				e.type = StateType::BatteryPercent;
				e.batteryPercent = batteryPercent;
				break;
			}
			case (CommType::ModulePlug) : {
				if(event->modulePlug.bus == ModuleBus::Left){
					leftModuleInsert = event->modulePlug.insert;
					leftModuleType = event->modulePlug.type;
				}else if(event->modulePlug.bus == ModuleBus::Right){
					rightModuleInsert = event->modulePlug.insert;
					rightModuleType = event->modulePlug.type;
				}

				e.type = StateType::Modules;
				e.modulePlug = event->modulePlug;
				break;
			}
			default: {
				break;
			}
		}

		Events::post(Facility::RoverState, e);
	}

	free(evt.data);
}
