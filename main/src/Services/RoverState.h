#ifndef PERSE_MISSIONCTRL_ROVERSTATE_H
#define PERSE_MISSIONCTRL_ROVERSTATE_H

#include <atomic>
#include <CommData.h>
#include "Util/Threaded.h"
#include "Util/Events.h"

class RoverState : private Threaded {
public:
	enum class StateType {
		None,
		Headlights,
		ArmPinch,
		ArmPos,
		CameraRotation,
		BatteryPercent,
		Modules
	};

	struct Event {
		StateType type = StateType::None;

		union{
			HeadlightsMode headlights = HeadlightsMode::Off;
			ArmPos armPos;
			ArmPinch armPinch;
			CameraRotation cameraRotation;
			uint8_t batteryPercent;
			ModulePlugData modulePlug;
		};
	};

	RoverState();

	inline HeadlightsMode getHeadlightsState() const { return headlightsState; }
	inline ArmPinch getArmPinchState() const { return armPinch; }
	inline ArmPos getArmPositionState() const { return armPos; }
	inline CameraRotation getCameraRotationState() const { return cameraRotation; }
	inline uint8_t getBatteryPercent() const { return batteryPercent; }
	inline bool getLeftModuleInsert() const { return leftModuleInsert; }
	inline ModuleType getLeftModuleType() const { return leftModuleType; }
	inline bool getRightModuleInsert() const { return rightModuleInsert; }
	inline ModuleType getRightModuleType() const { return rightModuleType; }

private:
	void loop() override;

private:
	EventQueue evts;

	// States
	std::atomic<HeadlightsMode> headlightsState;
	std::atomic<ArmPinch> armPinch;
	std::atomic<ArmPos> armPos;
	std::atomic<CameraRotation> cameraRotation;
	std::atomic<uint8_t> batteryPercent;
	std::atomic<bool> leftModuleInsert;
	std::atomic<ModuleType> leftModuleType;
	std::atomic<bool> rightModuleInsert;
	std::atomic<ModuleType> rightModuleType;
};


#endif //PERSE_MISSIONCTRL_ROVERSTATE_H
