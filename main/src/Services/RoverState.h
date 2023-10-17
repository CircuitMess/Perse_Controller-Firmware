#ifndef PERSE_MISSIONCTRL_ROVERSTATE_H
#define PERSE_MISSIONCTRL_ROVERSTATE_H

#include "Util/Threaded.h"
#include "Util/Events.h"

class RoverState : private Threaded {
public:
	RoverState();

private:
	void loop() override;

	EventQueue evts;

};


#endif //PERSE_MISSIONCTRL_ROVERSTATE_H
