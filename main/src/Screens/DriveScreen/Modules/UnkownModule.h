#ifndef PERSE_MISSIONCTRL_UNKOWNMODULE_H
#define PERSE_MISSIONCTRL_UNKOWNMODULE_H

#include "ModuleElement.h"

class UnkownModule : public ModuleElement {
public:
	UnkownModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~UnkownModule() override = default;
private:
	void dataReceived(ModuleData data) override;
};


#endif //PERSE_MISSIONCTRL_UNKOWNMODULE_H
