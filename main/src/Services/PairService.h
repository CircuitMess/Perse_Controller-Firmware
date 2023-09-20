#ifndef PERSE_MISSIONCTRL_PAIRSERVICE_H
#define PERSE_MISSIONCTRL_PAIRSERVICE_H


#include "Periph/WiFiSTA.h"
#include "TCPClient.h"
#include "Util/Threaded.h"
#include "Util/Events.h"

class PairService {
public:
	PairService();
	virtual ~PairService();

	enum class State{
		Pairing, Fail, Success
	};
	State getState() const;

private:
	WiFiSTA& wifi;
	TCPClient& tcp;

	State state = State::Pairing;
	ThreadedClosure thread;
	EventQueue queue;

	void loop();
};


#endif //PERSE_MISSIONCTRL_PAIRSERVICE_H