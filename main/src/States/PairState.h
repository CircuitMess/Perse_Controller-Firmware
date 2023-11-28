#ifndef PERSE_CTRL_PAIRSTATE_H
#define PERSE_CTRL_PAIRSTATE_H

#include <memory>
#include "Services/StateMachine.h"
#include "Util/Events.h"
#include "Services/PairService.h"
#include "Devices/Input.h"

class PairState : public State {
public:
	explicit PairState();
	~PairState() override;

protected:
	void loop() override;

private:
	EventQueue evts;
	std::unique_ptr<PairService> pair = nullptr;

	void processInput(const Input::Data& evt);

	static constexpr uint32_t PairBlinkInterval = 350;
};

#endif //PERSE_CTRL_PAIRSTATE_H
