#ifndef PERSE_MISSIONCTRL_CO2MODULE_H
#define PERSE_MISSIONCTRL_CO2MODULE_H

#include "ModuleElement.h"
#include "Util/stdafx.h"

class CO2Module : public ModuleElement {
public:
	CO2Module(ElementContainer* parent, ModuleBus bus, ModuleType type);
	~CO2Module() override = default;
	void loop() override;

private:
	LabelElement statusLabel;
	bool status = true;
	uint32_t lastMillis = millis();
	bool flashStatus = true;
	void dataReceived(ModuleData data) override;

	static constexpr uint16_t OKColor = TFT_GREEN;
	static constexpr uint16_t NOKColor = TFT_RED;
	static constexpr uint32_t FlashingInterval = 500;
};


#endif //PERSE_MISSIONCTRL_CO2MODULE_H
