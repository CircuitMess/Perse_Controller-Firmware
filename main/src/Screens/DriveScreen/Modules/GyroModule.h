#ifndef PERSE_MISSIONCTRL_GYROMODULE_H
#define PERSE_MISSIONCTRL_GYROMODULE_H

#include "ModuleElement.h"

class GyroModule : public ModuleElement{
public:
	GyroModule(ElementContainer* parent, ModuleBus bus, ModuleType type);

private:
	LabelElement dataLabel;
	void dataReceived(ModuleData data) override;
};


#endif //PERSE_MISSIONCTRL_GYROMODULE_H
