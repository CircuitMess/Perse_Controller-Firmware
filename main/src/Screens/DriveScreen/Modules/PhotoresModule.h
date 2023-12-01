#ifndef PERSE_MISSIONCTRL_PHOTORESMODULE_H
#define PERSE_MISSIONCTRL_PHOTORESMODULE_H

#include "ModuleElement.h"

class PhotoresModule : public ModuleElement{
public:
	PhotoresModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~PhotoresModule() override = default;
private:
	LabelElement percentLabel;
	void dataReceived(ModuleData data) override;

	static constexpr uint8_t valLength = 3; //3 digits
};


#endif //PERSE_MISSIONCTRL_PHOTORESMODULE_H
