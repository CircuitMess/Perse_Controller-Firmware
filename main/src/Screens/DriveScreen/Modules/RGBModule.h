#ifndef PERSE_MISSIONCTRL_RGBMODULE_H
#define PERSE_MISSIONCTRL_RGBMODULE_H

#include "ModuleElement.h"

class RGBModule : public ModuleElement{
public:
	RGBModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~RGBModule() override = default;
private:
	void dataReceived(ModuleData data) override;
};


#endif //PERSE_MISSIONCTRL_RGBMODULE_H
