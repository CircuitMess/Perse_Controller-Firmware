#include "AltPressModule.h"


AltPressModule::AltPressModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), altLabel(this, ""),
																						   pressLabel(this, ""), pressValueLabel(this, ""){

	auto style = altLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	style.shadingStyle = { TFT_BLACK, 1 };
	altLabel.setStyle(style);
	altLabel.setText(std::string(altValLength, ' ') + 'M');
	altLabel.setPos(0, 9);

	pressLabel.setStyle(style);
	pressLabel.setText("PRS");
	pressLabel.setPos(0, 18);

	pressValueLabel.setStyle(style);
	pressValueLabel.setText(std::string(pressValLength, ' ') + "HPA");
	pressValueLabel.setPos(0, 27);
}

void AltPressModule::dataReceived(ModuleData data){
	altLabel.setText(paddedValueRight(data.altPress.altitude, altValLength) + 'M');
	pressValueLabel.setText(paddedValueRight(data.altPress.pressure, pressValLength) + "HPA");
}
