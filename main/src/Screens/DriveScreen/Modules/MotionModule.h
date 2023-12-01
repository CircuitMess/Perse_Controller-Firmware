#ifndef PERSE_MISSIONCTRL_MOTIONMODULE_H
#define PERSE_MISSIONCTRL_MOTIONMODULE_H

#include "ModuleElement.h"
#include "Util/stdafx.h"

class MotionModule : public ModuleElement {
public:
	MotionModule(ElementContainer* parent, const ModuleBus& bus, const ModuleType& type);
	~MotionModule() override = default;
	void loop() override;
private:
	LabelElement statusLabel;
	bool status = false;
	uint32_t lastMillis = millis();
	bool flashStatus = true;
	uint8_t dotCounter = 0;
	void dataReceived(ModuleData data) override;

	static constexpr uint32_t FlashingInterval = 500;
	static constexpr uint8_t DotLimit = 4;
};


#endif //PERSE_MISSIONCTRL_MOTIONMODULE_H
