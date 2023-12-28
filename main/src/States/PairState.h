#ifndef PERSE_CTRL_PAIRSTATE_H
#define PERSE_CTRL_PAIRSTATE_H

#include <memory>
#include "Services/StateMachine.h"
#include "Util/Events.h"
#include "Services/PairService.h"
#include "Devices/Input.h"

class PairState : public State {
public:
	explicit PairState(bool connectionError = false);
	~PairState() override;

protected:
	void loop() override;

private:
	EventQueue evts;
	std::unique_ptr<PairService> pair = nullptr;

	void processInput(const Input::Data& evt);

	static constexpr uint32_t PairBlinkInterval = 350;

	void unblock() override;
};

#endif //PERSE_CTRL_PAIRSTATE_H
