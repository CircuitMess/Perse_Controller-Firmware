#include "RoverState.h"

RoverState::RoverState() : Threaded("RoverState", 2048), evts(12){
	Events::listen(Facility::Comm, &evts);
	start();
}

void RoverState::loop(){
	Event evt{};
	if(!evts.get(evt, portMAX_DELAY)) return;

	free(evt.data);
}
