#ifndef PERSE_MISSIONCTRL_MODULEELEMENT_H
#define PERSE_MISSIONCTRL_MODULEELEMENT_H

#include "UISystem/Element.h"
#include "UISystem/ElementContainer.h"
#include "CommData.h"
#include "UISystem/LabelElement.h"
#include "Util/Events.h"

class ModuleElement : public Element, protected ElementContainer {
public:
	ModuleElement(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~ModuleElement() override;

	void draw(Sprite* canvas) override;
	void loop() override;

protected:
	const ModuleBus bus;
	const ModuleType type;
	const lgfx::textdatum_t datum;

private:
	LabelElement nameLabel;
	EventQueue queue;

	virtual void dataReceived(ModuleData data) = 0;
};


#endif //PERSE_MISSIONCTRL_MODULEELEMENT_H
