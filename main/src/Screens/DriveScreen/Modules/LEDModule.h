#ifndef PERSE_MISSIONCTRL_LEDMODULE_H
#define PERSE_MISSIONCTRL_LEDMODULE_H

#include "ModuleElement.h"

class LEDModule : public ModuleElement{
public:
	LEDModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~LEDModule() override = default;
private:
	LabelElement statusLabel;
	void dataReceived(ModuleData data) override;
};


#endif //PERSE_MISSIONCTRL_LEDMODULE_H
