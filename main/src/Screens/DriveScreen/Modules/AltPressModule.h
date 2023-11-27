#ifndef PERSE_MISSIONCTRL_ALTPRESSMODULE_H
#define PERSE_MISSIONCTRL_ALTPRESSMODULE_H


#include "ModuleElement.h"

class AltPressModule : public ModuleElement {
public:
	AltPressModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~AltPressModule() override = default;

private:
	LabelElement altLabel;
	LabelElement pressLabel;
	LabelElement pressValueLabel;
	void dataReceived(ModuleData data) override;

	static constexpr uint8_t altValLength = 5; //4 digits + sign
	static constexpr uint8_t pressValLength = 4;

};


#endif //PERSE_MISSIONCTRL_ALTPRESSMODULE_H
