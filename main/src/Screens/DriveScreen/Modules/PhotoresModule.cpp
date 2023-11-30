#include "PhotoresModule.h"

PhotoresModule::PhotoresModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), percentLabel(this, ""){

	auto style = percentLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	percentLabel.setStyle(style);
	percentLabel.setText(std::string(valLength, ' ') + '%');
	percentLabel.setPos(0, 9);
}

void PhotoresModule::dataReceived(ModuleData data){
	percentLabel.setText(paddedValueLeft(data.photoRes.level, valLength) + '%');
}

