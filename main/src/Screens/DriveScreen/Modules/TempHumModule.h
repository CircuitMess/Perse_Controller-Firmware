#ifndef PERSE_MISSIONCTRL_TEMPHUMMODULE_H
#define PERSE_MISSIONCTRL_TEMPHUMMODULE_H

#include "ModuleElement.h"

class TempHumModule: public ModuleElement {
public:
	TempHumModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~TempHumModule() override = default;

private:
	LabelElement tempValueLabel;
	LabelElement humLabel;
	LabelElement humValueLabel;

	void dataReceived(ModuleData data) override;

	static constexpr uint8_t valueLength = 3; // 2 digits + sign or 3 digits unsigned
};


#endif //PERSE_MISSIONCTRL_TEMPHUMMODULE_H
