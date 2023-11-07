#ifndef PERSE_MISSIONCTRL_DUMMYMODULE_H
#define PERSE_MISSIONCTRL_DUMMYMODULE_H

#include "ModuleElement.h"

class DummyModule : public ModuleElement {
public:
	DummyModule(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~DummyModule() override = default;
private:
	void dataReceived(ModuleData data) override;
};


#endif //PERSE_MISSIONCTRL_DUMMYMODULE_H
