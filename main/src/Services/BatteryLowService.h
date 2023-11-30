#ifndef PERSE_CTRL_BATTERYLOWSERVICE_H
#define PERSE_CTRL_BATTERYLOWSERVICE_H

#include "Util/Threaded.h"
#include "Util/Events.h"

class BatteryLowService : private Threaded {
public:
	BatteryLowService();
	~BatteryLowService() override;

private:
	EventQueue queue;
	enum class WarningLEDState : uint8_t {
		None, Low, Critical
	} warningState = WarningLEDState::None;

	void loop() override;

	static constexpr uint8_t CriticalPercent = 8;
	static constexpr uint8_t LowPercent = 20;
	static constexpr uint32_t CriticalBlinkPeriod = 800;

};


#endif //PERSE_CTRL_BATTERYLOWSERVICE_H
