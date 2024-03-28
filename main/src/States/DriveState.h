#ifndef PERSE_CTRL_DRIVESTATE_H
#define PERSE_CTRL_DRIVESTATE_H

#include <CommData.h>
#include "Services/StateMachine.h"
#include "Services/Comm.h"
#include "Devices/Input.h"

class DriveState : public State {
public:
	explicit DriveState();
	~DriveState() override;

protected:
	void loop() override;

private:
	static constexpr uint64_t ArmControlInterval = 40; // [ms]
	static constexpr int8_t ArmDirectionMultiplier = 2;
	static constexpr int8_t PinchDirectionMultiplier = 5;

	Comm& comm;

	EventQueue evts;

	enum class ControlMode : uint8_t {
		Navigation, ArmPinch, SoundLight, COUNT
	} controlMode = ControlMode::Navigation;


	bool armMovement = false;
	int8_t armDir = 0;
	int8_t pinchDir = 0;

	uint8_t pinchPos = 50;
	uint8_t armPos = 50;
	uint32_t armControlStartTime = 0;

	ConnectionStrength lastSentStr = ConnectionStrength::None;

	bool shouldSendZeroDrive = true;

	void processInput(const Input::Data& evt);
	void changeMode(ControlMode nextMode);

	void processArmInput(const Input::Data& evt);
	void processSoundInput(const Input::Data& evt);
	void sendDriveDir();
	void sendArmControl();

	void unblock() override;
};


#endif //PERSE_CTRL_DRIVESTATE_H
