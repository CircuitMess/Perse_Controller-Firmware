#ifndef PERSE_MISSIONCTRL_GYROMODULE_H
#define PERSE_MISSIONCTRL_GYROMODULE_H

#include "ModuleElement.h"

class GyroModule : public ModuleElement{
public:
	GyroModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~GyroModule() override = default;

private:
	LabelElement xLabel;
	LabelElement yLabel;
	void dataReceived(ModuleData data) override;

	static constexpr uint8_t gyroValLength = 4; //3 digits + sign
};


#endif //PERSE_MISSIONCTRL_GYROMODULE_H
