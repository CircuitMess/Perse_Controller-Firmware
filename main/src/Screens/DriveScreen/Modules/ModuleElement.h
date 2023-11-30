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
	static constexpr uint16_t textColor = TFT_GREEN;

	/**
	 * @brief Pads a value with spaces to the left to make it a certain length
	 * @tparam T Type of value
	 * @param value Value to pad
	 * @param MaxLength Maximum length of the value
	 * @return String representation of value padded to MaxLength with spaces
	 */
	template <typename T>
	std::string paddedValueLeft(T value, uint8_t MaxLength){
		auto s = std::to_string(value);
		return std::string(std::max((int) (MaxLength - s.length()), (int) 0), ' ') + s;
	}

	template <typename T>
	std::string paddedValueRight(T value, uint8_t MaxLength){
		auto s = std::to_string(value);
		return s + std::string(std::max((int) (MaxLength - s.length()), (int) 0), ' ');
	}

private:
	LabelElement nameLabel;
	EventQueue queue;

	virtual void dataReceived(ModuleData data) = 0;
};


#endif //PERSE_MISSIONCTRL_MODULEELEMENT_H
